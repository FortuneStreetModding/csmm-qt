#include "frbmaptable.h"
#include "lib/powerpcasm.h"

quint32 FrbMapTable::writeTable(const std::vector<MapDescriptor> &descriptors) {
    QVector<quint32> table;
    for (auto &descriptor: descriptors) {
        QVector<quint32> subTable;
        subTable.append(descriptor.frbFiles.size());
        for (auto &frbFile: descriptor.frbFiles) {
            subTable.append(allocate(frbFile));
        }
        table.append(allocate(subTable, "FrbMapSubTable"));
    }
    return allocate(table, "FrbMapTable");
}

void FrbMapTable::writeAsm(QDataStream &stream, const AddressMapper &addressMapper, const std::vector<MapDescriptor> &mapDescriptors) {
    quint32 tableAddr = writeTable(mapDescriptors);
    PowerPcAsm::Pair16Bit v = PowerPcAsm::make16bitValuePair(tableAddr);

    // --- Game::GetMapFrbName ---
    stream.device()->seek(addressMapper.boomToFileAddress(0x801ccab0));
    // [0x801ccab0] r3 *= 4
    stream << PowerPcAsm::mulli(3, 3, 0x4);
    // [0x801ccab4, 0x801ccab8] r5 <- tableAddr
    stream << PowerPcAsm::lis(5, v.upper);
    stream << PowerPcAsm::addi(5, 5, v.lower);
    // [0x801ccabc] r5 <- r5[r3]
    stream << PowerPcAsm::lwzx(5, 5, 3);
    // [0x801ccac0] r4 *= 4
    stream << PowerPcAsm::mulli(4, 4, 0x4);
    // [0x801ccac4] r5 += 4
    stream << PowerPcAsm::addi(5, 5, 0x4);
    // [0x801ccac8] r3 <- r5[r4]
    stream << PowerPcAsm::lwzx(3, 5, 4);
    // [0x801ccacc] return
    stream << PowerPcAsm::blr();

    // --- Game::GetMapMapNum ---
    stream.device()->seek(addressMapper.boomToFileAddress(0x801ccad0));
    // [0x801ccad0] r3 <- r3 * 4
    stream << PowerPcAsm::mulli(3, 3, 0x4);
    // [0x801ccad4, 0x801ccad8] r4 <- tableAddr
    stream << PowerPcAsm::lis(4, v.upper);
    stream << PowerPcAsm::addi(4, 4, v.lower);
    // [0x801ccadc] r4 <- r4[r3]
    stream << PowerPcAsm::lwzx(4, 4, 3);
    // [0x801ccae0] r3 <- r4[0]
    stream << PowerPcAsm::lwz(3, 0x0, 4);
    // [0x801ccae4] return
    stream << PowerPcAsm::blr();
}

void FrbMapTable::readAsm(QDataStream &stream, std::vector<MapDescriptor> &mapDescriptors, const AddressMapper &addressMapper, bool isVanilla) {
    if (isVanilla) {
        stream.skipRawData(0x18);
    }
    for (auto &mapDescriptor: mapDescriptors) {
        mapDescriptor.frbFiles.clear();

        if (isVanilla) {
            for (int i=0; i<4; ++i) {
                quint32 addr;
                stream >> addr;
                auto frbFile = resolveAddressToString(addr, stream, addressMapper);
                if (!frbFile.isEmpty()) mapDescriptor.frbFiles.push_back(resolveAddressToString(addr, stream, addressMapper));
            }

            // in vanilla main.dol the table has other stuff in it like bgm id, map frb files, etc.
            // this we need to skip to go the next target amount in the table
            stream.skipRawData(0x38 - 0x10);
        } else {
            quint32 subTableAddr;
            stream >> subTableAddr;
            auto posToReturnTo = stream.device()->pos();
            stream.device()->seek(addressMapper.toFileAddress(subTableAddr));
            quint32 numFrbFiles;
            stream >> numFrbFiles;
            for (int i=0; i<numFrbFiles; ++i) {
                quint32 addr;
                stream >> addr;
                auto frbFile = resolveAddressToString(addr, stream, addressMapper);
                if (!frbFile.isEmpty()) mapDescriptor.frbFiles.push_back(resolveAddressToString(addr, stream, addressMapper));
            }
            stream.device()->seek(posToReturnTo);
        }
    }
}

quint32 FrbMapTable::readTableAddr(QDataStream &stream, const AddressMapper &addressMapper, bool isVanilla) {
    stream.device()->seek(addressMapper.boomToFileAddress(0x801ccab4));
    quint32 lisOpcode, addiOpcode;
    stream >> lisOpcode;
    if (isVanilla) {
        stream.skipRawData(4);
    }
    stream >> addiOpcode;
    return PowerPcAsm::make32bitValueFromPair(lisOpcode, addiOpcode);
}

bool FrbMapTable::readIsVanilla(QDataStream &stream, const AddressMapper &addressMapper) {
    stream.device()->seek(addressMapper.boomToFileAddress(0x801ccad0));
    quint32 opcode; stream >> opcode;
    // mulli r0,r3,0x38
    return opcode == PowerPcAsm::mulli(0, 3, 0x38);
}
