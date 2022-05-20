#include "mainwindow.h"

#include "darkdetect.h"
#include <QApplication>

#include "lib/python/pythonbindings.h"
#include <pybind11/embed.h>

#if defined( Q_OS_WIN )
#include <windows.h>
#endif

#include "maincli.h"

int main(int argc, char *argv[])
{
    bool consoleMode = argc > 1;
    // consoleMode = true;
    if(consoleMode) {
        QCoreApplication app(argc, argv);
        QCoreApplication::setApplicationName("csmm");
        QCoreApplication::setApplicationVersion(QString("%1").arg(CSMM_VERSION));

        QStringList arguments = QCoreApplication::arguments();
        // arguments.clear();
        // arguments << "csmm" << "save" << "C:\\BoomStreet\\ST7P01";
        maincli::run(arguments);

        QTimer::singleShot( 0, &app, &QCoreApplication::quit );
        return app.exec();
    } else {
        pybind11::scoped_interpreter guard{}; // start the python interpreter

        QApplication app(argc, argv);
    #if defined( Q_OS_WIN )
        // hide console window under Windows but only if the first argument is the full path to the executable
        //  -> this indicates that the exe file has been started by mouse double click
        if(QCoreApplication::applicationFilePath().replace("/", "\\") == QString(argv[0])) {
            ::ShowWindow( ::GetConsoleWindow(), SW_HIDE );
        }
    #endif
        initDarkThemeSettings();
        MainWindow *w = new MainWindow;
        w->setWindowTitle(QString("CSMM %1").arg(CSMM_VERSION));
        w->show();
        int e = app.exec();
        exit( e ); // needed to exit the hidden console
        return e;
    }
}
