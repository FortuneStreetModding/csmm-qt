#include "defaulttargetamounttable.h"
#include "lib/powerpcasm.h"

quint32 DefaultTargetAmountTable::writeTable(const QVector<MapDescriptor> &descriptors) {
    QVector<quint32> table;
    for (auto &descriptor: descriptors) table.append(descriptor.targetAmount);
    return allocate(table, "DefaultGoalMoneyTable");
}

void DefaultTargetAmountTable::writeAsm(QDataStream &stream, const AddressMapper &addressMapper, const QVector<MapDescriptor> &mapDescriptors) {
    int tableRowCount = mapDescriptors.size();
    quint32 tableAddr = writeTable(mapDescriptors);
    PowerPcAsm::Pair16Bit v = PowerPcAsm::make16bitValuePair((quint32)tableAddr);

    // subi r30,r30,0x15                                  ->  nop
    stream.device()->seek(addressMapper.boomToFileAddress(0x8020d0dc)); stream << PowerPcAsm::nop();
    // cmpwi r30,0x12                                     ->  cmpwi r30,tableElementsCount
    stream.device()->seek(addressMapper.boomToFileAddress(0x8020d0e8)); stream << PowerPcAsm::cmpwi(30, tableRowCount);
    // li r0,0x15                                         ->  nop
    stream.device()->seek(addressMapper.boomToFileAddress(0x8020d160)); stream << PowerPcAsm::nop();
    // mulli r0,r0,0x24                                   ->  mulli r0,r0,0x04
    stream.device()->seek(addressMapper.boomToFileAddress(0x8020d16c)); stream << PowerPcAsm::mulli(0, 0, 0x04);
    // r4 <- 804363c8                                     ->  r4 <- tableAddr
    stream.device()->seek(addressMapper.boomToFileAddress(0x8020d170)); stream << PowerPcAsm::lis(4, v.upper) << PowerPcAsm::addi(4, 4, v.lower);
    // mulli r3,r30,0x24                                  ->  mulli r3,r30,0x04
    stream.device()->seek(addressMapper.boomToFileAddress(0x8020d178)); stream << PowerPcAsm::mulli(3, 30, 0x04);

    // subi r31,r3,0x15                                   ->  nop
    stream.device()->seek(addressMapper.boomToFileAddress(0x80211c04)); stream << PowerPcAsm::nop();
    // cmpwi r31,0x12                                     ->  cmpwi r31,tableElementsCount
    stream.device()->seek(addressMapper.boomToFileAddress(0x80211c10)); stream << PowerPcAsm::cmpwi(31, tableRowCount);
    // li r3,0x15                                         ->  nop
    stream.device()->seek(addressMapper.boomToFileAddress(0x80211c88)); stream << PowerPcAsm::nop();
    // mulli r0,r3,0x24                                   ->  mulli r0,r3,0x04
    stream.device()->seek(addressMapper.boomToFileAddress(0x80211c94)); stream << PowerPcAsm::mulli(0, 3, 0x04);
    // r4 <- 804363c8                                     ->  r4 <- tableAddr
    stream.device()->seek(addressMapper.boomToFileAddress(0x80211c98)); stream << PowerPcAsm::lis(4, v.upper) << PowerPcAsm::addi(4, 4, v.lower);
    // mulli r3,r31,0x24                                  ->  mulli r3,r31,0x04
    stream.device()->seek(addressMapper.boomToFileAddress(0x80211ca0)); stream << PowerPcAsm::mulli(3, 31, 0x04);
}

void DefaultTargetAmountTable::readAsm(QDataStream &stream, QVector<MapDescriptor> &mapDescriptors, const AddressMapper &, bool isVanilla) {
    if (isVanilla) {
        stream.skipRawData(0x0);
    }
    for (auto &mapDescriptor: mapDescriptors) {
        stream >> mapDescriptor.targetAmount;
        if (isVanilla) {
            // in vanilla main.dol the table has other stuff in it like initial cash and tour opponents of the map
            // this we need to skip to go the next target amount in the table
            stream.skipRawData(0x24 - 0x4);
        }
    }
}

quint32 DefaultTargetAmountTable::readTableAddr(QDataStream &stream, const AddressMapper &addressMapper, bool) {
    stream.device()->seek(addressMapper.boomToFileAddress(0x8020d170));
    quint32 lisOpcode, addiOpcode;
    stream >> lisOpcode >> addiOpcode;
    return PowerPcAsm::make32bitValueFromPair(lisOpcode, addiOpcode);
}

qint16 DefaultTargetAmountTable::readTableRowCount(QDataStream &stream, const AddressMapper &addressMapper, bool) {
    quint32 opcode;
    stream.device()->seek(addressMapper.boomToFileAddress(0x8020d0e8)); stream >> opcode;
    return PowerPcAsm::getOpcodeParameter(opcode);
}

bool DefaultTargetAmountTable::readIsVanilla(QDataStream &stream, const AddressMapper &addressMapper) {
    stream.device()->seek(addressMapper.boomToFileAddress(0x8020d160));
    quint32 opcode; stream >> opcode;
    return opcode == PowerPcAsm::li(0, 0x15);
}
