#include "patchprocess.h"

#include <QFileInfo>
#include <QRegularExpression>
#include <QTemporaryDir>
#include <QCryptographicHash>
#include "asyncfuture.h"
#include "datafileset.h"
#include "exewrapper.h"
#include "maindol.h"
#include "uimessage.h"
#include "uimenu1900a.h"
#include "vanilladatabase.h"
#include "zip/zip.h"
#include "brsar.h"
#include "bsdiff/bspatchlib.h"

namespace PatchProcess {

static void loadUiMessages(QVector<MapDescriptor> &descriptors, const QDir &dir) {
    QMap<QString, UiMessage> uiMessages;
    for (auto &locale: FS_LOCALES) {
        QFile file(dir.filePath(uiMessageCsv(locale)));
        if (file.open(QIODevice::ReadOnly)) {
            uiMessages[locale] = fileToMessage(&file);
        }
    }
    for (auto &descriptor: descriptors) {
        for (auto &locale: FS_LOCALES) {
            if (locale == "uk") {
                continue; // we treat uk as the same as en here
            }
            descriptor.names[locale] = uiMessages[locale].value(descriptor.nameMsgId);
            descriptor.descs[locale] = uiMessages[locale].value(descriptor.descMsgId);
        }
        descriptor.readFrbFileInfo(dir.filePath(PARAM_FOLDER));
    }
}

QFuture<QVector<MapDescriptor>> openDir(const QDir &dir) {
    QString mainDol = dir.filePath(MAIN_DOL);
    return AsyncFuture::observe(ExeWrapper::readSections(mainDol))
            .subscribe([=](const QVector<AddressSection> &addressSections) -> QVector<MapDescriptor> {
        QFile mainDolFile(mainDol);
        if (mainDolFile.open(QIODevice::ReadOnly)) {
            QDataStream stream(&mainDolFile);
            MainDol mainDolObj(stream, addressSections);
            auto mapDescriptors = mainDolObj.readMainDol(stream);
            loadUiMessages(mapDescriptors, dir);
            return mapDescriptors;
        } else {
            // open failed
            return QVector<MapDescriptor>();
        }
    }, []() {
        return QVector<MapDescriptor>();
    }).future();
}

static void textReplace(QString& text, QString regexStr, QString replaceStr) {
    // need to use Regex, because there are different types of whitespaces in the messages (some are U+0020 while others are U+00A0)
    // QT does not reliably match all whitespace characters with \\s
    // so we have to make the whitespace matching ourselves
    // 0x0020 is the normal space unicode character
    // 0x00a0 is the no-break space unicode character
    QString space = QString::fromUtf8("[\u0020\u00a0\uc2a0\uc220\u202f]");
    QString regexStrWithSpaces = regexStr.replace(" ", space, Qt::CaseInsensitive);
    QRegularExpression regex = QRegularExpression(regexStrWithSpaces, QRegularExpression::CaseInsensitiveOption);
    text.replace(regex, replaceStr);
}

bool hasWiimmfiText(const QDir &dir) {
    QFile file(dir.filePath(uiMessageCsv("en")));
    if (file.open(QIODevice::ReadOnly)) {
        UiMessage uimsg = fileToMessage(&file);
        return uimsg[4909].contains("Wiimmfi");
    }
    return false;
}

static void writeLocalizationFiles(QVector<MapDescriptor> &mapDescriptors, const QDir &dir, bool patchWiimmfi) {
    // Key = locale, Value = file contents
    QMap<QString, UiMessage> uiMessages;
    for (auto &locale: FS_LOCALES) {
        QFile file(dir.filePath(uiMessageCsv(locale)));
        if (file.open(QIODevice::ReadOnly)) {
            uiMessages[locale] = fileToMessage(&file);
        }
    }
    // free up the used MSG IDs
    for (auto &mapDescriptor: mapDescriptors) {
        for (auto &locale: FS_LOCALES) {
            if (mapDescriptor.nameMsgId > 0) {
                uiMessages[locale].remove(mapDescriptor.nameMsgId);
            }
            if (mapDescriptor.descMsgId > 0) {
                uiMessages[locale].remove(mapDescriptor.descMsgId);
            }
        }
    }
    // make new msg ids
    quint32 msgId = 25000;
    // add msg id for event square
    uiMessages["en"][msgId] = "Event square";
    uiMessages["uk"][msgId] = uiMessages["en"][msgId];
    uiMessages["de"][msgId] = "Ereignisfeld";
    uiMessages["fr"][msgId] = "Case Action";
    uiMessages["it"][msgId] = "Casella Avvenimento";
    uiMessages["su"][msgId] = "Casilla del Acontecimiento";
    uiMessages["jp"][msgId] = "イベントマス";
    msgId++;

    // write map descriptor names and descriptions
    for (auto &mapDescriptor: mapDescriptors) {
        mapDescriptor.nameMsgId = msgId++;
        for (auto it=uiMessages.begin(); it!=uiMessages.end(); ++it) {
            auto locale = it.key();
            auto &uiMessage = it.value();
            // write EN messages to the UK file as well (we are not differentiating here)
            if (locale == "uk") locale = "en";
            // if there is no localization for this locale, use the english variant as default
            if (!mapDescriptor.names.contains(locale) || mapDescriptor.names[locale].trimmed().isEmpty()) {
                uiMessage[mapDescriptor.nameMsgId] = mapDescriptor.names["en"];
            } else {
                uiMessage[mapDescriptor.nameMsgId] = mapDescriptor.names[locale];
            }
        }
        mapDescriptor.descMsgId = msgId++;
        for (auto it=uiMessages.begin(); it!=uiMessages.end(); ++it) {
            auto locale = it.key();
            auto &uiMessage = it.value();
            // write EN messages to the UK file as well (we are not differentiating here)
            if (locale == "uk") locale = "en";
            // if there is no localization for this locale, use the english variant as default
            if (!mapDescriptor.descs.contains(locale) || mapDescriptor.descs[locale].trimmed().isEmpty()) {
                uiMessage[mapDescriptor.descMsgId] = mapDescriptor.descs["en"];
            } else {
                uiMessage[mapDescriptor.descMsgId] = mapDescriptor.descs[locale];
            }
        }
    }
    // text replace Nintendo WFC -> Wiimmfi
    if (patchWiimmfi) {
        for (auto it=uiMessages.begin(); it!=uiMessages.end(); ++it) {
            auto locale = it.key();
            auto &uiMessage = it.value();
            auto keys = uiMessage.keys();
            for (quint32 id: qAsConst(keys)) {
                auto &text = uiMessage[id];
                if (locale == "de") {
                    textReplace(text, "die Nintendo Wi-Fi Connection", "Wiimmfi");
                    textReplace(text, "der Nintendo Wi-Fi Connection", "Wiimmfi");
                    textReplace(text, "zur Nintendo Wi-Fi Connection", "Wiimmfi");
                }
                if (locale == "fr") {
                    textReplace(text, "Wi-Fi Nintendo", "Wiimmfi");
                    textReplace(text, "CWF Nintendo", "Wiimmfi");
                    textReplace(text, "Connexion Wi-Fi Nintendo", "Wiimmfi");
                }
                if (locale == "su") {
                    textReplace(text, "Conexión Wi-Fi de Nintendo", "Wiimmfi");
                    textReplace(text, "CWF de Nintendo", "Wiimmfi");
                    textReplace(text, "Conexión Wi-Fi de<n>Nintendo", "Wiimmfi<n>");
                    textReplace(text, "Conexión<n>Wi-Fi de Nintendo", "Wiimmfi<n>");
                }
                if (locale == "jp") {
                    textReplace(text, "Ｗｉ－Ｆｉ", "Ｗｉｉｍｍｆｉ");
                }
                textReplace(text, "Nintendo Wi-Fi Connection", "Wiimmfi");
                textReplace(text, "Nintendo WFC", "Wiimmfi");
                textReplace(text, "support.nintendo.com", "https://wiimmfi.de/error");
            }
        }
    }
    // write to files
    for (auto it=uiMessages.begin(); it!=uiMessages.end(); ++it) {
        auto locale = it.key();
        auto &uiMessage = it.value();
        QFile file(dir.filePath(uiMessageCsv(locale)));
        if (file.open(QIODevice::WriteOnly)) {
            messageToFile(&file, uiMessage);
        }
    }
}

static QFuture<void> patchMainDolAsync(const QVector<MapDescriptor> &mapDescriptors, const QDir &dir) {
    QString mainDol = dir.filePath(MAIN_DOL);
    auto fut = AsyncFuture::observe(ExeWrapper::readSections(mainDol)).subscribe([=](const QVector<AddressSection> &addressSections) {
        QFile mainDolFile(mainDol);
        if (mainDolFile.open(QIODevice::ReadWrite)) {
            QDataStream stream(&mainDolFile);
            MainDol mainDolObj(stream, addressSections);
            //mainDolFile.resize(0);
            mainDolObj.writeMainDol(stream, mapDescriptors);
        }
    });
    return fut.subscribe([=](){}, [=]() {
        try {
            fut.future().waitForFinished();
        } catch (const ExeWrapper::Exception &exception) {
            throw Exception(exception.getMessage());
        }
    }).future();
}

static QFuture<void> injectMapIcons(const QVector<MapDescriptor> &mapDescriptors, const QDir &dir, const QDir &tmpDir) {
    // first check if we need to inject any map icons in the first place. We do not need to if only vanilla map icons are used.
    bool allMapIconsVanilla = true;
    for (auto &mapDescriptor: mapDescriptors){
        if (mapDescriptor.mapIcon.isEmpty()) continue;
        if (!VanillaDatabase::hasVanillaTpl(mapDescriptor.mapIcon)) {
            allMapIconsVanilla = false;
            break;
        }
    }

    if (allMapIconsVanilla) {
        auto def = AsyncFuture::deferred<bool>();
        def.complete(true);
        return def.future();
    }

    QDir tmpDirObj(tmpDir.path());

    /** maps the path of the various variants of the game_sequenceXXXXXXXX.arc files to their respective extraction path in the tmp directory */
    auto gameSequenceExtractPaths = QSharedPointer<QMap<QString, QString>>::create();
    /** maps the path of the various variants of the game_sequenceXXXXXXXX.arc files to their respective temporary path for the converted xmlyt base path */
    auto gameSequenceToXmlytBasePaths = QSharedPointer<QMap<QString, QString>>::create();
    for (auto &locale: FS_LOCALES) {
        auto gameSequencePath = dir.filePath(gameSequenceArc(locale));

        auto extractPath = tmpDirObj.filePath(QFileInfo(gameSequencePath).baseName());
        tmpDirObj.mkpath(extractPath);
        (*gameSequenceExtractPaths)[gameSequencePath] = extractPath;

        auto xmlytPath = tmpDirObj.filePath(QFileInfo(gameSequencePath).baseName() + "_");
        tmpDirObj.mkpath(xmlytPath);
        (*gameSequenceToXmlytBasePaths)[gameSequencePath] = xmlytPath;
    }
    for (auto &locale: FS_LOCALES) {
        auto gameSequencePath = dir.filePath(gameSequenceWifiArc(locale));

        auto extractPath = tmpDirObj.filePath(QFileInfo(gameSequencePath).baseName());
        tmpDirObj.mkpath(extractPath);
        (*gameSequenceExtractPaths)[gameSequencePath] = extractPath;

        auto xmlytPath = tmpDirObj.filePath(QFileInfo(gameSequencePath).baseName() + "_");
        tmpDirObj.mkpath(xmlytPath);
        (*gameSequenceToXmlytBasePaths)[gameSequencePath] = xmlytPath;
    }

    // extract the arc files
    auto extractArcTasks = AsyncFuture::combine();
    for (auto it=gameSequenceExtractPaths->begin(); it!=gameSequenceExtractPaths->end(); ++it) {
        extractArcTasks << ExeWrapper::extractArcFile(it.key(), it.value());
    }

    return extractArcTasks.subscribe([=]() {
        // convert the png files to tpl and copy them to the correct location
        auto convertTasks = AsyncFuture::combine();

        QMap<QString, QString> mapIconToTplName;
        for (auto &mapDescriptor: mapDescriptors) {
            if (mapDescriptor.mapIcon.isEmpty()) continue;
            if (VanillaDatabase::hasVanillaTpl(mapDescriptor.mapIcon)) {
                mapIconToTplName[mapDescriptor.mapIcon] = VanillaDatabase::getVanillaTpl(mapDescriptor.mapIcon);
            } else {
                QString mapIconPng = tmpDirObj.filePath(PARAM_FOLDER + "/" + mapDescriptor.mapIcon + ".png");
                QFileInfo mapIconPngInfo(mapIconPng);
                QString mapIconTpl = tmpDirObj.filePath(PARAM_FOLDER + "/" + mapDescriptor.mapIcon + ".tpl");
                auto tplName = Ui_menu_19_00a::constructMapIconTplName(mapDescriptor.mapIcon);
                if (!mapIconToTplName.contains(mapDescriptor.mapIcon)) {
                    mapIconToTplName[mapDescriptor.mapIcon] = tplName;
                }
                if (mapIconPngInfo.exists() && mapIconPngInfo.isFile()) {
                    convertTasks << AsyncFuture::observe(ExeWrapper::convertPngToTpl(mapIconPng, mapIconTpl))
                                    .subscribe([=]() {
                        auto values = gameSequenceExtractPaths->values();
                        for (auto &value: qAsConst(values)) {
                            auto mapIconTplCopy = QDir(value).filePath("arc/timg/" + tplName);
                            QFile::remove(mapIconTplCopy);
                            QFile::copy(mapIconTpl, mapIconTplCopy);
                        }
                    }).future();
                }
            }
        }

        // convert the brlyt files to xmlyt, inject the map icons and convert it back
        for (auto it=gameSequenceExtractPaths->begin(); it!=gameSequenceExtractPaths->end(); ++it) {
            auto brlytFile = QDir(it.value()).filePath("arc/blyt/ui_menu_19_00a.brlyt");
            auto xmlytFile = QDir((*gameSequenceToXmlytBasePaths)[it.key()]).filePath(QFileInfo(brlytFile).baseName() + ".xmlyt");

            ExeWrapper::convertBrlytToXmlyt(brlytFile, xmlytFile);
            bool success = Ui_menu_19_00a::injectMapIconsLayout(xmlytFile, mapIconToTplName);
            if(!success)
                qCritical() << QString("Was unable to inject map icons into ") + brlytFile;
            ExeWrapper::convertXmlytToBrlyt(xmlytFile, brlytFile);

            // strange phenomenon: when converting the xmlyt files back to brlyt using benzin, sometimes the first byte is not correctly written. This fixes it as the first byte must be an 'R'.
            QFile brlytFileObj(brlytFile);
            if (brlytFileObj.open(QIODevice::ReadWrite)) {
                brlytFileObj.seek(0);
                brlytFileObj.putChar('R');
            }
        }
        // convert the brlan files to xmlan, inject the map icons and convert it back
        for (auto it=gameSequenceExtractPaths->begin(); it!=gameSequenceExtractPaths->end(); ++it) {
            QDir brlanFileDir(QDir(it.value()).filePath("arc/anim"));
            auto brlanFilesInfo = brlanFileDir.entryInfoList({"ui_menu_19_00a_Tag_*.brlan"}, QDir::Files);
            for (auto &brlanFileInfo: brlanFilesInfo) {
                auto brlanFile = brlanFileInfo.absoluteFilePath();
                auto xmlanFile = QDir((*gameSequenceToXmlytBasePaths)[it.key()]).filePath(brlanFileInfo.baseName() + ".xmlan");

                ExeWrapper::convertBrlytToXmlyt(brlanFile, xmlanFile);
                bool success = Ui_menu_19_00a::injectMapIconsAnimation(xmlanFile, mapIconToTplName);
                if(!success) {
                    qCritical() << QString("Was unable to inject map icons into ") + brlanFile;
                }
                ExeWrapper::convertXmlytToBrlyt(xmlanFile, brlanFile);

                // strange phenomenon: when converting the xmlyt files back to brlyt using benzin, sometimes the first byte is not correctly written. This fixes it as the first byte must be an 'R'.
                QFile brlytFileObj(brlanFile);
                if (brlytFileObj.open(QIODevice::ReadWrite)) {
                    brlytFileObj.seek(0);
                    brlytFileObj.putChar('R');
                }
            }
        }

        // add dummy deferred here for edge case where there
        // are no images to convert
        auto def = AsyncFuture::deferred<void>();
        def.complete();
        convertTasks << def;

        return convertTasks.future();
    }).subscribe([=]() {
        auto packArcFileTasks = AsyncFuture::combine();
        for (auto it=gameSequenceExtractPaths->begin(); it!=gameSequenceExtractPaths->end(); ++it) {
            packArcFileTasks << ExeWrapper::packDfolderToArc(it.value(), it.key());
        }
        return packArcFileTasks.future();
    }).subscribe([]() {
        return true;
    }).future();
}


static QFuture<void> packTurnlots(const QVector<MapDescriptor> &mapDescriptors, const QDir &dir, const QDir &tmpDir) {
    // Find out which backgrounds exist
    QSet<QString> uniqueNonVanillaBackgrounds;
    for (auto &mapDescriptor: mapDescriptors){
        if (!VanillaDatabase::isVanillaBackground(mapDescriptor.background)) {
            uniqueNonVanillaBackgrounds.insert(mapDescriptor.background);
        }
    }
    // no backgrounds to do? -> finish instantly
    if (uniqueNonVanillaBackgrounds.isEmpty()) {
        auto def = AsyncFuture::deferred<bool>();
        def.complete(true);
        return def.future();
    }
    // sort the backgrounds (not really needed, but helps in debugging the console output)
    QList<QString> backgrounds = uniqueNonVanillaBackgrounds.values();
    std::sort(backgrounds.begin(), backgrounds.end());

    QDir tmpDirObj(tmpDir.path());
    auto convertTurnlotsTask = AsyncFuture::combine();
    for (auto &background : backgrounds) {
        // convert turnlot images
        for(char extChr='a'; extChr <= 'c'; ++extChr)
        {
            QString turnlotPngPath = tmpDirObj.filePath(turnlotPng(extChr, background));
            QFileInfo turnlotPngInfo(turnlotPngPath);
            QString turnlotTplPath = tmpDirObj.filePath(turnlotTpl(extChr, background));
            QFileInfo turnlotTplInfo(turnlotTplPath);
            if (!turnlotTplInfo.dir().mkpath(".")) {
                throw Exception(QString("Cannot create path %1 in temporary directory").arg(turnlotTplInfo.dir().path()));
            }
            if (turnlotPngInfo.exists() && turnlotPngInfo.isFile()) {
                QFile(turnlotTplPath).remove();
                convertTurnlotsTask << AsyncFuture::observe(ExeWrapper::convertPngToTpl(turnlotPngPath, turnlotTplPath)).future();
            }
        }
    }
    // add dummy deferred here for edge case where there
    // are no images to convert
    auto def = AsyncFuture::deferred<void>();
    def.complete();
    convertTurnlotsTask << def;

    return convertTurnlotsTask.subscribe([=]() {
        auto packArcFileTasks = AsyncFuture::combine();
        for (auto &background : backgrounds) {
            packArcFileTasks << ExeWrapper::packTurnlotFolderToArc(tmpDirObj.filePath(turnlotArcDir(background)), dir.filePath(turnlotArc(background)));
        }
        return packArcFileTasks.future();
    }).subscribe([]() {
        return true;
    }).future();
}

/*
 * @brief inject the brstm files, copy the to the correct folder, patch the brsar and prepare variables for the dol patch.
 */
void brstmInject(const QDir &output, QVector<MapDescriptor> &descriptors, const QDir &tmpDir) {
    // copy brstm files from temp dir to output
    for (auto &descriptor: descriptors) {
        auto keys = descriptor.music.keys();
        for (auto &musicType: keys) {
            auto &musicEntry = descriptor.music[musicType];
            auto brstmFileFrom = tmpDir.filePath(SOUND_STREAM_FOLDER+"/"+musicEntry.brstmBaseFilename + ".brstm");
            auto brstmFileTo = output.filePath(SOUND_STREAM_FOLDER+"/"+musicEntry.brstmBaseFilename + ".brstm");
            QFileInfo brstmFileFromInfo(brstmFileFrom);
            if (!brstmFileFromInfo.exists() || !brstmFileFromInfo.isFile()) {
                throw Exception(QString("Cannot find %1 in temporary directory").arg(brstmFileFrom));
            }
            // update the file size -> needed for patching the brsar
            musicEntry.brstmFileSize = brstmFileFromInfo.size();
            QFile(brstmFileTo).remove();
            QFile::copy(brstmFileFrom, brstmFileTo);
        }
    }
    // patch the vanilla Itast.brsar to have special CSMM entries using the included bsdiff file
    auto brsarFilePath = output.filePath(SOUND_FOLDER+"/Itast.brsar");
    QFileInfo brsarFileInfo(brsarFilePath);
    if (brsarFileInfo.exists() && brsarFileInfo.isFile()) {
        QFile brsarFile(brsarFilePath);
        if (fileSha1(brsarFilePath) == originalItsarBrsarSha1) {
            QString errors = applyBspatch(brsarFilePath, brsarFilePath, ":/Itast.brsar.bsdiff");
            if(!errors.isEmpty()) {
                throw Exception(QString("Errors occured when applying Itast.brsar.bsdiff patch to file %1:\n%2").arg(brsarFilePath, errors));
            }
        }
        // patch the special csmm entries in the brsar file
        if (brsarFile.open(QIODevice::ReadWrite)) {
            QDataStream stream(&brsarFile);
            if(Brsar::containsCsmmEntries(stream)) {
                stream.device()->seek(0);
                Brsar::patch(stream, descriptors);
            } else {
                throw Exception(QString("The brsar file %1 does not contain CSMM entries. You must either start with a vanilla fortune street or use Tools->Save Clean Itast.csmm.brsar").arg(brsarFilePath));
            }
            brsarFile.close();
        } else {
            throw Exception(QString("Could not open file %1 for read/write. %2").arg(brsarFilePath, brsarFile.errorString()));
        }
    } else {
        throw Exception(QString("The file %1 does not exist.").arg(brsarFilePath));
    }
}

QString applyBspatch(const QString &oldfile, const QString &newfile, const QString &patchfile) {
    char* errs;
    if(patchfile.startsWith(":/")) {
        QTemporaryDir tmpDir;
        auto newPatchfile = getFileCopy(patchfile, tmpDir.path());
        QByteArray oldFileBytes = oldfile.toLocal8Bit();
        char* oldFileChars = oldFileBytes.data();
        QByteArray newfileBytes = newfile.toLocal8Bit();
        char* newfileChars = newfileBytes.data();
        QByteArray patchfileBytes = newPatchfile.toLocal8Bit();
        char* patchfileChars = patchfileBytes.data();
        errs = bspatch(oldFileChars, newfileChars, patchfileChars);
    } else {
        QByteArray oldFileBytes = oldfile.toLocal8Bit();
        char* oldFileChars = oldFileBytes.data();
        QByteArray newfileBytes = newfile.toLocal8Bit();
        char* newfileChars = newfileBytes.data();
        QByteArray patchfileBytes = patchfile.toLocal8Bit();
        char* patchfileChars = patchfileBytes.data();
        errs = bspatch(oldFileChars, newfileChars, patchfileChars);
    }
    if(errs == NULL)
        return QString();
    else
        return QString(errs);
}

QString getFileCopy(const QString &fileName, const QDir &dir) {
    // copy the file to dir
    QFileInfo fileInfo(fileName);
    auto fileFrom = fileName;
    auto fileTo = dir.filePath(fileInfo.fileName());
    if(!QFile::copy(fileFrom, fileTo)) {
        throw Exception(QString("Could not copy file %1 to %2").arg(fileFrom, fileTo));
    }
    QFile file(fileTo);
    // if the file is not writable or readable, set the permissions
    if(!file.isWritable() || !file.isReadable()) {
        file.setPermissions(QFile::WriteUser | QFile::ReadUser);
    }
    return fileTo;
}

QString fileSha1(const QString &fileName) {
   QFile f(fileName);
   if (f.open(QFile::ReadOnly)) {
       QCryptographicHash hash(QCryptographicHash::Sha1);
       if (hash.addData(&f)) {
           return hash.result().toHex();
       }
   }
   return QByteArray().toHex();
}

QFuture<void> saveDir(const QDir &output, QVector<MapDescriptor> &descriptors, bool patchWiimmfi, const QDir &tmpDir) {
    writeLocalizationFiles(descriptors, output, patchWiimmfi);
    brstmInject(output, descriptors, tmpDir);
    auto fut = AsyncFuture::observe(patchMainDolAsync(descriptors, output))
            .subscribe([=]() {
        return injectMapIcons(descriptors, output, tmpDir);
    }).subscribe([=]() {
        return packTurnlots(descriptors, output, tmpDir);
    });
    return fut.subscribe([=]() {
        // copy frb files from temp dir to output
        for (auto &descriptor: descriptors) {
            for (auto &frbFile: descriptor.frbFiles) {
                if (frbFile.isEmpty()) continue;
                auto frbFileFrom = tmpDir.filePath(PARAM_FOLDER+"/"+frbFile + ".frb");
                auto frbFileTo = output.filePath(PARAM_FOLDER+"/"+frbFile + ".frb");
                QFileInfo frbFileFromInfo(frbFileFrom);
                if (!frbFileFromInfo.exists() || !frbFileFromInfo.isFile()) {
                    continue;
                }
                QFile(frbFileTo).remove();
                QFile::copy(frbFileFrom, frbFileTo);
            }
            if(!VanillaDatabase::isVanillaBackground(descriptor.background)) {
                // copy .cmpres file
                for (auto &locale: FS_LOCALES) {
                    auto cmpresFileFrom = tmpDir.filePath(bgPath(locale, descriptor.background));
                    auto cmpresFileTo = output.filePath(bgPath(locale, descriptor.background));
                    QFileInfo cmpresFileFromInfo(cmpresFileFrom);
                    if (!cmpresFileFromInfo.exists() || !cmpresFileFromInfo.isFile()) {
                        continue;
                    }
                    QFile(cmpresFileTo).remove();
                    QFile::copy(cmpresFileFrom, cmpresFileTo);
                }
                // copy .scene file
                auto sceneFileFrom = tmpDir.filePath(SCENE_FOLDER+"/"+descriptor.background + ".scene");
                auto sceneFileTo = output.filePath(SCENE_FOLDER+"/"+descriptor.background + ".scene");
                QFileInfo sceneFileFromInfo(sceneFileFrom);
                if (!sceneFileFromInfo.exists() || !sceneFileFromInfo.isFile()) {
                    continue;
                }
                QFile(sceneFileTo).remove();
                QFile::copy(sceneFileFrom, sceneFileTo);
            }
        }
    }, [=]() {
        try {
            fut.future().waitForFinished();
        } catch (const ExeWrapper::Exception &exception) {
            throw Exception(exception.getMessage());
        }
    }).future();
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
            auto suffix = QFileInfo(candidate).suffix();
            if (suffix == "yaml") {
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
        auto node = YAML::LoadFile(extractedYamlFile.toStdString());
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
                            auto suffix = QFileInfo(candidate).suffix();
                            if (suffix == "cmpres") {
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
                for (MusicEntry &musicEntry: descriptor.music) {
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
                            auto suffix = QFileInfo(candidate).suffix();
                            if (suffix == "brstm") {
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
        auto node = YAML::LoadFile(yamlFileSrc.toStdString());
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
            for (MusicEntry &musicEntry: descriptor.music) {
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

Exception::Exception(const QString &msgVal) : message(msgVal) {}
const QString &Exception::getMessage() const { return message; }
void Exception::raise() const { throw *this; }
Exception *Exception::clone() const { return new Exception(*this); }

}
