#include "patchprocess.h"

#include <QFileInfo>
#include <QRegularExpression>
#include <QTemporaryDir>
#include "asyncfuture.h"
#include "datafileset.h"
#include "exewrapper.h"
#include "maindol.h"
#include "uimessage.h"
#include "uimenu1900a.h"
#include "vanilladatabase.h"
#include "zip/zip.h"

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
    return AsyncFuture::observe(ExeWrapper::readSections(mainDol))
            .subscribe([=](const QVector<AddressSection> &addressSections) {
        QFile mainDolFile(mainDol);
        if (mainDolFile.open(QIODevice::ReadWrite)) {
            QDataStream stream(&mainDolFile);
            MainDol mainDolObj(stream, addressSections);
            //mainDolFile.resize(0);
            mainDolObj.writeMainDol(stream, mapDescriptors);
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
            Ui_menu_19_00a::injectMapIconsLayout(xmlytFile, mapIconToTplName);
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
                Ui_menu_19_00a::injectMapIconsAnimation(xmlanFile, mapIconToTplName);
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

QFuture<void> saveDir(const QDir &output, QVector<MapDescriptor> &descriptors, bool patchWiimmfi, const QDir &tmpDir) {
    writeLocalizationFiles(descriptors, output, patchWiimmfi);
    auto fut = AsyncFuture::observe(patchMainDolAsync(descriptors, output))
            .subscribe([=]() {
        return injectMapIcons(descriptors, output, tmpDir);
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
        }
    }, [=]() {
        try {
            fut.future().waitForFinished();
        } catch (const ExeWrapper::Exception &exception) {
            throw Exception(exception.getMessage());
        }
    }).future();
}

void exportMd(const QDir &dir, const QString &mdFileDest, const MapDescriptor &descriptor) {
    QFile saveMdToFile(mdFileDest);
    if (saveMdToFile.open(QIODevice::WriteOnly)) {
        QTextStream stream(&saveMdToFile);
        stream.setCodec("UTF-8");
        stream << descriptor.toMd();

        // export frb files
        for (auto &frbFile: descriptor.frbFiles) {
            if (frbFile.isEmpty()) continue;
            auto frbFileFrom = dir.filePath(PARAM_FOLDER+"/"+frbFile + ".frb");
            auto frbFileTo = QFileInfo(mdFileDest).dir().filePath(frbFile + ".frb");
            QFile(frbFileTo).remove();
            QFile::copy(frbFileFrom, frbFileTo);
        }
    }
}

void importMd(const QDir &dir, const QString &mdFileSrc, MapDescriptor &descriptor, const QDir &tmpDir) {
    if (QFileInfo(mdFileSrc).suffix() == "zip") {
        QTemporaryDir intermediateDir;
        if (!intermediateDir.isValid()) {
            throw Exception("Could not create an intermediate directory");
        }
        QString extractedMdFile;
        int extractResult = zip_extract(mdFileSrc.toUtf8(), intermediateDir.path().toUtf8(), [](const char *candidate, void *arg) {
            auto mdFilePtr = (QString *)arg;
            auto suffix = QFileInfo(candidate).suffix();
            if (suffix == "yaml") {
                *mdFilePtr = candidate;
            }
            return 0;
        }, &extractedMdFile);
        if (extractResult < 0) {
            throw Exception("Could not extract zip file to intermediate directory");
        }
        if (extractedMdFile.isEmpty()) {
            throw Exception("Zip file has no map descriptor");
        }
        importMd(dir, extractedMdFile, descriptor, tmpDir);
    } else {
        auto node = YAML::LoadFile(mdFileSrc.toStdString());
        if (descriptor.fromMd(node)) {
            // import frb files
            for (auto &frbFile: descriptor.frbFiles) {
                if (frbFile.isEmpty()) continue; // skip unused slots

                auto frbFileFrom = QFileInfo(mdFileSrc).dir().filePath(frbFile + ".frb");
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
                auto mapIconFileFrom = QFileInfo(mdFileSrc).dir().filePath(descriptor.mapIcon + ".png");
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

            // set internal name
            descriptor.internalName = QFileInfo(mdFileSrc).baseName();
        } else {
            throw Exception(QString("File %1 could not be parsed").arg(mdFileSrc));
        }
    }
}

Exception::Exception(const QString &msgVal) : message(msgVal) {}
const QString &Exception::getMessage() const { return message; }
void Exception::raise() const { throw *this; }
Exception *Exception::clone() const { return new Exception(*this); }

}
