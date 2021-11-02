#ifndef ROLLSHOPPRICEMULTIPLIERPATCH_H
#define ROLLSHOPPRICEMULTIPLIERPATCH_H

#include "doliotable.h"

class RollShopPriceMultiplierPatch : public DolIO
{
public:
    void readAsm(QDataStream &stream, const AddressMapper &addressMapper, QVector<MapDescriptor> &mapDescriptors) override;
protected:
    void writeAsm(QDataStream &stream, const AddressMapper &addressMapper, const QVector<MapDescriptor> &mapDescriptors) override;
    QVector<quint32> writeRollDiceBeforePayingRoutine(const AddressMapper &addressMapper, quint32 routineStartAddress);
    QVector<quint32> writeCalculateGainRoutine(const AddressMapper &addressMapper, quint32 routineStartAddress);
};

#endif // ROLLSHOPPRICEMULTIPLIERPATCH_H
