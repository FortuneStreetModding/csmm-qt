#include "venturecardtable.h"
#include "lib/powerpcasm.h"

quint32 VentureCardTable::writeTable(const std::vector<MapDescriptor> &descriptors) {
    QByteArray ventureCardCompressedTable;
    for (auto &descriptor: descriptors) {
        // here we bitpack the venture card table so that each byte stores 8 venture cards. This results
        // in an array of just 16 bytes for each map.
        quint32 i = 0;
        while (i < descriptor.ventureCards.size()) {
            quint8 bitPackedVentureCardValue = 0;
            for (int j = 7; j >= 0; j--, i++) {
                if (i < descriptor.ventureCards.size()) {
                    bitPackedVentureCardValue |= (quint8)(descriptor.ventureCards[i] << j);
                }
            }
            ventureCardCompressedTable.append(bitPackedVentureCardValue);
        }
    }
    return allocate(ventureCardCompressedTable, "VentureCardCompressedTable");
}

/// <summary>
/// Hijack the LoadBoard() routine. Intercept the moment when the (now compressed) ventureCardTable is passed on to the InitChanceBoard() routine.
/// Call the decompressVentureCardTable routine and pass the resulting decompressed ventureCardTable (located at ventureCardDecompressedTableAddr) to the InitChanceBoard() routine instead.
/// </summary>
void VentureCardTable::writeAsm(QDataStream &stream, const AddressMapper &addressMapper, const std::vector<MapDescriptor> &mapDescriptors) {
    int tableRowCount = mapDescriptors.size();
    quint32 ventureCardCompressedTableAddr = writeTable(mapDescriptors);
    PowerPcAsm::Pair16Bit v = PowerPcAsm::make16bitValuePair(ventureCardCompressedTableAddr);

    // Allocate working memory space for a single uncompressed venture card table which is passed on for the game to use. We will use it to store the result of decompressing a compressed venture card table
    quint32 ventureCardDecompressedTableAddr = allocate(QByteArray(130, '\0'), "VentureCardReservedMemoryForDecompressedTable", false);
    quint32 ventureCardDecompressTableRoutine = allocate(writeSubroutine(ventureCardDecompressedTableAddr), "DecompressVentureCardSubroutine");

    // cmplwi r24,0x29                                     -> cmplwi r24,ventureCardTableCount-1
    stream.device()->seek(addressMapper.boomToFileAddress(0x8007e104)); stream << PowerPcAsm::cmplwi(24, (quint16)(tableRowCount - 1));
    // mulli r0,r24,0x82                                   -> mulli r0,r24,0x10
    stream.device()->seek(addressMapper.boomToFileAddress(0x8007e11c)); stream << PowerPcAsm::mulli(0, 24, 0x10);
    // r4 <- 0x80410648                                    -> r4 <- ventureCardCompressedTableAddr
    stream.device()->seek(addressMapper.boomToFileAddress(0x8007e120)); stream << PowerPcAsm::lis(4, v.upper);
    stream.skipRawData(4); stream << PowerPcAsm::addi(4, 4, v.lower);
    // li r5,0x0                                           -> bl ventureCardDecompressTableRoutine
    stream.device()->seek(addressMapper.boomToFileAddress(0x8007e130));
    stream << PowerPcAsm::bl(addressMapper.boomStreetToStandard(0x8007e130), ventureCardDecompressTableRoutine);
}

void VentureCardTable::readAsm(QDataStream &stream, std::vector<MapDescriptor> &mapDescriptors, const AddressMapper &, bool isVanilla) {
    if (isVanilla) {
        readVanillaVentureCardTable(stream, mapDescriptors);
    } else {
        readCompressedVentureCardTable(stream, mapDescriptors);
    }
}

quint32 VentureCardTable::readTableAddr(QDataStream &stream, const AddressMapper &addressMapper, bool) {
    stream.device()->seek(addressMapper.boomToFileAddress(0x8007e120));
    quint32 lisOpcode, dummy, addiOpcode;
    stream >> lisOpcode >> dummy >> addiOpcode;
    return PowerPcAsm::make32bitValueFromPair(lisOpcode, addiOpcode);
}

bool VentureCardTable::readIsVanilla(QDataStream &stream, const AddressMapper &addressMapper) {
    stream.device()->seek(addressMapper.boomToFileAddress(0x8007e130));
    quint32 opcode; stream >> opcode;
    return opcode == PowerPcAsm::li(5, 0);
}


/// <summary>
/// Write the subroutine which takes a compressed venture card table as input and writes it into ventureCardDecompressedTableAddr
/// </summary>
/// <param name="ventureCardDecompressedTableAddr">The address for the reserved memory space to store the decompressed venture card table in</param>
QVector<quint32> VentureCardTable::writeSubroutine(quint32 ventureCardDecompressedTableAddr) {
    QVector<quint32> asm_;
    auto labels = PowerPcAsm::LabelTable();
    PowerPcAsm::Pair16Bit ventureCardTableAddrPair = PowerPcAsm::make16bitValuePair(ventureCardDecompressedTableAddr);
    ///
    /// assume:
    /// r6 = ventureCardCompressedTableAddr
    ///
    /// variables:
    /// r4 = ventureCardId
    /// r5 = ventureCardCompressedTableAddr
    /// r6 = ventureCardDecompressedTableAddr
    /// r7 = ventureCardCompressedWord
    /// r8 = bitIndex
    /// r0 = tmp / currentByte
    ///
    /// return:
    /// r6 = ventureCardDecompressedTableAddr
    ///
    asm_.append(PowerPcAsm::li(4, 0));                                                      // ventureCardId = 0
    asm_.append(PowerPcAsm::mr(5, 6));                                                      // r6 is ventureCardCompressedTableAddr at this point. Copy it to r5 with which we will be working
    asm_.append(PowerPcAsm::lis(6, ventureCardTableAddrPair.upper));                        // \ load the ventureCardDecompressedTableAddr into r6. This address is
    asm_.append(PowerPcAsm::addi(6, 6, ventureCardTableAddrPair.lower));                    // /  where we will store the decompressed venture card table.
    labels.define("whileVentureCardIdSmaller128", asm_);                                    // do {
    {                                                                                       //
        asm_.append(PowerPcAsm::li(0, 0));                                                  //     \ load the next compressed word from ventureCardCompressedTableAddr
        asm_.append(PowerPcAsm::lwzx(7, 5, 0));                                             //     /  into r7. We will decompress the venture card table word by word.
        asm_.append(PowerPcAsm::li(8, 31));                                                 //     bitIndex = 31
        labels.define("whileBitIndexGreaterEqual32", asm_);                                 //     do
        {                                                                                   //     {
            asm_.append(PowerPcAsm::mr(0, 7));                                              //         get the current compressed word
            asm_.append(PowerPcAsm::srw(0, 0, 8));                                          //         shift it bitIndex times to the right
            asm_.append(PowerPcAsm::andi(0, 0, 1));                                         //         retrieve the lowest bit of it -> r0 contains the decompressed venture card byte now.
            asm_.append(PowerPcAsm::stbx(0, 4, 6));                                         //         store it into ventureCardDecompressedTableAddr[ventureCardId]
            asm_.append(PowerPcAsm::subi(8, 8, 1));                                         //         bitIndex--
            asm_.append(PowerPcAsm::addi(4, 4, 1));                                         //         ventureCardId++
            asm_.append(PowerPcAsm::cmpwi(8, 0));                                           //
            asm_.append(PowerPcAsm::bge(labels, "whileBitIndexGreaterEqual32", asm_));      //     } while(bitIndex >= 0)
        }                                                                                   //
        asm_.append(PowerPcAsm::addi(5, 5, 4));                                             //     ventureCardCompressedTableAddr += 4
        asm_.append(PowerPcAsm::cmpwi(4, 128));                                             //
        asm_.append(PowerPcAsm::blt(labels, "whileVentureCardIdSmaller128", asm_));         // } while(ventureCardId < 128)
    }                                                                                       //
    asm_.append(PowerPcAsm::li(4, 0));                                                      // \ reset r4 = 0
    asm_.append(PowerPcAsm::li(5, 0));                                                      // / reset r5 = 0
    asm_.append(PowerPcAsm::blr());                                                         // return

    labels.checkProperlyLinked();
    return asm_;
}

void VentureCardTable::readVanillaVentureCardTable(QDataStream &stream, std::vector<MapDescriptor> &mapDescriptors) {
    for (int i = 0; i < 42; i++) {
        for (int j = 0; j < 128; j++) {
            quint8 val;
            stream >> val;
            mapDescriptors[i].ventureCards[j] = val;
        }
        // discard the last two bytes
        stream.skipRawData(2);
    }
}

void VentureCardTable::readCompressedVentureCardTable(QDataStream &stream, std::vector<MapDescriptor> &mapDescriptors) {
    for (auto &mapDescriptor: mapDescriptors) {
        quint32 i = 0;
        while (i < mapDescriptor.ventureCards.size()) {
            quint8 bitPackedVentureCardValue;
            stream >> bitPackedVentureCardValue;
            for (int j = 7; j >= 0; j--, i++) {
                if (i < mapDescriptor.ventureCards.size()) {
                    mapDescriptor.ventureCards[i] = (bool)((bitPackedVentureCardValue >> j) & 1);
                }
            }
        }
    }
}
