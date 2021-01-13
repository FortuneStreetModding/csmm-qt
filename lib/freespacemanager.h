#ifndef FREESPACEMANAGER_H
#define FREESPACEMANAGER_H

#include <QMap>
#include "addressmapping.h"

class FreeSpaceManager {
public:
    void addFreeSpace(quint32 start, quint32 end);
    int calculateTotalRemainingFreeSpace() const;
    int calculateTotalFreeSpace() const;
    int calculateLargestFreeSpaceBlockSize() const;
    int calculateLargestRemainingFreeSpaceBlockSize() const;
    quint32 allocateUnusedSpace(const QByteArray &bytes, QDataStream &stream, const AddressMapper &fileMapper);
    void nullTheFreeSpace(QDataStream &stream, const AddressMapper &addressMapper);
    void reset();
private:
    QMap<quint32, quint32> remainingFreeSpaceBlocks;
    QMap<quint32, quint32> totalFreeSpaceBlocks;
    QMap<QByteArray, quint32> reuseValues;
    bool startedAllocating = false;

    quint32 findSuitableFreeSpaceBlock(int requiredSize) const;
    int calculateFreeSpace(const QMap<quint32, quint32> &freeSpaceBlocks) const;
    quint32 findLargestFreeSpaceBlock(const QMap<quint32, quint32> &freeSpaceBlocks) const;
};

#endif // FREESPACEMANAGER_H
