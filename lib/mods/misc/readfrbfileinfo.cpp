#include "readfrbfileinfo.h"
#include "lib/datafileset.h"

void ReadFrbFileInfo::loadFiles(const QString &root, GameInstance *gameInstance, const ModListType &modList)
{
    auto paramDir = QDir(root).filePath(PARAM_FOLDER);
    for (auto &descriptor: gameInstance->mapDescriptors()) {
        descriptor.readFrbFileInfo(paramDir);
    }
}

void ReadFrbFileInfo::saveFiles(const QString &root, GameInstance *gameInstance, const ModListType &modList)
{
    // crab nothing to do crab
}
