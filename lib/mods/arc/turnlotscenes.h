#ifndef TURNLOTSCENES_H
#define TURNLOTSCENES_H

#include "lib/mods/csmmmod.h"

class TurnlotScenes : public virtual CSMMMod, public virtual GeneralInterface
{
public:
    void loadFiles(const QString &root, GameInstance *gameInstance, const ModListType &modList) override;
    void saveFiles(const QString &root, GameInstance *gameInstance, const ModListType &modList) override;
    static constexpr std::string_view MODID = "turnlotScenes";
    QString modId() const override { return MODID.data(); }
};

#endif // TURNLOTSCENES_H
