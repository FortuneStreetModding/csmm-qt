#include "configuration.h"

#include <QSaveFile>
#include <QTextStream>
#include <yaml-cpp/yaml.h>
#include "QtCore/qregularexpression.h"
#include "importexportutils.h"
#include <fstream>
#include <filesystem>

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
                                             ? QString("!default%1_%2").arg(entry.mapId).arg(entry.name) : entry.mapDescriptorRelativePath).toStdString();
                    emitter << YAML::Value;
                    emitter << YAML::BeginMap;
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

// open and parse the config file
static ConfigFile parse(QString fileName) {
    ConfigFile configFile;
    if (QFileInfo(fileName).suffix() == "csv") {
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
    } else {
        QVector<ConfigEntry> configEntries;
        std::ifstream stream(std::filesystem::path(fileName.toStdU16String()));
        auto node = YAML::Load(stream);

        int defaultMapId = 0;
        for (auto it=node.begin(); it!=node.end(); ++it) { // iterate over map sets
            auto mapSet = it->first.as<int>();
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
        configFile.entries = std::move(configEntries);
    }
    return configFile;
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
        if(mapSet.has_value())
            config.entries[mapId.value()].mapSet = mapSet.value();
        if(zone.has_value())
            config.entries[mapId.value()].mapZone = zone.value();
        if(order.has_value())
            config.entries[mapId.value()].mapOrder = order.value();
        if(practiceBoard.has_value())
            config.entries[mapId.value()].practiceBoard = practiceBoard.value();
        if(mapDescriptorFile.has_value()) {
            config.entries[mapId.value()].mapDescriptorRelativePath = dir.relativeFilePath(mapDescriptorFile.value().absoluteFilePath());
            config.entries[mapId.value()].name = mapDescriptorFile->baseName();
        }
        returnValue = config.entries[mapId.value()].toCsv();
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
void load(const QString &fileName, std::vector<MapDescriptor> &descriptors, const QDir& tmpDir)
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
    for(ConfigEntry& entry : configFile.entries) {
        descriptors[entry.mapId].mapSet = entry.mapSet;
        descriptors[entry.mapId].zone = entry.mapZone;
        descriptors[entry.mapId].order = entry.mapOrder;
        descriptors[entry.mapId].isPracticeBoard = entry.practiceBoard;
        if(!entry.mapDescriptorRelativePath.isEmpty()) {
            ImportExportUtils::importYaml(dir.filePath(entry.mapDescriptorRelativePath), descriptors[entry.mapId], tmpDir);
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
