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
        quint32 startingVal;
        if (tableAddr) {
            stream >> startingVal;
        } else {
            startingVal = (isCapital ? 0x14d4 : 0x1468);
        }
        auto &l10nIds = (isCapital ? descriptor.capitalShopNameIds : descriptor.shopNameIds);
        l10nIds.clear();
        for (int i=1; i<=100; ++i) {
            l10nIds.push_back(startingVal + i);
        }
    }
}

bool CustomShopNames::readIsVanilla(QDataStream &stream, const AddressMapper &addressMapper)
{
    stream.device()->seek(addressMapper.boomToFileAddress(isCapital ? 0x800f89d4 : 0x800f865c));
    quint32 opcode; stream >> opcode;
    return opcode == PowerPcAsm::addi(4, 30, isCapital ? 0x14d4 : 0x1468);
}

qint16 CustomShopNames::readTableRowCount(QDataStream &stream, const AddressMapper &addressMapper, bool isVanilla)
{
    return -1;
}

quint32 CustomShopNames::readTableAddr(QDataStream &stream, const AddressMapper &addressMapper, bool isVanilla)
{
    return tableAddr;
}

quint32 CustomShopNames::writeTable(const std::vector<MapDescriptor> &descriptors)
{
    QVector<quint32> table;
    for (auto &descriptor: descriptors) {
        table.push_back((isCapital ? descriptor.capitalShopNameIds : descriptor.shopNameIds)[0] - 1);
    }
    return allocate(table, "Shop Name ID Table");
}

QVector<quint32> CustomShopNames::getMsgIdSubroutine(const AddressMapper &mapper, quint32 routineStartAddr)
{
    auto v = PowerPcAsm::make16bitValuePair(tableAddr);

    QVector<quint32> res;
    res.push_back(PowerPcAsm::li(4, isCapital ? 0x14d4 : 0x1468));           // r4 <- 0x14d4
    res.push_back(PowerPcAsm::cmpwi(30, 0));                                 // if (r30 > 0)
    res.push_back(PowerPcAsm::ble(9));                                       // {
    res.push_back(PowerPcAsm::mr(31, 3));                                    //   r31 <- r3
    res.push_back(PowerPcAsm::bl(routineStartAddr, res.size(),
                                 mapper.boomStreetToStandard(0x800113f0)));  //   r3 <- Flag::Volatile::GetGameSelectInfo()
    res.push_back(PowerPcAsm::lwz(3, 0x214, 3));                             //   r3 <- r3->mapId
    res.push_back(PowerPcAsm::lis(4, v.upper));                              //   r4 <- tableAddr
    res.push_back(PowerPcAsm::addi(4, 4, v.lower));                          //
    res.push_back(PowerPcAsm::mulli(3, 3, 4));                               //   r3 <- r3 * 4
    res.push_back(PowerPcAsm::lwzx(4, 4, 3));                                //   r4 <- tableAddr[r3]
    res.push_back(PowerPcAsm::mr(3, 31));                                    //   r3 <- r31
                                                                             // }
    res.push_back(PowerPcAsm::add(4, 4, 30));                                // r4 <- r4 + r30
    res.push_back(PowerPcAsm::b(routineStartAddr, res.size(),
                                mapper.boomStreetToStandard(isCapital ? 0x800f89d8 : 0x800f8660)));  // return

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
    for (auto &descriptor : gameInstance.mapDescriptors()) {
        auto &shopNameIds = (isCapital ? descriptor.capitalShopNameIds : descriptor.shopNameIds);
        shopNameIds.clear();
        for (int i=0; i<100; ++i) {
             shopNameIds.push_back(gameInstance.nextUiMessageId());
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
                auto &shopNames = (isCapital ? descriptor.capitalShopNames : descriptor.shopNames)[theLocale];
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
