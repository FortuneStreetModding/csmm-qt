#ifndef EXEWRAPPER_H
#define EXEWRAPPER_H

#include <QFuture>
#include "addressmapping.h"

#ifdef Q_OS_WIN
#define WIT_NAME "wit.exe"
#define WSZST_NAME "wszst.exe"
#define WIMGT_NAME "wimgt.exe"
#else
#define WIT_NAME "wit"
#define WSZST_NAME "wszst"
#define WIMGT_NAME "wimgt"
#endif

namespace ExeWrapper {
    QFuture<QVector<AddressSection>> readSections(const QString &inputFile);
    QFuture<QString> extractArcFile(const QString &arcFile, const QString &dFolder);
    QFuture<QString> packDfolderToArc(const QString &dFolder, const QString &arcFile);
    QFuture<QString> packTurnlotFolderToArc(const QString &dFolder, const QString &arcFile);
    QFuture<QString> extractBrresFile(const QString &brresFile, const QString &dFolder);
    QFuture<QString> packDfolderToBrres(const QString &dFolder, const QString &brresFile);
    QFuture<QString> convertPngToTpl(const QString &pngFile, const QString &tplFile, const QString &tplFormat = "RGB5A3");
    QFuture<QString> convertPngToTex(const QString &pngFile, const QString &texFile);
    QFuture<QString> extractWbfsIso(const QString &wbfsFile, const QString &extractDir);
    QFuture<QString> createWbfsIso(const QString &sourceDir, const QString &wbfsFile, const QString &markerCode, bool separateSaveGame);
    QFuture<QString> patchWiimmfi(const QString &wbfsFile);
    QFuture<QString> getId6(const QString &inputFile);

    class Exception : public QException, public std::runtime_error {
    public:
        const char *what() const noexcept override { return std::runtime_error::what(); }
        using std::runtime_error::runtime_error;
        Exception(const QString &str) : std::runtime_error(str.toStdString()) {}
        void raise() const override { throw *this; }
        Exception *clone() const override { return new Exception(*this); }
    };
}

#endif // EXEWRAPPER_H
