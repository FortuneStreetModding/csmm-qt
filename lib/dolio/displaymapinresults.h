#ifndef DISPLAYMAPINRESULTS_H
#define DISPLAYMAPINRESULTS_H

#include "dolio.h"

class DisplayMapInResults : public DolIO {
public:
    void readAsm(QDataStream &stream, const AddressMapper &addressMapper, QVector<MapDescriptor> &mapDescriptors) override;
protected:
    void writeAsm(QDataStream &stream, const AddressMapper &addressMapper, const QVector<MapDescriptor> &mapDescriptors) override;
};

#endif // DISPLAYMAPINRESULTS_H
