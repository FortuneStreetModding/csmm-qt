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
    QFuture<void> extractArcFile(const QString &arcFile, const QString &dFolder);
    QFuture<void> packDfolderToArc(const QString &dFolder, const QString &arcFile);
    QFuture<void> packTurnlotFolderToArc(const QString &dFolder, const QString &arcFile);
    QFuture<void> convertPngToTpl(const QString &pngFile, const QString &tplFile);
    QFuture<void> extractWbfsIso(const QString &wbfsFile, const QString &extractDir);
    QFuture<void> createWbfsIso(const QString &sourceDir, const QString &wbfsFile, const QString &saveId);
    QFuture<void> patchWiimmfi(const QString &wbfsFile);
    QFuture<QString> getId6(const QString &inputFile);

    class Exception : public QException, public std::runtime_error {
    public:
        const char *what() const override { return std::runtime_error::what(); }
        using std::runtime_error::runtime_error;
        Exception(const QString &str) : std::runtime_error(str.toStdString()) {}
        void raise() const override { throw *this; }
        Exception *clone() const override { return new Exception(*this); }
    };
}

#endif // EXEWRAPPER_H
