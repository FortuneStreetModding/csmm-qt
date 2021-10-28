#include "rollagaintable.h"
#include "lib/powerpcasm.h"

void RollAgainTable::writeAsm(QDataStream &stream, const AddressMapper &addressMapper, const QVector<MapDescriptor> &mapDescriptors) {
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

QVector<quint32> RollAgainTable::writeRollDiceBeforePayingRoutine(const AddressMapper &addressMapper, quint32 routineStartAddress) {
    // postcondition: r4 - progress mode
    //                r5 - progress mode afterwards
    //                r6 - progress mode if cancelled
    auto returnAddr = addressMapper.boomStreetToStandard(0x800fa7b8);

    // mode 0x05 = roll dice
    // mode 0x19 = transfer money
    // ChangeMode(r4 <- 0x05, r5 <- 0x19, r6 <- -1)
    return {
        PowerPcAsm::li(4, 0x5),
        PowerPcAsm::li(5, 0x19),
        PowerPcAsm::li(6, -1),
        PowerPcAsm::li(7, 0),
        PowerPcAsm::b(routineStartAddress, 4 /*asm.Count*/, returnAddr)
    };
}

QVector<quint32> RollAgainTable::writeCalculateGainRoutine(const AddressMapper &addressMapper, quint32 routineStartAddress) {
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

void RollAgainTable::readAsm(QDataStream &stream, QVector<MapDescriptor> &mapDescriptors, const AddressMapper &addressMapper, bool isVanilla) { /* crab nothing to do crab */ }
bool RollAgainTable::readIsVanilla(QDataStream &stream, const AddressMapper &addressMapper) { /* crab nothing to do crab */ return true; }
qint16 RollAgainTable::readTableRowCount(QDataStream &stream, const AddressMapper &addressMapper, bool isVanilla) { /* crab nothing to do crab */ return 0; }
quint32 RollAgainTable::readTableAddr(QDataStream &stream, const AddressMapper &addressMapper, bool isVanilla) { /* crab nothing to do crab */ return 0; }

