#ifndef PATCHPROCESS_H
#define PATCHPROCESS_H

#include <QDir>
#include <QFuture>
#include <QVector>
#include "mapdescriptor.h"
#include "lib/optionalpatch.h"

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
     * @param patchResultBoardName whether to display board name instead of "target amount" in results screen
     * @param tmpDir holds temporary files to copy to output if needed
     * @return a future resolving to whether the save was successful
     */
    QFuture<void> saveDir(const QDir &output, QVector<MapDescriptor> &descriptors, const QSet<OptionalPatch> &optionalPatches, const QDir &tmpDir);

    /**
     * @brief exportYaml Exports the map descriptor file and .frb(s).
     * @param dir the game directory
     * @param yamlFileDest the map descriptor file destination
     * @param descriptor the map descriptor
     */
    void exportYaml(const QDir &dir, const QString &yamlFileDest, const MapDescriptor &descriptor);

    /**
     * @brief importYaml Import the map descriptor file, .frb(s), and other assets if necessary.
     * @param dir the game directory
     * @param yamlFileSrc the map descriptor file destination
     * @param descriptor the descriptor to modify
     * @param tmpDir the directory to place .frb files before saving, etc.
     * @return whether the import was successful
     */
    void importYaml(const QString &yamlFileSrc, MapDescriptor &descriptor, const QDir &tmpDir);

    bool hasWiimmfiText(const QDir &dir);

    QString applyBspatch(const QString &oldfile, const QString &newfile, const QString &patchfile);

    class Exception : public QException {
    public:
        Exception(const QString &msgVal);
        const QString &getMessage() const;
        void raise() const override;
        Exception *clone() const override;
    private:
        QString message;
    };

    QString getSha1OfVanillaFileName(const QString &vanillaFileName);
    QString fileSha1(const QString &fileName);

    const QString SHA1_VANILLA_MAIN_DOLS[] = {"6c7ed3015c1aed62686d09cc068e57789cfbecd0", "327fe0460c0b5497c5b8fe17bb4b4475b6cdeed3"};
    bool isMainDolVanilla(const QDir &dir);
}

#endif // PATCHPROCESS_H
