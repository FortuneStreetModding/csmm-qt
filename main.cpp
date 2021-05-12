#include "mainwindow.h"

#include "darkdetect.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    initDarkThemeSettings();

    MainWindow w;
    w.setWindowTitle(QString("CSMM %1.%2.%3").arg(VERSION_MAJOR).arg(VERSION_MINOR).arg(VERSION_BUILD));
    w.show();
    return a.exec();
}
