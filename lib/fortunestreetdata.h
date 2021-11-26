#ifndef FORTUNESTREETDATA_H
#define FORTUNESTREETDATA_H

#include <QDataStream>
#include <QVector>

struct WaypointData {
    quint8 entryId = 255;
    quint8 destinations[3] = {255, 255, 255};

    friend QDataStream &operator>>(QDataStream &stream, WaypointData &data);
    friend QDataStream &operator<<(QDataStream &stream, const WaypointData &data);
};

enum SquareType : quint16 {
    Property = 0x00,
    Bank = 0x01,
    VentureSquare = 0x02,

    SuitSquareSpade = 0x03,
    SuitSquareHeart = 0x04,
    SuitSquareDiamond = 0x05,
    SuitSquareClub = 0x06,
    ChangeOfSuitSquareSpade = 0x07,
    ChangeOfSuitSquareHeart = 0x08,
    ChangeOfSuitSquareDiamond = 0x09,
    ChangeOfSuitSquareClub = 0x0A,

    TakeABreakSquare = 0x0B,
    BoonSquare = 0x0C,
    BoomSquare = 0x0D,
    StockBrokerSquare = 0x0E,
    RollOnSquare = 0x10,
    ArcadeSquare = 0x11,
    SwitchSquare = 0x12,
    CannonSquare = 0x13,

    BackStreetSquareA = 0x14,
    BackStreetSquareB = 0x15,
    BackStreetSquareC = 0x16,
    BackStreetSquareD = 0x17,
    BackStreetSquareE = 0x18,

    OneWayAlleyDoorA = 0x1C,
    OneWayAlleyDoorB = 0x1D,
    OneWayAlleyDoorC = 0x1E,
    OneWayAlleyDoorD = 0x1F,

    LiftMagmaliceSquareStart = 0x20,
    MagmaliceSquare = 0x21,
    OneWayAlleySquare = 0x22,
    LiftSquareEnd = 0x23,

    unknown0x24 = 0x24,
    unknown0x25 = 0x25,
    unknown0x26 = 0x26,
    unknown0x27 = 0x27,
    unknown0x28 = 0x28,
    unknown0x29 = 0x29,
    unknown0x2A = 0x2A,
    unknown0x2B = 0x2B,
    unknown0x2C = 0x2C,
    unknown0x2D = 0x2D,

    EventSquare = 0x2E, // custom square
    unknown0x2F = 0x2F,

    VacantPlot = 0x30,
};

struct SquareData {
    static constexpr size_t SIZE = 0x20;

    SquareData(quint8 idValue = 0) : id(idValue) {}

    quint8 id;
    SquareType squareType = Property;
    qint16 positionX = 0;
    qint16 positionY = 0;
    quint16 unknown1 = 0;
    WaypointData waypoints[4];
    quint8 districtDestinationId = 0;
    quint8 oneWayLift = 0;
    quint16 value = 0;
    quint16 price = 0;
    quint8 unknown2 = 0;
    quint8 shopModel = 0;

    friend QDataStream &operator>>(QDataStream &stream, SquareData &data);
    friend QDataStream &operator<<(QDataStream &stream, const SquareData &data);
};

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

enum LoopingMode : quint16 {
    None = 0,
    Vertical = 2,
    Both = 1
};

enum BoardTheme : quint32 {
    Mario = 0,
    DragonQuest = 1
};

struct BoardInfo {
    static constexpr size_t SIZE = 0x20;

    BoardInfo() : header("I4DT") {}
    quint16 initialCash = 1000;
    quint16 targetAmount = 10000;
    quint16 baseSalary = 100;
    quint16 salaryIncrement = 100;
    quint16 maxDiceRoll = 6;
    LoopingMode galaxyStatus = None;

    friend QDataStream &operator>>(QDataStream &stream, BoardInfo &data);
    friend QDataStream &operator<<(QDataStream &stream, const BoardInfo &data);
private:
    Header header;
};

struct BoardData {
    static constexpr size_t SIZE = 0x10;

    BoardData() : header("I4PL") {}
    QVector<SquareData> squares;

    friend QDataStream &operator>>(QDataStream &stream, BoardData &data);
    friend QDataStream &operator<<(QDataStream &stream, const BoardData &data);
private:
    Header header;
};

struct BoardFile {
    static constexpr size_t SIZE = Header::SIZE + BoardInfo::SIZE + BoardData::SIZE;

    BoardFile(bool initialize = false) : header("I4DT") {
        if (initialize) {
            SquareData bank(0);
            bank.squareType = Bank;
            boardData.squares.append(bank);
        }
    }
    quint64 unknown;
    BoardInfo boardInfo;
    BoardData boardData;

    friend QDataStream &operator>>(QDataStream &stream, BoardFile &data);
    friend QDataStream &operator<<(QDataStream &stream, const BoardFile &data);
private:
    Header header;
};

#endif // FORTUNESTREETDATA_H
