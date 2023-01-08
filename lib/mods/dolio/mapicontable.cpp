#include "mapicontable.h"
#include "lib/await.h"
#include "lib/datafileset.h"
#include "lib/exewrapper.h"
#include "lib/powerpcasm.h"
#include "lib/uimenu1900a.h"
#include "lib/vanilladatabase.h"

void MapIconTable::writeAsm(QDataStream &stream, const AddressMapper &addressMapper, const std::vector<MapDescriptor> &mapDescriptors) {
    auto mapIcons = writeIconStrings(mapDescriptors);
    QMap<QString, quint32> iconTableMap;
    quint32 iconTableAddr = writeIconTable(mapIcons, iconTableMap);
    quint32 mapIconPointerTable = writeMapIconPointerTable(mapDescriptors, iconTableMap);
    ushort iconCount = (ushort)iconTableMap.size();
    short tableRowCount = (short)mapDescriptors.size();

    PowerPcAsm::Pair16Bit v = PowerPcAsm::make16bitValuePair(iconTableAddr);
    PowerPcAsm::Pair16Bit w = PowerPcAsm::make16bitValuePair(mapIconPointerTable);
    // note: To add custom icons, the following files need to be editted as well:
    // - ui_menu_19_00a.brlyt within game_sequence.arc and within game_sequence_wifi.arc
    // - ui_menu_19_00a_Tag_*.brlan within game_sequence.arc and within game_sequence_wifi.arc

    // custom map icon hack (change it that way that it will call the GetMapDifficulty routine instead of the GetMapOrigin routine
    // the GetMapDifficulty routine is mostly unused by the game and we repurpose it to return the pointer to the pointer of the string of the map icon instead
    // then we go through all map icon pointer pointers and check if it is the same as the one retrieved. If it is then we make it visible, otherwise we set the visibility to false.
    quint32 GetMapDifficulty = addressMapper.boomStreetToStandard(0x80211da4);

    // bl GetMapOrigin                                     -> bl GetMapDifficulty
    quint32 offset = addressMapper.boomStreetToStandard(0x8021e77c);
    stream.device()->seek(addressMapper.toFileAddress(offset)); stream << PowerPcAsm::bl(offset, GetMapDifficulty);
    // cmpw r28,r30                                        -> cmpw r29,r30
    stream.device()->seek(addressMapper.boomToFileAddress(0x8021e790)); stream << PowerPcAsm::cmpw(29, 30);
    // cmplwi r28,0x12                                     -> cmplwi r28,iconCount
    stream.device()->seek(addressMapper.boomToFileAddress(0x8021e7c0)); stream << PowerPcAsm::cmplwi(28, iconCount);
    // bl GetMapOrigin                                     -> bl GetMapDifficulty
    offset = addressMapper.boomStreetToStandard(0x8021e8a4);
    stream.device()->seek(addressMapper.toFileAddress(offset)); stream << PowerPcAsm::bl(offset, GetMapDifficulty);
    // cmpw r29,r28                                        -> cmpw r30,r28
    stream.device()->seek(addressMapper.boomToFileAddress(0x8021e8b8)); stream << PowerPcAsm::cmpw(30, 28);
    // cmplwi r29,0x12                                     -> cmplwi r29,iconCount
    stream.device()->seek(addressMapper.boomToFileAddress(0x8021e8e8)); stream << PowerPcAsm::cmplwi(29, iconCount);
    // bl GetMapOrigin                                     -> bl GetMapDifficulty
    offset = addressMapper.boomStreetToStandard(0x8021e824);
    stream.device()->seek(addressMapper.toFileAddress(offset)); stream << PowerPcAsm::bl(offset, GetMapDifficulty);
    // cmplwi r28,0x12                                     -> cmplwi r28,iconCount
    stream.device()->seek(addressMapper.boomToFileAddress(0x8021e84c)); stream << PowerPcAsm::cmplwi(28, iconCount);
    // r29 <- 0x8047f5c0                                   -> r29 <- iconTableAddr
    stream.device()->seek(addressMapper.boomToFileAddress(0x8021e780)); stream << PowerPcAsm::lis(29, v.upper); stream.skipRawData(4); stream << PowerPcAsm::addi(29, 29, v.lower);
    // r30 <- 0x8047f5c0                                   -> r30 <- iconTableAddr
    stream.device()->seek(addressMapper.boomToFileAddress(0x8021e8a8)); stream << PowerPcAsm::lis(30, v.upper); stream.skipRawData(4); stream << PowerPcAsm::addi(30, 30, v.lower);
    // r30 <- 0x8047f5c0                                   -> r30 <- iconTableAddr
    stream.device()->seek(addressMapper.boomToFileAddress(0x8021e828)); stream << PowerPcAsm::lis(30, v.upper); stream.skipRawData(4); stream << PowerPcAsm::addi(30, 30, v.lower);
    // mr r3,r28                                           -> mr r3,r26
    stream.device()->seek(addressMapper.boomToFileAddress(0x8021e94c)); stream << PowerPcAsm::mr(3, 26);
    // mr r3,r28                                           -> mr r3,r26
    stream.device()->seek(addressMapper.boomToFileAddress(0x8021e968)); stream << PowerPcAsm::mr(3, 26);

    // Modify the GetMapDifficulty routine to retrieve the current map icon addr addr
    // subi r31,r3,0x15                                   ->  nop
    stream.device()->seek(addressMapper.boomToFileAddress(0x80211dc8)); stream << PowerPcAsm::nop();
    // cmpwi r31,0x12                                     ->  cmpwi r31,tableRowCount
    stream.device()->seek(addressMapper.boomToFileAddress(0x80211dd4)); stream << PowerPcAsm::cmpwi(31, tableRowCount);
    // li r3,0x15                                         ->  nop
    stream.device()->seek(addressMapper.boomToFileAddress(0x80211e4c)); stream << PowerPcAsm::nop();
    // mulli r4,r3,0x24                                   ->  mulli r4,r3,0x04
    stream.device()->seek(addressMapper.boomToFileAddress(0x80211e58)); stream << PowerPcAsm::mulli(4, 3, 0x04);
    // r3 <- 804363c8                                     ->  r3 <- mapIconPointerTable
    stream.device()->seek(addressMapper.boomToFileAddress(0x80211e5c)); stream << PowerPcAsm::lis(3, w.upper); stream << PowerPcAsm::addi(3, 3, w.lower);
    // mulli r0,r31,0x24                                  ->  mulli r0,r31,0x04
    stream.device()->seek(addressMapper.boomToFileAddress(0x80211e64)); stream << PowerPcAsm::mulli(0, 31, 0x04);
    // lwz r3,0x1c(r3)                                    ->  lwz r3,0x0(r3)
    stream.device()->seek(addressMapper.boomToFileAddress(0x80211e78)); stream << PowerPcAsm::lwz(3, 0x0, 3);

    // --- Hack to make icons invisible which do not have a map ---
    // -- Init Maps in the map array with -1 --
    auto subroutineInitMapIdsForMapIcons = allocate(writeSubroutineInitMapIdsForMapIcons(addressMapper, 0), "SubroutineInitMapIdsForMapIcons");
    stream.device()->seek(addressMapper.toFileAddress(subroutineInitMapIdsForMapIcons));
    auto insts = writeSubroutineInitMapIdsForMapIcons(addressMapper, subroutineInitMapIdsForMapIcons); // re-write the routine again since now we know where it is located in the main dol
    for (quint32 inst: qAsConst(insts)) stream << inst;
    // increase the array size
    // rlwinm r3,r16,0x2,0x0,0x1d                            -> r3,r16,0x3,0x0,0x1d
    stream.device()->seek(addressMapper.boomToFileAddress(0x80187794)); stream << PowerPcAsm::rlwinm(3, 16, 0x3, 0x0, 0x1d);
    quint32 hijackAddr = addressMapper.boomStreetToStandard(0x8018779c);
    // cmpwi r3,0x0                                          ->  bl subroutineInitMapIdsForMapIcons
    stream.device()->seek(addressMapper.toFileAddress(hijackAddr)); stream << PowerPcAsm::bl(hijackAddr, subroutineInitMapIdsForMapIcons);
    // mr r24,r3                                             ->  cmpwi r3,0x0
    stream << PowerPcAsm::cmpwi(3, 0);
    // increase the array size
    // rlwinm r3,r16,0x2,0x0,0x1d                            -> r3,r16,0x3,0x0,0x1d
    stream.device()->seek(addressMapper.boomToFileAddress(0x80187aa4)); stream << PowerPcAsm::rlwinm(3, 16, 0x3, 0x0, 0x1d);
    hijackAddr = addressMapper.boomStreetToStandard(0x80187aac);
    // cmpwi r3,0x0                                          ->  bl subroutineInitMapIdsForMapIcons
    stream.device()->seek(addressMapper.toFileAddress(hijackAddr)); stream << PowerPcAsm::bl(hijackAddr, subroutineInitMapIdsForMapIcons);
    // mr r24,r3                                             ->  cmpwi r3,0x0
    stream << PowerPcAsm::cmpwi(3, 0);
    // -- if a map id is -1, make the map icon invisible --
    hijackAddr = addressMapper.boomStreetToStandard(0x8021e73c);
    quint32 returnContinueAddr = addressMapper.boomStreetToStandard(0x8021e740);
    quint32 returnMakeInvisibleAddr = addressMapper.boomStreetToStandard(0x8021e808);
    quint32 subroutineMakeNoneMapIconsInvisible = allocate(writeSubroutineMakeNoneMapIconsInvisible(addressMapper, 0, returnContinueAddr, returnMakeInvisibleAddr), "SubroutineMakeNoneMapIconsInvisibleMultiplayer");
    stream.device()->seek(addressMapper.toFileAddress(subroutineMakeNoneMapIconsInvisible));
    insts = writeSubroutineMakeNoneMapIconsInvisible(addressMapper, subroutineMakeNoneMapIconsInvisible, returnContinueAddr, returnMakeInvisibleAddr); // re-write the routine again since now we know where it is located in the main dol
    for (quint32 inst: qAsConst(insts)) stream << inst;
    // lwz r0,0x184(r3)                                      ->  b subroutineMakeNoneMapIconsInvisible
    stream.device()->seek(addressMapper.toFileAddress(hijackAddr)); stream << PowerPcAsm::b(hijackAddr, subroutineMakeNoneMapIconsInvisible);

    // --- Various Map UI Fixes ---
    // -- if the map index is over the map array size, do not loop around to the first map index again --
    // ble 0x80187e1c                                        ->  b 0x80187e1c
    stream.device()->seek(addressMapper.boomToFileAddress(0x80187dfc)); stream << PowerPcAsm::b(8);
    // -- fix map selection going out of bounds in tour mode --
    // bne 0x80188258                                        ->  nop
    stream.device()->seek(addressMapper.boomToFileAddress(0x80188230)); stream << PowerPcAsm::nop();
    // -- fix out of array bounds error when opening tour mode and viewing the zones and having more than 6 maps in a zone ---
    // bl Game::GameSequenceDataAdapter::GetNumMapsInZone    -> li r3,0x6
    stream.device()->seek(addressMapper.boomToFileAddress(0x8021f880)); stream << PowerPcAsm::li(3, 0x6);
    // bl Game::GameSequenceDataAdapter::GetNumMapsInZone    -> li r3,0x6
    stream.device()->seek(addressMapper.boomToFileAddress(0x8021ff4c)); stream << PowerPcAsm::li(3, 0x6);
    // -- if the map id is -1, do not check if it has been unlocked or not ---
    hijackAddr = addressMapper.boomStreetToStandard(0x8021e570);
    returnContinueAddr = addressMapper.boomStreetToStandard(0x8021e574);
    quint32 returnSkipMapUnlockedCheck = addressMapper.boomStreetToStandard(0x8021e5a8);
    quint32 subroutineWriteSubroutineSkipMapUnlockCheck = allocate(writeSubroutineSkipMapUnlockCheck(0, returnContinueAddr, returnSkipMapUnlockedCheck), "SubroutineWriteSubroutineSkipMapUnlockCheck");
    stream.device()->seek(addressMapper.toFileAddress(subroutineWriteSubroutineSkipMapUnlockCheck));
    insts = writeSubroutineSkipMapUnlockCheck(subroutineWriteSubroutineSkipMapUnlockCheck, returnContinueAddr, returnSkipMapUnlockedCheck); // re-write the routine again since now we know where it is located in the main dol
    for (quint32 inst: qAsConst(insts)) stream << inst;
    // or r3,r26,r26                                      ->  b subroutineWriteSubroutineSkipMapUnlockCheck
    stream.device()->seek(addressMapper.toFileAddress(hijackAddr)); stream << PowerPcAsm::b(hijackAddr, subroutineWriteSubroutineSkipMapUnlockCheck);
}

void MapIconTable::readAsm(QDataStream &stream, std::vector<MapDescriptor> &mapDescriptors, const AddressMapper &addressMapper, bool isVanilla) {
    for (auto &mapDescriptor: mapDescriptors) {
        if (isVanilla) {
            // in vanilla there is a mapping of bgXXX to p_bg_XXX which we will assume here without actually reading what is inside the main.dol
            if (mapDescriptor.unlockId < 18) {
                QRegularExpression reg("\\d+");
                auto number = reg.match(mapDescriptor.background).captured();
                mapDescriptor.mapIcon = QString("p_bg_") + number;
            }
        } else {
            quint32 addr;
            stream >> addr;
            mapDescriptor.mapIcon = resolveAddressAddressToString(addr, stream, addressMapper);
        }
    }
}

quint32 MapIconTable::readTableAddr(QDataStream &stream, const AddressMapper &addressMapper, bool) {
    stream.device()->seek(addressMapper.boomToFileAddress(0x80211e5c));
    quint32 lisOpcode, addiOpcode;
    stream >> lisOpcode >> addiOpcode;
    return PowerPcAsm::make32bitValueFromPair(lisOpcode, addiOpcode);
}

bool MapIconTable::readIsVanilla(QDataStream &stream, const AddressMapper &addressMapper) {
    stream.device()->seek(addressMapper.boomToFileAddress(0x8021e790));
    quint32 opcode; stream >> opcode;
    return opcode == PowerPcAsm::cmpw(28, 30);
}

QMap<QString, quint32> MapIconTable::writeIconStrings(const std::vector<MapDescriptor> &mapDescriptors) {
    // Find out which map icons exist
    QSet<QString> allUniqueMapIcons;
    for (auto &mapDescriptor: mapDescriptors) {
        if (!mapDescriptor.mapIcon.isEmpty()) {
            allUniqueMapIcons.insert(mapDescriptor.mapIcon);
        }
    }
    // also add all vanilla map icons, since we do not remove them from the ui_menu file
    allUniqueMapIcons += VanillaDatabase::getVanillaIcons();

    // sort the unique map icons (not really needed, but helps in debugging the console output)
    QList<QString> allUniqueMapIconsSorted = allUniqueMapIcons.values();
    std::sort(allUniqueMapIconsSorted.begin(), allUniqueMapIconsSorted.end());

    // write each map icon into the main.dol and remember the location in the mapIcons dictionary
    QMap<QString, quint32> mapIcons;
    for (auto &mapIcon: allUniqueMapIconsSorted) {
        mapIcons[mapIcon] = allocate(mapIcon);
    }

    return mapIcons;
}

quint32 MapIconTable::writeIconTable(const QMap<QString, quint32> &mapIcons, QMap<QString, quint32> &iconTableMap) {
    // write the map icon lookup table and remember the location of each pointer in the mapIconLookupTable dictionary
    QMap<QString, int> iconTableOffsetMap;
    iconTableMap.clear();
    QVector<quint32> iconTable;
    int i = 0;
    for (auto it=mapIcons.begin(); it!=mapIcons.end(); ++it) {
        iconTable.append(it.value());
        iconTableOffsetMap[it.key()] = i;
        i += 4;
    }
    quint32 iconTableAddr = allocate(iconTable, "IconTable");
    for (auto it=iconTableOffsetMap.begin(); it!=iconTableOffsetMap.end(); ++it) {
        iconTableMap[it.key()] = iconTableAddr + it.value();
    }
    return iconTableAddr;
}

quint32 MapIconTable::writeMapIconPointerTable(const std::vector<MapDescriptor> &mapDescriptors, const QMap<QString, quint32> &iconTableMap) {
    QVector<quint32> mapIconTable;
    for (auto &mapDescriptor: mapDescriptors) {
        quint32 mapIconAddr = 0;
        if (!mapDescriptor.mapIcon.isEmpty()) {
            mapIconAddr = iconTableMap[mapDescriptor.mapIcon];
        }
        mapIconTable.append(mapIconAddr);
    }
    return allocate(mapIconTable, "MapIconPointerTable");
}

QString MapIconTable::resolveAddressAddressToString(quint32 virtualAddressAddress, QDataStream &stream, const AddressMapper &addressMapper) {
    if (!virtualAddressAddress) {
        return "";
    }
    int fileAddress = addressMapper.toFileAddress(virtualAddressAddress);
    qint64 pos = stream.device()->pos();
    stream.device()->seek(fileAddress);
    quint32 virtualAddress; stream >> virtualAddress;
    stream.device()->seek(pos);
    return resolveAddressToString(virtualAddress, stream, addressMapper);
}

QVector<quint32> MapIconTable::writeSubroutineInitMapIdsForMapIcons(const AddressMapper &addressMapper, quint32 entryAddr) {
    quint32 JUtility_memset = addressMapper.boomStreetToStandard(0x80004714);
    // precondition: r3 is newly created map icon array
    //               r16 is the amount of map ids in the array (size / 4)
    //               r24 is unused
    // postcondition: r24 is the map icon array
    return {
        PowerPcAsm::mflr(24),                                        // save the link register
        PowerPcAsm::li(4, -1),                                       // fill with 0xff
        PowerPcAsm::rlwinm(5, 16, 0x3, 0x0, 0x1d),                   // get the size of the array
        PowerPcAsm::bl(entryAddr, 3 /*asm.Count*/, JUtility_memset), // call JUtility_memset(array*, 0xff, array.size)
        PowerPcAsm::mtlr(24),                                        // restore the link register
        PowerPcAsm::mr(24, 3),                                       // move array* to r24
        PowerPcAsm::blr()                                            // return
    };
}

QVector<quint32> MapIconTable::writeSubroutineMakeNoneMapIconsInvisible(const AddressMapper &addressMapper, quint32 entryAddr, quint32 returnContinueAddr, quint32 returnMakeInvisibleAddr) {
    quint32 Scene_Layout_Obj_SetVisible = addressMapper.boomStreetToStandard(0x8006f854);
    // precondition:  r31  MapIconButton*
    //                 r5  is unused
    // postcondition:  r0  is map icon type
    //                 r5  is 0
    return {
        PowerPcAsm::lwz(5, 0x188, 31),                                          // get current map id into r5
        PowerPcAsm::cmpwi(5, -1),                                               // map id == -1 ?
        PowerPcAsm::bne(8),                                                     // {
        PowerPcAsm::lwz(3, 0x28, 31),                                           //   |
        PowerPcAsm::li(5, 0),                                                   //   |
        PowerPcAsm::lwz(4, -0x6600, 13),                                        //   | make "NEW" text invisible
        PowerPcAsm::bl(entryAddr, 6/*asm.Count*/, Scene_Layout_Obj_SetVisible), //   |
        PowerPcAsm::lwz(3, 0x28, 31),                                           //   |
        PowerPcAsm::li(5, 0),                                                   //   | make Locked Map Icon "(?)" invisible
        PowerPcAsm::b(entryAddr, 9/*asm.Count*/, returnMakeInvisibleAddr),      //   returnMakeInvisibleAddr
                                                                                // } else {
        PowerPcAsm::lwz(0, 0x184, 3),                                           //   get map icon type (replaced opcode)
        PowerPcAsm::b(entryAddr, 11/*asm.Count*/, returnContinueAddr)           //   returnContinueAddr
                                                                                // }
    };
}

QVector<quint32> MapIconTable::writeSubroutineSkipMapUnlockCheck(quint32 entryAddr, quint32 returnContinueAddr, quint32 returnSkipMapUnlockedCheckAddr) {
    // precondition:  r26  is mapid
    //                 r3  is unused
    // postcondition:  r3  is mapid
    return {
        PowerPcAsm::or_(3, 26, 26),                                                 // r3 <- mapid
        PowerPcAsm::cmpwi(3, -1),                                                   // mapid == -1
        PowerPcAsm::bne(2),                                                         // {
        PowerPcAsm::b(entryAddr, 3/*asm.Count*/, returnSkipMapUnlockedCheckAddr),   //   goto returnSkipMapUnlockedCheckAddr
        PowerPcAsm::b(entryAddr, 4/*asm.Count*/, returnContinueAddr),               // } else goto returnContinueAddr
    };
}

QMap<QString, ArcFileInterface::ModifyArcFunction> MapIconTable::modifyArcFile() {
    QMap<QString, ArcFileInterface::ModifyArcFunction> result;

    for (auto &locale: FS_LOCALES) {
        result[gameSequenceArc(locale)] = [&](const QString &root, GameInstance &gameInstance, const ModListType &, const QString &tmpDir) {
            QMap<QString, QString> mapIconToTplName;
            for (auto &mapDescriptor: gameInstance.mapDescriptors()) {
                if (mapDescriptor.mapIcon.isEmpty()) continue;
                if (VanillaDatabase::hasVanillaTpl(mapDescriptor.mapIcon)) {
                    mapIconToTplName[mapDescriptor.mapIcon] = VanillaDatabase::getVanillaTpl(mapDescriptor.mapIcon);
                } else {
                    auto tplName = Ui_menu_19_00a::constructMapIconTplName(mapDescriptor.mapIcon);
                    if (!mapIconToTplName.contains(mapDescriptor.mapIcon)) {
                        mapIconToTplName[mapDescriptor.mapIcon] = tplName;
                    }
                }
            }

            for (auto &mapDescriptor: gameInstance.mapDescriptors()) {
                if (mapDescriptor.mapIcon.isEmpty() || VanillaDatabase::hasVanillaTpl(mapDescriptor.mapIcon)) {
                    continue;
                }
                QString mapIconPng = QDir(gameInstance.getImportDir()).filePath(PARAM_FOLDER + "/" + mapDescriptor.mapIcon + ".png");
                QFileInfo mapIconPngInfo(mapIconPng);
                if (mapIconPngInfo.exists() && mapIconPngInfo.isFile()) {
                    auto mapIconTpl = QDir(tmpDir).filePath("arc/timg/" + mapIconToTplName[mapDescriptor.mapIcon]);
                    await(ExeWrapper::convertPngToTpl(mapIconPng, mapIconTpl));
                }
            }

            // convert the brlyt files to xmlyt, inject the map icons and convert it back
            auto brlytFile = QDir(tmpDir).filePath("arc/blyt/ui_menu_19_00a.brlyt");
            if (!Ui_menu_19_00a::injectMapIconsLayout(brlytFile, mapIconToTplName)) {
                throw ModException(QString("could not inject map icons into %1").arg(brlytFile));
            }

            // convert the brlan files to xmlan, inject the map icons and convert it back
            QDir brlanFileDir(QDir(tmpDir).filePath("arc/anim"));
            auto brlanFilesInfo = brlanFileDir.entryInfoList({"ui_menu_19_00a_Tag_*.brlan"}, QDir::Files);
            for (auto &brlanFileInfo: brlanFilesInfo) {
                auto brlanFile = brlanFileInfo.absoluteFilePath();

                bool success = Ui_menu_19_00a::injectMapIconsAnimation(brlanFile, mapIconToTplName);
                if (!success) {
                    throw ModException(QString("could not inject map icons into %1").arg(brlanFile));
                }
            }
        };
    }

    return result;
}
