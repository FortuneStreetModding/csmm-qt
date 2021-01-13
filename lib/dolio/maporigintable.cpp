#include "maporigintable.h"
#include "lib/powerpcasm.h"

quint32 MapOriginTable::writeTable(const QVector<MapDescriptor> &descriptors) {
    QVector<quint32> table;
    for (auto &descriptor: descriptors) table.append(descriptor.unlockId);
    return allocate(table);
}

void MapOriginTable::writeAsm(QDataStream &stream, const AddressMapper &addressMapper, const QVector<MapDescriptor> &mapDescriptors) {
    quint32 tableAddr = writeTable(mapDescriptors);
    PowerPcAsm::Pair16Bit v = PowerPcAsm::make16bitValuePair(tableAddr);

    // --- Update Table Addr ---
    // mulli r0,r3,0x38 ->  mulli r0,r3,0x04
    stream.device()->seek(addressMapper.boomToFileAddress(0x801ccb58)); stream << PowerPcAsm::mulli(0, 3, 0x04);
    // r3 <- 0x80428e50 ->  r3 <- tableAddr
    stream.device()->seek(addressMapper.boomToFileAddress(0x801ccb5c)); stream << PowerPcAsm::lis(3, v.upper) << PowerPcAsm::addi(3, 3, v.lower);
    // lwz r3,0x4(r3)   ->  lwz r3,0x0(r3)
    stream.device()->seek(addressMapper.boomToFileAddress(0x801ccb68)); stream << PowerPcAsm::lwz(3, 0x0, 3);
}

void MapOriginTable::readAsm(QDataStream &stream, QVector<MapDescriptor> &mapDescriptors, const AddressMapper &, bool isVanilla) {
    if (isVanilla) {
        stream.skipRawData(0x30);
    }
    for (auto &mapDescriptor: mapDescriptors) {
        stream >> mapDescriptor.unlockId;
        if (isVanilla) {
            // in vanilla main.dol the table has other stuff in it like bgm id, map frb files, etc.
            // this we need to skip to go the next target amount in the table
            stream.skipRawData(0x38 - 0x04);
        }
    }
}

quint32 MapOriginTable::readTableAddr(QDataStream &stream, const AddressMapper &addressMapper, bool) {
    stream.device()->seek(addressMapper.boomToFileAddress(0x801ccb5c));
    quint32 lisOpcode, addiOpcode;
    stream >> lisOpcode >> addiOpcode;
    return PowerPcAsm::make32bitValueFromPair(lisOpcode, addiOpcode);
}

qint16 MapOriginTable::readTableRowCount(QDataStream &, const AddressMapper &, bool) {
    return -1;
}

bool MapOriginTable::readIsVanilla(QDataStream &stream, const AddressMapper &addressMapper) {
    stream.device()->seek(addressMapper.boomToFileAddress(0x801ccb58));
    quint32 opcode; stream >> opcode;
    // mulli r0,r3,0x38
    return opcode == PowerPcAsm::mulli(0, 3, 0x38);
}

