#include "nameddistricts.h"
#include "lib/powerpcasm.h"
#include "lib/datafileset.h"
#include "lib/fslocale.h"
#include "lib/uigame013.h"
#include "lib/uigame052.h"
#include "lib/vanilladatabase.h"

void NamedDistricts::writeAsm(QDataStream &stream, const AddressMapper &addressMapper, const std::vector<MapDescriptor> &mapDescriptors) {
    quint32 tableAddr = writeTable(mapDescriptors);
    PowerPcAsm::Pair16Bit v = PowerPcAsm::make16bitValuePair(tableAddr);

    stream.device()->seek(addressMapper.boomToFileAddress(0x800f8458));
    // cmplwi r4,0x15    ->    cmplwi r4,len(descriptors)
    stream << PowerPcAsm::cmplwi(4, mapDescriptors.size());

    stream.device()->seek(addressMapper.boomToFileAddress(0x800f8484));
    // r29 <- 0x80417460    ->    r29 <- tableAddr
    stream << PowerPcAsm::lis(29, v.upper);
    stream.skipRawData(4);
    stream << PowerPcAsm::addi(29, 29, v.lower);
}

quint32 NamedDistricts::writeTable(const std::vector<MapDescriptor> &descriptors) {
    QVector<quint32> table;
    for (auto &desc: descriptors) {
        table.append(desc.districtNameIds[0]);
        table.append(desc.districtNameIds.size());
    }
    return allocate(table, "district localization table");
}

bool NamedDistricts::readIsVanilla(QDataStream &stream, const AddressMapper &addressMapper) {
    stream.device()->seek(addressMapper.boomToFileAddress(0x800f8458));
    quint32 opcode; stream >> opcode;
    return opcode == PowerPcAsm::cmplwi(4, 0x15);
}

quint32 NamedDistricts::readTableAddr(QDataStream &stream, const AddressMapper &addressMapper, bool)  {
    stream.device()->seek(addressMapper.boomToFileAddress(0x800f8484));
    quint32 lisOpcode, dummy, addiOpcode;
    stream >> lisOpcode >> dummy >> addiOpcode;
    return PowerPcAsm::make32bitValueFromPair(lisOpcode, addiOpcode);
}

void NamedDistricts::readAsm(QDataStream &stream, std::vector<MapDescriptor> &mapDescriptors, const AddressMapper &, bool isVanilla) {
    if (!isVanilla) {
        for (auto &mapDescriptor: mapDescriptors) {
            quint32 startVal, tableLen;
            stream >> startVal >> tableLen;
            //qDebug() << startVal << tableLen;
            mapDescriptor.districtNameIds.clear();
            for (quint32 i=startVal; i<startVal+tableLen; ++i) {
                mapDescriptor.districtNameIds.push_back(i);
            }
        }
    } else {
        for (int i=0; i<0x15; ++i) {
            quint32 startVal, tableLen;
            stream >> startVal >> tableLen;
            // loop map descriptors by mod 21
            for (int j = i; j < mapDescriptors.size(); j += 0x15) {
                mapDescriptors[j].districtNameIds.clear();
                for (quint32 k=startVal; k<startVal+tableLen; ++k) {
                    mapDescriptors[j].districtNameIds.push_back(k);
                }
            }
        }
    }
}

static inline QRegularExpression re(const QString &regexStr) {
    return QRegularExpression(regexStr,
                              QRegularExpression::CaseInsensitiveOption | QRegularExpression::UseUnicodePropertiesOption);
}

QMap<QString, UiMessageInterface::LoadMessagesFunction> NamedDistricts::loadUiMessages() {
    QMap<QString, UiMessageInterface::LoadMessagesFunction> result;
    for (auto &locale: FS_LOCALES) {
        if (locale == "uk") continue;
        result[uiMessageCsv(locale)] = [&](const QString &, GameInstance *instance, const ModListType &, const UiMessage *messages) {
            for (auto &descriptor: instance->mapDescriptors()) {
                descriptor.districtNames[locale].clear();
                for (int distNameId: std::as_const(descriptor.districtNameIds)) {
                    auto distName = messages->at(distNameId);
                    // add district word in front of vanilla district names
                    if (5454 <= distNameId && distNameId <= 5760) {
                        auto distWord = VanillaDatabase::localeToDistrictWord().at(locale);
                        distWord.replace("\\s", " ");
                        distName = distWord + distName;
                    }
                    descriptor.districtNames[locale].push_back(distName);
                }
            }
        };
    }
    return result;
}

void NamedDistricts::allocateUiMessages(const QString &, GameInstance *gameInstance, const ModListType &) {
    for (auto &descriptor: gameInstance->mapDescriptors()) {
        descriptor.districtNameIds.clear();
        for (int i=0; i<descriptor.districtNames["en"].size(); ++i) {
            int newId = gameInstance->nextUiMessageId();
            descriptor.districtNameIds.push_back(newId);
        }
    }
}

QMap<QString, UiMessageInterface::SaveMessagesFunction> NamedDistricts::saveUiMessages() {
    QMap<QString, UiMessageInterface::SaveMessagesFunction> result;
    for (auto &locale: FS_LOCALES) {
        result[uiMessageCsv(locale)] = [&](const QString &, GameInstance *instance, const ModListType &, UiMessage *messages) {
                QString theLocale = locale;
                if (locale == "uk") {
                    theLocale = "en";
                }

                auto districtWord = VanillaDatabase::localeToDistrictWord().at(theLocale);
                auto districtReplaceRegex = re(districtWord + "<area>");

                for (auto it=messages->begin(); it!=messages->end(); ++it) {
                    // text replace District <area> -> <area>
                    it->second.replace(districtReplaceRegex, "<area>");
                    if (theLocale == "it") {
                        it->second.replace("Quartiere<outline_off><n><outline_0><area>", "<area>", Qt::CaseInsensitive);
                    }

                    // strip "District" from shop squares in view board
                    if (it->first == 2781) {
                        it->second = "";
                    }

                    // TODO find out what 2963 does if anything at all
                }

                for (auto &descriptor: instance->mapDescriptors()) {
                    for (int i=0; i<descriptor.districtNameIds.size(); ++i) {
                        (*messages)[descriptor.districtNameIds[i]]
                                = (i < descriptor.districtNames[theLocale].size()
                                   ? descriptor.districtNames[theLocale][i]
                                     : descriptor.districtNames["en"][i]);
                    }
                }
            };
    }
    return result;
}



QMap<QString, ArcFileInterface::ModifyArcFunction> NamedDistricts::modifyArcFile()
{
    QMap<QString, ArcFileInterface::ModifyArcFunction> result;
    for (auto &locale: FS_LOCALES) {
        result[gameBoardArc(locale)] = [&](const QString &, GameInstance *, const ModListType &, const QString &tmpDir) {
            auto brlytFile = QDir(tmpDir).filePath("arc/blyt/ui_game_013.brlyt");
            Ui_game_013::widenDistrictName(brlytFile);

            brlytFile = QDir(tmpDir).filePath("arc/blyt/ui_game_052.brlyt");
            Ui_game_052::widenDistrictName(brlytFile);
        };
    }
    return result;
}
