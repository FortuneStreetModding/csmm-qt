#ifndef EXPANDMAPSINZONE_H
#define EXPANDMAPSINZONE_H

#include "dolio.h"

class ExpandMapsInZone : public DolIO
{
public:
    virtual void readAsm(QDataStream &stream, const AddressMapper &addressMapper, QVector<MapDescriptor> &mapDescriptors) override;
protected:
    virtual void writeAsm(QDataStream &stream, const AddressMapper &addressMapper, const QVector<MapDescriptor> &mapDescriptors) override;
};

#endif // EXPANDMAPSINZONE_H
