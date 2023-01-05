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
    const QStringList toCopyList{SOUND_STREAM_FOLDER, SCENE_FOLDER, "files/bg"};
    for (auto &toCopy: toCopyList) {
        if (QFileInfo::exists(QDir(gameInstance.getImportDir()).filePath(toCopy))) {
            std::filesystem::copy(
                        QDir(gameInstance.getImportDir()).filePath(toCopy).toStdU16String(),
                        QDir(root).filePath(toCopy).toStdU16String(),
                        std::filesystem::copy_options::recursive | std::filesystem::copy_options::overwrite_existing
                        );
        }
    }
}
