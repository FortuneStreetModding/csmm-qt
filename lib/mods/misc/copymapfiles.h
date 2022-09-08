#ifndef COPYMAPFILES_H
#define COPYMAPFILES_H

#include "lib/mods/csmmmod.h"

class CopyMapFiles : public virtual CSMMMod, public virtual GeneralInterface
{
public:
    static constexpr std::string_view MODID = "copyMapFiles";
    QString modId() const override { return MODID.data(); }
    void saveFiles(const QString &root, GameInstance &gameInstance, const ModListType &modList) override;
};

#endif // COPYMAPFILES_H
