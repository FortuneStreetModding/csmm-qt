#ifndef MODLOADER_H
#define MODLOADER_H

#include "csmmmodpack.h"

namespace ModLoader {
    ModListType importModpack(const QString &modpackDir);
}

#endif // MODLOADER_H
