#include "modloader.h"
#include "defaultmodlist.h"
#include "lib/python/pythonbindings.h"
#include "lib/zip/zip.h"

namespace ModLoader {

static const QRegularExpression modListSplit("\\s+|\\b", QRegularExpression::UseUnicodePropertiesOption);

struct ParsedModList {
    QSet<QString> added;
    QSet<QString> removed;
    qint8 relative;
};

static ParsedModList parseModListFile(QFile *file) {
    QTextStream stream(file);
    stream.setEncoding(QStringConverter::Utf8);

    ParsedModList result;
    result.relative = -1;
    QString line;
    while (stream.readLineInto(&line)) {
        int commentIdx = line.indexOf("#");
        if (commentIdx < 0) commentIdx = line.size();
        auto splitLine = line.left(commentIdx).split(modListSplit, Qt::SkipEmptyParts);
        if (!splitLine.empty()) {
            bool isCurLineRelative = (splitLine[0] == "+" || splitLine[0] == "-");
            if (result.relative < 0) {
                result.relative = isCurLineRelative;
            } else if (result.relative != isCurLineRelative) {
                throw ModException(QString("mixing absolute and relative modid specifiers; offending line: %1").arg(line));
            }
            if (splitLine.size() != (result.relative ? 2 : 1)) {
                throw ModException(QString("extraneous tokens; offending line: %1").arg(line));
            }
            if (result.relative && splitLine[0] == "-") {
                result.removed.insert(splitLine.back());
            } else {
                result.added.insert(splitLine.back());
            }
        }
    }
    return result;
}

/**
 * @brief applyParsedModLists Converts the parsed mod lists into a group of mod ids to patch.
 * @param modLists parsed mod lists
 * @param defaultModList list of csmm-default mods
 * @return set of mod ids
 */
static QSet<QString> applyParsedModLists(QVector<ParsedModList> &modLists, const ModListType &defaultModList) {
    QSet<QString> result;
    std::stable_sort(modLists.begin(), modLists.end(), [](const ParsedModList &A, const ParsedModList &B) { return A.relative < B.relative; });
    if (modLists.empty() || modLists.first().relative) {
        for (auto &mod: defaultModList) {
            result.insert(mod->modId());
        }
    }
    for (auto &modList: modLists) {
        result.unite(modList.added);
        for (auto &modId: qAsConst(modList.removed)) {
            if (!result.contains(modId)) {
                throw ModException(QString("mod to remove \"%1\" doesn't exist").arg(modId));
            }
        }
        result.subtract(modList.removed);
    }
    return result;
}

static ModListType importModpackDirs(const QVector<QString> &modpackDirs) {
    ModListType result(DefaultModList::defaultModList());

    QVector<ParsedModList> modLists;
    for (auto &modpackDir: modpackDirs) {
        QFile modListFile(QDir(modpackDir).filePath("modlist.txt"));
        if (!modListFile.open(QIODevice::ReadOnly)) {
            throw ModException("could not open modlist.txt");
        }
        modLists.append(parseModListFile(&modListFile));
    }
    QSet<QString> modids = applyParsedModLists(modLists, result);

    QVector<QString> defaultModIds;
    for (auto &mod: result) {
        defaultModIds.append(mod->modId());
    }

    // filter out default mods that aren't from the modlist.txt
    auto removeIterator = std::remove_if(result.begin(), result.end(), [&](const auto &mod) { return !modids.contains(mod->modId()); });
    // do the actual removing
    result.erase(removeIterator, result.end());

    // remove modids from modid set that are already accounted for by the default set of mods
    for (auto &modId: defaultModIds) {
        modids.remove(modId);
    }

    // temporarily add modpack folder to python module search path
    auto sys = pybind11::module_::import("sys");
    auto copy = pybind11::module_::import("copy");
    pybind11::list sysPathCopy(copy.attr("copy")(sys.attr("path")));
    using namespace pybind11::literals;
    sys.attr("path")[pybind11::slice(0, 0, 1)] = modpackDirs;
    //pybind11::print(sys.attr("path"), "file"_a = sys.attr("stderr"));

    try {
        for (auto &modid: qAsConst(modids)) {
            qInfo() << "trying to import user mod" << modid;

            // append mod instance for each user modid
            bool wasModuleThereBefore = sys.attr("modules").contains(modid);
            auto modModule = pybind11::module_::import(modid.toUtf8());
            if (wasModuleThereBefore) modModule.reload();
            auto modObjToAdd = CSMMModHolder::fromPyObj(modModule.attr("mod"));
            modObjToAdd->setModpackDir(QFileInfo(modModule.attr("__file__").cast<QString>()).dir().path());
            result.append(modObjToAdd);
            qInfo() << "successfully imported user mod" << result.back()->modId();
        }
    } catch (const std::runtime_error &error) {
        sys.attr("path") = sysPathCopy;
        throw error;
    }

    //pybind11::print(sysPathCopy, "file"_a=sys.attr("stderr"));

    sys.attr("path") = sysPathCopy;

    return result;
}

std::pair<ModListType, std::shared_ptr<QTemporaryDir[]>> importModpackCollection(const QVector<QString> &modlistColl) {
    if (modlistColl.isEmpty()) {
        return std::make_pair(DefaultModList::defaultModList(), nullptr);
    }

    std::shared_ptr<QTemporaryDir[]> tmpDirs(new QTemporaryDir[modlistColl.size()]);
    QVector<QString> modpackDirs;

    for (int i=0; i<modlistColl.size(); ++i) {
        QString extractedFile;

        if (!tmpDirs[i].isValid()) {
            throw ModException(QString("could not create temporary directory for mod list zip %1").arg(modlistColl[i]));
        }

        int zipResult = zip_extract(modlistColl[i].toUtf8(), tmpDirs[i].path().toUtf8(), [](const char *candidate, void *arg) {
            QFileInfo fi(candidate);
            QString *extractedFilePtr = (QString *)arg;

            if (fi.fileName() == "modlist.txt") {
                *extractedFilePtr = candidate;
            }

            return 0;
        }, &extractedFile);
        if (zipResult < 0) {
            throw ModException(QString("zip file %1 could not be extracted: %2").arg(modlistColl[i], zip_strerror(zipResult)));
        }
        modpackDirs.append(QFileInfo(extractedFile).dir().path());
    }

    return make_pair(ModLoader::importModpackDirs(modpackDirs), tmpDirs);
}

std::pair<ModListType, std::shared_ptr<QTemporaryDir>> importModpackFile(const QString &file) {
    if (file.isEmpty()) { // special case: empty string yields default modpack
        return std::make_pair(DefaultModList::defaultModList(), nullptr);
    }

    std::shared_ptr<QTemporaryDir> tmpDir;

    QString dir;

    if (QFileInfo(file).suffix() == "zip") {
        QString extractedFile;

        tmpDir = std::make_shared<QTemporaryDir>();
        if (!tmpDir->isValid()) {
            throw ModException(QString("could not create temporary directory to extract %1 to").arg(file));
        }

        int zipResult = zip_extract(file.toUtf8(), tmpDir->path().toUtf8(), [](const char *candidate, void *arg) {
            QFileInfo fi(candidate);
            QString *extractedFilePtr = (QString *)arg;

            if (fi.fileName() == "modlist.txt") {
                *extractedFilePtr = candidate;
            }

            return 0;
        }, &extractedFile);
        if (zipResult < 0) {
            throw ModException(QString("zip file %1 could not be extracted: %2").arg(file, zip_strerror(zipResult)));
        }
        dir = QFileInfo(extractedFile).dir().path();
    } else {
        dir = QFileInfo(file).dir().path();
    }

    return make_pair(ModLoader::importModpackDirs({dir}), tmpDir);
}

}
