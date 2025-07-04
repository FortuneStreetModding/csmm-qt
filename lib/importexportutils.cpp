#include "importexportutils.h"

#include <fstream>
#include <QFileInfo>
#include <QTemporaryDir>
#include <QCryptographicHash>
#include <filesystem>
#include "lib/progresscanceled.h"
#include "lib/vanilladatabase.h"
#include "lib/datafileset.h"
#include "zip/zip.h"
#include "bsdiff/bspatchlib.h"
#include "bsdiff/bsdifflib.h"
#include "lib/uimessage.h"
#include "lib/await.h"
#include "lib/asyncfuture/asyncfuture.h"
#include "lib/csmmnetworkmanager.h"
#include "lib/exewrapper.h"

namespace ImportExportUtils {

bool hasWiimmfiText(const QDir &dir) {
    QFile file(dir.filePath(uiMessageCsv("en")));
    if (file.open(QIODevice::ReadOnly)) {
        UiMessage uimsg = fileToMessage(&file);
        return uimsg[4909].contains("Wiimmfi");
    }
    return false;
}

QString getSha1OfVanillaFileName(const QString &vanillaFileName) {
    QFile f(":/files/bspatches.yaml");
    if (f.open(QFile::ReadOnly)) {
        QString contents = f.readAll();
        auto yaml = YAML::Load(contents.toStdString());
        return QString::fromStdString(yaml[vanillaFileName.toStdString()]["vanilla"].as<std::string>()).toLower();
    }
    throw Exception(QString(QObject::tr("The file %1 is not a vanilla file")).arg(vanillaFileName));
}

QString fileSha1(const QString &fileName) {
   QFile f(fileName);
   if (f.open(QFile::ReadOnly)) {
       QCryptographicHash hash(QCryptographicHash::Sha1);
       if (hash.addData(&f)) {
           return hash.result().toHex().toLower();
       }
   }
   return QByteArray().toHex();
}

QString createBsdiff(const QString &oldfileStr, const QString &newfileStr, const QString &patchfileStr) {
   QFile oldfile(oldfileStr);
   if (!oldfile.exists()) {
       return QString(QObject::tr("%1 does not exist.")).arg(oldfileStr);
   }
   QFile newFile(newfileStr);
   if (!newFile.exists()) {
       return QString(QObject::tr("%1 does not exist.")).arg(newfileStr);
   }

   char *errs = bsdiff(oldfileStr.toLatin1().constData(), newfileStr.toLatin1().constData(), patchfileStr.toLatin1().constData());
   if (errs)
       return QString(errs);
   return QString();
}

QString applyBspatch(const QString &oldfileStr, const QString &newfileStr, const QString &patchfileStr) {
   unsigned char	*in_buf, *out_buf, *patch_buf;
   int				in_sz, out_sz, patch_sz;
   char         	*errs;

   QFile oldfile(oldfileStr);
   if (!oldfile.open(QFile::ReadOnly)) {
       return QString(QObject::tr("Could not open %1 for reading.")).arg(oldfileStr);
   }
   auto oldfileBytes = oldfile.readAll();
   in_buf = (unsigned char*) oldfileBytes.data();
   in_sz = oldfileBytes.length();
   oldfile.close();

   QFile patchfile(patchfileStr);
   if (!patchfile.open(QFile::ReadOnly)) {
       return QString(QObject::tr("Could not open %1 for reading.")).arg(patchfileStr);
   }
   auto patchfileBytes = patchfile.readAll();
   patch_buf = (unsigned char*) patchfileBytes.data();
   patch_sz = patchfileBytes.length();
   patchfile.close();

   errs = bspatch_mem(in_buf,in_sz, &out_buf, &out_sz, patch_buf, patch_sz, -1, -1, -1);
   if (errs)
       return QString(errs);

   QFile newfile(newfileStr);
   if (!newfile.open(QFile::WriteOnly)) {
       return QString(QObject::tr("Could not open %1 for writing.")).arg(newfileStr);
   }
   newfile.write((char*) out_buf, out_sz);
   return QString();
}

void exportYaml(const QDir &dir, const QString &yamlFileDest, const MapDescriptor &descriptor) {
    QFile saveMdToFile(yamlFileDest);
    if (saveMdToFile.open(QIODevice::WriteOnly)) {
        QTextStream stream(&saveMdToFile);
        stream.setEncoding(QStringConverter::Utf8);
        stream << descriptor.toYaml();

        // export frb files
        for (auto &frbFile: descriptor.frbFiles) {
            if (frbFile.isEmpty()) continue;
            auto frbFileFrom = dir.filePath(PARAM_FOLDER+"/"+frbFile + ".frb");
            auto frbFileTo = QFileInfo(yamlFileDest).dir().filePath(frbFile + ".frb");
            QFile(frbFileTo).remove();
            QFile::copy(frbFileFrom, frbFileTo);
        }
    }
}

static void importYamlZip(const QString &yamlZipSrc, MapDescriptor &descriptor, const QDir &importDir,
                          const std::function<void(double)> &progressCallback,
                          const QString &backgroundZipDir) {
    QSettings settings;
    QTemporaryDir intermediateDir(createTempDir("intermediate"));
    if (!intermediateDir.isValid()) {
        throw Exception(QObject::tr("Could not create an intermediate directory").toUtf8().constData());
    }
    QFileInfo yamlFileZipInfo(yamlZipSrc);

    if(!yamlFileZipInfo.exists()) {
        autoClearCache();
        throw Exception(QString(QObject::tr("Could not extract %1 to intermediate directory %2: The archive does not exist.")).arg(yamlZipSrc, intermediateDir.path()));
    }
    if(yamlFileZipInfo.size() < 100) {
        autoClearCache();
        throw Exception(QString(QObject::tr("Could not extract %1 to intermediate directory %2: The archive is corrupt or there was an error in the download. Check the logs.")).arg(yamlZipSrc, intermediateDir.path()));
    }
    QString extractedYamlFile;
    int extractResult = zip_extract(yamlZipSrc.toUtf8(), intermediateDir.path().toUtf8(), [](const char *candidate, void *arg) {
        auto yamlFilePtr = (QString *)arg;
        QFileInfo fi(candidate);
        if (fi.suffix() == "yaml" && !fi.completeBaseName().startsWith(".")) {
            *yamlFilePtr = candidate;
        }
        return 0;
    }, &extractedYamlFile);
    if (extractResult < 0) {
        autoClearCache();
        throw Exception(QString(QObject::tr("Could not extract %1 to intermediate directory %2: %3")).arg(yamlZipSrc, intermediateDir.path(), zip_strerror(extractResult)));
    }
    if (extractedYamlFile.isEmpty()) {
        throw Exception(QObject::tr("Zip file has no map descriptor"));
    }

    std::ifstream yamlStream(std::filesystem::path(extractedYamlFile.toStdU16String()));
    auto node = YAML::Load(yamlStream);
    if (descriptor.fromYaml(node)) {
        // check if <BACKGROUNDNAME>-Background.zip also needs to be extracted
        if(!VanillaDatabase::isVanillaBackground(descriptor.background)) {
            QString extractedCmpresFile = QFileInfo(extractedYamlFile).dir().filePath(descriptor.background + ".cmpres");
            // the background.cmpres file is missing -> extract the background zip as well
            if (!QFileInfo::exists(extractedCmpresFile)) {
                QString zipBackgroundStr = (backgroundZipDir.isEmpty() ? QFileInfo(yamlZipSrc).dir() : QDir(backgroundZipDir))
                        .filePath(descriptor.background + ".background.zip");
                QFileInfo zipBackground(zipBackgroundStr);
                if(zipBackground.exists()) {
                    if(zipBackground.size() < 100) {
                        autoClearCache();
                        throw Exception(QString(QObject::tr("Could not extract %1 to intermediate directory %2: The archive is corrupt or there was an error in the download. Check the logs.")).arg(zipBackgroundStr, intermediateDir.path()));
                    }
                    QString extractedCmpresFile;
                    int extractResult = zip_extract(zipBackgroundStr.toUtf8(), intermediateDir.path().toUtf8(), [](const char *candidate, void *arg){
                        auto cmpresFilePtr = (QString *)arg;
                        QFileInfo fi(candidate);
                        if (fi.suffix() == "cmpres" && !fi.completeBaseName().startsWith(".")) {
                            *cmpresFilePtr = candidate;
                        }
                        return 0;
                    }, &extractedCmpresFile);
                    if (extractResult < 0) {
                        autoClearCache();
                        throw Exception(QString(QObject::tr("Could not extract %1 to intermediate directory %2: %3")).arg(zipBackgroundStr, intermediateDir.path(), zip_strerror(extractResult)));
                    }
                    if (extractedCmpresFile.isEmpty()) {
                        autoClearCache();
                        throw Exception(QString(QObject::tr("%1 has no cmpres files")).arg(zipBackgroundStr));
                    }
                } else {
                    autoClearCache();
                    throw Exception(QString(QObject::tr("%1 was not found.")).arg(zipBackgroundStr));
                }
            }
        }
        // check if <MAPNAME>-Music.zip also needs to be extracted
        if(!descriptor.music.empty()) {
            QVector<QString> missingBrstms;
            for (auto &mapEnt: descriptor.music) {
                for (auto &musicEntry: mapEnt.second) {
                    QString extractedBrstmFile = QFileInfo(extractedYamlFile).dir().filePath(musicEntry.brstmBaseFilename + ".brstm");
                    if(!QFileInfo::exists(extractedBrstmFile)) {
                        missingBrstms.append(musicEntry.brstmBaseFilename + ".brstm");
                    }
                }
            }
            QString missingBrstmsStr = "";
            for(auto &missingBrstmStr: missingBrstms) {
                missingBrstmsStr += QString("\n- %1").arg(missingBrstmStr);
            }
            // not all required brtsm files reside yet in the intermediate directory -> extract the music zip as well
            if (missingBrstms.size() > 0) {
                QString zipMusicStr = QFileInfo(yamlZipSrc).dir().filePath(yamlFileZipInfo.baseName() + ".music.zip");
                QFileInfo zipMusic(zipMusicStr);
                if(node["music"].IsDefined()
                        && node["music"]["download"].IsSequence()
                        && node["music"]["download"].size() > 0) {
                    qInfo() << QObject::tr("Downloading music: %1.music.zip").arg(yamlFileZipInfo.baseName()).toUtf8().constData();
                    auto urlsList = node["music"]["download"].as<std::vector<std::string>>();
                    for (auto &url: urlsList) {
                        QString urlStr = QString::fromStdString(url);
                        try {
                            CSMMNetworkManager::downloadFileIfUrl(urlStr, zipMusicStr, progressCallback);
                            break;
                        } catch (const ProgressCanceled &e) {
                            throw e;
                        } catch (const std::runtime_error &e) {
                            qWarning() << QObject::tr("warning:") << e.what();
                            // download failed, try next url
                        }
                    }
                }
                zipMusic = QFileInfo(zipMusicStr);
                if(zipMusic.exists()) {
                    if(zipMusic.size() < 100) {
                        autoClearCache();
                        throw Exception(QString(QObject::tr("Could not extract %1 to intermediate directory %2: The archive is corrupt or there was an error in the download. Check the logs.")).arg(zipMusicStr, intermediateDir.path()));
                    }
                    QString extractedBrstmFile;
                    int extractResult = zip_extract(zipMusicStr.toUtf8(), intermediateDir.path().toUtf8(), [](const char *candidate, void *arg){
                        auto brstmFilePtr = (QString *)arg;
                        auto fi = QFileInfo(candidate);
                        if (fi.suffix() == "brstm" && !fi.completeBaseName().startsWith(".")) {
                            *brstmFilePtr = candidate;
                        }
                        return 0;
                    }, &extractedBrstmFile);
                    if (extractResult < 0) {
                        autoClearCache();
                        throw Exception(QString(QObject::tr("Could not extract %1 to intermediate directory %2: %3")).arg(zipMusicStr, intermediateDir.path(), zip_strerror(extractResult)));
                    }
                    if (extractedBrstmFile.isEmpty()) {
                        autoClearCache();
                        throw Exception(QString(QObject::tr("%1 has no brstm files.\nThe following .brstm files are still missing:\n%2")).arg(zipMusicStr, missingBrstmsStr));
                    }
                } else {
                    autoClearCache();
                    throw Exception(QString(QObject::tr("%1 could not be retrieved.\nThe following .brstm files are still missing:\n%2")).arg(zipMusicStr, missingBrstmsStr));
                }
            }
        }
        importYaml(extractedYamlFile, descriptor, importDir);
    } else {
        throw Exception(QString(QObject::tr("File %1 could not be parsed")).arg(extractedYamlFile));
    }
}

void importYaml(const QString &yamlFileSrc, MapDescriptor &descriptor, const QDir &importDir,
                const std::function<void(double)> &progressCallback,
                const QString &backgroundZipDir) {
    if(descriptor.names["en"] != ""){
        qInfo() << QObject::tr("Importing: %1").arg(descriptor.names["en"]).toUtf8().constData();
    }

    if (QFileInfo(yamlFileSrc).suffix() == "zip") {
        importYamlZip(yamlFileSrc, descriptor, importDir, progressCallback, backgroundZipDir);
    } else {
        if (!importDir.mkpath(PARAM_FOLDER)) {
            throw Exception(QObject::tr("could not create import param folder"));
        }
        if (!importDir.mkpath(SOUND_STREAM_FOLDER)) {
            throw Exception(QObject::tr("could not create import sound stream folder"));
        }
        if (!importDir.mkpath(SCENE_FOLDER)) {
            throw Exception(QObject::tr("could not create import scene folder"));
        }
        if (!importDir.mkpath(GAME_FOLDER)) {
            throw Exception(QObject::tr("could not create import game folder"));
        }
        for (auto &locale: FS_LOCALES) {
            if (!importDir.mkpath(bgPath(locale))) {
                throw Exception(QObject::tr("could not create import background folder for locale %1").arg(locale));
            }
        }

        std::ifstream yamlStream(std::filesystem::path(yamlFileSrc.toStdU16String()));
        auto node = YAML::Load(yamlStream);
        if (descriptor.fromYaml(node)) {
            // import frb files
            for (auto &frbFile: descriptor.frbFiles) {
                if (frbFile.isEmpty()) continue; // skip unused slots

                auto frbFileFrom = QFileInfo(yamlFileSrc).dir().filePath(frbFile + ".frb");
                QFileInfo frbFileFromInfo(frbFileFrom);
                if (!frbFileFromInfo.exists() || !frbFileFromInfo.isFile()) {
                    autoClearCache();
                    throw Exception(QString(QObject::tr("File %1 does not exist")).arg(frbFileFrom));
                }
                auto frbFileTo = importDir.filePath(PARAM_FOLDER+"/"+frbFile + ".frb");
                QFile(frbFileTo).remove();
                QFile::copy(frbFileFrom, frbFileTo);
            }
            // import map icons if needed
            if (!descriptor.mapIcon.isEmpty() && !VanillaDatabase::hasVanillaTpl(descriptor.mapIcon)) {
                auto mapIconFileFrom = QFileInfo(yamlFileSrc).dir().filePath(descriptor.mapIcon + ".png");
                auto mapIconFileTo = importDir.filePath(PARAM_FOLDER + "/" + descriptor.mapIcon + ".png");
                QFileInfo mapIconFileFromInfo(mapIconFileFrom);
                if (!mapIconFileFromInfo.exists() || !mapIconFileFromInfo.isFile()) {
                    autoClearCache();
                    throw Exception(QString(QObject::tr("File %1 does not exist")).arg(mapIconFileFrom));
                }
                QFile(mapIconFileTo).remove();
                QFile::copy(mapIconFileFrom, mapIconFileTo);
            }
            // import music if needed
            for (auto &mapEnt: descriptor.music) {
                for (auto &musicEntry: mapEnt.second) {
                    auto brstmFileFrom = QFileInfo(yamlFileSrc).dir().filePath(musicEntry.brstmBaseFilename + ".brstm");
                    QFileInfo brstmFileInfo(brstmFileFrom);
                    if (!brstmFileInfo.exists() || !brstmFileInfo.isFile()) {
                        autoClearCache();
                        throw Exception(QString(QObject::tr("File %1 does not exist")).arg(brstmFileFrom));
                    }
                    auto frbFileTo = importDir.filePath(SOUND_STREAM_FOLDER+"/"+musicEntry.brstmBaseFilename + ".brstm");
                    QFile(frbFileTo).remove();
                    QFile::copy(brstmFileFrom, frbFileTo);
                }
            }
            // import background if needed
            if(!VanillaDatabase::isVanillaBackground(descriptor.background)) {
                // copy .cmpres file
                auto cmpresFileFrom = QFileInfo(yamlFileSrc).dir().filePath(descriptor.background + ".cmpres");
                QFileInfo cmpresFileInfo(cmpresFileFrom);
                if (!cmpresFileInfo.exists() || !cmpresFileInfo.isFile()) {
                    autoClearCache();
                    throw Exception(QString(QObject::tr("File %1 does not exist")).arg(cmpresFileFrom));
                }
                for (auto &locale: FS_LOCALES) {
                    auto cmpresFileTo = importDir.filePath(bgPath(locale, descriptor.background));
                    QFile(cmpresFileTo).remove();
                    QFile::copy(cmpresFileFrom, cmpresFileTo);
                }
                // copy .scene file
                auto sceneFileFrom = QFileInfo(yamlFileSrc).dir().filePath(descriptor.background + ".scene");
                QFileInfo sceneFileInfo(sceneFileFrom);
                if (!sceneFileInfo.exists() || !sceneFileInfo.isFile()) {
                    autoClearCache();
                    throw Exception(QString(QObject::tr("File %1 does not exist")).arg(sceneFileFrom));
                }
                auto sceneFileTo = importDir.filePath(SCENE_FOLDER+"/"+descriptor.background + ".scene");
                QFile(sceneFileTo).remove();
                QFile::copy(sceneFileFrom, sceneFileTo);
                // copy turnlot images
                for(char extChr='a'; extChr <= 'c'; ++extChr)
                {
                    QString turnlotPngFrom = QFileInfo(yamlFileSrc).dir().filePath(turnlotPngFilename(extChr, descriptor.background));
                    QFileInfo turnlotPngInfo(turnlotPngFrom);
                    if (!turnlotPngInfo.exists() || !turnlotPngInfo.isFile()) {
                        autoClearCache();
                        throw Exception(QString(QObject::tr("File %1 does not exist")).arg(turnlotPngFrom));
                    }
                    QString turnlotPngTo = importDir.filePath(turnlotPng(extChr, descriptor.background));
                    QFile(turnlotPngTo).remove();
                    QFile::copy(turnlotPngFrom, turnlotPngTo);
                }
            }
            // set internal name
            descriptor.internalName = QFileInfo(yamlFileSrc).baseName();
        } else {
            throw Exception(QString(QObject::tr("File %1 could not be parsed")).arg(yamlFileSrc));
        }
    }
}

bool isMainDolVanilla(const QDir &dir) {
    auto hash = fileSha1(dir.filePath(MAIN_DOL));
    return std::find(std::begin(SHA1_VANILLA_MAIN_DOLS), std::end(SHA1_VANILLA_MAIN_DOLS), hash) != std::end(SHA1_VANILLA_MAIN_DOLS);
}

void autoClearCache() {
    QSettings settings;
    // if networkAutoClearCacheOnError hasn't been set,
    // enable it and clear the cache this time.
    if(!settings.contains("networkAutoClearCacheOnError")){
        settings.setValue("networkAutoClearCacheOnError", true);
        CSMMNetworkManager::clearNetworkCache();
    }
    // if it has been set, then only clear the cache if
    // it's set to true
    else {
        if(settings.value("networkAutoClearCacheOnError").toBool()){
            CSMMNetworkManager::clearNetworkCache();
        }
    }
}

QTemporaryDir createTempDir(QString suffix) {
    QSettings settings;
    auto chosenDirectory = settings.value("temporaryDirectory").toString();
    if(QDir(chosenDirectory).exists()){
        chosenDirectory = QString("%1/%2").arg(chosenDirectory).arg(suffix);
    }
    else {
        chosenDirectory = QString("%1%2").arg(chosenDirectory).arg(suffix);
    }
    QTemporaryDir dir(chosenDirectory);
    return dir;
}

}
