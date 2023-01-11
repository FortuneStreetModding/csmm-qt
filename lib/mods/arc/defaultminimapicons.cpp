#include "defaultminimapicons.h"
#include "lib/await.h"
#include "lib/fslocale.h"
#include "lib/datafileset.h"
#include "lib/exewrapper.h"

static QString getFileCopy(const QString &fileName, const QDir &dir) {
    // copy the file to dir
    QFileInfo fileInfo(fileName);
    auto fileFrom = fileName;
    auto fileTo = dir.filePath(fileInfo.fileName());
    if(QFile::exists(fileTo))
        return fileTo;
    if(!QFile::copy(fileFrom, fileTo))
        throw ModException(QString("Could not copy file %1 to %2").arg(fileFrom, fileTo));
    QFile file(fileTo);
    // if the file is not writable or readable, set the permissions
    if(!file.isWritable() || !file.isReadable()) {
        file.setPermissions(QFile::WriteUser | QFile::ReadUser);
    }
    return fileTo;
}

QMap<QString, ArcFileInterface::ModifyArcFunction> DefaultMinimapIcons::modifyArcFile()
{    
    QMap<QString, ArcFileInterface::ModifyArcFunction> result;

    for (auto &locale: FS_LOCALES) {
        auto uppercasedLocale = localeToUpper(locale);
        QString langDir;
        if (!uppercasedLocale.isEmpty()) {
            langDir = QString("lang%1/").arg(uppercasedLocale);
        }

        auto minimapTemp = QSharedPointer<QTemporaryDir>::create();
        if (!minimapTemp->isValid()) {
            throw ModException(QString("could not create temporary directory %1").arg(minimapTemp->path()));
        }

        auto icon2 = getFileCopy(QString(":/files/minimap/%1ui_minimap_icon2_ja.png").arg(langDir), minimapTemp->path());
        auto icon2_w = getFileCopy(QString(":/files/minimap/%1ui_minimap_icon2_w_ja.png").arg(langDir), minimapTemp->path());
        auto icon = getFileCopy(QString(":/files/minimap/%1ui_minimap_icon_ja.png").arg(langDir), minimapTemp->path());
        auto icon_w = getFileCopy(QString(":/files/minimap/%1ui_minimap_icon_w_ja.png").arg(langDir), minimapTemp->path());
        auto mark_eventsquare = getFileCopy(QString(":/files/ui_mark_eventsquare.png"), minimapTemp->path());

        result[gameSequenceArc(locale)] = [=](const QString &, GameInstance *, const ModListType &, const QString &tmpDir) {
            (void)minimapTemp; // keep alive

            QFile::remove(QDir(tmpDir).filePath("arc/timg/ui_minimap_icon2_ja.tpl"));
            await(ExeWrapper::convertPngToTpl(icon2, QDir(tmpDir).filePath("arc/timg/ui_minimap_icon2_ja.tpl")));
            QFile::remove(QDir(tmpDir).filePath("arc/timg/ui_minimap_icon2_w_ja.tpl"));
            await(ExeWrapper::convertPngToTpl(icon2_w, QDir(tmpDir).filePath("arc/timg/ui_minimap_icon2_w_ja.tpl")));
            QFile::remove(QDir(tmpDir).filePath("arc/timg/ui_minimap_icon_ja.tpl"));
            await(ExeWrapper::convertPngToTpl(icon, QDir(tmpDir).filePath("arc/timg/ui_minimap_icon_ja.tpl")));
            QFile::remove(QDir(tmpDir).filePath("arc/timg/ui_minimap_icon_w_ja.tpl"));
            await(ExeWrapper::convertPngToTpl(icon_w, QDir(tmpDir).filePath("arc/timg/ui_minimap_icon_w_ja.tpl")));
        };

        result[gameBoardArc(locale)] = [=](const QString &, GameInstance *, const ModListType &, const QString &tmpDir) {
            (void)minimapTemp; // keep alive

            QFile::remove(QDir(tmpDir).filePath("arc/timg/ui_minimap_icon2_ja.tpl"));
            await(ExeWrapper::convertPngToTpl(icon2, QDir(tmpDir).filePath("arc/timg/ui_minimap_icon2_ja.tpl")));
            QFile::remove(QDir(tmpDir).filePath("arc/timg/ui_minimap_icon2_w_ja.tpl"));
            await(ExeWrapper::convertPngToTpl(icon2_w, QDir(tmpDir).filePath("arc/timg/ui_minimap_icon2_w_ja.tpl")));
            QFile::remove(QDir(tmpDir).filePath("arc/timg/ui_minimap_icon_ja.tpl"));
            await(ExeWrapper::convertPngToTpl(icon, QDir(tmpDir).filePath("arc/timg/ui_minimap_icon_ja.tpl")));
            QFile::remove(QDir(tmpDir).filePath("arc/timg/ui_minimap_icon_w_ja.tpl"));
            await(ExeWrapper::convertPngToTpl(icon_w, QDir(tmpDir).filePath("arc/timg/ui_minimap_icon_w_ja.tpl")));
            QFile::remove(QDir(tmpDir).filePath("arc/timg/ui_mark_eventsquare.tpl"));
            await(ExeWrapper::convertPngToTpl(mark_eventsquare, QDir(tmpDir).filePath("arc/timg/ui_mark_eventsquare.tpl")));
        };
    }
    return result;
}
