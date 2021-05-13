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

struct SectionBase {
    SectionBase(const SectionHeader *header) : header(header) {}
    SectionBase(const SectionBase &other) : header(other.header) {}
    const SectionHeader *header;
};

struct InfoSectionCollectionEntry : SectionBase {
    InfoSectionCollectionEntry(const SectionHeader *header) : SectionBase(header) {}

    quint32 externalFileLength; // important so that the tracks will not end abruptly prematurely
    quint32 audioDataLength; // 0 for RSEQ or external files
    static const quint32 unknown1 = 0xFFFFFFFF;
    static const quint32 useOffset1 = 0x01000000;
    quint32 externalFileNameOffset;
    static const quint32 useOffset2 = 0x01000000;
    quint32 offsetSecondSubsection;
    // -- first subsection --
    QString externalFileName;
    friend QDataStream &operator>>(QDataStream &stream, InfoSectionCollectionEntry &data);
};

struct InfoSectionCollectionTable : SectionBase {
    InfoSectionCollectionTable(const SectionHeader *header) : SectionBase(header) {}

    quint32 collectionTableSize;
    static const quint32 unknown = 0x01000000;
    QVector<quint32> collectionTableEntryOffsets;
    QVector<InfoSectionCollectionEntry> collectionTableEntries;

    friend QDataStream &operator>>(QDataStream &stream, InfoSectionCollectionTable &data);
};

struct InfoSectionSoundDataEntry : SectionBase {
    InfoSectionSoundDataEntry(const SectionHeader *header) : SectionBase(header) {}
    quint32 fileNameIndex;
    quint32 fileCollectionIndex;
    quint32 playerId;
    static const quint32 unknown1 = 0x01000000;
    quint32 offsetThirdSubsection;
    quint8 volume;
    quint8 playerPriority;
    quint8 soundType;
    quint8 remoteFilter;
    quint32 unknownFlags;
    quint32 offsetSecondSubsection;
    quint32 userParam1;
    quint32 userParam2;
    quint8 panMode;
    quint8 panCurve;
    quint8 actorPlayerId;
    static const quint8 unknown2 = 0x00;
    // -- sound data subsection 2 --
    // note: we always assume RSTM, as such this brsar parser is incomplete and only for
    //       CSMM relevant
    quint32 startPosition;
    quint16 allocChannelCount;
    quint16 rstmAllocTrack;
    quint32 unknown3;
    // -- sound data subsection 3 --
    quint32 sound3dParamFlags;
    quint8 decayCurve;
    quint8 decayRatio;

    friend QDataStream &operator>>(QDataStream &stream, InfoSectionSoundDataEntry &data);
};

struct InfoSectionSoundDataTable : SectionBase {
    InfoSectionSoundDataTable(const SectionHeader *header) : SectionBase(header) {}

    quint32 soundTableSize;
    static const quint32 unknown = 0x01000000;
    QVector<quint32> soundTableEntryOffsets;
    QVector<InfoSectionSoundDataEntry> soundTableEntries;

    friend QDataStream &operator>>(QDataStream &stream, InfoSectionSoundDataTable &data);
};

struct InfoSection {
    InfoSection() : header("INFO"), soundDataTable(&header), collectionTable(&header) {}

    SectionHeader header;
    static const quint32 unknown1 = 0x01000000;
    quint32 offsetSoundDataTable;
    static const quint32 unknown2 = 0x01000000;
    quint32 offsetSoundbankTable;
    static const quint32 unknown3 = 0x01000000;
    quint32 offsetPlayerInfoTable;
    static const quint32 unknown4 = 0x01000000;
    quint32 offsetCollectionTable;
    static const quint32 unknown5 = 0x01000000;
    quint32 offsetGroupTable;
    static const quint32 unknown6 = 0x01000000;
    quint32 offsetSoundCountTable;

    InfoSectionSoundDataTable soundDataTable;
    InfoSectionCollectionTable collectionTable;

    friend QDataStream &operator>>(QDataStream &stream, InfoSection &data);
};

struct SymbSectionFileNameTable : SectionBase {
    SymbSectionFileNameTable(const SectionHeader *header) : SectionBase(header) {}
    quint32 fileNameTableSize;
    QVector<quint32> fileNameTableEntryOffsets;
    QVector<QString> fileNameTableEntries;

    friend QDataStream &operator>>(QDataStream &stream, SymbSectionFileNameTable &data);
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

struct Entry {
    Entry(const InfoSectionCollectionEntry * collectionEntry,
          const InfoSectionSoundDataEntry * soundDataEntry,
          const QString * fileName) :
        collectionEntry(collectionEntry),
        soundDataEntry(soundDataEntry),
        fileName(fileName) {}
    Entry(const Entry &other) :
        collectionEntry(other.collectionEntry),
        soundDataEntry(other.soundDataEntry),
        fileName(other.fileName) {}
    const InfoSectionCollectionEntry * collectionEntry;
    const InfoSectionSoundDataEntry * soundDataEntry;
    const QString * fileName;
};

struct File {
    const QByteArray magicNumber = "RSAR";
    static const quint16 byteOrderMark = 0xFEFF;
    static const quint16 fileFormatVersion = 0x0104;
    quint32 fileSize;
    static const quint16 headerSize = 0x40;
    static const quint16 sectionCount = 3;
    quint32 symbOffset;
    quint32 symbLength;
    quint32 infoOffset;
    quint32 infoLength;
    quint32 fileOffset;
    quint32 fileLength;
    SymbSection symb;
    InfoSection info;
    QVector<Entry> entries;
    friend QDataStream &operator>>(QDataStream &stream, File &data);
};

void patch(QDataStream &stream, QVector<MapDescriptor> &mapDescriptors);

}

#endif // BRSAR_H
