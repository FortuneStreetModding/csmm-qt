#ifndef IMPORTEXPORTUTILS_H
#define IMPORTEXPORTUTILS_H

#include <QDir>
#include <QFuture>
#include <QVector>
#include "mapdescriptor.h"

namespace ImportExportUtils {
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

    QString getSha1OfVanillaFileName(const QString &vanillaFileName);

    QString fileSha1(const QString &fileName);

    QString applyBspatch(const QString &oldfileStr, const QString &newfileStr, const QString &patchfileStr);

    const QString SHA1_VANILLA_MAIN_DOLS[] = {"6c7ed3015c1aed62686d09cc068e57789cfbecd0", "327fe0460c0b5497c5b8fe17bb4b4475b6cdeed3"};
    bool isMainDolVanilla(const QDir &dir);

    bool hasWiimmfiText(const QDir &dir);

    class Exception : public QException, public std::runtime_error {
    public:
        using std::runtime_error::runtime_error;
        const char *what() const override { return std::runtime_error::what(); }
        Exception(const QString &str) : std::runtime_error(str.toStdString()) {}
        void raise() const override { throw *this; }
        Exception *clone() const override { return new Exception(*this); }
    };
}

#endif // IMPORTEXPORTUTILS_H