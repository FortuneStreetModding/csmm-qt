#include "tourclearkeytable.h"
#include "lib/powerpcasm.h"

quint32 TourClearKeyTable::writeTable(const QVector<MapDescriptor> &descriptors) {
    QVector<quint32> table;
    for (auto &descriptor: descriptors) table.append(descriptor.tourClearKey);
    return allocate(table, "TourClearKeyTable");
}

void TourClearKeyTable::writeAsm(QDataStream &stream, const AddressMapper &addressMapper, const QVector<MapDescriptor> &mapDescriptors) {
    quint32 tableAddr = writeTable(mapDescriptors);
    PowerPcAsm::Pair16Bit v = PowerPcAsm::make16bitValuePair(tableAddr);

    // --- Update Table Addr ---
    // mulli r0,r3,0x38 ->  mulli r0,r3,0x04
    stream.device()->seek(addressMapper.boomToFileAddress(0x801ccb58)); stream << PowerPcAsm::mulli(0, 3, 0x04);
    // r3 <- 0x80428e50 ->  r3 <- tableAddr
    stream.device()->seek(addressMapper.boomToFileAddress(0x801ccb5c)); stream << PowerPcAsm::lis(3, v.upper) << PowerPcAsm::addi(3, 3, v.lower);
    // lwz r3,0x4(r3)   ->  lwz r3,0x0(r3)
    stream.device()->seek(addressMapper.boomToFileAddress(0x801ccb68)); stream << PowerPcAsm::lwz(3, 0x0, 3);

    // Idea: 0x0f00 - 0x1c00 of the save game file seems to be unused. We will use this area to save the tourClearKey of each board if they have been completed.
    //       The tourClearKey is constrained to be max of 5 characters of only uppercase letters and digits. If we convert that to an integer using base 36,
    //       it will take up at most 3 bytes of memory. We can use the 4th byte for information on clear rank, new state, etc.

}

void TourClearKeyTable::readAsm(QDataStream &stream, QVector<MapDescriptor> &mapDescriptors, const AddressMapper &, bool isVanilla) {
    if (isVanilla) {
        stream.skipRawData(0x30);
    }
    for(int i=0;i<mapDescriptors.length();i++) {
        auto &mapDescriptor = mapDescriptors[i];
        stream >> mapDescriptor.tourClearKey;
        if (isVanilla) {
            // account for the offset (the clear flag of standard maps is stored in memory directly after easy maps)
            if(i<=20)
                mapDescriptor.tourClearKey += 20;
            // in vanilla main.dol the table has other stuff in it like bgm id, map frb files, etc.
            // this we need to skip to go the next target amount in the table
            stream.skipRawData(0x38 - 0x04);
        }
    }
}

quint32 TourClearKeyTable::readTableAddr(QDataStream &stream, const AddressMapper &addressMapper, bool) {
    stream.device()->seek(addressMapper.boomToFileAddress(0x801ccb5c));
    quint32 lisOpcode, addiOpcode;
    stream >> lisOpcode >> addiOpcode;
    return PowerPcAsm::make32bitValueFromPair(lisOpcode, addiOpcode);
}

qint16 TourClearKeyTable::readTableRowCount(QDataStream &, const AddressMapper &, bool) {
    return -1;
}

bool TourClearKeyTable::readIsVanilla(QDataStream &stream, const AddressMapper &addressMapper) {
    stream.device()->seek(addressMapper.boomToFileAddress(0x801ccb58));
    quint32 opcode; stream >> opcode;
    // mulli r0,r3,0x38
    return opcode == PowerPcAsm::mulli(0, 3, 0x38);
}

