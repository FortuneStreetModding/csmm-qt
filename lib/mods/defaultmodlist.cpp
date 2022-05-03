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
#include "dolio/mutatortable.h"
#include "dolio/mutatorshoppricemultiplier.h"
#include "dolio/expandmapsinzone.h"
#include "freespace/districtnamefreespace.h"
#include "freespace/initialfreespace.h"
#include "freespace/mapdatafreespace.h"
#include "freespace/venturecardfreespace.h"
#include "freespace/wififreespace.h"
#include "misc/defaultmiscpatches.h"

namespace DefaultModList {

ModListType defaultModList() {
    ModListType patches;

    patches.append(std::make_shared<AllocateDescriptorCount>());

    patches.append(std::make_shared<MapOriginTable>());
    patches.append(std::make_shared<MapDescriptionTable>());
    patches.append(std::make_shared<BackgroundTable>());
    patches.append(std::make_shared<MapIconTable>());
    patches.append(std::make_shared<MapSetZoneOrder>());
    patches.append(std::make_shared<PracticeBoard>());
    patches.append(std::make_shared<DefaultTargetAmountTable>());
    patches.append(std::make_shared<VentureCardTable>());
    patches.append(std::make_shared<EventSquareMod>());
    patches.append(std::make_shared<RuleSetTable>());
    patches.append(std::make_shared<TourBankruptcyLimitTable>());
    patches.append(std::make_shared<TourInitialCashTable>());
    patches.append(std::make_shared<TourOpponentsTable>());
    patches.append(std::make_shared<TourClearRankTable>());
    patches.append(std::make_shared<StageNameIDTable>());
    patches.append(std::make_shared<BGMIDTable>());
    patches.append(std::make_shared<DesignTypeTable>());
    patches.append(std::make_shared<FrbMapTable>());
    patches.append(std::make_shared<MapSwitchParamTable>());
    patches.append(std::make_shared<MapGalaxyParamTable>());
    patches.append(std::make_shared<BGSequenceTable>());
    patches.append(std::make_shared<InternalNameTable>());
    patches.append(std::make_shared<ForceSimulatedButtonPress>());
    patches.append(std::make_shared<WifiFix>());
    patches.append(std::make_shared<MusicTable>());
    patches.append(std::make_shared<DisplayMapInResults>());
    patches.append(std::make_shared<TinyDistricts>());
    patches.append(std::make_shared<NamedDistricts>());

    // mutators
    patches.append(std::make_shared<MutatorTable>());
    patches.append(std::make_shared<MutatorRollShopPriceMultiplier>());
    patches.append(std::make_shared<MutatorShopPriceMultiplier>());

    patches.append(std::make_shared<ExpandMapsInZone>());

    patches.append(std::make_shared<DefaultMinimapIcons>());
    patches.append(std::make_shared<TurnlotScenes>());

    patches.append(std::make_shared<DistrictNameFreeSpace>());
    patches.append(std::make_shared<WifiFreeSpace>());
    patches.append(std::make_shared<VentureCardFreeSpace>());
    patches.append(std::make_shared<InitialFreeSpace>());
    patches.append(std::make_shared<MapDataFreeSpace>());

    patches.append(std::make_shared<DefaultMiscPatches>());

    return patches;
}

}
