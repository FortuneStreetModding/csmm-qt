#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <QString>
#include <QTextStream>
#include <optional>
#include "mapdescriptor.h"

namespace Configuration {

struct ConfigEntry {
    int mapId;
    int mapSet;
    int mapZone;
    int mapOrder;
    int practiceBoard;
    QString name;
    QString mapDescriptorRelativePath;
    QString to_string();
};

struct ConfigFile {
    QVector<ConfigEntry> entries;
    bool error = false;
    int maxId();
    int maxMapSet();
    int maxZone(int mapSet);
    int maxOrder(int mapSet, int zone);
    QString to_string();
};

QString to_string(QString mapId, QString mapSet, QString mapZone, QString mapOrder, QString practiceBoard, QString name, QString mapDescriptorRelativePath);
QString to_string(int mapId, int mapSet, int mapZone, int mapOrder, int practiceBoard, QString name, QString mapDescriptorRelativePath);

void save(QString fileName, const QVector<MapDescriptor> &descriptors);
QString import(QString fileName, std::optional<QFileInfo>& mapDescriptorFile, std::optional<int>& mapId, std::optional<int>& mapSet, std::optional<int>& zone, std::optional<int>& order, std::optional<int>& practiceBoard);
void load(QString fileName, QVector<MapDescriptor> &descriptors, const QDir& tmpDir);
QString status(QString fileName);
QString status(const QVector<MapDescriptor> &descriptors, QString filePath);

}

#endif // CONFIGURATION_H
