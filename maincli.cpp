#include "maincli.h"

#include <QBuffer>
#include <QTimer>
#include <QCommandLineParser>
#include <QTemporaryDir>

#include "lib/await.h"
#include "lib/exewrapper.h"
#include "lib/importexportutils.h"
#include "lib/configuration.h"
#include "lib/riivolution.h"
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

static bool _isQuiet;
static bool _isVerbose;
static std::unique_ptr<QTextStream> coutp;
static std::unique_ptr<QTextStream> cerrp;

static QBuffer placeholderBuffer;

void run(QStringList arguments)
{
    auto description = QString(R"(
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

)").arg(CSMM_VERSION);

    QCommandLineParser parser;

    auto commandsDescription = QString(R"(
  extract         Extract a Fortune Street game disc image to a directory.
  export          Export one or several map descriptor files (*.yaml) from a Fortune Street game directory.
  import          Add importing a map descriptor file (*.yaml) into a Fortune Street game directory as a pending change.
  status          Check the pending changes in a Fortune Street game directory.
  discard         Discard the pending changes in a Fortune Street game directory.
  save            Save the pending changes in a Fortune Street game directory.
  pack            Pack a Fortune Street game directory to a disc image (pending changes must be saved prior).
  default-modlist Output a list of default mod ids
  riivolution     Create a Riivolution patch file from vanilla and patched game folders (WARNING: modifies the patched game folder)
  bsdiff          Create a .bsdiff file
  bspatch         Patch an existing file using a .bsdiff file
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
    QCommandLineOption verboseOption(QStringList() << "v" << "verbose", "Print extended information to console.");
    QCommandLineOption markerCodeOption(QStringList() << "markerCode", "Set the marker code for the iso/wbfs file. It can be any value between 00-ZZ using any digits or uppercase ASCII letters. The original game uses 01. Default is 02.", "markerCode");
    QCommandLineOption separateSaveGameOption(QStringList() << "separateSaveGame", "If set, the game will use a separate save game file instead of relying on the original save game.");
    QCommandLineOption mapZoneOption(QStringList() << "z" << "zone", "The <zone> of the map. 0=Super Mario Tour, 1=Dragon Quest Tour, 2=Special Tour.", "zone");
    QCommandLineOption modPackOption(QStringList() << "modpack", "The modpack file (.zip or modlist.txt) to load (leave blank for default).", "modpack");
    QCommandLineOption mapDescriptorConfigurationOption(QStringList() << "descCfg" << "descriptorCfg" << "descConfiguration" << "descriptorConfiguration", "The map description configuration .csv to use for saving instead of the default.", "descCfg", "");
    QCommandLineOption helpOption(QStringList() << "h" << "?" << "help", "Show the help");
    // add some generic options
    parser.addOption(helpOption);
    parser.addOption(quietOption);
    parser.addOption(verboseOption);
    quietOption.setDescription(verboseOption.description()); // add a linebreak at the last generic option

    // Call parse()
    parser.parse(arguments);

    placeholderBuffer.open(QIODevice::ReadWrite);
    coutp = parser.isSet(quietOption)
            ? std::make_unique<QTextStream>(&placeholderBuffer) : std::make_unique<QTextStream>(stdout);
    auto &cout = *coutp;
    cerrp = parser.isSet(quietOption)
            ? std::make_unique<QTextStream>(&placeholderBuffer) : std::make_unique<QTextStream>(stderr);
    auto &cerr = *cerrp;
    QTextStream &helpStream = parser.isSet(helpOption) ? cout : cerr;

    atexit([]() { // hack: manually set these to nullptr to prevent weird segfaults on macos
        coutp = nullptr;
        cerrp = nullptr;
    });

    _isQuiet = parser.isSet(quietOption);
    _isVerbose = parser.isSet(verboseOption);

    // add logging handler to deal with quiet/verbose options
    qInstallMessageHandler([](QtMsgType type, const QMessageLogContext &context, const QString &msg) {
        if (!_isQuiet && (_isVerbose || (type != QtMsgType::QtDebugMsg && type != QtMsgType::QtWarningMsg))) {
            *cerrp << msg << Qt::endl;
        }
    });

    pybind11::scoped_interpreter guard{};

    try {
        const QStringList args = parser.positionalArguments();
        const QString command = args.isEmpty() ? QString() : args.first();

        if (command.isEmpty()) {
            helpStream << description;
            helpStream << parser.helpText();
            helpStream << commandsDescription;
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
                helpStream << '\n' << parser.helpText();
            } else {
                const QString source = args.at(1);
                QFileInfo sourceFileInfo(source);
                if(!sourceFileInfo.exists()) {
                    qCritical() << source << "does not exist.";
                    exit(1);
                } else if(!sourceFileInfo.isFile()) {
                    qCritical() << source << "is not a file.";
                    exit(1);
                }
                const QString target = args.size() >= 3 ? args.at(2) : QDir::current().filePath(sourceFileInfo.baseName());
                const QDir targetDir(target);
                qInfo() << "Extracting" << source << "to" << target << "...";
                if(targetDir.exists()) {
                    if(parser.isSet(forceOption)) {
                        qInfo() << "Overwriting" << target << "...";
                    } else {
                        qCritical() << "Cannot extract as" << target << "already exists. Use force option to overwrite.";
                        exit(1);
                    }
                } else {
                    targetDir.mkpath(".");
                }
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
                helpStream << '\n' << parser.helpText();
            } else {
                const QString source = args.at(1);
                const QDir sourceDir(source);
                if(!sourceDir.exists()) {
                    qCritical() << source << "does not exist.";
                    exit(1);
                }
                const QString target = args.size() >= 3? args.at(2) : QDir::current().path();
                QDir targetDir(target);
                if(!targetDir.exists()) {
                    targetDir.mkpath(".");
                }
                auto gameInstance = GameInstance::fromGameDirectory(sourceDir.path(), "");
                auto mods = ModLoader::importModpackFile(parser.value(modPackOption));
                CSMMModpack modpack(gameInstance, mods.first.begin(), mods.first.end());
                modpack.load(sourceDir.path());
                auto descriptors = gameInstance.mapDescriptors();
                if (descriptors.empty()) {
                    qCritical() << " no maps were found in " << source;
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
                        qInfo() << "Exporting" << descriptor.internalName << "to" << targetPath << "...";
                        ImportExportUtils::exportYaml(sourceDir, targetPath, descriptor);
                        atLeastOneMatch = true;
                    }
                }
                if(!atLeastOneMatch) {
                    qCritical() << "No maps matched. Try providing --id or --name.";
                    helpStream << '\n' << parser.helpText();
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
                helpStream << '\n' << parser.helpText();
            } else {
                const QString source = args.at(1);
                const QDir sourceDir(source);
                if(!sourceDir.exists()) {
                    qCritical() << source << "does not exist.";
                    exit(1);
                }
                const QString yaml = args.at(2);
                const QFile yamlFile(yaml);
                if(!yamlFile.exists()) {
                    qCritical() << yaml << "does not exist.";
                    exit(1);
                }
                QFile file(sourceDir.filePath("csmm_pending_changes.yaml"));
                if(!file.exists()) {
                    auto gameInstance = GameInstance::fromGameDirectory(sourceDir.path(), "");
                    auto mods = ModLoader::importModpackFile(parser.value(modPackOption));
                    CSMMModpack modpack(gameInstance, mods.first.begin(), mods.first.end());
                    modpack.load(sourceDir.path());
                    auto descriptors = gameInstance.mapDescriptors();
                    if(descriptors.empty()) {
                        qCritical() << " no maps were found in " << source;
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
                helpStream << '\n' << parser.helpText();
            } else {
                const QString source = args.at(1);
                const QDir sourceDir(source);
                if(!sourceDir.exists()) {
                    qCritical() << source << "does not exist.";
                    exit(1);
                }
                QFile file(sourceDir.filePath("csmm_pending_changes.yaml"));
                if(file.exists()) {
                    cout << Configuration::status(sourceDir.filePath("csmm_pending_changes.yaml"));
                } else {
                    auto gameInstance = GameInstance::fromGameDirectory(sourceDir.path(), "");
                    auto mods = ModLoader::importModpackFile(parser.value(modPackOption));
                    CSMMModpack modpack(gameInstance, mods.first.begin(), mods.first.end());
                    modpack.load(sourceDir.path());
                    auto descriptors = gameInstance.mapDescriptors();
                    if (descriptors.empty()) {
                        qCritical() << " no maps were found in " << source;
                        exit(1);
                    }
                    cout << Configuration::status(descriptors, sourceDir.filePath("csmm_pending_changes.yaml"));
                    qInfo() << "There are no pending changes";
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
                helpStream << '\n' << parser.helpText();
            } else {
                const QString source = args.at(1);
                const QDir sourceDir(source);
                if(!sourceDir.exists()) {
                    qCritical() << source << "does not exist.";
                    exit(1);
                }
                auto cfgPath = parser.value(mapDescriptorConfigurationOption);
                if (cfgPath.isEmpty()) {
                    cfgPath = sourceDir.filePath("csmm_pending_changes.yaml");
                }
                if (QFile::exists(cfgPath)) {
                    QTemporaryDir importDir;
                    if (!importDir.isValid()) {
                        qCritical() << "Could not create temporary directory for importing descriptors." << importDir.errorString();
                        exit(1);
                    }
                    auto gameInstance = GameInstance::fromGameDirectory(sourceDir.path(), importDir.path());
                    auto mods = ModLoader::importModpackFile(parser.value(modPackOption));
                    CSMMModpack modpack(gameInstance, mods.first.begin(), mods.first.end());
                    modpack.load(sourceDir.path());
                    auto &descriptors = gameInstance.mapDescriptors();
                    try {
                        Configuration::load(cfgPath, descriptors, importDir.path());

                        if (std::any_of(mods.first.begin(), mods.first.end(), [](auto &mod) { return mod->modId() == "wifiFix"; })) {
                            qInfo() << "**> The game will be saved with Wiimmfi text replacing WFC. Wiimmfi will only be patched after packing it to a wbfs/iso using csmm pack command.";
                        }
                        modpack.save(sourceDir.path());

                        qInfo() << "Pending changes have been saved";
                    } catch (const std::runtime_error &exception) {
                        qCritical() << "Error loading the map:" << exception.what();
                        exit(1);
                    }
                } else {
                    qCritical() << "There are no pending changes to save. Run csmm import first.";
                }
            }
        } else if (command == "discard") {
            // --- discard ---
            setupSubcommand(parser, "discard", "Discard the pending changes.");
            parser.addPositionalArgument("gameDir", "Fortune Street game directory.", "discard <gameDir>");

            parser.process(arguments);

            const QStringList args = parser.positionalArguments();
            if(parser.isSet(helpOption) || args.size() < 2) {
                helpStream << '\n' << parser.helpText();
            } else {
                const QString source = args.at(1);
                const QDir sourceDir(source);
                if(!sourceDir.exists()) {
                    qCritical() << source << "does not exist.";
                    exit(1);
                }
                QFile file(sourceDir.filePath("csmm_pending_changes.yaml"));
                if(file.exists()) {
                    file.remove();
                    qInfo() << "Pending changes have been discarded";
                } else {
                    qCritical() << "There are no pending changes to discard";
                    exit(1);
                }
            }
        } else if (command == "riivolution") {
            setupSubcommand(parser, "riivolution", "Creates a Riivolution patch xml at <patchedDir>/../riivolution/<patchedDir name not including the path>.xml from the vanilla and patched game files.");
            parser.addPositionalArgument("vanillaDir", "Vanilla Fortune Street directory.", "pack <vanillaDir>");
            parser.addPositionalArgument("patchedDir", "Patched Fortune Street directory. (WARNING: modifies the patched game folder)", "<patchedDir>");

            parser.process(arguments);

            const QStringList args = parser.positionalArguments();
            if (parser.isSet(helpOption) || args.size() < 3) {
                helpStream << '\n' << parser.helpText();
            } else {
                const QString vanillaDir = args.at(1);
                const QString patchedDir = args.at(2);
                const QString patchName = QDir(patchedDir).dirName();
                if (!Riivolution::validateRiivolutionName(patchName)) {
                    qCritical() << "patch name" << patchName << "is invalid";
                    exit(1);
                }
                auto vanillaInst = GameInstance::fromGameDirectory(vanillaDir, "");
                auto patchedInst = GameInstance::fromGameDirectory(patchedDir, "");
                if (vanillaInst.addressMapper().getVersion() != patchedInst.addressMapper().getVersion()) {
                    qCritical() << "version mismatch between vanilla and patched roms";
                    exit(1);
                }
                Riivolution::write(vanillaDir, QFileInfo(patchedDir).dir(), vanillaInst.addressMapper(), patchName);
            }
        } else if (command == "pack") {
            // --- pack ---
            setupSubcommand(parser, "pack", "Pack the game into an iso/wbfs file.");
            parser.addPositionalArgument("gameDir", "Fortune Street game directory.", "pack <gameDir>");
            parser.addPositionalArgument("target", "Target filename.\n[default = <gameId6>.wbfs]", "[target]");

            parser.addOption(markerCodeOption);
            parser.addOption(separateSaveGameOption);
            parser.addOption(forceOption);

            parser.process(arguments);

            const QStringList args = parser.positionalArguments();
            if(parser.isSet(helpOption) || args.size() < 2) {
                helpStream << '\n' << parser.helpText();
            } else {
                const QString source = args.at(1);
                const QDir sourceDir(source);
                if(!sourceDir.exists()) {
                    qCritical() << source << "does not exist.";
                    exit(1);
                }

                QString markerCode = parser.isSet(markerCodeOption) ? parser.value(markerCodeOption) : "02";

                QString target;
                if(args.size() >= 3) {
                    target = args.at(2);
                } else {
                    QString id6 = await(ExeWrapper::getId6(source));
                    QString id4 = id6.remove(4,2);
                    target = QDir::current().filePath(id4 + markerCode + ".wbfs");
                }
                QFile targetFile(target);
                if(targetFile.exists()) {
                    if(parser.isSet(forceOption)) {
                        qInfo() << "Overwriting" << target << "...";
                    } else {
                        qCritical() << "Cannot extract as" << target << "already exists. Use force option to overwrite.";
                        exit(1);
                    }
                }
                QFileInfo targetInfo(target);

                QDir targetDir(targetInfo.dir());
                if(!targetDir.exists()) {
                    targetDir.mkpath(".");
                }

                qInfo() << "Creating" << targetInfo.suffix() << "file at" << target << "out from" << source << "...";

                bool patchWiimmfi = ImportExportUtils::hasWiimmfiText(sourceDir);
                await(ExeWrapper::createWbfsIso(source, target, markerCode, parser.isSet(separateSaveGameOption)));
                if(patchWiimmfi) {
                    await(ExeWrapper::patchWiimmfi(target));
                }

            }
        } else if (command == "bsdiff") {
            // --- bsdiff ---
            setupSubcommand(parser, "bsdiff", "Create a .bsdiff file");
            parser.addPositionalArgument("oldFile", "Old (original) file.", "bsdiff <gameDir>");
            parser.addPositionalArgument("newFile", "New (modified) file.", "<newFile>");
            parser.addPositionalArgument("bsdiffFile", "Resulting .bsdiff file.\n[default = <newFile>.bsdiff]", "[bsdiffFile]");

            parser.addOption(forceOption);

            parser.process(arguments);

            const QStringList args = parser.positionalArguments();
            if(parser.isSet(helpOption) || args.size() < 3) {
                helpStream << '\n' << parser.helpText();
            } else {
                const QString oldFileStr = args.at(1);
                const QFile oldFile = QFile(oldFileStr);
                const QFileInfo oldFileInfo = QFileInfo(oldFile);
                if(!oldFile.exists()) {
                    qCritical() << oldFileStr << "does not exist.";
                    exit(1);
                }
                const QString newFileStr = args.at(2);
                const QFile newFile = QFile(newFileStr);
                const QFileInfo newFileInfo(newFile);
                if(!newFile.exists()) {
                    qCritical() << newFileStr << "does not exist.";
                    exit(1);
                }
                QString bsdiffFileStr;
                if(args.size() >= 4) {
                    bsdiffFileStr = args.at(3);
                } else {
                    bsdiffFileStr = QDir(newFileInfo.absolutePath()).filePath(newFileInfo.baseName() + ".bsdiff");
                }
                QFile bsdiffFile(bsdiffFileStr);
                QFileInfo bsdiffFileInfo(bsdiffFile);
                if(bsdiffFile.exists()) {
                    if(parser.isSet(forceOption)) {
                        qInfo() << "Overwriting" << bsdiffFileInfo.filePath() << "...";
                    } else {
                        qCritical() << "Cannot create .bsdiff file as" << bsdiffFileInfo.filePath() << "already exists. Use force option to overwrite.";
                        exit(1);
                    }
                }

                QDir bsdiffFileDir(bsdiffFileInfo.dir());
                if(!bsdiffFileDir.exists()) {
                    bsdiffFileDir.mkpath(".");
                }

                qInfo() << "Creating" << bsdiffFileInfo.suffix() << "file at" << bsdiffFileInfo.filePath() << "out from" << oldFileInfo.filePath() << "...";

                QString errors = ImportExportUtils::createBsdiff(oldFileInfo.absoluteFilePath(), newFileInfo.absoluteFilePath(), bsdiffFileInfo.absoluteFilePath());
                if(!errors.isEmpty()) {
                    qCritical() << errors;
                    exit(1);
                }
            }
        } else if (command == "bspatch") {
            // --- bspatch ---
            setupSubcommand(parser, "bspatch", "Patch an existing file using a .bsdiff file");
            parser.addPositionalArgument("oldFile", "Old (original) file.", "bspatch <gameDir>");
            parser.addPositionalArgument("newFile", "New (patched) file.", "<newFile>");
            parser.addPositionalArgument("bsdiffFile", "Input .bsdiff file.", "<bsdiffFile>");
            parser.addOption(forceOption);

            parser.process(arguments);

            const QStringList args = parser.positionalArguments();
            if(parser.isSet(helpOption) || args.size() < 3) {
                helpStream << '\n' << parser.helpText();
            } else {
                const QString oldFileStr = args.at(1);
                const QFile oldFile = QFile(oldFileStr);
                const QFileInfo oldFileInfo = QFileInfo(oldFile);
                if(!oldFile.exists()) {
                    qCritical() << oldFileStr << "does not exist.";
                    exit(1);
                }
                const QString newFileStr = args.at(2);
                const QFile newFile = QFile(newFileStr);
                const QFileInfo newFileInfo(newFile);
                if(newFile.exists()) {
                    if(parser.isSet(forceOption)) {
                        qInfo() << "Overwriting" << newFileInfo.filePath() << "...";
                    } else {
                        qCritical() << "Cannot patch as" << newFileInfo.filePath() << "already exists. Use force option to overwrite.";
                        exit(1);
                    }
                }
                QString bsdiffFileStr = args.at(3);
                QFile bsdiffFile(bsdiffFileStr);
                QFileInfo bsdiffFileInfo(bsdiffFile);
                if(!bsdiffFile.exists()) {
                    qCritical() << newFileStr << "does not exist.";
                    exit(1);
                }

                QDir bsdiffFileDir(bsdiffFileInfo.dir());
                if(!bsdiffFileDir.exists()) {
                    bsdiffFileDir.mkpath(".");
                }

                qInfo() << "Applying patch" << bsdiffFileInfo.filePath() << "to" << oldFileInfo.filePath() << "and creating" << newFileInfo.filePath() <<"...";

                QString errors = ImportExportUtils::applyBspatch(oldFileInfo.absoluteFilePath(), newFileInfo.absoluteFilePath(), bsdiffFileInfo.absoluteFilePath());
                if(!errors.isEmpty()) {
                    qCritical() << errors;
                    exit(1);
                }
            }
        } else {
            helpStream << description;
            helpStream << parser.helpText();
            helpStream << commandsDescription;
        }
    } catch (const std::runtime_error &error) {
        qCritical() << error.what();
        exit(1);
    }
}

}
