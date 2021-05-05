#ifndef BRSAR_H
#define BRSAR_H

#include <QDataStream>
#include <QVector>

namespace Brsar {

struct Header {
    Header(const QByteArray &magicNumberValue) : magicNumber(magicNumberValue) {};

    const QByteArray &getMagicNumber() const {
        return magicNumber;
    }

    friend QDataStream &operator>>(QDataStream &stream, Header &data);
    friend QDataStream &operator<<(QDataStream &stream, const Header &data);
private:
    const QByteArray magicNumber;
};

// --- SYMB ---

struct SymbMaskEntry {
    static constexpr size_t HEADER_SIZE = 0x14;
    quint16 flags;
    qint16 bit;
    qint32 leftId;
    qint32 rightId;
    qint32 stringId;
    qint32 index;
    friend QDataStream &operator>>(QDataStream &stream, SymbMaskEntry &data);
    friend QDataStream &operator<<(QDataStream &stream, const SymbMaskEntry &data);
private:
};

struct SymbMaskHeader {
    static constexpr size_t HEADER_SIZE = 0x8;
    quint32 rootId;
    quint32 numEntries;
    QVector<SymbMaskEntry> entries;
    friend QDataStream &operator>>(QDataStream &stream, SymbMaskHeader &data);
    friend QDataStream &operator<<(QDataStream &stream, const SymbMaskHeader &data);
private:
};

struct SymbSection {
    static constexpr size_t HEADER_SIZE = 0x1c;
    SymbSection() : header("SYMB") {}
    quint32 sectionSize;
    const quint32 fileNameOffset = 0x14;
    SymbMaskHeader maskTableOffestSounds;
    SymbMaskHeader maskTableOffestTypes;
    SymbMaskHeader maskTableOffestGroups;
    SymbMaskHeader maskTableOffestBanks;
    friend QDataStream &operator>>(QDataStream &stream, SymbSection &data);
    friend QDataStream &operator<<(QDataStream &stream, const SymbSection &data);
private:
    Header header;
};

// --- INFO ---

struct InfoSection {
    static constexpr size_t HEADER_SIZE = 0x38;
    InfoSection() : header("INFO") {}

    friend QDataStream &operator>>(QDataStream &stream, InfoSection &data);
    friend QDataStream &operator<<(QDataStream &stream, const InfoSection &data);
private:
    Header header;
};

// --- FILE ---

struct FileSection {
    static constexpr size_t HEADER_SIZE = 0x20;
    FileSection() : header("FILE") {}

    friend QDataStream &operator>>(QDataStream &stream, FileSection &data);
    friend QDataStream &operator<<(QDataStream &stream, const FileSection &data);
private:
    Header header;
};

// --- Brsar File ---

struct File {
    static constexpr size_t HEADER_SIZE = 0x40;
    File() : header("RSAR") {}
    const quint16 byteOrderMark = 0xFEFF;
    const quint16 fileFormatVersion = 0x0104;
    const quint16 headerSize = HEADER_SIZE;
    const quint16 sectionCount = 3;
    SymbSection symb;
    InfoSection info;
    FileSection file;

    friend QDataStream &operator>>(QDataStream &stream, File &data);
    friend QDataStream &operator<<(QDataStream &stream, const File &data);
private:
    Header header;
};

}

#endif // BRSAR_H
