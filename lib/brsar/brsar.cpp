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
    stream >> data.sectionSize;
    stream >> data.fileNameOffset;
    stream >> data.maskTableOffestSounds;
    stream >> data.maskTableOffestTypes;
    stream >> data.maskTableOffestGroups;
    stream >> data.maskTableOffestBanks;
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
    symbStream << data.info;
    QByteArray file;
    QDataStream fileStream(&file, QIODevice::WriteOnly);
    symbStream << data.file;

    stream << data.header;
    stream << (quint16) data.byteOrderMark;
    stream << (quint16) data.fileFormatVersion;
    // file length
    stream << (quint32) data.headerSize + data.symbLength + data.infoLength + data.fileLength;
    stream << (quint16) data.headerSize;
    stream << (quint16) data.sectionCount;
    // symb offset
    stream << (quint32) data.headerSize;
    // symb size
    stream << (quint32) symb.length();
    // info offset
    stream << (quint32) data.headerSize + symb.length();
    // info size
    stream << (quint32) info.length();
    // file offset
    stream << (quint32) data.headerSize + symb.length() + info.length();
    // file size
    stream << (quint32) file.length();
    // padding
    stream.writeRawData(QByteArray(24, 0), 24);

    stream.writeRawData(symb, symb.length());
    stream.writeRawData(info, info.length());
    stream.writeRawData(file, file.length());

    return stream;
}

}
