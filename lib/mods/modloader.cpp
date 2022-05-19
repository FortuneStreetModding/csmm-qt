#include "modloader.h"
#include "defaultmodlist.h"
#include "lib/python/pythonbindings.h"
#include "lib/zip/zip.h"

namespace ModLoader {

static ModListType importModpackDir(const QString &modpackDir) {
    ModListType result(DefaultModList::defaultModList());

    QFile modListFile(QDir(modpackDir).filePath("modlist.txt"));
    if (!modListFile.open(QIODevice::ReadOnly)) {
        throw ModException("could not open modlist.txt");
    }

    QTextStream modListStream(&modListFile);
    modListStream.setCodec("UTF-8");
    QString modid;

    QSet<QString> modids;

    while (modListStream.readLineInto(&modid)) {
        modids.insert(modid);
    }

    // filter out default mods that aren't from the modlist.txt
    auto removeIterator = std::remove_if(result.begin(), result.end(), [&](const auto &mod) { return !modids.contains(mod->modId()); });
    // remove modids from modid set that are already accounted for
    for (auto &mod: result) {
        modids.remove(mod->modId());
    }
    // do the actual removing
    result.erase(removeIterator, result.end());

    // temporarily add modpack folder to python module search path
    auto sys = pybind11::module_::import("sys");
    auto copy = pybind11::module_::import("copy");
    pybind11::list sysPathCopy(copy.attr("copy")(sys.attr("path")));
    using namespace pybind11::literals;
    sys.attr("path").attr("insert")(0, modpackDir.toUtf8().data());

    try {
        for (auto &modid: qAsConst(modids)) {
            qDebug() << "trying to import user mod" << modid;

            // append mod instance for each user modid
            bool wasModuleThereBefore = sys.attr("modules").contains(modid);
            auto modModule = pybind11::module_::import(modid.toUtf8());
            if (wasModuleThereBefore) modModule.reload();
            result.append(CSMMModHolder::fromPyObj(modModule.attr("mod")));

            qDebug() << "successfully imported user mod" << result.back()->modId();
        }
    } catch (const pybind11::error_already_set &error) {
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
