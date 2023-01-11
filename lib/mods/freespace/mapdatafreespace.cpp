#include "mapdatafreespace.h"

void MapDataFreeSpace::loadFiles(const QString &root, GameInstance *gameInstance, const ModListType &modList)
{
    // nothing to do
}

void MapDataFreeSpace::saveFiles(const QString &root, GameInstance *gameInstance, const ModListType &modList)
{
    auto &freeSpaceManager = gameInstance->freeSpaceManager();
    auto &addressMapper = gameInstance->addressMapper();
    // Map Default Settings Table
    freeSpaceManager.addFreeSpace(addressMapper.boomStreetToStandard(0x804363b4), addressMapper.boomStreetToStandard(0x80436a88));
    // Map Data String Table and Map Data Table
    freeSpaceManager.addFreeSpace(addressMapper.boomStreetToStandard(0x80428978), addressMapper.boomStreetToStandard(0x804298d0));
}
