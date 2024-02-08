#include "mutatorshopprice.h"
#include "lib/powerpcasm.h"
#include "lib/mutator/mutator.h"

void MutatorShopPrice::writeAsm(QDataStream &stream, const AddressMapper &addressMapper, const std::vector<MapDescriptor> &) {
    quint32 getMutatorDataSubroutine = mutatorTableRoutineAddr(modList());

    // --- Base Shop Price Multiplier ---
    {
        quint32 hijackAddr = addressMapper.boomStreetToStandard(0x8008ee8c);
        quint32 procShopPriceMultiplier = allocate(writeBaseShopPriceMultiplier(addressMapper, 0, getMutatorDataSubroutine), "procShopPriceMultiplier");
        stream.device()->seek(addressMapper.toFileAddress(procShopPriceMultiplier));
        auto routineCode = writeBaseShopPriceMultiplier(addressMapper, procShopPriceMultiplier, getMutatorDataSubroutine);
        for (quint32 inst: std::as_const(routineCode)) stream << inst; // re-write the routine again since now we know where it is located in the main dol
        stream.device()->seek(addressMapper.toFileAddress(hijackAddr));
        // b LAB_8008eea0             ->  b procShopPriceMultiplier
        stream << PowerPcAsm::b(hijackAddr, procShopPriceMultiplier);
    }

    // --- 3 Star Hotel Base Shop Price Multiplier ---
    {
        quint32 hijackAddr = addressMapper.boomStreetToStandard(0x8008f1f8);
        quint32 proc3StarHotelPriceMultiplier = allocate(write3StarHotelPriceMultiplier(addressMapper, 0, getMutatorDataSubroutine), "proc3StarHotelPriceMultiplier");
        stream.device()->seek(addressMapper.toFileAddress(proc3StarHotelPriceMultiplier));
        auto routineCode = write3StarHotelPriceMultiplier(addressMapper, proc3StarHotelPriceMultiplier, getMutatorDataSubroutine);
        for (quint32 inst: std::as_const(routineCode)) stream << inst; // re-write the routine again since now we know where it is located in the main dol
        stream.device()->seek(addressMapper.toFileAddress(hijackAddr));
        // bl Gm_Place_CalcRank        ->  b proc3StarHotelPriceMultiplier
        stream << PowerPcAsm::b(hijackAddr, proc3StarHotelPriceMultiplier);
    }

    // --- Calc Rank Multiplier ---
    {
        quint32 hijackAddr = addressMapper.boomStreetToStandard(0x8008eb14);
        quint32 procRankPriceMultiplier = allocate(writeRankPriceMultiplier(addressMapper, 0, getMutatorDataSubroutine), "procRankPriceMultiplier");
        stream.device()->seek(addressMapper.toFileAddress(procRankPriceMultiplier));
        auto routineCode = writeRankPriceMultiplier(addressMapper, procRankPriceMultiplier, getMutatorDataSubroutine);
        for (quint32 inst: std::as_const(routineCode)) stream << inst; // re-write the routine again since now we know where it is located in the main dol
        stream.device()->seek(addressMapper.toFileAddress(hijackAddr));
        // lis r3, 0x8045        ->  b procRankPriceMultiplier
        stream << PowerPcAsm::b(hijackAddr, procRankPriceMultiplier);
    }
}

QVector<quint32> MutatorShopPrice::writeBaseShopPriceMultiplier(const AddressMapper &addressMapper, quint32 routineStartAddress, quint32 getMutatorDataSubroutine) {
    // precondition: r29 - Place*
    //                r0 - shop price
    // postcondition: r0 - dont care
    //                r3 - dont care
    //                r4 - dont care
    auto returnAddr = addressMapper.boomStreetToStandard(0x8008eea0);

    QVector<quint32> asm_;
    auto labels = PowerPcAsm::LabelTable();
    asm_.append(PowerPcAsm::li(3, ShopPriceType));                                             // \.
    asm_.append(PowerPcAsm::bl(routineStartAddress, asm_.count(), getMutatorDataSubroutine));  // /. get mutatorData
    asm_.append(PowerPcAsm::cmpwi(3, 0));                                                      // \. if mutator != NULL
    asm_.append(PowerPcAsm::bne(labels, "mutator", asm_));                                     // /.   goto mutator
                // vanilla
    asm_.append(PowerPcAsm::b(routineStartAddress, asm_.count(), returnAddr));                 // |. return
                // mutator stuff
    labels.define("mutator", asm_);
    asm_.append(PowerPcAsm::lhz(5, 0x0, 3));                                                   // |. r5 <- numerator
    asm_.append(PowerPcAsm::mullw(0, 0, 5));                                                   // |. r0 <- r0 * r5
    asm_.append(PowerPcAsm::lhz(5, 0x2, 3));                                                   // |. r5 <- denominator
    asm_.append(PowerPcAsm::divwu(0, 0, 5));                                                   // |. r0 <- r0 / r5
    asm_.append(PowerPcAsm::lha(5, 0x4, 3));                                                   // |. r5 <- constant
    asm_.append(PowerPcAsm::add(0, 5, 0));                                                     // |. r0 <- r4 + r0
    asm_.append(PowerPcAsm::stw(0, 0x28, 29));                                                 // |. base shop price <- r0
    asm_.append(PowerPcAsm::b(routineStartAddress, asm_.count(), returnAddr));                 // |. return
    labels.checkProperlyLinked();
    return asm_;
}

QVector<quint32> MutatorShopPrice::write3StarHotelPriceMultiplier(const AddressMapper &addressMapper, quint32 routineStartAddress, quint32 getMutatorDataSubroutine) {
    // precondition:  r4 <- shop price = 200?
    //               r29 <- Place*
    // postcondition: r0 <- dont care
    //                r3 <- Place*
    //                r4 <- shop price
    //                r5 <- dont care
    //                r6 <- dont care
    //                r7 <- dont care
    // exitcondition: bl Gm_Place_CalcRank

    auto Gm_Place_CalcRank = addressMapper.boomStreetToStandard(0x8008ea9c);
    auto returnAddr = addressMapper.boomStreetToStandard(0x8008f1fc);


    QVector<quint32> asm_;
    auto labels = PowerPcAsm::LabelTable();
    asm_.append(PowerPcAsm::mr(6, 4));                                                         // |. save shop price
    asm_.append(PowerPcAsm::li(3, ShopPriceType));                                             // \.
    asm_.append(PowerPcAsm::bl(routineStartAddress, asm_.count(), getMutatorDataSubroutine));  // /. get mutatorData
    asm_.append(PowerPcAsm::mr(4, 6));                                                         // |. restore shop price
    asm_.append(PowerPcAsm::cmpwi(3, 0));                                                      // \. if mutator == NULL
    asm_.append(PowerPcAsm::beq(labels, "exit", asm_));                                        // /.   goto exit
                // mutator stuff
    asm_.append(PowerPcAsm::lhz(6, 0x0, 3));                                                   // |. r6 <- numerator
    asm_.append(PowerPcAsm::mullw(4, 4, 6));                                                   // |. r4 <- r4 * r6
    asm_.append(PowerPcAsm::lhz(6, 0x2, 3));                                                   // |. r7 <- denominator
    asm_.append(PowerPcAsm::divwu(4, 4, 6));                                                   // |. r4 <- r4 / r6
    asm_.append(PowerPcAsm::lha(6, 0x4, 3));                                                   // |. r6 <- constant
    asm_.append(PowerPcAsm::add(4, 6, 4));                                                     // |. r4 <- r4 + r6
    asm_.append(PowerPcAsm::stw(4, 0x28, 29));                                                 // |. base shop price <- r4
    asm_.append(PowerPcAsm::stw(4, 0x40, 29));                                                 // |. calculated shop price <- r4
                // exit
    labels.define("exit", asm_);
    asm_.append(PowerPcAsm::mr(3, 29));                                                        // |. restore Place*
    asm_.append(PowerPcAsm::bl(routineStartAddress, asm_.count(), Gm_Place_CalcRank));         // |. bl Gm_Place_CalcRank
    asm_.append(PowerPcAsm::b(routineStartAddress, asm_.count(), returnAddr));                 // |. return
    labels.checkProperlyLinked();
    return asm_;
}

QVector<quint32> MutatorShopPrice::writeRankPriceMultiplier(const AddressMapper &addressMapper, quint32 routineStartAddress, quint32 getMutatorDataSubroutine) {
    // precondition:  r4 <- shop price
    // postcondition: r0 <- (r3)
    //                r3 <- 80453b38
    //                r4 <- shop price
    //                r5 <- dont care
    //                r6 <- dont care
    //                r7 <- dont care
    auto returnAddr = addressMapper.boomStreetToStandard(0x8008eb1c);
    auto tableAddr = PowerPcAsm::make16bitValuePair(addressMapper.boomStreetToStandard(0x80453b38));

    QVector<quint32> asm_;
    auto labels = PowerPcAsm::LabelTable();
    asm_.append(PowerPcAsm::mr(6, 4));                                                         // |. save shop price
    asm_.append(PowerPcAsm::li(3, ShopPriceType));                                             // \.
    asm_.append(PowerPcAsm::mflr(7));                                                          // |.
    asm_.append(PowerPcAsm::bl(routineStartAddress, asm_.count(), getMutatorDataSubroutine));  // |. get mutatorData
    asm_.append(PowerPcAsm::mtlr(7));                                                          // /.
    asm_.append(PowerPcAsm::mr(4, 6));                                                         // |. restore shop price
    asm_.append(PowerPcAsm::cmpwi(3, 0));                                                      // \. if mutator != NULL
    asm_.append(PowerPcAsm::bne(labels, "mutator", asm_));                                     // /.   goto mutator
                // vanilla
    labels.define("vanilla", asm_);
    asm_.append(PowerPcAsm::lis(3, tableAddr.upper));                                          // \. replaced opcodes
    asm_.append(PowerPcAsm::lwzu(0, tableAddr.lower, 3));                                      // /.
    asm_.append(PowerPcAsm::b(routineStartAddress, asm_.count(), returnAddr));                 // |. return
                // mutator stuff
    labels.define("mutator", asm_);
    asm_.append(PowerPcAsm::lbz(7, 0x6, 3));                                                   // |. r7 <- affectRank
    asm_.append(PowerPcAsm::cmpwi(7, 0x0));
    asm_.append(PowerPcAsm::beq(labels, "vanilla", asm_));

    asm_.append(PowerPcAsm::lha(7, 0x4, 3));                                                   // |. r7 <- constant
    asm_.append(PowerPcAsm::subf(4, 7, 4));                                                    // |. r4 <- r4 - r7
    asm_.append(PowerPcAsm::lhz(7, 0x0, 3));
    asm_.append(PowerPcAsm::cmpwi(7, 0x1));
    asm_.append(PowerPcAsm::beq(labels, "noRoundErrorFix", asm_));
    asm_.append(PowerPcAsm::addi(4, 4, 1));                                                    // |. r4 <- r4 + 1 (to account for rounding errors)

    labels.define("noRoundErrorFix", asm_);
    asm_.append(PowerPcAsm::lhz(7, 0x2, 3));                                                   // |. r7 <- denominator
    asm_.append(PowerPcAsm::mullw(4, 4, 7));                                                   // |. r4 <- r4 * r7
    asm_.append(PowerPcAsm::lhz(7, 0x0, 3));                                                   // |. r7 <- numerator
    asm_.append(PowerPcAsm::divwu(4, 4, 7));                                                   // |. r4 <- r4 / r7
    asm_.append(PowerPcAsm::lis(3, tableAddr.upper));                                          // \. replaced opcodes
    asm_.append(PowerPcAsm::lwzu(0, tableAddr.lower, 3));                                      // /.
    asm_.append(PowerPcAsm::b(routineStartAddress, asm_.count(), returnAddr));                 // |. return
    labels.checkProperlyLinked();
    return asm_;
}

void MutatorShopPrice::readAsm(QDataStream &, const AddressMapper &, std::vector<MapDescriptor> &) { /* crab nothing to do crab */ }

