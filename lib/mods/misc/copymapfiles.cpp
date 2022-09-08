#include "copymapfiles.h"
#include "lib/datafileset.h"
#include <filesystem>

void CopyMapFiles::saveFiles(const QString &root, GameInstance &gameInstance, const ModListType &modList)
{
    auto paramDir = QDir(gameInstance.getImportDir()).filePath(PARAM_FOLDER);
    for (auto &frbFile: QDir(paramDir).entryInfoList({"*.frb"}, QDir::Files)) {
        auto destFrbFile = QDir(root).filePath(PARAM_FOLDER + "/" + frbFile.fileName());
        QFile::remove(destFrbFile);
        QFile::copy(frbFile.filePath(), destFrbFile);
    }
    std::filesystem::copy(
                QDir(gameInstance.getImportDir()).filePath(SOUND_STREAM_FOLDER).toStdU16String(),
                QDir(root).filePath(SOUND_STREAM_FOLDER).toStdU16String(),
                std::filesystem::copy_options::recursive | std::filesystem::copy_options::overwrite_existing
                );
    std::filesystem::copy(
                QDir(gameInstance.getImportDir()).filePath(SCENE_FOLDER).toStdU16String(),
                QDir(root).filePath(SCENE_FOLDER).toStdU16String(),
                std::filesystem::copy_options::recursive | std::filesystem::copy_options::overwrite_existing
                );
    std::filesystem::copy(
                QDir(gameInstance.getImportDir()).filePath("files/bg").toStdU16String(),
                QDir(root).filePath("files/bg").toStdU16String(),
                std::filesystem::copy_options::recursive | std::filesystem::copy_options::overwrite_existing
                );
}
