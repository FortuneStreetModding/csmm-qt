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
    }).future();
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

                // need to use Regex, because there are different types of whitespaces in the messages (some are U+0020 while others are U+00A0)
                if (locale == "de") {
                    text.replace(QRegularExpression("die\\sNintendo\\sWi-Fi\\sConnection", QRegularExpression::CaseInsensitiveOption), "Wiimmfi");
                    text.replace(QRegularExpression("der\\sNintendo\\sWi-Fi\\sConnection", QRegularExpression::CaseInsensitiveOption), "Wiimmfi");
                    text.replace(QRegularExpression("zur\\sNintendo\\sWi-Fi\\sConnection", QRegularExpression::CaseInsensitiveOption), "Wiimmfi");
                }
                if (locale == "fr")
                {
                    text.replace(QRegularExpression("Wi-Fi\\sNintendo", QRegularExpression::CaseInsensitiveOption), "Wiimmfi");
                    text.replace(QRegularExpression("CWF\\sNintendo", QRegularExpression::CaseInsensitiveOption), "Wiimmfi");
                    text.replace(QRegularExpression("Connexion\\sWi-Fi\\sNintendo", QRegularExpression::CaseInsensitiveOption), "Wiimmfi");
                }
                if (locale == "su")
                {
                    text.replace(QRegularExpression("Conexión\\sWi-Fi\\sde\\sNintendo", QRegularExpression::CaseInsensitiveOption), "Wiimmfi");
                    text.replace(QRegularExpression("CWF\\sde\\sNintendo", QRegularExpression::CaseInsensitiveOption), "Wiimmfi");
                    text.replace(QRegularExpression("Conexión\\sWi-Fi\\sde<n>Nintendo", QRegularExpression::CaseInsensitiveOption), "Wiimmfi<n>");
                    text.replace(QRegularExpression("Conexión<n>Wi-Fi\\sde\\sNintendo", QRegularExpression::CaseInsensitiveOption), "Wiimmfi<n>");
                }
                if (locale == "jp")
                {
                    text.replace("Ｗｉ－Ｆｉ", "Ｗｉｉｍｍｆｉ", Qt::CaseInsensitive);
                }
                text.replace(QRegularExpression("Nintendo\\sWi-Fi\\sConnection", QRegularExpression::CaseInsensitiveOption), "Wiimmfi");
                text.replace(QRegularExpression("Nintendo\\sWFC", QRegularExpression::CaseInsensitiveOption), "Wiimmfi");
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

static QFuture<bool> injectMapIcons(const QVector<MapDescriptor> &mapDescriptors, const QDir &dir) {
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

    QSharedPointer<QTemporaryDir> tempDir(new QTemporaryDir);
    QDir tempDirObj(tempDir->path());

    /** maps the path of the various variants of the game_sequenceXXXXXXXX.arc files to their respective extraction path in the tmp directory */
    QSharedPointer<QMap<QString, QString>> gameSequenceExtractPaths;
    /** maps the path of the various variants of the game_sequenceXXXXXXXX.arc files to their respective temporary path for the converted xmlyt base path */
    QSharedPointer<QMap<QString, QString>> gameSequenceToXmlytBasePaths;
    for (auto &locale: FS_LOCALES) {
        auto gameSequencePath = dir.filePath(gameSequenceArc(locale));

        auto extractPath = tempDirObj.filePath(QFileInfo(gameSequencePath).baseName());
        tempDirObj.mkpath(extractPath);
        (*gameSequenceExtractPaths)[gameSequencePath] = extractPath;

        auto xmlytPath = tempDirObj.filePath(QFileInfo(gameSequencePath).baseName() + ".");
        tempDirObj.mkpath(xmlytPath);
        (*gameSequenceToXmlytBasePaths)[gameSequencePath] = xmlytPath;
    }
    for (auto &locale: FS_LOCALES) {
        auto gameSequencePath = dir.filePath(gameSequenceWifiArc(locale));

        auto extractPath = tempDirObj.filePath(QFileInfo(gameSequencePath).baseName());
        tempDirObj.mkpath(extractPath);
        (*gameSequenceExtractPaths)[gameSequencePath] = extractPath;

        auto xmlytPath = tempDirObj.filePath(QFileInfo(gameSequencePath).baseName() + ".");
        tempDirObj.mkpath(xmlytPath);
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
        QSharedPointer<QMap<QString, QString>> mapIconToTplName;
        for (auto &mapDescriptor: mapDescriptors) {
            if (mapDescriptor.mapIcon.isEmpty()) continue;
            if (VanillaDatabase::hasVanillaTpl(mapDescriptor.mapIcon)) {
                (*mapIconToTplName)[mapDescriptor.mapIcon] = VanillaDatabase::getVanillaTpl(mapDescriptor.mapIcon);
            } else {
                QString mapIconPng = mapDescriptor.mapIcon + ".png"; // TODO change this to point to right directory
                QFileInfo mapIconPngInfo(mapIconPng);
                QString mapIconTpl = mapIconPngInfo.baseName() + ".tpl";
                auto tplName = Ui_menu_19_00a::constructMapIconTplName(mapDescriptor.mapIcon);
                if (!mapIconToTplName->contains(mapDescriptor.mapIcon)) {
                    (*mapIconToTplName)[mapDescriptor.mapIcon] = tplName;
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
            convertTasks << AsyncFuture::observe(ExeWrapper::convertBryltToXmlyt(brlytFile, xmlytFile))
                            .subscribe([=]() {
                Ui_menu_19_00a::injectMapIconsLayout(xmlytFile, *mapIconToTplName);
                return ExeWrapper::convertXmlytToBrylt(xmlytFile, brlytFile);
            }).subscribe([=]() {
                // strange phenomenon: when converting the xmlyt files back to brlyt using benzin, sometimes the first byte is not correctly written. This fixes it as the first byte must be an 'R'.
                QFile brlytFileObj(brlytFile);
                if (brlytFileObj.open(QIODevice::ReadOnly)) {
                    brlytFileObj.putChar('R');
                }
            }).future();
        }

        // convert the brlan files to xmlan, inject the map icons and convert it back
        for (auto it=gameSequenceExtractPaths->begin(); it!=gameSequenceExtractPaths->end(); ++it) {
            QDir brlanFileDir(QDir(it.value()).filePath("arc/anim"));
            auto brlanFiles = brlanFileDir.entryList({"ui_menu_19_00a_Tag_*.brlan"}, QDir::Files);
            for (auto &brlanFile: brlanFiles) {
                auto xlmanFile = QDir((*gameSequenceToXmlytBasePaths)[it.key()]).filePath(QFileInfo(brlanFile).baseName() + ".xmlan");
                convertTasks << AsyncFuture::observe(ExeWrapper::convertBryltToXmlyt(brlanFile, xlmanFile))
                                .subscribe([=]() {
                    Ui_menu_19_00a::injectMapIconsLayout(xlmanFile, *mapIconToTplName);
                    return ExeWrapper::convertXmlytToBrylt(xlmanFile, brlanFile);
                }).subscribe([=]() {
                    // strange phenomenon: when converting the xmlyt files back to brlyt using benzin, sometimes the first byte is not correctly written. This fixes it as the first byte must be an 'R'.
                    QFile brlytFileObj(brlanFile);
                    if (brlytFileObj.open(QIODevice::ReadOnly)) {
                        brlytFileObj.putChar('R');
                    }
                }).future();
            }
        }

        return convertTasks.future();
    }).subscribe([=]() -> bool {
        auto packArcFileTasks = AsyncFuture::combine();
        for (auto it=gameSequenceExtractPaths->begin(); it!=gameSequenceExtractPaths->end(); ++it) {
            packArcFileTasks << ExeWrapper::packDfolderToArc(it.value(), it.key());
        }
        return true;
    }).future();
}

QFuture<bool> saveDir(const QDir &output, QVector<MapDescriptor> &descriptors, bool patchWiimmfi) {
    writeLocalizationFiles(descriptors, output, patchWiimmfi);
    return AsyncFuture::observe(patchMainDolAsync(descriptors, output))
            .subscribe([=]() -> QFuture<bool> {
        return injectMapIcons(descriptors, output);
    }).future();
    // TODO handle wiimmfi stuff
}

}
