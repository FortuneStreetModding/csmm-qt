#include "stagenameidtable.h"
#include "lib/powerpcasm.h"
#include "lib/fslocale.h"
#include "lib/datafileset.h"

quint32 StageNameIDTable::writeTable(const std::vector<MapDescriptor> &descriptors) {
    QVector<quint32> table;
    for (auto &descriptor: descriptors) table.append(descriptor.nameMsgId);
    return allocate(table, "StageNameIDTable");
}

void StageNameIDTable::writeAsm(QDataStream &stream, const AddressMapper &addressMapper, const std::vector<MapDescriptor> &mapDescriptors) {
    quint32 tableAddr = writeTable(mapDescriptors);
    PowerPcAsm::Pair16Bit v = PowerPcAsm::make16bitValuePair(tableAddr);

    // --- Update Table Addr ---
    // mulli r0,r3,0x38 ->  mulli r0,r3,0x04
    stream.device()->seek(addressMapper.boomToFileAddress(0x801cca6c)); stream << PowerPcAsm::mulli(0, 3, 0x04);
    // r3 <- 0x80428e50 ->  r3 <- tableAddr
    stream.device()->seek(addressMapper.boomToFileAddress(0x801cca70)); stream << PowerPcAsm::lis(3, v.upper) << PowerPcAsm::addi(3, 3, v.lower);
}

void StageNameIDTable::readAsm(QDataStream &stream, std::vector<MapDescriptor> &mapDescriptors, const AddressMapper &, bool isVanilla) {
    if (isVanilla) {
        stream.skipRawData(0x0);
    }
    for (auto &mapDescriptor: mapDescriptors) {
        stream >> mapDescriptor.nameMsgId;
        if (isVanilla) {
            // in vanilla main.dol the table has other stuff in it like bgm id, map frb files, etc.
            // this we need to skip to go the next target amount in the table
            stream.skipRawData(0x38 - 0x04);
        }
    }
}

quint32 StageNameIDTable::readTableAddr(QDataStream &stream, const AddressMapper &addressMapper, bool) {
    stream.device()->seek(addressMapper.boomToFileAddress(0x801cca70));
    quint32 lisOpcode, addiOpcode;
    stream >> lisOpcode >> addiOpcode;
    return PowerPcAsm::make32bitValueFromPair(lisOpcode, addiOpcode);
}

qint16 StageNameIDTable::readTableRowCount(QDataStream &, const AddressMapper &, bool) {
    return -1;
}

bool StageNameIDTable::readIsVanilla(QDataStream &stream, const AddressMapper &addressMapper) {
    stream.device()->seek(addressMapper.boomToFileAddress(0x801cca6c));
    quint32 opcode; stream >> opcode;
    // mulli r0,r3,0x38
    return opcode == PowerPcAsm::mulli(0, 3, 0x38);
}

QMap<QString, UiMessageInterface::LoadMessagesFunction> StageNameIDTable::loadUiMessages() {
    QMap<QString, UiMessageInterface::LoadMessagesFunction> result;
    for (auto &locale: FS_LOCALES) {
        if (locale == "uk") continue;
        result[uiMessageCsv(locale)] = [&](const QString &, GameInstance &instance, const ModListType &, const UiMessage *messages) {
            for (auto &descriptor: instance.mapDescriptors()) {
                descriptor.names[locale] = messages->at(descriptor.nameMsgId);
            }
        };
    }
    return result;
}

void StageNameIDTable::allocateUiMessages(const QString &, GameInstance &gameInstance, const ModListType &) {
    for (auto &descriptor: gameInstance.mapDescriptors()) {
        descriptor.nameMsgId = gameInstance.nextUiMessageId();
    }
}

QMap<QString, UiMessageInterface::SaveMessagesFunction> StageNameIDTable::saveUiMessages() {
    QMap<QString, UiMessageInterface::SaveMessagesFunction> result;
    for (auto &locale: FS_LOCALES) {
        result[uiMessageCsv(locale)] = [&](const QString &, GameInstance &instance, const ModListType &, UiMessage *messages) {
            QString theLocale = locale;
            if (locale == "uk") {
                theLocale = "en";
            }
            for (auto &descriptor: instance.mapDescriptors()) {
                (*messages)[descriptor.nameMsgId] = retrieveStr(descriptor.names, theLocale);
            }
        };
    }
    return result;
}

