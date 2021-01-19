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
    void convertBrlytToXmlyt(const QString &brlytFile, const QString &xmlytFile);
    void convertXmlytToBrlyt(const QString &xmlytFile, const QString &brlytFile);
    QFuture<void> convertPngToTpl(const QString &pngFile, const QString &tplFile);
}

#endif // EXEWRAPPER_H
