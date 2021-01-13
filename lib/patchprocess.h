#ifndef PATCHPROCESS_H
#define PATCHPROCESS_H

#include <QDir>
#include <QFuture>
#include <QVector>
#include "mapdescriptor.h"

namespace PatchProcess {
    QFuture<QVector<MapDescriptor>> openDir(const QDir &dir);
}

#endif // PATCHPROCESS_H
