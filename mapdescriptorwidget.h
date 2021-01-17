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
    const QVector<QSharedPointer<MapDescriptor>> &getDescriptors();
    void setGameDirectoryFunction(const std::function<QString()> &fn);
    const QTemporaryDir &getTmpDir();
private:
    QVector<QSharedPointer<MapDescriptor>> descriptors;
    std::function<QString()> getGameDirectory;
    QTemporaryDir tmpDir;
};

#endif // MAPDESCRIPTORWIDGET_H
