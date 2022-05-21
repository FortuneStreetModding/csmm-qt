#include "importexportutils.h"

#include <fstream>
#include <QFileInfo>
#include <QTemporaryDir>
#include <QCryptographicHash>
#include "lib/vanilladatabase.h"
#include "lib/datafileset.h"
#include "zip/zip.h"
#include "bsdiff/bspatchlib.h"
#include "lib/uimessage.h"
#include <filesystem>

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
    throw Exception(QString("The file %1 is not a vanilla file").arg(vanillaFileName));
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

QString applyBspatch(const QString &oldfileStr, const QString &newfileStr, const QString &patchfileStr) {
   unsigned char	*in_buf, *out_buf, *patch_buf;
   int				in_sz, out_sz, patch_sz;
   char			*errs;

   QFile oldfile(oldfileStr);
   if (!oldfile.open(QFile::ReadOnly)) {
       return QString("Could not open %1 for reading.").arg(oldfileStr);
   }
   auto oldfileBytes = oldfile.readAll();
   in_buf = (unsigned char*) oldfileBytes.data();
   in_sz = oldfileBytes.length();
   oldfile.close();

   QFile patchfile(patchfileStr);
   if (!patchfile.open(QFile::ReadOnly)) {
       return QString("Could not open %1 for reading.").arg(patchfileStr);
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
       return QString("Could not open %1 for writing.").arg(newfileStr);
   }
   newfile.write((char*) out_buf, out_sz);
   return QString();
}

void exportYaml(const QDir &dir, const QString &yamlFileDest, const MapDescriptor &descriptor) {
    QFile saveMdToFile(yamlFileDest);
    if (saveMdToFile.open(QIODevice::WriteOnly)) {
        QTextStream stream(&saveMdToFile);
        stream.setCodec("UTF-8");
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

void importYaml(const QString &yamlFileSrc, MapDescriptor &descriptor, const QDir &tmpDir) {
    if (QFileInfo(yamlFileSrc).suffix() == "zip") {
        QTemporaryDir intermediateDir;
        if (!intermediateDir.isValid()) {
            throw Exception("Could not create an intermediate directory");
        }
        QString extractedYamlFile;
        int extractResult = zip_extract(yamlFileSrc.toUtf8(), intermediateDir.path().toUtf8(), [](const char *candidate, void *arg) {
            auto yamlFilePtr = (QString *)arg;
            QFileInfo fi(candidate);
            if (fi.suffix() == "yaml" && !fi.completeBaseName().startsWith(".")) {
                *yamlFilePtr = candidate;
            }
            return 0;
        }, &extractedYamlFile);
        if (extractResult < 0) {
            throw Exception("Could not extract zip file to intermediate directory");
        }
        if (extractedYamlFile.isEmpty()) {
            throw Exception("Zip file has no map descriptor");
        }

        std::ifstream yamlStream(std::filesystem::path(extractedYamlFile.toStdU16String()));
        auto node = YAML::Load(yamlStream);
        if (descriptor.fromYaml(node)) {
            QFileInfo yamlFileZipInfo(yamlFileSrc);
            // check if <MAPNAME>-Background.zip also needs to be extracted
            if(!VanillaDatabase::isVanillaBackground(descriptor.background)) {
                QString extractedCmpresFile = QFileInfo(extractedYamlFile).dir().filePath(descriptor.background + ".cmpres");
                // the background.cmpres file is missing -> extract the background zip as well
                if (!QFileInfo::exists(extractedCmpresFile)) {
                    QString zipBackgroundStr = QFileInfo(yamlFileSrc).dir().filePath(descriptor.background + ".background.zip");
                    QFileInfo zipBackground(zipBackgroundStr);
                    if(!zipBackground.exists())
                        zipBackgroundStr = QDir::current().filePath(descriptor.background + ".background.zip");
                    zipBackground = QFileInfo(zipBackgroundStr);
                    if(zipBackground.exists()) {
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
                            throw Exception(QString("Could not extract %1 to intermediate directory").arg(zipBackgroundStr));
                        }
                        if (extractedCmpresFile.isEmpty()) {
                            throw Exception(QString("%1 has no cmpres files").arg(zipBackgroundStr));
                        }
                    } else {
                        throw Exception(QString("%1 was not found.").arg(zipBackgroundStr));
                    }
                }
            }
            // check if <MAPNAME>-Music.zip also needs to be extracted
            if(!descriptor.music.empty()) {
                bool allBrstmsAvailable = true;
                for (auto &mapEnt: descriptor.music) {
                    auto &musicEntry = mapEnt.second;
                    QString extractedBrstmFile = QFileInfo(extractedYamlFile).dir().filePath(musicEntry.brstmBaseFilename + ".brstm");
                    if(!QFileInfo::exists(extractedBrstmFile)) {
                        allBrstmsAvailable = false;
                        break;
                    }
                }
                // not all required brtsm files reside yet in the intermediate directory -> extract the music zip as well
                if (!allBrstmsAvailable) {
                    QString zipMusicStr = QFileInfo(yamlFileSrc).dir().filePath(yamlFileZipInfo.baseName() + ".music.zip");
                    QFileInfo zipMusic(zipMusicStr);
                    if(!zipMusic.exists())
                        zipMusicStr = QDir::current().filePath(yamlFileZipInfo.baseName() + ".music.zip");
                    zipMusic = QFileInfo(zipMusicStr);
                    if(zipMusic.exists()) {
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
                            throw Exception(QString("Could not extract %1 to intermediate directory").arg(zipMusicStr));
                        }
                        if (extractedBrstmFile.isEmpty()) {
                            throw Exception(QString("%1 has no brstm files").arg(zipMusicStr));
                        }
                    } else {
                        throw Exception(QString("%1 was not found.").arg(zipMusicStr));
                    }
                }
            }
            importYaml(extractedYamlFile, descriptor, tmpDir);
        } else {
            throw Exception(QString("File %1 could not be parsed").arg(extractedYamlFile));
        }
    } else {
        std::ifstream yamlStream(std::filesystem::path(yamlFileSrc.toStdU16String()));
        auto node = YAML::Load(yamlStream);
        if (descriptor.fromYaml(node)) {
            // import frb files
            for (auto &frbFile: descriptor.frbFiles) {
                if (frbFile.isEmpty()) continue; // skip unused slots

                auto frbFileFrom = QFileInfo(yamlFileSrc).dir().filePath(frbFile + ".frb");
                QFileInfo frbFileFromInfo(frbFileFrom);
                if (!frbFileFromInfo.exists() || !frbFileFromInfo.isFile()) {
                    throw Exception(QString("File %1 does not exist").arg(frbFileFrom));
                }
                if (!tmpDir.mkpath(PARAM_FOLDER)) {
                    throw Exception("Cannot create param folder in temporary directory");
                }
                auto frbFileTo = tmpDir.filePath(PARAM_FOLDER+"/"+frbFile + ".frb");
                QFile(frbFileTo).remove();
                QFile::copy(frbFileFrom, frbFileTo);
            }
            // import map icons if needed
            if (!VanillaDatabase::hasVanillaTpl(descriptor.mapIcon)) {
                auto mapIconFileFrom = QFileInfo(yamlFileSrc).dir().filePath(descriptor.mapIcon + ".png");
                auto mapIconFileTo = tmpDir.filePath(PARAM_FOLDER + "/" + descriptor.mapIcon + ".png");
                QFileInfo mapIconFileFromInfo(mapIconFileFrom);
                if (!mapIconFileFromInfo.exists() || !mapIconFileFromInfo.isFile()) {
                    throw Exception(QString("File %1 does not exist").arg(mapIconFileFrom));
                }
                if (!tmpDir.mkpath(PARAM_FOLDER)) {
                    throw Exception("Cannot create param folder in temporary directory");
                }
                QFile(mapIconFileTo).remove();
                QFile::copy(mapIconFileFrom, mapIconFileTo);
            }
            // import music if needed
            for (auto &mapEnt: descriptor.music) {
                auto &musicEntry = mapEnt.second;
                auto brstmFileFrom = QFileInfo(yamlFileSrc).dir().filePath(musicEntry.brstmBaseFilename + ".brstm");
                QFileInfo brstmFileInfo(brstmFileFrom);
                if (!brstmFileInfo.exists() || !brstmFileInfo.isFile()) {
                    throw Exception(QString("File %1 does not exist").arg(brstmFileFrom));
                }
                if (!tmpDir.mkpath(SOUND_STREAM_FOLDER)) {
                    throw Exception("Cannot create param folder in temporary directory");
                }
                auto frbFileTo = tmpDir.filePath(SOUND_STREAM_FOLDER+"/"+musicEntry.brstmBaseFilename + ".brstm");
                QFile(frbFileTo).remove();
                QFile::copy(brstmFileFrom, frbFileTo);
            }
            // import background if needed
            if(!VanillaDatabase::isVanillaBackground(descriptor.background)) {
                // copy .cmpres file
                auto cmpresFileFrom = QFileInfo(yamlFileSrc).dir().filePath(descriptor.background + ".cmpres");
                QFileInfo cmpresFileInfo(cmpresFileFrom);
                if (!cmpresFileInfo.exists() || !cmpresFileInfo.isFile()) {
                    throw Exception(QString("File %1 does not exist").arg(cmpresFileFrom));
                }
                for (auto &locale: FS_LOCALES) {
                    auto cmpresFileTo = tmpDir.filePath(bgPath(locale, descriptor.background));
                    QFileInfo cmpresFileToInfo(cmpresFileTo);
                    if (!cmpresFileToInfo.dir().mkpath(".")) {
                        throw Exception(QString("Cannot create path %1 in temporary directory").arg(cmpresFileToInfo.dir().path()));
                    }
                    QFile(cmpresFileTo).remove();
                    QFile::copy(cmpresFileFrom, cmpresFileTo);
                }
                // copy .scene file
                auto sceneFileFrom = QFileInfo(yamlFileSrc).dir().filePath(descriptor.background + ".scene");
                QFileInfo sceneFileInfo(sceneFileFrom);
                if (!sceneFileInfo.exists() || !sceneFileInfo.isFile()) {
                    throw Exception(QString("File %1 does not exist").arg(sceneFileFrom));
                }
                auto sceneFileTo = tmpDir.filePath(SCENE_FOLDER+"/"+descriptor.background + ".scene");
                QFileInfo sceneFileToInfo(sceneFileTo);
                if (!sceneFileToInfo.dir().mkpath(".")) {
                    throw Exception(QString("Cannot create path %1 in temporary directory").arg(sceneFileToInfo.dir().path()));
                }
                QFile(sceneFileTo).remove();
                QFile::copy(sceneFileFrom, sceneFileTo);
                // copy turnlot images
                for(char extChr='a'; extChr <= 'c'; ++extChr)
                {
                    QString turnlotPngFrom = QFileInfo(yamlFileSrc).dir().filePath(turnlotPngFilename(extChr, descriptor.background));
                    QFileInfo turnlotPngInfo(turnlotPngFrom);
                    if (!turnlotPngInfo.exists() || !turnlotPngInfo.isFile()) {
                        throw Exception(QString("File %1 does not exist").arg(turnlotPngFrom));
                    }
                    if (!tmpDir.mkpath(GAME_FOLDER)) {
                        throw Exception(QString("Cannot create path %1 in temporary directory").arg(GAME_FOLDER));
                    }
                    QString turnlotPngTo = tmpDir.filePath(turnlotPng(extChr, descriptor.background));
                    QFile(turnlotPngTo).remove();
                    QFile::copy(turnlotPngFrom, turnlotPngTo);
                }
            }
            // set internal name
            descriptor.internalName = QFileInfo(yamlFileSrc).baseName();
        } else {
            throw Exception(QString("File %1 could not be parsed").arg(yamlFileSrc));
        }
    }
}

bool isMainDolVanilla(const QDir &dir) {
    auto hash = fileSha1(dir.filePath(MAIN_DOL));
    return std::find(std::begin(SHA1_VANILLA_MAIN_DOLS), std::end(SHA1_VANILLA_MAIN_DOLS), hash) != std::end(SHA1_VANILLA_MAIN_DOLS);
}

}
