#include "mapdescriptiontable.h"
#include "lib/powerpcasm.h"
#include "lib/fslocale.h"
#include "lib/datafileset.h"
#include <fstream>
#include <filesystem>
#include <yaml-cpp/yaml.h>

// TODO add configuration for adding author to desription

quint32 MapDescriptionTable::writeTable(const std::vector<MapDescriptor> &descriptors) {
    QVector<quint32> table;
    for (auto &descriptor: descriptors) table.append(descriptor.descMsgId);
    return allocate(table, "MapDescriptionTable");
}

void MapDescriptionTable::writeAsm(QDataStream &stream, const AddressMapper &addressMapper, const std::vector<MapDescriptor> &mapDescriptors) {
    short tableRowCount = (short)mapDescriptors.size();
    quint32 mapDescriptionTableAddr = writeTable(mapDescriptors);
    PowerPcAsm::Pair16Bit v = PowerPcAsm::make16bitValuePair(mapDescriptionTableAddr);
    // HACK: Expand the description message ID table
    // subi r3,r3,0x15                                     -> nop
    stream.device()->seek(addressMapper.boomToFileAddress(0x8021214c)); stream << PowerPcAsm::nop();
    // cmpwi r3,0x12                                       -> cmpwi r3,tableRowCount
    stream.device()->seek(addressMapper.boomToFileAddress(0x80212158)); stream << PowerPcAsm::cmpwi(3, tableRowCount);
    // r4 <- 0x80436bc0                                    -> r4 <- mapDescriptionTableAddr
    stream.device()->seek(addressMapper.boomToFileAddress(0x80212164)); stream << PowerPcAsm::lis(4, v.upper);
    stream.skipRawData(4); stream << PowerPcAsm::addi(4, 4, v.lower);
}

void MapDescriptionTable::readAsm(QDataStream &stream, std::vector<MapDescriptor> &mapDescriptors, const AddressMapper &, bool isVanilla) {
    if (isVanilla) {
        readVanillaTable(stream, mapDescriptors);
    } else {
        for (auto &mapDescriptor: mapDescriptors) {
            stream >> mapDescriptor.descMsgId;
        }
    }
}

quint32 MapDescriptionTable::readTableAddr(QDataStream &stream, const AddressMapper &addressMapper, bool) {
    stream.device()->seek(addressMapper.boomToFileAddress(0x80212164));
    quint32 lisOpcode, dummy, addiOpcode;
    stream >> lisOpcode >> dummy >> addiOpcode;
    return PowerPcAsm::make32bitValueFromPair(lisOpcode, addiOpcode);
}

qint16 MapDescriptionTable::readTableRowCount(QDataStream &stream, const AddressMapper &addressMapper, bool) {
    stream.device()->seek(addressMapper.boomToFileAddress(0x80212158));
    quint32 opcode; stream >> opcode;
    return PowerPcAsm::getOpcodeParameter(opcode);
}

bool MapDescriptionTable::readIsVanilla(QDataStream &stream, const AddressMapper &addressMapper) {
    stream.device()->seek(addressMapper.boomToFileAddress(0x8021214c));
    quint32 opcode; stream >> opcode;
    // subi r3,r3,0x15
    return opcode == PowerPcAsm::subi(3, 3, 0x15);
}

void MapDescriptionTable::readVanillaTable(QDataStream &stream, std::vector<MapDescriptor> &mapDescriptors) {
    quint32 descMsgIdTable[18];
    for (auto &elem: descMsgIdTable) {
        stream >> elem;
    }
    int j = 0;
    for (auto &mapDescriptor: mapDescriptors) {
        if (mapDescriptor.unlockId < 18) {
            mapDescriptor.descMsgId = descMsgIdTable[j];
            ++j;
            if (j == 18) {
                j = 0;
            }
        }
    }
}

QMap<QString, UiMessageInterface::LoadMessagesFunction> MapDescriptionTable::loadUiMessages() {
    QMap<QString, UiMessageInterface::LoadMessagesFunction> result;
    for (auto &locale: FS_LOCALES) {
        if (locale == "uk") continue;
        result[uiMessageCsv(locale)] = [&](const QString &, GameInstance &instance, const ModListType &, const UiMessage *messages) {
            for (auto &descriptor: instance.mapDescriptors()) {
                descriptor.descs[locale] = messages->at(descriptor.descMsgId);
            }
        };
    }
    return result;
}

QMap<QString, UiMessageInterface::SaveMessagesFunction> MapDescriptionTable::freeUiMessages() {
    QMap<QString, UiMessageInterface::SaveMessagesFunction> result;
    for (auto &locale: FS_LOCALES) {
        result[uiMessageCsv(locale)] = [&](const QString &, GameInstance &gameInstance, const ModListType &, UiMessage *messages) {
            for (auto &descriptor: gameInstance.mapDescriptors()) {
                messages->erase(descriptor.descMsgId);
            }
        };
    }
    return result;
}

void MapDescriptionTable::allocateUiMessages(const QString &, GameInstance &gameInstance, const ModListType &) {
    for (auto &descriptor: gameInstance.mapDescriptors()) {
        descriptor.descMsgId = gameInstance.nextUiMessageId();
    }
}

QMap<QString, UiMessageInterface::SaveMessagesFunction> MapDescriptionTable::saveUiMessages() {
    QMap<QString, UiMessageInterface::SaveMessagesFunction> result;
    for (auto &locale: FS_LOCALES) {
        result[uiMessageCsv(locale)] = [&](const QString &, GameInstance &instance, const ModListType &, UiMessage *messages) {
            QString theLocale = locale == "uk" ? "en" : locale;
            bool addAuthorToDesc = true;
            if (!modpackDir().isEmpty()) {
                auto cfg = QDir(modpackDir()).filePath("mapDescriptionTable.yaml");
                if (QFileInfo::exists(cfg)) {
                    std::ifstream cfgStream(std::filesystem::path(cfg.toStdU16String()));
                    auto node = YAML::Load(cfgStream);
                    auto addAuthorToDescNode = node["addAuthor"];
                    addAuthorToDesc = !addAuthorToDescNode.IsDefined() || addAuthorToDescNode.as<bool>();
                }
            }
            for (auto &descriptor: instance.mapDescriptors()) {
                (*messages)[descriptor.descMsgId] = descriptor.descs[theLocale] + (addAuthorToDesc && !descriptor.authors.empty()
                                                                                ? " [" + QStringList(descriptor.authors.begin(), descriptor.authors.end()).join(", ") + "]"
                                                                                : "");
            }
        };
    }
    return result;
}

