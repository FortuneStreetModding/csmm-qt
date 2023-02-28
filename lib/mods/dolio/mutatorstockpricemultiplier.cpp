#include "mutatorstockpricemultiplier.h"
#include "lib/powerpcasm.h"
#include "lib/mutator/mutator.h"

void MutatorStockPriceMultiplier::writeAsm(QDataStream &stream, const AddressMapper &addressMapper, const std::vector<MapDescriptor> &) {
    quint32 getMutatorDataSubroutine = mutatorTableRoutineAddr(modList());

    quint32 hijackAddr = addressMapper.boomStreetToStandard(0x80092384);
    quint32 procStockPriceMultiplier = allocate(writeStockPriceMultiplier(addressMapper, 0, getMutatorDataSubroutine), "procStockPriceMultiplier");
    stream.device()->seek(addressMapper.toFileAddress(procStockPriceMultiplier));
    auto routineCode = writeStockPriceMultiplier(addressMapper, procStockPriceMultiplier, getMutatorDataSubroutine);
    for (quint32 inst: qAsConst(routineCode)) stream << inst; // re-write the routine again since now we know where it is located in the main dol
    stream.device()->seek(addressMapper.toFileAddress(hijackAddr));
    // cmpwi r4,0x0            ->  b procStockPriceMultiplier
    stream << PowerPcAsm::b(hijackAddr, procStockPriceMultiplier);
}

QVector<quint32> MutatorStockPriceMultiplier::writeStockPriceMultiplier(const AddressMapper &addressMapper, quint32 routineStartAddress, quint32 getMutatorDataSubroutine) {
    // precondition: r29 - Place*
    //                r0 - shop price
    // postcondition: r0 - dont care
    //                r3 - dont care
    //                r4 - dont care
    auto returnAddr = addressMapper.boomStreetToStandard(0x80092388);

    QVector<quint32> asm_;
    auto labels = PowerPcAsm::LabelTable();
    asm_.append(PowerPcAsm::li(3, StockPriceMultiplierType));                                  // \.
    asm_.append(PowerPcAsm::bl(routineStartAddress, asm_.count(), getMutatorDataSubroutine));  // /. get mutatorData
    asm_.append(PowerPcAsm::cmpwi(3, 0));                                                      // \. if mutator != NULL
    asm_.append(PowerPcAsm::bne(labels, "mutator", asm_));                                     // /.   goto mutator
                // vanilla
    asm_.append(PowerPcAsm::cmpwi(4,0x0));                                                     // |. replaced opcode
    asm_.append(PowerPcAsm::b(routineStartAddress, asm_.count(), returnAddr));                 // |. return
                // mutator stuff
    labels.define("mutator", asm_);
    asm_.append(PowerPcAsm::lhz(5, 0x0, 3));                                                   // |. r5 <- numerator
    asm_.append(PowerPcAsm::mullw(4, 4, 5));                                                   // |. r4 <- r4 * r5
    asm_.append(PowerPcAsm::lhz(5, 0x2, 3));                                                   // |. r5 <- denominator
    asm_.append(PowerPcAsm::divwu(4, 4, 5));                                                   // |. r4 <- r4 / r5
    asm_.append(PowerPcAsm::cmpwi(4,0x0));                                                     // |. replaced opcode
    asm_.append(PowerPcAsm::b(routineStartAddress, asm_.count(), returnAddr));                 // |. return
    labels.checkProperlyLinked();
    return asm_;
}

void MutatorStockPriceMultiplier::readAsm(QDataStream &, const AddressMapper &, std::vector<MapDescriptor> &) { /* crab nothing to do crab */ }

