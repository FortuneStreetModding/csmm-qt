#ifndef MUTATORTABLE_H
#define MUTATORTABLE_H

#include "doliotable.h"

// TODO instead of writing subroutine to fixed addr, write the address of the subroutine in separate file
// so that csmm is more modularized
class MutatorTable : public virtual DolIOTable
{
public:
    static constexpr std::string_view MODID = "mutatorTable";
    QString modId() const override { return MODID.data(); }
    void readAsm(QDataStream &stream, QVector<MapDescriptor> &mapDescriptors, const AddressMapper &addressMapper, bool isVanilla) override;
protected:
    void writeAsm(QDataStream &stream, const AddressMapper &addressMapper, const QVector<MapDescriptor> &mapDescriptors) override;
    quint32 writeTable(const QVector<MapDescriptor> &descriptors);
    quint32 writeMutatorData(const MapDescriptor &descriptor);
    bool readIsVanilla(QDataStream &stream, const AddressMapper &addressMapper) override;
    qint16 readTableRowCount(QDataStream &stream, const AddressMapper &addressMapper, bool isVanilla) override;
    quint32 readTableAddr(QDataStream &stream, const AddressMapper &addressMapper, bool isVanilla) override;
    QVector<quint32> writeGetMutatorDataSubroutine(const AddressMapper &addressMapper, quint32 tableAddr);
};

#endif // MUTATORTABLE_H
