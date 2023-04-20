#ifndef MUTATORGENERICHOOK_H
#define MUTATORGENERICHOOK_H

#include "doliomutator.h"

class MutatorGenericHook : public virtual DolIOMutator
{
public:
    static constexpr std::string_view MODID = "mutatorGenericHook";
    QString modId() const override { return MODID.data(); }
    QSet<QString> depends() const override { return { "eventSquare" }; }
    QSet<QString> after() const override { return { "eventSquare" }; }
    void readAsm(QDataStream &stream, const AddressMapper &addressMapper, std::vector<MapDescriptor> &mapDescriptors) override;
protected:
    void writeAsm(QDataStream &stream, const AddressMapper &addressMapper, const std::vector<MapDescriptor> &mapDescriptors) override;
    quint32 getForceVentureCardVariable(const ModListType &modList);
    QVector<quint32> writeInitMutatorAtBeginTurn(const AddressMapper &addressMapper, quint32 routineStartAddress, const quint32 hasRolledDice, quint32 getMutatorDataSubroutine);
    QVector<quint32> writeRememberDiceRoll(const AddressMapper &addressMapper, quint32 routineStartAddress, const quint32 hasRolledDice);
    QVector<quint32> writeCalculateGainRoutine(const AddressMapper &addressMapper, quint32 routineStartAddress, const quint32 lastShopPrice, const quint32 hasRolledDice, quint32 getMutatorDataSubroutine);
    QVector<quint32> writeRollAgainRoutine(const AddressMapper &addressMapper, quint32 routineStartAddress, const quint32 hasRolledDice, const quint32 forceVentureCardVariable, quint32 getMutatorDataSubroutine);
    QVector<quint32> writeDontShowQuestionBoxAgain(const AddressMapper &addressMapper, quint32 routineStartAddress, const quint32 hasRolledDice);
    QVector<quint32> writeCloseDice(const AddressMapper &addressMapper, quint32 routineStartAddress, const quint32 hasRolledDice);
};

#endif // MUTATORGENERICHOOK_H
