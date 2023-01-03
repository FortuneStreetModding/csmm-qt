#include "configuration.h"

#include <QSaveFile>
#include <QTextStream>
#include <yaml-cpp/yaml.h>
#include <fstream>
#include <filesystem>
#include <QRegularExpression>
#include <QNetworkReply>
#include "importexportutils.h"
#include "lib/asyncfuture.h"
#include "lib/await.h"
#include "lib/csmmnetworkmanager.h"

namespace Configuration {

// to string methods
static QString toCsv(const QString &mapId, const QString & mapSet, const QString & mapZone, const QString & mapOrder, const QString & practiceBoard, const QString & name, const QString & mapDescriptorRelativePath) {
    QString table("%1, %2, %3, %4, %5, %6, %7");
    return table
            .arg(mapId, 3, QChar(' '))
            .arg(mapSet, 5, QChar(' '))
            .arg(mapZone, 5, QChar(' '))
            .arg(mapOrder, 5, QChar(' '))
            .arg(practiceBoard, 5, QChar(' '))
            .arg(name.leftJustified(24, ' ', true), mapDescriptorRelativePath);
}

static QString toCsv(int mapId, int mapSet, int mapZone, int mapOrder, int practiceBoard, const QString &name, const QString &mapDescriptorRelativePath) {
    return toCsv(QString::number(mapId),QString::number(mapSet),QString::number(mapZone),QString::number(mapOrder),QString::number(practiceBoard),name,mapDescriptorRelativePath);
}

QString ConfigEntry::toCsv() {
    return Configuration::toCsv(mapId, mapSet, mapZone, mapOrder, practiceBoard, name, mapDescriptorRelativePath);
}

QString ConfigFile::toCsv() {
    QString result("");
    result += Configuration::toCsv("id", "mapS.", "zone", "order", "prac.", "name", "yaml") + "\n";
    for(auto& entry : entries) {
        result += Configuration::toCsv(entry.mapId, entry.mapSet, entry.mapZone, entry.mapOrder, entry.practiceBoard, entry.name, entry.mapDescriptorRelativePath) + "\n";
    }
    return result;
}

QString ConfigFile::toYaml()
{
    YAML::Emitter emitter;
    emitter << YAML::BeginDoc;
    emitter << YAML::BeginMap;
    std::set<int> mapSets, zones; // don't use QSet as that is unsorted
    for (auto &entry: entries) {
        mapSets.insert(entry.mapSet);
        zones.insert(entry.mapZone);
    }
    emitter << YAML::Comment("optional: add the backgrounds key to point to a list of backgrounds in cswt yaml format");
    for (int mapSet: mapSets) {
        emitter << YAML::Key << mapSet << YAML::Comment("Map Set");
        emitter << YAML::Value;
        emitter << YAML::BeginMap;
        for (int zone: zones) {
            emitter << YAML::Key << zone << YAML::Comment("Map Zone");
            emitter << YAML::Value;
            emitter << YAML::BeginMap;
            for (auto &entry: entries) {
                if (entry.mapSet == mapSet && entry.mapZone == zone) {
                    emitter << YAML::Key << (entry.mapDescriptorRelativePath.isEmpty()
                                             ? QString("!default%1_%2").arg(entry.mapId).arg(entry.name) : entry.mapDescriptorRelativePath).toStdString()
                            << YAML::Comment("Path relative to this file to the descriptor yaml, or !default<mapid>_<internalname> if left as default");
                    emitter << YAML::Value;
                    emitter << YAML::BeginMap;
                    emitter << YAML::Key << "urls" << YAML::Value << std::vector<std::string>() << YAML::Comment("Can be omitted, list of urls to download board from");
                    emitter << YAML::Key << "mapId" << YAML::Value << entry.mapId << YAML::Comment("Omit to deduce map id from order in file");
                    emitter << YAML::Key << "mapOrder" << YAML::Value << entry.mapOrder << YAML::Comment("Omit to deduce map order in mapSet/zone from order in file");
                    emitter << YAML::Key << "practiceBoard" << YAML::Value << (bool)entry.practiceBoard << YAML::Comment("Defaults to false");
                    emitter << YAML::EndMap;
                }
            }
            emitter << YAML::EndMap;
        }
        emitter << YAML::EndMap;
    }
    emitter << YAML::EndMap;
    emitter << YAML::EndDoc;
    return emitter.c_str();
}

static const QRegularExpression DEFAULT_REGEXP("^!default\\d+_(.*)", QRegularExpression::UseUnicodePropertiesOption);

static ConfigFile parseLegacy(const QString &fileName) {
    ConfigFile configFile;
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return configFile;
    }
    QTextStream in(&file);
    QVector<ConfigEntry> configEntries;
    while (!in.atEnd()) {
        QString line = in.readLine();
        // if the first line starts with id, skip this line (it is the header)
        if(line.trimmed().toLower().startsWith("id")) {
            continue;
        }
        QStringList lineSplit = line.split(",");
        if(lineSplit.count() != 7) {
            configFile.error = true;
            return configFile;
        }
        ConfigEntry entry;
        entry.mapId = lineSplit.at(0).trimmed().toInt();
        entry.mapSet = lineSplit.at(1).trimmed().toInt();
        entry.mapZone = lineSplit.at(2).trimmed().toInt();
        entry.mapOrder = lineSplit.at(3).trimmed().toInt();
        entry.practiceBoard = lineSplit.at(4).trimmed().toInt();
        entry.name = lineSplit.at(5).trimmed();
        entry.mapDescriptorRelativePath = lineSplit.at(6).trimmed();
        configEntries.append(entry);
    }
    configFile.entries = std::move(configEntries);
    return configFile;
}

static ConfigFile parseYaml(const QString &fileName) {
    ConfigFile configFile;
    QVector<ConfigEntry> configEntries;
    std::ifstream stream(std::filesystem::path(fileName.toStdU16String()));
    auto node = YAML::Load(stream);

    int defaultMapId = 0;
    for (auto it=node.begin(); it!=node.end(); ++it) { // iterate over map sets
        auto mapSetStr = it->first.as<std::string>();
        int mapSet;
        try {
            mapSet = std::stoi(mapSetStr);
        } catch (const std::invalid_argument &) {
            continue; // non-integer key so ignore
        }
        for (auto jt=it->second.begin(); jt!=it->second.end(); ++jt) { // iterate over zones
            int zone = jt->first.as<int>();
            int defaultMapOrder = 0;
            for (auto kt = jt->second.begin(); kt != jt->second.end(); ++kt, ++defaultMapOrder, ++defaultMapId) { // iterate over maps
                ConfigEntry entryToAdd;
                auto rawName = QString::fromStdString(kt->first.as<std::string>());
                auto match = DEFAULT_REGEXP.match(rawName);
                if (match.hasMatch()) {
                    entryToAdd.name = match.captured(1);
                    entryToAdd.mapDescriptorRelativePath = "";
                } else {
                    entryToAdd.name = QFileInfo(rawName).baseName();
                    entryToAdd.mapDescriptorRelativePath = QFileInfo(fileName).dir().relativeFilePath(rawName);
                    if (kt->second["urls"].IsDefined()) {
                        auto urls = kt->second["urls"].as<std::vector<std::string>>();
                        for (auto &url: urls) {
                            entryToAdd.mapDescriptorUrls.push_back(QString::fromStdString(url));
                        }
                    }
                }
                entryToAdd.mapId = kt->second["mapId"].IsDefined() ? kt->second["mapId"].as<int>() : defaultMapId;
                entryToAdd.mapOrder = kt->second["mapOrder"].IsDefined() ? kt->second["mapOrder"].as<int>() : defaultMapOrder;
                entryToAdd.mapSet = mapSet;
                entryToAdd.mapZone = zone;
                entryToAdd.practiceBoard = kt->second["practiceBoard"].IsDefined() && kt->second["practiceBoard"].as<bool>();
                configEntries.push_back(entryToAdd);
            }
        }
    }
    // load background urls if needed
    if (node["backgrounds"].IsDefined()) {
        auto backgroundPath = QFileInfo(fileName).dir().filePath(
                    QString::fromStdString(node["backgrounds"].as<std::string>())
                );
        if (!QFileInfo::exists(backgroundPath)) {
            throw std::runtime_error("file " + backgroundPath.toStdString() + " does not exist");
        }
        std::ifstream bgStream(std::filesystem::path(backgroundPath.toStdU16String()));
        auto bgNode = YAML::Load(bgStream);
        for (auto it=bgNode.begin(); it!=bgNode.end(); ++it) {
            auto bgEntry = *it;
            auto downloadElem = bgEntry["download"];
            if (downloadElem.IsDefined()) {
                QVector<QString> downloadsEntry;
                for (auto jt=downloadElem.begin(); jt!=downloadElem.end(); ++jt) {
                    downloadsEntry.push_back(
                                QString::fromStdString(jt->as<std::string>())
                                );
                }
                configFile.backgroundPaths[
                        QString::fromStdString(bgEntry["background"].as<std::string>())
                        ]
                        = std::move(downloadsEntry);
            }
        }
    }
    configFile.entries = std::move(configEntries);
    return configFile;
}

// open and parse the config file
static ConfigFile parse(const QString &fileName) {
    if (QFileInfo(fileName).suffix() == "csv") {
        return parseLegacy(fileName);
    } else {
        return parseYaml(fileName);
    }
}

static ConfigFile parse(const std::vector<MapDescriptor> &descriptors, const QString &fileName) {
    ConfigFile configFile;
    QVector<ConfigEntry> configEntries;
    QFileInfo fileInfo(fileName);
    QDir dir(fileInfo.dir());
    for(int i=0;i<descriptors.size();i++) {
        auto md = descriptors.at(i);
        ConfigEntry entry;
        entry.mapId = i;
        entry.mapSet = md.mapSet;
        entry.mapZone = md.zone;
        entry.mapOrder = md.order;
        entry.practiceBoard = md.isPracticeBoard;
        entry.name = md.internalName;
        entry.mapDescriptorRelativePath = dir.relativeFilePath(md.mapDescriptorFilePath);
        configEntries.append(entry);
    }
    configFile.entries = configEntries;
    return configFile;
}

int ConfigFile::maxId() {
    int maxId = -1;
    for(auto& entry : entries) {
        if(maxId < entry.mapId) {
            maxId = entry.mapId;
        }
    }
    return maxId;
}

int ConfigFile::maxMapSet() {
    int maxMapSet = -1;
    for(auto& entry : entries) {
        if(maxMapSet < entry.mapSet) {
            maxMapSet = entry.mapSet;
        }
    }
    return maxMapSet;
}

int ConfigFile::maxZone(int mapSet) {
    int maxZone = -1;
    for(auto& entry : entries) {
        if(maxZone < entry.mapZone && entry.mapSet == mapSet) {
            maxZone = entry.mapZone;
        }
    }
    return maxZone;
}

int ConfigFile::maxOrder(int mapSet, int zone) {
    int maxOrder = -1;
    for(auto& entry : entries) {
        if(maxOrder < entry.mapOrder && entry.mapSet == mapSet && entry.mapZone == zone) {
            maxOrder = entry.mapOrder;
        }
    }
    return maxOrder;
}

void save(const QString &fileName, const std::vector<MapDescriptor> &descriptors)
{
    QSaveFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        throw std::runtime_error("Could not open file for saving");
    }
    ConfigFile config = parse(descriptors, fileName);
    QTextStream out(&file);
    out << config.toYaml();
    if (!file.commit()) {
        throw std::runtime_error("There was an error writing the file");
    }
}

// creates/updates the configuration file with the given arguments
void import(const QString &fileName, const std::optional<QFileInfo>& mapDescriptorFile, const std::optional<int>& mapId, const std::optional<int>& mapSet, const std::optional<int>& zone, const std::optional<int>& order, const std::optional<int>& practiceBoard)
{
    ConfigFile config = parse(fileName);
    QFileInfo fileInfo(fileName);
    QDir dir(fileInfo.dir());

    QString returnValue;
    if(mapId.has_value()) {
        // modify existing board
        auto entry_to_modify = std::find_if(config.entries.begin(), config.entries.end(),
                     [&](const auto &entry) { return entry.mapId == mapId.value(); });
        if (entry_to_modify == config.entries.end()) {
            throw std::runtime_error("Error: Could not write to file");
        }
        if(mapSet.has_value())
            entry_to_modify->mapSet = mapSet.value();
        if(zone.has_value())
            entry_to_modify->mapZone = zone.value();
        if(order.has_value())
            entry_to_modify->mapOrder = order.value();
        if(practiceBoard.has_value())
            entry_to_modify->practiceBoard = practiceBoard.value();
        if(mapDescriptorFile.has_value()) {
            entry_to_modify->mapDescriptorRelativePath = dir.relativeFilePath(mapDescriptorFile.value().absoluteFilePath());
            entry_to_modify->name = mapDescriptorFile->baseName();
        }
        returnValue = entry_to_modify->toCsv();
    } else {
        // insert new board
        ConfigEntry entry;
        entry.mapId = config.maxId() + 1;
        if(mapSet.has_value())
            entry.mapSet = mapSet.value();
        else
            entry.mapSet = config.maxMapSet();
        if(zone.has_value())
            entry.mapZone = zone.value();
        else
            entry.mapZone = config.maxZone(entry.mapSet);
        if(order.has_value())
            entry.mapOrder = order.value();
        else
            entry.mapOrder = config.maxOrder(entry.mapSet, entry.mapZone) + 1;
        entry.practiceBoard = practiceBoard.value_or(0);
        entry.mapDescriptorRelativePath = dir.relativeFilePath(mapDescriptorFile.value().absoluteFilePath());
        entry.name = mapDescriptorFile->baseName();
        config.entries.append(entry);
        returnValue = entry.toCsv();
    }

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        throw std::runtime_error("Error: Could not write to file"); // TODO throw a subclass of QException
    }
    QTextStream out(&file);
    out << config.toYaml();
}


// loads the configuration file and loads the yaml files defined in the configuration file into the given tmpDir and the given descriptors object
void load(const QString &fileName, std::vector<MapDescriptor> &descriptors, const QDir& tmpDir,
          const std::function<void(double)> &progressCallback)
{
    ConfigFile configFile = parse(fileName);
    QFileInfo fileInfo(fileName);
    QDir dir(fileInfo.dir());
    while(descriptors.size() < configFile.maxId() + 1) {
        auto md = MapDescriptor();
        descriptors.push_back(md);
    }
    while(descriptors.size() > configFile.maxId() + 1) {
        descriptors.pop_back();
    }
    {
        int i = 0;
        for (auto it = configFile.backgroundPaths.begin();
             it != configFile.backgroundPaths.end();
             ++it) {

            qInfo() << "attempting to download background" << it.key();
            for (auto &url: it.value()) {
                auto dest = dir.filePath(it.key() + ".background.zip");
                try {
                    await(CSMMNetworkManager::downloadFileIfUrl(url, dest, [&](double progress) {
                        progressCallback((progress + i) / configFile.backgroundPaths.size() * 0.5);
                    }));
                    break;
                } catch (const std::runtime_error &e) {
                    qWarning() << "warning:" << e.what();
                    continue; // try next url
                }
            }
            ++i;
        }
    }
    for(int i=0; i<configFile.entries.size(); ++i) {
        auto &entry = configFile.entries[i];
        descriptors[entry.mapId].mapSet = entry.mapSet;
        descriptors[entry.mapId].zone = entry.mapZone;
        descriptors[entry.mapId].order = entry.mapOrder;
        descriptors[entry.mapId].isPracticeBoard = entry.practiceBoard;
        if(!entry.mapDescriptorRelativePath.isEmpty()) {
            auto descPath = dir.filePath(entry.mapDescriptorRelativePath);
            if (!entry.mapDescriptorUrls.empty()) {
                qInfo() << "trying to download map descriptor to" << descPath;
                for (auto &url: entry.mapDescriptorUrls) {
                    try {
                        await(CSMMNetworkManager::downloadFileIfUrl(url, descPath));
                    } catch (const std::runtime_error &e) {
                        qWarning() << "warning:" << e.what();
                        continue;
                    }
                }
            }
            ImportExportUtils::importYaml(descPath, descriptors[entry.mapId],
                    tmpDir,
                    [=](double progress) {
                double taskProgress = (i + progress) / configFile.entries.size();
                progressCallback(0.5 + taskProgress * 0.5);
            }, dir.path());
        }
    }
}

QString status(const QString &fileName)
{
    return parse(fileName).toYaml();
}

QString status(const std::vector<MapDescriptor> &descriptors, const QString &filePath)
{
    return parse(descriptors, filePath).toYaml();
}

}
