#include "riivolution.h"
#include "lib/datafileset.h"
#include "qcoreapplication.h"

#include <QXmlStreamWriter>
#include <QDirIterator>

namespace Riivolution {

bool validateRiivolutionName(const QString &riivolutionName) {
    return !riivolutionName.isEmpty() && !riivolutionName.contains('/') && !riivolutionName.contains('\\')
            && riivolutionName != "riivolution";
}

static bool fileContentsEqual(const QString &filePath0, const QString &filePath1) {
    QFileInfo fileInfo0(filePath0), fileInfo1(filePath1);
    if (!fileInfo0.exists()) {
        return !fileInfo1.exists();
    }
    if (fileInfo0.size() != fileInfo1.size()) {
        return false;
    }
    QFile file0(filePath0), file1(filePath1);
    if (!file0.open(QFile::ReadOnly)) {
        throw Exception(QObject::tr( "could not open %1 for inspection").arg(filePath0));
    }
    if (!file1.open(QFile::ReadOnly)) {
        throw Exception(QObject::tr( "could not open %1 for inspection").arg(filePath1));
    }
    char buffer0[4096], buffer1[4096];
    while (!file0.atEnd() && !file1.atEnd()) {
        auto numRead0 = file0.read(buffer0, 4096);
        auto numRead1 = file1.read(buffer1, 4096);
        if (numRead0 != numRead1 || memcmp(buffer0, buffer1, numRead0) != 0) {
            return false;
        }
    }
    return true;
}

void write(const QDir &vanilla, const QDir &fullPatchDir, const AddressMapper &addressMapper, const QString &riivolutionName) {
    const char *discId;
    switch (addressMapper.getVersion()) {
    case GameVersion::BOOM:
        discId = "ST7P";
        break;
    case GameVersion::FORTUNE:
        discId = "ST7E";
        break;
    case GameVersion::ITADAKI:
        discId = "ST7J";
        break;
    }

    if (!fullPatchDir.mkdir("riivolution")) {
        throw Exception(QString(QObject::tr( "could not create riivolution dir in %1")).arg(fullPatchDir.path()));
    }

    QFile riivolutionFile(fullPatchDir.filePath("riivolution/" + riivolutionName +".xml"));
    if (!riivolutionFile.open(QFile::WriteOnly)) {
        throw Exception(QString(QObject::tr( "could not create riivolution.xml for writing")));
    }
    QXmlStreamWriter xmlWriter(&riivolutionFile);

    xmlWriter.setAutoFormatting(true);
    xmlWriter.writeStartDocument();

    // write root element
    xmlWriter.writeStartElement("wiidisc");
    xmlWriter.writeAttribute("version", "1");

    // restrict id
    xmlWriter.writeEmptyElement("id");
    xmlWriter.writeAttribute("game", discId);

    // write options
    xmlWriter.writeStartElement("options");
    xmlWriter.writeStartElement("section");
    xmlWriter.writeAttribute("name", riivolutionName);
    xmlWriter.writeStartElement("option");
    xmlWriter.writeAttribute("name", riivolutionName);
    xmlWriter.writeStartElement("choice");
    xmlWriter.writeAttribute("name", "Enabled");
    xmlWriter.writeEmptyElement("patch");
    xmlWriter.writeAttribute("id", "csmm");
    xmlWriter.writeEndElement();
    xmlWriter.writeEndElement();
    xmlWriter.writeEndElement();
    xmlWriter.writeEndElement();

    // write the patch
    xmlWriter.writeStartElement("patch");
    xmlWriter.writeAttribute("id", "csmm");
    xmlWriter.writeAttribute("root", "/" + riivolutionName);

    // patch files folder
    xmlWriter.writeEmptyElement("folder");
    xmlWriter.writeAttribute("external", "files");
    xmlWriter.writeAttribute("disc", "/");
    xmlWriter.writeAttribute("create", "true");

    // patch main dol
    {
        QFile vanillaMainDol(vanilla.filePath(MAIN_DOL)),
                patchedMainDol(fullPatchDir.filePath(riivolutionName + "/" + MAIN_DOL));
        if (!vanillaMainDol.open(QFile::ReadOnly)) {
            throw Exception(QString(QObject::tr( "couldn't open vanilla main dol for reading")));
        }
        if (!patchedMainDol.open(QFile::ReadOnly)) {
            throw Exception(QString(QObject::tr( "couldn't open patched main dol for reading")));
        }
        QVector<QPair<quint32, QByteArray>> memoryValues;
        char vanillaOp, patchedOp;
        while (vanillaMainDol.getChar(&vanillaOp) && patchedMainDol.getChar(&patchedOp)) {
            if (vanillaOp != patchedOp) {
                auto fileAddr = patchedMainDol.pos() - 1;
                auto memoryAddr = addressMapper.fileAddressToStandardVirtualAddress(fileAddr);
                if (memoryAddr != -1) {
                    // attempt to compress runs of changed memory
                    if (!memoryValues.empty()
                            && memoryAddr - memoryValues.back().first == memoryValues.back().second.size()) {
                        memoryValues.back().second.push_back(patchedOp);
                    } else {
                        memoryValues.push_back({memoryAddr, QByteArray(1, patchedOp)});
                    }
                }
            }
        }
        for (auto &memoryRun: memoryValues) {
            xmlWriter.writeEmptyElement("memory");
            xmlWriter.writeAttribute("offset", "0x" + QString::number(memoryRun.first, 16));
            xmlWriter.writeAttribute("value", memoryRun.second.toHex());
        }
    }

    xmlWriter.writeEndElement();

    xmlWriter.writeEndElement();

    xmlWriter.writeEndDocument();

    // purge unneeded files
    // first purge non-file directories
    for (auto &fileInfo: QDir(fullPatchDir.filePath(riivolutionName)).entryInfoList(
             QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot)) {
        if (fileInfo.fileName() != "files") {
            if (fileInfo.isDir()) {
                QDir(fileInfo.filePath()).removeRecursively();
            } else {
                QFile::remove(fileInfo.filePath());
            }
        }
    }

    // then purge unchanged files
    QDirIterator dirIt(fullPatchDir.filePath(riivolutionName + "/files"),
                       QDir::Files | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    while (dirIt.hasNext()) {
        auto patchedFilePath = dirIt.next();
        auto vanillaFilePath = vanilla.filePath("files/" + QDir(dirIt.path()).relativeFilePath(patchedFilePath));
        if (fileContentsEqual(vanillaFilePath, patchedFilePath)) {
            QFile::remove(patchedFilePath);
        }
    }
}

}
