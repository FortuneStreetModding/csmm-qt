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
    QFuture<bool> extractArcFile(const QString &arcFile, const QString &dFolder);
    QFuture<bool> downloadCli(const QUrl &toDownloadFrom, const QString &dest, const std::function<void(double)> &progressCallback = [](double){});
    QFuture<bool> packDfolderToArc(const QString &dFolder, const QString &arcFile);
    QFuture<bool> packTurnlotFolderToArc(const QString &dFolder, const QString &arcFile);
    QFuture<bool> convertPngToTpl(const QString &pngFile, const QString &tplFile);
    QFuture<bool> extractWbfsIso(const QString &wbfsFile, const QString &extractDir);
    QFuture<bool> createWbfsIso(const QString &sourceDir, const QString &wbfsFile, const QString &markerCode, bool separateSaveGame);
    QFuture<bool> patchWiimmfi(const QString &wbfsFile);
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
