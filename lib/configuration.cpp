#include "configuration.h"

#include <QTextStream>

#include "importexportutils.h"

namespace Configuration {

// to string methods
QString to_string(QString mapId, QString mapSet, QString mapZone, QString mapOrder, QString practiceBoard, QString name, QString mapDescriptorRelativePath) {
    QString table("%1, %2, %3, %4, %5, %6, %7");
    return table
            .arg(mapId, 3, QChar(' '))
            .arg(mapSet, 5, QChar(' '))
            .arg(mapZone, 5, QChar(' '))
            .arg(mapOrder, 5, QChar(' '))
            .arg(practiceBoard, 5, QChar(' '))
            .arg(name.leftJustified(24, ' ', true), mapDescriptorRelativePath);
}

QString to_string(int mapId, int mapSet, int mapZone, int mapOrder, int practiceBoard, QString name, QString mapDescriptorRelativePath) {
    return to_string(QString::number(mapId),QString::number(mapSet),QString::number(mapZone),QString::number(mapOrder),QString::number(practiceBoard),name,mapDescriptorRelativePath);
}

QString ConfigEntry::to_string() {
    return Configuration::to_string(mapId, mapSet, mapZone, mapOrder, practiceBoard, name, mapDescriptorRelativePath);
}

QString ConfigFile::to_string() {
    QString result("");
    result += Configuration::to_string("id", "mapS.", "zone", "order", "prac.", "name", "yaml") + "\n";
    for(auto& entry : entries) {
        result += Configuration::to_string(entry.mapId, entry.mapSet, entry.mapZone, entry.mapOrder, entry.practiceBoard, entry.name, entry.mapDescriptorRelativePath) + "\n";
    }
    return result;
}

// open and parse the config file
ConfigFile parse(QString fileName) {
    QFile file(fileName);
    ConfigFile configFile;
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
    configFile.entries = configEntries;
    return configFile;
}

ConfigFile parse(const std::vector<MapDescriptor> &descriptors, QString fileName) {
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

void save(QString fileName, const std::vector<MapDescriptor> &descriptors)
{
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;
    ConfigFile config = parse(descriptors, fileName);
    QTextStream out(&file);
    out << config.to_string();
}

// creates/updates the configuration file with the given arguments
QString import(QString fileName, std::optional<QFileInfo>& mapDescriptorFile, std::optional<int>& mapId, std::optional<int>& mapSet, std::optional<int>& zone, std::optional<int>& order, std::optional<int>& practiceBoard)
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
        returnValue = config.entries[mapId.value()].to_string();
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
        returnValue = entry.to_string();
    }

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return "Error: Could not write to file";
    QTextStream out(&file);
    out << config.to_string();

    return returnValue;
}


// loads the configuration file and loads the yaml files defined in the configuration file into the given tmpDir and the given descriptors object
void load(QString fileName, std::vector<MapDescriptor> &descriptors, const QDir& tmpDir)
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

QString status(QString fileName)
{
    return parse(fileName).to_string();
}

QString status(const std::vector<MapDescriptor> &descriptors, QString filePath)
{
    return parse(descriptors, filePath).to_string();
}

}
