#ifndef DISTRICTNAMEFREESPACE_H
#define DISTRICTNAMEFREESPACE_H

#include "lib/mods/csmmmod.h"

class DistrictNameFreeSpace : public virtual CSMMMod, public virtual GeneralInterface
{
public:
    static constexpr std::string_view MODID = "districtNameFreeSpace";
    QString modId() const override { return MODID.data(); }
    int priority() const override { return FREE_SPACE_PRIORITY; }
    QSet<QString> depends() const override { return {"namedDistricts"}; }
    void loadFiles(const QString &root, GameInstance *gameInstance, const ModListType &modList) override;
    void saveFiles(const QString &root, GameInstance *gameInstance, const ModListType &modList) override;
};

#endif // DISTRICTNAMEFREESPACE_H
