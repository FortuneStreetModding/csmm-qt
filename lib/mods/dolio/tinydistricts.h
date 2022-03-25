#ifndef TINYDISTRICTS_H
#define TINYDISTRICTS_H

#include "dolio.h"

class TinyDistricts : public virtual DolIO
{
public:
    static constexpr std::string_view MODID = "tinyDistricts";
    QString modId() const override { return MODID.data(); }
    void readAsm(QDataStream &stream, const AddressMapper &addressMapper, QVector<MapDescriptor> &mapDescriptors) override;
protected:
    void writeAsm(QDataStream &stream, const AddressMapper &addressMapper, const QVector<MapDescriptor> &mapDescriptors) override;
    QVector<quint32> writeSingleShopDistrictCheckRoutine(const AddressMapper &addressMapper, quint32 routineStartAddress);
};

#endif // TINYDISTRICTS_H
