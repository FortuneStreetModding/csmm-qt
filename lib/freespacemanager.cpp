#include "freespacemanager.h"
#include <QDataStream>
#include <limits>

void FreeSpaceManager::addFreeSpace(quint32 start, quint32 end) {
    if (startedAllocating) {
        // todo throw exception here
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
        // todo notify of lack of free space
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
    // todo check largest block end == std::numeric_limits<quint32>::max()?
    return largestBlockEnd;
}

int FreeSpaceManager::calculateLargestFreeSpaceBlockSize() const {
    quint32 end = findLargestFreeSpaceBlock(totalFreeSpaceBlocks);
    return end - totalFreeSpaceBlocks[end];
}

int FreeSpaceManager::calculateLargestRemainingFreeSpaceBlockSize() const {
    quint32 end = findLargestFreeSpaceBlock(remainingFreeSpaceBlocks);
    // todo edit below to remainingFreeSpaceBlocks?
    return end - totalFreeSpaceBlocks[end];
}

quint32 FreeSpaceManager::allocateUnusedSpace(const QByteArray &bytes, QDataStream &stream, const AddressMapper &fileMapper) {
    startedAllocating = true;
    if (reuseValues.contains(bytes)) {
        return reuseValues[bytes];
    }
    quint32 end = findSuitableFreeSpaceBlock(bytes.size());
    quint32 start = remainingFreeSpaceBlocks[end];
    quint32 newStart = start + bytes.size();
    while (newStart % 4 != 0) {
        ++newStart;
    }
    if (newStart > end) {
        newStart = end;
    }
    remainingFreeSpaceBlocks[end] = newStart;
    stream.device()->seek(fileMapper.toFileAddress(start));
    stream.writeRawData(bytes, bytes.size());
    QByteArray padding(newStart - start - bytes.size(), '\0');
    stream.writeRawData(padding, padding.size());
    reuseValues[bytes] = start;
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
