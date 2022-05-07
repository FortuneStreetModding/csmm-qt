#ifndef MutatorShopPriceMultiplier_H
#define MutatorShopPriceMultiplier_H

#include "doliomutator.h"

class MutatorShopPriceMultiplier : public virtual DolIOMutator
{
public:
    static constexpr std::string_view MODID = "mutatorShopPriceMultiplier";
    QString modId() const override { return MODID.data(); }
    void readAsm(QDataStream &stream, const AddressMapper &addressMapper, std::vector<MapDescriptor> &mapDescriptors) override;
protected:
    void writeAsm(QDataStream &stream, const AddressMapper &addressMapper, const std::vector<MapDescriptor> &mapDescriptors) override;
    QVector<quint32> writeBaseShopPriceMultiplier(const AddressMapper &addressMapper, quint32 routineStartAddress, quint32 getMutatorDataSubroutine);
    QVector<quint32> write3StarHotelPriceMultiplier(const AddressMapper &addressMapper, quint32 routineStartAddress, quint32 getMutatorDataSubroutine);
    QVector<quint32> writeRankPriceMultiplier(const AddressMapper &addressMapper, quint32 routineStartAddress, quint32 getMutatorDataSubroutine);
};

#endif // MutatorShopPriceMultiplier_H
