#ifndef MutatorShopPriceMultiplier_H
#define MutatorShopPriceMultiplier_H

#include "doliotable.h"

class MutatorShopPriceMultiplier : public DolIO
{
public:
    void readAsm(QDataStream &stream, const AddressMapper &addressMapper, QVector<MapDescriptor> &mapDescriptors) override;
protected:
    void writeAsm(QDataStream &stream, const AddressMapper &addressMapper, const QVector<MapDescriptor> &mapDescriptors) override;
    QVector<quint32> writeShopPriceMultiplier(const AddressMapper &addressMapper, quint32 routineStartAddress);
    QVector<quint32> writeUnownedShopPriceMultiplier(const AddressMapper &addressMapper, quint32 routineStartAddress);
};

#endif // MutatorShopPriceMultiplier_H
