#ifndef MAPICONTABLE_H
#define MAPICONTABLE_H

#include "doliotable.h"

class MapIconTable : public DolIOTable
{
public:
    void readAsm(QDataStream &stream, QVector<MapDescriptor> &mapDescriptors, const AddressMapper &addressMapper, bool isVanilla) override;
protected:
    void writeAsm(QDataStream &stream, const AddressMapper &addressMapper, const QVector<MapDescriptor> &mapDescriptors) override;
    bool readIsVanilla(QDataStream &stream, const AddressMapper &addressMapper) override;
    qint16 readTableRowCount(QDataStream &stream, const AddressMapper &addressMapper, bool isVanilla) override;
    quint32 readTableAddr(QDataStream &stream, const AddressMapper &addressMapper, bool isVanilla) override;

    QMap<QString, quint32> writeIconStrings(const QVector<MapDescriptor> &mapDescriptors);
    quint32 writeIconTable(const QMap<QString, quint32> &mapIcons, QMap<QString, quint32> &iconTableMap);
    quint32 writeMapIconPointerTable(const QVector<MapDescriptor> &mapDescriptors, const QMap<QString, quint32> &iconTableMap);

    QString resolveAddressAddressToString(quint32 virtualAddressAddress, QDataStream &stream, const AddressMapper &addressMapper);
private:
    QVector<quint32> writeSubroutineInitMapIdsForMapIcons(const AddressMapper &addressMapper, quint32 entryAddr);
    QVector<quint32> writeSubroutineMakeNoneMapIconsInvisible(const AddressMapper &addressMapper, quint32 entryAddr, quint32 returnContinueAddr, quint32 returnMakeInvisibleAddr);
};

#endif // MAPICONTABLE_H
