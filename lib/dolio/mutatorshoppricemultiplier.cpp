#include "mutatorshoppricemultiplier.h"
#include "lib/powerpcasm.h"
#include "lib/mutator/mutator.h"

void MutatorShopPriceMultiplier::writeAsm(QDataStream &stream, const AddressMapper &addressMapper, const QVector<MapDescriptor> &) {
    // --- Owned Shop Price Multiplier ---
    {
        quint32 hijackAddr = addressMapper.boomStreetToStandard(0x8008ff18);
        quint32 procShopPriceMultiplier = allocate(writeShopPriceMultiplier(addressMapper, 0), "procShopPriceMultiplier");
        stream.device()->seek(addressMapper.toFileAddress(procShopPriceMultiplier));
        auto routineCode = writeShopPriceMultiplier(addressMapper, procShopPriceMultiplier);
        for (quint32 inst: qAsConst(routineCode)) stream << inst; // re-write the routine again since now we know where it is located in the main dol
        stream.device()->seek(addressMapper.toFileAddress(hijackAddr));
        // lwz r30,0x18(r1)           ->  b procShopPriceMultiplier
        stream << PowerPcAsm::b(hijackAddr, procShopPriceMultiplier);
    }

    // --- Unowned Shop Price Multiplier ---
   /* {
        quint32 hijackAddr = addressMapper.boomStreetToStandard(0x8008f79c);
        quint32 procUnownedShopPriceMultiplier = allocate(writeUnownedShopPriceMultiplier(addressMapper, 0), "procUnownedShopPriceMultiplier");
        stream.device()->seek(addressMapper.toFileAddress(procUnownedShopPriceMultiplier));
        auto routineCode = writeUnownedShopPriceMultiplier(addressMapper, procUnownedShopPriceMultiplier);
        for (quint32 inst: qAsConst(routineCode)) stream << inst; // re-write the routine again since now we know where it is located in the main dol
        stream.device()->seek(addressMapper.toFileAddress(hijackAddr));
        // stw r4,0x40(r31)             ->  b procShopPriceMultiplier
        stream << PowerPcAsm::b(hijackAddr, procUnownedShopPriceMultiplier);
    }*/

}

QVector<quint32> MutatorShopPriceMultiplier::writeShopPriceMultiplier(const AddressMapper &addressMapper, quint32 routineStartAddress) {
    // precondition:  r3 - shop price
    // postcondition: r3 - shop price

    auto getMutatorDataSubroutine = addressMapper.boomStreetToStandard(0x80412c8c);
    auto returnAddr = addressMapper.boomStreetToStandard(0x8008ff1c);

    QVector<quint32> asm_;
    asm_.append(PowerPcAsm::mr(30, 3));                                                        // |. remember shop price
    asm_.append(PowerPcAsm::li(3, ShopPriceMultiplierType));                                   // \.
    asm_.append(PowerPcAsm::bl(routineStartAddress, asm_.count(), getMutatorDataSubroutine));  // /. get mutatorData
    asm_.append(PowerPcAsm::cmpwi(3, 0));                                                      // \. if mutator != NULL
    asm_.append(PowerPcAsm::bne(4));                                                           // /.   goto mutator
                // vanilla
    asm_.append(PowerPcAsm::mr(3, 30));                                                        // |. restore shop price
    asm_.append(PowerPcAsm::lwz(30, 0x18, 1));                                                 // |. replaced opcode
    asm_.append(PowerPcAsm::b(routineStartAddress, asm_.count(), returnAddr));                 // |. return
                // mutator stuff
    asm_.append(PowerPcAsm::mr(4, 30));                                                        // |. restore shop price
    asm_.append(PowerPcAsm::lhz(29, 0x0, 3));                                                  // |. r29 <- numerator
    asm_.append(PowerPcAsm::mullw(4, 4, 29));                                                  // |. r4 <- r4 * r29
    asm_.append(PowerPcAsm::lhz(29, 0x2, 3));                                                  // |. r29 <- denominator
    asm_.append(PowerPcAsm::divwu(4, 4, 29));                                                  // |. r4 <- r4 / r29
    asm_.append(PowerPcAsm::mr(3, 4));                                                         // |. r3 <- r4
    asm_.append(PowerPcAsm::lwz(30, 0x18, 1));                                                 // |. replaced opcode
    asm_.append(PowerPcAsm::b(routineStartAddress, asm_.count(), returnAddr));                 // |. return
    return asm_;
}

QVector<quint32> MutatorShopPriceMultiplier::writeUnownedShopPriceMultiplier(const AddressMapper &addressMapper, quint32 routineStartAddress) {
    // precondition:  r4 - unowned shop price
    //                r5 - owner
    // postcondition: r3 - dont care
    //                r4 - unowned shop price
    //                r5 - dont care
    //                r6 - dont care

    auto getMutatorDataSubroutine = addressMapper.boomStreetToStandard(0x80412c8c);
    auto returnAddr = addressMapper.boomStreetToStandard(0x8008f7a0);

    QVector<quint32> asm_;
    asm_.append(PowerPcAsm::mr(6, 4));                                                         // |. r6 <- remember shop price
    asm_.append(PowerPcAsm::li(3, ShopPriceMultiplierType));                                   // \.
    asm_.append(PowerPcAsm::bl(routineStartAddress, asm_.count(), getMutatorDataSubroutine));  // /. get mutatorData
    asm_.append(PowerPcAsm::cmpwi(3, 0));                                                      // \. if mutator != NULL
    asm_.append(PowerPcAsm::bne(4));                                                           // /.   goto mutator
                // vanilla
    asm_.append(PowerPcAsm::mr(4, 6));                                                         // |. r4 <- restore shop price
    asm_.append(PowerPcAsm::stw(4, 0x40, 31));                                                 // |. replaced opcode
    asm_.append(PowerPcAsm::b(routineStartAddress, asm_.count(), returnAddr));                 // |. return
                // mutator stuff
    asm_.append(PowerPcAsm::mr(4, 6));                                                         // |. r4 <- restore shop price
    asm_.append(PowerPcAsm::lhz(5, 0x0, 3));                                                   // |. r5 <- numerator
    asm_.append(PowerPcAsm::mullw(4, 4, 5));                                                   // |. r4 <- r4 * r5
    asm_.append(PowerPcAsm::lhz(5, 0x2, 3));                                                   // |. r5 <- denominator
    asm_.append(PowerPcAsm::divwu(4, 4, 5));                                                   // |. r4 <- r4 / r5
    asm_.append(PowerPcAsm::stw(4, 0x40, 31));                                                 // |. replaced opcode
    asm_.append(PowerPcAsm::b(routineStartAddress, asm_.count(), returnAddr));                 // |. return
    return asm_;
}

void MutatorShopPriceMultiplier::readAsm(QDataStream &, const AddressMapper &, QVector<MapDescriptor> &) { /* crab nothing to do crab */ }

