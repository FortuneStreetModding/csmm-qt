#include "customshopnames.h"

#include "lib/datafileset.h"
#include "lib/fslocale.h"
#include "lib/powerpcasm.h"

void CustomShopNames::writeAsm(QDataStream &stream, const AddressMapper &addressMapper, const std::vector<MapDescriptor> &mapDescriptors)
{
    tableAddr = writeTable(mapDescriptors);
    auto msgIdSubroutineAddr = allocate(getMsgIdSubroutine(addressMapper, 0), "Message ID Subroutine", false);
    auto msgIdSubroutine = getMsgIdSubroutine(addressMapper, msgIdSubroutineAddr);
    stream.device()->seek(addressMapper.toFileAddress(msgIdSubroutineAddr));
    for (auto word: msgIdSubroutine) stream << word;
    auto hijackAddr = addressMapper.boomStreetToStandard(isCapital ? 0x800f89d4 : 0x800f865c);
    stream.device()->seek(addressMapper.toFileAddress(hijackAddr));
    // addi r4, r30, 0x14d4 -> b msgIdSubroutine
    stream << PowerPcAsm::b(hijackAddr, msgIdSubroutineAddr);
}

void CustomShopNames::readAsm(QDataStream &stream, std::vector<MapDescriptor> &mapDescriptors, const AddressMapper &addressMapper, bool isVanilla)
{
    for (auto &descriptor: mapDescriptors) {
        auto &l10nIds = (isCapital ? descriptor.capitalShopNameIds : descriptor.shopNameIds);
        l10nIds.clear();
        if (tableAddr) {
            quint32 subTableAddr;
            stream >> subTableAddr;
            auto oldPos = stream.device()->pos();
            stream.device()->seek(addressMapper.toFileAddress(subTableAddr));
            for (int i=0; i<100; ++i) {
                quint32 l10nId; stream >> l10nId;
                l10nIds.push_back(l10nId);
            }
            stream.device()->seek(oldPos);
        } else {
            auto startingVal = (isCapital ? 0x14d4 : 0x1468) + 1;
            for (int i=0; i<100; ++i) {
                l10nIds.push_back(startingVal + i);
            }
        }
    }
}

bool CustomShopNames::readIsVanilla(QDataStream &stream, const AddressMapper &addressMapper)
{
    stream.device()->seek(addressMapper.boomToFileAddress(isCapital ? 0x800f89d4 : 0x800f865c));
    quint32 opcode; stream >> opcode;
    return opcode == PowerPcAsm::addi(4, 30, isCapital ? 0x14d4 : 0x1468);
}

quint32 CustomShopNames::readTableAddr(QDataStream &stream, const AddressMapper &addressMapper, bool isVanilla)
{
    return tableAddr;
}

quint32 CustomShopNames::writeTable(const std::vector<MapDescriptor> &descriptors)
{
    QVector<quint32> table;
    for (auto &descriptor: descriptors) {
        auto &shopNameIds = (isCapital ? descriptor.capitalShopNameIds : descriptor.shopNameIds);
        table.push_back(allocate(shopNameIds.begin(), shopNameIds.end(), "Shop Name ID Subtable"));
    }
    return allocate(table, "Shop Name ID Table");
}

QVector<quint32> CustomShopNames::getMsgIdSubroutine(const AddressMapper &mapper, quint32 routineStartAddr)
{
    auto v = PowerPcAsm::make16bitValuePair(tableAddr);
    auto returnAddr = mapper.boomStreetToStandard(isCapital ? 0x800f89d8 : 0x800f8660);

    QVector<quint32> res;
    res.push_back(PowerPcAsm::addi(4, 30, isCapital ? 0x14d4 : 0x1468));     // r4 <- r30 + 0x14d4
    res.push_back(PowerPcAsm::cmpwi(30, 0));                                 // if (r30 > 0)
    res.push_back(PowerPcAsm::ble(12));                                      // {
    res.push_back(PowerPcAsm::mr(31, 3));                                    //   r31 <- r3
    res.push_back(PowerPcAsm::bl(routineStartAddr, res.size(),
                                 mapper.boomStreetToStandard(0x800113f0)));  //   r3 <- Flag::Volatile::GetGameSelectInfo()
    res.push_back(PowerPcAsm::lwz(3, 0x214, 3));                             //   r3 <- r3->mapId
    res.push_back(PowerPcAsm::lis(4, v.upper));                              //   r4 <- tableAddr
    res.push_back(PowerPcAsm::addi(4, 4, v.lower));                          //
    res.push_back(PowerPcAsm::mulli(3, 3, 4));                               //   r3 <- r3 * 4
    res.push_back(PowerPcAsm::lwzx(4, 4, 3));                                //   r4 <- r4[r3]
    res.push_back(PowerPcAsm::mulli(3, 30, 4));                              //   r3 <- r30 * 4
    res.push_back(PowerPcAsm::subi(3, 3, 4));                                //   r3 <- r3 - 4
    res.push_back(PowerPcAsm::lwzx(4, 4, 3));                                //   r4 <- r4[r3]
    res.push_back(PowerPcAsm::mr(3, 31));                                    //   r3 <- r31
                                                                             // }
    res.push_back(PowerPcAsm::b(routineStartAddr, res.size(), returnAddr));  // return

    return res;
}


QMap<QString, UiMessageInterface::LoadMessagesFunction> CustomShopNames::loadUiMessages()
{
    QMap<QString, UiMessageInterface::LoadMessagesFunction> result;
    for (auto &locale : FS_LOCALES) {
        if (locale == "uk") continue;
        result[uiMessageCsv(locale)] = [=](const QString &, GameInstance &gameInstance,
                const ModListType &, const UiMessage *messages) {
            for (auto &descriptor: gameInstance.mapDescriptors()) {
                auto &shopNames = (isCapital ? descriptor.capitalShopNames : descriptor.shopNames)[locale];
                shopNames.clear();
                for (auto id : (isCapital ? descriptor.capitalShopNameIds : descriptor.shopNameIds)) {
                    shopNames.push_back((*messages).at(id));
                }
            }
        };
    }
    return result;
}

void CustomShopNames::allocateUiMessages(const QString &root, GameInstance &gameInstance, const ModListType &modList)
{
    // reuse shop name ids if they are equivalent in all languages
    std::map<std::vector<QString>, int> uiMessageIdReuse;
    for (auto &descriptor : gameInstance.mapDescriptors()) {
        auto &shopNameIds = (isCapital ? descriptor.capitalShopNameIds : descriptor.shopNameIds);
        auto &shopNames = (isCapital ? descriptor.capitalShopNames : descriptor.shopNames);
        shopNameIds.clear();
        for (int i=0; i<100; ++i) {
            std::vector<QString> reuse;
            for (auto &ent: shopNames) {
                reuse.push_back(ent.second[i]);
            }
            if (!uiMessageIdReuse.count(reuse)) {
                uiMessageIdReuse[reuse] = gameInstance.nextUiMessageId();
            }
            shopNameIds.push_back(uiMessageIdReuse[reuse]);
        }
    }
}

QMap<QString, UiMessageInterface::SaveMessagesFunction> CustomShopNames::saveUiMessages()
{
    QMap<QString, UiMessageInterface::SaveMessagesFunction> result;
    for (auto &locale : FS_LOCALES) {
        result[uiMessageCsv(locale)] = [=](const QString &, GameInstance &gameInstance,
                const ModListType &, UiMessage *messages) {
            auto theLocale = locale == "uk" ? "en" : locale;
            for (auto &descriptor: gameInstance.mapDescriptors()) {
                auto &shopNames = retrieveStr(isCapital ? descriptor.capitalShopNames : descriptor.shopNames, theLocale);
                auto &shopNameIds = (isCapital ? descriptor.capitalShopNameIds : descriptor.shopNameIds);
                for (int i=0; i<shopNameIds.size(); ++i) {
                    (*messages)[shopNameIds[i]] = shopNames[i];
                }
            }
        };
    }
    return result;
}


void CustomShopNames::loadFiles(const QString &root, GameInstance &gameInstance, const ModListType &modList)
{
    QFile addrFile(QDir(root).filePath(tableAddrFileName()));
    if (addrFile.open(QFile::ReadOnly)) {
        QDataStream stream(&addrFile);
        stream >> tableAddr;
    }
    DolIOTable::loadFiles(root, gameInstance, modList);
}

void CustomShopNames::saveFiles(const QString &root, GameInstance &gameInstance, const ModListType &modList)
{
    DolIOTable::saveFiles(root, gameInstance, modList);
    QFile addrFile(QDir(root).filePath(tableAddrFileName()));
    if (addrFile.open(QFile::WriteOnly)) {
        QDataStream stream(&addrFile);
        stream << tableAddr;
    }
}
