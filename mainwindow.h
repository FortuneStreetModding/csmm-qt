#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFuture>
#include <QtNetwork>
#include <QSharedPointer>
#include <QTemporaryDir>
#include "lib/mapdescriptor.h"
#include "lib/mods/csmmmodpack.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    QSharedPointer<QTemporaryDir> tempGameDir;

    ModListType modList;

    std::shared_ptr<QTemporaryDir> tempModpackDir;

    void openDir();
    void openIsoWbfs();
    void exportToFolder(bool riivolution);
    void exportIsoWbfs();
    void loadDescriptors(const std::vector<MapDescriptor> &descriptors);
    QFuture<void> checkForRequiredFiles(bool alwaysAsk = false);
    void validateMaps();
    void saveCleanItastCsmmBrsar();
    void saveMapList();
    void loadMapList();
    void updateModListWidget();

    QString getSaveId();
};
#endif // MAINWINDOW_H
