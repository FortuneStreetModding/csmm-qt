#include "brsar.h"
#include <QtDebug>

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

QDataStream &operator>>(QDataStream &stream, Brsar::InfoSection &data) {
    stream >> data.header;
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
    // How to write a string:
    /*  QByteArray data(str.toUtf8());
        data.append('\0');
        stream.writeRawData(bytes, bytes.size());  */

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
    return stream;
}

void patch(QDataStream &stream, const QVector<MapDescriptor> &mapDescriptors) {
    Brsar::File brsar;
    stream >> brsar;

}

}
