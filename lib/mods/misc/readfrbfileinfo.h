#ifndef READFRBFILEINFO_H
#define READFRBFILEINFO_H

#include "lib/mods/csmmmod.h"

class ReadFrbFileInfo : public CSMMMod, public GeneralInterface
{
    static constexpr std::string_view MODID = "readFrbFileInfo";
    void loadFiles(const QString &root, GameInstance *gameInstance, const ModListType &modList) override;
    void saveFiles(const QString &root, GameInstance *gameInstance, const ModListType &modList) override;
    QString modId() const override { return MODID.data(); };
    QSet<QString> after() const override { return {"frbMapTable"}; }
};

#endif // READFRBFILEINFO_H
