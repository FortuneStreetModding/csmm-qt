#include "expandmapsinzone.h"
#include "lib/powerpcasm.h"

void ExpandMapsInZone::writeAsm(QDataStream &stream, const AddressMapper &addressMapper, const QVector<MapDescriptor> &descriptors) {
    int mapsPerMapSetAndZone[2*3] = {0};
    for (auto &descriptor: descriptors) {
        if (descriptor.mapSet == 0 || descriptor.mapSet == 1) {
            if (descriptor.zone >= 0 && descriptor.zone <= 2) {
                ++mapsPerMapSetAndZone[descriptor.mapSet*3 + descriptor.zone];
            }
        }
    }
    // Maximum array size used by GetMapsInZone.
    int maxMapsPerMapSetAndZone = *std::max_element(std::begin(mapsPerMapSetAndZone), std::end(mapsPerMapSetAndZone));
    if (maxMapsPerMapSetAndZone % 2 == 1) { // make sure stuff is 8-byte-aligned
        ++maxMapsPerMapSetAndZone;
    }

    // For SearchMap, the original array size was 16.

    int N = 0x60 + std::max(maxMapsPerMapSetAndZone-16, 0)*sizeof(quint32);

    stream.device()->seek(addressMapper.boomToFileAddress(0x8020f570));
    // stwu r1, -0x60(r1)
    stream << PowerPcAsm::stwu(1, -N, 1);
    stream.skipRawData(8);
    // stw r0, 0x64(r1)
    stream << PowerPcAsm::stw(0, N+4, 1);
    // stmw r26, 0x48(r1)
    stream << PowerPcAsm::stmw(26, N-0x18, 1);

    stream.device()->seek(addressMapper.boomToFileAddress(0x8020f670));
    // lmw r26, 0x48(r1)
    stream << PowerPcAsm::lmw(26, N-0x18, 1);
    // lwz r0, 0x64(r1)
    stream << PowerPcAsm::lwz(0, N+4, 1);
    stream.skipRawData(4);
    // addi r1, r1, 0x60
    stream << PowerPcAsm::addi(1, 1, N);

    // Do similar stuff with CanUnlockMap. Array size is originally 19.

    N = 0x60 + std::max(maxMapsPerMapSetAndZone-18, 0)*sizeof(quint32); // 18 instead of 19 for alignment

    stream.device()->seek(addressMapper.boomToFileAddress(0x801e72f0));
    // stwu r1, -0x60(r1)
    stream << PowerPcAsm::stwu(1, -N, 1);
    stream.skipRawData(4);
    // stw r0, 0x64(r1)
    stream << PowerPcAsm::stw(0, N+4, 1);
    // stw r0, 0x5c(r1)
    stream << PowerPcAsm::stw(31, N-4, 1);

    stream.device()->seek(addressMapper.boomToFileAddress(0x801e73ac));
    // lwz r0, 0x64(r1)
    stream << PowerPcAsm::lwz(0, N+4, 1);
    // lwz r0, 0x5c(r1)
    stream << PowerPcAsm::lwz(31, N-4, 1);
    stream.skipRawData(4);
    // addi r1, r1, 0x60
    stream << PowerPcAsm::addi(1, 1, N);

    // And SetZone. Original array size is also 19.

    N = 0x70 + std::max(maxMapsPerMapSetAndZone-18, 0)*sizeof(quint32);

    stream.device()->seek(addressMapper.boomToFileAddress(0x8021ebd4));
    // stwu r1, -0x70(r1)
    stream << PowerPcAsm::stwu(1, -N, 1);
    stream.skipRawData(8);
    // stw r0, 0x74(r1)
    stream << PowerPcAsm::stw(0, N+4, 1);
    // stmw r25, 0x54(r1)
    stream << PowerPcAsm::stmw(25, N-0x1c, 1);

    stream.device()->seek(addressMapper.boomToFileAddress(0x8021ed38));
    // lmw r25, 0x54(r1)
    stream << PowerPcAsm::lmw(25, N-0x1c, 1);
    // lwz r0, 0x74(r1)
    stream << PowerPcAsm::lwz(0, N+4, 1);
    stream.skipRawData(4);
    // addi r1, r1, 0x70
    stream << PowerPcAsm::addi(1, 1, N);

    // CreateMapList requires the array to be allocated to be large enough to fit all 3 zones.
    // The array originally could fit up to 66 elements.

    N = 0x190 + std::max(3*maxMapsPerMapSetAndZone - 66, 0)*sizeof(quint32);

    stream.device()->seek(addressMapper.boomToFileAddress(0x801875d0));
    // stwu r1, -0x190(r1)
    stream << PowerPcAsm::stwu(1, -N, 1);
    stream.skipRawData(4);
    // stw r0, 0x194(r1)
    stream << PowerPcAsm::stw(0, N+4, 1);
    // stmw r16, 0x150(r1)
    stream << PowerPcAsm::stmw(16, N-0x40, 1);

    stream.device()->seek(addressMapper.boomToFileAddress(0x80187c38));
    // lmw r16, 0x150(r1)
    stream << PowerPcAsm::lmw(16, N-0x40, 1);
    // lwz r0, 0x194(r1)
    stream << PowerPcAsm::lwz(0, N+4, 1);
    stream.skipRawData(4);
    // addi r1, r1, 0x190
    stream << PowerPcAsm::addi(1, 1, N);

    // ForceSelectMap

    N = 0xb0 + std::max(maxMapsPerMapSetAndZone - 34, 0) * sizeof(quint32);

    stream.device()->seek(addressMapper.boomToFileAddress(0x80185aac));
    // stwu r1, -0xb0(r1)
    stream << PowerPcAsm::stwu(1, -N, 1);
    stream.skipRawData(4);
    // stw r0, 0xb4(r1)
    stream << PowerPcAsm::stw(0, N+4, 1);
    // stmw r24, 0x90(r1)
    stream << PowerPcAsm::stmw(24, N-0x20, 1);

    stream.device()->seek(addressMapper.boomToFileAddress(0x80185bb4));
    // lmw r24, 0x90(r1)
    stream << PowerPcAsm::lmw(24, N-0x20, 1);
    // lwz r0, 0xb4(r1)
    stream << PowerPcAsm::lwz(0, N+4, 1);
    // addi r1, r1, 0xb0
    stream << PowerPcAsm::addi(1, 1, N);

    // IsLastStage
    N = 0x90 + std::max(maxMapsPerMapSetAndZone - 32, 0) * sizeof(quint32);

    stream.device()->seek(addressMapper.boomToFileAddress(0x8020f684));
    // stwu r1, -0x90(r1)
    stream << PowerPcAsm::stwu(1, -N, 1);
    stream.skipRawData(12);
    // stw r0, 0x94(r1)
    stream << PowerPcAsm::stw(0, N+4, 1);
    // stw r31, 0x8c(r1)
    stream << PowerPcAsm::stw(31, N-4, 1);

    stream.device()->seek(addressMapper.boomToFileAddress(0x8020f70c));
    // lwz r0, 0x94(r1)
    stream << PowerPcAsm::lwz(0, N+4, 1);
    // lwz r31, 0x8c(r1)
    stream << PowerPcAsm::lwz(31, N-4, 1);
    stream.skipRawData(4);
    // addi r1, r1, 0x90
    stream << PowerPcAsm::addi(1, 1, N);

    // IsZoneClear

    N = 0xa0 + std::max(maxMapsPerMapSetAndZone - 34, 0) * sizeof(quint32);
    stream.device()->seek(addressMapper.boomToFileAddress(0x80210370));
    // stwu r1, -0xa0(r1)
    stream << PowerPcAsm::stwu(1, -N, 1);
    stream.skipRawData(12);
    // stw r0, 0xa4(r1)
    stream << PowerPcAsm::stw(0, N+4, 1);
    stream.skipRawData(4);
    // stw r31, 0x9c(r1)
    stream << PowerPcAsm::stw(31, N-4, 1);
    // stw r30, 0x98(r1)
    stream << PowerPcAsm::stw(30, N-8, 1);
    // stw r29, 0x94(r1)
    stream << PowerPcAsm::stw(29, N-12, 1);
    // stw r28, 0x90(r1)
    stream << PowerPcAsm::stw(28, N-16, 1);

    stream.device()->seek(addressMapper.boomToFileAddress(0x802103e0));
    // lwz r31, 0x9c(r1)
    stream << PowerPcAsm::lwz(31, N-4, 1);
    stream.skipRawData(4);
    // lwz r30, 0x98(r1)
    stream << PowerPcAsm::lwz(30, N-8, 1);
    // lwz r29, 0x94(r1)
    stream << PowerPcAsm::lwz(29, N-12, 1);
    // lwz r28, 0x90(r1)
    stream << PowerPcAsm::lwz(28, N-16, 1);
    // lwz r0, 0xa4(r1)
    stream << PowerPcAsm::lwz(0, N+4, 1);
    stream.skipRawData(4);
    // addi r1, r1, 0xa0
    stream << PowerPcAsm::addi(1, 1, N);

    // IsZoneVictory
    // yes there are local variables after the map array here but it doesn't matter
    // since the function ignores them after the map array is written

    N = 0x2f0 + std::max(maxMapsPerMapSetAndZone - 168, 0) * sizeof(quint32);

    stream.device()->seek(addressMapper.boomToFileAddress(0x80210970));
    // stwu r1, -0x2f0(r1)
    stream << PowerPcAsm::stwu(1, -N, 1);
    stream.skipRawData(8);
    // stw r0, 0x2f4(r1)
    stream << PowerPcAsm::stw(0, N+4, 1);
    // stmw r27, 0x2dc(r1)
    stream << PowerPcAsm::stmw(27, N-0x14, 1);

    stream.device()->seek(addressMapper.boomToFileAddress(0x80210a38));
    // lmw r27, 0x2dc(r1)
    stream << PowerPcAsm::lmw(27, N-0x14, 1);
    // lwz r0, 0x2f4(r1)
    stream << PowerPcAsm::lwz(0, N+4, 1);
    stream.skipRawData(4);
    // addi r1, r1, 0x2f0
    stream << PowerPcAsm::addi(1, 1, N);

    // GetMapLockInfoText

    N = 0x90 + std::max(maxMapsPerMapSetAndZone - 32, 0) * sizeof(quint32);

    stream.device()->seek(addressMapper.boomToFileAddress(0x80212178));
    // stwu r1, -0x90(r1)
    stream << PowerPcAsm::stwu(1, -N, 1);
    stream.skipRawData(12);
    // stw r0, 0x94(r1)
    stream << PowerPcAsm::stw(0, N+4, 1);
    // stw r31, 0x8c(r1)
    stream << PowerPcAsm::stw(31, N-4, 1);
    stream.skipRawData(4);
    // stw r30, 0x88(r1)
    stream << PowerPcAsm::stw(30, N-8, 1);

    stream.device()->seek(addressMapper.boomToFileAddress(0x8021223c));
    // lwz r0, 0x94(r1)
    stream << PowerPcAsm::lwz(0, N+4, 1);
    // lwz r31, 0x8c(r1)
    stream << PowerPcAsm::lwz(31, N-4, 1);
    // lwz r30, 0x88(r1)
    stream << PowerPcAsm::lwz(30, N-8, 1);
    stream.skipRawData(4);
    // addi r1, r1, 0x90
    stream << PowerPcAsm::addi(1, 1, N);

    // Init_SelectBit
    N = 0xc0 + std::max(maxMapsPerMapSetAndZone - 34, 0) * sizeof(quint32);

    stream.device()->seek(addressMapper.boomToFileAddress(0x802542a4));
    // stwu r1, -0xc0(r1)
    stream << PowerPcAsm::stwu(1, -N, 1);
    stream.skipRawData(4);
    // stw r0, 0xc4(r1)
    stream << PowerPcAsm::stw(0, N+4, 1);
    stream.skipRawData(4);
    // stmw r24, 0xa0(r1)
    stream << PowerPcAsm::stw(24, N-0x20, 1);

    stream.device()->seek(addressMapper.boomToFileAddress(0x802543bc));
    // lmw r24, 0xa0(r1)
    stream << PowerPcAsm::lmw(24, N-0x20, 1);
    // lwz r0, 0xc4(r1)
    stream << PowerPcAsm::lwz(0, N+4, 1);
    stream.skipRawData(4);
    // addi r1, r1, 0xc0
    stream << PowerPcAsm::addi(1, 1, N);

    // InitializeInfo
    N = 0xc0 + std::max(maxMapsPerMapSetAndZone - 34, 0) * sizeof(quint32);

    stream.device()->seek(addressMapper.boomToFileAddress(0x802543d0));
    // stwu r1, -0xc0(r1)
    stream << PowerPcAsm::stwu(1, -N, 1);
    stream.skipRawData(12);
    // stw r0, 0xc4(r1)
    stream << PowerPcAsm::stw(0, N+4, 1);
    stream.skipRawData(4);
    // stmw r25, 0xa4(r1)
    stream << PowerPcAsm::stw(25, N-0x1c, 1);

    stream.device()->seek(addressMapper.boomToFileAddress(0x80254548));
    // lmw r25, 0xa4(r1)
    stream << PowerPcAsm::lmw(25, N-0x1c, 1);
    // lwz r0, 0xc4(r1)
    stream << PowerPcAsm::lwz(0, N+4, 1);
    stream.skipRawData(4);
    // addi r1, r1, 0xc0
    stream << PowerPcAsm::addi(1, 1, N);
}

void ExpandMapsInZone::readAsm(QDataStream &, const AddressMapper &, QVector<MapDescriptor> &) {
    // nothing to do
}
