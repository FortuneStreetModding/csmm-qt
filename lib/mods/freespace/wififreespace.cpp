#include "wififreespace.h"

void WifiFreeSpace::loadFiles(const QString &root, GameInstance *gameInstance, const ModListType &modList)
{
    // nothing to do
}

void WifiFreeSpace::saveFiles(const QString &root, GameInstance *gameInstance, const ModListType &modList)
{
    auto &freeSpaceManager = gameInstance->freeSpaceManager();
    auto &addressMapper = gameInstance->addressMapper();
    // -- Random Matchmaking Code --
    // WifiFreeScene
    freeSpaceManager.addFreeSpace(addressMapper.boomStreetToStandard(0x8023d088), addressMapper.boomStreetToStandard(0x8023d308));
    // WifiFreeUI
    freeSpaceManager.addFreeSpace(addressMapper.boomStreetToStandard(0x8023d600), addressMapper.boomStreetToStandard(0x8023d7d4));
    // WifiFreeLobbyScene
    freeSpaceManager.addFreeSpace(addressMapper.boomStreetToStandard(0x80240f54), addressMapper.boomStreetToStandard(0x802434ec));
    // WifiFreeRegulationScene
    freeSpaceManager.addFreeSpace(addressMapper.boomStreetToStandard(0x80243508), addressMapper.boomStreetToStandard(0x80243b50));
}
