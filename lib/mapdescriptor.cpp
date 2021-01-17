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

static std::map<std::string, std::string> toStdStrMap(const QMap<QString, QString> &map) {
    std::map<std::string, std::string> result;
    for (auto it=map.begin(); it!=map.end(); ++it) {
        result[it.key().toStdString()] = it.value().toStdString();
    }
    return result;
}

QString MapDescriptor::toMd() const {
    YAML::Emitter out;

    out << YAML::BeginMap;

    out << YAML::Key << "name" << YAML::Value << toStdStrMap(names);
    out << YAML::Key << "desc" << YAML::Value << toStdStrMap(descs);
    out << YAML::Key << "ruleSet" << YAML::Value << (ruleSet == Easy ? "Easy" : "Standard");
    out << YAML::Key << "theme" << YAML::Value << (theme == Mario ? "Mario" : "DragonQuest");
    out << YAML::Key << "initialCash" << YAML::Value << initialCash;
    out << YAML::Key << "targetAmount" << YAML::Value << targetAmount;
    out << YAML::Key << "baseSalary" << YAML::Value << baseSalary;
    out << YAML::Key << "salaryIncrement" << YAML::Value << salaryIncrement;
    out << YAML::Key << "maxDiceRoll" << YAML::Value << maxDiceRoll;
    for (int i=0; i<4; ++i) {
        out << YAML::Key << QString("frbFile%1").arg(i+1).toStdString() << YAML::Value << frbFiles[i].toStdString();
    }

    out << YAML::Key << "switchRotationOriginPoints" << YAML::Value << YAML::BeginSeq;
    for (auto &originPoint: switchRotationOrigins) {
        out << YAML::BeginMap;
        out << YAML::Key << "x" << YAML::Value << originPoint.x;
        out << YAML::Key << "y" << YAML::Value << originPoint.y;
        out << YAML::EndMap;
    }
    out << YAML::EndSeq;

    out << YAML::Key << "looping" << YAML::Value << YAML::BeginMap;
    static const char *loopingModes[] = {"None", "Vertical", "Both"};
    out << YAML::Key << "mode" << YAML::Value << loopingModes[loopingMode];
    out << YAML::Key << "radius" << YAML::Value << loopingModeRadius;
    out << YAML::Key << "horizontalPadding" << YAML::Value << loopingModeHorizontalPadding;
    out << YAML::Key << "verticalSquareCount" << YAML::Value << loopingModeVerticalSquareCount;
    out << YAML::EndMap;

    out << YAML::Key << "tourMode" << YAML::Value << YAML::BeginMap;
    out << YAML::Key << "bankruptcyLimit" << YAML::Value << tourBankruptcyLimit;
    out << YAML::Key << "initialCash" << YAML::Value << tourInitialCash;
    for (int i=0; i<3; ++i) {
        out << YAML::Key << QString("opponent%1").arg(i+1).toStdString() << YAML::Value << tourCharacterToString(tourCharacters[i]).toStdString();
    }
    out << YAML::Key << "clearRank" << YAML::Value << tourClearRank;
    out << YAML::EndMap;

    out << YAML::Key << "bgmId" << YAML::Value << bgmId;
    out << YAML::Key << "background" << YAML::Value << background.toStdString();
    out << YAML::Key << "mapIcon" << YAML::Value << mapIcon.toStdString();
    out << YAML::Key << "ventureCards" << YAML::Value << std::vector<int>(std::begin(ventureCards), std::end(ventureCards));

    out << YAML::EndMap;

    return out.c_str();
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

bool MapDescriptor::fromMd(const YAML::Node &yaml) {
    try {
        for (auto it=yaml["name"].begin(); it!=yaml["name"].end(); ++it) {
            names[QString::fromStdString(it->first.as<std::string>())] = QString::fromStdString(it->second.as<std::string>());
        }
        for (auto it=yaml["desc"].begin(); it!=yaml["desc"].end(); ++it) {
            descs[QString::fromStdString(it->first.as<std::string>())] = QString::fromStdString(it->second.as<std::string>());
        }
        ruleSet = yaml["ruleSet"].as<std::string>() == "Easy" ? Easy : Standard;
        theme = yaml["theme"].as<std::string>() == "Mario" ? Mario : DragonQuest;
        initialCash = yaml["initialCash"].as<quint32>();
        targetAmount = yaml["targetAmount"].as<quint32>();
        baseSalary = yaml["baseSalary"].as<quint32>();
        salaryIncrement = yaml["salaryIncrement"].as<quint32>();
        maxDiceRoll = yaml["maxDiceRoll"].as<quint32>();
        for (int i=0; i<4; ++i) {
            frbFiles[i] = QString::fromStdString(yaml[QString("frbFile%1").arg(i+1).toStdString()].as<std::string>());
        }
        switchRotationOrigins.clear();
        for (auto &originPoint: yaml["switchRotationOriginPoints"]) {
            switchRotationOrigins.append({originPoint["x"].as<float>(), originPoint["y"].as<float>(),});
        }
        auto loopingModeStr = yaml["looping"]["mode"].as<std::string>();
        if (loopingModeStr == "Vertical") {
            loopingMode = Vertical;
        } else if (loopingModeStr == "Both") {
            loopingMode = Both;
        } else {
            loopingMode = None;
        }
        loopingModeRadius = yaml["looping"]["radius"].as<float>();
        loopingModeHorizontalPadding = yaml["looping"]["horizontalPadding"].as<float>();
        loopingModeVerticalSquareCount = yaml["looping"]["verticalSquareCount"].as<float>();
        tourBankruptcyLimit = yaml["tourMode"]["bankruptcyLimit"].as<quint32>();
        tourInitialCash = yaml["tourMode"]["initialCash"].as<quint32>();
        for (int i=0; i<3; ++i) {
            tourCharacters[i] = stringToTourCharacter(
                        QString::fromStdString(yaml["tourMode"][QString("opponent%1").arg(i+1).toStdString()].as<std::string>())
                    );
        }
        tourClearRank = yaml["tourMode"]["clearRank"].as<quint32>();
        bgmId = yaml["bgmId"].as<quint32>();
        background = QString::fromStdString(yaml["background"].as<std::string>());
        mapIcon = QString::fromStdString(yaml["mapIcon"].as<std::string>());
        for (quint32 i=0; i<sizeof(ventureCards); ++i) {
            ventureCards[i] = yaml["ventureCards"][i].as<int>();
        }
        return true;
    } catch (const YAML::Exception &exception) {
        return false;
    }
}

MapDescriptor &MapDescriptor::setFromImport(const MapDescriptor &other) {
    auto stuffToRetain = std::make_tuple(mapSet, zone, order, isPracticeBoard, unlockId, nameMsgId, descMsgId);
    *this = other;
    std::tie(mapSet, zone, order, isPracticeBoard, unlockId, nameMsgId, descMsgId) = stuffToRetain;
    return *this;
}

QDebug &operator<<(QDebug &debugStream, const MapDescriptor &obj) {
    // TODO add more info here?
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

void getPracticeBoards(const QVector<MapDescriptor> &descriptors, short &easyPracticeBoard, short &standardPracticeBoard, QStringList &errorMsgs) {
    // TODO improve error stuff
    easyPracticeBoard = -1;
    standardPracticeBoard = -1;
    for (int i=0; i<descriptors.size(); ++i) {
        if (descriptors[i].isPracticeBoard) {
            if (descriptors[i].mapSet == 0) {
                if (easyPracticeBoard == -1) {
                    easyPracticeBoard = i;
                } else {
                    errorMsgs << "There can be only 1 practice board for the easy map set";
                }
            } else if (descriptors[i].mapSet == 1) {
                if (standardPracticeBoard == -1) {
                    standardPracticeBoard = i;
                } else {
                    errorMsgs << "There can be only 1 practice board for the standard map set";
                }
            } else {
                errorMsgs << "A practice board can only be set for map sets 0 or 1";
            }
        }
    }
    if (easyPracticeBoard == -1) {
        errorMsgs << "Easy map set needs at least 1 practice board";
    }
    if (standardPracticeBoard == -1) {
        errorMsgs << "Standard map set needs at least 1 practice board";
    }
}
