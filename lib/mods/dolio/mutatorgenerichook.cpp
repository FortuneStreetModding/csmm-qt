#include "mutatorgenerichook.h"
#include "eventsquaremod.h"
#include "lib/mods/dolio/mutatortable.h"
#include "lib/powerpcasm.h"
#include "lib/mutator/mutator.h"

quint32 MutatorGenericHook::getForceVentureCardVariable(const ModListType &modList) {
    auto it = std::find_if(modList.begin(), modList.end(), [](const auto &mod) { return mod->modId() == EventSquareMod::MODID.data(); });
    auto modHolder = it != modList.end() ? *it : CSMMModHolder();
    auto castedMutatorMod = std::dynamic_pointer_cast<const EventSquareMod>(modHolder.modHandle());
    if (!castedMutatorMod) {
        throw ModException("EventSquare mod not found!");
    }
    return castedMutatorMod->forceVentureCardVariable;
}

void MutatorGenericHook::writeAsm(QDataStream &stream, const AddressMapper &addressMapper, const std::vector<MapDescriptor> &) {
    quint32 getMutatorDataSubroutine = mutatorTableRoutineAddr(modList());
    quint32 forceVentureCardVariable = getForceVentureCardVariable(modList());

    // --- Mutator Dice Rolled Flag ---
    QVector<quint32> singleValue;
    singleValue.append(0);
    const quint32 hasRolledDice = allocate(singleValue, "MutatorGenericHook.HasRolledDice", false);

    // --- Set MutatorGenericHook.HasRolledDice = 0 at beginTurn ---
    quint32 hijackAddr = addressMapper.boomStreetToStandard(0x8007ea70);
    quint32 procInitMutatorAtBeginTurn = allocate(writeInitMutatorAtBeginTurn(addressMapper, 0, hasRolledDice, getMutatorDataSubroutine), "procInitMutatorAtBeginTurn");
    stream.device()->seek(addressMapper.toFileAddress(procInitMutatorAtBeginTurn));
    auto routineCode = writeInitMutatorAtBeginTurn(addressMapper, procInitMutatorAtBeginTurn, hasRolledDice, getMutatorDataSubroutine);
    for (quint32 inst: qAsConst(routineCode)) stream << inst; // re-write the routine again since now we know where it is located in the main dol
    stream.device()->seek(addressMapper.toFileAddress(hijackAddr));
    // li r4,0        ->  b procInitMutatorAtBeginTurn
    stream << PowerPcAsm::b(hijackAddr, procInitMutatorAtBeginTurn);

    // --- Set MutatorGenericHook.HasRolledDice = diceRoll on first roll ---
    hijackAddr = addressMapper.boomStreetToStandard(0x800c1554);
    quint32 procRememberDiceRoll = allocate(writeRememberDiceRoll(addressMapper, 0, hasRolledDice), "procRememberDiceRoll");
    stream.device()->seek(addressMapper.toFileAddress(procRememberDiceRoll));
    routineCode = writeRememberDiceRoll(addressMapper, procRememberDiceRoll, hasRolledDice);
    for (quint32 inst: qAsConst(routineCode)) stream << inst; // re-write the routine again since now we know where it is located in the main dol
    stream.device()->seek(addressMapper.toFileAddress(hijackAddr));
    // lwz r0,0x64(r1)   ->  b procRememberDiceRoll
    stream << PowerPcAsm::b(hijackAddr, procRememberDiceRoll);

    // --- Clear Mutator Dice Rolled Flag ---
    hijackAddr = addressMapper.boomStreetToStandard(0x800c57dc);
    quint32 procRollAgainRoutine = allocate(writeRollAgainRoutine(addressMapper, 0, hasRolledDice, forceVentureCardVariable, getMutatorDataSubroutine), "procRollAgainRoutine");
    stream.device()->seek(addressMapper.toFileAddress(procRollAgainRoutine));
    routineCode = writeRollAgainRoutine(addressMapper, procRollAgainRoutine, hasRolledDice, forceVentureCardVariable, getMutatorDataSubroutine);
    for (quint32 inst: qAsConst(routineCode)) stream << inst; // re-write the routine again since now we know where it is located in the main dol
    stream.device()->seek(addressMapper.toFileAddress(hijackAddr));
    // mr r3,r24      ->  b procRollAgainRoutine
    stream << PowerPcAsm::b(hijackAddr, procRollAgainRoutine);
}

QVector<quint32> MutatorGenericHook::writeInitMutatorAtBeginTurn(const AddressMapper &addressMapper, quint32 routineStartAddress, const quint32 hasRolledDice, quint32 getMutatorDataSubroutine) {
    // postcondition: r4 - 0x0
    //                r5 - 0xe0
    PowerPcAsm::Pair16Bit d = PowerPcAsm::make16bitValuePair(hasRolledDice);
    auto returnAddr = addressMapper.boomStreetToStandard(0x8007ea74);

    QVector<quint32> asm_;
    asm_.append(PowerPcAsm::lis(4, d.upper));           // \.
    asm_.append(PowerPcAsm::addi(4, 4, d.lower));       // /. r4 <- &hasRolledDice
    asm_.append(PowerPcAsm::li(5, 0));                  // \.
    asm_.append(PowerPcAsm::stw(5, 0, 4));              // /. hasRolledDice = false
    asm_.append(PowerPcAsm::li(4, 0x0));                // \.
    asm_.append(PowerPcAsm::li(5, 0xe0));               // /. replaced opcodes
    asm_.append(PowerPcAsm::b(routineStartAddress, asm_.count(), returnAddr));
    return asm_;
}

QVector<quint32> MutatorGenericHook::writeRememberDiceRoll(const AddressMapper &addressMapper, quint32 routineStartAddress, const quint32 hasRolledDice) {
    // precondition: r3 - diceRollValue
    PowerPcAsm::Pair16Bit d = PowerPcAsm::make16bitValuePair(hasRolledDice);
    auto returnAddr = addressMapper.boomStreetToStandard(0x800c1558);
    QVector<quint32> asm_;
    asm_.append(PowerPcAsm::lis(4, d.upper));           // \.
    asm_.append(PowerPcAsm::addi(4, 4, d.lower));       // /. r4 <- &hasRolledDice
    asm_.append(PowerPcAsm::lwz(5, 0, 4));              // |. r5 <- hasRolledDice
    asm_.append(PowerPcAsm::cmpwi(5, 0));               // \. if (hasRolledDice == 0) {
    asm_.append(PowerPcAsm::bne(2));                    // |.
    asm_.append(PowerPcAsm::stw(3, 0, 4));              // |.   hasRolledDice = diceRollValue
                                                        // /. }
    asm_.append(PowerPcAsm::lwz(0, 0x14, 1));                                       // |. replaced opcdode
    asm_.append(PowerPcAsm::b(routineStartAddress, asm_.count(), returnAddr));      // |.
    return asm_;
}

QVector<quint32> MutatorGenericHook::writeRollAgainRoutine(const AddressMapper &addressMapper, quint32 routineStartAddress, const quint32 hasRolledDice, const quint32 forceVentureCardVariable, quint32 getMutatorDataSubroutine) {
    auto Gm_Board = addressMapper.boomStreetToStandard(0x8054d018);
    PowerPcAsm::Pair16Bit b = PowerPcAsm::make16bitValuePair(Gm_Board);
    auto Gm_Board_StopPlace = addressMapper.boomStreetToStandard(0x8007f1ec);
    auto gameProgressChangeModeRoutine = addressMapper.boomStreetToStandard(0x800c093c);
    PowerPcAsm::Pair16Bit d = PowerPcAsm::make16bitValuePair(hasRolledDice);
    PowerPcAsm::Pair16Bit v = PowerPcAsm::make16bitValuePair(forceVentureCardVariable);
    auto returnAddr = addressMapper.boomStreetToStandard(0x800c57e0);
    auto completeReturnAddr = addressMapper.boomStreetToStandard(0x800c5a9c);

    QVector<quint32> asm_;
    asm_.append(PowerPcAsm::li(3, 3/* TODO */));
    asm_.append(PowerPcAsm::bl(routineStartAddress, asm_.count(), getMutatorDataSubroutine));
    asm_.append(PowerPcAsm::cmpwi(3, 0));
    asm_.append(PowerPcAsm::bne(3)); // goto mutator
    int vanilla = asm_.size();
                // vanilla
    asm_.append(PowerPcAsm::mr(3, 24));                                           // |. replaced opcode
    asm_.append(PowerPcAsm::b(routineStartAddress, asm_.count(), returnAddr));
                // mutator stuff
    asm_.append(PowerPcAsm::lwz(6, 0x0, 3));            // r6 <- whenRollIs
    asm_.append(PowerPcAsm::lis(4, d.upper));           // \.
    asm_.append(PowerPcAsm::addi(4, 4, d.lower));       // /. r4 <- &hasRolledDice
    asm_.append(PowerPcAsm::lwz(5, 0, 4));              // /. r5 <- hasRolledDice
    asm_.append(PowerPcAsm::cmpwi(6, -1));              // \. if(hasRolledDice == -1)
    asm_.append(PowerPcAsm::beq(4));                    // /.   goto rollAgain
    asm_.append(PowerPcAsm::cmpw(6, 5));                // \. if(whenRollIs == hasRolledDice)
    asm_.append(PowerPcAsm::beq(2));                    // /.   goto rollAgain
    asm_.append(PowerPcAsm::b(asm_.size(), vanilla));   // /. else goto vanilla

    asm_.append(PowerPcAsm::lwz(6, 0x4, 3));            // r6 <- onlyOnce
    asm_.append(PowerPcAsm::cmpwi(6, 0x0));             // \. if (!onlyOnce) {
    asm_.append(PowerPcAsm::bne(3));                    // |.
    asm_.append(PowerPcAsm::li(5, 0));                  // |.
    asm_.append(PowerPcAsm::stw(5, 0, 4));              // |.   hasRolledDice <- 0
                                                        // /. }
    asm_.append(PowerPcAsm::lis(3, v.upper));           // \.
    asm_.append(PowerPcAsm::addi(3, 3, v.lower));       // |. forceVentureCardVariable <- 2
    asm_.append(PowerPcAsm::li(5, 2));                  // |.
    asm_.append(PowerPcAsm::stw(5, 0x0, 3));            // /.

    asm_.append(PowerPcAsm::mr(3, 24));                 // | r3 <- GameProgress *
    asm_.append(PowerPcAsm::li(4, 0x1f));               // | li r4,0x1n  (the GameProgress mode id 0x1b is for picking a venture card)
    asm_.append(PowerPcAsm::li(5, -0x1));               // | li r5,-0x1
    asm_.append(PowerPcAsm::li(6, -0x1));               // | li r6,-0x1
    asm_.append(PowerPcAsm::li(7, 0x0));                // | li r7,0x0
    asm_.append(PowerPcAsm::bl(routineStartAddress, asm_.count(), gameProgressChangeModeRoutine));        // | bl Game::GameProgress::changeMode
    asm_.append(PowerPcAsm::b(routineStartAddress, asm_.count(), completeReturnAddr));
    return asm_;
}

void MutatorGenericHook::readAsm(QDataStream &, const AddressMapper &, std::vector<MapDescriptor> &) { /* crab nothing to do crab */ }

