#include "tourclearranktable.h"
#include "lib/powerpcasm.h"

quint32 TourClearRankTable::writeTable(const std::vector<MapDescriptor> &descriptors) {
    QVector<quint32> table;
    for (auto &descriptor: descriptors) table.append(descriptor.tourClearRank);
    return allocate(table, "TourClearRankTable");
}

void TourClearRankTable::writeAsm(QDataStream &stream, const AddressMapper &addressMapper, const std::vector<MapDescriptor> &mapDescriptors) {
    quint32 tableAddr = writeTable(mapDescriptors);
    PowerPcAsm::Pair16Bit v = PowerPcAsm::make16bitValuePair(tableAddr);

    // mulli r0,r0,0x24                                   ->  mulli r0,r0,0x04
    stream.device()->seek(addressMapper.boomToFileAddress(0x802105a4)); stream << PowerPcAsm::mulli(0, 0, 0x04);
    // r3 <- 804363c8                                     ->  r3 <- tableAddr
    stream.device()->seek(addressMapper.boomToFileAddress(0x802105a8)); stream << PowerPcAsm::lis(3, v.upper);
    stream.skipRawData(0x4); stream << PowerPcAsm::addi(3, 3, v.lower);
    // mulli r0,r29,0x24                                  ->  mulli r0,r29,0x04
    stream.device()->seek(addressMapper.boomToFileAddress(0x802105c4)); stream << PowerPcAsm::mulli(0, 29, 0x04);
    // lwz r4,0x18(r3)                                    ->  lwz r4,0x0(r3)
    stream.device()->seek(addressMapper.boomToFileAddress(0x802105d8)); stream << PowerPcAsm::lwz(4, 0x0, 3);

    // mulli r4,r0,0x24                                   ->  mulli r4,r0,0x04
    stream.device()->seek(addressMapper.boomToFileAddress(0x8021210c)); stream << PowerPcAsm::mulli(4, 0, 0x04);
    // r3 <- 804363c8                                     ->  r3 <- tableAddr
    stream.device()->seek(addressMapper.boomToFileAddress(0x80212110)); stream << PowerPcAsm::lis(3, v.upper) << PowerPcAsm::addi(3, 3, v.lower);
    // mulli r0,r31,0x24                                  ->  mulli r0,r31,0x04
    stream.device()->seek(addressMapper.boomToFileAddress(0x80212118)); stream << PowerPcAsm::mulli(0, 31, 0x04);
    // lwz r3,0x18(r3)                                    ->  lwz r3,0x0(r3)
    stream.device()->seek(addressMapper.boomToFileAddress(0x8021212c)); stream << PowerPcAsm::lwz(3, 0x0, 3);
}

void TourClearRankTable::readAsm(QDataStream &stream, std::vector<MapDescriptor> &mapDescriptors, const AddressMapper &, bool isVanilla) {
    if (isVanilla) {
        stream.skipRawData(0x18);
    }
    for (auto &mapDescriptor: mapDescriptors) {
        stream >> mapDescriptor.tourClearRank;
        if (isVanilla) {
            // in vanilla main.dol the table has other stuff in it like bgm id, map frb files, etc.
            // this we need to skip to go the next target amount in the table
            stream.skipRawData(0x24 - 0x04);
        }
    }
}

quint32 TourClearRankTable::readTableAddr(QDataStream &stream, const AddressMapper &addressMapper, bool) {
    stream.device()->seek(addressMapper.boomToFileAddress(0x80212110));
    quint32 lisOpcode, addiOpcode;
    stream >> lisOpcode >> addiOpcode;
    return PowerPcAsm::make32bitValueFromPair(lisOpcode, addiOpcode);
}

bool TourClearRankTable::readIsVanilla(QDataStream &stream, const AddressMapper &addressMapper) {
    stream.device()->seek(addressMapper.boomToFileAddress(0x8021210c));
    quint32 opcode; stream >> opcode;
    // mulli r4,r0,0x24
    return opcode == PowerPcAsm::mulli(4, 0, 0x24);
}

