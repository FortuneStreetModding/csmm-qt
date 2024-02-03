#include "mutatorrollshoppricemultiplier.h"
#include "lib/mods/dolio/mutatortable.h"
#include "lib/powerpcasm.h"
#include "lib/mutator/mutator.h"

// Here is how vanilla game works:
//
// if(gameProgressMode == player_stops_0x09) {
//   Game::GPProcMoveStop::ProcStop()                           // This function decides what to do when a player stops at a square
//    |_ Gm::Board::StopPlace()                                 // This function executes a specific square function. It also handles updating the player's cash.
//        |_ Place::CalcGain()                                  // This function calculates how much the base price of a square changes (e.g. when having a 30% bonus venture card active)
//    |_ Game::GameProgress::changeMode(transfer_money_0x19)    // This function moves the game state from "playerStopping" to "moneyTransfering". After the money has been transferred the end of the turn is executed.
// }
//
// We modify the game code to be
//
// if(gameProgressMode == player_stop_0x09) {
//     Game::GPProcMoveStop::ProcStop()
//      |_ Gm::Board::StopPlace()
//          |_ Place::CalcGain()
//              |_ MutatorRollShopPriceMultiplier()
//      |_ {
//           if(mutator.diceRolled || !mutator.active) {
//             mutator.diceRolled = false
//             Game::GameProgress::changeMode(transfer_money_0x19)
//           } else {
//             mutator.diceRolled = true
//             Game::GameProgress::changeMode(roll_dice_0x05, player_stop_0x09)
//           }
//         }
//   }
//
// MutatorRollShopPriceMultiplier() {
//   if(mutatorDiceRolled) {
//     return price * diceValue
//   } else {
//     return 0
//   }
// }
//
void MutatorRollShopPriceMultiplier::writeAsm(QDataStream &stream, const AddressMapper &addressMapper, const std::vector<MapDescriptor> &) {
    quint32 getMutatorDataSubroutine = mutatorTableRoutineAddr(modList());

    // --- Mutator Dice Rolled Flag ---
    QVector<quint32> singleValue;
    singleValue.append(0);
    const quint32 hasRolledDice = allocate(singleValue, "MutatorRollShopPriceMultiplier.HasRolledDice", false);
    const quint32 lastShopPrice = allocate(singleValue, "MutatorRollShopPriceMultiplier.lastShopPrice", false);

    // --- Roll before paying up ---
    quint32 hijackAddr = addressMapper.boomStreetToStandard(0x800fa7b4);
    quint32 procRollDiceBeforePayingRoutine = allocate(writeRollDiceBeforePayingRoutine(addressMapper, 0, lastShopPrice, hasRolledDice, getMutatorDataSubroutine), "procRollDiceBeforePayingRoutine");
    stream.device()->seek(addressMapper.toFileAddress(procRollDiceBeforePayingRoutine));
    auto routineCode = writeRollDiceBeforePayingRoutine(addressMapper, procRollDiceBeforePayingRoutine, lastShopPrice, hasRolledDice, getMutatorDataSubroutine);
    for (quint32 inst: std::as_const(routineCode)) stream << inst; // re-write the routine again since now we know where it is located in the main dol
    stream.device()->seek(addressMapper.toFileAddress(hijackAddr));
    // li r7,0        ->  b procRollDiceBeforePayingRoutine
    stream << PowerPcAsm::b(hijackAddr, procRollDiceBeforePayingRoutine);

    // --- Remember Square in r10 ---
    hijackAddr = addressMapper.boomStreetToStandard(0x8008ff30);
    quint32 procRememberSquareType = allocate(writeRememberSquareType(addressMapper, 0, hasRolledDice), "procRememberSquareType");
    stream.device()->seek(addressMapper.toFileAddress(procRememberSquareType));
    routineCode = writeRememberSquareType(addressMapper, procRememberSquareType, hasRolledDice);
    for (quint32 inst: std::as_const(routineCode)) stream << inst; // re-write the routine again since now we know where it is located in the main dol
    stream.device()->seek(addressMapper.toFileAddress(hijackAddr));
    // lbz r0,0x51(r3)   ->  b procRememberSquareType
    stream << PowerPcAsm::b(hijackAddr, procRememberSquareType);

    // --- Calc shop price depending on previously rolled dice value ---
    hijackAddr = addressMapper.boomStreetToStandard(0x8008ff9c);
    quint32 procCalculateGainRoutine = allocate(writeCalculateGainRoutine(addressMapper, 0, lastShopPrice, hasRolledDice, getMutatorDataSubroutine), "procCalculateGainRoutine");
    stream.device()->seek(addressMapper.toFileAddress(procCalculateGainRoutine));
    routineCode = writeCalculateGainRoutine(addressMapper, procCalculateGainRoutine, lastShopPrice, hasRolledDice, getMutatorDataSubroutine);
    for (quint32 inst: std::as_const(routineCode)) stream << inst; // re-write the routine again since now we know where it is located in the main dol
    stream.device()->seek(addressMapper.toFileAddress(hijackAddr));
    // add r3,r0,r3   ->  b procCalculateGainRoutine
    stream << PowerPcAsm::b(hijackAddr, procCalculateGainRoutine);

    // --- Clear Mutator Dice Rolled Flag ---
    hijackAddr = addressMapper.boomStreetToStandard(0x80475e20);
    quint32 procClearDiceRolledFlag = allocate(writeClearDiceRolledFlag(addressMapper, 0, hasRolledDice), "procClearDiceRolledFlag");
    stream.device()->seek(addressMapper.toFileAddress(procClearDiceRolledFlag));
    routineCode = writeClearDiceRolledFlag(addressMapper, procClearDiceRolledFlag, hasRolledDice);
    for (quint32 inst: std::as_const(routineCode)) stream << inst; // re-write the routine again since now we know where it is located in the main dol
    stream.device()->seek(addressMapper.toFileAddress(hijackAddr));
    stream << procClearDiceRolledFlag;

    // --- Do not show "Do you want to stop here" message twice ---
    hijackAddr = addressMapper.boomStreetToStandard(0x800f9cf0);
    quint32 procDontShowQuestionBoxAgain = allocate(writeDontShowQuestionBoxAgain(addressMapper, 0, hasRolledDice), "procDontShowQuestionBoxAgain");
    stream.device()->seek(addressMapper.toFileAddress(procDontShowQuestionBoxAgain));
    routineCode = writeDontShowQuestionBoxAgain(addressMapper, procDontShowQuestionBoxAgain, hasRolledDice);
    for (quint32 inst: std::as_const(routineCode)) stream << inst; // re-write the routine again since now we know where it is located in the main dol
    stream.device()->seek(addressMapper.toFileAddress(hijackAddr));
    // lwz r0,0x28(r20)      ->  b procDontShowQuestionBoxAgain
    stream << PowerPcAsm::b(hijackAddr, procDontShowQuestionBoxAgain);

    // --- Close dice after pressing OK when seing how much you have paid ---
    hijackAddr = addressMapper.boomStreetToStandard(0x80118994);
    quint32 procCloseDice = allocate(writeCloseDice(addressMapper, 0, hasRolledDice), "procCloseDice");
    stream.device()->seek(addressMapper.toFileAddress(procCloseDice));
    routineCode = writeCloseDice(addressMapper, procCloseDice, hasRolledDice);
    for (quint32 inst: std::as_const(routineCode)) stream << inst; // re-write the routine again since now we know where it is located in the main dol
    stream.device()->seek(addressMapper.toFileAddress(hijackAddr));
    // li r4,0x1       ->  b procCloseDice
    stream << PowerPcAsm::b(hijackAddr, procCloseDice);

}

QVector<quint32> MutatorRollShopPriceMultiplier::writeRollDiceBeforePayingRoutine(const AddressMapper &addressMapper, quint32 routineStartAddress, const quint32 lastShopPrice, const quint32 hasRolledDice, quint32 getMutatorDataSubroutine) {
    // postcondition: r4 - progress mode
    //                r5 - progress mode afterwards
    //                r6 - progress mode if cancelled

    // mode 0x05 = roll dice
    // mode 0x09 = player stop
    // mode 0x19 = transfer money

    auto Gm_Board = addressMapper.boomStreetToStandard(0x8054d018);
    PowerPcAsm::Pair16Bit b = PowerPcAsm::make16bitValuePair(Gm_Board);
    auto Gm_Board_StopPlace = addressMapper.boomStreetToStandard(0x8007f1ec);
    PowerPcAsm::Pair16Bit d = PowerPcAsm::make16bitValuePair(hasRolledDice);
    PowerPcAsm::Pair16Bit s = PowerPcAsm::make16bitValuePair(lastShopPrice);
    auto returnAddr = addressMapper.boomStreetToStandard(0x800fa7b8);

    QVector<quint32> asm_;
    auto labels = PowerPcAsm::LabelTable();
    asm_.append(PowerPcAsm::li(3, RollShopPriceMultiplierType));
    asm_.append(PowerPcAsm::bl(routineStartAddress, asm_.count(), getMutatorDataSubroutine));
    asm_.append(PowerPcAsm::cmpwi(3, 0));
    asm_.append(PowerPcAsm::bne(labels, "mutator", asm_)); // goto mutator
    labels.define("vanilla", asm_);
                // vanilla
    asm_.append(PowerPcAsm::lwz(3, 0x18, 20));
    asm_.append(PowerPcAsm::li(4, 0x19));
    asm_.append(PowerPcAsm::li(5, -1));
    asm_.append(PowerPcAsm::li(6, -1));
    asm_.append(PowerPcAsm::li(7, 0));
    asm_.append(PowerPcAsm::b(routineStartAddress, asm_.count(), returnAddr));
                // mutator stuff
    labels.define("mutator", asm_);
    asm_.append(PowerPcAsm::lis(4, s.upper));              // \.
    asm_.append(PowerPcAsm::addi(4, 4, s.lower));          // |.
    asm_.append(PowerPcAsm::lwz(5, 0, 4));                 // /. r5 <- lastShopPrice
    asm_.append(PowerPcAsm::cmpwi(5, 0));                  // \. if(lastShopPrice == 0)
    asm_.append(PowerPcAsm::beq(labels, "vanilla", asm_)); // /.   goto vanilla

    asm_.append(PowerPcAsm::lis(4, d.upper));              // \.
    asm_.append(PowerPcAsm::addi(4, 4, d.lower));          // |.
    asm_.append(PowerPcAsm::lwz(5, 0, 4));                 // /. r5 <- hasRolledDice
    asm_.append(PowerPcAsm::cmpwi(5, 0));                  // \. if(hasRolledDice)
    asm_.append(PowerPcAsm::bne(labels, "vanilla", asm_)); // /.   goto vanilla

    asm_.append(PowerPcAsm::li(5, 1));                     // \. hasRolledDice = true
    asm_.append(PowerPcAsm::stw(5, 0, 4));                 // /.

    asm_.append(PowerPcAsm::lbz(3, 0x0, 3));               // r3 <- maxDiceRoll
    asm_.append(PowerPcAsm::lwz(4, 0x18, 20));             // r4 <- GameProgress*
    asm_.append(PowerPcAsm::addis(4, 4, 0x2));             // \ GameProgress->MaxDiceRoll <- r4
    asm_.append(PowerPcAsm::stw(3, 0x2814, 4));            // /

    // asm_.append(PowerPcAsm::lis(3, b.upper));                                             // \.
    // asm_.append(PowerPcAsm::addi(3, 3, b.lower));                                         // |.
    // asm_.append(PowerPcAsm::li(4, -1));                                                   // |. call Gm_Board_StopPlace()
    // asm_.append(PowerPcAsm::bl(routineStartAddress, asm_.count(), Gm_Board_StopPlace));   // /.

    asm_.append(PowerPcAsm::lwz(3, 0x18, 20));
    asm_.append(PowerPcAsm::li(4, 0x5));
    asm_.append(PowerPcAsm::li(5, 0x9));
    asm_.append(PowerPcAsm::li(6, -1));
    asm_.append(PowerPcAsm::li(7, 0));
    asm_.append(PowerPcAsm::b(routineStartAddress, asm_.count(), returnAddr));
    labels.checkProperlyLinked();
    return asm_;
}

QVector<quint32> MutatorRollShopPriceMultiplier::writeRememberSquareType(const AddressMapper &addressMapper, quint32 routineStartAddress, const quint32 hasRolledDice) {
    auto returnAddr = addressMapper.boomStreetToStandard(0x8008ff34);
    QVector<quint32> asm_;
    asm_.append(PowerPcAsm::lbz(0, 0x51, 3));                                       // |. replaced opcdode
    asm_.append(PowerPcAsm::lbz(10, 0x4d, 3));                                      // |. r10 <- squareType
    asm_.append(PowerPcAsm::b(routineStartAddress, asm_.count(), returnAddr));      // |.
    return asm_;
}

QVector<quint32> MutatorRollShopPriceMultiplier::writeCalculateGainRoutine(const AddressMapper &addressMapper, quint32 routineStartAddress, const quint32 lastShopPrice, const quint32 hasRolledDice, quint32 getMutatorDataSubroutine) {
    // precondition:  r3 - calculated shop price
    //               r10 - square type
    // postcondition: r3 - calculated shop price
    auto gameProgressObject = addressMapper.boomStreetToStandard(0x80817908);
    auto returnAddr = addressMapper.boomStreetToStandard(0x8008ffa0);
    PowerPcAsm::Pair16Bit v = PowerPcAsm::make16bitValuePair(gameProgressObject);
    PowerPcAsm::Pair16Bit d = PowerPcAsm::make16bitValuePair(hasRolledDice);
    PowerPcAsm::Pair16Bit s = PowerPcAsm::make16bitValuePair(lastShopPrice);

    QVector<quint32> asm_;
    auto labels = PowerPcAsm::LabelTable();
    asm_.append(PowerPcAsm::add(3, 0, 3));
    asm_.append(PowerPcAsm::cmpwi(10, 0x0));                                        // \.
    asm_.append(PowerPcAsm::beq(labels, "shop", asm_));                              // |. if (squareType != PROPERTY_0x0 &&
    asm_.append(PowerPcAsm::cmpwi(10, 0x37));                                       // |.     squareType != VACANT_PLOT_3_STAR_SHOP_0x37) {
    asm_.append(PowerPcAsm::beq(labels, "shop", asm_));                              // /.
    asm_.append(PowerPcAsm::b(routineStartAddress, asm_.count(), returnAddr));      // |.   return vanilla shop price
                                                                                    // |. }
    labels.define("shop", asm_);                                                    // property or 3 star shop
    asm_.append(PowerPcAsm::lis(7, s.upper));                                       // \.
    asm_.append(PowerPcAsm::addi(7, 7, s.lower));                                   // /. r7 <- &lastShopPrice
    asm_.append(PowerPcAsm::stw(3, 0, 7));                                          // |. lastShopPrice = price

    asm_.append(PowerPcAsm::lis(7, d.upper));                                       // \.
    asm_.append(PowerPcAsm::addi(7, 7, d.lower));                                   // /. r7 <- &hasRolledDice
    asm_.append(PowerPcAsm::lwz(7, 0, 7));                                          // |. r7 <- hasRolledDice
    asm_.append(PowerPcAsm::cmpwi(7, 1));                                           // \. if(!hasRolledDice) {
    asm_.append(PowerPcAsm::beq(labels, "mulDice", asm_));                          // |.
    asm_.append(PowerPcAsm::mr(6, 3));                                              // |.   remember shop price in r6
    asm_.append(PowerPcAsm::mflr(7));                                               // |.   remember link register value in r7
    asm_.append(PowerPcAsm::li(3, RollShopPriceMultiplierType));                    // \.
    asm_.append(PowerPcAsm::bl(routineStartAddress, asm_.count(), getMutatorDataSubroutine)); // /.   call getMutatorData()
    asm_.append(PowerPcAsm::mtlr(7));                                               // |.   restore link register value from r7
    asm_.append(PowerPcAsm::cmpwi(3, 0));                                           // \.   if(mutator == NULL) {
    asm_.append(PowerPcAsm::bne(labels, "returnZero", asm_));                       // |.
    asm_.append(PowerPcAsm::mr(3, 6));                                              // |.     restore shop price from r6
    asm_.append(PowerPcAsm::b(routineStartAddress, asm_.count(), returnAddr));      // /.     return vanilla shop price
    labels.define("returnZero", asm_);                                              // \.   } else {
    asm_.append(PowerPcAsm::li(3, 0));                                              // |.
    asm_.append(PowerPcAsm::b(routineStartAddress, asm_.count(), returnAddr));      // /.     return 0
                                                                                    // |.   }
    labels.define("mulDice", asm_);                                                 // /. } else {
    asm_.append(PowerPcAsm::lis(7, v.upper));                                       // \.
    asm_.append(PowerPcAsm::addi(7, 7, v.lower));                                   // |.
    asm_.append(PowerPcAsm::lwz(7, 0, 7));                                          // |.   r7 <- dice value
    asm_.append(PowerPcAsm::lwz(7, 0x414, 7));                                      // /.
    asm_.append(PowerPcAsm::mullw(3, 7, 3));                                        // |    return r3 * r7
    asm_.append(PowerPcAsm::b(routineStartAddress, asm_.count(), returnAddr));      // |. }
    labels.checkProperlyLinked();
    return asm_;
}

QVector<quint32> MutatorRollShopPriceMultiplier::writeClearDiceRolledFlag(const AddressMapper &addressMapper, quint32 routineStartAddress, const quint32 hasRolledDice) {
    // auto returnAddr = addressMapper.boomStreetToStandard(0x8007ea64);
    PowerPcAsm::Pair16Bit d = PowerPcAsm::make16bitValuePair(hasRolledDice);

    QVector<quint32> asm_;
    asm_.append(PowerPcAsm::li(0, 0));                                            // \.> replaced opcode
    asm_.append(PowerPcAsm::lis(4, d.upper));                                     // |
    asm_.append(PowerPcAsm::addi(4, 4, d.lower));                                 // |. hasRolledDice = false
    asm_.append(PowerPcAsm::stw(0, 0, 4));                                        // /.
    asm_.append(PowerPcAsm::blr());
    // asm_.append(PowerPcAsm::b(routineStartAddress, asm_.count(), returnAddr));
    return asm_;
}

QVector<quint32> MutatorRollShopPriceMultiplier::writeDontShowQuestionBoxAgain(const AddressMapper &addressMapper, quint32 routineStartAddress, const quint32 hasRolledDice) {
    auto Game_GameProgress_GetDice = addressMapper.boomStreetToStandard(0x800c5c34);
    auto Game_Dice_Close = addressMapper.boomStreetToStandard(0x801015c4);

    auto returnAddr = addressMapper.boomStreetToStandard(0x800f9cf4);
    PowerPcAsm::Pair16Bit d = PowerPcAsm::make16bitValuePair(hasRolledDice);

    QVector<quint32> asm_;
    auto labels = PowerPcAsm::LabelTable();
    asm_.append(PowerPcAsm::lwz(0, 0x28, 20));                                       // |. r0 <- GPProcMoveStop.state
    asm_.append(PowerPcAsm::lis(26, d.upper));                                       // \.
    asm_.append(PowerPcAsm::addi(26, 26, d.lower));                                  // |. r26 <- hasRolledDice
    asm_.append(PowerPcAsm::lwz(26, 0, 26));                                         // /.
    asm_.append(PowerPcAsm::cmpwi(26, 0));                                           // \. if(!hasRolledDice) {
    asm_.append(PowerPcAsm::bne(labels, "skipQuestionBox", asm_));                   // |.   return
    asm_.append(PowerPcAsm::b(routineStartAddress, asm_.count(), returnAddr));       // /. }
    //asm_.append(PowerPcAsm::mr(3, 24));                                              // \.
    //asm_.append(PowerPcAsm::lwz(3, 0x404, 21));                                      // |.
    //asm_.append(PowerPcAsm::li(4, 0));                                               // |. close dice
    //asm_.append(PowerPcAsm::bl(routineStartAddress, asm_.count(), Game_Dice_Close)); // /.
    //asm_.append(PowerPcAsm::lwz(3, 0x188, 28));                                      // \.
    //asm_.append(PowerPcAsm::lwz(4, 0x18, 20));                                       // |. restore vanilla parameters
    //asm_.append(PowerPcAsm::lwz(3, 0x74, 3));                                        // /.
    labels.define("skipQuestionBox", asm_);
    asm_.append(PowerPcAsm::li(0, 0x11c));                                           // |. r0 <- 0x11c  (state which is after confirming "yes")
    asm_.append(PowerPcAsm::b(routineStartAddress, asm_.count(), returnAddr));       // |. return
    labels.checkProperlyLinked();
    return asm_;
}

QVector<quint32> MutatorRollShopPriceMultiplier::writeCloseDice(const AddressMapper &addressMapper, quint32 routineStartAddress, const quint32 hasRolledDice) {
    auto Game_GameProgress_GetDice = addressMapper.boomStreetToStandard(0x800c5c34);
    auto Game_Dice_Close = addressMapper.boomStreetToStandard(0x801015c4);
    auto returnAddr = addressMapper.boomStreetToStandard(0x80118998);

    QVector<quint32> asm_;
    asm_.append(PowerPcAsm::lwz(3, 0x18, 20));                                                 // \.
    asm_.append(PowerPcAsm::bl(routineStartAddress, asm_.count(), Game_GameProgress_GetDice)); // |.
    asm_.append(PowerPcAsm::li(4, 0));                                                         // |. close dice
    asm_.append(PowerPcAsm::bl(routineStartAddress, asm_.count(), Game_Dice_Close));           // /.
    asm_.append(PowerPcAsm::addi(3, 1, 0x158));                                                // \. restore vanilla op
    asm_.append(PowerPcAsm::li(4, 1));                                                         // /. restore vanilla op
    asm_.append(PowerPcAsm::b(routineStartAddress, asm_.count(), returnAddr));                 // |. return
    return asm_;
}

void MutatorRollShopPriceMultiplier::readAsm(QDataStream &, const AddressMapper &, std::vector<MapDescriptor> &) { /* crab nothing to do crab */ }

