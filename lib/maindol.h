#ifndef MAINDOL_H
#define MAINDOL_H

#include "dolio/dolio.h"

class MainDol {
public:
    AddressMapper addressMapper;
    FreeSpaceManager freeSpaceManager;
    QVector<QSharedPointer<DolIO>> patches;
    MainDol(QDataStream &stream, const QVector<AddressSection> &mappingSections);
    QVector<MapDescriptor> readMainDol(QDataStream &stream);
    QVector<MapDescriptor> writeMainDol(QDataStream &stream, const QVector<MapDescriptor> &mapDescriptors);
private:
    AddressMapper setupAddressMapper(QDataStream &stream, const QVector<AddressSection> &fileMappingSections);
    FreeSpaceManager setupFreeSpaceManager(AddressMapper addressMapper);
    QVector<QSharedPointer<DolIO>> setupPatches();
};

#endif // MAINDOL_H
