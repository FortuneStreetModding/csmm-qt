#ifndef GAMEINSTANCE_H
#define GAMEINSTANCE_H

#include "lib/addressmapping.h"
#include "lib/freespacemanager.h"
#include "lib/mapdescriptor.h"

#define MIN_CSMM_UI_MESSAGE_ID 25000

struct GameInstance {
public:
    GameInstance(const QVector<MapDescriptor> &descriptors, const AddressMapper &addressMapper, const FreeSpaceManager &freeSpaceManager);
    QVector<MapDescriptor> &mapDescriptors();
    const QVector<MapDescriptor> &mapDescriptors() const;
    const AddressMapper &addressMapper() const;
    FreeSpaceManager &freeSpaceManager();
    const FreeSpaceManager &freeSpaceManager() const;
    int nextUiMessageId();
    static GameInstance fromGameDirectory(const QString &dir, const QVector<MapDescriptor> &descriptors = {});
private:
    QVector<MapDescriptor> descriptors;
    AddressMapper mapper;
    FreeSpaceManager fsm;
    int curUiMessageId;
};

#endif // GAMEINSTANCE_H
