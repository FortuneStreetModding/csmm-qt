#include "districtnamefreespace.h"

void DistrictNameFreeSpace::loadFiles(const QString &root, GameInstance *gameInstance, const ModListType &modList)
{
    // nothing to do
}

void DistrictNameFreeSpace::saveFiles(const QString &root, GameInstance *gameInstance, const ModListType &modList)
{
    auto &freeSpaceManager = gameInstance->freeSpaceManager();
    auto &addressMapper = gameInstance->addressMapper();
    // District name table
    freeSpaceManager.addFreeSpace(addressMapper.boomStreetToStandard(0x80417460), addressMapper.boomStreetToStandard(0x80417508));
}
