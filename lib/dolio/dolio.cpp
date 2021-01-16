#include "dolio.h"

quint32 DolIO::allocate(const QByteArray &data) {
    return fsmPtr->allocateUnusedSpace(data, *streamPtr, *mapperPtr);
}

quint32 DolIO::allocate(const QVector<quint32> &words) {
    QByteArray data;
    QDataStream dataStream(&data, QIODevice::WriteOnly);
    for (auto word: words) {
        dataStream << word;
    }
    return allocate(data);
}

quint32 DolIO::allocate(const QString &str) {
    if (str.isEmpty()) {
        return 0;
    }
    QByteArray data(str.toUtf8());
    data.append('\0');
    return allocate(data);
}

void DolIO::write(QDataStream &stream, const AddressMapper &addressMapper, const QVector<MapDescriptor> &mapDescriptors, FreeSpaceManager &freeSpaceManager) {
    streamPtr = &stream;
    mapperPtr = &addressMapper;
    fsmPtr = &freeSpaceManager;
    writeAsm(stream, addressMapper, mapDescriptors);
    streamPtr = nullptr;
    mapperPtr = nullptr;
    fsmPtr = nullptr;
}

QString DolIO::resolveAddressToString(quint32 virtualAddress, QDataStream &stream, const AddressMapper &addressMapper) {
    if (!virtualAddress) {
        return "";
    }
    int fileAddress = addressMapper.toFileAddress(virtualAddress);
    auto pos = stream.device()->pos();
    stream.device()->seek(fileAddress);
    char buf[64+1] = {0};
    stream.readRawData(buf, 64);
    stream.device()->seek(pos);
    return buf;
}

DolIO::~DolIO() {}
