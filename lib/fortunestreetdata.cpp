#include "fortunestreetdata.h"
#include "orderedmap.h"
#include <QtMath>

QDataStream &operator>>(QDataStream &stream, WaypointData &data) {
    stream >> data.entryId;
    for (quint8 &dest: data.destinations) {
        stream >> dest;
    }
    return stream;
}

QDataStream &operator<<(QDataStream &stream, const WaypointData &data) {
    stream << data.entryId;
    for (quint8 dest: data.destinations) {
        stream << dest;
    }
    return stream;
}

QDataStream &operator>>(QDataStream &stream, SquareData &data) {
    stream >> data.squareType;
    stream >> data.positionX >> data.positionY;
    stream >> data.unknown1;
    for (auto &waypoint: data.waypoints) {
        stream >> waypoint;
    }
    stream >> data.districtDestinationId;
    stream >> data.oneWayLift;
    stream >> data.value >> data.price;
    stream >> data.unknown2;
    stream >> data.shopModel;
    return stream;
}

QDataStream &operator<<(QDataStream &stream, const SquareData &data) {
    stream << data.squareType;
    stream << data.positionX << data.positionY;
    stream << data.unknown1;
    for (auto &waypoint: data.waypoints) {
        stream << waypoint;
    }
    stream << data.districtDestinationId;
    stream << data.oneWayLift;
    stream << data.value << data.price;
    stream << data.unknown2;
    stream << data.shopModel;
    return stream;
}

QDataStream &operator>>(QDataStream &stream, Header &data) {
    char header[4];
    stream.readRawData(header, 4);
    if (QByteArray(header, 4) != data.magicNumber) {
        stream.setStatus(QDataStream::ReadCorruptData);
    } else {
        stream >> data.headerSize;
    }
    return stream;
}

QDataStream &operator<<(QDataStream &stream, const Header &data) {
    stream.writeRawData(data.magicNumber, 4);
    stream << data.headerSize;
    return stream;
}

QDataStream &operator>>(QDataStream &stream, BoardInfo &data) {
    stream >> data.header;
    stream.skipRawData(8);
    stream >> data.initialCash;
    stream >> data.targetAmount;
    stream >> data.baseSalary >> data.salaryIncrement;
    stream >> data.maxDiceRoll;
    stream >> data.galaxyStatus;
    stream.skipRawData(4);
    return stream;
}

QDataStream &operator<<(QDataStream &stream, const BoardInfo &data) {
    stream << Header(data.header.getMagicNumber(), BoardInfo::SIZE);
    stream << (quint64)0;
    stream << data.initialCash;
    stream << data.targetAmount;
    stream << data.baseSalary << data.salaryIncrement;
    stream << data.maxDiceRoll;
    stream << data.galaxyStatus;
    stream << (quint32)0;
    return stream;
}

QDataStream &operator>>(QDataStream &stream, BoardData &data) {
    quint16 squareCount;
    stream >> data.header;
    stream.skipRawData(4);
    stream >> squareCount;
    stream.skipRawData(2);
    for (quint16 i=0; i<squareCount; ++i) {
        SquareData square(i);
        stream >> square;
        data.squares.append(square);
    }
    return stream;
}

QDataStream &operator<<(QDataStream &stream, const BoardData &data) {
    stream << Header(data.header.getMagicNumber(), BoardData::SIZE + SquareData::SIZE*data.squares.size());
    stream << (quint32)0;
    stream << (quint16)data.squares.size();
    stream << (quint16)0;
    for (const auto &square: data.squares) {
        stream << square;
    }
    return stream;
}

QDataStream &operator>>(QDataStream &stream, BoardFile &data) {
    stream >> data.header;
    stream >> data.unknown;
    stream >> data.boardInfo;
    stream >> data.boardData;
    return stream;
}

QDataStream &operator<<(QDataStream &stream, const BoardFile &data) {
    stream << Header(data.header.getMagicNumber(), BoardFile::SIZE + SquareData::SIZE*data.boardData.squares.size());
    stream << data.unknown;
    stream << data.boardInfo;
    stream << data.boardData;
    return stream;
}
