#include "expandmapsinzone.h"
#include "lib/powerpcasm.h"

void ExpandMapsInZone::writeAsm(QDataStream &stream, const AddressMapper &addressMapper, const QVector<MapDescriptor> &) {
    // For SearchMap, we increase the allocated array size by 0x40 == 16 * sizeof(quint32).

    int N = 0x60 + 16*sizeof(quint32);

    stream.device()->seek(addressMapper.boomToFileAddress(0x8020f570));
    // stwu r1, -0x60(r1) -> stwu r1, -0xa0(r1)
    stream << PowerPcAsm::stwu(1, -N, 1);
    stream.skipRawData(8);
    // stw r0, 0x64(r1) -> stw r0, 0xa4(r1)
    stream << PowerPcAsm::stw(0, N+4, 1);
    // stmw r26, 0x48(r1) -> stmw r26, 0x88(r1)
    stream << PowerPcAsm::stmw(26, N-0x18, 1);

    stream.device()->seek(addressMapper.boomToFileAddress(0x8020f670));
    // lmw r26, 0x48(r1) -> lmw r26, 0x88(r1)
    stream << PowerPcAsm::lmw(26, N-0x18, 1);
    // lwz r0, 0x64(r1) -> lwz r0, 0xa4(r1)
    stream << PowerPcAsm::lwz(0, N+4, 1);
    stream.skipRawData(4);
    // addi r1, r1, 0x60 -> addi r1, r1, 0xa0
    stream << PowerPcAsm::addi(1, 1, N);

    // Do similar stuff with CanUnlockMap.

    N = 0x60 + 16*sizeof(quint32);

    stream.device()->seek(addressMapper.boomToFileAddress(0x801e72f0));
    // stwu r1, -0x60(r1) -> stwu r1, -0xa0(r1)
    stream << PowerPcAsm::stwu(1, -N, 1);
    stream.skipRawData(4);
    // stw r0, 0x64(r1) -> stw r0, 0xa4(r1)
    stream << PowerPcAsm::stw(0, N+4, 1);
    // stw r0, 0x5c(r1) -> stw r0, 0x9c(r1)
    stream << PowerPcAsm::stw(31, N-4, 1);

    stream.device()->seek(addressMapper.boomToFileAddress(0x801e73ac));
    // lwz r0, 0x64(r1) -> lwz r0, 0xa4(r1)
    stream << PowerPcAsm::lwz(0, N+4, 1);
    // lwz r0, 0x5c(r1) -> lwz r0, 0x9c(r1)
    stream << PowerPcAsm::lwz(31, N-4, 1);
    stream.skipRawData(4);
    // addi r1, r1, 0x60 -> addi r1, r1, 0xa0
    stream << PowerPcAsm::addi(1, 1, N);

    // And SetZone.

    N = 0x70 + 16*sizeof(quint32);

    stream.device()->seek(addressMapper.boomToFileAddress(0x8021ebd4));
    // stwu r1, -0x70(r1) -> stwu r1, -0xb0(r1)
    stream << PowerPcAsm::stwu(1, -N, 1);
    stream.skipRawData(8);
    // stw r0, 0x74(r1) -> stw r0, 0xb4(r1)
    stream << PowerPcAsm::stw(0, N+4, 1);
    // stmw r25, 0x54(r1) -> stmw r25, 0x94(r1)
    stream << PowerPcAsm::stmw(25, N-0x1c, 1);

    stream.device()->seek(addressMapper.boomToFileAddress(0x8021ed38));
    // lmw r25, 0x54(r1) -> lmw r25, 0x94(r1)
    stream << PowerPcAsm::lmw(25, N-0x1c, 1);
    // lwz r0, 0x74(r1) -> lwz r0, 0xb4(r1)
    stream << PowerPcAsm::lwz(0, N+4, 1);
    stream.skipRawData(4);
    // addi r1, r1, 0x70 -> addi r1, r1, 0xb0
    stream << PowerPcAsm::addi(1, 1, N);

    // CreateMapList requires the array to be allocated to be large enough to fit all 3 zones.
    // The array originally could fit up to 66 elements, we enlarge it to fit 66+32=98.

    N = 0x190 + 32*sizeof(quint32);

    stream.device()->seek(addressMapper.boomToFileAddress(0x801875d0));
    // stwu r1, -0x190(r1) -> stwu r1, -0x210(r1)
    stream << PowerPcAsm::stwu(1, -N, 1);
    stream.skipRawData(4);
    // stw r0, 0x194(r1) -> stw r0, 0x214(r1)
    stream << PowerPcAsm::stw(0, N+4, 1);
    // stmw r16, 0x150(r1) -> stmw r16, 0x1d0(r1)
    stream << PowerPcAsm::stmw(25, N-0x40, 1);

    stream.device()->seek(addressMapper.boomToFileAddress(0x80187c38));
    // lmw r16, 0x150(r1) -> lmw r16, 0x1d0(r1)
    stream << PowerPcAsm::lmw(25, N-0x40, 1);
    // lwz r0, 0x194(r1) -> lwz r0, 0x214(r1)
    stream << PowerPcAsm::lwz(0, N+4, 1);
    stream.skipRawData(4);
    // addi r1, r1, 0x190 -> addi r1, r1, 0x210
    stream << PowerPcAsm::addi(1, 1, N);
}

void ExpandMapsInZone::readAsm(QDataStream &, const AddressMapper &, QVector<MapDescriptor> &) {
    // nothing to do
}
