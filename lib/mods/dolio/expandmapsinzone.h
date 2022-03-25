#ifndef EXPANDMAPSINZONE_H
#define EXPANDMAPSINZONE_H

#include "dolio.h"

class ExpandMapsInZone : public virtual DolIO
{
public:
    static constexpr std::string_view MODID = "expandMapsInZone";
    QString modId() const override { return MODID.data(); }
    QSet<QString> after() const override { return { "mapSetZoneOrder" }; }
    virtual void readAsm(QDataStream &stream, const AddressMapper &addressMapper, QVector<MapDescriptor> &mapDescriptors) override;
protected:
    virtual void writeAsm(QDataStream &stream, const AddressMapper &addressMapper, const QVector<MapDescriptor> &mapDescriptors) override;
};

#endif // EXPANDMAPSINZONE_H
