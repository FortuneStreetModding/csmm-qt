#ifndef MODLOADER_H
#define MODLOADER_H

#include "csmmmodpack.h"
#include "lib/python/pythonbindings.h"

namespace ModLoader {

ModListType importModpackFile(const QString &file);
ModListType importModpackDir(const QString &modpackDir);

}

#endif // MODLOADER_H
