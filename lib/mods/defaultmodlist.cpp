#include "defaultmodlist.h"

#include "arc/defaultminimapicons.h"
#include "arc/turnlotscenes.h"
#include "dolio/allocatedescriptorcount.h"
#include "dolio/backgroundtable.h"
#include "dolio/bgmidtable.h"
#include "dolio/bgsequencetable.h"
#include "dolio/defaulttargetamounttable.h"
#include "dolio/designtypetable.h"
#include "dolio/eventsquaremod.h"
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
#include "dolio/mutatorgenerichook.h"
#include "dolio/mutatortable.h"
#include "dolio/mutatorshopprice.h"
#include "dolio/mutatorstockprice.h"
#include "dolio/expandmapsinzone.h"
#include "freespace/districtnamefreespace.h"
#include "freespace/initialfreespace.h"
#include "freespace/mapdatafreespace.h"
#include "freespace/venturecardfreespace.h"
#include "freespace/wififreespace.h"
#include "freespace/tinydistrictsfreespace.h"
#include "freespace/vmovestopfreespace.h"
#include "dolio/changemusiconswitch.h"
#include "misc/readfrbfileinfo.h"
#include "misc/defaultmiscpatches.h"
#include "dolio/arbitrarynumswitchstates.h"
#include "dolio/customshopnames.h"
#include "misc/copymapfiles.h"

namespace DefaultModList {

ModListType defaultModList() {
    ModListType patches;

    patches.append(CSMMModHolder::fromCppObj<AllocateDescriptorCount>());

    patches.append(CSMMModHolder::fromCppObj<MapOriginTable>());
    patches.append(CSMMModHolder::fromCppObj<MapDescriptionTable>());
    patches.append(CSMMModHolder::fromCppObj<BackgroundTable>());
    patches.append(CSMMModHolder::fromCppObj<MapIconTable>());
    patches.append(CSMMModHolder::fromCppObj<MapSetZoneOrder>());
    patches.append(CSMMModHolder::fromCppObj<PracticeBoard>());
    patches.append(CSMMModHolder::fromCppObj<DefaultTargetAmountTable>());
    patches.append(CSMMModHolder::fromCppObj<VentureCardTable>());
    patches.append(CSMMModHolder::fromCppObj<EventSquareMod>());
    patches.append(CSMMModHolder::fromCppObj<RuleSetTable>());
    patches.append(CSMMModHolder::fromCppObj<TourBankruptcyLimitTable>());
    patches.append(CSMMModHolder::fromCppObj<TourInitialCashTable>());
    patches.append(CSMMModHolder::fromCppObj<TourOpponentsTable>());
    patches.append(CSMMModHolder::fromCppObj<TourClearRankTable>());
    patches.append(CSMMModHolder::fromCppObj<StageNameIDTable>());
    patches.append(CSMMModHolder::fromCppObj<BGMIDTable>());
    patches.append(CSMMModHolder::fromCppObj<DesignTypeTable>());
    patches.append(CSMMModHolder::fromCppObj<FrbMapTable>());
    patches.append(CSMMModHolder::fromCppObj<MapSwitchParamTable>());
    patches.append(CSMMModHolder::fromCppObj<MapGalaxyParamTable>());
    patches.append(CSMMModHolder::fromCppObj<BGSequenceTable>());
    patches.append(CSMMModHolder::fromCppObj<InternalNameTable>());
    patches.append(CSMMModHolder::fromCppObj<ForceSimulatedButtonPress>());
    patches.append(CSMMModHolder::fromCppObj<WifiFix>());
    patches.append(CSMMModHolder::fromCppObj<MusicTable>());
    patches.append(CSMMModHolder::fromCppObj<DisplayMapInResults>());
    patches.append(CSMMModHolder::fromCppObj<TinyDistricts>());
    patches.append(CSMMModHolder::fromCppObj<NamedDistricts>());
    patches.append(CSMMModHolder::fromCppObj<ArbitraryNumSwitchStates>());
    patches.append(CSMMModHolder::fromCppObj<CustomShopNames>(false));
    patches.append(CSMMModHolder::fromCppObj<CustomShopNames>(true));
    patches.append(CSMMModHolder::fromCppObj<ChangeMusicOnSwitch>());

    // mutators
    patches.append(CSMMModHolder::fromCppObj<MutatorTable>());
    patches.append(CSMMModHolder::fromCppObj<MutatorRollShopPriceMultiplier>());
    patches.append(CSMMModHolder::fromCppObj<MutatorShopPrice>());
    patches.append(CSMMModHolder::fromCppObj<MutatorStockPrice>());
    patches.append(CSMMModHolder::fromCppObj<MutatorGenericHook>());

    patches.append(CSMMModHolder::fromCppObj<ExpandMapsInZone>());
    patches.append(CSMMModHolder::fromCppObj<DefaultMinimapIcons>());
    patches.append(CSMMModHolder::fromCppObj<TurnlotScenes>());

    patches.append(CSMMModHolder::fromCppObj<DistrictNameFreeSpace>());
    patches.append(CSMMModHolder::fromCppObj<WifiFreeSpace>());
    patches.append(CSMMModHolder::fromCppObj<VentureCardFreeSpace>());
    patches.append(CSMMModHolder::fromCppObj<InitialFreeSpace>());
    patches.append(CSMMModHolder::fromCppObj<MapDataFreeSpace>());
    patches.append(CSMMModHolder::fromCppObj<TinyDistrictsFreeSpace>());
    patches.append(CSMMModHolder::fromCppObj<VMoveStopFreeSpace>());

    patches.append(CSMMModHolder::fromCppObj<DefaultMiscPatches>());
    patches.append(CSMMModHolder::fromCppObj<ReadFrbFileInfo>());
    patches.append(CSMMModHolder::fromCppObj<CopyMapFiles>());

    return patches;
}

}
