#ifndef BRSAR_H
#define BRSAR_H

#include <QDataStream>
#include <QVector>
#include "../mapdescriptor.h"

namespace Brsar{

struct SectionHeader {
    SectionHeader(const QByteArray &magicNumberValue): magicNumber(magicNumberValue) {};

    const QByteArray magicNumber;
    quint32 sectionSize;
    qint64 sectionStart; // the absolute file offset within the brsar file to the start of this section

    friend QDataStream &operator>>(QDataStream &stream, SectionHeader &data);
};

struct InfoSection {
    InfoSection() : header("INFO") {}

    SectionHeader header;
    const quint32 unknown1 = 0x01000000;
    quint32 offsetSoundDataTable;
    const quint32 unknown2 = 0x01000000;
    quint32 offsetSoundbankTable;
    const quint32 unknown3 = 0x01000000;
    quint32 offsetPlayerInfoTable;
    const quint32 unknown4 = 0x01000000;
    quint32 offsetCollectionTable;
    const quint32 unknown5 = 0x01000000;
    quint32 offsetGroupTable;
    const quint32 unknown6 = 0x01000000;
    quint32 offsetSoundCountTable;

    friend QDataStream &operator>>(QDataStream &stream, InfoSection &data);
};

struct SymbSectionFileNameTable {
    SymbSectionFileNameTable(const SectionHeader *header) : header(header) {}

    quint32 fileNameTableSize;
    QVector<quint32> fileNameTableEntryOffsets;
    QVector<QString> fileNameTableEntries;

    friend QDataStream &operator>>(QDataStream &stream, SymbSectionFileNameTable &data);
private:
    const SectionHeader *header;
};

struct SymbSection {
    SymbSection() : header("SYMB"), fileNameTable(&header) {}

    SectionHeader header;
    quint32 offsetFileNameTable;
    quint32 maskTableOffset1;
    quint32 maskTableOffset2;
    quint32 maskTableOffset3;
    quint32 maskTableOffset4;
    SymbSectionFileNameTable fileNameTable;

    friend QDataStream &operator>>(QDataStream &stream, SymbSection &data);
};

struct File {
    const QByteArray magicNumber = "RSAR";
    const quint16 byteOrderMark = 0xFEFF;
    const quint16 fileFormatVersion = 0x0104;
    quint32 fileSize;
    const quint16 headerSize = 0x40;
    const quint16 sectionCount = 3;
    quint32 symbOffset;
    quint32 symbLength;
    quint32 infoOffset;
    quint32 infoLength;
    quint32 fileOffset;
    quint32 fileLength;
    SymbSection symb;

    friend QDataStream &operator>>(QDataStream &stream, File &data);
};

void patch(QDataStream &stream, const QVector<MapDescriptor> &mapDescriptors);

}

#endif // BRSAR_H
