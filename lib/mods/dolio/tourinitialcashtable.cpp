#include "tourinitialcashtable.h"
#include "lib/powerpcasm.h"

quint32 TourInitialCashTable::writeTable(const std::vector<MapDescriptor> &descriptors) {
    QVector<quint32> table;
    for (auto &descriptor: descriptors) table.append(descriptor.tourInitialCash);
    return allocate(table, "TourInitialCashTable");
}

void TourInitialCashTable::writeAsm(QDataStream &stream, const AddressMapper &addressMapper, const std::vector<MapDescriptor> &mapDescriptors) {
    int tableRowCount = mapDescriptors.size();
    quint32 tableAddr = writeTable(mapDescriptors);
    PowerPcAsm::Pair16Bit v = PowerPcAsm::make16bitValuePair(tableAddr);

    // subi r30,r30,0x15                                  ->  nop
    stream.device()->seek(addressMapper.boomToFileAddress(0x8020d1c4)); stream << PowerPcAsm::nop();
    // cmpwi r30,0x12                                     ->  cmpwi r30,tableElementsCount
    stream.device()->seek(addressMapper.boomToFileAddress(0x8020d1d0)); stream << PowerPcAsm::cmpwi(30, (short)(tableRowCount));
    // li r0,0x15                                         ->  nop
    stream.device()->seek(addressMapper.boomToFileAddress(0x8020d248)); stream << PowerPcAsm::nop();
    // mulli r4,r0,0x24                                   ->  mulli r4,r0,0x04
    stream.device()->seek(addressMapper.boomToFileAddress(0x8020d254)); stream << PowerPcAsm::mulli(4, 0, 0x04);
    // r3 <- 804363c8                                     ->  r3 <- tableAddr
    stream.device()->seek(addressMapper.boomToFileAddress(0x8020d258)); stream << PowerPcAsm::lis(3, v.upper) << PowerPcAsm::addi(3, 3, v.lower);
    // mulli r0,r30,0x24                                  ->  mulli r0,r30,0x04
    stream.device()->seek(addressMapper.boomToFileAddress(0x8020d260)); stream << PowerPcAsm::mulli(0, 30, 0x04);
    // lwz r0,0x8(r3)                                     ->  lwz r0,0x0(r3)
    stream.device()->seek(addressMapper.boomToFileAddress(0x8020d26c)); stream << PowerPcAsm::lwz(0, 0x0, 3);

    // subi r31,r3,0x15                                   ->  nop
    stream.device()->seek(addressMapper.boomToFileAddress(0x80211ce4)); stream << PowerPcAsm::nop();
    // cmpwi r31,0x12                                     ->  cmpwi r31,tableElementsCount
    stream.device()->seek(addressMapper.boomToFileAddress(0x80211cf0)); stream << PowerPcAsm::cmpwi(31, (short)(tableRowCount));
    // li r3,0x15                                         ->  nop
    stream.device()->seek(addressMapper.boomToFileAddress(0x80211d68)); stream << PowerPcAsm::nop();
    // mulli r4,r3,0x24                                   ->  mulli r4,r3,0x04
    stream.device()->seek(addressMapper.boomToFileAddress(0x80211d74)); stream << PowerPcAsm::mulli(4, 3, 0x04);
    // r3 <- 804363c8                                     ->  r3 <- tableAddr
    stream.device()->seek(addressMapper.boomToFileAddress(0x80211d78)); stream << PowerPcAsm::lis(3, v.upper) << PowerPcAsm::addi(3, 3, v.lower);
    // mulli r0,r31,0x24                                  ->  mulli r0,r31,0x04
    stream.device()->seek(addressMapper.boomToFileAddress(0x80211d80)); stream << PowerPcAsm::mulli(0, 31, 0x04);
    // lwz r3,0x8(r3)                                     ->  lwz r3,0x0(r3)
    stream.device()->seek(addressMapper.boomToFileAddress(0x80211d94)); stream << PowerPcAsm::lwz(3, 0x0, 3);
}

void TourInitialCashTable::readAsm(QDataStream &stream, std::vector<MapDescriptor> &mapDescriptors, const AddressMapper &, bool isVanilla) {
    if (isVanilla) {
        stream.skipRawData(0x8);
    }
    for (auto &mapDescriptor: mapDescriptors) {
        stream >> mapDescriptor.tourInitialCash;
        if (isVanilla) {
            // in vanilla main.dol the table has other stuff in it like bgm id, map frb files, etc.
            // this we need to skip to go the next target amount in the table
            stream.skipRawData(0x24 - 0x04);
        }
    }
}

quint32 TourInitialCashTable::readTableAddr(QDataStream &stream, const AddressMapper &addressMapper, bool) {
    stream.device()->seek(addressMapper.boomToFileAddress(0x80211d78));
    quint32 lisOpcode, addiOpcode;
    stream >> lisOpcode >> addiOpcode;
    return PowerPcAsm::make32bitValueFromPair(lisOpcode, addiOpcode);
}

qint16 TourInitialCashTable::readTableRowCount(QDataStream &stream, const AddressMapper &addressMapper, bool) {
    stream.device()->seek(addressMapper.boomToFileAddress(0x80211cf0));
    quint32 opcode; stream >> opcode;
    return PowerPcAsm::getOpcodeParameter(opcode);
}

bool TourInitialCashTable::readIsVanilla(QDataStream &stream, const AddressMapper &addressMapper) {
    stream.device()->seek(addressMapper.boomToFileAddress(0x8020d248));
    quint32 opcode; stream >> opcode;
    // li r0,0x15
    return opcode == PowerPcAsm::li(0, 0x15);
}

