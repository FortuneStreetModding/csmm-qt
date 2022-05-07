#ifndef MUTATORROLLSHOPPRICEMULTIPLIER_H
#define MUTATORROLLSHOPPRICEMULTIPLIER_H

#include "doliomutator.h"

class MutatorRollShopPriceMultiplier : public virtual DolIOMutator
{
public:
    static constexpr std::string_view MODID = "mutatorRollShopPriceMultiplier";
    QString modId() const override { return MODID.data(); }
    void readAsm(QDataStream &stream, const AddressMapper &addressMapper, std::vector<MapDescriptor> &mapDescriptors) override;
protected:
    void writeAsm(QDataStream &stream, const AddressMapper &addressMapper, const std::vector<MapDescriptor> &mapDescriptors) override;
    QVector<quint32> writeRollDiceBeforePayingRoutine(const AddressMapper &addressMapper, quint32 routineStartAddress, const quint32 hasRolledDice, quint32 getMutatorDataSubroutine);
    QVector<quint32> writeRememberSquareType(const AddressMapper &addressMapper, quint32 routineStartAddress, const quint32 hasRolledDice);
    QVector<quint32> writeCalculateGainRoutine(const AddressMapper &addressMapper, quint32 routineStartAddress, const quint32 hasRolledDice, quint32 getMutatorDataSubroutine);
    QVector<quint32> writeClearDiceRolledFlag(const AddressMapper &addressMapper, quint32 routineStartAddress, const quint32 hasRolledDice);
    QVector<quint32> writeDontShowQuestionBoxAgain(const AddressMapper &addressMapper, quint32 routineStartAddress, const quint32 hasRolledDice);
    QVector<quint32> writeCloseDice(const AddressMapper &addressMapper, quint32 routineStartAddress, const quint32 hasRolledDice);
};

#endif // MUTATORROLLSHOPPRICEMULTIPLIER_H
