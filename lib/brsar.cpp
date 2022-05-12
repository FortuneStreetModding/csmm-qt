#include "brsar.h"
#include <QtDebug>
#include "music.h"

// documentation:
//   http://wiki.tockdom.com/wiki/BRSAR_(File_Format)
//   https://github.com/soopercool101/BrawlCrate/blob/master/BrawlLib/SSBB/Types/Audio/RSAR.cs

namespace Brsar {

QDataStream &operator>>(QDataStream &stream, Brsar::SectionHeader &data) {
    char header[4];
    stream.readRawData(header, 4);
    if (QByteArray(header, 4) != data.magicNumber) {
        stream.setStatus(QDataStream::ReadCorruptData);
        return stream;
    }
    stream >> data.sectionSize;
    data.sectionStart = stream.device()->pos();
    return stream;
}

QDataStream &operator>>(QDataStream &stream, Brsar::InfoSectionCollectionEntry &data) {
    data.entryStart = stream.device()->pos();
    stream >> data.externalFileLength;
    stream >> data.audioDataLength;
    quint32 unknown;
    stream >> unknown;
    if(unknown != data.unknown){
        stream.setStatus(QDataStream::ReadCorruptData);
        return stream;
    }
    quint32 useOffset;
    stream >> useOffset;
    if(useOffset != 0x01000000 && useOffset != 0x00000000){
        stream.setStatus(QDataStream::ReadCorruptData);
        return stream;
    }
    stream >> data.externalFileNameOffset;
    stream >> useOffset;
    if(useOffset != 0x01000000 && useOffset != 0x00000000){
        stream.setStatus(QDataStream::ReadCorruptData);
        return stream;
    }
    stream >> data.offsetSecondSubsection;
    // external file name
    stream.device()->seek(data.header->sectionStart + data.externalFileNameOffset);
    char buf[128+1] = {0};
    stream.readRawData(buf, 128);
    data.externalFileName = buf;

    // we do not care about handling the second subsection
    return stream;
}

QDataStream &operator>>(QDataStream &stream, Brsar::InfoSectionCollectionTable &data) {
    stream >> data.collectionTableSize;

    for(quint32 i = 0; i < data.collectionTableSize; i++) {
        quint32 unknown;
        stream >> unknown;
        if(unknown != data.unknown){
            stream.setStatus(QDataStream::ReadCorruptData);
            return stream;
        }
        quint32 collectionTableEntryOffset;
        stream >> collectionTableEntryOffset;
        data.collectionTableEntryOffsets.append(collectionTableEntryOffset);
    }
    for(quint32 i = 0; i < data.collectionTableSize; i++) {
        stream.device()->seek(data.header->sectionStart + data.collectionTableEntryOffsets[i]);
        InfoSectionCollectionEntry collectionEntry(data.header);
        stream >> collectionEntry;
        data.collectionTableEntries.append(collectionEntry);
    }
    return stream;
}

QDataStream &operator>>(QDataStream &stream, Brsar::InfoSectionSoundDataEntry &data) {
    data.entryStart = stream.device()->pos();
    stream >> data.fileNameIndex;
    stream >> data.fileCollectionIndex;
    stream >> data.playerId;
    quint32 unknown;
    stream >> unknown;
    if(unknown != data.unknown1){
        stream.setStatus(QDataStream::ReadCorruptData);
        return stream;
    }
    stream >> data.offsetThirdSubsection; // we do not actually need it, since it always comes directly after secondSubsection
    stream >> data.volume;
    stream >> data.playerPriority;
    stream >> data.soundType;
    stream >> data.remoteFilter;
    stream >> data.unknownFlags;
    stream >> data.offsetSecondSubsection; // we do not actually need it, since it always comes directly after this dataEntry
    stream >> data.userParam1;
    stream >> data.userParam2;
    stream >> data.panMode;
    stream >> data.panCurve;
    stream >> data.actorPlayerId;
    quint8 unknown2;
    stream >> unknown2;
    if(unknown2 != data.unknown2){
        stream.setStatus(QDataStream::ReadCorruptData);
        return stream;
    }
    stream.device()->seek(data.header->sectionStart + data.offsetSecondSubsection);
    // -- sound data subsection 2 --
    // note: we always assume RSTM, as such this brsar parser is incomplete and only for
    //       CSMM relevant
    stream >> data.startPosition;
    stream >> data.allocChannelCount;
    stream >> data.rstmAllocTrack;
    stream >> data.unknown3;
    // -- sound data subsection 3 --
    stream.device()->seek(data.header->sectionStart + data.offsetThirdSubsection);
    stream >> data.sound3dParamFlags;
    stream >> data.decayCurve;
    stream >> data.decayRatio;
    return stream;
}

QDataStream &operator>>(QDataStream &stream, Brsar::InfoSectionSoundDataTable &data) {
    stream >> data.soundTableSize;

    for(quint32 i = 0; i < data.soundTableSize; i++) {
        quint32 unknown;
        stream >> unknown;
        if(unknown != data.unknown){
            stream.setStatus(QDataStream::ReadCorruptData);
            return stream;
        }
        quint32 soundTableEntryOffset;
        stream >> soundTableEntryOffset;
        data.soundTableEntryOffsets.append(soundTableEntryOffset);
    }
    for(quint32 i = 0; i < data.soundTableSize; i++) {
        stream.device()->seek(data.header->sectionStart + data.soundTableEntryOffsets[i]);
        InfoSectionSoundDataEntry soundDataEntry(data.header);
        stream >> soundDataEntry;
        data.soundTableEntries.append(soundDataEntry);
    }
    return stream;
}

QDataStream &operator>>(QDataStream &stream, Brsar::InfoSection &data) {
    stream >> data.header;

    quint32 unknown;
    stream >> unknown;
    if(unknown != data.unknown1){
        stream.setStatus(QDataStream::ReadCorruptData);
        return stream;
    }
    stream >> data.offsetSoundDataTable;
    stream >> unknown;
    if(unknown != data.unknown2){
        stream.setStatus(QDataStream::ReadCorruptData);
        return stream;
    }
    stream >> data.offsetSoundbankTable;
    stream >> unknown;
    if(unknown != data.unknown3){
        stream.setStatus(QDataStream::ReadCorruptData);
        return stream;
    }
    stream >> data.offsetPlayerInfoTable;
    stream >> unknown;
    if(unknown != data.unknown4){
        stream.setStatus(QDataStream::ReadCorruptData);
        return stream;
    }
    stream >> data.offsetCollectionTable;
    stream >> unknown;
    if(unknown != data.unknown5){
        stream.setStatus(QDataStream::ReadCorruptData);
        return stream;
    }
    stream >> data.offsetGroupTable;
    stream >> unknown;
    if(unknown != data.unknown6){
        stream.setStatus(QDataStream::ReadCorruptData);
        return stream;
    }
    stream.device()->seek(data.header.sectionStart + data.offsetSoundDataTable);
    stream >> data.soundDataTable;
    stream.device()->seek(data.header.sectionStart + data.offsetCollectionTable);
    stream >> data.collectionTable;
    return stream;
}

QDataStream &operator>>(QDataStream &stream, Brsar::SymbSectionFileNameTable &data) {
    stream >> data.fileNameTableSize;

    for(quint32 i = 0; i < data.fileNameTableSize; i++) {
        quint32 fileNameTableEntryOffset;
        stream >> fileNameTableEntryOffset;
        data.fileNameTableEntryOffsets.append(fileNameTableEntryOffset);
    }
    for(quint32 i = 0; i < data.fileNameTableSize; i++) {
        stream.device()->seek(data.header->sectionStart + data.fileNameTableEntryOffsets[i]);
        char buf[128+1] = {0};
        stream.readRawData(buf, 128);
        data.fileNameTableEntries.append(buf);
    }
    return stream;
}

QDataStream &operator>>(QDataStream &stream, Brsar::SymbSection &data) {
    stream >> data.header;
    stream >> data.offsetFileNameTable;
    stream >> data.maskTableOffset1;
    stream >> data.maskTableOffset2;
    stream >> data.maskTableOffset3;
    stream >> data.maskTableOffset4;
    stream.device()->seek(data.header.sectionStart + data.offsetFileNameTable);
    stream >> data.fileNameTable;
    return stream;
}

QDataStream &operator>>(QDataStream &stream, Brsar::File &data) {
    char header[4];
    stream.readRawData(header, 4);
    if (QByteArray(header, 4) != data.magicNumber) {
        stream.setStatus(QDataStream::ReadCorruptData);
        return stream;
    }
    quint16 byteOrderMark;
    stream >> byteOrderMark;
    if(byteOrderMark != data.byteOrderMark){
        stream.setStatus(QDataStream::ReadCorruptData);
        return stream;
    }
    quint16 fileFormatVersion;
    stream >> fileFormatVersion;
    if(fileFormatVersion != data.fileFormatVersion){
        stream.setStatus(QDataStream::ReadCorruptData);
        return stream;
    }
    stream >> data.fileLength;
    quint16 headerSize;
    stream >> headerSize;
    if(headerSize != data.headerSize){
        stream.setStatus(QDataStream::ReadCorruptData);
        return stream;
    }
    quint16 sectionCount;
    stream >> sectionCount;
    if(sectionCount != data.sectionCount){
        stream.setStatus(QDataStream::ReadCorruptData);
        return stream;
    }
    stream >> data.symbOffset;
    stream >> data.symbLength;
    stream >> data.infoOffset;
    stream >> data.infoLength;
    stream >> data.fileOffset;
    stream >> data.fileLength;
    stream.skipRawData(24);
    stream.device()->seek(data.symbOffset);
    stream >> data.symb;
    stream.device()->seek(data.infoOffset);
    stream >> data.info;
    // set the pointers to corresponding entries
    const InfoSectionCollectionEntry *collectionEntry;
    const InfoSectionSoundDataEntry *soundDataEntry;
    const QString *fileName;
    for(int i = 0; i < data.info.soundDataTable.soundTableEntries.length(); i++) {
        soundDataEntry = &data.info.soundDataTable.soundTableEntries.at(i);
        fileName = &data.symb.fileNameTable.fileNameTableEntries.at(soundDataEntry->fileNameIndex);
        collectionEntry = &data.info.collectionTable.collectionTableEntries.at(soundDataEntry->fileCollectionIndex);

        Entry entry(collectionEntry, soundDataEntry, fileName);
        data.entries.append(entry);
    }
    return stream;
}

bool containsCsmmEntries(QDataStream &stream) {
    Brsar::File brsar;
    stream >> brsar;
    if(stream.status() == QDataStream::ReadCorruptData)
        return false;
    for(int i = 0; i < brsar.entries.length(); i++) {
        auto &entry = brsar.entries[i];
        if(entry.fileName == QString("CSMM_999")) {
            return true;
        }
    }
    return false;
}

int patch(QDataStream &stream, QVector<MapDescriptor> &descriptors) {
    Brsar::File brsar;
    stream >> brsar;

    if(stream.status() == QDataStream::ReadCorruptData)
        return -1;

    // find boundary indices
    int brsarIndex_min = 0, brsarIndex_max = 0;
    for(int i = 0; i < brsar.entries.length(); i++) {
        auto &entry = brsar.entries[i];
        if(entry.fileName == QString("CSMM_000")) {
            brsarIndex_min = i;
        }
        if(entry.fileName == QString("CSMM_999")) {
            brsarIndex_max = i;
        }
    }

    QMap<QString, quint32> mapBrstmBaseFilenameToBrsarIndex;
    int brsarIndex = brsarIndex_min;
    for (int i=0; i<descriptors.length(); i++) {
        auto &descriptor = descriptors[i];
        auto keys = descriptor.music.keys();
        for (auto &musicType: keys) {
            auto &musicEntry = descriptor.music[musicType];
            if(mapBrstmBaseFilenameToBrsarIndex.contains(musicEntry.brstmBaseFilename)) {
                // reuse the brsar index and set it to the map descriptor
                musicEntry.brsarIndex = mapBrstmBaseFilenameToBrsarIndex[musicEntry.brstmBaseFilename];
            } else {
                if(brsarIndex > brsarIndex_max) {
                    return -2;
                }
                // map the brstm filename to the current brsar index
                mapBrstmBaseFilenameToBrsarIndex[musicEntry.brstmBaseFilename] = brsarIndex;
                // determine music type (if its ME or BGM)
                bool isBgm = Music::musicTypeIsBgm(musicType);
                quint32 playerId;
                quint8 playerPriority;
                if(isBgm) {
                    playerId = 0;
                    playerPriority = 110;
                } else {
                    playerId = 1;
                    playerPriority = 127;
                }
                // patch sound data entry
                stream.device()->seek(brsar.entries[brsarIndex].soundDataEntry->entryStart);
                stream.device()->skip(8);
                stream << playerId;
                stream.device()->skip(8);
                stream << musicEntry.volume;
                stream << playerPriority;
                // patch collection entry
                stream.device()->seek(brsar.entries[brsarIndex].collectionEntry->entryStart);
                stream << musicEntry.brstmFileSize;
                stream.device()->seek(brsar.entries[brsarIndex].collectionEntry->header->sectionStart + brsar.entries[brsarIndex].collectionEntry->externalFileNameOffset);
                QByteArray data(QString("stream/%1.brstm").arg(musicEntry.brstmBaseFilename).toUtf8());
                data = data.leftJustified(7 + 48 + 6, '\0', true);
                stream.writeRawData(data, data.size());
                // set the brsar index of the map descriptor
                musicEntry.brsarIndex = brsarIndex;
                brsarIndex++;
            }
        }
    }
    return brsarIndex - brsarIndex_min;
}

}
