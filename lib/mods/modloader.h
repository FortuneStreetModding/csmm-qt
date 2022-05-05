#ifndef MODLOADER_H
#define MODLOADER_H

#include "csmmmodpack.h"
#include "lib/python/pythonbindings.h"

namespace ModLoader {

/**
 * @return a pair containing (1) a list of mods and (2) a temporary directory holding the modpack if applicable
 */
std::pair<ModListType, std::shared_ptr<QTemporaryDir>> importModpackFile(const QString &file);

}

#endif // MODLOADER_H
