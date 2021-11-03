#include "mutatorshoppricemultiplier.h"
#include "lib/powerpcasm.h"
#include "lib/mutator/mutator.h"

void MutatorShopPriceMultiplier::writeAsm(QDataStream &stream, const AddressMapper &addressMapper, const QVector<MapDescriptor> &) {
    // --- Owned Shop Price Multiplier ---
    {
        quint32 hijackAddr = addressMapper.boomStreetToStandard(0x8008fb48);
        quint32 procShopPriceMultiplier = allocate(writeShopPriceMultiplier(addressMapper, 0), "procShopPriceMultiplier");
        stream.device()->seek(addressMapper.toFileAddress(procShopPriceMultiplier));
        auto routineCode = writeShopPriceMultiplier(addressMapper, procShopPriceMultiplier);
        for (quint32 inst: qAsConst(routineCode)) stream << inst; // re-write the routine again since now we know where it is located in the main dol
        stream.device()->seek(addressMapper.toFileAddress(hijackAddr));
        // lwz r0,0x24(r1)            ->  b procShopPriceMultiplier
        stream << PowerPcAsm::b(hijackAddr, procShopPriceMultiplier);
    }

    // --- Unowned Shop Price Multiplier ---
    {
        quint32 hijackAddr = addressMapper.boomStreetToStandard(0x8008f0f4);
        quint32 procUnownedShopPriceMultiplier = allocate(writeUnownedShopPriceMultiplier(addressMapper, 0), "procUnownedShopPriceMultiplier");
        stream.device()->seek(addressMapper.toFileAddress(procUnownedShopPriceMultiplier));
        auto routineCode = writeUnownedShopPriceMultiplier(addressMapper, procUnownedShopPriceMultiplier);
        for (quint32 inst: qAsConst(routineCode)) stream << inst; // re-write the routine again since now we know where it is located in the main dol
        stream.device()->seek(addressMapper.toFileAddress(hijackAddr));
        // stw r4,0x40(r31)             ->  b procShopPriceMultiplier
        stream << PowerPcAsm::b(hijackAddr, procUnownedShopPriceMultiplier);
    }

    // lwz r24,0x28(r5)                                                                    -> lwz r24,0x40(r5)
    stream.device()->seek(addressMapper.boomToFileAddress(0x80089328)); stream << PowerPcAsm::lwz(24, 0x40, 5);
}

QVector<quint32> MutatorShopPriceMultiplier::writeShopPriceMultiplier(const AddressMapper &addressMapper, quint32 routineStartAddress) {
    auto getMutatorDataSubroutine = addressMapper.boomStreetToStandard(0x80412c8c);
    auto returnAddr = addressMapper.boomStreetToStandard(0x8008fb4c);

    QVector<quint32> asm_;
    asm_.append(PowerPcAsm::li(3, ShopPriceMultiplierType));                                   // \.
    asm_.append(PowerPcAsm::bl(routineStartAddress, asm_.count(), getMutatorDataSubroutine));  // /. get mutatorData
    asm_.append(PowerPcAsm::cmpwi(3, 0));                                                      // \. if mutator != NULL
    asm_.append(PowerPcAsm::bne(3));                                                           // /.   goto mutator
                // vanilla
    asm_.append(PowerPcAsm::lwz(0, 0x24, 1));                                                  // |. replaced opcode
    asm_.append(PowerPcAsm::b(routineStartAddress, asm_.count(), returnAddr));                 // |. return
                // mutator stuff
    asm_.append(PowerPcAsm::lwz(4, 0x40, 31));                                                 // |. r4 <- calculated shop price
    asm_.append(PowerPcAsm::lhz(5, 0x0, 3));                                                   // |. r5 <- numerator
    asm_.append(PowerPcAsm::mullw(4, 4, 5));                                                   // |. r4 <- r4 * r5
    asm_.append(PowerPcAsm::lhz(5, 0x2, 3));                                                   // |. r5 <- denominator
    asm_.append(PowerPcAsm::divwu(4, 4, 5));                                                   // |. r4 <- r4 / r5
    asm_.append(PowerPcAsm::stw(4, 0x40, 31));                                                 // |. calculated shop price <- r4
    asm_.append(PowerPcAsm::lwz(0, 0x24, 1));                                                  // |. replaced opcode
    asm_.append(PowerPcAsm::b(routineStartAddress, asm_.count(), returnAddr));                 // |. return
    return asm_;
}

QVector<quint32> MutatorShopPriceMultiplier::writeUnownedShopPriceMultiplier(const AddressMapper &addressMapper, quint32 routineStartAddress) {
    auto getMutatorDataSubroutine = addressMapper.boomStreetToStandard(0x80412c8c);

    QVector<quint32> asm_;
    asm_.append(PowerPcAsm::mr(31, 3));                                                        // |. save place pointer
    asm_.append(PowerPcAsm::li(3, ShopPriceMultiplierType));                                   // \.
    asm_.append(PowerPcAsm::mflr(6));                                                          // |.
    asm_.append(PowerPcAsm::bl(routineStartAddress, asm_.count(), getMutatorDataSubroutine));  // |. get mutatorData
    asm_.append(PowerPcAsm::mtlr(6));                                                          // |.
    asm_.append(PowerPcAsm::cmpwi(3, 0));                                                      // \. if mutator != NULL
    asm_.append(PowerPcAsm::bne(2));                                                           // /.   goto mutator
                // vanilla
    asm_.append(PowerPcAsm::blr());                                                            // |. return
                // mutator stuff
    asm_.append(PowerPcAsm::lwz(4, 0x40, 31));                                                 // |. r4 <- calculated shop price
    asm_.append(PowerPcAsm::lhz(5, 0x0, 3));                                                   // |. r5 <- numerator
    asm_.append(PowerPcAsm::mullw(4, 4, 5));                                                   // |. r4 <- r4 * r5
    asm_.append(PowerPcAsm::lhz(5, 0x2, 3));                                                   // |. r5 <- denominator
    asm_.append(PowerPcAsm::divwu(4, 4, 5));                                                   // |. r4 <- r4 / r5
    asm_.append(PowerPcAsm::stw(4, 0x40, 31));                                                 // |. calculated shop price <- r4
    asm_.append(PowerPcAsm::blr());                                                            // |. return
    return asm_;
}

void MutatorShopPriceMultiplier::readAsm(QDataStream &, const AddressMapper &, QVector<MapDescriptor> &) { /* crab nothing to do crab */ }

