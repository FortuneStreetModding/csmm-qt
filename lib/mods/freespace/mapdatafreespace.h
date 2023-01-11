#ifndef MAPDATAFREESPACE_H
#define MAPDATAFREESPACE_H

#include "lib/mods/csmmmod.h"

class MapDataFreeSpace : public virtual CSMMMod, public virtual GeneralInterface
{
public:
    static constexpr std::string_view MODID = "mapDataFreeSpace";
    QString modId() const override { return MODID.data(); }
    int priority() const override { return FREE_SPACE_PRIORITY; }
    QSet<QString> depends() const override { return {}; } // TODO add the various map data dolio mods as dependencies?
    void loadFiles(const QString &root, GameInstance *gameInstance, const ModListType &modList) override;
    void saveFiles(const QString &root, GameInstance *gameInstance, const ModListType &modList) override;
};

#endif // MAPDATAFREESPACE_H
