#include "frbmaptable.h"
#include "lib/powerpcasm.h"

quint32 FrbMapTable::writeTable(const std::vector<MapDescriptor> &descriptors) {
    QVector<quint32> table;
    for (auto &descriptor: descriptors) {
        for (auto &frbFile: descriptor.frbFiles) {
            table.append(allocate(frbFile));
        }
    }
    return allocate(table, "FrbMapTable");
}

void FrbMapTable::writeAsm(QDataStream &stream, const AddressMapper &addressMapper, const std::vector<MapDescriptor> &mapDescriptors) {
    quint32 tableAddr = writeTable(mapDescriptors);
    PowerPcAsm::Pair16Bit v = PowerPcAsm::make16bitValuePair(tableAddr);

    // --- Game::GetMapFrbName ---
    // mulli r3,r3,0x38  ->  mulli r3,r3,0x10
    stream.device()->seek(addressMapper.boomToFileAddress(0x801ccab0)); stream << PowerPcAsm::mulli(3, 3, 0x10);
    // r5 <- 0x80428e50  ->  r5 <- tableAddr
    stream.device()->seek(addressMapper.boomToFileAddress(0x801ccab4)); stream << PowerPcAsm::lis(5, v.upper);
    stream.skipRawData(0x4); stream << PowerPcAsm::addi(5, 5, v.lower);
    // lwz r3,0x18(r3)   ->  lwz r3,0x0(r3)
    stream.device()->seek(addressMapper.boomToFileAddress(0x801ccac8)); stream << PowerPcAsm::lwz(3, 0x0, 3);

    // --- Game::GetMapMapNum ---
    // mulli r0,r3,0x38  ->  mulli r0,r3,0x10
    stream.device()->seek(addressMapper.boomToFileAddress(0x801ccad0)); stream << PowerPcAsm::mulli(0, 3, 0x10);
    // r4 <- 0x80428e50  ->  r4 <- tableAddr
    stream.device()->seek(addressMapper.boomToFileAddress(0x801ccad4)); stream << PowerPcAsm::lis(4, v.upper);
    stream.skipRawData(0x4); stream << PowerPcAsm::addi(4, 4, v.lower);
    // lwz r0,0x18(r4)   ->  lwz r0,0x0(r4)
    stream.device()->seek(addressMapper.boomToFileAddress(0x801ccae4)); stream << PowerPcAsm::lwz(0, 0x0, 4);
    // lwz r0,0x1c(r4)   ->  lwz r0,0x4(r4)
    stream.device()->seek(addressMapper.boomToFileAddress(0x801ccaf0)); stream << PowerPcAsm::lwz(0, 0x4, 4);
    // lwz r0,0x20(r4)   ->  lwz r0,0x8(r4)
    stream.device()->seek(addressMapper.boomToFileAddress(0x801ccb00)); stream << PowerPcAsm::lwz(0, 0x8, 4);
    // lwz r0,0x24(r4)   ->  lwz r0,0xc(r4)
    stream.device()->seek(addressMapper.boomToFileAddress(0x801ccb10)); stream << PowerPcAsm::lwz(0, 0xc, 4);
}

void FrbMapTable::readAsm(QDataStream &stream, std::vector<MapDescriptor> &mapDescriptors, const AddressMapper &addressMapper, bool isVanilla) {
    if (isVanilla) {
        stream.skipRawData(0x18);
    }
    for (auto &mapDescriptor: mapDescriptors) {
        for (auto &frbFile: mapDescriptor.frbFiles) {
            quint32 addr;
            stream >> addr;
            frbFile = resolveAddressToString(addr, stream, addressMapper);
        }
        if (isVanilla) {
            // in vanilla main.dol the table has other stuff in it like bgm id, map frb files, etc.
            // this we need to skip to go the next target amount in the table
            stream.skipRawData(0x38 - 0x10);
        }
    }
}

quint32 FrbMapTable::readTableAddr(QDataStream &stream, const AddressMapper &addressMapper, bool) {
    stream.device()->seek(addressMapper.boomToFileAddress(0x801ccab4));
    quint32 lisOpcode, dummy, addiOpcode;
    stream >> lisOpcode >> dummy >> addiOpcode;
    return PowerPcAsm::make32bitValueFromPair(lisOpcode, addiOpcode);
}

bool FrbMapTable::readIsVanilla(QDataStream &stream, const AddressMapper &addressMapper) {
    stream.device()->seek(addressMapper.boomToFileAddress(0x801ccad0));
    quint32 opcode; stream >> opcode;
    // mulli r0,r3,0x38
    return opcode == PowerPcAsm::mulli(0, 3, 0x38);
}
