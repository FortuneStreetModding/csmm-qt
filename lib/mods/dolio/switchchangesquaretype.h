#ifndef SWITCHCHANGESQUARETYPE_H
#define SWITCHCHANGESQUARETYPE_H

#include "lib/mods/dolio/dolio.h"


class SwitchChangeSquareType : public virtual DolIO
{
public:
    static constexpr std::string_view MODID = "switchChangeSquareType";
    QString modId() const override { return MODID.data(); }
    virtual void readAsm(QDataStream &stream, const AddressMapper &addressMapper, std::vector<MapDescriptor> &mapDescriptors) override;
protected:
    virtual void writeAsm(QDataStream &stream, const AddressMapper &addressMapper, const std::vector<MapDescriptor> &mapDescriptors) override;
};

#endif // SWITCHCHANGESQUARETYPE_H
