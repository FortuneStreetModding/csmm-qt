#include "maincli.h"

#include<QBuffer>
#include<QTimer>
#include<QCommandLineParser>

#include "lib/asyncfuture.h"
#include "lib/exewrapper.h"
#include "lib/patchprocess.h"

namespace maincli {

// From: https://github.com/benlau/asyncfuture/issues/11
template <typename T>
 inline void await(QFuture<T> future, int timeout = -1) {
     if (future.isFinished()) {
         return;
     }

     QFutureWatcher<T> watcher;
     watcher.setFuture(future);
     QEventLoop loop;

     if (timeout > 0) {
         QTimer::singleShot(timeout, &loop, &QEventLoop::quit);
     }

     QObject::connect(&watcher, SIGNAL(finished()), &loop, SLOT(quit()));
     loop.exec();
 }

void setupSubcommand(QCommandLineParser& parser, QString name, QString description) {
    parser.clearPositionalArguments();
    parser.setApplicationDescription("\n" + description);
    QCommandLineOption openCategory(" " + name + " command --\n");
    parser.addOption(openCategory);
}

void run(QStringList arguments)
{
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

-- Make sure that you have installed the required external tools via the GUI --
    File -> (Re-)Download External Tools

)").arg(VERSION_MAJOR).arg(VERSION_MINOR).arg(VERSION_BUILD));
    parser.setOptionsAfterPositionalArgumentsMode(QCommandLineParser::ParseAsOptions);

    QString commandsDescription = QString(R"(
  extract         Extract a Fortune Street game disc image to a directory.
  export          Export one or several map descriptor files (*.yaml) from a Fortune Street game directory.
  import          Import a map descriptor file (*.yaml) to a Fortune Street game directory.
  save            Save the pending changes in a Fortune Street game directory.
  discard         Discard the pending changes in a Fortune Street game directory.
  pack            Pack a Fortune Street game directory to a disc image (pending changes must be saved prior).
)").remove(0,1);

    parser.addPositionalArgument(QString(), QString(), "command");

    QCommandLineOption allOption(QStringList() << "a" << "all", "all");
    QCommandLineOption forceOption(QStringList() << "f" << "force", "Overwrite existing files.");
    QCommandLineOption mapSetOption(QStringList() << "m" << "mapSet", "The <mapSet> of the map (0=Standard, 1=Easy).", "mapSet");
    QCommandLineOption internalNamesOption(QStringList() << "n" << "name", "Comma seperated list of <names> of the maps to export. The name should be the internal name of the map.", "name");
    QCommandLineOption mapOrderOption(QStringList() << "o" << "order", "The <order> of the map within its zone (0,1,2...).", "order");
    QCommandLineOption mapIdOption(QStringList() << "i" << "id", "The <id> of the map (0,1,2...).", "id");
    QCommandLineOption mapPracticeBoardOption(QStringList() << "p" << "practice-board", "If set, the map is regarded as a practice board.");
    QCommandLineOption quietOption(QStringList() << "q" << "quiet", "Do not print anything to console (overrides verbose).");
    QCommandLineOption verboseOption(QStringList() << "v" << "verbose", "Print extended information to console.");
    QCommandLineOption directoryOption(QStringList() << "x" << "extract", "Extract the Fortune Street disc to directory.");
    QCommandLineOption mapZoneOption(QStringList() << "z" << "zone", "The <zone> of the map. 0=Super Mario Tour, 1=Dragon Quest Tour, 2=Special Tour.", "zone");
    QCommandLineOption helpOption(QStringList() << "h" << "?" << "help", "Show the help");

    // add some generic options
    parser.addOption(helpOption);
    parser.addOption(quietOption);
    verboseOption.setDescription(verboseOption.description() + "\n"); // add a linebreak at the last generic option
    parser.addOption(verboseOption);

    // Call parse()
    parser.parse(arguments);

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
    } else if (command == "extract") {
        setupSubcommand(parser, "extract", "Extract a Fortune Street game disc image to a directory. This is equivalent to\n  wit copy --psel data --preserve --fst source extractDir");

        parser.addPositionalArgument("source", "Source Fortune Street game disc image (.wbfs, .iso).", "extract <source>");
        parser.addPositionalArgument("extractDir", "Target directory to be the container of the Fortune Street game disc. [default = <workingDirectory>/<sourceFilename>]", "[extractDir]");
        parser.addOption(forceOption);

        parser.process(arguments);
        const QStringList args = parser.positionalArguments();
        if(parser.isSet(helpOption) || args.size() < 2) {
            cout << '\n' << parser.helpText();
        } else {
            const QString source = args.at(1);
            QFileInfo sourceFileInfo(source);
            const QString target = args.size() >= 3? args.at(2) : QDir::current().filePath(sourceFileInfo.baseName());
            QFileInfo targetFileInfo(target);
            cout << "Extracting " << source << " to " << target << "...\n";
            if(targetFileInfo.exists()) {
                if(parser.isSet(forceOption)) {
                    cout << "Overwriting " << target << "...\n";
                } else {
                    cout << "Cannot extract as " << target << " already exists. Use force option to overwrite.\n";
                    return;
                }
            }
            cout.flush();
            await(ExeWrapper::extractWbfsIso(source, target));
            cout << "done!";
        }
    } else if (command == "close") {

    } else if (command == "export") {

    } else if (command == "import") {

    } else {
        cout << description;
        cout << parser.helpText();
        cout << commandsDescription;
    }
}

}
