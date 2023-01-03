#include "initialfreespace.h"

void InitialFreeSpace::loadFiles(const QString &root, GameInstance &gameInstance, const ModListType &modList)
{
    // nothing to do
}

void InitialFreeSpace::saveFiles(const QString &root, GameInstance &gameInstance, const ModListType &modList)
{
    auto &freeSpaceManager = gameInstance.freeSpaceManager();
    auto &addressMapper = gameInstance.addressMapper();
    // Unused costume string table 1
    freeSpaceManager.addFreeSpace(addressMapper.boomStreetToStandard(0x8042bc78), addressMapper.boomStreetToStandard(0x8042c240));
    // Unused costume string table 2
    freeSpaceManager.addFreeSpace(addressMapper.boomStreetToStandard(0x8042dfc0), addressMapper.boomStreetToStandard(0x8042e230));
    // Unused costume string table 3
    freeSpaceManager.addFreeSpace(addressMapper.boomStreetToStandard(0x8042ef30), addressMapper.boomStreetToStandard(0x8042f7f0));
    // Unused menu id=0x06 (MapSelectScene_E3)
    freeSpaceManager.addFreeSpace(addressMapper.boomStreetToStandard(0x801f8520), addressMapper.boomStreetToStandard(0x801f94bc));
    // Unused menu id=0x38 (WorldMenuScene)
    freeSpaceManager.addFreeSpace(addressMapper.boomStreetToStandard(0x801ed6a8), addressMapper.boomStreetToStandard(0x801edab8));
    // Unused menu id=0x39 (FreePlayScene)
    freeSpaceManager.addFreeSpace(addressMapper.boomStreetToStandard(0x801edad4), addressMapper.boomStreetToStandard(0x801ee720));
    // Unused menu class (SelectMapUI)
    freeSpaceManager.addFreeSpace(addressMapper.boomStreetToStandard(0x801fce28), addressMapper.boomStreetToStandard(0x801ff778));
}
