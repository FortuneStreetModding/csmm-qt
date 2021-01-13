#include "bgmidtable.h"
#include "lib/powerpcasm.h"

quint32 BGMIDTable::writeTable(const QVector<MapDescriptor> &descriptors) {
    QVector<quint32> table;
    for (auto &descriptor: descriptors) table.append(descriptor.bgmId);
    return allocate(table);
}

void BGMIDTable::writeAsm(QDataStream &stream, const AddressMapper &addressMapper, const QVector<MapDescriptor> &mapDescriptors) {
    quint32 tableAddr = writeTable(mapDescriptors);
    PowerPcAsm::Pair16Bit v = PowerPcAsm::make16bitValuePair(tableAddr);

    // --- Update Table Addr ---
    // mulli r0,r3,0x38 ->  mulli r0,r3,0x04
    stream.device()->seek(addressMapper.boomToFileAddress(0x801cca50)); stream << PowerPcAsm::mulli(0, 3, 0x04);
    // r3 <- 0x80428e50 ->  r3 <- tableAddr
    stream.device()->seek(addressMapper.boomToFileAddress(0x801cca54)); stream << PowerPcAsm::lis(3, v.upper);
    stream.skipRawData(0x4); stream << PowerPcAsm::addi(3, 3, v.lower);
    // lwz r3,0x4(r3)   ->  lwz r3,0x0(r3)
    stream.device()->seek(addressMapper.boomToFileAddress(0x801cca64)); stream << PowerPcAsm::lwz(3, 0x0, 3);
}

void BGMIDTable::readAsm(QDataStream &stream, QVector<MapDescriptor> &mapDescriptors, const AddressMapper &, bool isVanilla) {
    if (isVanilla) {
        stream.skipRawData(0x4);
    }
    for (auto &mapDescriptor: mapDescriptors) {
        stream >> mapDescriptor.bgmId;
        if (isVanilla) {
            // in vanilla main.dol the table has other stuff in it like bgm id, map frb files, etc.
            // this we need to skip to go the next target amount in the table
            stream.skipRawData(0x38 - 0x04);
        }
    }
}

quint32 BGMIDTable::readTableAddr(QDataStream &stream, const AddressMapper &addressMapper, bool) {
    stream.device()->seek(addressMapper.boomToFileAddress(0x801cca54));
    quint32 lisOpcode, dummy, addiOpcode;
    stream >> lisOpcode >> dummy >> addiOpcode;
    return PowerPcAsm::make32bitValueFromPair(lisOpcode, addiOpcode);
}

qint16 BGMIDTable::readTableRowCount(QDataStream &, const AddressMapper &, bool) {
    return -1;
}

bool BGMIDTable::readIsVanilla(QDataStream &stream, const AddressMapper &addressMapper) {
    stream.device()->seek(addressMapper.boomToFileAddress(0x801cca50));
    quint32 opcode; stream >> opcode;
    // mulli r0,r3,0x38
    return opcode == PowerPcAsm::mulli(0, 3, 0x38);
}
