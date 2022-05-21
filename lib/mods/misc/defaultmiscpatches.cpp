#include "defaultmiscpatches.h"
#include "lib/importexportutils.h"

void DefaultMiscPatches::loadFiles(const QString &, GameInstance &, const ModListType &)
{
    // crab nothing to do crab
}

void DefaultMiscPatches::saveFiles(const QString &root, GameInstance &, const ModListType &)
{
    QFile f(":/files/bspatches.yaml");
    if (f.open(QFile::ReadOnly)) {
        QString contents = f.readAll();
        auto yaml = YAML::Load(contents.toStdString());
        for (auto it=yaml.begin(); it!=yaml.end(); ++it) {
            QString vanillaRelPath = QString::fromStdString(it->first.as<std::string>());
            QString vanilla = QString::fromStdString(yaml[vanillaRelPath.toStdString()]["vanilla"].as<std::string>()).toLower();
            QString patched = QString::fromStdString(yaml[vanillaRelPath.toStdString()]["patched"].as<std::string>()).toLower();
            QString bsdiffPath = ":/" + vanillaRelPath + ".bsdiff";
            QString cmpresPath = QDir(root).filePath(vanillaRelPath);
            QString sha1 = ImportExportUtils::fileSha1(cmpresPath);
            if (sha1 == vanilla) {
                QString errors = ImportExportUtils::applyBspatch(cmpresPath, cmpresPath, bsdiffPath);
                if(!errors.isEmpty()) {
                    throw ModException(QString("Errors occured when applying %1 patch to file %2:\n%3").arg(bsdiffPath, cmpresPath, errors));
                } else {
                    qDebug() << "Patched: " << cmpresPath << " sha1: " << ImportExportUtils::fileSha1(cmpresPath);
                }
            } else if (sha1 == patched) {
                qDebug() << "Already patched: " << cmpresPath << " sha1: " << ImportExportUtils::fileSha1(cmpresPath);
            } else {
                qDebug() << "Not patched (unknown file): " << cmpresPath << " sha1: " << ImportExportUtils::fileSha1(cmpresPath);
            }
        }
    }
}
