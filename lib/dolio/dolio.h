#ifndef DOLIO_H
#define DOLIO_H

#include <QDataStream>
#include "lib/freespacemanager.h"
#include "lib/mapdescriptor.h"

class DolIO {
public:
    void write(QDataStream &stream, const AddressMapper &addressMapper, const QVector<MapDescriptor> &mapDescriptors, FreeSpaceManager &freeSpaceManager);
    virtual void readAsm(QDataStream &stream, const AddressMapper &addressMapper, QVector<MapDescriptor> &mapDescriptors) = 0;
    virtual ~DolIO();
protected:
    virtual void writeAsm(QDataStream &stream, const AddressMapper &addressMapper, const QVector<MapDescriptor> &mapDescriptors) = 0;
    quint32 allocate(const QByteArray &data, const QString &purpose);
    quint32 allocate(const QVector<quint32> &words, const QString &purpose);
    quint32 allocate(const QString &str);
    QString resolveAddressToString(quint32 virtualAddress, QDataStream &stream, const AddressMapper &addressMapper);
private:
    FreeSpaceManager *fsmPtr;
    QDataStream *streamPtr;
    const AddressMapper *mapperPtr;
};

#endif // DOLIO_H
