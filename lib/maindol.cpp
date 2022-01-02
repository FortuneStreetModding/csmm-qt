#include "maindol.h"

#include "dolio/backgroundtable.h"
#include "dolio/bgmidtable.h"
#include "dolio/bgsequencetable.h"
#include "dolio/defaulttargetamounttable.h"
#include "dolio/designtypetable.h"
#include "dolio/eventsquare.h"
#include "dolio/forcesimulatedbuttonpress.h"
#include "dolio/frbmaptable.h"
#include "dolio/internalnametable.h"
#include "dolio/mapdescriptiontable.h"
#include "dolio/mapgalaxyparamtable.h"
#include "dolio/mapicontable.h"
#include "dolio/maporigintable.h"
#include "dolio/mapsetzoneorder.h"
#include "dolio/mapswitchparamtable.h"
#include "dolio/practiceboard.h"
#include "dolio/rulesettable.h"
#include "dolio/stagenameidtable.h"
#include "dolio/tourbankruptcylimittable.h"
#include "dolio/tourclearranktable.h"
#include "dolio/tourinitialcashtable.h"
#include "dolio/touropponentstable.h"
#include "dolio/venturecardtable.h"
#include "dolio/wififix.h"
#include "dolio/musictable.h"
#include "dolio/displaymapinresults.h"
#include "dolio/tinydistricts.h"
#include "dolio/nameddistricts.h"
#include "dolio/mutatorrollshoppricemultiplier.h"
#include "dolio/mutatortable.h"
#include "dolio/mutatorshoppricemultiplier.h"
#include "dolio/expandmapsinzone.h"
#include "powerpcasm.h"

MainDol::MainDol(QDataStream &stream, const QVector<AddressSection> &mappingSections, bool patchResultBoardName) {
    addressMapper = setupAddressMapper(stream, mappingSections);
    freeSpaceManager = setupFreeSpaceManager(addressMapper);
    patches = setupPatches(patchResultBoardName);
}

AddressMapper MainDol::setupAddressMapper(QDataStream &stream, const QVector<AddressSection> &fileMappingSections) {
    AddressMapper addressMapperVal(fileMappingSections);
    stream.device()->seek(addressMapperVal.toFileAddress(0x8007a314));
    quint32 inst;
    stream >> inst;
    if (inst == PowerPcAsm::lwz(0, -0x547c, 13)) {
        // Boom Street detected
        addressMapperVal.setVersionMapper(AddressSectionMapper({ AddressSection() }));
    } else {
        // Fortune Street
        stream.device()->seek(addressMapperVal.toFileAddress(0x8007a2c0));
        // check PowerPcAsm::lwz(0, -0x547c, 13)?
        addressMapperVal.setVersionMapper(AddressSectionMapper({
            {0x80000100, 0x8007a283, 0x0, ".text, .data0, .data1 and beginning of .text1 until InitSoftLanguage"},
            {0x8007a2f4, 0x80268717, 0x54, "continuation of .text1 until AIRegisterDMACallback"},
            {0x80268720, 0x8040d97b, 0x50, "continuation of .text1"},
            {0x8040d980, 0x8041027f, 0x40, ".data2, .data3 and beginning of .data4 until Boom Street / Fortune Street strings"},
            {0x804105f0, 0x8044ebe7, 0x188, "continuation of .data4"},
            {0x8044ec00, 0x804ac804, 0x1A0, ".data5"},
            {0x804ac880, 0x8081f013, 0x200, ".uninitialized0, .data6, .uninitialized1, .data7, .uninitialized2"}
        }));
    }
    return addressMapperVal;
}

FreeSpaceManager MainDol::setupFreeSpaceManager(AddressMapper addressMapper) {
    FreeSpaceManager result;
    // Venture Card Table
    result.addFreeSpace(addressMapper.boomStreetToStandard(0x80410648), addressMapper.boomStreetToStandard(0x80411b9b));
    // Map Data String Table and Map Data Table
    result.addFreeSpace(addressMapper.boomStreetToStandard(0x80428978), addressMapper.boomStreetToStandard(0x804298cf));
    //
    // used additional address:
    // 0x804363b4 (4 bytes):  force simulated button press
    // 0x804363b8 (12 bytes): pointer to internal name table
    // 0x804363c4 (4 bytes):  ForceVentureCardVariable
    // 0x80412c88 - 0x80412d5f (214 bytes): GetMutatorDataSubroutine (originally: GetGainCoeffTable and GetMaxCapitalCoeff, but since it is replaced by tinydistricts.cpp we can repurpose that as a code cave)
    //
    // Map Default Settings Table
    result.addFreeSpace(addressMapper.boomStreetToStandard(0x804363c8), addressMapper.boomStreetToStandard(0x80436a87));
    // Unused costume string table 1
    result.addFreeSpace(addressMapper.boomStreetToStandard(0x8042bc78), addressMapper.boomStreetToStandard(0x8042c23f));
    // Unused costume string table 2
    result.addFreeSpace(addressMapper.boomStreetToStandard(0x8042dfc0), addressMapper.boomStreetToStandard(0x8042e22f));
    // Unused costume string table 3
    result.addFreeSpace(addressMapper.boomStreetToStandard(0x8042ef30), addressMapper.boomStreetToStandard(0x8042f7ef));
    // Unused menu id=0x06 (MapSelectScene_E3)
    result.addFreeSpace(addressMapper.boomStreetToStandard(0x801f8520), addressMapper.boomStreetToStandard(0x801f94bb));
    // Unused menu id=0x38 (WorldMenuScene)
    result.addFreeSpace(addressMapper.boomStreetToStandard(0x801ed6a8), addressMapper.boomStreetToStandard(0x801edab7));
    // Unused menu id=0x39 (FreePlayScene)
    result.addFreeSpace(addressMapper.boomStreetToStandard(0x801edad4), addressMapper.boomStreetToStandard(0x801ee71f));
    // Unused menu class (SelectMapUI)
    result.addFreeSpace(addressMapper.boomStreetToStandard(0x801fce28), addressMapper.boomStreetToStandard(0x801ff777));
    // District name table
    result.addFreeSpace(addressMapper.boomStreetToStandard(0x80417460), addressMapper.boomStreetToStandard(0x80417507));
    return result;
}

QVector<QSharedPointer<DolIO>> MainDol::setupPatches(bool patchResultBoardName) {
    QVector<QSharedPointer<DolIO>> patches;
    patches.append(QSharedPointer<DolIO>(new MapOriginTable()));
    // map description table must be after map origin table
    patches.append(QSharedPointer<DolIO>(new MapDescriptionTable()));
    patches.append(QSharedPointer<DolIO>(new BackgroundTable()));
    // map icon table must be after the map background table and map origin table
    patches.append(QSharedPointer<DolIO>(new MapIconTable()));

    patches.append(QSharedPointer<DolIO>(new MapSetZoneOrder()));
    // practice board comes after category zone order
    patches.append(QSharedPointer<DolIO>(new PracticeBoard()));

    // the rest does not have any dependencies
    patches.append(QSharedPointer<DolIO>(new DefaultTargetAmountTable()));
    patches.append(QSharedPointer<DolIO>(new VentureCardTable()));
    patches.append(QSharedPointer<DolIO>(new class EventSquare()));
    patches.append(QSharedPointer<DolIO>(new RuleSetTable()));
    patches.append(QSharedPointer<DolIO>(new TourBankruptcyLimitTable()));
    patches.append(QSharedPointer<DolIO>(new TourInitialCashTable()));
    patches.append(QSharedPointer<DolIO>(new TourOpponentsTable()));
    patches.append(QSharedPointer<DolIO>(new TourClearRankTable()));
    patches.append(QSharedPointer<DolIO>(new StageNameIDTable()));
    patches.append(QSharedPointer<DolIO>(new BGMIDTable()));
    patches.append(QSharedPointer<DolIO>(new DesignTypeTable()));
    patches.append(QSharedPointer<DolIO>(new FrbMapTable()));
    patches.append(QSharedPointer<DolIO>(new MapSwitchParamTable()));
    patches.append(QSharedPointer<DolIO>(new MapGalaxyParamTable()));
    patches.append(QSharedPointer<DolIO>(new BGSequenceTable()));
    patches.append(QSharedPointer<DolIO>(new InternalNameTable()));
    patches.append(QSharedPointer<DolIO>(new ForceSimulatedButtonPress()));
    patches.append(QSharedPointer<DolIO>(new WifiFix()));
    patches.append(QSharedPointer<DolIO>(new MusicTable()));
    if (patchResultBoardName) {
        patches.append(QSharedPointer<DolIO>(new DisplayMapInResults()));
    }
    patches.append(QSharedPointer<DolIO>(new TinyDistricts()));
    patches.append(QSharedPointer<DolIO>(new NamedDistricts()));

    // mutators
    // TODO re-enable when we find the ai message looping bug
#if 0
    patches.append(QSharedPointer<DolIO>(new MutatorTable()));
    patches.append(QSharedPointer<DolIO>(new MutatorRollShopPriceMultiplier()));
    patches.append(QSharedPointer<DolIO>(new MutatorShopPriceMultiplier()));
#endif

    patches.append(QSharedPointer<DolIO>(new ExpandMapsInZone()));

    return patches;
}

QVector<MapDescriptor> MainDol::readMainDol(QDataStream &stream) {
    stream.device()->seek(addressMapper.boomToFileAddress(0x801cca30));
    quint32 opcode;
    stream >> opcode;
    qint16 count = PowerPcAsm::getOpcodeParameter(opcode);
    QVector<MapDescriptor> mapDescriptors(count);
    for (auto &patch: patches) {
        patch->readAsm(stream, addressMapper, mapDescriptors);
    }
    return mapDescriptors;
}

QVector<MapDescriptor> MainDol::writeMainDol(QDataStream &stream, const QVector<MapDescriptor> &mapDescriptors) {
    for (auto &patch: patches) {
        patch->write(stream, addressMapper, mapDescriptors, freeSpaceManager);
    }

    auto totalFreeSpace = freeSpaceManager.calculateTotalFreeSpace();
    auto largestFreeSpaceBlockSize = freeSpaceManager.calculateLargestFreeSpaceBlockSize();
    auto remainingFreeSpace = freeSpaceManager.calculateTotalRemainingFreeSpace();
    auto largestRemainingFreeSpaceBlockSize = freeSpaceManager.calculateLargestRemainingFreeSpaceBlockSize();
    qfloat16 freeSpaceUsage = (qfloat16) remainingFreeSpace / (qfloat16) totalFreeSpace;
    qfloat16 largestBlockUsage = (qfloat16) largestRemainingFreeSpaceBlockSize / (qfloat16) largestFreeSpaceBlockSize;
    qDebug().noquote() << QString("Free Space Left: %1/%2 (%3)").arg(remainingFreeSpace).arg(totalFreeSpace).arg(freeSpaceUsage);
    qDebug().noquote() << QString("  Largest Block: %1/%2 (%3)").arg(largestRemainingFreeSpaceBlockSize).arg(largestFreeSpaceBlockSize).arg(largestBlockUsage);

    freeSpaceManager.nullTheFreeSpace(stream, addressMapper);

    stream.device()->seek(addressMapper.boomToFileAddress(0x801cca30));
    stream << PowerPcAsm::li(3, mapDescriptors.size());
    return mapDescriptors;
}
