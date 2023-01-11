#include "venturecardfreespace.h"

void VentureCardFreeSpace::loadFiles(const QString &root, GameInstance *gameInstance, const ModListType &modList)
{
    // nothing to do
}

void VentureCardFreeSpace::saveFiles(const QString &root, GameInstance *gameInstance, const ModListType &modList)
{
    auto &freeSpaceManager = gameInstance->freeSpaceManager();
    auto &addressMapper = gameInstance->addressMapper();
    // Venture Card Table
    freeSpaceManager.addFreeSpace(addressMapper.boomStreetToStandard(0x80410648), addressMapper.boomStreetToStandard(0x80411b9c));
}
