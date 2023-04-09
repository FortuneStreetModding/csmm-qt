#include "musictable.h"
#include "lib/brsar.h"
#include "lib/datafileset.h"
#include "lib/powerpcasm.h"
#include "lib/vanilladatabase.h"


quint32 MusicTable::writeBgmTable(const std::vector<MapDescriptor> &descriptors) {
    QVector<quint32> table;
    for (auto &descriptor: descriptors) {
        QVector<quint32> mapMusicTable;
        // get the BGM music types only
        QVector<MusicType> bgmOnlyMusicTypes;
        for (auto &ent: descriptor.music) {
            auto &musicType = ent.first;
            if(Music::musicTypeIsBgm(musicType))
                bgmOnlyMusicTypes.append(musicType);
        }
        // write the BGM replacement table
        // write the size of the table first
        mapMusicTable.append(bgmOnlyMusicTypes.size());
        for (auto &musicType: bgmOnlyMusicTypes) {
            auto &musicEntries = descriptor.music.at(musicType);
            // write the original bgmid (the value to look for)
            mapMusicTable.append(musicType);
            // write # of entries
            mapMusicTable.append(musicEntries.size());
            // write the new brsar index (the value which shall be replaced)
            for (auto &musicEntry: musicEntries) {
                mapMusicTable.append(musicEntry.brsarIndex);
            }
        }
        if(bgmOnlyMusicTypes.empty()) {
            table.append(0);
        } else {
            table.append(allocate(mapMusicTable, "MapBgmReplacementTable"));
        }
    }
    return allocate(table, "MapBgmPointerTable");
}

quint32 MusicTable::writeMeTable(const std::vector<MapDescriptor> &descriptors) {
    QVector<quint32> table;
    for (auto &descriptor: descriptors) {
        QVector<quint32> mapMusicTable;
        // get the ME music types only
        QVector<MusicType> meOnlyMusicTypes;
        for (auto &ent: descriptor.music) {
            auto &musicType = ent.first;
            if(!Music::musicTypeIsBgm(musicType))
                meOnlyMusicTypes.append(musicType);
        }
        // build the ME replacement table
        // write the size of the table first
        mapMusicTable.append(meOnlyMusicTypes.size());
        for (auto &musicType: meOnlyMusicTypes) {
            auto &musicEntries = descriptor.music.at(musicType);
            // write the original meId (the value to look for)
            mapMusicTable.append(musicType);
            // write # of entries
            mapMusicTable.append(musicEntries.size());
            // write the new brsar index (the value which shall be replaced)
            for (auto &musicEntry: musicEntries) {
                mapMusicTable.append(musicEntry.brsarIndex);
            }
        }
        if(meOnlyMusicTypes.empty()) {
            table.append(0);
        } else {
            table.append(allocate(mapMusicTable, "MapMeReplacementTable"));
        }
    }
    return allocate(table, "MapMePointerTable");
}


void MusicTable::writeAsm(QDataStream &stream, const AddressMapper &addressMapper, const std::vector<MapDescriptor> &mapDescriptors) {
    bgmTableAddr = writeBgmTable(mapDescriptors);
    meTableAddr = writeMeTable(mapDescriptors);

    // Hijack Game::ConvBGMID(int bgmId = r3, int theme = r4)
    quint32 hijackAddr = addressMapper.boomStreetToStandard(0x801cc8a0);
    quint32 returnContinueAddr = addressMapper.boomStreetToStandard(0x801cc8a4);
    quint32 returnBgmReplacedAddr = addressMapper.boomStreetToStandard(0x801cc93c);

    quint32 subroutineReplaceBgmId = allocate(writeSubroutineReplaceBgmId(addressMapper, bgmTableAddr, 0, returnContinueAddr, returnBgmReplacedAddr), "SubroutineReplaceBgmId");
    stream.device()->seek(addressMapper.toFileAddress(subroutineReplaceBgmId));
    auto insts = writeSubroutineReplaceBgmId(addressMapper, bgmTableAddr, subroutineReplaceBgmId, returnContinueAddr, returnBgmReplacedAddr); // re-write the routine again since now we know where it is located in the main dol
    for (quint32 inst: qAsConst(insts)) stream << inst;
    stream.device()->seek(addressMapper.toFileAddress(hijackAddr));
    // mr r31, r3                                   -> b subroutineReplaceBgmId
    stream.device()->seek(addressMapper.toFileAddress(hijackAddr));
    stream << PowerPcAsm::b(hijackAddr, subroutineReplaceBgmId);

    // Hijack Game::ConvSEID(int bgmId = r3, int theme = r4)
    hijackAddr = addressMapper.boomStreetToStandard(0x801cc964);
    returnContinueAddr = addressMapper.boomStreetToStandard(0x801cc968);
    returnBgmReplacedAddr = addressMapper.boomStreetToStandard(0x801cca00);

    quint32 subroutineReplaceMeId = allocate(writeSubroutineReplaceBgmId(addressMapper, meTableAddr, 0, returnContinueAddr, returnBgmReplacedAddr), "SubroutineReplaceMeId");
    stream.device()->seek(addressMapper.toFileAddress(subroutineReplaceMeId));
    insts = writeSubroutineReplaceBgmId(addressMapper, meTableAddr, subroutineReplaceMeId, returnContinueAddr, returnBgmReplacedAddr); // re-write the routine again since now we know where it is located in the main dol
    for (quint32 inst: qAsConst(insts)) stream << inst;
    stream.device()->seek(addressMapper.toFileAddress(hijackAddr));
    // mr r31, r3                                   -> b subroutineReplaceMeId
    stream.device()->seek(addressMapper.toFileAddress(hijackAddr));
    stream << PowerPcAsm::b(hijackAddr, subroutineReplaceMeId);
}

QVector<quint32> MusicTable::writeSubroutineReplaceBgmId(const AddressMapper &addressMapper, quint32 tableAddr, quint32 entryAddr, quint32 returnContinueAddr, quint32 returnBgmReplacedAddr) {
    quint32 Game_Manager = addressMapper.boomStreetToStandard(0x8081794c);
    quint32 Global_MapID = addressMapper.boomStreetToStandard(0x80552408);
    quint32 Switch_State = addressMapper.boomStreetToStandard(0x80552410);
    PowerPcAsm::Pair16Bit g = PowerPcAsm::make16bitValuePair(Game_Manager);
    PowerPcAsm::Pair16Bit m = PowerPcAsm::make16bitValuePair(Global_MapID);
    PowerPcAsm::Pair16Bit t = PowerPcAsm::make16bitValuePair(tableAddr);
    PowerPcAsm::Pair16Bit s = PowerPcAsm::make16bitValuePair(Switch_State);

    QVector<quint32> asm_;
    auto labels = PowerPcAsm::LabelTable();
    // precondition:   r3 is bgmId
    //                 r4 is theme
    //        unused:  r0,r5,r6,r7,r31
    // postcondition:  r3 and r31 is bgmId
    asm_.append(PowerPcAsm::mr(31, 3));                                        // r31 <- r3
    asm_.append(PowerPcAsm::lis(3, g.upper));                                  // \.
    asm_.append(PowerPcAsm::addi(3, 3, g.lower));                              // |.
    asm_.append(PowerPcAsm::lwz(5, 0x0, 3));                                   // /. r5 <- Game_Manager
    asm_.append(PowerPcAsm::cmpwi(5, 0x0));                                    // r5 != 0? (is a game Manager defined? if it is, that means a game is running, otherwise we are still in menu)
    asm_.append(PowerPcAsm::bne(labels, "continue", asm_));                    // continue
    asm_.append(PowerPcAsm::mr(3, 31));                                        // r3 <- r31
    asm_.append(PowerPcAsm::cmplwi(3, 0xffff));                                // make the comparision again from the original function
    asm_.append(PowerPcAsm::b(entryAddr, asm_.size(), returnContinueAddr));    // else return returnContinueAddr
    labels.define("continue", asm_);
    asm_.append(PowerPcAsm::lis(3, m.upper));                                  // \.
    asm_.append(PowerPcAsm::addi(3, 3, m.lower));                              // |.
    asm_.append(PowerPcAsm::lwz(5, 0x0, 3));                                   // /. r5 <- Global_MapID
    asm_.append(PowerPcAsm::li(3, 2));                                         // \.
    asm_.append(PowerPcAsm::slw(5, 5, 3));                                     // /. r5 <- r5 * 4
    asm_.append(PowerPcAsm::lis(3, t.upper));                                  // \.
    asm_.append(PowerPcAsm::addi(3, 3, t.lower));                              // |.
    asm_.append(PowerPcAsm::lwzx(5, 5, 3));                                    // /. r5 <- MapMusicReplacementTable = MapBgmMePointerTable[r5]
    asm_.append(PowerPcAsm::cmpwi(5, 0x0));                                    // r5 != 0?
    asm_.append(PowerPcAsm::bne(labels, "continue2", asm_));                   // continue2
    asm_.append(PowerPcAsm::mr(3, 31));                                        // r3 <- r31
    asm_.append(PowerPcAsm::cmplwi(3, 0xffff));                                // make the comparision again from the original function
    asm_.append(PowerPcAsm::b(entryAddr, asm_.size(), returnContinueAddr));    // else return returnContinueAddr
    labels.define("continue2", asm_);
    asm_.append(PowerPcAsm::lwz(6, 0x0, 5));                                   // r6 <- size of MapMusicReplacementTable
    asm_.append(PowerPcAsm::addi(5, 5, 0x4));                                  // r5+=4
    labels.define("loop", asm_);
    asm_.append(PowerPcAsm::lwz(3, 0x0, 5));                                   // r3 <- firstBgmId
    asm_.append(PowerPcAsm::lwz(7, 0x4, 5));                                   // r7 <- numMusicEntries
    asm_.append(PowerPcAsm::cmpw(3, 31));                                      // r3 == r31?
    asm_.append(PowerPcAsm::bne(labels, "continue3", asm_));                   // {
    asm_.append(PowerPcAsm::lis(6, s.upper));                                  // \.
    asm_.append(PowerPcAsm::addi(6, 6, s.lower));                              // |.
    asm_.append(PowerPcAsm::lwz(7, 0x0, 6));                                   // /. r7 <- switchState
    asm_.append(PowerPcAsm::lwz(6, 0x4, 5));                                   // r6 <- numMusicEntries
    asm_.append(PowerPcAsm::subi(6, 6, 1));                                    // r6 -= 1
    asm_.append(PowerPcAsm::cmplw(6, 7));                                      // r6 <= r7?
    asm_.append(PowerPcAsm::blt(labels, "min", asm_));
    asm_.append(PowerPcAsm::mr(6, 7));                                         // r6 <= min(r6, r7)
    labels.define("min", asm_);
    asm_.append(PowerPcAsm::mulli(6, 6, 4));                                   // r6 *= 4
    asm_.append(PowerPcAsm::add(5, 5, 6));                                     // r5 += r6
    asm_.append(PowerPcAsm::lwz(31, 0x8, 5));                                  // r31 <- secondBgmId
    asm_.append(PowerPcAsm::mr(3, 31));                                        // r3 <- r31
    asm_.append(PowerPcAsm::cmplwi(3, 0xffff));                                // make the comparision again from the original function
    asm_.append(PowerPcAsm::b(entryAddr, asm_.size(), returnBgmReplacedAddr)); // return returnBgmReplacedAddr
                                                                               // }
    labels.define("continue3", asm_);
    asm_.append(PowerPcAsm::addi(5, 5, 0x8));                                  // r5+=8
    asm_.append(PowerPcAsm::mulli(7, 7, 4));                                   // r7 *= 4
    asm_.append(PowerPcAsm::addi(5, 5, 7));                                    // r5 += r7
    asm_.append(PowerPcAsm::subi(6, 6, 0x1));                                  // r6--
    asm_.append(PowerPcAsm::cmpwi(6, 0x0));                                    // r6 != 0?
    asm_.append(PowerPcAsm::bne(labels, "loop", asm_));                        // loop
    asm_.append(PowerPcAsm::mr(3, 31));                                        // r3 <- r31
    asm_.append(PowerPcAsm::cmplwi(3, 0xffff));                                // make the comparision again from the original function
    asm_.append(PowerPcAsm::b(entryAddr, asm_.size(), returnContinueAddr));    // else return returnContinueAddr

    labels.checkProperlyLinked();
    return asm_;
}

bool readIsVanilla(QDataStream &stream, const AddressMapper &addressMapper) {
    stream.device()->seek(addressMapper.boomToFileAddress(0x801cc8a0));
    quint32 opcode; stream >> opcode;
    return opcode == PowerPcAsm::mr(31, 3);
}

void readTable(QDataStream &stream, const AddressMapper &addressMapper, MapDescriptor &descriptor) {
    quint32 tableAddr;
    stream >> tableAddr;
    if(tableAddr == 0)
        return;
    stream.device()->seek(addressMapper.toFileAddress(tableAddr));
    quint32 tableSize;
    stream >> tableSize;
    for(int i=0; i<tableSize; i++) {
        MusicType musicType;
        quint32 numEntries;

        // read the original bgmid (the value to look for)
        stream >> musicType;
        stream >> numEntries;

        for (int i=0; i<numEntries; ++i) {
            MusicEntry musicEntry;
            // read the new brsar index (the value which shall be replaced)
            stream >> musicEntry.brsarIndex;
            descriptor.music[musicType].push_back(musicEntry);
        }
    }
}

void MusicTable::readAsm(QDataStream &stream, const AddressMapper &addressMapper, std::vector<MapDescriptor> &mapDescriptors) {
    if(!readIsVanilla(stream, addressMapper) && bgmTableAddr != 0 && meTableAddr != 0) {
        quint32 addr = bgmTableAddr;
        for (auto &descriptor: mapDescriptors) {
            stream.device()->seek(addressMapper.toFileAddress(addr));
            readTable(stream, addressMapper, descriptor);
            addr+=4;
        }
        addr = meTableAddr;
        for (auto &descriptor: mapDescriptors) {
            stream.device()->seek(addressMapper.toFileAddress(addr));
            readTable(stream, addressMapper, descriptor);
            addr+=4;
        }
    }
}

void MusicTable::loadFiles(const QString &root, GameInstance *gameInstance, const ModListType &modList) {
    // read the addresses
    QFile addrFile(QDir(root).filePath(ADDRESS_FILE.data()));
    if (addrFile.open(QFile::ReadOnly)) {
        QDataStream stream(&addrFile);
        stream >> bgmTableAddr;
        stream >> meTableAddr;
    }
    DolIO::loadFiles(root, gameInstance, modList);
    // read the brsar
    auto brsarFilePath = QDir(root).filePath(ITAST_BRSAR);
    QFileInfo brsarFileInfo(brsarFilePath);
    if (brsarFileInfo.exists() && brsarFileInfo.isFile()) {
        QFile brsarFile(brsarFilePath);
        if (brsarFile.open(QIODevice::ReadOnly)) {
            QDataStream stream(&brsarFile);
            if (Brsar::containsCsmmEntries(stream)) {
                stream.device()->seek(0);
                Brsar::read(stream, gameInstance->mapDescriptors());
            }
        } else {
            throw ModException(QString("Could not open file %1 for read/write. %2").arg(brsarFilePath, brsarFile.errorString()));
        }
    } else {
        throw ModException(QString("The file %1 does not exist.").arg(brsarFilePath));
    }

}

void MusicTable::saveFiles(const QString &root, GameInstance *gameInstance, const ModListType &modList) {
    for (auto &descriptor: gameInstance->mapDescriptors()) {
        for (auto &mapEnt: descriptor.music) {
            for (auto &musicEntry: mapEnt.second) {
                QFileInfo brstmFileInfo(QDir(root).filePath(SOUND_STREAM_FOLDER+"/"+musicEntry.brstmBaseFilename+".brstm"));
                if (!brstmFileInfo.exists()) {
                    throw ModException(QString("file %1 doesn't exist").arg(brstmFileInfo.absoluteFilePath()));
                }
                // update the file size -> needed for patching the brsar
                musicEntry.brstmFileSize = brstmFileInfo.size();
            }
        }
    }

    // the Itast.brsar is assumed to have already been patched with Itast.brsar.bsdiff in the applyAllBspatches() method and now contains special CSMM entries
    auto brsarFilePath = QDir(root).filePath(ITAST_BRSAR);
    QFileInfo brsarFileInfo(brsarFilePath);
    if (brsarFileInfo.exists() && brsarFileInfo.isFile()) {
        QFile brsarFile(brsarFilePath);
        // patch the special csmm entries in the brsar file
        if (brsarFile.open(QIODevice::ReadWrite)) {
            QDataStream stream(&brsarFile);
            if (Brsar::containsCsmmEntries(stream)) {
                stream.device()->seek(0);
                int brsarSlots = Brsar::patch(stream, gameInstance->mapDescriptors());
                if(brsarSlots == -1) {
                    throw ModException(QString("An error happened during patching the brsar file %1. The brsar file seems to be corrupt.").arg(brsarFilePath));
                } else if(brsarSlots == -2) {
                    throw ModException(QString("An error happened during patching the brsar file %1. All music %2 slots have been used up.").arg(brsarFilePath).arg(1000));
                } else {
                    qDebug() << "Used" << brsarSlots << "music slots out of" << 1000;
                }
            } else {
                throw ModException(QString("The brsar file %1 does not contain CSMM entries. You must either start with a vanilla fortune street or use Tools->Save Clean Itast.csmm.brsar").arg(brsarFilePath));
            }
        } else {
            throw ModException(QString("Could not open file %1 for read/write. %2").arg(brsarFilePath, brsarFile.errorString()));
        }
    } else {
        throw ModException(QString("The file %1 does not exist.").arg(brsarFilePath));
    }

    DolIO::saveFiles(root, gameInstance, modList);

    // store the addresses
    QFile addrFile(QDir(root).filePath(ADDRESS_FILE.data()));
    if (addrFile.open(QFile::WriteOnly)) {
        QDataStream stream(&addrFile);
        stream << bgmTableAddr;
        stream << meTableAddr;
    }
}
