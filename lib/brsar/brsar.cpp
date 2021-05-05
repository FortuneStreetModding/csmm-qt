#include "brsar.h"
#include <QtDebug>

// documentation:
//   http://wiki.tockdom.com/wiki/BRSAR_(File_Format)
//   https://github.com/soopercool101/BrawlCrate/blob/master/BrawlLib/SSBB/Types/Audio/RSAR.cs

namespace Brsar {

QDataStream &operator>>(QDataStream &stream, Brsar::Header &data) {
    char header[4];
    stream.readRawData(header, 4);
    if (QByteArray(header, 4) != data.magicNumber) {
        stream.setStatus(QDataStream::ReadCorruptData);
        return stream;
    }
    return stream;
}

QDataStream &operator<<(QDataStream &stream, const Brsar::Header &data) {
    stream.writeRawData(data.magicNumber, 4);
    return stream;
}

// --- SYMB ---
QDataStream &operator>>(QDataStream &stream, Brsar::SymbSection &data) {
    stream >> data.header;
    return stream;
}

QDataStream &operator<<(QDataStream &stream, const Brsar::SymbSection &data) {
    stream << data.header;
    stream << data.sectionSize;
    stream << data.fileNameOffset;
    stream << data.maskTableOffestSounds;
    stream << data.maskTableOffestTypes;
    stream << data.maskTableOffestGroups;
    stream << data.maskTableOffestBanks;
    return stream;
}

// --- INFO ---
QDataStream &operator>>(QDataStream &stream, Brsar::InfoSection &data) {
    stream >> data.header;
    return stream;
}

QDataStream &operator<<(QDataStream &stream, const Brsar::InfoSection &data) {
    stream << data.header;
    return stream;
}

// --- FILE ---
QDataStream &operator>>(QDataStream &stream, Brsar::FileSection &data) {
    stream >> data.header;
    return stream;
}

QDataStream &operator<<(QDataStream &stream, const Brsar::FileSection &data) {
    stream << data.header;
    return stream;
}

// --- Brsar File ---

QDataStream &operator>>(QDataStream &stream, Brsar::File &data) {
    stream >> data.header;
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
    quint32 fileSize;
    stream >> fileSize;
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
    quint32 symbOffset;
    stream >> symbOffset;
    quint32 symbLength;
    stream >> symbLength;
    quint32 infoOffset;
    stream >> infoOffset;
    quint32 infoLength;
    stream >> infoLength;
    quint32 fileOffset;
    stream >> fileOffset;
    quint32 fileLength;
    stream >> fileLength;

    stream.skipRawData(24);

    stream >> data.symb;
    stream >> data.info;
    stream >> data.file;

    return stream;
}

QDataStream &operator<<(QDataStream &stream, const Brsar::File &data) {
    QByteArray symb;
    QDataStream symbStream(&symb, QIODevice::WriteOnly);
    symbStream << data.symb;
    QByteArray info;
    QDataStream infoStream(&info, QIODevice::WriteOnly);
    infoStream << data.info;
    QByteArray file;
    QDataStream fileStream(&file, QIODevice::WriteOnly);
    fileStream << data.file;

    stream << data.header;
    stream << (quint16) data.byteOrderMark;
    stream << (quint16) data.fileFormatVersion;
    // file size
    stream << (quint32) data.headerSize + symb.length() + info.length() + file.length();
    stream << (quint16) data.headerSize;
    stream << (quint16) data.sectionCount;
    // SYMB offset
    stream << (quint32) data.headerSize;
    // SYMB length
    stream << (quint32) symb.length();
    // INFO offset
    stream << (quint32) data.headerSize + symb.length();
    // INFO length
    stream << (quint32) info.length();
    // FILE offset
    stream << (quint32) data.headerSize + symb.length() + info.length();
    // FILE length
    stream << (quint32) file.length();
    // padding
    stream.writeRawData(QByteArray(24, 0), 24);

    stream.writeRawData(symb, symb.length());
    stream.writeRawData(info, info.length());
    stream.writeRawData(file, file.length());

    return stream;
}

}
