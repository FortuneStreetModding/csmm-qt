#include "mapdescriptor.h"
#include <QDebug>
#include "vanilladatabase.h"
#include "fslocale.h"
#include "importexportutils.h"
#include "getordefault.h"

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

static YAML::Node customDataToNode(pybind11::object obj) {
    using namespace pybind11;
    YAML::Node res;
    if (isinstance<dict>(obj)) {
        dict dct(obj);
        for (auto &entry: dct) {
            res[reinterpret_borrow<object>(entry.first).cast<std::string>()] = customDataToNode(reinterpret_borrow<object>(entry.second));
        }
    } else if (isinstance<str>(obj)) {
        res = obj.cast<std::string>();
    } else if (isinstance<sequence>(obj)) {
        sequence seq(obj);
        for (auto elem: seq) {
            res.push_back(customDataToNode(elem));
        }
    } else if (isinstance<none>(obj)) {
        // do nothing
    } else if (isinstance<bool_>(obj)) {
        res = obj.cast<bool>();
    } else if (isinstance<int_>(obj) || isinstance<float_>(obj)) {
        res = obj.cast<double>();
    } else {
        throw std::runtime_error("object " + str(obj).cast<std::string>() + " is invalid for yaml!");
    }
    return res;
}

static pybind11::object nodeToCustomData(const YAML::Node &node) {
    switch (node.Type()) {
    case YAML::NodeType::Undefined:
    case YAML::NodeType::Null:
        return pybind11::none();
    case YAML::NodeType::Scalar:
        // TODO try to detect ints/floats?
        return pybind11::str(node.Scalar());
    case YAML::NodeType::Sequence:
    {
        pybind11::list lst;
        for (auto &ent: node) {
            lst.append(nodeToCustomData(ent));
        }
        return lst;
    }
    case YAML::NodeType::Map:
    {
        pybind11::dict dct;
        for (auto &ent: node) {
            dct[pybind11::str(ent.first.Scalar())] = nodeToCustomData(ent.second);
        }
        return dct;
    }
    default:
        throw std::runtime_error("malformed yaml node for extra data");
    }
}

#define SHOP_NAMES_TO_YAML(shopNameType) \
    if (!shopNameType.empty()) { \
        out << YAML::Key << #shopNameType << YAML::Value << YAML::BeginMap; \
        for (auto &fslocale: FS_LOCALES) { \
            auto namesForLocale = getOrDefault(shopNameType, fslocale, std::vector<QString>()); \
            if (fslocale == "uk" || namesForLocale.empty()) \
                continue; \
            out << YAML::Key << localeToYamlKey(fslocale).toStdString() << YAML::Value << YAML::BeginMap; \
            for (int i=0; i<namesForLocale.size(); ++i) { \
                out << YAML::Key << (i+1) << YAML::Value << namesForLocale[i].toStdString(); \
            } \
            out << YAML::EndMap; \
        } \
        out << YAML::EndMap; \
    }

QString MapDescriptor::toYaml() const {
    YAML::Emitter out;

    out << YAML::BeginDoc;

    out << YAML::BeginMap;

    out << YAML::Key << "name" << YAML::Value << YAML::BeginMap;
    for (auto &fslocale: FS_LOCALES) {
        auto name = getOrDefault(names, fslocale, "");
        if(fslocale == "uk" || name.isEmpty())
            continue;
        out << YAML::Key << localeToYamlKey(fslocale).toStdString() << YAML::Value << name.toStdString();
    }
    out << YAML::EndMap;
    out << YAML::Key << "desc" << YAML::Value << YAML::BeginMap;
    for (auto &fslocale: FS_LOCALES) {
        auto desc = getOrDefault(descs, fslocale, "");
        if(fslocale == "uk" || desc.isEmpty())
            continue;
        out << YAML::Key << localeToYamlKey(fslocale).toStdString() << YAML::Value << desc.toStdString();
    }
    out << YAML::EndMap;
    out << YAML::Key << "ruleSet" << YAML::Value << (ruleSet == Easy ? "Easy" : "Standard");
    out << YAML::Key << "theme" << YAML::Value << (theme == Mario ? "Mario" : "DragonQuest");
    out << YAML::Key << "initialCash" << YAML::Value << initialCash;
    out << YAML::Key << "targetAmount" << YAML::Value << targetAmount;
    out << YAML::Key << "baseSalary" << YAML::Value << baseSalary;
    out << YAML::Key << "salaryIncrement" << YAML::Value << salaryIncrement;
    out << YAML::Key << "maxDiceRoll" << YAML::Value << maxDiceRoll;
    out << YAML::Key << "frbFiles" << YAML::BeginSeq;
    for (int i=0; i<frbFiles.size(); ++i) {
        out << frbFiles[i].toStdString();
    }
    out << YAML::EndSeq;

    out << YAML::Key << "background" << YAML::Value << background.toStdString();
    if(!VanillaDatabase::hasDefaultMapIcon(background) || VanillaDatabase::getDefaultMapIcon(background) != mapIcon)
        out << YAML::Key << "mapIcon" << YAML::Value << mapIcon.toStdString();

    if(!VanillaDatabase::hasDefaultBgmId(background) || VanillaDatabase::getDefaultBgmId(background) != bgmId) {
        if(music.empty()) {
            out << YAML::Key << "bgmId" << YAML::Value << Bgm::bgmIdToString(bgmId).toStdString();
        } else if(!music.count(MusicType::map)) {
            out << YAML::Key << "bgmId" << YAML::Value << Bgm::bgmIdToString(bgmId).toStdString();
        }
    }

    if(!music.empty()) {
        out << YAML::Key << "music" << YAML::Value << YAML::BeginMap;
        for (auto &musicTypeEnt: music) {
            auto &musicType = musicTypeEnt.first;
            for (auto &musicEntry: musicTypeEnt.second) {
                out << YAML::Key << Music::musicTypeToString(musicType).toStdString()
                    << YAML::Value << musicEntry.brstmBaseFilename.toStdString();
            }
        }
        out << YAML::EndMap;
    }

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

    if(!mutators.empty()) {
        out << YAML::Key << "mutators" << YAML::Value << YAML::BeginMap;
        for (auto &mutator: mutators) {
            out << YAML::Key << mutatorTypeToString(mutator.second->type).toStdString() << YAML::Value;
            mutator.second->toYaml(out);
        }
        out << YAML::EndMap;
    }

    if(!VanillaDatabase::isDefaultVentureCards(ventureCards, ruleSet)) {
        out << YAML::Key << "ventureCards" << YAML::Value << YAML::BeginSeq;
        for (int i=0; i<128; ++i) {
            out << (int) ventureCards[i] << YAML::Comment(QString("%1 %2").arg(i+1, 3).arg(VanillaDatabase::getVentureCardDesc(i)).toStdString());
        }
        out << YAML::EndSeq;
    }

    if (!districtNames.empty()) {
        out << YAML::Key << "districtNames" << YAML::Value << YAML::BeginMap;
        for (auto &fslocale: FS_LOCALES) {
            auto namesForLocale = getOrDefault(districtNames, fslocale, std::vector<QString>());
            if (fslocale == "uk" || namesForLocale.empty())
                continue;
            out << YAML::Key << localeToYamlKey(fslocale).toStdString() << YAML::Value << YAML::BeginSeq;
            for (auto &distName: namesForLocale) {
                out << distName.toStdString();
            }
            out << YAML::EndSeq;
        }
        out << YAML::EndMap;
    }

    SHOP_NAMES_TO_YAML(shopNames);
    SHOP_NAMES_TO_YAML(capitalShopNames);

    if (pybind11::len(extraData.get()) > 0) {
        out << YAML::Key << "extraData" << YAML::Value << customDataToNode(extraData.get());
    }

    out << YAML::EndMap;

    out << YAML::EndDoc;

    return out.c_str();
}

bool MapDescriptor::operator==(const MapDescriptor &other) const {
    if(mutators.size() != other.mutators.size())
        return false;
    for (auto &mutatorEnt: mutators) {
        auto &mutatorName = mutatorEnt.first;
        if (!other.mutators.count(mutatorName)) {
            return false;
        }
        auto mutator = mutators.at(mutatorName);
        auto otherMutator = other.mutators.at(mutatorName);
        if (*mutator != *otherMutator) {
            return false;
        }
    }

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
            && frbFiles == other.frbFiles
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
            && mapDescriptorFilePath == other.mapDescriptorFilePath
            && districtNames == other.districtNames
            && districtNameIds == other.districtNameIds
            && shopNames == other.shopNames
            && shopNameIds == other.shopNameIds
            && capitalShopNames == other.capitalShopNames
            && capitalShopNameIds == other.capitalShopNameIds
            && extraData.get().equal(other.extraData.get())
            && std::equal(std::begin(authors), std::end(authors), std::begin(other.authors));
}

void MapDescriptor::shopNamesFromYaml(const YAML::Node &yaml, bool isCapital) {
    auto shopNameType = (isCapital ? "capitalShopNames" : "shopNames");
    auto &theShopNames = (isCapital ? capitalShopNames : shopNames);
    if (yaml[shopNameType]) {
        auto snNode = yaml[shopNameType]; // shop name node of the yaml
        auto snNodeCopy = YAML::Clone(snNode);
        for (auto it=snNode.begin(); it!=snNode.end(); ++it) {
            // locale key
            auto key = yamlKeyToLocale(QString::fromStdString(it->first.as<std::string>()));
            for (const auto &kv: it->second) { // iterate over shop name entries
                auto convKey = kv.first.as<int>(); // shop model id
                auto convVal = QString::fromStdString(kv.second.as<std::string>()); // shop name
                theShopNames[key].at(convKey - 1) = convVal;
                auto capitalConvVal = convVal;
                if (!capitalConvVal.isEmpty()) {
                    capitalConvVal[0] = capitalConvVal[0].toTitleCase();
                }
                if (!isCapital) { // add default capital shop names for explicitly specified names
                    capitalShopNames[key].at(convKey - 1) = capitalConvVal;
                }
                if (key == "en") { // fill defaults
                    for (auto &otherLocale: FS_LOCALES) {
                        auto otherKey = localeToYamlKey(otherLocale);
                        if (otherKey == "uk") continue;
                        auto otherKeyStdStr = otherKey.toStdString();
                        if (!snNodeCopy[otherKeyStdStr].IsDefined()
                                || !snNodeCopy[otherKeyStdStr][convKey].IsDefined()) {
                            theShopNames[otherLocale].at(convKey - 1) = convVal;
                            if (!isCapital
                                    && (!yaml["capitalShopNames"].IsDefined()
                                        || !yaml["capitalShopNames"][otherKeyStdStr].IsDefined()
                                        || !yaml["capitalShopNames"][otherKeyStdStr][convKey].IsDefined())) {
                                capitalShopNames[otherLocale].at(convKey - 1) = capitalConvVal;
                            }
                        }
                    }
                }
            }
        }
    }
}

bool MapDescriptor::fromYaml(const YAML::Node &yaml) {
    names.clear();
    descs.clear();
    for (auto it=yaml["name"].begin(); it!=yaml["name"].end(); ++it) {
        names[yamlKeyToLocale(QString::fromStdString(it->first.as<std::string>()))] = QString::fromStdString(it->second.as<std::string>());
    }
    for (auto it=yaml["desc"].begin(); it!=yaml["desc"].end(); ++it) {
        descs[yamlKeyToLocale(QString::fromStdString(it->first.as<std::string>()))] = QString::fromStdString(it->second.as<std::string>());
    }
    names.erase("uk");
    descs.erase("uk");

    ruleSet = yaml["ruleSet"].as<std::string>() == "Easy" ? Easy : Standard;
    theme = yaml["theme"].as<std::string>() == "Mario" ? Mario : DragonQuest;
    initialCash = yaml["initialCash"].as<quint32>();
    tourInitialCash = initialCash;
    targetAmount = yaml["targetAmount"].as<quint32>();
    baseSalary = yaml["baseSalary"].as<quint32>();
    salaryIncrement = yaml["salaryIncrement"].as<quint32>();
    maxDiceRoll = yaml["maxDiceRoll"].as<quint32>();
    frbFiles.clear();
    if (yaml["frbFiles"]) {
        for (auto &frbFile: yaml["frbFiles"]) {
            frbFiles.push_back(QString::fromStdString(frbFile.as<std::string>()));
        }
    } else {
        for (int i=0; i<4; ++i) {
            auto key = QString("frbFile%1").arg(i+1).toStdString();
            if (yaml[key] || i==0) {
                frbFiles.push_back(QString::fromStdString(yaml[key].as<std::string>()));
            } else {
                break;
            }
        }
    }
    switchRotationOrigins.clear();
    for (auto &originPoint: yaml["switchRotationOriginPoints"]) {
        switchRotationOrigins.push_back({originPoint["x"].as<float>(), originPoint["y"].as<float>()});
    }
    if (yaml["looping"]) {
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
    if (yaml["tourMode"]["initialCash"])
        tourInitialCash = yaml["tourMode"]["initialCash"].as<quint32>();
    for (int i=0; i<3; ++i) {
        tourCharacters[i] = stringToTourCharacter(
                    QString::fromStdString(yaml["tourMode"][QString("opponent%1").arg(i+1).toStdString()].as<std::string>())
                );
    }
    tourClearRank = yaml["tourMode"]["clearRank"].as<quint32>();
    background = QString::fromStdString(yaml["background"].as<std::string>());

    if (VanillaDatabase::hasDefaultMapIcon(background))
        mapIcon = VanillaDatabase::getDefaultMapIcon(background);
    if (yaml["mapIcon"]) {
        mapIcon = QString::fromStdString(yaml["mapIcon"].as<std::string>());
        // Check for too long map icon file name, because http://wiki.tockdom.com/wiki/BRLYT_(File_Format) says that the pane name can only have 0x10 bytes.
        if (mapIcon.length() > 13) {
            throw ImportExportUtils::Exception(
                        QString("The filename of the map icon %1 is too long. It must be max 13 characters, but is %2 characters.")
                        .arg(mapIcon).arg(mapIcon.length()));
        }
    }

    if (VanillaDatabase::hasDefaultBgmId(background))
        bgmId = VanillaDatabase::getDefaultBgmId(background);
    if (yaml["bgmId"])
        bgmId = Bgm::stringToBgmId(QString::fromStdString(yaml["bgmId"].as<std::string>()));

    music.clear();
    if (yaml["music"]) {
        for (auto it=yaml["music"].begin(); it!=yaml["music"].end(); ++it) {
            if(it->second.IsNull()) {
                continue;
            }
            auto musicTypeStr = QString::fromStdString(it->first.as<std::string>());
            if(!Music::isMusicType(musicTypeStr)) {
                continue;
            }
            QVector<QString> brstmBaseFilenames;
            if (it->second.IsSequence()) {
                for (auto &val: it->second) {
                    brstmBaseFilenames.append(QString::fromStdString(val.as<std::string>()));
                }
            } else {
                brstmBaseFilenames = { QString::fromStdString(it->second.as<std::string>()) };
            }
            for (auto &brstmBaseFilename: brstmBaseFilenames) {
                if(brstmBaseFilename.length() > 48) {
                    throw ImportExportUtils::Exception(QString("The filename of the brstm file %1 is too long. It must be max 48 characters, but is %2 characters.").arg(brstmBaseFilename).arg(brstmBaseFilename.length()));
                }
                MusicEntry entry;
                entry.brstmBaseFilename = brstmBaseFilename;
                // get the volume out of the file name
                QFileInfo fileInfo(brstmBaseFilename);
                QString suffix = fileInfo.suffix();
                if(!suffix.isEmpty()) {
                    bool ok;
                    auto volume = suffix.toInt(&ok);
                    if(ok && volume >= 0 && volume <= 255) {
                        entry.volume = volume;
                    }
                }
                auto musicType = Music::stringToMusicType(musicTypeStr);
                music[musicType].push_back(entry);
                if(musicType == MusicType::map) {
                    bgmId = BGM_MAP_CIRCUIT;
                }
            }
        }
    }
    mutators.clear();
    if (yaml["mutators"]) {
        for (auto it=yaml["mutators"].begin(); it!=yaml["mutators"].end(); ++it) {
            auto mutatorStr = QString::fromStdString(it->first.as<std::string>());
            mutators.emplace(mutatorStr, Mutator::fromYaml(mutatorStr, it->second));
        }
    }
    if (yaml["ventureCards"]) {
        for (quint32 i=0; i<ventureCards.size(); ++i) {
            ventureCards[i] = yaml["ventureCards"][i].as<int>();
        }
    } else {
        VanillaDatabase::setDefaultVentureCards(ruleSet, ventureCards);
    }
    if (yaml["districtNames"]) {
        auto dnNode = yaml["districtNames"];
        districtNames.clear();
        for (auto it=dnNode.begin(); it!=dnNode.end(); ++it) {
            auto key = yamlKeyToLocale(QString::fromStdString(it->first.as<std::string>()));
            for (const auto &val: it->second) {
                auto convVal = QString::fromStdString(val.as<std::string>());
                districtNames[key].push_back(convVal);
            }
        }
    } else {
        districtNames = VanillaDatabase::getVanillaDistrictNames();
    }

    authors.clear();
    if (yaml["authors"]) {
        auto authorNodes = yaml["authors"];
        for (auto it=authorNodes.begin(); it!=authorNodes.end(); ++it) {
            const YAML::Node& authorNode = *it;
            auto authorName = QString::fromStdString(authorNode["name"].as<std::string>());
            authors.push_back(authorName);
        }
    }

    shopNames = VanillaDatabase::getVanillaShopNames(false);
    capitalShopNames = VanillaDatabase::getVanillaShopNames(true);
    shopNamesFromYaml(yaml, false);
    shopNamesFromYaml(yaml, true);

    if (yaml["extraData"]) extraData = pybind11::dict(nodeToCustomData(yaml["extraData"]));

    return true;
}

MapDescriptor &MapDescriptor::setFromImport(const MapDescriptor &other) {
    auto stuffToRetain = std::make_tuple(mapSet, zone, order, isPracticeBoard, unlockId, nameMsgId, descMsgId);
    *this = other;
    std::tie(mapSet, zone, order, isPracticeBoard, unlockId, nameMsgId, descMsgId) = stuffToRetain;
    return *this;
}

QDebug &operator<<(QDebug &debugStream, const MapDescriptor &obj) {
    // TODO add more info here?
    debugStream << "MapDescriptor(" << getOrDefault(obj.names, "en", "") << ", firstFile=" << obj.frbFiles[0] << ")";
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
const QMap<QString, Character> &stringToTourCharactersMapping() {
    return stringToTourCharacters;
}

void getPracticeBoards(const std::vector<MapDescriptor> &descriptors, short &easyPracticeBoard, short &standardPracticeBoard, QStringList &errorMsgs) {
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

QMap<int, int> getMapSets(const std::vector<MapDescriptor> &descriptors, QStringList &errorMsgs) {
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

QMap<int, int> getMapZones(const std::vector<MapDescriptor> &descriptors, int mapSet, QStringList &errorMsgs) {
    QMap<int, int> result;
    QMap<int, int> numMapsPerZone;
    for (int i=0; i<descriptors.size(); ++i) {
        if (descriptors[i].mapSet == mapSet) {
            if (descriptors[i].zone >= 0 && descriptors[i].zone <= 2) {
                result[i] = descriptors[i].zone;
                ++numMapsPerZone[descriptors[i].zone];
            } else if (descriptors[i].zone != -1) {
                errorMsgs << QString("[board %1] A board can only be in zones -1, 0, 1, or 2").arg(i);
            }
        }
    }
    for (int zoneVal: {0, 1, 2}) {
        if (numMapsPerZone[zoneVal] < 6) {
            errorMsgs << QString("[map set %1] The number of boards in zone %2 must be at least 6").arg(mapSet).arg(zoneVal);
        }
    }

    return result;
}

QMap<int, int> getMapOrderings(const std::vector<MapDescriptor> &descriptors, int mapSet, int zone, QStringList &errorMsgs) {
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
