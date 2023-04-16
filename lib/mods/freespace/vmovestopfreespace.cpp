#include "vmovestopfreespace.h"

void VMoveStopFreeSpace::loadFiles(const QString &root, GameInstance *gameInstance, const ModListType &modList)
{
    // crab nothing to do crab
}

void VMoveStopFreeSpace::saveFiles(const QString &root, GameInstance *gameInstance, const ModListType &modList)
{
    auto &freeSpaceManager = gameInstance->freeSpaceManager();
    auto &addressMapper = gameInstance->addressMapper();
    // V_MOVE/V_STOP string pointers
    freeSpaceManager.addFreeSpace(addressMapper.boomStreetToStandard(0x80474468), addressMapper.boomStreetToStandard(0x80474488));
    // V_MOVE/V_STOP strings proper
    freeSpaceManager.addFreeSpace(addressMapper.boomStreetToStandard(0x80819cf0), addressMapper.boomStreetToStandard(0x80819d30));
}
