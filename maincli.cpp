#include "maincli.h"

#include<QBuffer>
#include<QTimer>
#include<QCommandLineParser>
#include<QTemporaryDir>

#include "lib/asyncfuture.h"
#include "lib/exewrapper.h"
#include "lib/patchprocess.h"
#include "lib/configuration.h"

namespace maincli {

// From: https://github.com/benlau/asyncfuture/issues/11

template <typename T>
inline T await(QFuture<T> future, int timeout = -1) {
     if (future.isFinished()) {
         return future.result();
     }

     QFutureWatcher<T> watcher;
     watcher.setFuture(future);
     QEventLoop loop;

     if (timeout > 0) {
         QTimer::singleShot(timeout, &loop, &QEventLoop::quit);
     }

     QObject::connect(&watcher, SIGNAL(finished()), &loop, SLOT(quit()));
     loop.exec();

     return future.result();
}

inline void await(QFuture<void> future, int timeout = -1) {
     if (future.isFinished()) {
         return;
     }

     QFutureWatcher<void> watcher;
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
  import          Add importing a map descriptor file (*.yaml) into a Fortune Street game directory as a pending change.
  status          Check the pending changes in a Fortune Street game directory.
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
    QCommandLineOption mapPracticeBoardOption(QStringList() << "p" << "practice-board", "Whether the map is regarded as a practice board (0=no|1=yes).", "practiceBoard");
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

    const QStringList args = parser.positionalArguments();
    const QString command = args.isEmpty() ? QString() : args.first();

    if (command.isEmpty()) {
        cout << description;
        cout << parser.helpText();
        cout << commandsDescription;
    } else if (command == "extract") {
        // --- extract ---
        setupSubcommand(parser, "extract", "Extract a Fortune Street game disc image to a directory. This is equivalent to\n  wit copy --psel data --preserve --fst source extractDir");

        parser.addPositionalArgument("source", "Source Fortune Street game disc image (.wbfs, .iso).", "extract <source>");
        parser.addPositionalArgument("extractDir", "Target directory to be the container of the Fortune Street game disc.\n[default = <workingDirectory>/<sourceFilename>]", "[extractDir]");
        parser.addOption(forceOption);

        parser.process(arguments);
        const QStringList args = parser.positionalArguments();
        if(parser.isSet(helpOption) || args.size() < 2) {
            cout << '\n' << parser.helpText();
        } else {
            const QString source = args.at(1);
            QFileInfo sourceFileInfo(source);
            if(!sourceFileInfo.exists()) {
                cout << source << " does not exist.\n";
                QCoreApplication::exit(1); return;
            } else if(!sourceFileInfo.isFile()) {
                cout << source << " is not a file.\n";
                QCoreApplication::exit(1); return;
            }
            const QString target = args.size() >= 3? args.at(2) : QDir::current().filePath(sourceFileInfo.baseName());
            const QDir targetDir(target);
            cout << "Extracting " << source << " to " << target << "...\n";
            if(targetDir.exists()) {
                if(parser.isSet(forceOption)) {
                    cout << "Overwriting " << target << "...\n";
                } else {
                    cout << "Cannot extract as " << target << " already exists. Use force option to overwrite.\n";
                    QCoreApplication::exit(1); return;
                }
            } else {
                targetDir.mkpath("");
            }
            cout.flush();
            await(ExeWrapper::extractWbfsIso(source, target));
        }
    } else if (command == "export") {
        // --- export ---
        setupSubcommand(parser, "export", "Export one or several map descriptor files (*.yaml) from a Fortune Street game directory.");

        parser.addPositionalArgument("gameDir", "Fortune Street game directory.", "export <gameDir>");
        parser.addPositionalArgument("targetDir", "Target directory to be the container of the exported map descriptor.\n[default = <workingDirectory>/<mapName>.yaml]", "[targetDir]");
        parser.addOption(allOption);
        parser.addOption(forceOption);
        parser.addOption(mapIdOption);
        parser.addOption(internalNamesOption);

        parser.process(arguments);
        const QStringList args = parser.positionalArguments();
        if(parser.isSet(helpOption) || args.size() < 2) {
            cout << '\n' << parser.helpText();
        } else {
            const QString source = args.at(1);
            const QDir sourceDir(source);
            if(!sourceDir.exists()) {
                cout << source << " does not exist.";
                QCoreApplication::exit(1); return;
            }
            const QString target = args.size() >= 3? args.at(2) : QDir::current().path();
            QDir targetDir(target);
            if(!targetDir.exists()) {
                targetDir.mkpath("");
            }
            auto descriptors = await(PatchProcess::openDir(sourceDir));
            if (descriptors.isEmpty()) {
                cout << source << " is not a proper Fortune Street directory.";
                QCoreApplication::exit(1); return;
            }
            QString internalNames = parser.value(internalNamesOption);
            QStringList internalNamesList = internalNames.split(",");

            QString mapIds = parser.value(mapIdOption);
            QVector<int> mapIdList;
            for(auto& str : mapIds.split(","))
                mapIdList.append(str.toInt());

            bool atLeastOneMatch = false;
            for(int i=0;i<descriptors.count();i++) {
                auto descriptor = descriptors.at(i);
                bool exportIt = false;
                if (parser.isSet(allOption))
                    exportIt = true;
                if (internalNamesList.contains(descriptor.internalName, Qt::CaseSensitivity::CaseInsensitive))
                    exportIt = true;
                if (mapIdList.contains(i))
                    exportIt = true;
                if (exportIt) {
                    QString targetPath(targetDir.filePath(descriptor.internalName + ".yaml"));
                    cout << "Exporting " << descriptor.internalName << " to " << targetPath << "...\n";
                    cout.flush();
                    PatchProcess::exportYaml(sourceDir, targetPath, descriptor);
                    atLeastOneMatch = true;
                }
            }
            if(!atLeastOneMatch) {
                cout << "No maps matched. Try providing --id or --name.\n";
                cout << '\n' << parser.helpText();
            }
        }
    } else if (command == "import") {
        // --- import ---
        setupSubcommand(parser, "import", "Add importing a map descriptor file (*.yaml) into a Fortune Street game directory as a pending change.\n\n"
            "  if --id is provided, the map with the given id is replaced/updated.\n"
            "  if --id is not provided, a new map will be added.");

        parser.addPositionalArgument("gameDir", "Fortune Street game directory.", "import <gameDir>");
        parser.addPositionalArgument("yaml", "The yaml file to import.", "<yaml>");
        parser.addOption(mapIdOption);
        parser.addOption(mapSetOption);
        parser.addOption(mapZoneOption);
        parser.addOption(mapOrderOption);
        parser.addOption(mapPracticeBoardOption);
        parser.process(arguments);

        std::optional<int> mapId, mapSet, mapZone, mapOrder, mapPracticeBoard;
        if(parser.isSet(mapIdOption)) mapId.emplace(parser.value(mapIdOption).toInt());
        if(parser.isSet(mapSetOption)) mapSet.emplace(parser.value(mapSetOption).toInt());
        if(parser.isSet(mapZoneOption)) mapZone.emplace(parser.value(mapZoneOption).toInt());
        if(parser.isSet(mapOrderOption)) mapOrder.emplace(parser.value(mapOrderOption).toInt());
        if(parser.isSet(mapPracticeBoardOption)) mapPracticeBoard.emplace(parser.value(mapPracticeBoardOption).toInt());

        const QStringList args = parser.positionalArguments();
        if(parser.isSet(helpOption) || args.size() < 3) {
            cout << '\n' << parser.helpText();
        } else {
            const QString source = args.at(1);
            const QDir sourceDir(source);
            if(!sourceDir.exists()) {
                cout << source << " does not exist.";
                QCoreApplication::exit(1); return;
            }
            const QString yaml = args.at(2);
            const QFile yamlFile(yaml);
            if(!yamlFile.exists()) {
                cout << yaml << " does not exist.";
                QCoreApplication::exit(1); return;
            }
            QFile file(sourceDir.filePath("csmm_pending_changes.csv"));
            if(!file.exists()) {
                auto descriptors = await(PatchProcess::openDir(sourceDir));
                if (descriptors.isEmpty()) {
                    cout << source << " is not a proper Fortune Street directory.";
                    QCoreApplication::exit(1); return;
                }
                Configuration::save(sourceDir.filePath("csmm_pending_changes.csv"), descriptors);
            }
            std::optional<QFileInfo> yamlOpt(yamlFile);
            cout << Configuration::import(sourceDir.filePath("csmm_pending_changes.csv"), yamlOpt, mapId, mapSet, mapZone, mapOrder, mapPracticeBoard);
        }

    } else if (command == "status") {
        // --- status ---
        setupSubcommand(parser, "status", "Print out the pending changes");
        parser.addPositionalArgument("gameDir", "Fortune Street game directory.", "status <gameDir>");

        parser.process(arguments);

        const QStringList args = parser.positionalArguments();
        if(parser.isSet(helpOption) || args.size() < 2) {
            cout << '\n' << parser.helpText();
        } else {
            const QString source = args.at(1);
            const QDir sourceDir(source);
            if(!sourceDir.exists()) {
                cout << source << " does not exist.";
                QCoreApplication::exit(1); return;
            }
            QFile file(sourceDir.filePath("csmm_pending_changes.csv"));
            if(file.exists()) {
                cout << Configuration::status(sourceDir.filePath("csmm_pending_changes.csv"));
            } else {
                cout << "There are no pending changes";
            }
        }
    } else if (command == "save") {
        // --- save ---
        setupSubcommand(parser, "save", "Write the pending changes into the Fortune Street game directory.");
        parser.addPositionalArgument("gameDir", "Fortune Street game directory.", "save <gameDir>");

        parser.process(arguments);
        const QStringList args = parser.positionalArguments();
        if(parser.isSet(helpOption) || args.size() < 2) {
            cout << '\n' << parser.helpText();
        } else {
            const QString source = args.at(1);
            const QDir sourceDir(source);
            if(!sourceDir.exists()) {
                cout << source << " does not exist.";
                QCoreApplication::exit(1); return;
            }
            QFile file(sourceDir.filePath("csmm_pending_changes.csv"));
            if(file.exists()) {
                QTemporaryDir intermediateDir;
                if (!intermediateDir.isValid()) {
                    cout << "Could not create an intermediate directory.";
                }
                auto descriptors = await(PatchProcess::openDir(sourceDir));
                if (descriptors.isEmpty()) {
                    cout << source << " is not a proper Fortune Street directory.";
                    QCoreApplication::exit(1); return;
                }
                Configuration::load(sourceDir.filePath("csmm_pending_changes.csv"), descriptors, intermediateDir.path());

                await(PatchProcess::saveDir(sourceDir, descriptors, false, intermediateDir.path()));

                file.remove();
                cout << "Pending changes have been saved";
            } else {
                cout << "There are no pending changes to save. Run csmm import first.";
            }
        }
    } else if (command == "discard") {
        // --- discard ---
        setupSubcommand(parser, "discard", "Discard the pending changes");
        parser.addPositionalArgument("gameDir", "Fortune Street game directory.", "discard <gameDir>");

        parser.process(arguments);

        const QStringList args = parser.positionalArguments();
        if(parser.isSet(helpOption) || args.size() < 2) {
            cout << '\n' << parser.helpText();
        } else {
            const QString source = args.at(1);
            const QDir sourceDir(source);
            if(!sourceDir.exists()) {
                cout << source << " does not exist.";
                QCoreApplication::exit(1); return;
            }
            QFile file(sourceDir.filePath("csmm_pending_changes.csv"));
            if(file.exists()) {
                file.remove();
                cout << "Pending changes have been discarded";
            } else {
                cout << "There are no pending changes to discard";
            }
        }
    } else if (command == "pack") {

    } else {
        cout << description;
        cout << parser.helpText();
        cout << commandsDescription;
    }
}

}
