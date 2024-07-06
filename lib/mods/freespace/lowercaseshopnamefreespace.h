#ifndef LOWERCASESHOPNAMEFREESPACE_H
#define LOWERCASESHOPNAMEFREESPACE_H

#include "lib/mods/csmmmod.h"

class LowerCaseShopNameFreeSpace : public virtual CSMMMod, public virtual GeneralInterface
{
public:
    static constexpr std::string_view MODID = "lowerCaseShopNameFreeSpace";
    QString modId() const override { return MODID.data(); }
    int priority() const override { return FREE_SPACE_PRIORITY; }
    QSet<QString> depends() const override { return {"customShopNames"}; }
    void loadFiles(const QString &root, GameInstance *gameInstance, const ModListType &modList) override;
    void saveFiles(const QString &root, GameInstance *gameInstance, const ModListType &modList) override;
};

#endif // LOWERCASESHOPNAMEFREESPACE_H
