#ifndef MUTATORSHOPPRICE_H
#define MUTATORSHOPPRICE_H

#include "doliomutator.h"

class MutatorShopPrice : public virtual DolIOMutator
{
public:
    static constexpr std::string_view MODID = "mutatorShopPrice";
    QString modId() const override { return MODID.data(); }
    void readAsm(QDataStream &stream, const AddressMapper &addressMapper, std::vector<MapDescriptor> &mapDescriptors) override;
protected:
    void writeAsm(QDataStream &stream, const AddressMapper &addressMapper, const std::vector<MapDescriptor> &mapDescriptors) override;
    QVector<quint32> writeBaseShopPriceMultiplier(const AddressMapper &addressMapper, quint32 routineStartAddress, quint32 getMutatorDataSubroutine);
    QVector<quint32> write3StarHotelPriceMultiplier(const AddressMapper &addressMapper, quint32 routineStartAddress, quint32 getMutatorDataSubroutine);
    QVector<quint32> writeRankPriceMultiplier(const AddressMapper &addressMapper, quint32 routineStartAddress, quint32 getMutatorDataSubroutine);
};

#endif // MUTATORSHOPPRICE_H
