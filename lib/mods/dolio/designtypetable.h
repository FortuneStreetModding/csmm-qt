#ifndef DESIGNTYPETABLE_H
#define DESIGNTYPETABLE_H

#include "doliotable.h"

class DesignTypeTable : public virtual DolIOTable
{
public:
    static constexpr std::string_view MODID = "designTypeTable";
    QString modId() const override { return MODID.data(); }
    void readAsm(QDataStream &stream, QVector<MapDescriptor> &mapDescriptors, const AddressMapper &addressMapper, bool isVanilla) override;
protected:
    void writeAsm(QDataStream &stream, const AddressMapper &addressMapper, const QVector<MapDescriptor> &mapDescriptors) override;
    quint32 writeTable(const QVector<MapDescriptor> &descriptors);
    bool readIsVanilla(QDataStream &stream, const AddressMapper &addressMapper) override;
    qint16 readTableRowCount(QDataStream &stream, const AddressMapper &addressMapper, bool isVanilla) override;
    quint32 readTableAddr(QDataStream &stream, const AddressMapper &addressMapper, bool isVanilla) override;
};

#endif // DESIGNTYPETABLE_H