#ifndef TINYDISTRICTSFREESPACE_H
#define TINYDISTRICTSFREESPACE_H

#include "lib/mods/csmmmod.h"

class TinyDistrictsFreeSpace : public virtual CSMMMod, public virtual GeneralInterface {
public:
    static constexpr std::string_view MODID = "tinyDistrictsFreeSpace";
    QString modId() const override { return MODID.data(); }
    int priority() const override { return FREE_SPACE_PRIORITY; }
    QSet<QString> depends() const override { return {"tinyDistricts"}; }
    void loadFiles(const QString &root, GameInstance *gameInstance, const ModListType &modList) override;
    void saveFiles(const QString &root, GameInstance *gameInstance, const ModListType &modList) override;
};

#endif // TINYDISTRICTSFREESPACE_H
