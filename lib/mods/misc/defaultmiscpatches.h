#ifndef DEFAULTMISCPATCHES_H
#define DEFAULTMISCPATCHES_H

#include "lib/mods/csmmmod.h"

class DefaultMiscPatches : public virtual CSMMMod, public virtual GeneralInterface
{
public:
    static constexpr std::string_view MODID = "defaultMiscPatches";
    QString modId() const override { return MODID.data(); }
    void loadFiles(const QString &root, GameInstance &gameInstance, const ModListType &modList) override;
    void saveFiles(const QString &root, GameInstance &gameInstance, const ModListType &modList) override;
};

#endif // DEFAULTMISCPATCHES_H
