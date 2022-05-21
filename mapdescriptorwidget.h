#ifndef MAPDESCRIPTORWIDGET_H
#define MAPDESCRIPTORWIDGET_H

#include <functional>
#include <QTableWidget>
#include <QTemporaryDir>
#include <QObject>
#include "lib/mapdescriptor.h"

class MapDescriptorWidget : public QTableWidget
{
public:
    explicit MapDescriptorWidget(QWidget *parent = nullptr);
    void loadRowWithMapDescriptor(int row, const MapDescriptor &descriptor);
    void appendMapDescriptor(const MapDescriptor &descriptor);
    void duplicateSelectedMapDescriptors();
    void removeSelectedMapDescriptors();
    void clearDescriptors();
    const QVector<QSharedPointer<MapDescriptor>> &getDescriptors();
    void setGameDirectoryFunction(const std::function<QString()> &fn);
    bool dirty = false;
private:
    QVector<QSharedPointer<MapDescriptor>> descriptors;
    std::function<QString()> getGameDirectory;
};

#endif // MAPDESCRIPTORWIDGET_H
