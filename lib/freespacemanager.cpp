#include "freespacemanager.h"
#include <QDataStream>
#include <QDebug>
#include <QIODevice>
#include <limits>

void FreeSpaceManager::addFreeSpace(quint32 start, quint32 end) {
    if (startedAllocating) {
        // TODO throw exception here
    }
    totalFreeSpaceBlocks[end] = start;
    remainingFreeSpaceBlocks[end] = start;
}

quint32 FreeSpaceManager::findSuitableFreeSpaceBlock(int requiredSize) const {
    int smallestFreeSpaceBlockSize = std::numeric_limits<int>::max();
    quint32 smallestFreeSpaceBlockEnd = std::numeric_limits<quint32>::max();
    for (auto it=remainingFreeSpaceBlocks.begin(); it!=remainingFreeSpaceBlocks.end(); ++it) {
        quint32 start = it.value();
        quint32 end = it.key();
        int freeSpaceBlockSize = (int)(end - start);
        if (freeSpaceBlockSize < smallestFreeSpaceBlockSize && freeSpaceBlockSize >= requiredSize) {
            smallestFreeSpaceBlockSize = freeSpaceBlockSize;
            smallestFreeSpaceBlockEnd = end;
        }
    }
    if (smallestFreeSpaceBlockEnd == std::numeric_limits<quint32>::max()) {
        throw Exception(QString("requested %1 bytes but not enough free space").arg(requiredSize));
    }
    return smallestFreeSpaceBlockEnd;
}

int FreeSpaceManager::calculateFreeSpace(const QMap<quint32, quint32> &freeSpaceBlocks) const {
    int result = 0;
    for (auto it=freeSpaceBlocks.begin(); it!=freeSpaceBlocks.end(); ++it) {
        result += it.key() - it.value();
    }
    return result;
}

int FreeSpaceManager::calculateTotalRemainingFreeSpace() const { return calculateFreeSpace(remainingFreeSpaceBlocks); }
int FreeSpaceManager::calculateTotalFreeSpace() const { return calculateFreeSpace(totalFreeSpaceBlocks); }

quint32 FreeSpaceManager::findLargestFreeSpaceBlock(const QMap<quint32, quint32> &freeSpaceBlocks) const {
    int largestBlockSize = -1;
    quint32 largestBlockEnd = std::numeric_limits<quint32>::max();
    for (auto it=freeSpaceBlocks.begin(); it!=freeSpaceBlocks.end(); ++it) {
        int candidate = it.key() - it.value();
        if (candidate > largestBlockSize) {
            largestBlockSize = candidate;
            largestBlockEnd = it.key();
        }
    }
    if (largestBlockEnd == std::numeric_limits<quint32>::max()) {
        throw Exception("no blocks found"); // should never happen
    }
    return largestBlockEnd;
}

int FreeSpaceManager::calculateLargestFreeSpaceBlockSize() const {
    quint32 end = findLargestFreeSpaceBlock(totalFreeSpaceBlocks);
    return end - totalFreeSpaceBlocks[end];
}

int FreeSpaceManager::calculateLargestRemainingFreeSpaceBlockSize() const {
    quint32 end = findLargestFreeSpaceBlock(remainingFreeSpaceBlocks);
    return end - remainingFreeSpaceBlocks[end];
}

quint32 FreeSpaceManager::allocateUnusedSpace(const QByteArray &bytes, QDataStream &stream, const AddressMapper &fileMapper, const QString &purpose, bool reuse) {
    QString purposeMsg = purpose.isEmpty() ? "" : QString(" for %1").arg(purpose);
    QString byteArrayAsString = byteArrayToStringOrHex(bytes);
    /*if (!startedAllocating) {
        for (auto it=remainingFreeSpaceBlocks.begin(); it!=remainingFreeSpaceBlocks.end(); ++it) {
            qDebug() << QString::number(it.value(), 16) << " to " << QString::number(it.key(), 16);
        }
    }*/
    startedAllocating = true;
    if (reuse && reuseValues.contains(bytes)) {
        qDebug().noquote() << "Reuse " + byteArrayAsString + " at " + QString::number(reuseValues[bytes], 16) + purposeMsg;
        return reuseValues[bytes];
    }
    quint32 end = findSuitableFreeSpaceBlock(bytes.size());
    quint32 start = remainingFreeSpaceBlocks[end];
    /*if (bytes == QByteArray::fromHex("98bb023a98bb022d98bb022e4e800020")) {
        qDebug() << "old start: " << remainingFreeSpaceBlocks[end];
    }*/
    quint32 newStart = start + bytes.size();
    while (newStart % 4 != 0) {
        ++newStart;
    }
    if (newStart > end) {
        newStart = end;
    }
    remainingFreeSpaceBlocks[end] = newStart;
    /*if (bytes == QByteArray::fromHex("98bb023a98bb022d98bb022e4e800020")) {
        qDebug() << "new start: " << remainingFreeSpaceBlocks[end];
    }*/
    stream.device()->seek(fileMapper.toFileAddress(start));
    stream.writeRawData(bytes, bytes.size());
    QByteArray padding(newStart - start - bytes.size(), '\0');
    stream.writeRawData(padding, padding.size());
    if (reuse) {
        reuseValues[bytes] = start;
    }
    qDebug().noquote() << "Allocate " + byteArrayAsString + " (" + QString::number(bytes.size()) + " bytes) at " + QString::number(start, 16) + purposeMsg;

    /*
    qDebug() << "== START" << (void *)this << "==";
    for (auto it=remainingFreeSpaceBlocks.begin(); it!=remainingFreeSpaceBlocks.end(); ++it) {
        qDebug() << QString::number(it.value(), 16) << " to " << QString::number(it.key(), 16);
    }
    qDebug() << "== END ==";
    */

    return start;
}

void FreeSpaceManager::nullTheFreeSpace(QDataStream &stream, const AddressMapper &addressMapper) {
    for (auto it=remainingFreeSpaceBlocks.begin(); it!=remainingFreeSpaceBlocks.end(); ++it) {
        stream.device()->seek(addressMapper.toFileAddress(it.value()));
        QByteArray nullBytes(it.key() - it.value(), '\0');
        stream.writeRawData(nullBytes, nullBytes.size());
    }
}

void FreeSpaceManager::reset() {
    remainingFreeSpaceBlocks = totalFreeSpaceBlocks;
    reuseValues.clear();
    startedAllocating = false;
}

QString FreeSpaceManager::byteArrayToStringOrHex(const QByteArray &byteArray) const {
    bool constainsAnsiCharsOnlyOrZero = true;
    bool containsAtLeastOneAnsiChar = false;
    for (int i = 0; i < byteArray.size(); ++i) {
        if (byteArray.at(i) >= 32) {
            // the character can be printed
            containsAtLeastOneAnsiChar = true;
        } else if(byteArray.at(i) != 0) {
            // the character cannot be printed and is not the null terminator
            constainsAnsiCharsOnlyOrZero = false;
        }
    }
    if (constainsAnsiCharsOnlyOrZero && containsAtLeastOneAnsiChar)
        return QString(byteArray);
    else
        return QString(byteArray.toHex());
}
