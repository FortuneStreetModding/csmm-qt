#include "riivolution.h"
#include "lib/datafileset.h"

#include <QXmlStreamWriter>

namespace Riivolution {

bool validateRiivolutionName(const QString &riivolutionName) {
    return !riivolutionName.contains('/') && !riivolutionName.contains('\\')
            && riivolutionName != "riivolution";
}

void write(const QDir &vanilla, const QDir &fullPatchDir, const AddressMapper &addressMapper, const QString &discId, const QString &riivolutionName) {
    if (!fullPatchDir.mkdir("riivolution")) {
        throw Exception("could not create riivolution dir in " + fullPatchDir.path());
    }

    QFile riivolutionFile(fullPatchDir.filePath("riivolution/" + riivolutionName +".xml"));
    if (!riivolutionFile.open(QFile::WriteOnly)) {
        throw Exception("could not create riivolution.xml for writing");
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
            throw Exception("couldn't open vanilla main dol for reading");
        }
        if (!patchedMainDol.open(QFile::ReadOnly)) {
            throw Exception("couldn't open patched main dol for reading");
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
}

}
