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

void SquareData::updateValueFromShopModel() {
    value = shopModel*10;
}
void SquareData::updatePriceFromValue() {
    qreal yield = -0.15 * qPow(0.2, 0.005 * value) + 0.2;
    price = qRound(value * yield - 0.2);
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

OrderedMap<QString, SquareType> textToSquareTypes = {
    {"Property", Property},
    {"Bank", Bank},
    {"Venture", VentureSquare},
    {"Spade", SuitSquareSpade},
    {"Heart", SuitSquareHeart},
    {"Diamond", SuitSquareDiamond},
    {"Club", SuitSquareClub},
    {"Spade (Change-of-Suit)", ChangeOfSuitSquareSpade},
    {"Heart (Change-of-Suit)", ChangeOfSuitSquareHeart},
    {"Diamond (Change-of-Suit)", ChangeOfSuitSquareDiamond},
    {"Club (Change-of-Suit)", ChangeOfSuitSquareClub},
    {"Take-A-Break", TakeABreakSquare},
    {"Boon", BoonSquare},
    {"Boom", BoomSquare},
    {"Stockbroker", StockBrokerSquare},
    {"Roll On", RollOnSquare},
    {"Arcade", ArcadeSquare},
    {"Switch", SwitchSquare},
    {"Cannon", CannonSquare},
    {"Backstreet A", BackStreetSquareA},
    {"Backstreet B", BackStreetSquareB},
    {"Backstreet C", BackStreetSquareC},
    {"Backstreet D", BackStreetSquareD},
    {"Backstreet E", BackStreetSquareE},
    {"One-Way Alley Door A", OneWayAlleyDoorA},
    {"One-Way Alley Door B", OneWayAlleyDoorB},
    {"One-Way Alley Door C", OneWayAlleyDoorC},
    {"One-Way Alley Door D", OneWayAlleyDoorD},
    {"Lift/Magmalice Start", LiftMagmaliceSquareStart},
    {"Lift End", LiftSquareEnd},
    {"Magmalice", MagmaliceSquare},
    {"One-Way Alley End", OneWayAlleySquare},
    {"Event", EventSquare},
    {"Vacant Plot", VacantPlot}
};
OrderedMap<QString, quint8> textToShopTypes = {
    { "",0},
    { "(50g) Scrap-paper shop",5},
    { "(60g) Wool shop",6},
    { "(70g) Bottle store",7},
    { "(80g) Secondhand book shop",8},
    { "(90g) Scrap-metal supplier",9},
    {"(100g) Stationery shop",10},
    {"(110g) General store",11},
    {"(120g) Florist's",12},
    {"(130g) Ice-cream shop",13},
    {"(140g) Comic-book shop",14},
    {"(150g) Dairy",15},
    {"(160g) Doughnut shop",16},
    {"(170g) Pizza shack",17},
    {"(180g) Bakery",18},
    {"(190g) Grocery store",19},
    {"(200g) Pharmacy",20},
    {"(210g) Fish market",21},
    {"(220g) Toy shop",22},
    {"(230g) Bookshop",23},
    {"(240g) Cosmetics boutique",24},
    {"(250g) T-shirt shop",25},
    {"(260g) Fruit stall",26},
    {"(270g) Photography studio",27},
    {"(280g) Coffee shop",28},
    {"(290g) Butcher shop",29},
    {"(300g) Restaurant",30},
    {"(310g) Barbershop",31},
    {"(320g) Hat boutique",32},
    {"(330g) Hardware store",33},
    {"(340g) Gift shop",34},
    {"(350g) Launderette",35},
    {"(360g) Shoe shop",36},
    {"(370g) Clothing store",37},
    {"(380g) Optician's",38},
    {"(390g) Clockmaker's",39},
    {"(400g) Furniture shop",40},
    {"(410g) Sports shop",41},
    {"(420g) Locksmith's",42},
    {"(430g) Glassmaker's",43},
    {"(440g) Sushi restaurant",44},
    {"(450g) Art gallery",45},
    {"(460g) Leatherware boutique",46},
    {"(470g) Pet shop",47},
    {"(480g) Nail salon",48},
    {"(490g) Spice shop",49},
    {"(500g) Music shop",50},
    {"(510g) Surf shop",51},
    {"(520g) Boating shop",52},
    {"(530g) Cartographer's",53},
    {"(540g) Alloy rims shop",54},
    {"(550g) Fashion boutique",55},
    {"(560g) Waxworks",56},
    {"(570g) Lens shop",57},
    {"(580g) Kaleidoscope shop",58},
    {"(590g) Crystal ball shop",59},
    {"(610g) Gemstone supplier",61},
    {"(620g) Taxidermy studio",62},
    {"(650g) Antiques dealer's",65},
    {"(680g) Goldsmith's",68},
    {"(700g) Fossil shop",70},
    {"(720g) Music-box shop",72},
    {"(750g) Marionette workshop",75},
    {"(760g) Health shop",76},
    {"(800g) Organic food shop",80},
    {"(810g) Bridal boutique",81},
    {"(850g) Autograph shop",85},
    {"(900g) Meteorite shop",90},
    {"(980g) Department store",98}
};

QString squareTypeToText(SquareType type) {
    for (auto it = textToSquareTypes.begin(); it != textToSquareTypes.end(); ++it) {
        if (it.value() == type) {
            return it.key();
        }
    }
    return "";
}
SquareType textToSquareType(QString string) { return textToSquareTypes.value(string, Property); }
QList<QString> squareTexts() { return textToSquareTypes.keys(); }

QString shopTypeToText(quint8 shopType) {
    for (auto it = textToShopTypes.begin(); it != textToShopTypes.end(); ++it) {
        if (it.value() == shopType) {
            return it.key();
        }
    }
    return "";
}
quint8 textToShopType(QString string) { return textToShopTypes.value(string, 0); }
QList<QString> shopTexts() { return textToShopTypes.keys(); }
