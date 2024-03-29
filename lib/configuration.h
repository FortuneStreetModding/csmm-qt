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
    QVector<QString> mapDescriptorUrls;
    QString toCsv();
};

struct ConfigFile {
    QVector<ConfigEntry> entries;
    QMap<QString, QVector<QString>> backgroundPaths;
    bool error = false;
    int maxId();
    int maxMapSet();
    int maxZone(int mapSet);
    int maxOrder(int mapSet, int zone);
    QString toCsv();
    QString toYaml();
};

void save(const QString &fileName, const std::vector<MapDescriptor> &descriptors);
void import(const QString &fileName,
            const std::optional<QFileInfo> &mapDescriptorFile,
            const std::optional<int> &mapId,
            const std::optional<int> &mapSet,
            const std::optional<int> &zone,
            const std::optional<int> &order,
            const std::optional<int> &practiceBoard);
void load(const QString &fileName, std::vector<MapDescriptor> &descriptors, const QDir& tmpDir, const std::function<void (double)> &progressCallback = [](double) {});
QString status(const QString &fileName);
QString status(const std::vector<MapDescriptor> &descriptors, const QString &filePath);

}

#endif // CONFIGURATION_H
