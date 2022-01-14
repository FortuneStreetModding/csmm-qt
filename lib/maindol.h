#ifndef MAINDOL_H
#define MAINDOL_H

#include "dolio/dolio.h"
#include "lib/optionalpatch.h"

class MainDol {
public:
    AddressMapper addressMapper;
    FreeSpaceManager freeSpaceManager;
    QVector<QSharedPointer<DolIO>> patches;
    MainDol(QDataStream &stream, const QVector<AddressSection> &mappingSections, const QSet<OptionalPatch> &optionalPatches);
    QVector<MapDescriptor> readMainDol(QDataStream &stream);
    QVector<MapDescriptor> writeMainDol(QDataStream &stream, const QVector<MapDescriptor> &mapDescriptors);
private:
    AddressMapper setupAddressMapper(QDataStream &stream, const QVector<AddressSection> &fileMappingSections);
    FreeSpaceManager setupFreeSpaceManager(AddressMapper addressMapper);
    QVector<QSharedPointer<DolIO>> setupPatches(const QSet<OptionalPatch> &optionalPatches);
};

#endif // MAINDOL_H
