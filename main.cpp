#include "mainwindow.h"

#include "darkdetect.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    initDarkThemeSettings();

    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
