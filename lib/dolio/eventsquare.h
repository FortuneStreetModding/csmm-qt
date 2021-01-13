#ifndef EVENTSQUARE_H
#define EVENTSQUARE_H

#include "dolio.h"

class EventSquare : public DolIO
{
public:
    void readAsm(QDataStream &stream, const AddressMapper &addressMapper, QVector<MapDescriptor> &mapDescriptors) override;
protected:
    void writeAsm(QDataStream &stream, const AddressMapper &addressMapper, const QVector<MapDescriptor> &mapDescriptors) override;
private:
    QVector<quint32> writeGetDescriptionForCustomSquareRoutine(const AddressMapper &addressMapper, quint32 routineStartAddress);
    QVector<quint32> writeGetTextureForCustomSquareRoutine(quint8 register_textureType, quint8 register_squareType);
    QVector<quint32> writeProcStopEventSquareRoutine(const AddressMapper &addressMapper, quint32 forceVentureCardVariable, quint32 routineStartAddress);
    QVector<quint32> writeSubroutineForceFetchFakeVentureCard(quint32 fakeVentureCard);
};

#endif // EVENTSQUARE_H
