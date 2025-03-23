#ifndef MODLOADER_H
#define MODLOADER_H

#include "csmmmodpack.h"
#include "lib/python/pythonbindings.h"

namespace ModLoader {

/**
 * Loads the mod lists in the files and aggregates them.
 * @return a pair containing (1) a list of mods and (2) temporary directories holding the modpacks if applicable
 */
std::pair<ModListType, std::vector<std::shared_ptr<QTemporaryDir>>> importModpackCollection(const QVector<QString> &modlistColl, const std::vector<std::shared_ptr<QTemporaryDir>> tmpDirs);

/**
 * Loads the mod list from the file, or the default modlist if file is an empty string.
 * @return a pair containing (1) a list of mods and (2) a temporary directory holding the modpack if applicable
 */
std::pair<ModListType, std::shared_ptr<QTemporaryDir>> importModpackFile(const QString &file, const std::shared_ptr<QTemporaryDir> tmpDir);

}

#endif // MODLOADER_H
