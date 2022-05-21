#ifndef FREESPACEMANAGER_H
#define FREESPACEMANAGER_H

#include <QMap>
#include <QException>
#include "addressmapping.h"

class FreeSpaceManager {
public:
    void addFreeSpace(quint32 start, quint32 end);
    int calculateTotalRemainingFreeSpace() const;
    int calculateTotalFreeSpace() const;
    int calculateLargestFreeSpaceBlockSize() const;
    int calculateLargestRemainingFreeSpaceBlockSize() const;
    quint32 allocateUnusedSpace(const QByteArray &bytes, QDataStream &stream, const AddressMapper &fileMapper, const QString &purpose, bool reuse = true);
    void nullTheFreeSpace(QDataStream &stream, const AddressMapper &addressMapper);
    void reset();

    class Exception : public QException, public std::runtime_error {
    public:
        using std::runtime_error::runtime_error;
        const char *what() const noexcept override { return std::runtime_error::what(); }
        Exception(const QString &str) : std::runtime_error(str.toStdString()) {}
        void raise() const override { throw *this; }
        Exception *clone() const override { return new Exception(*this); }
    };
private:
    QMap<quint32, quint32> remainingFreeSpaceBlocks;
    QMap<quint32, quint32> totalFreeSpaceBlocks;
    QMap<QByteArray, quint32> reuseValues;
    bool startedAllocating = false;

    quint32 findSuitableFreeSpaceBlock(int requiredSize) const;
    int calculateFreeSpace(const QMap<quint32, quint32> &freeSpaceBlocks) const;
    quint32 findLargestFreeSpaceBlock(const QMap<quint32, quint32> &freeSpaceBlocks) const;
    QString byteArrayToStringOrHex(const QByteArray &byteArray) const;
};

#endif // FREESPACEMANAGER_H
