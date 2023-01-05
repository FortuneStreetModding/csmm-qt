#include "mainwindow.h"

#include "darkdetect.h"
#include <QApplication>

#include "lib/python/pythonbindings.h"
#include <pybind11/embed.h>
#include <iostream>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

#include "maincli.h"

PYBIND11_EMBEDDED_MODULE(pycsmm, m) {
    init_pycsmm(m);
}

static void setPyHome() {
#ifdef Q_OS_MAC
    Py_SetPythonHome(QDir(QApplication::applicationDirPath()).filePath("../Resources").toStdWString().c_str());
#else
    Py_SetPythonHome(QDir(QApplication::applicationDirPath()).filePath("py").toStdWString().c_str());
#endif
}

int main(int argc, char *argv[])
{
    // HACK: we need ENSUREPIP_OPTIONS set on Unix b/c pip otherwise thinks we're using a framework
    if (qEnvironmentVariableIsEmpty("ENSUREPIP_OPTIONS")) {
        qputenv("ENSUREPIP_OPTIONS", "1");
    }

    bool consoleMode = argc > 1;
    // consoleMode = true;
    if(consoleMode) {
        QCoreApplication app(argc, argv);
        QCoreApplication::setApplicationName("csmm");
        QCoreApplication::setApplicationVersion(QString("%1").arg(CSMM_VERSION));

        setPyHome();

        QStringList arguments = QCoreApplication::arguments();
        // arguments.clear();
        // arguments << "csmm" << "save" << "C:\\BoomStreet\\ST7P01";
        maincli::run(arguments);

        return 0;
    } else {
        QApplication app(argc, argv);
        QCoreApplication::setApplicationName("csmm");

        setPyHome();

        pybind11::scoped_interpreter guard{}; // start the python interpreter

#ifdef Q_OS_WIN
        // hide console window under Windows but only if the first argument is the full path to the executable
        //  -> this indicates that the exe file has been started by mouse double click
        if(QCoreApplication::applicationFilePath().replace("/", "\\") == QApplication::arguments().front()) {
            ::ShowWindow( ::GetConsoleWindow(), SW_HIDE );
        }
#endif
        initDarkThemeSettings();
        MainWindow *w = new MainWindow;
#ifdef Q_OS_LINUX
        auto iconPath = QDir(QApplication::applicationDirPath()).filePath("../../AppIcon.png");
        if (QFile::exists(iconPath)) {
            w->setWindowIcon(QIcon(iconPath));
        }
#endif
        w->setWindowTitle(QString("CSMM %1").arg(CSMM_VERSION));
        w->show();
        int e = app.exec();
        exit( e ); // needed to exit the hidden console
        return e;
    }
}
