#include "tourbankruptcylimittable.h"
#include "lib/powerpcasm.h"

quint32 TourBankruptcyLimitTable::writeTable(const QVector<MapDescriptor> &descriptors) {
    QVector<quint32> table;
    for (auto &descriptor: descriptors) table.append(descriptor.tourBankruptcyLimit);
    return allocate(table, "TourBankruptcyLimitTable");
}

void TourBankruptcyLimitTable::writeAsm(QDataStream &stream, const AddressMapper &addressMapper, const QVector<MapDescriptor> &mapDescriptors) {
    int tableRowCount = mapDescriptors.size();
    quint32 tableAddr = writeTable(mapDescriptors);
    PowerPcAsm::Pair16Bit v = PowerPcAsm::make16bitValuePair(tableAddr);

    // subi r30,r30,0x15                                  ->  nop
    stream.device()->seek(addressMapper.boomToFileAddress(0x8020d2b0)); stream << PowerPcAsm::nop();
    // cmpwi r30,0x12                                     ->  cmpwi r30,tableElementsCount
    stream.device()->seek(addressMapper.boomToFileAddress(0x8020d2bc)); stream << PowerPcAsm::cmpwi(30, (short)(tableRowCount));
    // li r0,0x15                                         ->  nop
    stream.device()->seek(addressMapper.boomToFileAddress(0x8020d334)); stream << PowerPcAsm::nop();
    // mulli r5,r0,0x24                                   ->  mulli r5,r0,0x04
    stream.device()->seek(addressMapper.boomToFileAddress(0x8020d340)); stream << PowerPcAsm::mulli(5, 0, 0x04);
    // r4 <- 804363c8                                     ->  r4 <- tableAddr
    stream.device()->seek(addressMapper.boomToFileAddress(0x8020d344)); stream << PowerPcAsm::lis(4, v.upper);
    stream.skipRawData(0x4); stream << PowerPcAsm::addi(4, 4, v.lower);
    // mulli r0,r30,0x24                                  ->  mulli r0,r30,0x04
    stream.device()->seek(addressMapper.boomToFileAddress(0x8020d350)); stream << PowerPcAsm::mulli(0, 30, 0x04);
    // lwz r0,0x4(r4)                                     ->  lwz r0,0x4(r4)
    stream.device()->seek(addressMapper.boomToFileAddress(0x8020d35c)); stream << PowerPcAsm::lwz(0, 0x0, 4);

    // subi r31,r3,0x15                                   ->  nop
    stream.device()->seek(addressMapper.boomToFileAddress(0x80211f90)); stream << PowerPcAsm::nop();
    // cmpwi r31,0x12                                     ->  cmpwi r31,tableElementsCount
    stream.device()->seek(addressMapper.boomToFileAddress(0x80211f9c)); stream << PowerPcAsm::cmpwi(31, (short)(tableRowCount));
    // li r3,0x15                                         ->  nop
    stream.device()->seek(addressMapper.boomToFileAddress(0x80212014)); stream << PowerPcAsm::nop();
    // mulli r4,r3,0x24                                   ->  mulli r4,r3,0x04
    stream.device()->seek(addressMapper.boomToFileAddress(0x80212020)); stream << PowerPcAsm::mulli(4, 3, 0x04);
    // r3 <- 804363c8                                     ->  r3 <- tableAddr
    stream.device()->seek(addressMapper.boomToFileAddress(0x80212024)); stream << PowerPcAsm::lis(3, v.upper) << PowerPcAsm::addi(3, 3, v.lower);
    // mulli r0,r31,0x24                                  ->  mulli r0,r31,0x04
    stream.device()->seek(addressMapper.boomToFileAddress(0x8021202c)); stream << PowerPcAsm::mulli(0, 31, 0x04);
    // lwz r3,0x4(r3)                                     ->  lwz r3,0x0(r3)
    stream.device()->seek(addressMapper.boomToFileAddress(0x80212040)); stream << PowerPcAsm::lwz(3, 0x0, 3);
}

void TourBankruptcyLimitTable::readAsm(QDataStream &stream, QVector<MapDescriptor> &mapDescriptors, const AddressMapper &, bool isVanilla) {
    if (isVanilla) {
        stream.skipRawData(0x4);
    }
    for (auto &mapDescriptor: mapDescriptors) {
        stream >> mapDescriptor.tourBankruptcyLimit;
        if (isVanilla) {
            // in vanilla main.dol the table has other stuff in it like bgm id, map frb files, etc.
            // this we need to skip to go the next target amount in the table
            stream.skipRawData(0x24 - 0x04);
        }
    }
}

quint32 TourBankruptcyLimitTable::readTableAddr(QDataStream &stream, const AddressMapper &addressMapper, bool) {
    stream.device()->seek(addressMapper.boomToFileAddress(0x80212024));
    quint32 lisOpcode, addiOpcode;
    stream >> lisOpcode >> addiOpcode;
    return PowerPcAsm::make32bitValueFromPair(lisOpcode, addiOpcode);
}

qint16 TourBankruptcyLimitTable::readTableRowCount(QDataStream &stream, const AddressMapper &addressMapper, bool) {
    quint32 opcode;
    stream.device()->seek(addressMapper.boomToFileAddress(0x8020d2bc));
    stream >> opcode;
    return PowerPcAsm::getOpcodeParameter(opcode);
}

bool TourBankruptcyLimitTable::readIsVanilla(QDataStream &stream, const AddressMapper &addressMapper) {
    stream.device()->seek(addressMapper.boomToFileAddress(0x8020d334));
    quint32 opcode; stream >> opcode;
    // li r0,0x15
    return opcode == PowerPcAsm::li(0, 0x15);
}

