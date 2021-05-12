#ifndef PATCHPROCESS_H
#define PATCHPROCESS_H

#include <QDir>
#include <QFuture>
#include <QVector>
#include <QCryptographicHash>
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
    QFuture<void> saveDir(const QDir &output, QVector<MapDescriptor> &descriptors, bool patchWiimmfi, const QDir &tmpDir);

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
    void importYaml(const QDir &dir, const QString &yamlFileSrc, MapDescriptor &descriptor, const QDir &tmpDir);

    class Exception : public QException {
    public:
        Exception(const QString &msgVal);
        const QString &getMessage() const;
        void raise() const override;
        Exception *clone() const override;
    private:
        QString message;
    };

    static const QString originalItsarBrsarSha1 = "0d79aa07533c23d1724bf130743e78a101191a16";
    static QString fileSha1(const QString &fileName) {
        QFile f(fileName);
        if (f.open(QFile::ReadOnly)) {
            QCryptographicHash hash(QCryptographicHash::Sha1);
            if (hash.addData(&f)) {
                return hash.result().toHex();
            }
        }
        return QByteArray().toHex();
    }
}

#endif // PATCHPROCESS_H
