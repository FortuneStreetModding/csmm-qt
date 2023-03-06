#include "mutatorstockprice.h"
#include "lib/powerpcasm.h"
#include "lib/mutator/mutator.h"

void MutatorStockPrice::writeAsm(QDataStream &stream, const AddressMapper &addressMapper, const std::vector<MapDescriptor> &) {
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

QVector<quint32> MutatorStockPrice::writeStockPriceMultiplier(const AddressMapper &addressMapper, quint32 routineStartAddress, quint32 getMutatorDataSubroutine) {
    auto returnAddr = addressMapper.boomStreetToStandard(0x80092388);

    QVector<quint32> asm_;
    auto labels = PowerPcAsm::LabelTable();
    asm_.append(PowerPcAsm::li(3, StockPriceType));                                            // \.
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
    asm_.append(PowerPcAsm::lha(5, 0x4, 3));                                                   // |. r5 <- constant
    asm_.append(PowerPcAsm::add(4, 5, 4));                                                     // |. r4 <- r4 + r5
    asm_.append(PowerPcAsm::cmpwi(4, 0x0));                                                    // |. replaced opcode
    asm_.append(PowerPcAsm::b(routineStartAddress, asm_.count(), returnAddr));                 // |. return
    labels.checkProperlyLinked();
    return asm_;
}

void MutatorStockPrice::readAsm(QDataStream &, const AddressMapper &, std::vector<MapDescriptor> &) { /* crab nothing to do crab */ }

