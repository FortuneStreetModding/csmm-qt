#include "mapdescriptor.h"
#include <QDebug>

bool OriginPoint::operator==(const OriginPoint &other) const {
    return x == other.x && y == other.y;
}

QSet<SquareType> MapDescriptor::readFrbFileInfo(const QDir &paramDir) {
    QSet<SquareType> usedSquareTypes;
    QFile frbFile(paramDir.filePath(frbFiles[0] + ".frb"));
    if (frbFile.open(QIODevice::ReadOnly)) {
        QDataStream stream(&frbFile);
        BoardFile boardFile;
        stream >> boardFile;
        maxDiceRoll = boardFile.boardInfo.maxDiceRoll;
        baseSalary = boardFile.boardInfo.baseSalary;
        salaryIncrement = boardFile.boardInfo.salaryIncrement;
        initialCash = boardFile.boardInfo.initialCash;
        if (!tourInitialCash) {
            tourInitialCash = initialCash;
        }
        // ignore target amount
        loopingMode = boardFile.boardInfo.galaxyStatus;
        // check looping mode radius?
        for (auto &square: boardFile.boardData.squares) {
            usedSquareTypes.insert(square.squareType);
        }
    }
    // check other frbs for validity?
    return usedSquareTypes;
}

QString MapDescriptor::toMd() const {
    QString result;
    QTextStream stream(&result);
    stream << "# " << names["en"] << "\n\n";
    stream << descs["en"] << "\n\n";
    stream << "# Screenshots\n\n";
    stream << "<Placeholder for screenshots>\n\n";
    stream << "## Features\n\n";
    stream << "### Map Properties\n\n";

    // todo implement the rest of this function
    return result;
}

bool MapDescriptor::operator==(const MapDescriptor &other) const {
    return mapSet == other.mapSet
            && zone == other.zone
            && order == other.order
            && isPracticeBoard == other.isPracticeBoard
            && ruleSet == other.ruleSet
            && initialCash == other.initialCash
            && targetAmount == other.targetAmount
            && baseSalary == other.baseSalary
            && salaryIncrement == other.salaryIncrement
            && maxDiceRoll == other.maxDiceRoll
            && std::equal(std::begin(ventureCards), std::end(ventureCards), std::begin(other.ventureCards))
            && std::equal(std::begin(frbFiles), std::end(frbFiles), std::begin(other.frbFiles))
            && switchRotationOrigins == other.switchRotationOrigins
            && theme == other.theme
            && background == other.background
            && bgmId == other.bgmId
            && mapIcon == other.mapIcon
            && loopingMode == other.loopingMode
            && loopingModeRadius == other.loopingModeRadius
            && loopingModeHorizontalPadding == other.loopingModeHorizontalPadding
            && loopingModeVerticalSquareCount == other.loopingModeVerticalSquareCount
            && tourBankruptcyLimit == other.tourBankruptcyLimit
            && tourInitialCash == other.tourInitialCash
            && std::equal(std::begin(tourCharacters), std::end(tourCharacters), std::begin(other.tourCharacters))
            && tourClearRank == other.tourClearRank
            && unlockId == other.unlockId
            && nameMsgId == other.nameMsgId
            && descMsgId == other.descMsgId
            && names == other.names
            && descs == other.descs
            && internalName == other.internalName
            && mapDescriptorFilePath == other.mapDescriptorFilePath;
}

QDebug &operator<<(QDebug &debugStream, const MapDescriptor &obj) {
    // todo add more info here?
    debugStream << "MapDescriptor(" << obj.names["en"] << ", firstFile=" << obj.frbFiles[0] << ")";
    return debugStream;
}

OriginPoint::operator QString() const {
    return QString("(%1, %2)").arg(x).arg(y);
}

static const QMap<QString, Character> stringToTourCharacters = {
    {"Mario", CharacterMario},
    {"Luigi",  Luigi},
    {"Peach",  Peach},
    {"Yoshi",  Yoshi},
    {"Bowser",  Bowser},
    {"Toad",  Toad},
    {"DonkeyKong",  DonkeyKong},
    {"Wario",  Wario},
    {"Waluigi",  Waluigi},
    {"Daisy",  Daisy},
    {"Birdo",  Birdo},
    {"DiddyKong",  DiddyKong},
    {"BowserJr",  BowserJr},
    {"Slime",  Slime},
    {"Princessa",  Princessa},
    {"Kiryl",  Kiryl},
    {"Yangus",  Yangus},
    {"Angelo",  Angelo},
    {"Platypunk",  Platypunk},
    {"Bianca",  Bianca},
    {"Alena",  Alena},
    {"Carver",  Carver},
    {"Jessica",  Jessica},
    {"Dragonlord",  Dragonlord},
    {"Stella",  Stella},
    {"Patty",  Patty}
};

QString tourCharacterToString(Character character) {
    return stringToTourCharacters.key(character);
}
Character stringToTourCharacter(const QString &str) {
    return stringToTourCharacters.value(str);
}
