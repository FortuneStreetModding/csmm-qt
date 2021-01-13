#ifndef PRACTICEBOARD_H
#define PRACTICEBOARD_H

#include "dolio.h"

class PracticeBoard : public DolIO
{
public:
    void readAsm(QDataStream &stream, const AddressMapper &addressMapper, QVector<MapDescriptor> &mapDescriptors) override;
protected:
    void writeAsm(QDataStream &stream, const AddressMapper &addressMapper, const QVector<MapDescriptor> &mapDescriptors) override;
};

#endif // PRACTICEBOARD_H
