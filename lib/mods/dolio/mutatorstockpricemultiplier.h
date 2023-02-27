#ifndef MUTATORSTOCKPRICEMULTIPLIER_H
#define MUTATORSTOCKPRICEMULTIPLIER_H

#include "doliomutator.h"

class MutatorStockPriceMultiplier : public virtual DolIOMutator
{
public:
    static constexpr std::string_view MODID = "mutatorStockPriceMultiplier";
    QString modId() const override { return MODID.data(); }
    void readAsm(QDataStream &stream, const AddressMapper &addressMapper, std::vector<MapDescriptor> &mapDescriptors) override;
protected:
    void writeAsm(QDataStream &stream, const AddressMapper &addressMapper, const std::vector<MapDescriptor> &mapDescriptors) override;
    QVector<quint32> writeStockPriceMultiplier(const AddressMapper &addressMapper, quint32 routineStartAddress, quint32 getMutatorDataSubroutine);
};

#endif // MUTATORSTOCKPRICEMULTIPLIER_H
