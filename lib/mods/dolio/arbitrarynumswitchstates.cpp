#include "arbitrarynumswitchstates.h"
#include "lib/powerpcasm.h"

void ArbitraryNumSwitchStates::readAsm(QDataStream &stream, const AddressMapper &addressMapper, std::vector<MapDescriptor> &mapDescriptors)
{
    // crab nothing to do crab
}

void ArbitraryNumSwitchStates::writeAsm(QDataStream &stream, const AddressMapper &addressMapper, const std::vector<MapDescriptor> &mapDescriptors)
{
    support3State(stream, addressMapper, mapDescriptors);

    size_t maxStates = 0;
    for (auto &descriptor: mapDescriptors) {
        maxStates = qMax(maxStates, descriptor.frbFiles.size());
    }

    moveMapDataArray(stream, addressMapper, mapDescriptors, maxStates);
    increaseGameManagerStorage(stream, addressMapper, mapDescriptors, maxStates);
    expandedAnimations(stream, addressMapper, mapDescriptors, maxStates);
    increaseFrbBuffer(stream, addressMapper, mapDescriptors, maxStates);

    // Expand Comparisons to go past 4 states
    stream.device()->seek(addressMapper.boomToFileAddress(0x800cccc8));
    // cmpwi cr1, r17, 0x4 -> cmpwi cr1, r17, 0x7FFF
    stream << PowerPcAsm::cmpwi(17, 0x7FFF, 1);

    stream.device()->seek(addressMapper.boomToFileAddress(0x800cefb4));
    // cmpwi r29, 0x4 -> cmpwi r29, 0x7FFF
    stream << PowerPcAsm::cmpwi(29, 0x7FFF);
}

void ArbitraryNumSwitchStates::support3State(QDataStream &stream, const AddressMapper &addressMapper, const std::vector<MapDescriptor> &mapDescriptors)
{
    // StopSwitch
    stream.device()->seek(addressMapper.boomToFileAddress(0x8007f120));

    // cmpwi r4, 4 -> cmpwi r4, 0
    stream << PowerPcAsm::cmpwi(4, 0);
    stream.skipRawData(4);
    // li r5, 2 -> mr r5, r4
    stream << PowerPcAsm::mr(5, 4);
    stream.skipRawData(4);
    // li r5, 4 -> li r5, 2
    stream << PowerPcAsm::li(5, 2);

    // StopPlace
    stream.device()->seek(addressMapper.boomToFileAddress(0x8007f5a0));
    // li r5, 2 -> mr r5, r0
    stream << PowerPcAsm::mr(5, 0);
    stream.skipRawData(4);
    // cmpwi r0, 4 -> cmpwi r0, 0
    stream << PowerPcAsm::cmpwi(0, 0);
    stream.skipRawData(8);
    // mr r5, r7 -> li r5, 2
    stream << PowerPcAsm::li(5, 2);
}

void ArbitraryNumSwitchStates::moveMapDataArray(QDataStream &stream, const AddressMapper &addressMapper, const std::vector<MapDescriptor> &mapDescriptors, size_t maxStates)
{
    quint32 newMapDataArrayAddr = allocate(QByteArray(4 * maxStates, '\0'), "Map Data Array", false);
    auto newMapDataArrayPair = PowerPcAsm::make16bitValuePair(newMapDataArrayAddr);

    stream.device()->seek(addressMapper.boomToFileAddress(0x8007dde8));
    // r3 <- &s_map_data_array -> r3 <- newMapDataArrayAddr
    stream << PowerPcAsm::lis(3, newMapDataArrayPair.upper);
    stream.skipRawData(4);
    stream << PowerPcAsm::addi(3, 3, newMapDataArrayPair.lower);
    stream.skipRawData(4);
    // li r5, 10 -> li r5, 4*maxStates
    stream << PowerPcAsm::li(5, 4 * maxStates);

    stream.device()->seek(addressMapper.boomToFileAddress(0x8007e048));
    // r3 <- &s_map_data_array -> r3 <- newMapDataArrayAddr
    stream << PowerPcAsm::lis(3, newMapDataArrayPair.upper);
    stream.skipRawData(4);
    stream << PowerPcAsm::addi(3, 3, newMapDataArrayPair.lower);
    stream.skipRawData(4);
    // li r5, 10 -> li r5, 4*maxStates
    stream << PowerPcAsm::li(5, 4 * maxStates);

    stream.device()->seek(addressMapper.boomToFileAddress(0x8007e0d4));
    // r3 <- &s_map_data_array -> r3 <- newMapDataArrayAddr
    stream << PowerPcAsm::lis(28, newMapDataArrayPair.upper);
    stream.skipRawData(4);
    stream << PowerPcAsm::addi(3, 28, newMapDataArrayPair.lower);
    stream.skipRawData(4);
    // li r5, 10 -> li r5, 4*maxStates
    stream << PowerPcAsm::li(5, 4 * maxStates);
    stream.device()->seek(addressMapper.boomToFileAddress(0x8007e108));
    // s_map_data_array <- r0 -> *newMapDataArrayAddr <- r0
    stream << PowerPcAsm::stw(0, newMapDataArrayPair.lower, 28);

    stream.device()->seek(addressMapper.boomToFileAddress(0x8007e480));
    // r3 <- &s_map_data_array -> r3 <- newMapDataArrayAddr
    stream << PowerPcAsm::lis(3, newMapDataArrayPair.upper);
    stream.skipRawData(4);
    stream << PowerPcAsm::addi(3, 3, newMapDataArrayPair.lower);

    stream.device()->seek(addressMapper.boomToFileAddress(0x8007f138));
    // r4 <- &s_map_data_array -> r4 <- newMapDataArrayAddr
    stream << PowerPcAsm::lis(4, newMapDataArrayPair.upper);
    stream.skipRawData(8);
    stream << PowerPcAsm::addi(4, 4, newMapDataArrayPair.lower);

    stream.device()->seek(addressMapper.boomToFileAddress(0x8007f18c));
    // r4 <- &s_map_data_array -> r4 <- newMapDataArrayAddr
    stream << PowerPcAsm::lis(4, newMapDataArrayPair.upper);
    stream.skipRawData(12);
    stream << PowerPcAsm::addi(4, 4, newMapDataArrayPair.lower);

    stream.device()->seek(addressMapper.boomToFileAddress(0x8007f5bc));
    // r4 <- &s_map_data_array -> r4 <- newMapDataArrayAddr
    stream << PowerPcAsm::lis(4, newMapDataArrayPair.upper);
    stream.skipRawData(8);
    stream << PowerPcAsm::addi(4, 4, newMapDataArrayPair.lower);
}

void ArbitraryNumSwitchStates::increaseGameManagerStorage(QDataStream &stream, const AddressMapper &addressMapper, const std::vector<MapDescriptor> &mapDescriptors, size_t maxStates)
{
    // There's a set of fields in GameManager that store some map data for each state.
    // The idea is to enlarge the set of fields to support the desired # of states
    // and move the fields to the end of the expanded GameManager object (as originally,
    // the fields were sandwiched in the middle starting at offset 0x48).

    stream.device()->seek(addressMapper.boomToFileAddress(0x80016e88));
    // below expands the memory allocated for the GameManager
    // li r3, 0x4f8 -> li r3, (0x4f8 + 8*maxStates + 8)
    stream << PowerPcAsm::li(3, 0x4f8 + maxStates * 8 + 8);

    // The following lines shift the fields from starting at 0x48 to starting at 0x4f8.
    stream.device()->seek(addressMapper.boomToFileAddress(0x800ccc64));
    stream << PowerPcAsm::lwz(0, 0x4f8, 4);
    stream.skipRawData(4);
    stream << PowerPcAsm::stw(5, 0x4f8, 4);
    stream.skipRawData(4);
    stream << PowerPcAsm::lwz(3, 0x4fc, 4);
    stream.skipRawData(12);
    stream << PowerPcAsm::stw(5, 0x4fc, 4);

    stream.device()->seek(addressMapper.boomToFileAddress(0x800ccca8));
    stream << PowerPcAsm::lwz(3, 0x4f8, 3);

    stream.device()->seek(addressMapper.boomToFileAddress(0x800cef50));
    stream << PowerPcAsm::lwz(3, 0x4f8, 28);
}

void ArbitraryNumSwitchStates::expandedAnimations(QDataStream &stream, const AddressMapper &addressMapper, const std::vector<MapDescriptor> &mapDescriptors, size_t maxStates)
{
    QVector<quint32> vMoveTable, vStopTable;
    for (int i=1; i<=maxStates; ++i) {
        vMoveTable.append(allocate(QString("V_MOVE%1").arg(i)));
        vStopTable.append(allocate(QString("V_STOP%1").arg(i)));
    }
    quint32 vMoveTableAddr = allocate(vMoveTable, "V_MOVE table");
    quint32 vStopTableAddr = allocate(vStopTable, "V_STOP table");

    auto vMoveTableAddrPair = PowerPcAsm::make16bitValuePair(vMoveTableAddr);
    auto vStopTableAddrPair = PowerPcAsm::make16bitValuePair(vStopTableAddr);

    stream.device()->seek(addressMapper.boomToFileAddress(0x800d2354));
    stream << PowerPcAsm::lis(30, vMoveTableAddrPair.upper);
    stream.skipRawData(4);
    stream << PowerPcAsm::addi(30, 30, vMoveTableAddrPair.lower);

    stream.device()->seek(addressMapper.boomToFileAddress(0x800d23f0));
    stream << PowerPcAsm::lis(30, vStopTableAddrPair.upper);
    stream.skipRawData(4);
    stream << PowerPcAsm::addi(30, 30, vStopTableAddrPair.lower);

    stream.device()->seek(addressMapper.boomToFileAddress(0x800d2514));
    stream << PowerPcAsm::lis(30, vMoveTableAddrPair.upper);
    stream.skipRawData(4);
    stream << PowerPcAsm::addi(30, 30, vMoveTableAddrPair.lower);
}

static constexpr quint32 FRB_BUFFER_SIZE_NEW = 1U << 18;

void ArbitraryNumSwitchStates::increaseFrbBuffer(QDataStream &stream, const AddressMapper &addressMapper, const std::vector<MapDescriptor> &mapDescriptors, size_t maxStates)
{
    using namespace std::placeholders;

    auto oldBufAddrPair = PowerPcAsm::make16bitValuePair(addressMapper.boomStreetToStandard(0x80552758));
    auto value_bufsz = PowerPcAsm::make16bitValuePair(FRB_BUFFER_SIZE_NEW);
    auto operatorNewAddr = addressMapper.boomStreetToStandard(0x800084d8);

    auto getBufferPtrSubroutine = [&](quint32 subroutineAddr, quint32 destAddr, int destReg, int unusedReg) {
        QVector<quint32> asm_;
        auto labels = PowerPcAsm::LabelTable();
        // rU <- 0x80552758
        asm_.append(PowerPcAsm::lis(unusedReg, oldBufAddrPair.upper));
        asm_.append(PowerPcAsm::addi(unusedReg, unusedReg, oldBufAddrPair.lower));
        // rD <- *rU
        asm_.append(PowerPcAsm::lwz(destReg, 0x0, unusedReg));
        // if (rD != 0) goto already_initialized
        asm_.append(PowerPcAsm::cmplwi(destReg, 0));
        asm_.append(PowerPcAsm::bne(labels, "already_initialized", asm_));

        for (int regToSave = 3; regToSave <= 5; ++regToSave) {
            if (regToSave != destReg) {
                asm_.append(PowerPcAsm::stw(regToSave, 0x4 * (regToSave - 2), unusedReg));
            }
        }
        // r3 <- FRB_BUFFER_SIZE_NEW
        asm_.append(PowerPcAsm::lis(3, value_bufsz.upper));
        asm_.append(PowerPcAsm::addi(3, 3, value_bufsz.lower));
        // r3 <- operator new(size = r3)
        asm_.append(PowerPcAsm::bl(subroutineAddr, asm_.size(), operatorNewAddr));
        // rD <- r3
        asm_.append(PowerPcAsm::mr(destReg, 3));
        // rU <- 0x80552758
        asm_.append(PowerPcAsm::lis(unusedReg, oldBufAddrPair.upper));
        asm_.append(PowerPcAsm::addi(unusedReg, unusedReg, oldBufAddrPair.lower));
        // *rU <- rD
        asm_.append(PowerPcAsm::stw(destReg, 0x0, unusedReg));

        for (int regToSave = 3; regToSave <= 5; ++regToSave) {
            if (regToSave != destReg) {
                asm_.append(PowerPcAsm::lwz(regToSave, 0x4 * (regToSave - 2), unusedReg));
            }
        }

        labels.define("already_initialized", asm_);
        // return to original function
        asm_.append(PowerPcAsm::b(subroutineAddr, asm_.size(), destAddr));

        labels.checkProperlyLinked();
        return asm_;
    };

    // Board
    // -----
    auto hijackAddr = addressMapper.boomStreetToStandard(0x8007dcd0);
    auto subroutineAddr = writeSubroutine(stream, std::bind(getBufferPtrSubroutine, _1, hijackAddr+4, 4, 15), "FRB Buffer Pointer Subroutine");
    // r4 <- 0x80552758 -> bl getBufferPtrSubroutine
    stream.device()->seek(addressMapper.toFileAddress(hijackAddr));
    stream << PowerPcAsm::b(hijackAddr, subroutineAddr);

    // Clear
    // -----
    hijackAddr = addressMapper.boomStreetToStandard(0x8007ddcc);
    subroutineAddr = writeSubroutine(stream, std::bind(getBufferPtrSubroutine, _1, hijackAddr+4, 31, 15), "FRB Buffer Pointer Subroutine");
    // r31 <- UPPER16(0x80552758) -> bl getBufferPtrSubroutine
    stream.device()->seek(addressMapper.toFileAddress(hijackAddr));
    stream << PowerPcAsm::b(hijackAddr, subroutineAddr);

    // r3 <- 0x80552758 -> r3 <- r31
    stream.device()->seek(addressMapper.boomToFileAddress(0x8007dddc));
    stream << PowerPcAsm::mr(3, 31);

    // r0 <- 0x80552758 -> r0 <- r31
    stream.device()->seek(addressMapper.boomToFileAddress(0x8007dde4));
    stream << PowerPcAsm::mr(0, 31);

    // Init
    // ----
    hijackAddr = addressMapper.boomStreetToStandard(0x8007e020);
    subroutineAddr = writeSubroutine(stream, std::bind(getBufferPtrSubroutine, _1, hijackAddr+4, 31, 15), "FRB Buffer Pointer Subroutine");
    // r31 <- UPPER16(0x80552758) -> bl getBufferPtrSubroutine
    stream.device()->seek(addressMapper.toFileAddress(hijackAddr));
    stream << PowerPcAsm::b(hijackAddr, subroutineAddr);

    // r3 <- 0x80552758 -> r3 <- r31
    stream.device()->seek(addressMapper.boomToFileAddress(0x8007e034));
    stream << PowerPcAsm::mr(3, 31);

    // r0 <- 0x80552758 -> r0 <- r31
    stream.device()->seek(addressMapper.boomToFileAddress(0x8007e044));
    stream << PowerPcAsm::mr(0, 31);

    // Load
    // ----
    hijackAddr = addressMapper.boomStreetToStandard(0x8007e0a4);
    subroutineAddr = writeSubroutine(stream, std::bind(getBufferPtrSubroutine, _1, hijackAddr+4, 28, 15), "FRB Buffer Pointer Subroutine");
    // r28 <- UPPER16(0x80552758) -> bl getBufferPtrSubroutine
    stream.device()->seek(addressMapper.toFileAddress(hijackAddr));
    stream << PowerPcAsm::b(hijackAddr, subroutineAddr);

    // r3 <- 0x80552758 -> r3 <- r28
    stream.device()->seek(addressMapper.boomToFileAddress(0x8007e0c8));
    stream << PowerPcAsm::mr(3, 28);

    // r0 <- 0x80552758 -> r0 <- r28
    stream.device()->seek(addressMapper.boomToFileAddress(0x8007e0d0));
    stream << PowerPcAsm::mr(0, 28);
}
