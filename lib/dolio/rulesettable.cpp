#include "rulesettable.h"
#include "lib/powerpcasm.h"

quint32 RuleSetTable::writeTable(const QVector<MapDescriptor> &descriptors) {
    QVector<quint32> table;
    for (auto &descriptor: descriptors) table.append(descriptor.ruleSet);
    return allocate(table, "RuleSetTable");
}

void RuleSetTable::writeAsm(QDataStream &stream, const AddressMapper &addressMapper, const QVector<MapDescriptor> &mapDescriptors) {
    quint32 tableAddr = writeTable(mapDescriptors);
    PowerPcAsm::Pair16Bit v = PowerPcAsm::make16bitValuePair(tableAddr);

    // --- Update Table Addr ---
    stream.device()->seek(addressMapper.boomToFileAddress(0x801cca98));
    // mulli r0,r3,0x38 ->  mulli r0,r3,0x04
    stream << PowerPcAsm::mulli(0, 3, 0x04);
    // r3 <- 0x80428e50 ->  r3 <- tableAddr
    stream << PowerPcAsm::lis(3, v.upper) << PowerPcAsm::addi(3, 3, v.lower);
    stream.skipRawData(0x4);
    // lwz r3,0x10(r3)  ->  lwz r3,0x0(r3)
    stream << PowerPcAsm::lwz(3, 0, 3);

    // --- ASM hack: Use the rule set from map instead of from global setting ---
    auto ruleSetFromMapRoutine = allocate(writeRuleSetFromMapRoutine(addressMapper, 0), "writeRuleSetFromMapRoutine");
    stream.device()->seek(addressMapper.toFileAddress(ruleSetFromMapRoutine));
    auto insts = writeRuleSetFromMapRoutine(addressMapper, ruleSetFromMapRoutine); // re-write the routine again since now we know where it is located in the main dol
    for (quint32 inst: qAsConst(insts)) stream << inst;

    quint32 virtualPos = addressMapper.boomStreetToStandard(0x8007e13c);
    stream.device()->seek(addressMapper.toFileAddress(virtualPos));
    // lha r3,0x3c(r30)  -> bl ruleSetFromMapRoutine
    stream << PowerPcAsm::bl(virtualPos, ruleSetFromMapRoutine);
    // cmpwi r23,0x0     -> lha r3,0x3c(r30)
    stream << PowerPcAsm::lha(3, 0x3c, 30);
    // lha r0,0x28(r30)  -> cmpwi r23,0x0
    stream << PowerPcAsm::cmpwi(23, 0x0);
    // stw r25,0x53f4(r29) -> lha r0,0x28(r30)
    stream << PowerPcAsm::lha(0, 0x28, 30);
}

void RuleSetTable::readAsm(QDataStream &stream, QVector<MapDescriptor> &mapDescriptors, const AddressMapper &, bool isVanilla) {
    if (isVanilla) {
        stream.skipRawData(0x10);
    }
    for (auto &mapDescriptor: mapDescriptors) {
        stream >> mapDescriptor.ruleSet;
        if (isVanilla) {
            // in vanilla main.dol the table has other stuff in it like bgm id, map frb files, etc.
            // this we need to skip to go the next target amount in the table
            stream.skipRawData(0x38 - 0x04);
        }
    }
}

quint32 RuleSetTable::readTableAddr(QDataStream &stream, const AddressMapper &addressMapper, bool) {
    stream.device()->seek(addressMapper.boomToFileAddress(0x801cca9c));
    quint32 lisOpcode, addiOpcode;
    stream >> lisOpcode >> addiOpcode;
    return PowerPcAsm::make32bitValueFromPair(lisOpcode, addiOpcode);
}

qint16 RuleSetTable::readTableRowCount(QDataStream &, const AddressMapper &, bool) {
    return -1;
}

bool RuleSetTable::readIsVanilla(QDataStream &stream, const AddressMapper &addressMapper) {
    stream.device()->seek(addressMapper.boomToFileAddress(0x801cca98));
    quint32 opcode; stream >> opcode;
    // mulli r0,r3,0x38
    return opcode == PowerPcAsm::mulli(0, 3, 0x38);
}

QVector<quint32> RuleSetTable::writeRuleSetFromMapRoutine(const AddressMapper &addressMapper, quint32 routineStartAddress) {
    quint32 Game_GetRuleFlag = addressMapper.boomStreetToStandard(0x801cca98);

    // precondition: r24 is mapId
    // precondition: r25 is global rule set which we are gonna use to store the linkreturn
    return {
        PowerPcAsm::mflr(25),
        PowerPcAsm::mr(3, 24),                                             // r3 <- r24
        PowerPcAsm::bl(routineStartAddress, 2 /*asm.Count*/, Game_GetRuleFlag),  // r3 <- bl Game_GetRuleFlag(r3)
        PowerPcAsm::stw(3, 0x53f4, 29),                                    // gameRule <- r3
        PowerPcAsm::mtlr(25),
        PowerPcAsm::blr()                                                 // return
    };
}
