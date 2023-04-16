#ifndef VMOVESTOPFREESPACE_H
#define VMOVESTOPFREESPACE_H

#include "lib/mods/csmmmod.h"

class VMoveStopFreeSpace : public virtual CSMMMod, public virtual GeneralInterface
{
public:
    static constexpr std::string_view MODID = "vMoveStopFreeSpace";
    QString modId() const override { return MODID.data(); }
    int priority() const override { return FREE_SPACE_PRIORITY; }
    QSet<QString> depends() const override { return {"arbitraryNumSwitchStates"}; }
    void loadFiles(const QString &root, GameInstance *gameInstance, const ModListType &modList) override;
    void saveFiles(const QString &root, GameInstance *gameInstance, const ModListType &modList) override;
};

#endif // VMOVESTOPFREESPACE_H
