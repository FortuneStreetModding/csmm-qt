#include "mutatorrollshoppricemultiplier.h"
#include "lib/powerpcasm.h"
#include "lib/mutator/mutator.h"

void MutatorRollShopPriceMultiplier::writeAsm(QDataStream &stream, const AddressMapper &addressMapper, const QVector<MapDescriptor> &mapDescriptors) {
    // QVector<quint16> table;
    // quint32 priceMultTableAddr = allocate(table, "PRICE_MULTIPLIER_TABLE");

    // --- Roll before paying up ---
    quint32 hijackAddr = addressMapper.boomStreetToStandard(0x800fa7b4);
    quint32 procRollDiceBeforePayingRoutine = allocate(writeRollDiceBeforePayingRoutine(addressMapper, 0), "procRollDiceBeforePayingRoutine");
    stream.device()->seek(addressMapper.toFileAddress(procRollDiceBeforePayingRoutine));
    auto routineCode = writeRollDiceBeforePayingRoutine(addressMapper, procRollDiceBeforePayingRoutine);
    for (quint32 inst: qAsConst(routineCode)) stream << inst; // re-write the routine again since now we know where it is located in the main dol
    stream.device()->seek(addressMapper.toFileAddress(hijackAddr));
    // li r7,0        ->  b procRollDiceBeforePayingRoutine
    stream << PowerPcAsm::b(hijackAddr, procRollDiceBeforePayingRoutine);

    // --- Calc shop price depending on previously rolled dice value ---
    hijackAddr = addressMapper.boomStreetToStandard(0x8008ff9c);
    quint32 procCalculateGainRoutine = allocate(writeCalculateGainRoutine(addressMapper, 0), "procCalculateGainRoutine");
    stream.device()->seek(addressMapper.toFileAddress(procCalculateGainRoutine));
    routineCode = writeCalculateGainRoutine(addressMapper, procCalculateGainRoutine);
    for (quint32 inst: qAsConst(routineCode)) stream << inst; // re-write the routine again since now we know where it is located in the main dol
    stream.device()->seek(addressMapper.toFileAddress(hijackAddr));
    // cmpw r0,r3       ->  b procCalculateGainRoutine
    stream << PowerPcAsm::b(hijackAddr, procCalculateGainRoutine);

    // Here inject code:
    // Gm::Place::CalcGain
    // 8008ff9c
}

QVector<quint32> MutatorRollShopPriceMultiplier::writeRollDiceBeforePayingRoutine(const AddressMapper &addressMapper, quint32 routineStartAddress) {

    auto getMutatorDataSubroutine = addressMapper.boomStreetToStandard(0x80412c8c);

    // postcondition: r4 - progress mode
    //                r5 - progress mode afterwards
    //                r6 - progress mode if cancelled
    auto returnAddr = addressMapper.boomStreetToStandard(0x800fa7b8);

    // mode 0x05 = roll dice
    // mode 0x19 = transfer money
    // ChangeMode(r4 <- 0x05, r5 <- 0x19, r6 <- -1)
    return {
        PowerPcAsm::li(3, RollShopPriceMultiplierType),
        PowerPcAsm::bl(routineStartAddress, 1 /*asm.count*/, getMutatorDataSubroutine),
        PowerPcAsm::cmpw(3, 0),
        PowerPcAsm::bne(7),
                // vanilla parameters
        PowerPcAsm::lwz(3, 0x18, 20),
        PowerPcAsm::li(4, 0x19),
        PowerPcAsm::li(5, -1),
        PowerPcAsm::li(6, -1),
        PowerPcAsm::li(7, 0),
        PowerPcAsm::b(routineStartAddress, 9 /*asm.Count*/, returnAddr),
                // mutator parameters
        PowerPcAsm::lwz(3, 0x0, 3),    // r3 <- maxDiceRoll
        PowerPcAsm::lwz(4, 0x18, 20),  // r4 <- GameProgress*
        PowerPcAsm::addis(4, 4, 0x2),  // \ GameProgress->MaxDiceRoll <- r4
        PowerPcAsm::stw(3, 0x2814, 4), // /
        PowerPcAsm::lwz(3, 0x18, 20),
        PowerPcAsm::li(4, 0x5),
        PowerPcAsm::li(5, 0x19),
        PowerPcAsm::li(6, -1),
        PowerPcAsm::li(7, 0),
        PowerPcAsm::b(routineStartAddress, 19 /*asm.Count*/, returnAddr)
    };
}

QVector<quint32> MutatorRollShopPriceMultiplier::writeCalculateGainRoutine(const AddressMapper &addressMapper, quint32 routineStartAddress) {
    // precondition: r0 - total shops in district
    //               r3 - shops owned by player
    auto diceRollValue = addressMapper.boomStreetToStandard(0x8055274c);
    auto returnAddr = addressMapper.boomStreetToStandard(0x8008ffa0);
    PowerPcAsm::Pair16Bit v = PowerPcAsm::make16bitValuePair(diceRollValue);

    return {
        PowerPcAsm::add(3, 0, 3),
        PowerPcAsm::lis(7, v.upper),      // |
        PowerPcAsm::addi(7, 7, v.lower),  // |
        PowerPcAsm::lwz(7, 0, 7),         // | r0 <- dice value
        PowerPcAsm::mullw(3, 7, 3),
        PowerPcAsm::b(routineStartAddress, 5 /*asm.Count*/, returnAddr)
    };
}

void MutatorRollShopPriceMultiplier::readAsm(QDataStream &stream, const AddressMapper &addressMapper, QVector<MapDescriptor> &mapDescriptors) { /* crab nothing to do crab */ }

