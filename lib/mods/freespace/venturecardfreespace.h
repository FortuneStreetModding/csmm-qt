#ifndef VENTURECARDFREESPACE_H
#define VENTURECARDFREESPACE_H

#include "lib/mods/csmmmod.h"

class VentureCardFreeSpace : public virtual CSMMMod, public virtual GeneralInterface
{
public:
    static constexpr std::string_view MODID = "ventureCardFreeSpace";
    QString modId() const override { return MODID.data(); }
    int priority() const override { return FREE_SPACE_PRIORITY; }
    QSet<QString> depends() const override { return {"ventureCardTable"}; }
    void loadFiles(const QString &root, GameInstance *gameInstance, const ModListType &modList) override;
    void saveFiles(const QString &root, GameInstance *gameInstance, const ModListType &modList) override;
};

#endif // VENTURECARDFREESPACE_H
