#ifndef DEFAULTMINIMAPICONS_H
#define DEFAULTMINIMAPICONS_H

#include "lib/mods/csmmmod.h"

class DefaultMinimapIcons : public virtual CSMMMod, public virtual ArcFileInterface {
public:
    QMap<QString, ModifyArcFunction> modifyArcFile() override;
    static constexpr std::string_view MODID = "defaultMinimapIcons";
    QString modId() const override { return MODID.data(); }
    QSet<QString> depends() const override { return {"mapIconTable"}; };
};

#endif // DEFAULTMINIMAPICONS_H
