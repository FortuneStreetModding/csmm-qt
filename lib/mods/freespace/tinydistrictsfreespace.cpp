#include "tinydistrictsfreespace.h"

void TinyDistrictsFreeSpace::loadFiles(const QString &root, GameInstance *gameInstance, const ModListType &modList)
{
    // nothing to do
}

void TinyDistrictsFreeSpace::saveFiles(const QString &root, GameInstance *gameInstance, const ModListType &modList)
{
    auto &freeSpaceManager = gameInstance->freeSpaceManager();
    auto &addressMapper = gameInstance->addressMapper();
    // GetGainCoeffTable and GetMaxCapitalCoeff
    freeSpaceManager.addFreeSpace(addressMapper.boomStreetToStandard(0x80412c88), addressMapper.boomStreetToStandard(0x80412d60));
}
