#ifndef ALLOCATEDESCRIPTORCOUNT_H
#define ALLOCATEDESCRIPTORCOUNT_H

#include "dolio.h"

class AllocateDescriptorCount : public DolIO
{
public:
    static constexpr std::string_view MODID = "allocateDescriptorCount";
    QString modId() const override { return MODID.data(); }
    int priority() const override { return MAP_DESCRIPTOR_COUNT_PRIORITY; }
    void readAsm(QDataStream &stream, const AddressMapper &addressMapper, QVector<MapDescriptor> &mapDescriptors) override;
protected:
    void writeAsm(QDataStream &stream, const AddressMapper &addressMapper, const QVector<MapDescriptor> &mapDescriptors) override;
};

#endif // ALLOCATEDESCRIPTORCOUNT_H
