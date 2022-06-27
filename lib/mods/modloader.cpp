#include "modloader.h"
#include "defaultmodlist.h"
#include "lib/python/pythonbindings.h"
#include "lib/zip/zip.h"

namespace ModLoader {

static const QRegularExpression modListSplit("\\s+|\\b", QRegularExpression::UseUnicodePropertiesOption);

static QSet<QString> parseModListFile(QTextStream &stream, const ModListType &defaultModList) {
    QSet<QString> result;
    for (auto &mod: defaultModList) {
        result.insert(mod->modId());
    }
    int isRelative = -1;
    QString line;
    while (stream.readLineInto(&line)) {
        int commentIdx = line.indexOf("#");
        if (commentIdx < 0) commentIdx = line.size();
        auto splitLine = line.left(commentIdx).split(modListSplit, Qt::SkipEmptyParts);
        if (!splitLine.empty()) {
            bool isCurLineRelative = (splitLine[0] == "+" || splitLine[0] == "-");
            if (isRelative < 0) {
                isRelative = isCurLineRelative;
                if (!isRelative) {
                    result.clear();
                }
            } else if (isRelative != isCurLineRelative) {
                throw ModException(QString("mixing absolute and relative modid specifiers; offending line: %1").arg(line));
            }
            if (splitLine.size() != (isRelative ? 2 : 1)) {
                throw ModException(QString("extraneous tokens; offending line: %1").arg(line));
            }
            if (isRelative && splitLine[0] == "-") {
                if (!result.contains(splitLine.back())) {
                    throw ModException(QString("mod to remove doesn't exist; offending line: %1").arg(line));
                }
                result.remove(splitLine.back());
            } else {
                if (result.contains(splitLine.front())) {
                    throw ModException(QString("mod to add is duplicated; offending line: %1").arg(line));
                }
                result.insert(splitLine.back());
            }
        }
    }
    return result;
}

static ModListType importModpackDir(const QString &modpackDir) {
    ModListType result(DefaultModList::defaultModList());

    QFile modListFile(QDir(modpackDir).filePath("modlist.txt"));
    if (!modListFile.open(QIODevice::ReadOnly)) {
        throw ModException("could not open modlist.txt");
    }

    QTextStream modListStream(&modListFile);
    modListStream.setCodec("UTF-8");

    QSet<QString> modids = parseModListFile(modListStream, result);

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
    sys.attr("path").attr("insert")(0, modpackDir);

    try {
        for (auto &modid: qAsConst(modids)) {
            qInfo() << "trying to import user mod" << modid;

            // append mod instance for each user modid
            bool wasModuleThereBefore = sys.attr("modules").contains(modid);
            auto modModule = pybind11::module_::import(modid.toUtf8());
            if (wasModuleThereBefore) modModule.reload();
            result.append(CSMMModHolder::fromPyObj(modModule.attr("mod")));

            qInfo() << "successfully imported user mod" << result.back()->modId();
        }
    } catch (const std::runtime_error &error) {
        sys.attr("path") = sysPathCopy;
        throw error;
    }

    //pybind11::print(sysPathCopy, "file"_a=sys.attr("stderr"));

    sys.attr("path") = sysPathCopy;

    for (auto &mod: result) {
        mod->setModpackDir(modpackDir);
    }

    return result;
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
            throw ModException("could not create temporary directory");
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
            throw ModException("zip file could not be extracted");
        }
        dir = QFileInfo(extractedFile).dir().path();
    } else {
        dir = QFileInfo(file).dir().path();
    }

    return make_pair(ModLoader::importModpackDir(dir), tmpDir);
}

}
