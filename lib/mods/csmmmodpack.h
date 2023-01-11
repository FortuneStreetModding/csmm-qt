#ifndef CSMMMODPACK_H
#define CSMMMODPACK_H

#include "csmmmod.h"
#include "lib/await.h"
#include "lib/asyncfuture.h"
#include "lib/exewrapper.h"
#include "lib/mods/csmmmod.h"
#include "lib/python/pythonbindings.h"
#include "lib/importexportutils.h"
#include "lib/datafileset.h"

class CSMMModpack {
public:
    template<class InputIterator>
    CSMMModpack(GameInstance &gameInstance, InputIterator modsStart, InputIterator modsEnd) : gameInstance(gameInstance) {
        QMap<int, QHash<QString, CSMMModHolder>> modTable;
        QHash<QString, CSMMModHolder> priorityIndependentModTable;
        QMultiHash<QString, QString> afters;
        for (auto it = modsStart; it != modsEnd; ++it) {
            modTable[-(*it)->priority()][(*it)->modId()] = *it; // store negative priority so that higher priority is visited first
            if (priorityIndependentModTable.contains((*it)->modId())) {
                throw ModException(QString("duplicate mod '%1' detected").arg((*it)->modId()));
            }
            priorityIndependentModTable[(*it)->modId()] = *it;
            auto beforesForMod = (*it)->before(), aftersForMod = (*it)->after();
            for (auto &nxtId: beforesForMod) {
                afters.insert(nxtId, (*it)->modId());
            }
            for (auto &prevId: aftersForMod) {
                afters.insert((*it)->modId(), prevId);
            }
        }

        // marks for the vertices (mod ids) in the topological sort
        QSet<QString> temporaryMarks, permanentMarks;

        for (auto &modTableForPriority: modTable) {
            // do a reverse toplogical sort

            std::function<void(const CSMMModHolder &)> visit = [&](const CSMMModHolder &mod) {
                auto modid = mod->modId();

                if (permanentMarks.contains(modid)) {
                    return; // already visited
                } else if (temporaryMarks.contains(modid)) {
                    throw ModException(QString("circular dependency spotted with mod '%1'").arg(modid));
                }

                temporaryMarks.insert(modid);

                auto depends = mod->depends();

                for (auto &nxt: depends) {
                    if (!priorityIndependentModTable.contains(nxt)) {
                        throw ModException(QString("'%1' is missing dependency '%2'").arg(modid, nxt));
                    }
                }

                auto after = afters.values(modid);

                for (auto &nxt: after) {
                    if (modTableForPriority.contains(nxt)) {
                        visit(modTableForPriority[nxt]);
                    }
                }

                temporaryMarks.remove(modid);
                permanentMarks.insert(modid);

                modList.append(mod);
            };

            for (auto &mod: modTableForPriority) {
                visit(mod);
            }
        }
    }

    void load(const QString &root) {
        QHash<QString, UiMessage> messageFiles;
        QHash<QString, QMap<QString, UiMessageInterface::LoadMessagesFunction>> modToLoaders;

        for (auto &mod: modList) {
            auto uiMessageInterface = mod.getCapability<UiMessageInterface>();
            if (uiMessageInterface) {
                auto loaders = uiMessageInterface->loadUiMessages();
                for (auto it=loaders.begin(); it!=loaders.end(); ++it) {
                    messageFiles[it.key()]; // create blank uimessage
                }
                modToLoaders[mod->modId()] = std::move(loaders);
            }
        }

        for (auto it=messageFiles.begin(); it!=messageFiles.end(); ++it) {
            QFile file(QDir(root).filePath(it.key()));
            if (!file.open(QFile::ReadOnly)) {
                throw ModException(QString("could not open file %1").arg(it.key()));
            }
            it.value() = fileToMessage(&file);
        }

        for (auto &mod: modList) {
            qDebug() << "loading mod" << mod->modId();

            auto generalFileInterface = mod.getCapability<GeneralInterface>();
            if (generalFileInterface) {
                qDebug() << "loading general interface for" << mod->modId();

                generalFileInterface->loadFiles(root, &gameInstance.get(), modList);
            }

            if (modToLoaders.contains(mod->modId())) {
                qDebug() << "loading UI messages for" << mod->modId();

                auto &loaders = modToLoaders[mod->modId()];
                for (auto it = loaders.begin(); it != loaders.end(); ++it) {
                    it.value()(root, &gameInstance.get(), modList, &messageFiles[it.key()]);
                }
            }
        }
    }

    void backupAndRestore(const QTemporaryDir &tempDir, const QString &root, bool restore) {
        QString mainDolStr = QDir(root).filePath(MAIN_DOL);
        QFile mainDolFile(mainDolStr);
        QFileInfo mainDolInfo(mainDolStr);

        QString mainDolTempStr = QDir(root).filePath(MAIN_DOL_TEMP_BACKUP);
        QFile mainDolTempFile(mainDolTempStr);
        QFileInfo mainDolTempInfo(mainDolTempStr);

        QString mainDolVanillaStr = QDir(root).filePath(MAIN_DOL_VANILLA_BACKUP);
        QFile mainDolVanillaFile(mainDolVanillaStr);
        QFileInfo mainDolVanillaInfo(mainDolVanillaStr);

        QString mainDolCsmmStr = QDir(root).filePath(MAIN_DOL_CSMM_BACKUP);
        QFile mainDolCsmmFile(mainDolCsmmStr);
        QFileInfo mainDolCsmmInfo(mainDolCsmmStr);

        QString mainDolBsdiffStr = tempDir.filePath(mainDolInfo.fileName() + ".bsdiff");
        QFileInfo mainDolBsdiffInfo(mainDolBsdiffStr);

        QString itastBrsarStr = QDir(root).filePath(ITAST_BRSAR);
        QFile itastBrsarFile(itastBrsarStr);
        QFileInfo itastBrsarInfo(itastBrsarStr);

        QString itastBrsarTempStr = QDir(root).filePath(ITAST_BRSAR_TEMP_BACKUP);
        QFile itastBrsarTempFile(itastBrsarTempStr);
        QFileInfo itastBrsarTempInfo(itastBrsarTempStr);

        QString itastBrsarCsmmStr = QDir(root).filePath(ITAST_BRSAR_CSMM_BACKUP);
        QFile itastBrsarCsmmFile(itastBrsarCsmmStr);
        QFileInfo itastBrsarCsmmInfo(itastBrsarCsmmStr);

        QString itastBrsarBsdiffStr = tempDir.filePath(itastBrsarInfo.fileName() + ".bsdiff");
        QFileInfo itastBrsarBsdiffInfo(itastBrsarBsdiffStr);

        if(!restore) {
            if(mainDolTempInfo.exists()) {
                // something went wrong last time -> restore main.dol.temp.bak
                mainDolFile.remove();
                mainDolTempFile.rename(mainDolStr);
            }
            if(itastBrsarTempInfo.exists()) {
                // something went wrong last time -> restore Itast.brsar.temp.bak
                itastBrsarFile.remove();
                itastBrsarTempFile.rename(itastBrsarStr);
            }
            // save the main.dol changes that happened since last time CSMM touched it
            //   (bsdiff main.dol.csmm.bak <-> main.dol)
            if(mainDolCsmmInfo.exists()) {
                ImportExportUtils::createBsdiff(mainDolCsmmStr, mainDolStr, mainDolBsdiffStr);
            }
            if(itastBrsarCsmmInfo.exists()) {
                // save the Itast.brsar changes that happened since last time CSMM touched it
                //   (bsdiff Itast.brsar.csmm.bak <-> Itast.brsar)
                ImportExportUtils::createBsdiff(itastBrsarCsmmStr, itastBrsarStr, itastBrsarBsdiffStr);
                // backup the current Itast.brsar -> Itast.brsar.temp.bak in case something goes wrong
                itastBrsarFile.rename(itastBrsarTempStr);
                // restore the Itast.brsar.csmm.bak -> Itast.brsar.bak
                itastBrsarCsmmFile.copy(itastBrsarStr);
            }
            if(mainDolVanillaInfo.exists()) {
                // restore the original main.dol
                //   (main.dol -> main.dol.temp.bak)
                //   (main.dol.orig.bak -> main.dol)
                mainDolFile.rename(mainDolTempStr);
                mainDolVanillaFile.copy(mainDolStr);
            } else {
                // backup the original main.dol
                //  (main.dol -> main.dol.orig.bak)
                mainDolFile.copy(mainDolVanillaStr);
            }
        } else {
            // backup the csmm modified main.dol
            //  (main.dol -> main.dol.csmm.bak)
            if(mainDolCsmmInfo.exists())
                mainDolCsmmFile.remove();
            mainDolFile.copy(mainDolCsmmStr);

            // backup the csmm modified Itast.brsar
            //  (Itast.brsar -> Itast.brsar.csmm.bak)
            if(itastBrsarCsmmInfo.exists())
                itastBrsarCsmmFile.remove();
            itastBrsarFile.copy(itastBrsarCsmmStr);

            // apply the saved main.dol changes that happened since last time CSMM touched it
            //   (bspatch main.dol)
            if(mainDolBsdiffInfo.exists()) {
                ImportExportUtils::applyBspatch(mainDolStr, mainDolStr, mainDolBsdiffStr);
            }
            // apply the saved Itast.brsar changes that happened since last time CSMM touched it
            //   (bspatch Itast.brsar)
            if(itastBrsarBsdiffInfo.exists()) {
                ImportExportUtils::applyBspatch(itastBrsarStr, itastBrsarStr, itastBrsarBsdiffStr);
            }

            if(mainDolTempInfo.exists()) {
                // everything done -> delete main.dol.temp.bak
                mainDolTempFile.remove();
            }
            if(itastBrsarTempInfo.exists()) {
                // everything done -> delete Itast.brsar.temp.bak
                itastBrsarTempFile.remove();
            }
        }
    }

    void save(const QString &root, const std::function<void(double)> &progressCallback = [](double) {}) {
        QHash<QString, QMap<QString, UiMessageInterface::SaveMessagesFunction>> messageSavers;
        QHash<QString, QMap<QString, ArcFileInterface::ModifyArcFunction>> arcModifiers;
        QMap<QString, UiMessage> messageFiles;
        QTemporaryDir arcFilesDir;
        QSet<QString> arcFiles;
        if (!arcFilesDir.isValid()) {
            throw ModException(QString("error creating temporary directory: %1").arg(arcFilesDir.errorString()));
        }

        backupAndRestore(arcFilesDir, root, false);

        for (auto &mod: modList) {
            auto uiMessageInterface = mod.getCapability<UiMessageInterface>();
            if (uiMessageInterface) {
                auto savers = uiMessageInterface->saveUiMessages();
                for (auto it = savers.begin(); it != savers.end(); ++it) {
                    messageFiles[it.key()]; // create blank uimessage
                }
                messageSavers[mod->modId()] = std::move(savers);
            }
            auto arcFileInterface = mod.getCapability<ArcFileInterface>();
            if (arcFileInterface) {
                auto modArcModifiers = arcFileInterface->modifyArcFile();
                for (auto it=modArcModifiers.begin(); it!=modArcModifiers.end(); ++it) {
                    arcFiles.insert(it.key());
                }
                arcModifiers[mod->modId()] = std::move(modArcModifiers);
            }
        }

        for (auto it=messageFiles.begin(); it!=messageFiles.end(); ++it) {
            QFile file(QDir(root).filePath(it.key()));
            if (!file.open(QFile::ReadOnly)) {
                throw ModException(QString("could not open file %1").arg(it.key()));
            }
            messageFiles[it.key()] = fileToMessage(&file);
        }

        {
            int i=0;
            for (auto &arcFile: arcFiles) {
                qInfo() << "extracting arc file" << arcFile;
                progressCallback((double)i / arcFiles.size() / 3);
                QDir(arcFilesDir.path()).mkpath(arcFile);
                await(ExeWrapper::extractArcFile(QDir(root).filePath(arcFile), arcFilesDir.filePath(arcFile)));
                ++i;
            }
        }

        for (int i=0; i<modList.size(); ++i) {
            auto &mod = modList[i];
            qInfo() << "saving mod" << mod->modId();

            progressCallback((1 + (double)i / modList.size()) / 3);

            auto uiMessageInterface = mod.getCapability<UiMessageInterface>();
            if (uiMessageInterface) {
                qInfo() << "allocating ui messages for" << mod->modId();
                uiMessageInterface->allocateUiMessages(root, &gameInstance.get(), modList);
            }
            if (messageSavers.contains(mod->modId())) {
                qInfo() << "saving ui messages for" << mod->modId();
                auto &savers = messageSavers[mod->modId()];
                for (auto it=savers.begin(); it!=savers.end(); ++it) {
                    it.value()(root, &gameInstance.get(), modList, &messageFiles[it.key()]);
                }
            }
            auto generalFileInterface = mod.getCapability<GeneralInterface>();
            if (generalFileInterface) {
                qInfo() << "processing general interface for" << mod->modId();
                generalFileInterface->saveFiles(root, &gameInstance.get(), modList);
            }
            if (arcModifiers.contains(mod->modId())) {
                qInfo() << "saving arc files for" << mod->modId();
                auto &modifiers = arcModifiers[mod->modId()];
                for (auto it=modifiers.begin(); it!=modifiers.end(); ++it) {
                    it.value()(root, &gameInstance.get(), modList, arcFilesDir.filePath(it.key()));
                }
            }
        }

        backupAndRestore(arcFilesDir, root, true);

        for (auto it=messageFiles.begin(); it!=messageFiles.end(); ++it) {
            QFile file(QDir(root).filePath(it.key()));
            if (!file.open(QFile::WriteOnly)) {
                throw ModException(QString("could not open file %1").arg(it.key()));
            }
            messageToFile(&file, messageFiles[it.key()]);
        }

        {
            int i = 0;
            for (auto &arcFile: arcFiles) {
                qInfo() << "saving arc file" << arcFile;
                progressCallback((2 + (double)i / arcFiles.size()) / 3);
                await(ExeWrapper::packDfolderToArc(arcFilesDir.filePath(arcFile), QDir(root).filePath(arcFile)));
                ++i;
            }
        }
    }
private:
    std::reference_wrapper<GameInstance> gameInstance;
    ModListType modList;
};

#endif // CSMMMODPACK_H
