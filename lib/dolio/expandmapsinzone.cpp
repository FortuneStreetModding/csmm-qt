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
    // addi r1, r1, 0x60 ->addi r1, r1, 0xa0
    stream << PowerPcAsm::addi(1, 1, N);
}

void ExpandMapsInZone::readAsm(QDataStream &, const AddressMapper &, QVector<MapDescriptor> &) {
    // nothing to do
}
