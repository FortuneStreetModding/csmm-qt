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

    patches.append(QSharedPointer<AllocateDescriptorCount>::create());

    patches.append(QSharedPointer<MapOriginTable>::create());
    patches.append(QSharedPointer<MapDescriptionTable>::create());
    patches.append(QSharedPointer<BackgroundTable>::create());
    patches.append(QSharedPointer<MapIconTable>::create());
    patches.append(QSharedPointer<MapSetZoneOrder>::create());
    patches.append(QSharedPointer<PracticeBoard>::create());
    patches.append(QSharedPointer<DefaultTargetAmountTable>::create());
    patches.append(QSharedPointer<VentureCardTable>::create());
    patches.append(QSharedPointer<EventSquareMod>::create());
    patches.append(QSharedPointer<RuleSetTable>::create());
    patches.append(QSharedPointer<TourBankruptcyLimitTable>::create());
    patches.append(QSharedPointer<TourInitialCashTable>::create());
    patches.append(QSharedPointer<TourOpponentsTable>::create());
    patches.append(QSharedPointer<TourClearRankTable>::create());
    patches.append(QSharedPointer<StageNameIDTable>::create());
    patches.append(QSharedPointer<BGMIDTable>::create());
    patches.append(QSharedPointer<DesignTypeTable>::create());
    patches.append(QSharedPointer<FrbMapTable>::create());
    patches.append(QSharedPointer<MapSwitchParamTable>::create());
    patches.append(QSharedPointer<MapGalaxyParamTable>::create());
    patches.append(QSharedPointer<BGSequenceTable>::create());
    patches.append(QSharedPointer<InternalNameTable>::create());
    patches.append(QSharedPointer<ForceSimulatedButtonPress>::create());
    patches.append(QSharedPointer<WifiFix>::create());
    patches.append(QSharedPointer<MusicTable>::create());
    patches.append(QSharedPointer<DisplayMapInResults>::create());
    patches.append(QSharedPointer<TinyDistricts>::create());
    patches.append(QSharedPointer<NamedDistricts>::create());

    // mutators
    patches.append(QSharedPointer<MutatorTable>::create());
    patches.append(QSharedPointer<MutatorRollShopPriceMultiplier>::create());
    patches.append(QSharedPointer<MutatorShopPriceMultiplier>::create());

    patches.append(QSharedPointer<ExpandMapsInZone>::create());

    patches.append(QSharedPointer<DefaultMinimapIcons>::create());
    patches.append(QSharedPointer<TurnlotScenes>::create());

    patches.append(QSharedPointer<DistrictNameFreeSpace>::create());
    patches.append(QSharedPointer<WifiFreeSpace>::create());
    patches.append(QSharedPointer<VentureCardFreeSpace>::create());
    patches.append(QSharedPointer<InitialFreeSpace>::create());
    patches.append(QSharedPointer<MapDataFreeSpace>::create());

    patches.append(QSharedPointer<DefaultMiscPatches>::create());

    return patches;
}

}
