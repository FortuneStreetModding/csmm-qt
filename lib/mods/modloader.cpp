#include "modloader.h"
#include "defaultmodlist.h"
#include "lib/python/pythonbindings.h"
#include "lib/zip/zip.h"

namespace ModLoader {

ModListType importModpackFile(const QString &file) {
    if (file.isEmpty()) { // special case: empty string yields default modpack
        return DefaultModList::defaultModList();
    }

    QTemporaryDir tmpDir;

    if (!tmpDir.isValid()) {
        throw ModException("could not create temporary directory");
    }

    QString dir;

    if (QFileInfo(file).suffix() == "zip") {
        QString extractedFile;
        int zipResult = zip_extract(file.toUtf8(), tmpDir.path().toUtf8(), [](const char *candidate, void *arg) {
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

    return ModLoader::importModpackDir(dir);
}

ModListType importModpackDir(const QString &modpackDir) {
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

    //qDebug() << "length 1: " << result.length();

    // filter out default mods that aren't from the modlist.txt
    auto removeIterator = std::remove_if(result.begin(), result.end(), [&](const auto &mod) { return !modids.contains(mod->modId()); });
    // remove modids from modid set that are already accounted for
    for (auto &mod: result) {
        modids.remove(mod->modId());
    }
    // do the actual removing
    result.erase(removeIterator, result.end());

    //qDebug() << "length 2: " << result.length();

    // temporarily add modpack folder to python module search path
    auto sys = pybind11::module_::import("sys");
    auto sysPath = sys.attr("path");
    sysPath.attr("insert")(0, modpackDir.toUtf8().data());

    try {
        for (auto &modid: qAsConst(modids)) {
            qDebug() << "trying to import user mod" << modid;

            // append mod instance for each user modid
            auto modModule = pybind11::module_::import(modid.toUtf8());
            result.append(CSMMModHolder::fromPyObj(modModule.attr("mod")));

            qDebug() << "successfully imported user mod" << result.back()->modId();
        }
    } catch (const pybind11::error_already_set &error) {
        sysPath.attr("pop")(0); // revert python module search path
        throw error;
    }

    sysPath.attr("pop")(0);

    //qDebug() << "length 3: " << result.length();

    return result;
}

}
