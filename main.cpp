#include "mainwindow.h"

#include "darkdetect.h"
#include <QApplication>

#include <windows.h>

int main(int argc, char *argv[])
{
    bool consoleMode = argc > 1;
    if(consoleMode) {
        QCoreApplication app(argc, argv);
        QCoreApplication::setApplicationName("csmm");
        QCoreApplication::setApplicationVersion(QString("%1.%2.%3").arg(VERSION_MAJOR).arg(VERSION_MINOR).arg(VERSION_BUILD));
        QCommandLineParser parser;
        QString description(QString(R"(
        ****************************************************************
        *                                                              *
        *           .-------.   _____  _____ __  __ __  __             *
        *          /   o   /|  / ____|/ ____|  \/  |  \/  |            *
        *         /_______/o| | |    | (___ | \  / | \  / |            *
        *         | o   o | | | |     \___ \| |\/| | |\/| |            *
        *         |   o   |o/ | |____ ____) | |  | | |  | |            *
        *         | o   o |/   \_____|_____/|_|  |_|_|  |_|            *
        *         '-------'                                            *
        *                     Custom Street Map Manager %1.%2.%3          *
        *                                                              *
        *                     github.com/FortuneStreetModding          *
        *                                                              *
        ****************************************************************

)").arg(VERSION_MAJOR).arg(VERSION_MINOR).arg(VERSION_BUILD));
        parser.setOptionsAfterPositionalArgumentsMode(QCommandLineParser::ParseAsOptions);

        QString commandsDescription = QString(R"(
  open            open a Fortune Street game disc image or extracted directory
  export          export one or several map descriptor files (*.yaml)
  import          import a map descriptor file (*.yaml)
  save            save the changes to a Fortune Street game disc image or
                    extracted directory
  close           close a Fortune Street game disc image or extracted directory
)").remove(0,1);

        parser.addPositionalArgument(QString(), QString(), "command");

        QCommandLineOption allOption(QStringList() << "a" << "all", "all");
        QCommandLineOption forceOption(QStringList() << "f" << "force", "Overwrite existing files");
        QCommandLineOption mapSetOption(QStringList() << "m" << "mapSet", "The <mapSet> of the map (0=Standard, 1=Easy)", "mapSet");
        QCommandLineOption internalNamesOption(QStringList() << "n" << "name", "Comma seperated list of <names> of the maps to export. The name should be the internal name of the map", "name");
        QCommandLineOption mapOrderOption(QStringList() << "o" << "order", "The <order> of the map within its zone (0,1,2...)", "order");
        QCommandLineOption mapIdOption(QStringList() << "i" << "id", "The <id> of the map (0,1,2...)", "id");
        QCommandLineOption mapPracticeBoardOption(QStringList() << "p" << "practice-board", "If set, the map is regarded as a practice board");
        QCommandLineOption quietOption(QStringList() << "q" << "quiet", "Do not print anything to console (overrides verbose)");
        QCommandLineOption verboseOption(QStringList() << "v" << "verbose", "Print extended information to console");
        QCommandLineOption directoryOption(QStringList() << "x" << "extract", "Extract the Fortune Street disc to directory");
        QCommandLineOption mapZoneOption(QStringList() << "z" << "zone", "The zone of the map. 0=Super Mario Tour, 1=Dragon Quest Tour, 2=Special Tour", "zone");
        QCommandLineOption helpOption(QStringList() << "h" << "?" << "help", "Show the help");

        // add some generic options
        parser.addOption(helpOption);
        parser.addOption(quietOption);
        verboseOption.setDescription(verboseOption.description() + "\n"); // add a linebreak at the last generic option
        parser.addOption(verboseOption);

        // Call parse()
        parser.parse(QCoreApplication::arguments());

        QBuffer b;
        b.open(QIODevice::ReadWrite);
        QSharedPointer<QTextStream> coutp(parser.isSet(quietOption)? new QTextStream(&b): new QTextStream(stdout));
        QTextStream& cout = *coutp;

        QTextStream cerr(stderr);

        const QStringList args = parser.positionalArguments();
        const QString command = args.isEmpty() ? QString() : args.first();

        if (command.isEmpty()) {
            cout << description;
            cout << parser.helpText();
            cout << commandsDescription;
        } else if (command == "open") {
            parser.clearPositionalArguments();
            parser.setApplicationDescription("\nOpen a Fortune Street game disc image or extracted directory.");
            QCommandLineOption openCategory(" open command --\n");
            parser.addOption(openCategory);

            parser.addPositionalArgument("source", "Source Fortune Street game disc image or extracted directory", "open source");
            forceOption.setDescription("Force reopen the game disc image or extracted directory, even if it is already opened. This will result in the loss of pending changes.");
            parser.addOption(forceOption);

            parser.process(app);
            const QStringList args = parser.positionalArguments();
            if(parser.isSet(helpOption) || args.size() < 2) {
                cout << '\n' << parser.helpText();
            } else {
                const QString source = args.at(1);
                cout << "Opening " << source;
            }
        } else if (command == "close") {

        } else if (command == "export") {

        } else if (command == "import") {

        } else {
            cout << description;
            cout << parser.helpText();
            cout << commandsDescription;
        }
        QTimer::singleShot( 0, &app, &QCoreApplication::quit );
        return app.exec();
    } else {
        QApplication app(argc, argv);
    #if defined( Q_OS_WIN )
        // hide console window under Windows but only if the first argument is the full path to the executable
        //  -> this indicates that the exe file has been started by mouse double click
        if(QCoreApplication::applicationFilePath().replace("/", "\\") == QString(argv[0])) {
            ::ShowWindow( ::GetConsoleWindow(), SW_HIDE );
        }
    #endif
        initDarkThemeSettings();
        MainWindow w;
        w.setWindowTitle(QString("CSMM %1.%2.%3").arg(VERSION_MAJOR).arg(VERSION_MINOR).arg(VERSION_BUILD));
        w.show();
        int e = app.exec();
        exit( e ); // needed to exit the hidden console
        return e;
    }
}
