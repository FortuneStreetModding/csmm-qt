#ifndef MUTATORROLLSHOPPRICEMULTIPLIER_H
#define MUTATORROLLSHOPPRICEMULTIPLIER_H

#include "doliotable.h"

class MutatorRollShopPriceMultiplier : public DolIO
{
public:
    void readAsm(QDataStream &stream, const AddressMapper &addressMapper, QVector<MapDescriptor> &mapDescriptors) override;
protected:
    void writeAsm(QDataStream &stream, const AddressMapper &addressMapper, const QVector<MapDescriptor> &mapDescriptors) override;
    QVector<quint32> writeRollDiceBeforePayingRoutine(const AddressMapper &addressMapper, quint32 routineStartAddress, const quint32 hasRolledDice);
    QVector<quint32> writeCalculateGainRoutine(const AddressMapper &addressMapper, quint32 routineStartAddress, const quint32 hasRolledDice);
    QVector<quint32> writeClearDiceRolledFlag(const AddressMapper &addressMapper, quint32 routineStartAddress, const quint32 hasRolledDice);
    QVector<quint32> writeDontShowQuestionBoxAgain(const AddressMapper &addressMapper, quint32 routineStartAddress, const quint32 hasRolledDice);
    QVector<quint32> writeCloseDice(const AddressMapper &addressMapper, quint32 routineStartAddress, const quint32 hasRolledDice);
};

#endif // MUTATORROLLSHOPPRICEMULTIPLIER_H
