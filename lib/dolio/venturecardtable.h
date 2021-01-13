#ifndef VENTURECARDTABLE_H
#define VENTURECARDTABLE_H

#include "doliotable.h"

class VentureCardTable : public DolIOTable {
public:
    void readAsm(QDataStream &stream, QVector<MapDescriptor> &mapDescriptors, const AddressMapper &addressMapper, bool isVanilla) override;
protected:
    void writeAsm(QDataStream &stream, const AddressMapper &addressMapper, const QVector<MapDescriptor> &mapDescriptors) override;
    quint32 writeTable(const QVector<MapDescriptor> &descriptors);
    bool readIsVanilla(QDataStream &stream, const AddressMapper &addressMapper) override;
    qint16 readTableRowCount(QDataStream &stream, const AddressMapper &addressMapper, bool isVanilla) override;
    quint32 readTableAddr(QDataStream &stream, const AddressMapper &addressMapper, bool isVanilla) override;
private:
    QVector<quint32> writeSubroutine(quint32 ventureCardDecompressedTableAddr);
    void readVanillaVentureCardTable(QDataStream &stream, QVector<MapDescriptor> &mapDescriptors);
    void readCompressedVentureCardTable(QDataStream &stream, QVector<MapDescriptor> &mapDescriptors);
};

#endif // VENTURECARDTABLE_H
