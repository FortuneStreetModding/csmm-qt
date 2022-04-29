#include "dolio.h"
#include "lib/datafileset.h"

void DolIO::loadFiles(const QString &root, GameInstance &gameInstance, const ModListType &) {
    auto mainDolLoc = QDir(root).filePath(MAIN_DOL);
    QFile mainDolFile(mainDolLoc);
    if (!mainDolFile.open(QFile::ReadOnly)) {
        throw ModException(QString("could not open file %1").arg(mainDolLoc));
    }
    QDataStream mainDolStream(&mainDolFile);
    readAsm(mainDolStream, gameInstance.addressMapper(), gameInstance.mapDescriptors());
}

void DolIO::saveFiles(const QString &root, GameInstance &gameInstance, const ModListType &) {
    auto mainDolLoc = QDir(root).filePath(MAIN_DOL);
    QFile mainDolFile(mainDolLoc);
    if (!mainDolFile.open(QFile::ReadWrite)) {
        throw ModException(QString("could not open file %1").arg(mainDolLoc));
    }
    QDataStream mainDolStream(&mainDolFile);
    write(mainDolStream, gameInstance.addressMapper(), gameInstance.mapDescriptors(), gameInstance.freeSpaceManager());
}

quint32 DolIO::allocate(const QByteArray &data, const QString &purpose) {
    return fsmPtr->allocateUnusedSpace(data, *streamPtr, *mapperPtr, purpose);
}

quint32 DolIO::allocate(const QVector<quint32> &words, const QString &purpose) {
    QByteArray data;
    QDataStream dataStream(&data, QIODevice::WriteOnly);
    for (auto word: words) {
        dataStream << word;
    }
    return allocate(data, purpose);
}

quint32 DolIO::allocate(const QVector<quint16> &words, const QString &purpose) {
    QByteArray data;
    QDataStream dataStream(&data, QIODevice::WriteOnly);
    for (auto word: words) {
        dataStream << word;
    }
    return allocate(data, purpose);
}

quint32 DolIO::allocate(const QString &str) {
    if (str.isEmpty()) {
        return 0;
    }
    QByteArray data(str.toUtf8());
    data.append('\0');
    return allocate(data, "");
}

void DolIO::write(QDataStream &stream, const AddressMapper &addressMapper, const std::vector<MapDescriptor> &mapDescriptors, FreeSpaceManager &freeSpaceManager) {
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
