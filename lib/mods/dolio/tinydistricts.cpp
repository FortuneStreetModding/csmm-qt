#include "tinydistricts.h"
#include "lib/powerpcasm.h"

/*
 * Vanilla:
 *
 * .data5:80412B00  .short     0,0x100,0x200,0x3C0,    0,    0,    0,    0,    0
 * .data5:80412B12  .short     0,0x100,0x140,0x280,0x500,    0,    0,    0,    0
 * .data5:80412B24  .short     0,0x100,0x140,0x200,0x340,0x600,    0,    0,    0
 * .data5:80412B36  .short     0,0x100,0x140,0x200,0x2C0,0x440,0x6C0,    0,    0
 * .data5:80412B48  .short     0,0x100,0x140,0x1C0,0x2C0,0x3C0,0x540,0x780,    0
 * .data5:80412B5A  .short     0,0x100,0x140,0x1C0,0x280,0x380,0x480,0x600,0x800
 */
static const quint16 PRICE_MULTIPLIER_TABLE[8*9] = {
    0, 0x100,     0,     0,     0,     0,     0,     0,     0,
    0, 0x100, 0x200,     0,     0,     0,     0,     0,     0,
    0, 0x100, 0x180, 0x3C0,     0,     0,     0,     0,     0,
    0, 0x100, 0x140, 0x280, 0x500,     0,     0,     0,     0,
    0, 0x100, 0x140, 0x200, 0x340, 0x600,     0,     0,     0,
    0, 0x100, 0x140, 0x200, 0x2C0, 0x440, 0x6C0,     0,     0,
    0, 0x100, 0x140, 0x1C0, 0x2C0, 0x3C0, 0x540, 0x780,     0,
    0, 0x100, 0x140, 0x1C0, 0x280, 0x380, 0x480, 0x600, 0x800
};

/*
 * Vanilla:
 *
 * .data5:80412B6C  .short      0, 0x200, 0x300, 0x600,     0,     0,     0,     0,     0
 * .data5:80412B7E  .short      0, 0x180, 0x200, 0x400, 0xA00,     0,     0,     0,     0
 * .data5:80412B90  .short      0, 0x180, 0x200, 0x400, 0xA00, 0xC00,     0,     0,     0
 * .data5:80412BA2  .short      0, 0x180, 0x200, 0x400, 0xA00, 0xC00 ,0xE00,     0,     0
 * .data5:80412BB4  .short      0, 0x180, 0x200, 0x400, 0xA00, 0xC00, 0xE00,0x1000,     0
 * .data5:80412BC6  .short      0, 0x180, 0x200, 0x400, 0xA00, 0xC00, 0xE00,0x1000,0x1300
 */
static const quint16 VALUE_MULTIPLIER_TABLE[8*9] = {
    0, 0x200,     0,     0,     0,     0,     0,     0,     0,
    0, 0x180, 0x300,     0,     0,     0,     0,     0,     0,
    0, 0x180, 0x240, 0x600,     0,     0,     0,     0,     0,
    0, 0x180, 0x200, 0x400, 0xA00,     0,     0,     0,     0,
    0, 0x180, 0x200, 0x400, 0xA00, 0xC00,     0,     0,     0,
    0, 0x180, 0x200, 0x400, 0xA00, 0xC00 ,0xE00,     0,     0,
    0, 0x180, 0x200, 0x400, 0xA00, 0xC00, 0xE00,0x1000,     0,
    0, 0x180, 0x200, 0x400, 0xA00, 0xC00, 0xE00,0x1000,0x1300
};

void TinyDistricts::writeAsm(QDataStream &stream, const AddressMapper &addressMapper, const std::vector<MapDescriptor> &) {
    QVector<quint16> table;
    for (const auto & v : PRICE_MULTIPLIER_TABLE) {
        table.append(v);
    }
    quint32 priceMultTableAddr = allocate(table, "PRICE_MULTIPLIER_TABLE");
    table.clear();
    for (const auto & v : VALUE_MULTIPLIER_TABLE) {
        table.append(v);
    }
    quint32 valueMultTableAddr = allocate(table, "VALUE_MULTIPLIER_TABLE");

    // --- Replace PRICE_MULTIPLIER_TABLE ---
    PowerPcAsm::Pair16Bit v = PowerPcAsm::make16bitValuePair(priceMultTableAddr);
    // r5 <- 0x80412c88  ->  r5 <- priceMultTableAddr
    stream.device()->seek(addressMapper.boomToFileAddress(0x80092424)); stream << PowerPcAsm::lis(5, v.upper);
                                                                        stream << PowerPcAsm::addi(5, 5, v.lower);
    // subi r0,r6,0x3    ->  subi r0,r6,0x1
    stream.device()->seek(addressMapper.boomToFileAddress(0x80092430)); stream << PowerPcAsm::subi(0, 6, 0x01);

    // --- Replace VALUE_MULTIPLIER_TABLE ---
    PowerPcAsm::Pair16Bit v2 = PowerPcAsm::make16bitValuePair(valueMultTableAddr);
    // r5 <- 0x80412cf4  ->  r5 <- priceMultTableAddr
    stream.device()->seek(addressMapper.boomToFileAddress(0x80092448)); stream << PowerPcAsm::lis(5, v2.upper);
                                                                        stream << PowerPcAsm::addi(5, 5, v2.lower);
    // subi r0,r6,0x3    ->  subi r0,r6,0x1
    stream.device()->seek(addressMapper.boomToFileAddress(0x80092454)); stream << PowerPcAsm::subi(0, 6, 0x01);

    // --- Do not play domination theme when there is only a single shop in the district ---
    quint32 hijackAddr = addressMapper.boomStreetToStandard(0x801c473c);
    quint32 procSingleShopDistrictCheckRoutine = allocate(writeSingleShopDistrictCheckRoutine(addressMapper, 0), "procSingleShopDistrictCheckRoutine");
    stream.device()->seek(addressMapper.toFileAddress(procSingleShopDistrictCheckRoutine));
    auto routineCode = writeSingleShopDistrictCheckRoutine(addressMapper, procSingleShopDistrictCheckRoutine);
    for (quint32 inst: std::as_const(routineCode)) stream << inst; // re-write the routine again since now we know where it is located in the main dol
    stream.device()->seek(addressMapper.toFileAddress(hijackAddr));
    // cmpw r0,r3       ->  b procSingleShopDistrictCheckRoutine
    stream << PowerPcAsm::b(hijackAddr, procSingleShopDistrictCheckRoutine);
}

QVector<quint32> TinyDistricts::writeSingleShopDistrictCheckRoutine(const AddressMapper &addressMapper, quint32 routineStartAddress) {
    // precondition: r0 - total shops in district
    //               r3 - shops owned by player
    auto playDominationThemeReturnAddr = addressMapper.boomStreetToStandard(0x801c474c);
    auto returnAddr = addressMapper.boomStreetToStandard(0x801c4744);

    QVector<quint32> asm_;
    auto labels = PowerPcAsm::LabelTable();
    asm_.append(PowerPcAsm::cmpwi(0, 1));
    asm_.append(PowerPcAsm::beq(labels, "doNotPlayDomination", asm_));
    asm_.append(PowerPcAsm::cmpw(0, 3));
    asm_.append(PowerPcAsm::ble(labels, "playDomination", asm_));
    labels.define("doNotPlayDomination", asm_);
    asm_.append(PowerPcAsm::b(routineStartAddress, asm_.size(), returnAddr));
    labels.define("playDomination", asm_);
    asm_.append(PowerPcAsm::b(routineStartAddress, asm_.size(), playDominationThemeReturnAddr));
    labels.checkProperlyLinked();
    return asm_;
}


void TinyDistricts::readAsm(QDataStream &, const AddressMapper &, std::vector<MapDescriptor> &) { /* crab nothing to do crab */ }
