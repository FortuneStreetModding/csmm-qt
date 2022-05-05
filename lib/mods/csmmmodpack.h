#ifndef CSMMMODPACK_H
#define CSMMMODPACK_H

#include "csmmmod.h"
#include "lib/await.h"
#include "lib/asyncfuture.h"
#include "lib/exewrapper.h"

class CSMMModpack {
public:
    template<class InputIterator>
    CSMMModpack(GameInstance &gameInstance, InputIterator modsStart, InputIterator modsEnd) : gameInstance(gameInstance) {
        QMap<int, QHash<QString, CSMMModHolder>> modTable;
        QHash<QString, CSMMModHolder> priorityIndependentModTable;
        for (auto it = modsStart; it != modsEnd; ++it) {
            modTable[-(*it)->priority()][(*it)->modId()] = *it; // store negative priority so that higher priority is visited first
            if (priorityIndependentModTable.contains((*it)->modId())) {
                throw ModException(QString("duplicate mod '%1' detected").arg((*it)->modId()));
            }
            priorityIndependentModTable[(*it)->modId()] = *it;
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

                auto after = mod->after();

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
            qDebug() << QString("loading mod '%1'").arg(mod->modId());

            auto generalFileInterface = mod.getCapability<GeneralInterface>();
            if (generalFileInterface) {
                qDebug() << QString("mod '%1' has general file interface, loading that").arg(mod->modId());

                generalFileInterface->loadFiles(root, gameInstance, modList);
            }

            if (modToLoaders.contains(mod->modId())) {
                auto &loaders = modToLoaders[mod->modId()];
                for (auto it = loaders.begin(); it != loaders.end(); ++it) {
                    it.value()(root, gameInstance, modList, messageFiles[it.key()]);
                }
            }
        }
    }

    void save(const QString &root) {
        QHash<QString, QMap<QString, UiMessageInterface::SaveMessagesFunction>> messageFreers, messageSavers;
        QHash<QString, QMap<QString, ArcFileInterface::ModifyArcFunction>> arcModifiers;
        QMap<QString, UiMessage> messageFiles;
        QTemporaryDir arcFilesDir;
        QSet<QString> arcFiles;

        if (!arcFilesDir.isValid()) {
            throw ModException(QString("error creating temporary directory: %1").arg(arcFilesDir.errorString()));
        }

        for (auto &mod: modList) {
            auto uiMessageInterface = mod.getCapability<UiMessageInterface>();
            if (uiMessageInterface) {
                auto freers = uiMessageInterface->freeUiMessages();
                auto savers = uiMessageInterface->saveUiMessages();
                for (auto it = freers.begin(); it != freers.end(); ++it) {
                    messageFiles[it.key()]; // create blank uimessage
                }
                for (auto it = savers.begin(); it != savers.end(); ++it) {
                    messageFiles[it.key()]; // create blank uimessage
                }
                messageFreers[mod->modId()] = std::move(freers);
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

        for (auto &arcFile: arcFiles) {
            qDebug() << QString("extracting arc file %1").arg(arcFile);
            QDir(arcFilesDir.path()).mkpath(arcFile);
            await(ExeWrapper::extractArcFile(QDir(root).filePath(arcFile), arcFilesDir.filePath(arcFile)));
        }

        for (auto &mod: modList) {
            qDebug() << QString("saving mod '%1'").arg(mod->modId());

            if (messageFreers.contains(mod->modId())) {
                auto &freers = messageFreers[mod->modId()];
                for (auto it=freers.begin(); it!=freers.end(); ++it) {
                    it.value()(root, gameInstance, modList, messageFiles[it.key()]);
                }
            }
            auto uiMessageInterface = mod.getCapability<UiMessageInterface>();
            if (uiMessageInterface) {
                uiMessageInterface->allocateUiMessages(root, gameInstance, modList);
            }
            if (messageSavers.contains(mod->modId())) {
                auto &savers = messageSavers[mod->modId()];
                for (auto it=savers.begin(); it!=savers.end(); ++it) {
                    it.value()(root, gameInstance, modList, messageFiles[it.key()]);
                }
            }
            auto generalFileInterface = mod.getCapability<GeneralInterface>();
            if (generalFileInterface) {
                generalFileInterface->saveFiles(root, gameInstance, modList);
            }
            if (arcModifiers.contains(mod->modId())) {
                auto &modifiers = arcModifiers[mod->modId()];
                for (auto it=modifiers.begin(); it!=modifiers.end(); ++it) {
                    it.value()(root, gameInstance, modList, arcFilesDir.filePath(it.key()));
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

        for (auto &arcFile: arcFiles) {
            qDebug() << QString("saving arc file %1").arg(arcFile);
            await(ExeWrapper::packDfolderToArc(arcFilesDir.filePath(arcFile), QDir(root).filePath(arcFile)));
        }
    }
private:
    GameInstance &gameInstance;
    ModListType modList;
};

#endif // CSMMMODPACK_H
