#include "touropponentstable.h"
#include "lib/powerpcasm.h"

quint32 TourOpponentsTable::writeTable(const std::vector<MapDescriptor> &descriptors) {
    QVector<quint32> table;
    for (auto &descriptor: descriptors) {
        for (Character character: descriptor.tourCharacters) {
            table.append(character);
        }
    }
    return allocate(table, "TourOpponentsTable");
}

void TourOpponentsTable::writeAsm(QDataStream &stream, const AddressMapper &addressMapper, const std::vector<MapDescriptor> &mapDescriptors) {
    int tableRowCount = mapDescriptors.size();
    quint32 tableAddr = writeTable(mapDescriptors);
    PowerPcAsm::Pair16Bit v = PowerPcAsm::make16bitValuePair(tableAddr);

    // subi r3,r3,0x15                                    ->  nop
    stream.device()->seek(addressMapper.boomToFileAddress(0x8020cec8)); stream << PowerPcAsm::nop();
    // cmpwi r3,0x12                                      ->  cmpwi r3,tableElementsCount
    stream.device()->seek(addressMapper.boomToFileAddress(0x8020ced4)); stream << PowerPcAsm::cmpwi(3, (short)(tableRowCount));
    // li r0,0x15                                         ->  nop
    stream.device()->seek(addressMapper.boomToFileAddress(0x8020cf68)); stream << PowerPcAsm::nop();
    // mulli r0,r3,0x24                                   ->  mulli r0,r3,0x0c
    stream.device()->seek(addressMapper.boomToFileAddress(0x8020cee0)); stream << PowerPcAsm::mulli(0, 3, 0x0c);
    // r3 <- 804363c8                                     ->  r3 <- tableAddr
    stream.device()->seek(addressMapper.boomToFileAddress(0x8020cee4)); stream << PowerPcAsm::lis(3, v.upper);
    stream.skipRawData(0x4); stream << PowerPcAsm::addi(3, 3, v.lower);
    // mulli r3,r0,0x24                                  ->  mulli r3,r0,0x0c
    stream.device()->seek(addressMapper.boomToFileAddress(0x8020cf74)); stream << PowerPcAsm::mulli(3, 0, 0x0c);
    // lwz r0,0xc(r3)                                     ->  lwz r0,0x0(r3)
    stream.device()->seek(addressMapper.boomToFileAddress(0x8020cf8c)); stream << PowerPcAsm::lwz(0, 0x0, 3);

    // subi r4,r4,0x15                                   ->  nop
    stream.device()->seek(addressMapper.boomToFileAddress(0x80211af4)); stream << PowerPcAsm::nop();
    // cmpwi r4,0x12                                     ->  cmpwi r4,tableElementsCount
    stream.device()->seek(addressMapper.boomToFileAddress(0x80211b00)); stream << PowerPcAsm::cmpwi(4, (short)(tableRowCount));
    // li r3,0x15                                         ->  nop
    stream.device()->seek(addressMapper.boomToFileAddress(0x80211b94)); stream << PowerPcAsm::nop();
    // mulli r0,r4,0x24                                   ->  mulli r0,r4,0x0c
    stream.device()->seek(addressMapper.boomToFileAddress(0x80211b0c)); stream << PowerPcAsm::mulli(0, 4, 0x0c);
    // r4 <- 804363c8                                     ->  r4 <- tableAddr
    stream.device()->seek(addressMapper.boomToFileAddress(0x80211b10)); stream << PowerPcAsm::lis(4, v.upper);
    stream.skipRawData(0x4); stream << PowerPcAsm::addi(4, 4, v.lower);
    // mulli r3,r3,0x24                                  ->  mulli r3,r3,0x0c
    stream.device()->seek(addressMapper.boomToFileAddress(0x80211ba0)); stream << PowerPcAsm::mulli(3, 3, 0x0c);
    // lwz r0,0xc(r3)                                     ->  lwz r0,0x0(r3)
    stream.device()->seek(addressMapper.boomToFileAddress(0x80211bb8)); stream << PowerPcAsm::lwz(0, 0x0, 3);
}

void TourOpponentsTable::readAsm(QDataStream &stream, std::vector<MapDescriptor> &mapDescriptors, const AddressMapper &, bool isVanilla) {
    if (isVanilla) {
        stream.skipRawData(0xC);
    }
    for (auto &mapDescriptor: mapDescriptors) {
        for (auto &character: mapDescriptor.tourCharacters) {
            stream >> character;
        }
        if (isVanilla) {
            // in vanilla main.dol the table has other stuff in it like bgm id, map frb files, etc.
            // this we need to skip to go the next target amount in the table
            stream.skipRawData(0x24 - 0xC);
        }
    }
}

quint32 TourOpponentsTable::readTableAddr(QDataStream &stream, const AddressMapper &addressMapper, bool) {
    stream.device()->seek(addressMapper.boomToFileAddress(0x80211b10));
    quint32 lisOpcode, dummy, addiOpcode;
    stream >> lisOpcode >> dummy >> addiOpcode;
    return PowerPcAsm::make32bitValueFromPair(lisOpcode, addiOpcode);
}

qint16 TourOpponentsTable::readTableRowCount(QDataStream &stream, const AddressMapper &addressMapper, bool) {
    stream.device()->seek(addressMapper.boomToFileAddress(0x80211b00));
    quint32 opcode; stream >> opcode;
    return PowerPcAsm::getOpcodeParameter(opcode);
}

bool TourOpponentsTable::readIsVanilla(QDataStream &stream, const AddressMapper &addressMapper) {
    stream.device()->seek(addressMapper.boomToFileAddress(0x8020cf68));
    quint32 opcode; stream >> opcode;
    // li r0,0x15
    return opcode == PowerPcAsm::li(0, 0x15);
}
