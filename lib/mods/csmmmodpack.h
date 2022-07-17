#ifndef CSMMMODPACK_H
#define CSMMMODPACK_H

#include "csmmmod.h"
#include "lib/await.h"
#include "lib/asyncfuture.h"
#include "lib/exewrapper.h"
#include "lib/mods/csmmmod.h"
#include "lib/python/pythonbindings.h"
#include "lib/lz77.h"

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
            qInfo() << "loading mod" << mod->modId();

            auto generalFileInterface = mod.getCapability<GeneralInterface>();
            if (generalFileInterface) {
                qInfo() << "loading general interface for" << mod->modId();

                generalFileInterface->loadFiles(root, gameInstance, modList);
            }

            if (modToLoaders.contains(mod->modId())) {
                qInfo() << "loading UI messages for" << mod->modId();

                auto &loaders = modToLoaders[mod->modId()];
                for (auto it = loaders.begin(); it != loaders.end(); ++it) {
                    it.value()(root, gameInstance, modList, &messageFiles[it.key()]);
                }
            }
        }
    }

    void save(const QString &root, const std::function<void(double)> &progressCallback = [](double) {}) {
        QHash<QString, QMap<QString, UiMessageInterface::SaveMessagesFunction>> messageSavers;
        QHash<QString, QMap<QString, ArcFileInterface::ModifyArcFunction>> arcModifiers;
        QHash<QString, QMap<QString, CmpresInterface::ModifyCmpresFunction>> cmpresModifiers;
        QMap<QString, UiMessage> messageFiles;
        QTemporaryDir archiveFilesDir;
        QSet<QString> arcFiles;
        QSet<QString> cmpresFiles;

        if (!archiveFilesDir.isValid()) {
            throw ModException(QString("error creating temporary directory: %1").arg(archiveFilesDir.errorString()));
        }

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
            auto cmpresInterface = mod.getCapability<CmpresInterface>();
            if (cmpresInterface) {
                auto modCmpresModifiers = cmpresInterface->modifyCmpresFile();
                for (auto it = modCmpresModifiers.begin(); it != modCmpresModifiers.end(); ++it) {
                    cmpresFiles.insert(it.key());
                }
                cmpresModifiers[mod->modId()] = std::move(modCmpresModifiers);
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
            int arcCmpresSize = arcFiles.size() + cmpresFiles.size();
            for (auto &arcFile: arcFiles) {
                qInfo() << "extracting arc file" << arcFile;
                progressCallback((double)i / arcCmpresSize / 3);
                QDir(archiveFilesDir.path()).mkpath(arcFile);
                await(ExeWrapper::extractArcFile(QDir(root).filePath(arcFile), archiveFilesDir.filePath(arcFile)));
                ++i;
            }
            for (auto &cmpres: cmpresFiles) {
                qInfo() << "extracting cmpres file" << cmpres;
                progressCallback((double)i / arcCmpresSize / 3);
                QFile cmpresFileObj(QDir(root).filePath(cmpres));
                if (!cmpresFileObj.open(QFile::ReadOnly)) {
                    throw ModException("could not open " + cmpresFileObj.fileName());
                }
                QDataStream stream(&cmpresFileObj);
                auto extracted = LZ77::extract(stream).second;
                QDir(archiveFilesDir.path()).mkpath(cmpres);
                await(ExeWrapper::extractArcFileStdin(extracted, archiveFilesDir.filePath(cmpres)));
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
                uiMessageInterface->allocateUiMessages(root, gameInstance, modList);
            }
            if (messageSavers.contains(mod->modId())) {
                qInfo() << "saving ui messages for" << mod->modId();
                auto &savers = messageSavers[mod->modId()];
                for (auto it=savers.begin(); it!=savers.end(); ++it) {
                    it.value()(root, gameInstance, modList, &messageFiles[it.key()]);
                }
            }
            auto generalFileInterface = mod.getCapability<GeneralInterface>();
            if (generalFileInterface) {
                qInfo() << "processing general interface for" << mod->modId();
                generalFileInterface->saveFiles(root, gameInstance, modList);
            }
            if (arcModifiers.contains(mod->modId())) {
                qInfo() << "saving arc files for" << mod->modId();
                auto &modifiers = arcModifiers[mod->modId()];
                for (auto it=modifiers.begin(); it!=modifiers.end(); ++it) {
                    it.value()(root, gameInstance, modList, archiveFilesDir.filePath(it.key()));
                }
            }
            if (cmpresModifiers.contains(mod->modId())) {
                qInfo() << "saving cmpres files for" << mod->modId();
                auto &modifiers = cmpresModifiers[mod->modId()];
                for (auto it=modifiers.begin(); it!=modifiers.end(); ++it) {
                    it.value()(root, gameInstance, modList, archiveFilesDir.filePath(it.key()));
                }
            }
        }

        for (auto it=messageFiles.begin(); it!=messageFiles.end(); ++it) {
            QFile file(QDir(root).filePath(it.key()));
            if (!file.open(QFile::WriteOnly)) {
                throw ModException(QString("could not open file %1").arg(it.key()));
            }
            messageToFile(&file, messageFiles[it.key()]);
        }

        {
            int i = 0;
            int arcCmpresSize = arcFiles.size() + cmpresFiles.size();
            for (auto &arcFile: arcFiles) {
                qInfo() << "saving arc file" << arcFile;
                progressCallback((2 + (double)i / arcCmpresSize) / 3);
                await(ExeWrapper::packDfolderToArc(archiveFilesDir.filePath(arcFile), QDir(root).filePath(arcFile)));
                ++i;
            }
            for (auto &cmpres: cmpresFiles) {
                qInfo() << "saving cmpres file" << cmpres;
                progressCallback((2 + (double)i / arcCmpresSize) / 3);
                QByteArray packed = await(ExeWrapper::packDfolderToArcStdout(archiveFilesDir.filePath(cmpres)));
                QSaveFile cmpresFileObj(QDir(root).filePath(cmpres));
                if (!cmpresFileObj.open(QFile::WriteOnly)) {
                    throw ModException("could not open " + cmpresFileObj.fileName() + " for writing");
                }
                QDataStream stream(&cmpresFileObj);
                LZ77::compress(packed, stream, true);
                if (!cmpresFileObj.commit()) {
                    throw ModException("could not save " + cmpresFileObj.fileName());
                }
                ++i;
            }
        }
    }
private:
    std::reference_wrapper<GameInstance> gameInstance;
    ModListType modList;
};

#endif // CSMMMODPACK_H
