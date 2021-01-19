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
    QFuture<bool> packDfolderToArc(const QString &dFolder, const QString &arcFile);
    void convertBrlytToXmlyt(const QString &brlytFile, const QString &xmlytFile);
    void convertXmlytToBrlyt(const QString &xmlytFile, const QString &brlytFile);
    QFuture<bool> convertPngToTpl(const QString &pngFile, const QString &tplFile);
    QFuture<bool> extractWbfsIso(const QString &wbfsFile, const QString &extractDir);
    QFuture<bool> createWbfsIso(const QString &sourceDir, const QString &wbfsFile);
}

#endif // EXEWRAPPER_H
