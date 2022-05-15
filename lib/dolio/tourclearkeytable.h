#ifndef TOURCLEARKEYTABLE_H
#define TOURCLEARKEYTABLE_H

#include "doliotable.h"

class TourClearKeyTable : public DolIOTable {
public:
    void readAsm(QDataStream &stream, QVector<MapDescriptor> &mapDescriptors, const AddressMapper &addressMapper, bool isVanilla) override;
protected:
    void writeAsm(QDataStream &stream, const AddressMapper &addressMapper, const QVector<MapDescriptor> &mapDescriptors) override;
    quint32 writeTable(const QVector<MapDescriptor> &descriptors);
    bool readIsVanilla(QDataStream &stream, const AddressMapper &addressMapper) override;
    qint16 readTableRowCount(QDataStream &stream, const AddressMapper &addressMapper, bool isVanilla) override;
    quint32 readTableAddr(QDataStream &stream, const AddressMapper &addressMapper, bool isVanilla) override;
private:
    QVector<quint32> writeRoutine_InitializeSaveGameData(const AddressMapper &addressMapper, quint32 routineStartAddress);
    QVector<quint32> writeRoutine_GetGainedMapClearKeyAddress(const AddressMapper &addressMapper, quint32 routineStartAddress, quint32 tableAddr);
    QVector<quint32> writeRoutine_GetAmountOfSlots(const AddressMapper &addressMapper, quint32 routineStartAddress);
};

#endif // TOURCLEARKEYTABLE_H
