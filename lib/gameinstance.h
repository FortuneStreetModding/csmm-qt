#ifndef GAMEINSTANCE_H
#define GAMEINSTANCE_H

#include "lib/addressmapping.h"
#include "lib/freespacemanager.h"
#include "lib/mapdescriptor.h"

#define MIN_CSMM_UI_MESSAGE_ID 25000

struct GameInstance {
    Q_DECLARE_TR_FUNCTIONS(GameInstance)

public:
    std::vector<MapDescriptor> &mapDescriptors();
    const std::vector<MapDescriptor> &mapDescriptors() const;
    const AddressMapper &addressMapper() const;
    FreeSpaceManager &freeSpaceManager();
    const FreeSpaceManager &freeSpaceManager() const;
    int nextUiMessageId();
    static GameInstance fromGameDirectory(const QString &dir, const QString &importDir, const std::vector<MapDescriptor> &descriptors = {});
    const QString &getImportDir() const;
private:
    GameInstance(
            const std::vector<MapDescriptor> &descriptors,
            const AddressMapper &addressMapper,
            const FreeSpaceManager &freeSpaceManager,
            const QString &importDir
            );
    std::vector<MapDescriptor> descriptors;
    AddressMapper mapper;
    FreeSpaceManager fsm;
    int curUiMessageId;
    QString importDir;
};

#endif // GAMEINSTANCE_H
