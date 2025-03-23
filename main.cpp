#include "choosemode.h"
#include "csmmmode.h"
#include <QApplication>
#include <QDir>
#include <QProgressDialog>
#include <QStyleHints>
#include "lib/python/pythonbindings.h"
#include "mainwindow.h"
#include "darkdetect.h"
#include "quicksetupdialog.h"
#include <pybind11/embed.h>

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

static constexpr int MAX_LOGS = 10;

static QFile logFile;

static void initLogFile() {
    if (!logFile.isOpen()) {
        QDir logFileDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
        if (!logFileDir.mkpath(".")) {
            qWarning() << "could not create directory" << logFileDir.path();
            return;
        }
        logFile.setFileName(logFileDir.filePath(QString("csmmgui-%0.log").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd-HH-mm-ss"))));
        if (!logFile.open(QFile::WriteOnly)) {
            qWarning() << "could not create log file" << logFile.fileName() << "for writing";
            return;
        }
        logFile.write(QString("Running CSMM %1\n").arg(QCoreApplication::applicationVersion()).toUtf8());
        if (!logFile.link(logFileDir.filePath("csmmgui-latest.log"))) {
            qWarning() << "could not symlink to latest log file" << logFile.fileName();
        }
        auto pastLogs = logFileDir.entryInfoList({"csmmgui*.log"}, QDir::Files);
        std::sort(pastLogs.begin(), pastLogs.end(), [](const QFileInfo &A, const QFileInfo &B) {
            return A.fileTime(QFile::FileBirthTime) > B.fileTime(QFile::FileBirthTime);
        });
        while (pastLogs.size() > MAX_LOGS) {
            if (!QFile::remove(pastLogs.back().absoluteFilePath())) {
                qWarning() << "could not remove old log file" << pastLogs.back().absoluteFilePath();
            }
            pastLogs.pop_back();
        }
    }
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
        QCoreApplication::setOrganizationName("Custom Street");
        QCoreApplication::setOrganizationDomain("fortunestreetmodding.github.io");
        QCoreApplication::setApplicationName("csmm");
        QCoreApplication::setApplicationVersion(QString("%1").arg(CSMM_VERSION));
        initWindowPaletteSettings(app.styleHints()->colorScheme());

        setPyHome();

        pybind11::scoped_interpreter guard{}; // start the python interpreter

        initLogFile();

        // show progress messages in progress dialog
        qInstallMessageHandler([](QtMsgType type, const QMessageLogContext &context, const QString &msg) {
            // filter this specific message since it is littering the whole console output
            if(msg == "QFutureWatcher::connect: connecting after calling setFuture() is likely to produce race")
                return;
            static QTextStream cerr(stderr);
            static QTextStream logFileStream(&logFile);
            cerr << msg << Qt::endl;
            if (logFile.isOpen()) {
                logFileStream << msg << Qt::endl;
            }
            auto progressDialog = dynamic_cast<QProgressDialog *>(QApplication::activeModalWidget());
            if (type != QtMsgType::QtDebugMsg && progressDialog != nullptr) {
                progressDialog->setLabelText(msg);
            }
        });

#ifdef Q_OS_WIN
        // hide console window under Windows but only if the first argument is the full path to the executable
        //  -> this indicates that the exe file has been started by mouse double click
        if(QCoreApplication::applicationFilePath().replace("/", "\\") == QApplication::arguments().at(0)) {
            ::ShowWindow( ::GetConsoleWindow(), SW_HIDE );
        }
#endif
        QSettings settings;
        // configure the network cache and temporary directories if they are not present
        if (!settings.contains("networkCacheDirectory") || !settings.value("networkCacheDirectory").isValid()) {
            QDir applicationCacheDir(QStandardPaths::writableLocation(QStandardPaths::CacheLocation));
            auto defaultNetworkCacheDir = applicationCacheDir.filePath("networkCache");
            settings.setValue("networkCacheDirectory", defaultNetworkCacheDir);
        }
        if (!settings.contains("temporaryDirectory") || !settings.value("temporaryDirectory").isValid()) {
            QTemporaryDir d;
            settings.setValue("temporaryDirectory", d.path());
            d.remove();
        }
        QWidget *w;
        switch (settings.value("csmmMode", INDETERMINATE).toInt()) {
        case INDETERMINATE:
            w = new ChooseMode();
            break;
        case EXPRESS:
            w = new QuickSetupDialog("02", false);
            break;
        case ADVANCED:
            w = new MainWindow();
            break;
        }
#ifdef Q_OS_LINUX
        auto iconPath = QDir(QApplication::applicationDirPath()).filePath("../../AppIcon.png");
        if (QFile::exists(iconPath)) {
            w->setWindowIcon(QIcon(iconPath));
        }
#endif
        w->show();
        int e = app.exec();
        exit( e ); // needed to exit the hidden console
        return e;
    }
}
