#include "patchprocess.h"

#include <fstream>
#include <QFileInfo>
#include <QRegularExpression>
#include <QTemporaryDir>
#include <QCryptographicHash>
#include <QDirIterator>
#include <QtConcurrent>
#include "asyncfuture.h"
#include "await.h"
#include "datafileset.h"
#include "exewrapper.h"
#include "maindol.h"
#include "uimessage.h"
#include "uigame013.h"
#include "uimenu1900a.h"
#include "vanilladatabase.h"
#include "zip/zip.h"
#include "brsar.h"
#include "bsdiff/bspatchlib.h"
#include "resultscenes.h"
#include "unicodefilenameutils.h"
#include <QMessageBox>

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
            descriptor.districtNames[locale].clear();
            for (int i=0; i<descriptor.districtNameIds.size(); ++i) {
                quint32 distId = descriptor.districtNameIds[i];
                auto distName = uiMessages[locale].value(distId);
                if (5454 <= distId && distId <= 5760) {
                    auto distWord = VanillaDatabase::localeToDistrictWord()[locale];
                    distWord.replace("\\s", " ");
                    distName = distWord + distName;
                }
                descriptor.districtNames[locale].append(distName);
            }
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
            QSet<OptionalPatch> emptySet;
            MainDol mainDolObj(stream, addressSections, emptySet);
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

static inline QRegularExpression re(const QString &regexStr) {
    return QRegularExpression(regexStr,
                              QRegularExpression::CaseInsensitiveOption | QRegularExpression::UseUnicodePropertiesOption);
}

bool hasWiimmfiText(const QDir &dir) {
    QFile file(dir.filePath(uiMessageCsv("en")));
    if (file.open(QIODevice::ReadOnly)) {
        UiMessage uimsg = fileToMessage(&file);
        return uimsg[4909].contains("Wiimmfi");
    }
    return false;
}

static QString getFileCopy(const QString &fileName, const QDir &dir) {
    // copy the file to dir
    QFileInfo fileInfo(fileName);
    auto fileFrom = fileName;
    auto fileTo = dir.filePath(fileInfo.fileName());
    if(QFile::exists(fileTo))
        return fileTo;
    if(!QFile::copy(fileFrom, fileTo))
        throw Exception(QString("Could not copy file %1 to %2").arg(fileFrom, fileTo));
    QFile file(fileTo);
    // if the file is not writable or readable, set the permissions
    if(!file.isWritable() || !file.isReadable()) {
        file.setPermissions(QFile::WriteUser | QFile::ReadUser);
    }
    return fileTo;
}

static const auto REG_WIFI_DE = re("(?:die|der|zur)\\sNintendo\\sWi-Fi\\sConnection");
static const auto REG_WIFI_FR = re("(?:Wi-Fi|CWF|Connexion)\\sNintendo");
static const auto REG_WIFI_SU = re("(?:Conexión(?:\\s|(<n>))Wi-Fi|CWF)\\sde(?:\\s|(<n>))Nintendo(\\.?)");
static const auto REG_WIFI = re("Nintendo\\s(?:Wi-Fi\\sConnection|WFC)");

static void writeLocalizationFiles(QVector<MapDescriptor> &mapDescriptors, const QDir &dir, bool patchWiimmfi, bool addAuthorToMapDescription) {
    qDebug() << "Running writeLocalizationFiles()";
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
            for (quint32 districtNameId: qAsConst(mapDescriptor.districtNameIds)) {
                if (districtNameId > 0) uiMessages[locale].remove(districtNameId);
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
    msgId = 25001;
    // add msg id for free parking square
    uiMessages["en"][msgId] = "Free Parking";
    uiMessages["uk"][msgId] = uiMessages["en"][msgId];
    uiMessages["de"][msgId] = "Frei Parken";
    uiMessages["fr"][msgId] = "Parc Gratuit";
    uiMessages["it"][msgId] = "Posteggio gratuito";
    uiMessages["su"][msgId] = "Parque Gratuito";
    uiMessages["jp"][msgId] = "無料駐車場";
    msgId = 25002;
    // add msg id for free parking square description
    uiMessages["en"][msgId] = "This is just a \"free\" resting place.";
    uiMessages["uk"][msgId] = uiMessages["en"][msgId];
    uiMessages["de"][msgId] = "Hier passiert gar nichts.";
    uiMessages["fr"][msgId] = "Ce n'est qu'un espace de stationnement \"gratuit\".";
    uiMessages["it"][msgId] = "Rilassatevi! Qui non succede assolutamente nulla.";
    uiMessages["su"][msgId] = "Se trata simplemente de un lugar de descanso \"gratis\".";
    uiMessages["jp"][msgId] = "このマスに止まると<n>何も起こりません。";
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
                locale = "en";
            }
            if(addAuthorToMapDescription && !mapDescriptor.authors.isEmpty()) {
                uiMessage[mapDescriptor.descMsgId] = mapDescriptor.descs[locale] + " [" + mapDescriptor.authors.join(", ") + "]";
            } else {
                uiMessage[mapDescriptor.descMsgId] = mapDescriptor.descs[locale];
            }
        }
        int dnsz = mapDescriptor.districtNames["en"].size();
        mapDescriptor.districtNameIds = QVector<quint32>(dnsz);
        for (int i=0; i<dnsz; ++i) {
            // NOTE: We need to make sure that the district name ids for each map are contiguous
            // due to the way that Fortune Street searches the district name.
            mapDescriptor.districtNameIds[i] = msgId++;
            for (auto it=uiMessages.begin(); it!=uiMessages.end(); ++it) {
                auto locale = it.key();
                auto &uiMessage = it.value();
                // write EN messages to the UK file as well (we are not differentiating here)
                if (locale == "uk") locale = "en";
                // if there is no localization for this locale, use the english variant as default
                if (!mapDescriptor.districtNames.contains(locale)
                        || i >= mapDescriptor.districtNames[locale].size()) {
                    uiMessage[mapDescriptor.districtNameIds[i]] = mapDescriptor.districtNames["en"][i];
                } else {
                    uiMessage[mapDescriptor.districtNameIds[i]] = mapDescriptor.districtNames[locale][i];
                }
            }
        }
    }

    for (auto it=uiMessages.begin(); it!=uiMessages.end(); ++it) {
        auto locale = it.key();
        auto &uiMessage = it.value();
        auto keys = uiMessage.keys();

        auto districtWord = VanillaDatabase::localeToDistrictWord()[locale];
        auto districtReplaceRegex = re(districtWord + "<area>");

        for (quint32 id: qAsConst(keys)) {
            auto &text = uiMessage[id];

            // text replace District <area> -> <area>
            text.replace(districtReplaceRegex, "<area>");
            if (locale == "it") {
                text.replace("Quartiere<outline_off><n><outline_0><area>", "<area>", Qt::CaseInsensitive);
            }

            // strip "District" from shop squares in view board
            if (id == 2781) {
                text = "";
            }

            // TODO find out what 2963 does if anything at all

            // text replace Nintendo WFC -> Wiimmfi
            if (patchWiimmfi) {
                if (locale == "de") {
                    text.replace(REG_WIFI_DE, "Wiimmfi");
                }
                if (locale == "fr") {
                    text.replace(REG_WIFI_FR, "Wiimmfi");
                }
                if (locale == "su") {
                    text.replace(REG_WIFI_SU, "Wiimmfi\\3\\1\\2");
                }
                if (locale == "jp") {
                    text.replace("Ｗｉ－Ｆｉ", "Ｗｉｉｍｍｆｉ", Qt::CaseInsensitive);
                }
                text.replace(REG_WIFI, "Wiimmfi");
                text.replace("support.nintendo.com", "https://wiimmfi.de/error", Qt::CaseInsensitive);
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

static QFuture<void> patchMainDolAsync(const QVector<MapDescriptor> &mapDescriptors, const QDir &dir, const QSet<OptionalPatch> &optionalPatches) {
    qDebug() << "Running patchMainDolAsync()";
    QString mainDol = dir.filePath(MAIN_DOL);
    auto fut = AsyncFuture::observe(ExeWrapper::readSections(mainDol)).subscribe([=](const QVector<AddressSection> &addressSections) {
        QFile mainDolFile(mainDol);
        if (mainDolFile.open(QIODevice::ReadWrite)) {
            QDataStream stream(&mainDolFile);
            MainDol mainDolObj(stream, addressSections, optionalPatches);
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

static QFuture<void> injectMapIcons(const QVector<MapDescriptor> &mapDescriptors, const QDir &dir, const QDir &tmpDir, bool updateMinimapIcons) {
    qDebug() << "Running injectMapIcons()";
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
    /** maps the locale to the path */
    auto localeGameBoardExtractPaths = QSharedPointer<QMap<QString, QString>>::create();
    for (auto &locale: FS_LOCALES) {
        auto gameSequencePath = dir.filePath(gameSequenceArc(locale));
        (*localeGameBoardExtractPaths)[gameSequencePath] = locale;

        auto extractPath = tmpDirObj.filePath(QFileInfo(gameSequencePath).baseName());
        tmpDirObj.mkpath(extractPath);
        (*gameSequenceExtractPaths)[gameSequencePath] = extractPath;
    }
    for (auto &locale: FS_LOCALES) {
        auto gameSequencePath = dir.filePath(gameSequenceWifiArc(locale));
        (*localeGameBoardExtractPaths)[gameSequencePath] = locale;

        auto extractPath = tmpDirObj.filePath(QFileInfo(gameSequencePath).baseName());
        tmpDirObj.mkpath(extractPath);
        (*gameSequenceExtractPaths)[gameSequencePath] = extractPath;
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
                    await(AsyncFuture::observe(ExeWrapper::convertPngToTpl(mapIconPng, mapIconTpl))
                                    .subscribe([=]() {
                        //qDebug() << mapIconPng;
                        auto values = gameSequenceExtractPaths->values();
                        for (auto &value: qAsConst(values)) {
                            auto mapIconTplCopy = QDir(value).filePath("arc/timg/" + tplName);
                            QFile::remove(mapIconTplCopy);
                            QFile::copy(mapIconTpl, mapIconTplCopy);
                        }
                    }).future());
                }
            }
        }

        // minimap icons to accomodate event square
        if (updateMinimapIcons) {
            for (auto it = gameSequenceExtractPaths->begin(); it != gameSequenceExtractPaths->end(); ++it) {
                auto locale = localeToUpper((*localeGameBoardExtractPaths)[it.key()]);
                auto langDir = QString("lang%1/").arg(localeToUpper(locale));
                if(locale.isEmpty())
                    langDir = QString();
                auto minimapTmpPath = tmpDirObj.filePath(QString("minimap/%1").arg(langDir));
                tmpDirObj.mkpath(minimapTmpPath);

                auto icon2 = getFileCopy(QString(":/files/minimap/%1ui_minimap_icon2_ja.png").arg(langDir), minimapTmpPath);
                auto icon2_w = getFileCopy(QString(":/files/minimap/%1ui_minimap_icon2_w_ja.png").arg(langDir), minimapTmpPath);
                auto icon = getFileCopy(QString(":/files/minimap/%1ui_minimap_icon_ja.png").arg(langDir), minimapTmpPath);
                auto icon_w = getFileCopy(QString(":/files/minimap/%1ui_minimap_icon_w_ja.png").arg(langDir), minimapTmpPath);
                QFile::remove(QDir(it.value()).filePath("arc/timg/ui_minimap_icon2_ja.tpl"));
                await(ExeWrapper::convertPngToTpl(icon2, QDir(it.value()).filePath("arc/timg/ui_minimap_icon2_ja.tpl")));
                QFile::remove(QDir(it.value()).filePath("arc/timg/ui_minimap_icon2_w_ja.tpl"));
                await(ExeWrapper::convertPngToTpl(icon2_w, QDir(it.value()).filePath("arc/timg/ui_minimap_icon2_w_ja.tpl")));
                QFile::remove(QDir(it.value()).filePath("arc/timg/ui_minimap_icon_ja.tpl"));
                await(ExeWrapper::convertPngToTpl(icon, QDir(it.value()).filePath("arc/timg/ui_minimap_icon_ja.tpl")));
                QFile::remove(QDir(it.value()).filePath("arc/timg/ui_minimap_icon_w_ja.tpl"));
                await(ExeWrapper::convertPngToTpl(icon_w, QDir(it.value()).filePath("arc/timg/ui_minimap_icon_w_ja.tpl")));
            }
        }

        // convert the brlyt files to xmlyt, inject the map icons and convert it back
        for (auto it=gameSequenceExtractPaths->begin(); it!=gameSequenceExtractPaths->end(); ++it) {
            auto brlytFile = QDir(it.value()).filePath("arc/blyt/ui_menu_19_00a.brlyt");

            bool success = Ui_menu_19_00a::injectMapIconsLayout(brlytFile, mapIconToTplName);
            if(!success) {
                qCritical() << QString("Was unable to inject map icons into ") + brlytFile;
            }
        }
        // convert the brlan files to xmlan, inject the map icons and convert it back
        for (auto it=gameSequenceExtractPaths->begin(); it!=gameSequenceExtractPaths->end(); ++it) {
            QDir brlanFileDir(QDir(it.value()).filePath("arc/anim"));
            auto brlanFilesInfo = brlanFileDir.entryInfoList({"ui_menu_19_00a_Tag_*.brlan"}, QDir::Files);
            for (auto &brlanFileInfo: brlanFilesInfo) {
                auto brlanFile = brlanFileInfo.absoluteFilePath();

                bool success = Ui_menu_19_00a::injectMapIconsAnimation(brlanFile, mapIconToTplName);
                if(!success) {
                    qCritical() << QString("Was unable to inject map icons into ") + brlanFile;
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
    qDebug() << "Running packTurnlots()";
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
static void brstmInject(const QDir &output, QVector<MapDescriptor> &descriptors, const QDir &tmpDir) {
    qDebug() << "Running brstmInject()";
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
    // the Itast.brsar is assumed to have already been patched with Itast.brsar.bsdiff in the applyAllBspatches() method and now contains special CSMM entries
    auto brsarFilePath = output.filePath(SOUND_FOLDER+"/Itast.brsar");
    QFileInfo brsarFileInfo(brsarFilePath);
    if (brsarFileInfo.exists() && brsarFileInfo.isFile()) {
        QFile brsarFile(brsarFilePath);
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

static QString applyAllBspatches(const QDir &output) {
    qDebug() << "Running applyAllBspatches()";

    QFile f(":/files/bspatches.yaml");
    if (f.open(QFile::ReadOnly)) {
        QString contents = f.readAll();
        auto yaml = YAML::Load(contents.toStdString());
        for (auto it=yaml.begin(); it!=yaml.end(); ++it) {
            QString vanillaRelPath = QString::fromStdString(it->first.as<std::string>());
            QString vanilla = QString::fromStdString(yaml[vanillaRelPath.toStdString()]["vanilla"].as<std::string>()).toLower();
            QString patched = QString::fromStdString(yaml[vanillaRelPath.toStdString()]["patched"].as<std::string>()).toLower();
            QString bsdiffPath = ":/" + vanillaRelPath + ".bsdiff";
            QString cmpresPath = output.filePath(vanillaRelPath);
            QString sha1 = PatchProcess::fileSha1(cmpresPath);
            if (sha1 == vanilla) {
                QString errors = applyBspatch(cmpresPath, cmpresPath, bsdiffPath);
                if(!errors.isEmpty()) {
                    return QString("Errors occured when applying %1 patch to file %2:\n%3").arg(bsdiffPath, cmpresPath, errors);
                } else {
                    qDebug() << "Patched: " << cmpresPath << " sha1: " << fileSha1(cmpresPath);
                }
            } else if (sha1 == patched) {
                qDebug() << "Already patched: " << cmpresPath << " sha1: " << fileSha1(cmpresPath);
            } else {
                qDebug() << "Not patched (unknown file): " << cmpresPath << " sha1: " << fileSha1(cmpresPath);
            }
        }
    }
    return QString();
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


static void copyMapFiles(const QVector<MapDescriptor> &descriptors, const QDir &dir, const QDir &tmpDir) {
    qDebug() << "Running copyMapFiles()";
    // copy frb files from temp dir to output
    for (auto &descriptor: descriptors) {
        for (auto &frbFile: descriptor.frbFiles) {
            if (frbFile.isEmpty()) continue;
            auto frbFileFrom = tmpDir.filePath(PARAM_FOLDER+"/"+frbFile + ".frb");
            auto frbFileTo = dir.filePath(PARAM_FOLDER+"/"+frbFile + ".frb");
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
                auto cmpresFileTo = dir.filePath(bgPath(locale, descriptor.background));
                QFileInfo cmpresFileFromInfo(cmpresFileFrom);
                if (!cmpresFileFromInfo.exists() || !cmpresFileFromInfo.isFile()) {
                    continue;
                }
                QFile(cmpresFileTo).remove();
                QFile::copy(cmpresFileFrom, cmpresFileTo);
            }
            // copy .scene file
            auto sceneFileFrom = tmpDir.filePath(SCENE_FOLDER+"/"+descriptor.background + ".scene");
            auto sceneFileTo = dir.filePath(SCENE_FOLDER+"/"+descriptor.background + ".scene");
            QFileInfo sceneFileFromInfo(sceneFileFrom);
            if (!sceneFileFromInfo.exists() || !sceneFileFromInfo.isFile()) {
                continue;
            }
            QFile(sceneFileTo).remove();
            QFile::copy(sceneFileFrom, sceneFileTo);
        }
    }
}

static QFuture<void> widenResultsMapBox(const QDir &dir, const QDir &tmpDir) {
    qDebug() << "Running widenResultsMapBox()";
    // t_m_00

    QDir tmpDirObj(tmpDir.path());

    /** maps the path of the various variants of the game_sequence_resultXXXXXXXX.arc files to their respective extraction path in the tmp directory */
    auto gameResultExtractPaths = QSharedPointer<QMap<QString, QString>>::create();
    /** maps the path of the various variants of the game_sequence_resultXXXXXXXX.arc files to their respective temporary path for the converted xmlyt base path */
    auto gameResultToXmlytBasePaths = QSharedPointer<QMap<QString, QString>>::create();
    for (auto &locale: FS_LOCALES) {
        auto gameResultPath = dir.filePath(gameSequenceResultArc(locale));

        auto extractPath = tmpDirObj.filePath(QFileInfo(gameResultPath).baseName());
        tmpDirObj.mkpath(extractPath);
        (*gameResultExtractPaths)[gameResultPath] = extractPath;
    }
    // extract the arc files
    auto extractArcTasks = AsyncFuture::combine();
    for (auto it=gameResultExtractPaths->begin(); it!=gameResultExtractPaths->end(); ++it) {
        extractArcTasks << ExeWrapper::extractArcFile(it.key(), it.value());
    }

    return extractArcTasks.subscribe([=]() {
        for (auto it = gameResultExtractPaths->begin(); it != gameResultExtractPaths->end(); ++it) {
            auto brlytFile = QDir(it.value()).filePath("arc/blyt/ui_menu_011_scene.brlyt");

            ResultScenes::widenResultTitle(brlytFile);

            brlytFile = QDir(it.value()).filePath("arc/blyt/ui_game_049_scene.brlyt");

            ResultScenes::widenResultTitle(brlytFile);
        }

        auto packArcFileTasks = AsyncFuture::combine();
        for (auto it=gameResultExtractPaths->begin(); it!=gameResultExtractPaths->end(); ++it) {
            packArcFileTasks << ExeWrapper::packDfolderToArc(it.value(), it.key());
        }
        return packArcFileTasks.future();
    }).future();
}

static QFuture<void> modifyGameBoardArc(const QDir &dir, const QDir &tmpDir, bool updateMinimapIcons) {
    qDebug() << "Running modifyGameBoardArc()";

    QDir tmpDirObj(tmpDir.path());

    /** maps the path of the various variants of the game_boardXXXXXXXX.arc files to their respective extraction path in the tmp directory */
    auto gameBoardExtractPaths = QSharedPointer<QMap<QString, QString>>::create();
    /** maps the path of the various variants of the game_boardXXXXXXXX.arc files to their respective temporary path for the converted xmlyt base path */
    auto gameBoardToXmlytBasePaths = QSharedPointer<QMap<QString, QString>>::create();
    /** maps the locale to the path */
    auto localeGameBoardExtractPaths = QSharedPointer<QMap<QString, QString>>::create();
    for (auto &locale: FS_LOCALES) {
        auto gameBoardPath = dir.filePath(gameBoardArc(locale));
        (*localeGameBoardExtractPaths)[gameBoardPath] = locale;

        auto extractPath = tmpDirObj.filePath(QFileInfo(gameBoardPath).baseName());
        tmpDirObj.mkpath(extractPath);
        (*gameBoardExtractPaths)[gameBoardPath] = extractPath;

        auto xmlytPath = tmpDirObj.filePath(QFileInfo(gameBoardPath).baseName() + "_");
        tmpDirObj.mkpath(xmlytPath);
        (*gameBoardToXmlytBasePaths)[gameBoardPath] = xmlytPath;
    }

    // extract the arc files
    auto extractArcTasks = AsyncFuture::combine();
    for (auto it=gameBoardExtractPaths->begin(); it!=gameBoardExtractPaths->end(); ++it) {
        extractArcTasks << ExeWrapper::extractArcFile(it.key(), it.value());
    }

    return extractArcTasks.subscribe([=]() {
        // convert the png files to tpl and copy them to the correct location
        auto convertTasks = AsyncFuture::combine();

        if (updateMinimapIcons) {
            auto mark_eventsquare = getFileCopy(QString(":/files/ui_mark_eventsquare.png"), tmpDirObj);
            for (auto it = gameBoardExtractPaths->begin(); it != gameBoardExtractPaths->end(); ++it) {
                auto locale = localeToUpper((*localeGameBoardExtractPaths)[it.key()]);
                auto langDir = QString("lang%1/").arg(localeToUpper(locale));
                if(locale.isEmpty())
                    langDir = QString();
                auto minimapTmpPath = tmpDirObj.filePath(QString("minimap/%1").arg(langDir));
                tmpDirObj.mkpath(minimapTmpPath);
                auto icon2 = getFileCopy(QString(":/files/minimap/%1ui_minimap_icon2_ja.png").arg(langDir), minimapTmpPath);
                auto icon2_w = getFileCopy(QString(":/files/minimap/%1ui_minimap_icon2_w_ja.png").arg(langDir), minimapTmpPath);
                auto icon = getFileCopy(QString(":/files/minimap/%1ui_minimap_icon_ja.png").arg(langDir), minimapTmpPath);
                auto icon_w = getFileCopy(QString(":/files/minimap/%1ui_minimap_icon_w_ja.png").arg(langDir), minimapTmpPath);
                QFile::remove(QDir(it.value()).filePath("arc/timg/ui_minimap_icon2_ja.tpl"));
                convertTasks << ExeWrapper::convertPngToTpl(icon2, QDir(it.value()).filePath("arc/timg/ui_minimap_icon2_ja.tpl"));
                QFile::remove(QDir(it.value()).filePath("arc/timg/ui_minimap_icon2_w_ja.tpl"));
                convertTasks << ExeWrapper::convertPngToTpl(icon2_w, QDir(it.value()).filePath("arc/timg/ui_minimap_icon2_w_ja.tpl"));
                QFile::remove(QDir(it.value()).filePath("arc/timg/ui_minimap_icon_ja.tpl"));
                convertTasks << ExeWrapper::convertPngToTpl(icon, QDir(it.value()).filePath("arc/timg/ui_minimap_icon_ja.tpl"));
                QFile::remove(QDir(it.value()).filePath("arc/timg/ui_minimap_icon_w_ja.tpl"));
                convertTasks << ExeWrapper::convertPngToTpl(icon_w, QDir(it.value()).filePath("arc/timg/ui_minimap_icon_w_ja.tpl"));
                QFile::remove(QDir(it.value()).filePath("arc/timg/ui_mark_eventsquare.tpl"));
                convertTasks << ExeWrapper::convertPngToTpl(mark_eventsquare, QDir(it.value()).filePath("arc/timg/ui_mark_eventsquare.tpl"));
            }
        }

        for (auto it = gameBoardExtractPaths->begin(); it != gameBoardExtractPaths->end(); ++it) {
            auto brlytFile = QDir(it.value()).filePath("arc/blyt/ui_game_013.brlyt");

            Ui_game_013::widenDistrictName(brlytFile);
        }

        auto packArcFileTasks = AsyncFuture::combine();
        for (auto it=gameBoardExtractPaths->begin(); it!=gameBoardExtractPaths->end(); ++it) {
            packArcFileTasks << ExeWrapper::packDfolderToArc(it.value(), it.key());
        }
        return packArcFileTasks.future();
    }).subscribe([]() {
        return true;
    }).future();
}

QFuture<void> saveDir(const QDir &output, QVector<MapDescriptor> &descriptors, const QSet<OptionalPatch> &optionalPatches, const QDir &tmpDir) {
    bool patchWiimmfi = optionalPatches.contains(Wiimmfi);
    bool patchResultBoardName = optionalPatches.contains(ResultBoardName);
    bool updateMinimapIcons = optionalPatches.contains(UpdateMinimapIcons);
    bool addAuthorToMapDescription = optionalPatches.contains(AddAuthorToDescription);
    writeLocalizationFiles(descriptors, output, patchWiimmfi, addAuthorToMapDescription);
    applyAllBspatches(output);
    brstmInject(output, descriptors, tmpDir);
    auto fut = AsyncFuture::observe(patchMainDolAsync(descriptors, output, optionalPatches))
            .subscribe([=]() {
        return injectMapIcons(descriptors, output, tmpDir, updateMinimapIcons);
    }).subscribe([=]() {
        return packTurnlots(descriptors, output, tmpDir);
    }).subscribe([=]() {
        if (!patchResultBoardName) {
            auto def = AsyncFuture::deferred<void>();
            def.complete();
            return def.future();
        }
        return widenResultsMapBox(output, tmpDir);
    }).subscribe([=]() {
        return modifyGameBoardArc(output, tmpDir, updateMinimapIcons);
    });
    return fut.subscribe([=]() {
        copyMapFiles(descriptors, output, tmpDir);
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

        ufutils::unicode_ifstream yamlStream(extractedYamlFile);
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
        ufutils::unicode_ifstream yamlStream(yamlFileSrc);
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

bool isMainDolVanilla(const QDir &dir) {
    auto hash = fileSha1(dir.filePath(MAIN_DOL));
    return std::find(std::begin(SHA1_VANILLA_MAIN_DOLS), std::end(SHA1_VANILLA_MAIN_DOLS), hash) != std::end(SHA1_VANILLA_MAIN_DOLS);
}

Exception::Exception(const QString &msgVal) : message(msgVal) {}
const QString &Exception::getMessage() const { return message; }
void Exception::raise() const { throw *this; }
Exception *Exception::clone() const { return new Exception(*this); }

}
