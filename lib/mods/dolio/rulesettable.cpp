#include "rulesettable.h"
#include "lib/powerpcasm.h"

quint32 RuleSetTable::writeTable(const std::vector<MapDescriptor> &descriptors) {
    QVector<quint32> table;
    for (auto &descriptor: descriptors) table.append(descriptor.ruleSet);
    return allocate(table, "RuleSetTable");
}

void RuleSetTable::writeAsm(QDataStream &stream, const AddressMapper &addressMapper, const std::vector<MapDescriptor> &mapDescriptors) {
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

    /*
     * Bug Fix: The monopoly check uses the GameRule flag from GameSelectInfo instead of the global variable. However, the GameRuleFlag is now repurposed as the mapset and as such
     * independent from the actual game rules. As both standard and easy maps can be in either mapsets, we need to query here the easyrule global flag instead of the GameRuleFlag
     * from GameSelectInfo.
     */
    quint32 returnAddr = addressMapper.boomStreetToStandard(0x801c4690);
    auto getRuleSetRoutine = allocate(writeGetRuleSetRoutine(addressMapper, 0, returnAddr), "writeRuleSetFromMapRoutine");
    stream.device()->seek(addressMapper.toFileAddress(getRuleSetRoutine));
    insts = writeGetRuleSetRoutine(addressMapper, getRuleSetRoutine, returnAddr); // re-write the routine again since now we know where it is located in the main dol
    for (quint32 inst: qAsConst(insts)) stream << inst;
    virtualPos = addressMapper.boomStreetToStandard(0x801c468c);
    stream.device()->seek(addressMapper.toFileAddress(virtualPos));
    // lha r3,0x3c(r30)  -> bl ruleSetFromMapRoutine
    stream << PowerPcAsm::bl(virtualPos, getRuleSetRoutine);

    // -- Always enable the buttons for ON/OFF negotiations and "Add vacant plots"
    // easy mode negotiations
    stream.device()->seek(addressMapper.boomToFileAddress(0x801e237c)); stream << PowerPcAsm::nop();
    stream.device()->seek(addressMapper.boomToFileAddress(0x801e238c)); stream << PowerPcAsm::nop();
    // easy mode vacant plots
    stream.device()->seek(addressMapper.boomToFileAddress(0x801e239c)); stream << PowerPcAsm::nop();
    stream.device()->seek(addressMapper.boomToFileAddress(0x801e23ac)); stream << PowerPcAsm::nop();
    // standard mode vacant plots
    stream.device()->seek(addressMapper.boomToFileAddress(0x801e2544)); stream << PowerPcAsm::nop();
    stream.device()->seek(addressMapper.boomToFileAddress(0x801e2554)); stream << PowerPcAsm::nop();

    // enable vacant plots placement in easy mode as well
    stream.device()->seek(addressMapper.boomToFileAddress(0x800ccd20)); stream << PowerPcAsm::nop();

}

void RuleSetTable::readAsm(QDataStream &stream, std::vector<MapDescriptor> &mapDescriptors, const AddressMapper &, bool isVanilla) {
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

bool RuleSetTable::readIsVanilla(QDataStream &stream, const AddressMapper &addressMapper) {
    stream.device()->seek(addressMapper.boomToFileAddress(0x801cca98));
    quint32 opcode; stream >> opcode;
    // mulli r0,r3,0x38
    return opcode == PowerPcAsm::mulli(0, 3, 0x38);
}

QVector<quint32> RuleSetTable::writeRuleSetFromMapRoutine(const AddressMapper &addressMapper, quint32 routineStartAddress) {
    quint32 Game_GetRuleFlag = addressMapper.boomStreetToStandard(0x801cca98);
    quint32 GetGameSelectInfo = addressMapper.boomStreetToStandard(0x800113f0);

    // precondition: r7 is unused
    // precondition: r24 is mapId
    // precondition: r25 is global rule set which we are gonna use to store the linkreturn
    return {
        PowerPcAsm::mflr(25),
        PowerPcAsm::mr(3, 24),                                                   // r3 <- r24
        PowerPcAsm::bl(routineStartAddress, 2 /*asm.Count*/, Game_GetRuleFlag),  // r3 <- bl Game_GetRuleFlag(r3)
        PowerPcAsm::stw(3, 0x53f4, 29),                                          // gameRule <- r3
        //PowerPcAsm::mr(7, 3),                                                    // r7 <- r3
        //PowerPcAsm::bl(routineStartAddress, 5 /*asm.Count*/, GetGameSelectInfo), // r3 <- bl GetGameSelectInfo()
        //PowerPcAsm::stb(7, 0x226, 3),                                            // GetGameSelectInfo.easyRules = gameRule
        PowerPcAsm::mtlr(25),
        PowerPcAsm::blr()                                                        // return
    };
}

QVector<quint32> RuleSetTable::writeGetRuleSetRoutine(const AddressMapper &addressMapper, quint32 routineStartAddress, quint32 routineReturnAddress) {
    PowerPcAsm::Pair16Bit EasyMode = PowerPcAsm::make16bitValuePair(addressMapper.boomStreetToStandard(0x8055240f));

    return {
        PowerPcAsm::lis(28, EasyMode.upper),             // |
        PowerPcAsm::addi(28, 28, EasyMode.lower),        // / load address of global easy variable
        PowerPcAsm::lbz(28, 0, 28),                      // r28 <- easyMode
        PowerPcAsm::b(routineStartAddress, 3/*asm.count*/, routineReturnAddress)
    };
}

