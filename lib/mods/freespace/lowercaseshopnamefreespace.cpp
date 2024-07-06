#include "lowercaseshopnamefreespace.h"

void LowerCaseShopNameFreeSpace::loadFiles(const QString &root, GameInstance *gameInstance, const ModListType &modList)
{
    // crab nothing to do crab
}

void LowerCaseShopNameFreeSpace::saveFiles(const QString &root, GameInstance *gameInstance, const ModListType &modList)
{
    auto &freeSpaceManager = gameInstance->freeSpaceManager();
    auto &addressMapper = gameInstance->addressMapper();
    freeSpaceManager.addFreeSpace(addressMapper.boomStreetToStandard(0x800f85ac), addressMapper.boomStreetToStandard(0x800f86cc));
}
