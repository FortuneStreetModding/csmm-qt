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
    void convertBrlytToXmlyt(const QString &brlytFile, const QString &xmlytFile);
    void convertXmlytToBrlyt(const QString &xmlytFile, const QString &brlytFile);
    QFuture<void> convertPngToTpl(const QString &pngFile, const QString &tplFile);
    QFuture<void> extractWbfsIso(const QString &wbfsFile, const QString &extractDir);
    QFuture<void> createWbfsIso(const QString &sourceDir, const QString &wbfsFile, const QString &saveId);
    QFuture<void> patchWiimmfi(const QString &wbfsFile);
    QFuture<QString> getId6(const QString &inputFile);

    class Exception : public QException {
    public:
        Exception(const QString &msgVal);
        const QString &getMessage() const;
        void raise() const override;
        Exception *clone() const override;
    private:
        QString message;
    };
}

#endif // EXEWRAPPER_H
