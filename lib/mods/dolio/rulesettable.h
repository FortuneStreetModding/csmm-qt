#ifndef RULESETTABLE_H
#define RULESETTABLE_H

#include "doliotable.h"

class RuleSetTable : public virtual DolIOTable
{
public:
    static constexpr std::string_view MODID = "ruleSetTable";
    QString modId() const override { return MODID.data(); }
    void readAsm(QDataStream &stream, QVector<MapDescriptor> &mapDescriptors, const AddressMapper &addressMapper, bool isVanilla) override;
protected:
    void writeAsm(QDataStream &stream, const AddressMapper &addressMapper, const QVector<MapDescriptor> &mapDescriptors) override;
    quint32 writeTable(const QVector<MapDescriptor> &descriptors);
    bool readIsVanilla(QDataStream &stream, const AddressMapper &addressMapper) override;
    qint16 readTableRowCount(QDataStream &stream, const AddressMapper &addressMapper, bool isVanilla) override;
    quint32 readTableAddr(QDataStream &stream, const AddressMapper &addressMapper, bool isVanilla) override;
private:
    QVector<quint32> writeRuleSetFromMapRoutine(const AddressMapper &addressMapper, quint32 routineStartAddress);
    QVector<quint32> writeGetRuleSetRoutine(const AddressMapper &addressMapper, quint32 routineStartAddress, quint32 routineReturnAddress);
};

#endif // RULESETTABLE_H
