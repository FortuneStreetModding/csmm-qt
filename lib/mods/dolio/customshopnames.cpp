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
    auto hijackAddr = addressMapper.boomStreetToStandard(0x800f89d4);
    stream.device()->seek(addressMapper.toFileAddress(hijackAddr));
    // addi r4, r30, 0x14d4 -> b msgIdSubroutine
    stream << PowerPcAsm::b(hijackAddr, msgIdSubroutineAddr);

    hijackAddr = addressMapper.boomStreetToStandard(0x8010cd74);
    stream.device()->seek(addressMapper.toFileAddress(hijackAddr));
    // bl get_shop_name -> bl getShopName
    stream << PowerPcAsm::bl(hijackAddr, addressMapper.boomStreetToStandard(0x800f8924));
}

void CustomShopNames::readAsm(QDataStream &stream, std::vector<MapDescriptor> &mapDescriptors, const AddressMapper &addressMapper, bool isVanilla)
{
    for (auto &descriptor: mapDescriptors) {
        if (isVanilla) {
            descriptor.shopNameStartId = 5333;
        } else {
            stream >> descriptor.shopNameStartId;
        }
    }
}

bool CustomShopNames::readIsVanilla(QDataStream &stream, const AddressMapper &addressMapper)
{
    stream.device()->seek(addressMapper.boomToFileAddress(0x800f89d4));
    quint32 opcode; stream >> opcode;
    return opcode == PowerPcAsm::addi(4, 30, 0x14d4);
}

quint32 CustomShopNames::readTableAddr(QDataStream &stream, const AddressMapper &addressMapper, bool isVanilla)
{
    return tableAddr;
}

quint32 CustomShopNames::writeTable(const std::vector<MapDescriptor> &descriptors)
{
    QVector<quint32> table;
    for (auto &descriptor: descriptors) {
        table.append(descriptor.shopNameStartId);
    }
    return allocate(table, "Shop Name StartID Table");
}

QVector<quint32> CustomShopNames::getMsgIdSubroutine(const AddressMapper &mapper, quint32 routineStartAddr)
{
    auto v = PowerPcAsm::make16bitValuePair(tableAddr);
    auto returnAddr = mapper.boomStreetToStandard(0x800f89d8);

    // r30 = shop id

    QVector<quint32> res;
    auto labels = PowerPcAsm::LabelTable();
    res.push_back(PowerPcAsm::addi(4, 30, 0x14d4));                          // r4 <- r30 + 0x14d4
    res.push_back(PowerPcAsm::cmpwi(30, 0));                                 // if (r30 > 0)
    res.push_back(PowerPcAsm::ble(labels, "Return", res));                   // {
    res.push_back(PowerPcAsm::mr(31, 3));                                    //   r31 <- r3
    res.push_back(PowerPcAsm::bl(routineStartAddr, res.size(),
                                 mapper.boomStreetToStandard(0x800113f0)));  //   r3 <- Flag::Volatile::GetGameSelectInfo()
    res.push_back(PowerPcAsm::lwz(3, 0x214, 3));                             //   r3 <- r3->mapId
    res.push_back(PowerPcAsm::lis(4, v.upper));                              //   r4 <- tableAddr
    res.push_back(PowerPcAsm::addi(4, 4, v.lower));                          //
    res.push_back(PowerPcAsm::mulli(3, 3, 4));                               //   r3 <- r3 * 4
    res.push_back(PowerPcAsm::lwzx(4, 4, 3));                                //   r4 <- r4[r3]   (r4 is shopNameStartId now)
    res.push_back(PowerPcAsm::add(4, 30, 4));                                //   r4 <- r30 + r4
    res.push_back(PowerPcAsm::subi(4, 4, 1));                                //   r4 <- r4 - 1
    res.push_back(PowerPcAsm::mr(3, 31));                                    //   r3 <- r31
    labels.define("Return", res);                                            // }
    res.push_back(PowerPcAsm::b(routineStartAddr, res.size(), returnAddr));  // return

    return res;
}


QMap<QString, UiMessageInterface::LoadMessagesFunction> CustomShopNames::loadUiMessages()
{
    QMap<QString, UiMessageInterface::LoadMessagesFunction> result;
    for (auto &locale : FS_LOCALES) {
        if (locale == "uk") continue;
        result[uiMessageCsv(locale)] = [=](const QString &, GameInstance *gameInstance,
                const ModListType &, const UiMessage *messages) {
            for (auto &descriptor: gameInstance->mapDescriptors()) {
                auto &shopNames = descriptor.shopNames[locale];
                shopNames.clear();
                for (int i=0; i<100; i++) {
                    shopNames.push_back((*messages).at(descriptor.shopNameStartId+i));
                }
            }
        };
    }
    return result;
}

void CustomShopNames::allocateUiMessages(const QString &root, GameInstance *gameInstance, const ModListType &modList)
{
    // reuse shop name start id if they are all equivalent in all languages
    std::map< std::map<QString, std::vector<QString>> , int> uiMessageIdReuse;
    for (auto &descriptor : gameInstance->mapDescriptors()) {
        // Check if the shopNames are already in the uiMessageIdReuse map
        auto it = uiMessageIdReuse.find(descriptor.shopNames);
        if (it != uiMessageIdReuse.end()) {
            // If found, reuse the shopNameStartId
            descriptor.shopNameStartId = it->second;
        } else {
            // Otherwise, generate a new shopNameStartId (and allocate the next 99 message ids) and add it to the map
            for(int i=0;i<100;i++) {
                int newId = gameInstance->nextUiMessageId();
                if(i==0) {
                    descriptor.shopNameStartId = newId;
                    uiMessageIdReuse[descriptor.shopNames] = newId;
                }
            }
        }
    }
}

QMap<QString, UiMessageInterface::SaveMessagesFunction> CustomShopNames::saveUiMessages()
{
    QMap<QString, UiMessageInterface::SaveMessagesFunction> result;
    for (auto &locale : FS_LOCALES) {
        result[uiMessageCsv(locale)] = [=](const QString &, GameInstance *gameInstance,
                const ModListType &, UiMessage *messages) {
            auto theLocale = locale == "uk" ? "en" : locale;
            for (auto &descriptor: gameInstance->mapDescriptors()) {
                auto &shopNames = retrieveStr(descriptor.shopNames, theLocale);
                for (int i=0; i<100; ++i) {
                    (*messages)[descriptor.shopNameStartId+i] = shopNames[i];
                }
            }
            // Rephrase some messages in the English locale so that uppercase shop name fits.
            // The other locales' phrasing seem to fit already so no need to touch them.
            if(locale == "uk" || locale == "en") {
                (*messages)[3037] = "<en>'s bid of <auc_bid><g> for “<sp>” is accepted!";
                (*messages)[3131] = "The deal is a success! “<sp>” is sold to <en>!";
                (*messages)[3132] = "The deal is a success! “<sp>” is bought from <en>!";
                (*messages)[3724] = "The value of “<sp>” increases by 20%!";
                (*messages)[3730] = "The value of “<sp>” increases by 30%!";
                (*messages)[3753] = "The value of “<sp>” increases by 50%!";
                (*messages)[3756] = "The value of “<sp>” increases by 75%!";
            }
        };
    }
    return result;
}


void CustomShopNames::loadFiles(const QString &root, GameInstance *gameInstance, const ModListType &modList)
{
    QFile addrFile(QDir(root).filePath(tableAddrFileName()));
    if (addrFile.open(QFile::ReadOnly)) {
        QDataStream stream(&addrFile);
        stream >> tableAddr;
    }
    DolIOTable::loadFiles(root, gameInstance, modList);
}

void CustomShopNames::saveFiles(const QString &root, GameInstance *gameInstance, const ModListType &modList)
{
    DolIOTable::saveFiles(root, gameInstance, modList);
    QFile addrFile(QDir(root).filePath(tableAddrFileName()));
    if (addrFile.open(QFile::WriteOnly)) {
        QDataStream stream(&addrFile);
        stream << tableAddr;
    }
}
