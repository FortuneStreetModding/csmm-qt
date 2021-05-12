#include "mapdescriptor.h"
#include <QDebug>
#include "vanilladatabase.h"
#include "fslocale.h"

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

QString MapDescriptor::toYaml() const {
    YAML::Emitter out;

    out << YAML::BeginDoc;

    out << YAML::BeginMap;

    out << YAML::Key << "name" << YAML::Value << YAML::BeginMap;
    for (auto &fslocale: FS_LOCALES) {
        if(fslocale == "uk" || names[fslocale].isEmpty())
            continue;
        out << YAML::Key << localeToYamlKey(fslocale).toStdString() << YAML::Value << names[fslocale].toStdString();
    }
    out << YAML::EndMap;
    out << YAML::Key << "desc" << YAML::Value << YAML::BeginMap;
    for (auto &fslocale: FS_LOCALES) {
        if(fslocale == "uk" || names[fslocale].isEmpty())
            continue;
        out << YAML::Key << localeToYamlKey(fslocale).toStdString() << YAML::Value << descs[fslocale].toStdString();
    }
    out << YAML::EndMap;
    out << YAML::Key << "ruleSet" << YAML::Value << (ruleSet == Easy ? "Easy" : "Standard");
    out << YAML::Key << "theme" << YAML::Value << (theme == Mario ? "Mario" : "DragonQuest");
    out << YAML::Key << "initialCash" << YAML::Value << initialCash;
    out << YAML::Key << "targetAmount" << YAML::Value << targetAmount;
    out << YAML::Key << "baseSalary" << YAML::Value << baseSalary;
    out << YAML::Key << "salaryIncrement" << YAML::Value << salaryIncrement;
    out << YAML::Key << "maxDiceRoll" << YAML::Value << maxDiceRoll;
    for (int i=0; i<4; ++i) {
        if(!frbFiles[i].isEmpty())
            out << YAML::Key << QString("frbFile%1").arg(i+1).toStdString() << YAML::Value << frbFiles[i].toStdString();
    }

    out << YAML::Key << "background" << YAML::Value << background.toStdString();
    if(!VanillaDatabase::hasDefaultMapIcon(background) || VanillaDatabase::getDefaultMapIcon(background) != mapIcon)
        out << YAML::Key << "mapIcon" << YAML::Value << mapIcon.toStdString();
    if(!VanillaDatabase::hasDefaultBgmId(background) || VanillaDatabase::getDefaultBgmId(background) != bgmId)
        out << YAML::Key << "bgmId" << YAML::Value << bgmIdToString(bgmId).toStdString();

    out << YAML::Key << "music" << YAML::Value << YAML::BeginMap;
    for (auto &musicType: music.keys()) {
        auto musicEntry = music[musicType];
        out << YAML::Key << musicTypeToString(musicType).toStdString() << YAML::Value << musicEntry.brstmBaseFilename.toStdString();
    }
    out << YAML::EndMap;

    if(!switchRotationOrigins.empty()) {
        out << YAML::Key << "switchRotationOriginPoints" << YAML::Value << YAML::BeginSeq;
        for (auto &originPoint: switchRotationOrigins) {
            out << YAML::BeginMap;
            out << YAML::Key << "x" << YAML::Value << originPoint.x;
            out << YAML::Key << "y" << YAML::Value << originPoint.y;
            out << YAML::EndMap;
        }
        out << YAML::EndSeq;
    }

    if (loopingMode != None) {
        out << YAML::Key << "looping" << YAML::Value << YAML::BeginMap;
        out << YAML::Key << "mode" << YAML::Value;
        if (loopingMode == Vertical) {
            out << "Vertical";
        } else {
            out << "Both";
        }
        out << YAML::Key << "radius" << YAML::Value << loopingModeRadius;
        out << YAML::Key << "horizontalPadding" << YAML::Value << loopingModeHorizontalPadding;
        out << YAML::Key << "verticalSquareCount" << YAML::Value << loopingModeVerticalSquareCount;
        out << YAML::EndMap;
    }

    out << YAML::Key << "tourMode" << YAML::Value << YAML::BeginMap;
    out << YAML::Key << "bankruptcyLimit" << YAML::Value << tourBankruptcyLimit;
    if(tourInitialCash != initialCash)
        out << YAML::Key << "initialCash" << YAML::Value << tourInitialCash;
    for (int i=0; i<3; ++i) {
        out << YAML::Key << QString("opponent%1").arg(i+1).toStdString() << YAML::Value << tourCharacterToString(tourCharacters[i]).toStdString();
    }
    out << YAML::Key << "clearRank" << YAML::Value << tourClearRank;
    out << YAML::EndMap;

    if(!VanillaDatabase::isDefaultVentureCards(ventureCards, ruleSet)) {
        out << YAML::Key << "ventureCards" << YAML::Value << YAML::BeginSeq;
        for (int i=0; i<128; ++i) {
            out << YAML::Value << (int) ventureCards[i] << YAML::Comment(QString("%1 %2").arg(i+1, 3).arg(VanillaDatabase::getVentureCardDesc(i)).toStdString());
        }
        out << YAML::EndSeq;
    }

    out << YAML::EndMap;

    out << YAML::EndDoc;

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
            && music == other.music
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

bool MapDescriptor::fromYaml(const YAML::Node &yaml) {
    try {
        for (auto it=yaml["name"].begin(); it!=yaml["name"].end(); ++it) {
            names[yamlKeyToLocale(QString::fromStdString(it->first.as<std::string>()))] = QString::fromStdString(it->second.as<std::string>());
        }
        for (auto it=yaml["desc"].begin(); it!=yaml["desc"].end(); ++it) {
            descs[yamlKeyToLocale(QString::fromStdString(it->first.as<std::string>()))] = QString::fromStdString(it->second.as<std::string>());
        }
        names.remove("uk");
        descs.remove("uk");

        ruleSet = yaml["ruleSet"].as<std::string>() == "Easy" ? Easy : Standard;
        theme = yaml["theme"].as<std::string>() == "Mario" ? Mario : DragonQuest;
        initialCash = yaml["initialCash"].as<quint32>();
        tourInitialCash = initialCash;
        targetAmount = yaml["targetAmount"].as<quint32>();
        baseSalary = yaml["baseSalary"].as<quint32>();
        salaryIncrement = yaml["salaryIncrement"].as<quint32>();
        maxDiceRoll = yaml["maxDiceRoll"].as<quint32>();
        for (int i=0; i<4; ++i) {
            auto key = QString("frbFile%1").arg(i+1).toStdString();
            if(yaml[key] || i==0) {
                frbFiles[i] = QString::fromStdString(yaml[key].as<std::string>());
            } else {
                frbFiles[i] = "";
            }
        }
        switchRotationOrigins.clear();
        for (auto &originPoint: yaml["switchRotationOriginPoints"]) {
            switchRotationOrigins.append({originPoint["x"].as<float>(), originPoint["y"].as<float>(),});
        }
        if(yaml["looping"]) {
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
        } else {
            loopingMode = None;
        }
        tourBankruptcyLimit = yaml["tourMode"]["bankruptcyLimit"].as<quint32>();
        if(yaml["tourMode"]["initialCash"])
            tourInitialCash = yaml["tourMode"]["initialCash"].as<quint32>();
        for (int i=0; i<3; ++i) {
            tourCharacters[i] = stringToTourCharacter(
                        QString::fromStdString(yaml["tourMode"][QString("opponent%1").arg(i+1).toStdString()].as<std::string>())
                    );
        }
        tourClearRank = yaml["tourMode"]["clearRank"].as<quint32>();
        background = QString::fromStdString(yaml["background"].as<std::string>());
        if(VanillaDatabase::hasDefaultMapIcon(background)) {
            mapIcon = VanillaDatabase::getDefaultMapIcon(background);
        }
        if(yaml["mapIcon"])
            mapIcon = QString::fromStdString(yaml["mapIcon"].as<std::string>());
        if(VanillaDatabase::hasDefaultBgmId(background)) {
            bgmId = VanillaDatabase::getDefaultBgmId(background);
        }
        for (auto it=yaml["music"].begin(); it!=yaml["music"].end(); ++it) {
            QString brstmBaseFilename = QString::fromStdString(it->second.as<std::string>());
            MusicEntry entry;
            entry.brstmBaseFilename = brstmBaseFilename;
            // get the volume out of the file name
            QFileInfo fileInfo(brstmBaseFilename);
            QString suffix = fileInfo.suffix();
            if(!suffix.isEmpty()) {
                entry.volume = suffix.toInt();
            }
            music[stringToMusicType(QString::fromStdString(it->first.as<std::string>()))] = entry;
        }
        bgmId = stringToBgmId(QString::fromStdString(yaml["bgmId"].as<std::string>()));
        if(yaml["ventureCards"]) {
            for (quint32 i=0; i<sizeof(ventureCards); ++i) {
                ventureCards[i] = yaml["ventureCards"][i].as<int>();
            }
        } else {
            VanillaDatabase::setDefaultVentureCards(ruleSet, ventureCards);
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

static const QMap<QString, BgmId> stringToBgmIds = {
    {"BGM_MAP_CIRCUIT", BGM_MAP_CIRCUIT},
    {"BGM_MAP_PEACH", BGM_MAP_PEACH},
    {"BGM_MAP_KOOPA", BGM_MAP_KOOPA},
    {"BGM_MAP_GHOSTSHIP", BGM_MAP_GHOSTSHIP},
    {"BGM_MAP_MAJINZOU", BGM_MAP_MAJINZOU},
    {"BGM_MAP_SINOKAZAN", BGM_MAP_SINOKAZAN},
    {"BGM_MAP_SLABACCA", BGM_MAP_SLABACCA},
    {"BGM_MAP_KANDATA", BGM_MAP_KANDATA},
    {"BGM_MAP_KANDATA_old", BGM_MAP_KANDATA_old},
    {"BGM_MAP_ALEFGARD", BGM_MAP_ALEFGARD},
    {"BGM_MAP_ALEFGARD_old", BGM_MAP_ALEFGARD_old},
    {"BGM_MAP_YOSHI", BGM_MAP_YOSHI},
    {"BGM_MAP_STADIUM", BGM_MAP_STADIUM},
    {"BGM_MAP_DOLPIC", BGM_MAP_DOLPIC},
    {"BGM_MAP_SMB", BGM_MAP_SMB},
    {"BGM_MAP_STARSHIP", BGM_MAP_STARSHIP},
    {"BGM_MAP_EGG", BGM_MAP_EGG},
    {"BGM_MAP_TRODAIN", BGM_MAP_TRODAIN},
    {"BGM_MAP_TRODAIN_old", BGM_MAP_TRODAIN_old},
    {"BGM_MAP_DHAMA", BGM_MAP_DHAMA},
    {"BGM_MAP_DHAMA_old", BGM_MAP_DHAMA_old},
    {"BGM_MAP_ANGEL", BGM_MAP_ANGEL},
    {"BGM_MENU", BGM_MENU},
    {"BGM_GOALPROP", BGM_GOALPROP},
    {"BGM_WINNER", BGM_WINNER},
    {"BGM_CHANCECARD", BGM_CHANCECARD},
    {"BGM_STOCK", BGM_STOCK},
    {"BGM_AUCTION", BGM_AUCTION},
    {"BGM_CASINO_SLOT", BGM_CASINO_SLOT},
    {"BGM_CASINO_BLOCK", BGM_CASINO_BLOCK},
    {"BGM_CASINO_RACE", BGM_CASINO_RACE},
    {"BGM_TITLE", BGM_TITLE},
    {"BGM_SAVELOAD", BGM_SAVELOAD},
    {"BGM_SAVELOAD_old", BGM_SAVELOAD_old},
    {"BGM_WIFI", BGM_WIFI},
    {"BGM_ENDING", BGM_ENDING}
};

QString bgmIdToString(BgmId bgmId) {
    return stringToBgmIds.key(bgmId);
}
BgmId stringToBgmId(const QString &str) {
    return stringToBgmIds.value(str);
}

static const QMap<QString, MusicType> stringToMusicTypes = {
    {"map"               , map},
    {"stock"             , stock},
    {"ventureCards"      , ventureCards},
    {"auction"           , auction},
    {"targetMet"         , targetMet},
    {"win"               , win},
    {"guestAppear"       , guestAppear},
    {"guestLeave"        , guestLeave},
    {"badVentureCard"    , badVentureCard},
    {"takeAbreak"        , takeAbreak},
    {"promotion"         , promotion},
    {"forcedBuyout"      , forcedBuyout},
    {"domination"        , domination},
    {"bankruptcy"        , bankruptcy},
    // arcade
    {"roundTheBlocks"    , roundTheBlocks},
    {"roundTheBlocksWin" , roundTheBlocksWin},
    {"roundTheBlocks777" , roundTheBlocks777},
    {"memoryBlock"       , memoryBlock},
    {"dartOfGold"        , dartOfGold},
    {"slurpodromeSelect" , slurpodromeSelect},
    {"slurpodromeStart"  , slurpodromeStart},
    {"slurpodromeRace"   , slurpodromeRace}
};

QString musicTypeToString(MusicType musicType) {
    return stringToMusicTypes.key(musicType);
}
MusicType stringToMusicType(const QString &str) {
    return stringToMusicTypes.value(str);
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
                    errorMsgs << QString("[board %1] There can be only 1 practice board for the easy map set").arg(i);
                }
            } else if (descriptors[i].mapSet == 1) {
                if (standardPracticeBoard == -1) {
                    standardPracticeBoard = i;
                } else {
                    errorMsgs << QString("[board %1] There can be only 1 practice board for the standard map set").arg(i);
                }
            } else if (descriptors[i].mapSet == -1) {
                errorMsgs << QString("[board %1] A practice board can only be set for map sets 0 or 1").arg(i);
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

QMap<int, int> getMapSets(const QVector<MapDescriptor> &descriptors, QStringList &errorMsgs) {
    QMap<int, int> result;
    for (int i=0; i<descriptors.size(); ++i) {
        if (descriptors[i].mapSet == 0 || descriptors[i].mapSet == 1) {
            result[i] = descriptors[i].mapSet;
        } else if (descriptors[i].mapSet != -1) {
            errorMsgs << QString("[board %1] A board can only be in map sets -1, 0, or 1").arg(i);
        }
    }

    // note: the iterators here are by value, not by key
    for (int mapSetVal: {0, 1}) {
        if (std::find(result.begin(), result.end(), mapSetVal) == result.end()) {
            errorMsgs << QString("There much be at least one map in map set %1").arg(mapSetVal);
        }
    }

    return result;
}

QMap<int, int> getMapZones(const QVector<MapDescriptor> &descriptors, int mapSet, QStringList &errorMsgs) {
    QMap<int, int> result;
    QMap<int, int> numMapsPerZone, numMapsPerZoneOtherSet;
    for (int i=0; i<descriptors.size(); ++i) {
        if (descriptors[i].mapSet == mapSet) {
            if (descriptors[i].zone >= 0 && descriptors[i].zone <= 2) {
                result[i] = descriptors[i].zone;
                ++numMapsPerZone[descriptors[i].zone];
            } else if (descriptors[i].zone != -1) {
                errorMsgs << QString("[board %1] A board can only be in zones -1, 0, 1, or 2").arg(i);
            }
        } else {
            ++numMapsPerZoneOtherSet[descriptors[i].zone];
        }
    }
    for (int zoneVal: {0, 1, 2}) {
        if (numMapsPerZone[zoneVal] != numMapsPerZoneOtherSet[zoneVal]) {
            errorMsgs << QString("The number of boards in zone %1 must be the same in map sets 0 and 1").arg(zoneVal);
        }
        if (numMapsPerZone[zoneVal] < 6) {
            errorMsgs << QString("[map set %1] The number of boards in zone %2 must be at least 6").arg(mapSet).arg(zoneVal);
        }
        if (numMapsPerZone[zoneVal] > 16) {
            errorMsgs << QString("[map set %1] The number of boards in zone %2 must be at most 16").arg(mapSet).arg(zoneVal);
        }
    }
    return result;
}

QMap<int, int> getMapOrderings(const QVector<MapDescriptor> &descriptors, int mapSet, int zone, QStringList &errorMsgs) {
    QMap<int, int> result;

    for (int i=0; i<descriptors.size(); ++i) {
        if (descriptors[i].mapSet == mapSet && descriptors[i].zone == zone) {
            if (descriptors[i].order >= 0) {
                result[i] = descriptors[i].order;
            } else {
                errorMsgs << QString("[board %1] All orders in a zone must be nonnegative").arg(i);
            }
        }
    }

    auto orders = result.values();
    std::sort(orders.begin(), orders.end());
    auto uniqueEnd = std::unique(orders.begin(), orders.end());
    if (uniqueEnd != orders.end()) {
        errorMsgs << QString("[mapset %1, zone %2] The order value within a zone must be unique").arg(mapSet).arg(zone);
    }
    if (orders.front() != 0) {
        errorMsgs << QString("[mapset %1, zone %2] The lowest order within a zone must be 0").arg(mapSet).arg(zone);
    }
    if (orders.back() != uniqueEnd - orders.begin() - 1) {
        errorMsgs << QString("[mapset %1, zone %2] There must be no gaps in the ordering").arg(mapSet).arg(zone);
    }
    return result;
}
