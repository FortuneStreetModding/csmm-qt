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
     * @param tempDir holds temporary files to copy to output if needed
     * @return a future resolving to whether the save was successful
     */
    QFuture<bool> saveDir(const QDir &output, QVector<MapDescriptor> &descriptors, bool patchWiimmfi, const QDir &tempDir);

    /**
     * @brief exportMd Exports the map descriptor file and .frb(s).
     * @param dir the game directory
     * @param mdFileDest the map descriptor file destination
     * @param descriptor the map descriptor
     */
    void exportMd(const QDir &dir, const QString &mdFileDest, const MapDescriptor &descriptor);

    bool importMd(const QDir &dir, const QString &mdFileSrc, MapDescriptor &descriptor, const QDir &tmpDir);
}

#endif // PATCHPROCESS_H
