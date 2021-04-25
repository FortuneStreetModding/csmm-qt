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

struct SymbSection {
    static constexpr size_t HEADER_SIZE = 0x1c;
    SymbSection() : header("SYMB") {}
    quint32 sectionSize;
    quint32 fileNameOffset;
    quint32 maskTableOffestSounds;
    quint32 maskTableOffestTypes;
    quint32 maskTableOffestGroups;
    quint32 maskTableOffestBanks;
    friend QDataStream &operator>>(QDataStream &stream, SymbSection &data);
    friend QDataStream &operator<<(QDataStream &stream, const SymbSection &data);
private:
    Header header;
};

struct InfoSection {
    static constexpr size_t HEADER_SIZE = 0x38;
    InfoSection() : header("INFO") {}

    friend QDataStream &operator>>(QDataStream &stream, InfoSection &data);
    friend QDataStream &operator<<(QDataStream &stream, const InfoSection &data);
private:
    Header header;
};

struct FileSection {
    static constexpr size_t HEADER_SIZE = 0x20;
    FileSection() : header("FILE") {}

    friend QDataStream &operator>>(QDataStream &stream, FileSection &data);
    friend QDataStream &operator<<(QDataStream &stream, const FileSection &data);
private:
    Header header;
};

struct File {
    static constexpr size_t HEADER_SIZE = 0x40;
    File() : header("RSAR") {}
    const quint16 byteOrderMark = 0xFEFF;
    const quint16 fileFormatVersion = 0x0104;
    quint32 fileSize;
    const quint16 headerSize = HEADER_SIZE;
    const quint16 sectionCount = 3;
    quint32 symbOffset;
    quint32 symbLength;
    quint32 infoOffset;
    quint32 infoLength;
    quint32 fileOffset;
    quint32 fileLength;
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
