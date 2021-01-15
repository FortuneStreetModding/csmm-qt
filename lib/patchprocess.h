#ifndef PATCHPROCESS_H
#define PATCHPROCESS_H

#include <QDir>
#include <QFuture>
#include <QVector>
#include "mapdescriptor.h"

namespace PatchProcess {
    /**
     * @brief openDir Loads the map descriptors from a Fortune Street directory.
     * @param dir the game directory
     * @return a future resolving to a list of MapDescriptors when loaded
     */
    QFuture<QVector<MapDescriptor>> openDir(const QDir &dir);

    /**
     * @brief saveDir Saves the map descriptors to an output game directory.
     * @param output the output directory
     * @param descriptors the descriptors
     * @param patchWiimmfi whether to patch the game with Wiimmfi
     * @return a future resolving to whether the save was successful
     */
    QFuture<bool> saveDir(const QDir &output, QVector<MapDescriptor> &descriptors, bool patchWiimmfi);
}

#endif // PATCHPROCESS_H
