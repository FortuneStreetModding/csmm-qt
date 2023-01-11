#ifndef WIFIFREESPACE_H
#define WIFIFREESPACE_H

#include "lib/mods/csmmmod.h"

class WifiFreeSpace : public virtual CSMMMod, public virtual GeneralInterface
{
public:
    static constexpr std::string_view MODID = "wifiFreeSpace";
    QString modId() const override { return MODID.data(); }
    int priority() const override { return FREE_SPACE_PRIORITY; }
    QSet<QString> depends() const override { return {"wifiFix"}; }
    void loadFiles(const QString &root, GameInstance *gameInstance, const ModListType &modList) override;
    void saveFiles(const QString &root, GameInstance *gameInstance, const ModListType &modList) override;
};

#endif // WIFIFREESPACE_H
