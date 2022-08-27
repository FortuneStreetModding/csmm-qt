#include "turnlotscenes.h"
#include "lib/await.h"
#include "lib/exewrapper.h"
#include "lib/datafileset.h"
#include "lib/vanilladatabase.h"

void TurnlotScenes::loadFiles(const QString &root, GameInstance &gameInstance, const ModListType &modList)
{
    // nothing to do
}

void TurnlotScenes::saveFiles(const QString &root, GameInstance &gameInstance, const ModListType &modList)
{
    // Find out which backgrounds exist
    // use std::set as that guarentees sorted order
    std::set<QString> uniqueNonVanillaBackgrounds;
    for (auto &mapDescriptor: gameInstance.mapDescriptors()) {
        if (!VanillaDatabase::isVanillaBackground(mapDescriptor.background)) {
            uniqueNonVanillaBackgrounds.insert(mapDescriptor.background);
        }
    }
    // no backgrounds to do? -> finish instantly
    if (uniqueNonVanillaBackgrounds.empty()) {
        return;
    }

    for (auto &background : uniqueNonVanillaBackgrounds) {
        // convert turnlot images
        for(char extChr='a'; extChr <= 'c'; ++extChr)
        {
            QString turnlotPngPath = QDir(gameInstance.getImportDir()).filePath(turnlotPng(extChr, background));
            QFileInfo turnlotPngInfo(turnlotPngPath);
            QString turnlotTplPath = QDir(root).filePath(turnlotTpl(extChr, background));
            QFileInfo turnlotTplInfo(turnlotTplPath);
            if (!turnlotTplInfo.dir().mkpath(".")) {
                throw ModException(QString("Cannot create path %1 in temporary directory").arg(turnlotTplInfo.dir().path()));
            }
            if (turnlotPngInfo.exists() && turnlotPngInfo.isFile()) {
                QFile(turnlotTplPath).remove();
                await(ExeWrapper::convertPngToTpl(turnlotPngPath, turnlotTplPath));
            }
        }
    }

    for (auto &background : uniqueNonVanillaBackgrounds) {
        await(ExeWrapper::packTurnlotFolderToArc(QDir(root).filePath(turnlotArcDir(background)), QDir(root).filePath(turnlotArc(background))));
    }
}
