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
    } else {
        stream >> data.headerSize;
    }
    return stream;
}

QDataStream &operator<<(QDataStream &stream, const Brsar::Header &data) {
    stream.writeRawData(data.magicNumber, 4);
    stream << data.headerSize;
    return stream;
}

QDataStream &operator>>(QDataStream &stream, Brsar::SymbSection &data) {
    return stream;
}

QDataStream &operator<<(QDataStream &stream, const Brsar::SymbSection &data) {
    return stream;
}

QDataStream &operator>>(QDataStream &stream, Brsar::InfoSection &data) {
    return stream;
}

QDataStream &operator<<(QDataStream &stream, const Brsar::InfoSection &data) {
    return stream;
}

QDataStream &operator>>(QDataStream &stream, Brsar::FileSection &data) {
    return stream;
}

QDataStream &operator<<(QDataStream &stream, const Brsar::FileSection &data) {
    return stream;
}

QDataStream &operator>>(QDataStream &stream, Brsar::File &data) {
    char header[4];
    stream.readRawData(header, 4);
    if (QByteArray(header, 4) != data.magicNumberValue) {
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

    stream.writeRawData(data.magicNumberValue, 4);
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

    QByteArray padding = QByteArray(24, 0);
    stream << padding;

    stream << symb;
    stream << info;
    stream << file;
    return stream;
}

}
