#include "musictable.h"
#include "lib/brsar.h"
#include "lib/datafileset.h"
#include "lib/powerpcasm.h"
#include "lib/vanilladatabase.h"


quint32 MusicTable::writeBgmTable(const QVector<MapDescriptor> &descriptors) {
    QVector<quint32> table;
    for (auto &descriptor: descriptors) {
        QVector<quint32> mapMusicTable;
        // get the BGM music types only
        QVector<MusicType> bgmOnlyMusicTypes;
        auto keys = descriptor.music.keys();
        for (auto &musicType: keys) {
            if(Music::musicTypeIsBgm(musicType))
                bgmOnlyMusicTypes.append(musicType);
        }
        // write the BGM replacement table
        // write the size of the table first
        mapMusicTable.append(bgmOnlyMusicTypes.size());
        for (auto &musicType: bgmOnlyMusicTypes) {
            // write the original bgmid (the value to look for)
            mapMusicTable.append(musicType);
            // write the new brsar index (the value which shall be replaced)
            mapMusicTable.append(descriptor.music[musicType].brsarIndex);
        }
        if(bgmOnlyMusicTypes.empty()) {
            table.append(0);
        } else {
            table.append(allocate(mapMusicTable, "MapBgmReplacementTable"));
        }
    }
    return allocate(table, "MapBgmPointerTable");
}

quint32 MusicTable::writeMeTable(const QVector<MapDescriptor> &descriptors) {
    QVector<quint32> table;
    for (auto &descriptor: descriptors) {
        QVector<quint32> mapMusicTable;
        // get the ME music types only
        QVector<MusicType> meOnlyMusicTypes;
        auto keys = descriptor.music.keys();
        for (auto &musicType: keys) {
            if(!Music::musicTypeIsBgm(musicType))
                meOnlyMusicTypes.append(musicType);
        }
        // build the ME replacement table
        // write the size of the table first
        mapMusicTable.append(meOnlyMusicTypes.size());
        for (auto &musicType: meOnlyMusicTypes) {
            // write the original meId (the value to look for)
            mapMusicTable.append(musicType);
            // write the new brsar index (the value which shall be replaced)
            mapMusicTable.append(descriptor.music[musicType].brsarIndex);
        }
        if(meOnlyMusicTypes.empty()) {
            table.append(0);
        } else {
            table.append(allocate(mapMusicTable, "MapMeReplacementTable"));
        }
    }
    return allocate(table, "MapMePointerTable");
}


void MusicTable::writeAsm(QDataStream &stream, const AddressMapper &addressMapper, const QVector<MapDescriptor> &mapDescriptors) {
    quint32 bgmTableAddr = writeBgmTable(mapDescriptors);
    quint32 meTableAddr = writeMeTable(mapDescriptors);

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
    PowerPcAsm::Pair16Bit g = PowerPcAsm::make16bitValuePair(Game_Manager);
    PowerPcAsm::Pair16Bit m = PowerPcAsm::make16bitValuePair(Global_MapID);
    PowerPcAsm::Pair16Bit t = PowerPcAsm::make16bitValuePair(tableAddr);

    QVector<quint32> asm_;
    // precondition:   r3 is bgmId
    //                 r4 is theme
    //        unused:  r0,r5,r6,r7,r31
    // postcondition:  r3 and r31 is bgmId
    asm_.append(PowerPcAsm::mr(31, 3));             // r31 <- r3
    asm_.append(PowerPcAsm::lis(3, g.upper));       // \.
    asm_.append(PowerPcAsm::addi(3, 3, g.lower));   // |.
    asm_.append(PowerPcAsm::lwz(5, 0x0, 3));        // /. r5 <- Game_Manager
    asm_.append(PowerPcAsm::cmpwi(5, 0x0));         // r5 != 0? (is a game Manager defined? if it is, that means a game is running, otherwise we are still in menu)
    asm_.append(PowerPcAsm::bne(4));                // continue
    asm_.append(PowerPcAsm::mr(3, 31));             // r3 <- r31
    asm_.append(PowerPcAsm::cmplwi(3, 0xffff));     // make the comparision again from the original function
    asm_.append(PowerPcAsm::b(entryAddr, asm_.size(), returnContinueAddr));    // else return returnContinueAddr
    asm_.append(PowerPcAsm::lis(3, m.upper));       // \.
    asm_.append(PowerPcAsm::addi(3, 3, m.lower));   // |.
    asm_.append(PowerPcAsm::lwz(5, 0x0, 3));        // /. r5 <- Global_MapID
    asm_.append(PowerPcAsm::li(3, 2));              // \.
    asm_.append(PowerPcAsm::slw(5, 5, 3));          // /. r5 <- r5 * 4
    asm_.append(PowerPcAsm::lis(3, t.upper));       // \.
    asm_.append(PowerPcAsm::addi(3, 3, t.lower));   // |.
    asm_.append(PowerPcAsm::lwzx(5, 5, 3));         // /. r5 <- MapMusicReplacementTable = MapBgmMePointerTable[r5]
    asm_.append(PowerPcAsm::cmpwi(5, 0x0));         // r5 != 0?
    asm_.append(PowerPcAsm::bne(4));                // continue
    asm_.append(PowerPcAsm::mr(3, 31));             // r3 <- r31
    asm_.append(PowerPcAsm::cmplwi(3, 0xffff));     // make the comparision again from the original function
    asm_.append(PowerPcAsm::b(entryAddr, asm_.size(), returnContinueAddr));    // else return returnContinueAddr
    asm_.append(PowerPcAsm::lwz(6, 0x0, 5));        // r6 <- size of MapMusicReplacementTable
    asm_.append(PowerPcAsm::addi(5, 5, 0x4));       // r5+=4
    int whileVentureCardIdSmaller128 = asm_.size();
    {
        asm_.append(PowerPcAsm::lwz(3, 0x0, 5));    // r3 <- firstBgmId
        asm_.append(PowerPcAsm::cmpw(3, 31));       // r3 == r31?
        asm_.append(PowerPcAsm::bne(6));            // {
        asm_.append(PowerPcAsm::addi(5, 5, 0x4));   // r5+=4
        asm_.append(PowerPcAsm::lwz(31, 0x0, 5));   // r31 <- secondBgmId
        asm_.append(PowerPcAsm::mr(3, 31));         // r3 <- r31
        asm_.append(PowerPcAsm::cmplwi(3, 0xffff)); // make the comparision again from the original function
        asm_.append(PowerPcAsm::b(entryAddr, asm_.size(), returnBgmReplacedAddr));     // return returnBgmReplacedAddr
                                                    // }
        asm_.append(PowerPcAsm::addi(5, 5, 0x8));   // r5+=8
        asm_.append(PowerPcAsm::subi(6, 6, 0x1));   // r6--
        asm_.append(PowerPcAsm::cmpwi(6, 0x0));     // r6 != 0?
        asm_.append(PowerPcAsm::bne(asm_.size(), whileVentureCardIdSmaller128));   // loop
        asm_.append(PowerPcAsm::mr(3, 31));         // r3 <- r31
        asm_.append(PowerPcAsm::cmplwi(3, 0xffff)); // make the comparision again from the original function
        asm_.append(PowerPcAsm::b(entryAddr, asm_.size(), returnContinueAddr));    // else return returnContinueAddr
    }
    return asm_;
}

void MusicTable::readAsm(QDataStream &, QVector<MapDescriptor> &, const AddressMapper &, bool ) {
    // TODO
}

quint32 MusicTable::readTableAddr(QDataStream &, const AddressMapper &, bool) {
    // TODO
    return 0;
}

qint16 MusicTable::readTableRowCount(QDataStream &, const AddressMapper &, bool) {
    return -1;
}

bool MusicTable::readIsVanilla(QDataStream &stream, const AddressMapper &addressMapper) {
    stream.device()->seek(addressMapper.boomToFileAddress(0x801cc8a0));
    quint32 opcode; stream >> opcode;
    return opcode == PowerPcAsm::mr(31, 3);
}



void MusicTable::loadFiles(const QString &root, GameInstance &gameInstance, const ModListType &modList)
{
    DolIOTable::loadFiles(root, gameInstance, modList);
}

void MusicTable::saveFiles(const QString &root, GameInstance &gameInstance, const ModListType &modList)
{
    DolIOTable::saveFiles(root, gameInstance, modList);

    for (auto &descriptor: gameInstance.mapDescriptors()) {
        for (auto &musicEntry: descriptor.music) {
            QFileInfo brstmFileInfo(QDir(root).filePath(SOUND_STREAM_FOLDER+"/"+musicEntry.brstmBaseFilename+".brstm"));
            if (!brstmFileInfo.exists()) {
                throw ModException(QString("file %1 doesn't exist").arg(brstmFileInfo.absoluteFilePath()));
            }
            // update the file size -> needed for patching the brsar
            musicEntry.brstmFileSize = brstmFileInfo.size();
        }
    }

    // the Itast.brsar is assumed to have already been patched with Itast.brsar.bsdiff in the applyAllBspatches() method and now contains special CSMM entries
    auto brsarFilePath = QDir(root).filePath(SOUND_FOLDER+"/Itast.brsar");
    QFileInfo brsarFileInfo(brsarFilePath);
    if (brsarFileInfo.exists() && brsarFileInfo.isFile()) {
        QFile brsarFile(brsarFilePath);
        // patch the special csmm entries in the brsar file
        if (brsarFile.open(QIODevice::ReadWrite)) {
            QDataStream stream(&brsarFile);
            if (Brsar::containsCsmmEntries(stream)) {
                stream.device()->seek(0);
                Brsar::patch(stream, gameInstance.mapDescriptors());
            } else {
                throw ModException(QString("The brsar file %1 does not contain CSMM entries. You must either start with a vanilla fortune street or use Tools->Save Clean Itast.csmm.brsar").arg(brsarFilePath));
            }
        } else {
            throw ModException(QString("Could not open file %1 for read/write. %2").arg(brsarFilePath, brsarFile.errorString()));
        }
    } else {
        throw ModException(QString("The file %1 does not exist.").arg(brsarFilePath));
    }
}
