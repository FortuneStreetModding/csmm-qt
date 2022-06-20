#include "maincli.h"

#include <QBuffer>
#include <QTimer>
#include <QCommandLineParser>
#include <QTemporaryDir>
//#include <filesystem>

#include "lib/await.h"
#include "lib/asyncfuture.h"
#include "lib/exewrapper.h"
#include "lib/importexportutils.h"
#include "lib/configuration.h"
#include "lib/downloadtools.h"
#include "lib/datafileset.h"
#include "lib/mods/csmmmodpack.h"
#include "lib/mods/modloader.h"
#include "lib/mods/defaultmodlist.h"
#include <pybind11/embed.h>

namespace maincli {

static void setupSubcommand(QCommandLineParser& parser, QString name, QString description) {
    parser.clearPositionalArguments();
    parser.setApplicationDescription("\n" + description);
    QCommandLineOption openCategory(" " + name + " command --\n");
    parser.addOption(openCategory);
}

void run(QStringList arguments)
{
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
    *                     Custom Street Map Manager %1          *
    *                                                              *
    *                     github.com/FortuneStreetModding          *
    *                                                              *
    ****************************************************************

-- Make sure that you have installed the required external tools via the GUI --
    File -> (Re-)Download External Tools

)").arg(CSMM_VERSION));

    if (arguments.size() > 1 && arguments[1] == "python") {
        wchar_t **args = new wchar_t *[arguments.size()]();
        auto programName = (arguments[0] + " python");
        args[0] = new wchar_t[programName.size() + 1]();
        programName.toWCharArray(args[0]);
        for (int i=2; i<arguments.size(); ++i) {
            args[i-1] = new wchar_t[arguments[i].size() + 1]();
            arguments[i].toWCharArray(args[i-1]);
        }

        int result = Py_Main(arguments.size() - 1, args);

        for (int i=0; i<arguments.size() - 1; ++i) {
            delete [] args[i];
        }
        delete [] args;

        exit(result);
    }

    QCommandLineParser parser;

    QString commandsDescription = QString(R"(
  extract         Extract a Fortune Street game disc image to a directory.
  export          Export one or several map descriptor files (*.yaml) from a Fortune Street game directory.
  import          Add importing a map descriptor file (*.yaml) into a Fortune Street game directory as a pending change.
  status          Check the pending changes in a Fortune Street game directory.
  discard         Discard the pending changes in a Fortune Street game directory.
  save            Save the pending changes in a Fortune Street game directory.
  pack            Pack a Fortune Street game directory to a disc image (pending changes must be saved prior).
  download-tools  Downloads the external tools that CSMM requires.
  python          Run CSMM's embedded Python interpreter
  default-modlist Output a list of default mod ids
)").remove(0,1);

    parser.setOptionsAfterPositionalArgumentsMode(QCommandLineParser::ParseAsOptions);

    parser.addPositionalArgument(QString(), QString(), "command");

    QCommandLineOption allOption(QStringList() << "a" << "all", "all");
    QCommandLineOption forceOption(QStringList() << "f" << "force", "Overwrite existing files.");
    QCommandLineOption mapSetOption(QStringList() << "m" << "mapSet", "The <mapSet> of the map (0=Standard, 1=Easy).", "mapSet");
    QCommandLineOption internalNamesOption(QStringList() << "n" << "name", "Comma seperated list of <names> of the maps to export. The name should be the internal name of the map.", "name");
    QCommandLineOption mapOrderOption(QStringList() << "o" << "order", "The <order> of the map within its zone (0,1,2...).", "order");
    QCommandLineOption mapIdOption(QStringList() << "i" << "id", "The <id> of the map (0,1,2...).", "id");
    QCommandLineOption mapPracticeBoardOption(QStringList() << "p" << "practice-board", "Whether the map is regarded as a practice board (0=no|1=yes).", "practiceBoard");
    QCommandLineOption quietOption(QStringList() << "q" << "quiet", "Do not print anything to console (overrides verbose).");
    // QCommandLineOption verboseOption(QStringList() << "v" << "verbose", "Print extended information to console.");
    QCommandLineOption saveIdOption(QStringList() << "s" << "saveId", "Set the save id for the iso/wbfs file. It can be any value between 00-ZZ using any digits or uppercase ASCII letters. The original game uses 01. Default is 02.", "saveId");
    QCommandLineOption mapZoneOption(QStringList() << "z" << "zone", "The <zone> of the map. 0=Super Mario Tour, 1=Dragon Quest Tour, 2=Special Tour.", "zone");
    QCommandLineOption modPackOption(QStringList() << "modpack", "The modpack file (.zip or modlist.txt) to load (leave blank for default).", "modpack");
    QCommandLineOption mapDescriptorConfigurationOption(QStringList() << "descCfg" << "descriptorCfg" << "descConfiguration" << "descriptorConfiguration", "The map description configuration .csv to use for saving instead of the default.", "descCfg", "");
    QCommandLineOption witUrlOption("wit-url", "The URL where to download WIT", "url", WIT_URL);
    QCommandLineOption wszstUrlOption("wszst-url","The URL where to download WSZST", "url", WSZST_URL);
    QCommandLineOption helpOption(QStringList() << "h" << "?" << "help", "Show the help");
    // add some generic options
    parser.addOption(helpOption);
    parser.addOption(quietOption);
    quietOption.setDescription(quietOption.description()); // add a linebreak at the last generic option
    //parser.addOption(verboseOption);

    // Call parse()
    parser.parse(arguments);

    QBuffer b;
    b.open(QIODevice::ReadWrite);
    QSharedPointer<QTextStream> coutp(parser.isSet(quietOption)? new QTextStream(&b): new QTextStream(stdout));
    QTextStream& cout = *coutp;

    pybind11::scoped_interpreter guard{};

    try {
        const QStringList args = parser.positionalArguments();
        const QString command = args.isEmpty() ? QString() : args.first();

        if (command.isEmpty()) {
            cout << description;
            cout << parser.helpText();
            cout << commandsDescription;
        } else if (command == "default-modlist") {
            auto defaultModList = DefaultModList::defaultModList();
            for (auto &mod: defaultModList) {
                cout << mod->modId() << "\n";
            }
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
                    exit(1);
                } else if(!sourceFileInfo.isFile()) {
                    cout << source << " is not a file.\n";
                    exit(1);
                }
                const QString target = args.size() >= 3? args.at(2) : QDir::current().filePath(sourceFileInfo.baseName());
                const QDir targetDir(target);
                cout << "Extracting " << source << " to " << target << "...\n";
                if(targetDir.exists()) {
                    if(parser.isSet(forceOption)) {
                        cout << "Overwriting " << target << "...\n";

                    } else {
                        cout << "Cannot extract as " << target << " already exists. Use force option to overwrite.\n";
                        exit(1);
                    }
                } else {
                    targetDir.mkpath(".");
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
            parser.addOption(modPackOption);

            parser.process(arguments);
            const QStringList args = parser.positionalArguments();
            if(parser.isSet(helpOption) || args.size() < 2) {
                cout << '\n' << parser.helpText();
            } else {
                const QString source = args.at(1);
                const QDir sourceDir(source);
                if(!sourceDir.exists()) {
                    cout << source << " does not exist.";
                    exit(1);
                }
                const QString target = args.size() >= 3? args.at(2) : QDir::current().path();
                QDir targetDir(target);
                if(!targetDir.exists()) {
                    targetDir.mkpath(".");
                }
                auto gameInstance = GameInstance::fromGameDirectory(sourceDir.path());
                auto mods = ModLoader::importModpackFile(parser.value(modPackOption));
                CSMMModpack modpack(gameInstance, mods.first.begin(), mods.first.end());
                modpack.load(sourceDir.path());
                auto descriptors = gameInstance.mapDescriptors();
                if (descriptors.empty()) {
                    cout << source << " is not a proper Fortune Street directory.";
                    exit(1);
                }
                QString internalNames = parser.value(internalNamesOption);
                QStringList internalNamesList = internalNames.split(",");

                QString mapIds = parser.value(mapIdOption);
                QVector<int> mapIdList;
                for(auto& str : mapIds.split(","))
                    mapIdList.append(str.toInt());

                bool atLeastOneMatch = false;
                for(int i=0;i<descriptors.size();i++) {
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
                        ImportExportUtils::exportYaml(sourceDir, targetPath, descriptor);
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
            parser.addOption(modPackOption);

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
                    exit(1);
                }
                const QString yaml = args.at(2);
                const QFile yamlFile(yaml);
                if(!yamlFile.exists()) {
                    cout << yaml << " does not exist.";
                    exit(1);
                }
                QFile file(sourceDir.filePath("csmm_pending_changes.yaml"));
                if(!file.exists()) {
                    auto gameInstance = GameInstance::fromGameDirectory(sourceDir.path());
                    auto mods = ModLoader::importModpackFile(parser.value(modPackOption));
                    CSMMModpack modpack(gameInstance, mods.first.begin(), mods.first.end());
                    modpack.load(sourceDir.path());
                    auto descriptors = gameInstance.mapDescriptors();
                    if (descriptors.empty()) {
                        cout << source << " is not a proper Fortune Street directory.";
                        exit(1);
                    }
                    Configuration::save(sourceDir.filePath("csmm_pending_changes.yaml"), descriptors);
                }
                std::optional<QFileInfo> yamlOpt(yamlFile);
                Configuration::import(sourceDir.filePath("csmm_pending_changes.yaml"), yamlOpt, mapId, mapSet, mapZone, mapOrder, mapPracticeBoard);
            }

        } else if (command == "status") {
            // --- status ---
            setupSubcommand(parser, "status", "Print out the pending changes.");
            parser.addPositionalArgument("gameDir", "Fortune Street game directory.", "status <gameDir>");
            parser.addOption(modPackOption);

            parser.process(arguments);

            const QStringList args = parser.positionalArguments();
            if(parser.isSet(helpOption) || args.size() < 2) {
                cout << '\n' << parser.helpText();
            } else {
                const QString source = args.at(1);
                const QDir sourceDir(source);
                if(!sourceDir.exists()) {
                    cout << source << " does not exist.";
                    exit(1);
                }
                QFile file(sourceDir.filePath("csmm_pending_changes.yaml"));
                if(file.exists()) {
                    cout << Configuration::status(sourceDir.filePath("csmm_pending_changes.yaml"));
                } else {
                    auto gameInstance = GameInstance::fromGameDirectory(sourceDir.path());
                    auto mods = ModLoader::importModpackFile(parser.value(modPackOption));
                    CSMMModpack modpack(gameInstance, mods.first.begin(), mods.first.end());
                    modpack.load(sourceDir.path());
                    auto descriptors = gameInstance.mapDescriptors();
                    if (descriptors.empty()) {
                        cout << source << " is not a proper Fortune Street directory.";
                        exit(1);
                    }
                    cout << Configuration::status(descriptors, sourceDir.filePath("csmm_pending_changes.yaml"));
                    cout << "There are no pending changes";
                }
            }
        } else if (command == "save") {
            // --- save ---
            setupSubcommand(parser, "save", "Write the pending changes into the Fortune Street game directory.");
            parser.addPositionalArgument("gameDir", "Fortune Street game directory.", "save <gameDir>");
            parser.addOption(modPackOption);
            parser.addOption(mapDescriptorConfigurationOption);

            parser.process(arguments);
            const QStringList args = parser.positionalArguments();
            if(parser.isSet(helpOption) || args.size() < 2) {
                cout << '\n' << parser.helpText();
            } else {
                const QString source = args.at(1);
                const QDir sourceDir(source);
                if(!sourceDir.exists()) {
                    cout << source << " does not exist.";
                    exit(1);
                }
                QFile file(sourceDir.filePath("csmm_pending_changes.yaml"));
                if(file.exists()) {
                    auto gameInstance = GameInstance::fromGameDirectory(sourceDir.path());
                    auto mods = ModLoader::importModpackFile(parser.value(modPackOption));
                    CSMMModpack modpack(gameInstance, mods.first.begin(), mods.first.end());
                    modpack.load(sourceDir.path());
                    auto &descriptors = gameInstance.mapDescriptors();
                    if (descriptors.empty()) {
                        cout << source << " is not a proper Fortune Street directory.";
                        exit(1);
                    }
                    try {
                        auto cfgPath = parser.value(mapDescriptorConfigurationOption);
                        if (cfgPath.isEmpty()) {
                            cfgPath = sourceDir.filePath("csmm_pending_changes.yaml");
                        }
                        Configuration::load(cfgPath, descriptors, sourceDir.path());

                        QString dolOriginalPath(sourceDir.filePath(MAIN_DOL));
                        QString dolBackupPath(sourceDir.filePath(MAIN_DOL) + ".bak");
                        QFile dolOriginal(dolOriginalPath);
                        QFile dolBackup(dolBackupPath);
                        if(dolBackup.exists()) {
                            dolOriginal.remove();
                            dolBackup.copy(dolOriginalPath);
                        } else {
                            dolOriginal.copy(dolBackupPath);
                        }

                        if (std::any_of(mods.first.begin(), mods.first.end(), [](auto &mod) { return mod->modId() == "wifiFix"; })) {
                            cout << "**> The game will be saved with Wiimmfi text replacing WFC. Wiimmfi will only be patched after packing it to a wbfs/iso using csmm pack command.\n";
                            cout << "\n";
                            cout.flush();
                        }
                        modpack.save(sourceDir.path());

                        cout << "\n";
                        cout << "Pending changes have been saved\n";
                    } catch (const std::runtime_error &exception) {
                        cout << QString("Error loading the map: %1").arg(exception.what());
                        exit(1);
                    }
                    // file.remove();
                } else {
                    cout << "There are no pending changes to save. Run csmm import first.";
                }
            }
        } else if (command == "discard") {
            // --- discard ---
            setupSubcommand(parser, "discard", "Discard the pending changes.");
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
                    exit(1);
                }
                QFile file(sourceDir.filePath("csmm_pending_changes.yaml"));
                if(file.exists()) {
                    file.remove();
                    cout << "Pending changes have been discarded";
                } else {
                    cout << "There are no pending changes to discard";
                }
            }
        } else if (command == "pack") {
            // --- pack ---
            setupSubcommand(parser, "pack", "Pack the game into an iso/wbfs file.");
            parser.addPositionalArgument("gameDir", "Fortune Street game directory.", "pack <gameDir>");
            parser.addPositionalArgument("target", "Target filename.\n[default = <gameId6>.wbfs]", "[target]");

            parser.addOption(saveIdOption);
            parser.addOption(forceOption);

            parser.process(arguments);

            const QStringList args = parser.positionalArguments();
            if(parser.isSet(helpOption) || args.size() < 2) {
                cout << '\n' << parser.helpText();
            } else {
                const QString source = args.at(1);
                const QDir sourceDir(source);
                if(!sourceDir.exists()) {
                    cout << source << " does not exist.";
                    exit(1);
                }

                QString saveId = parser.isSet(saveIdOption)? parser.value(saveIdOption) : "02";

                QString target;
                if(args.size() >= 3) {
                    target = args.at(2);
                } else {
                    QString id6 = await(ExeWrapper::getId6(source));
                    QString id4 = id6.remove(4,2);
                    target = QDir::current().filePath(id4 + saveId + ".wbfs");
                }
                QFile targetFile(target);
                if(targetFile.exists()) {
                    if(parser.isSet(forceOption)) {
                        cout << "Overwriting " << target << "...\n";
                    } else {
                        cout << "Cannot extract as " << target << " already exists. Use force option to overwrite.\n";
                        exit(1);
                    }
                }
                QFileInfo targetInfo(target);

                QDir targetDir(targetInfo.dir());
                if(!targetDir.exists()) {
                    targetDir.mkpath(".");
                }

                cout << "Creating " << targetInfo.suffix() << " file at " << target << " out from " << source << "...";
                cout.flush();

                bool patchWiimmfi = ImportExportUtils::hasWiimmfiText(sourceDir);
                await(ExeWrapper::createWbfsIso(source, target, saveId));
                if(patchWiimmfi) {
                    await(ExeWrapper::patchWiimmfi(target));
                }

            }
        } else if (command == "download-tools") {
            // --- discard ---
            setupSubcommand(parser, "download-tools", "Download the external tools that CSMM requires.");

            forceOption.setDescription("Force re-download of the tools even if they are already downloaded.");
            parser.addOption(forceOption);
            parser.addOption(witUrlOption);
            parser.addOption(wszstUrlOption);

            parser.process(arguments);

            if(parser.isSet(helpOption)) {
                cout << '\n' << parser.helpText();
            } else {
                auto force = parser.isSet(forceOption);

                if(force || !DownloadTools::requiredFilesAvailable()) {
                    QNetworkAccessManager manager(QCoreApplication::instance());

                    auto fut = DownloadTools::downloadAllRequiredFiles(&manager, [&](const QString &error) {
                        cout << error;
                        cout.flush();
                    }, parser.value(witUrlOption), parser.value(wszstUrlOption));
                    fut = AsyncFuture::observe(fut).subscribe([&]() {
                        cout << "Successfuly downloaded and extracted the tools at:" << Qt::endl;
                        cout << DownloadTools::getToolsLocation().path() << Qt::endl;
                        cout.flush();
                    }).future();
                    await(fut);
                } else {
                    cout << "Required tools already available at:" << Qt::endl;
                    cout << DownloadTools::getToolsLocation().path() << Qt::endl;
                    cout.flush();
                }
            }
        } else {
            cout << description;
            cout << parser.helpText();
            cout << commandsDescription;
        }
    } catch (const std::runtime_error &error) {
        cout << error.what() << Qt::endl;
        exit(1);
    }
}

}
