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
     * @param tmpDir holds temporary files to copy to output if needed
     * @return a future resolving to whether the save was successful
     */
    QFuture<bool> saveDir(const QDir &output, QVector<MapDescriptor> &descriptors, bool patchWiimmfi, const QDir &tmpDir);

    /**
     * @brief exportMd Exports the map descriptor file and .frb(s).
     * @param dir the game directory
     * @param mdFileDest the map descriptor file destination
     * @param descriptor the map descriptor
     */
    void exportMd(const QDir &dir, const QString &mdFileDest, const MapDescriptor &descriptor);

    /**
     * @brief importMd Import the map descriptor file, .frb(s), and other assets if necessary.
     * @param dir the game directory
     * @param mdFileSrc the map descriptor file destination
     * @param descriptor the descriptor to modify
     * @param tmpDir the directory to place .frb files before saving, etc.
     * @return whether the import was successful
     */
    bool importMd(const QDir &dir, const QString &mdFileSrc, MapDescriptor &descriptor, const QDir &tmpDir);
}

#endif // PATCHPROCESS_H
