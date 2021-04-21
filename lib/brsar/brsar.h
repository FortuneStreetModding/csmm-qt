#ifndef BRSAR_H
#define BRSAR_H

#include <QDataStream>
#include <QVector>

namespace Brsar {

struct Header {
    static constexpr size_t SIZE = 0x10;

    Header(const QByteArray &magicNumberValue, qint32 headerSizeValue = 0)
        : headerSize(headerSizeValue), magicNumber(magicNumberValue) {};
    qint32 headerSize;

    const QByteArray &getMagicNumber() const {
        return magicNumber;
    }

    friend QDataStream &operator>>(QDataStream &stream, Header &data);
    friend QDataStream &operator<<(QDataStream &stream, const Header &data);
private:
    const QByteArray magicNumber;
};

struct SymbSection {
    static constexpr size_t SIZE = 0x20;

    SymbSection() : header("SYMB") {}

    friend QDataStream &operator>>(QDataStream &stream, SymbSection &data);
    friend QDataStream &operator<<(QDataStream &stream, const SymbSection &data);
private:
    Header header;
};

struct InfoSection {
    static constexpr size_t SIZE = 0x20;

    InfoSection() : header("INFO") {}

    friend QDataStream &operator>>(QDataStream &stream, InfoSection &data);
    friend QDataStream &operator<<(QDataStream &stream, const InfoSection &data);
private:
    Header header;
};

struct FileSection {
    static constexpr size_t SIZE = 0x20;

    FileSection() : header("FILE") {}

    friend QDataStream &operator>>(QDataStream &stream, FileSection &data);
    friend QDataStream &operator<<(QDataStream &stream, const FileSection &data);
private:
    Header header;
};

struct File {
    const QByteArray magicNumberValue = "RSAR";
    const quint16 byteOrderMark = 0xFEFF;
    const quint16 fileFormatVersion = 0x0104;
    quint32 fileSize;
    const quint32 headerSize = 0x40;
    const quint16 sectionCount = 3;
    qint32 symbOffset;
    qint32 symbLength;
    qint32 infoOffset;
    qint32 infoLength;
    qint32 fileOffset;
    qint32 fileLength;
    SymbSection symb;
    InfoSection info;
    FileSection file;

    friend QDataStream &operator>>(QDataStream &stream, File &data);
    friend QDataStream &operator<<(QDataStream &stream, const File &data);
private:
};

}

#endif // BRSAR_H
