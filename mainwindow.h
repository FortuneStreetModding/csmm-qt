#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFuture>
#include <QtNetwork>

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

    QNetworkAccessManager *manager;

    void openFile();
    void saveFile();
    QFuture<bool> checkForRequiredFiles();
    template<class InToOutFiles>
    QFuture<bool> downloadRequiredFiles(QUrl witURL, InToOutFiles func);
};
#endif // MAINWINDOW_H
