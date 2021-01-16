#ifndef MAPDESCRIPTORWIDGET_H
#define MAPDESCRIPTORWIDGET_H

#include <QTableWidget>
#include <QObject>
#include "lib/mapdescriptor.h"

class MapDescriptorWidget : public QTableWidget
{
public:
    explicit MapDescriptorWidget(QWidget *parent = nullptr);
    void loadRowWithMapDescriptor(int row, const MapDescriptor &descriptor);
    void appendMapDescriptor(const MapDescriptor &descriptor);
    const QVector<QSharedPointer<MapDescriptor>> &getDescriptors();
private:
    QVector<QSharedPointer<MapDescriptor>> descriptors;
};

#endif // MAPDESCRIPTORWIDGET_H
