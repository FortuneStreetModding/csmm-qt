#ifndef INITIALFREESPACE_H
#define INITIALFREESPACE_H

#include "lib/mods/csmmmod.h"

class InitialFreeSpace : public virtual GeneralInterface, public virtual CSMMMod
{
public:
    static constexpr std::string_view MODID = "initialFreeSpace";
    QString modId() const override { return MODID.data(); }
    int priority() const override { return FREE_SPACE_PRIORITY; }
    void loadFiles(const QString &root, GameInstance *gameInstance, const ModListType &modList) override;
    void saveFiles(const QString &root, GameInstance *gameInstance, const ModListType &modList) override;
};

#endif // INITIALFREESPACE_H
