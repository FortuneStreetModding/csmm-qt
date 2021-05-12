#ifndef WIFIFIX_H
#define WIFIFIX_H

#include "dolio.h"

class WifiFix : public DolIO
{
public:
    void readAsm(QDataStream &stream, const AddressMapper &addressMapper, QVector<MapDescriptor> &mapDescriptors) override;
protected:
    void writeAsm(QDataStream &stream, const AddressMapper &addressMapper, const QVector<MapDescriptor> &mapDescriptors) override;
private:
    QVector<quint32> writeSubroutineGetDefaultMapIdIntoR3(const AddressMapper &addressMapper, short defaultEasyMap, short defaultStandardMap, quint32 entryAddr, quint32 returnAddr);
    QVector<quint32> writeSubroutineGetDefaultMapIdIntoR4(short defaultEasyMap, short defaultStandardMap, quint32 entryAddr, quint32 returnAddr);
};

#endif // WIFIFIX_H
