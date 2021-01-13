#ifndef FORCESIMULATEDBUTTONPRESS_H
#define FORCESIMULATEDBUTTONPRESS_H

#include "dolio.h"

class ForceSimulatedButtonPress : public DolIO
{
public:
    void readAsm(QDataStream &stream, const AddressMapper &addressMapper, QVector<MapDescriptor> &mapDescriptors) override;
protected:
    void writeAsm(QDataStream &stream, const AddressMapper &addressMapper, const QVector<MapDescriptor> &mapDescriptors) override;
private:
    QVector<quint32> writeUploadSimulatedButtonPress(const AddressMapper &addressMapper, quint32 routineStartAddress, quint32 returnAddr);
};

#endif // FORCESIMULATEDBUTTONPRESS_H
