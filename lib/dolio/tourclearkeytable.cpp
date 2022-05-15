#include "tourclearkeytable.h"
#include "lib/powerpcasm.h"

quint32 TourClearKeyTable::writeTable(const QVector<MapDescriptor> &descriptors) {
    QVector<quint32> table;
    for (auto &descriptor: descriptors) table.append(descriptor.tourClearKey);
    return allocate(table, "TourClearKeyTable");
}

void TourClearKeyTable::writeAsm(QDataStream &stream, const AddressMapper &addressMapper, const QVector<MapDescriptor> &mapDescriptors) {
    quint32 tableAddr = writeTable(mapDescriptors);
    PowerPcAsm::Pair16Bit v = PowerPcAsm::make16bitValuePair(tableAddr);

    // --- Game::GetMapOriginID ---
    // mulli r0,r3,0x38 ->  mulli r0,r3,0x04
    stream.device()->seek(addressMapper.boomToFileAddress(0x801ccb58)); stream << PowerPcAsm::mulli(0, 3, 0x04);
    // r3 <- 0x80428e50 ->  r3 <- tableAddr
    stream.device()->seek(addressMapper.boomToFileAddress(0x801ccb5c)); stream << PowerPcAsm::lis(3, v.upper) << PowerPcAsm::addi(3, 3, v.lower);
    // lwz r3,0x4(r3)   ->  lwz r3,0x0(r3)
    stream.device()->seek(addressMapper.boomToFileAddress(0x801ccb68)); stream << PowerPcAsm::lwz(3, 0x0, 3);

    // --- Remove all uses of Game::GameSequenceDataAdapter::GetMapOrigin ---
    // The Game::GameSequenceDataAdapter::GetMapOrigin subroutine makes a mapping of mapId to mapIndex. We do not want that anymore and as such
    // we need to remove all uses of this method

    // -- isMapUnlockedByAnyUser ---
    // bl Game::GameSequenceDataAdapter::GetMapOrigin   ->  nop
    stream.device()->seek(addressMapper.boomToFileAddress(0x80185298)); stream << PowerPcAsm::nop();
    // -- isMapUnlockedByAnyUser ---
    // bl Game::GameSequenceDataAdapter::GetMapOrigin   ->  nop
    stream.device()->seek(addressMapper.boomToFileAddress(0x8021e6ac)); stream << PowerPcAsm::nop();

    // --- Flag::SaveData::ClearUserInfo ---
    // TODO: Clear all clearKeys when the user wants to delete his save file
    // 80012a80

    // --- Game::GameResultScene::Base_Exec ---
    // > check all maps if anything can be unlocked --
    // cmpwi r3,0x12    ->  cmpwi r3,mapDescriptors.size()
    stream.device()->seek(addressMapper.boomToFileAddress(0x80178884)); stream << PowerPcAsm::cmpwi(3, mapDescriptors.size());

    // --- Game::DataSelectUI::Exec_Active ---
    // > Since the save game file is pretty packed, we remove the 10th save game slot from the game and repurpose it to store general data.
    auto a = allocate(writeRoutine_GetAmountOfSlots(addressMapper, 0), "GetAmountOfSlots(");
    stream.device()->seek(addressMapper.toFileAddress(a));
    auto insts = writeRoutine_GetAmountOfSlots(addressMapper, a); // re-write the routine again since now we know where it is located in the main dol
    for (quint32 inst: qAsConst(insts)) stream << inst;
    quint32 hijackAddr = addressMapper.boomStreetToStandard(0x8016b244);
    stream.device()->seek(addressMapper.toFileAddress(hijackAddr));
    // cmpwi r4,0x9   -> b
    stream << PowerPcAsm::b(hijackAddr, a);

    // > Initialize CSMM Save Game Data
    a = allocate(writeRoutine_InitializeSaveGameData(addressMapper, 0), "InitializeSaveGameData");
    stream.device()->seek(addressMapper.toFileAddress(a));
    insts = writeRoutine_InitializeSaveGameData(addressMapper, a); // re-write the routine again since now we know where it is located in the main dol
    for (quint32 inst: qAsConst(insts)) stream << inst;
    hijackAddr = addressMapper.boomStreetToStandard(0x8016a0e8);
    stream.device()->seek(addressMapper.toFileAddress(hijackAddr));
    // beq 0x8016a0e8   -> b
    stream << PowerPcAsm::b(hijackAddr, a);

    // For each board that has been completed in tour mode, its tourClearKey is stored in a list.
    // The tourClearKey is constrained to be max of 5 characters of only uppercase letters and digits. If we convert that to an integer using base 36,
    // it will take up at most 3 bytes of memory. We can use the 4th byte for information on clear rank, new state, etc.

    // 0x8053d6a0 table of vanilla maps clear information
    // 2x20 bytes (first 20 bytes = easy maps, second 20 bytes = standard maps)
    //      |__ eachByte: 0x03 = Uncleared
    //                    0x02 = Cleared 3rd place
    //                    0x01 = Cleared 2nd place
    //                    0x00 = Cleared 1st place

    // highest byte specification (0xFF000000):
    //  0000 0000
    //       ||||
    //       |||__ ClearRank: 11 = Cleared 1st place
    //       ||               10 = Cleared 2nd place
    //       ||               01 = Cleared 3rd place
    //       ||               00 = Uncleared
    //       ||___ ReleaseState: 1 = Released (= unlocked), 0 = Unreleased (= locked)
    //       |____ NewState: 1 = Show "NEW" label on map Icon, 0 = Do not show "NEW" label on map icon



    // Stuff we need to reimplement

    // enum GAME_MODE_TYPE { LOAD_EXISTING_GAME = 0, NEW_GAME = 1, TUTORIAL = 2, AUTO = 3 }
    // enum GAME_RULE_TYPE { EASY = 0, STANDARD = 1, AUTO = 2 }                               NOTE: This is the other way around from MapDescriptor::RuleSet
    // enum ZONE_TYPE { MARIO_TOUR = 0, DRAGON_QUEST_TOUR = 1, SPECIAL_TOUR = 2 }             NOTE: This order is different from how the vanilla game shows the zones
    // void Game::GameSequenceDataAdapter::InitMapReleased(int mapId, GAME_RULE_TYPE gameRule, BASE_USERINFO* userInfo)
    // bool Game::GameSequenceDataAdapter::IsMapReleased(int mapId)
    // void Game::GameSequenceDataAdapter::SetMapReleased(int mapId, bool released)
    // bool Game::GameSequenceDataAdapter::IsMapNew(int mapId, GAME_MODE_TYPE param_2)
    // void Game::GameSequenceDataAdapter::SetMapNew(int mapId, GAME_MODE_TYPE param_2)
    // bool Game::GameSequenceDataAdapter::IsMapNewFirst(int mapId)
    // void Game::GameSequenceDataAdapter::SetMapNewFirst(int mapId)
    // bool Game::GameSequenceDataAdapter::IsMapClearFirst(int mapId)
    // void Game::GameSequenceDataAdapter::SetMapClearFirst(int mapId)
    // bool Game::GameSequenceDataAdapter::IsMapVictoryFirst(int mapId)
    // void Game::GameSequenceDataAdapter::SetMapVictoryFirst(int mapId)
    // bool Game::GameSequenceDataAdapter::IsZoneClear(ZONE_TYPE zone)
    // bool Game::GameSequenceDataAdapter::IsMapClear(int mapId, GAME_RULE_TYPE gameRule, int userIndex)
    // void Game::GameSequenceDataAdapter::SetMapClearRank(int mapId, int clearRank)
    // int Game::GameSequenceDataAdapter::GetMapRank(int mapId, GAME_RULE_TYPE gameRule)
    // bool Game::GameSequenceDataAdapter::IsNormalClear(int userIndex)
    // bool Game::GameSequenceDataAdapter::IsEasyClear(int userIndex)
    // bool Game::GameSequenceDataAdapter::IsMapVictory(int mapId, GAME_RULE_TYPE gameRule)
    // bool Game::GameSequenceDataAdapter::IsZoneVictory(ZONE_TYPE zone, GAME_RULE_TYPE gameRule)
    // void Game::GameSequenceDataAdapter::SetMapVictory(int mapId, bool victory)
    // void Flag::SaveData::ClearUserInfo(int userIndex)
    // Game::CanUnlockMap(int mapId)
    // Game::SetMapUnlock(int mapId)
    //
    // Also all accounts of RULESET_MAP_COUNT (18), which are around 38 references
}

QVector<quint32> TourClearKeyTable::writeRoutine_GetAmountOfSlots(const AddressMapper &addressMapper, quint32 routineStartAddress) {
    PowerPcAsm::Pair16Bit v = PowerPcAsm::make16bitValuePair(addressMapper.boomStreetToStandard(0x8052ed60));

    auto retAddress = addressMapper.boomStreetToStandard(0x8016b248);

    QVector<quint32> asm_;
    asm_.append(PowerPcAsm::lis(3, v.upper));
    asm_.append(PowerPcAsm::addi(3, 3, v.lower));
    asm_.append(PowerPcAsm::lbz(3, 0, 3));
    asm_.append(PowerPcAsm::cmpwi(3, 0xff));
    asm_.append(PowerPcAsm::beq(3));
    asm_.append(PowerPcAsm::cmpwi(4, 0x9));
    asm_.append(PowerPcAsm::b(routineStartAddress, asm_.count(), retAddress));
    asm_.append(PowerPcAsm::cmpwi(4, 0x8));
    asm_.append(PowerPcAsm::b(routineStartAddress, asm_.count(), retAddress));

    return asm_;
}

QVector<quint32> TourClearKeyTable::writeRoutine_InitializeSaveGameData(const AddressMapper &addressMapper, quint32 routineStartAddress) {
    // precondition:  r0 <- current state
    // postcondition:  r0 <- current state

    QTextCodec *codec = QTextCodec::codecForName("UTF-16");
    QTextEncoder *encoderWithoutBom = codec->makeEncoder(QTextCodec::IgnoreHeader);
    QByteArray bytes = encoderWithoutBom->fromUnicode("CSMM Save Game Data");

    auto Game_MessageWindowUI_OneShot = addressMapper.boomStreetToStandard(0x8018ae38);
    auto Flag_SaveData_GetUserInfo = addressMapper.boomStreetToStandard(0x80012264);

    QVector<quint32> asm_;
    // start
    asm_.append(PowerPcAsm::mr(28, 0));
    asm_.append(PowerPcAsm::mr(27, 3));
    asm_.append(PowerPcAsm::mr(26, 4));
    asm_.append(PowerPcAsm::mr(30, 5));
    asm_.append(PowerPcAsm::cmpwi(0, 0));
    asm_.append(PowerPcAsm::beq(133, 153-3));
    asm_.append(PowerPcAsm::cmpwi(0, 1));
    asm_.append(PowerPcAsm::beq(135, 145-2));
    // --- Other State -> Resume normal work ---
    auto ret = asm_.count(); asm_.append(PowerPcAsm::mr(0, 28));
    asm_.append(PowerPcAsm::mr(3, 27));
    asm_.append(PowerPcAsm::mr(4, 26));
    asm_.append(PowerPcAsm::mr(5, 30));
    asm_.append(PowerPcAsm::cmpwi(0, 0x3bf));
    asm_.append(PowerPcAsm::beq(routineStartAddress, asm_.size(), addressMapper.boomStreetToStandard(0x8016b2b0)));
    asm_.append(PowerPcAsm::b(routineStartAddress, asm_.size(), addressMapper.boomStreetToStandard(0x8016a0ec)));
    // --- State == 1 -> Wait until Message Box closed ---
    asm_.append(PowerPcAsm::lwz(3, 0x98, 31));
    asm_.append(PowerPcAsm::lwz(3, 0x38, 3));
    asm_.append(PowerPcAsm::cmpwi(3, 1));
    asm_.append(PowerPcAsm::bne(3));
    asm_.append(PowerPcAsm::li(0, 0x40d));
    asm_.append(PowerPcAsm::stw(0, 0x30, 31));
    asm_.append(PowerPcAsm::b(routineStartAddress, asm_.size(), addressMapper.boomStreetToStandard(0x8016b2b8)));
    // --- State == 0 -> Check and Init Save Game Slot 10 ---
    asm_.append(PowerPcAsm::li(3, 0x9));
    asm_.append(PowerPcAsm::bl(routineStartAddress, asm_.size(), Flag_SaveData_GetUserInfo)); // get Save Game Slot 10
    asm_.append(PowerPcAsm::lbz(4, 0x38, 3));
    asm_.append(PowerPcAsm::cmpwi(4, 0xff));
    asm_.append(PowerPcAsm::beq(asm_.count(), ret));  // Save Game Slot 10 already initialized as CSMM -> return normal work
    asm_.append(PowerPcAsm::lhz(4, 0x8, 3));          // Get Name of Save Game Slot 10
    asm_.append(PowerPcAsm::cmpwi(4, 0x0));
    asm_.append(PowerPcAsm::beq(160, 193-2));         // Save Game Slot 10 is empty -> Init Save Game Slot
    // Save Game Slot 10 is already in use -> open message box
    asm_.append(PowerPcAsm::li(0, -0x1));
    asm_.append(PowerPcAsm::stw(0, 0x188, 1));
    asm_.append(PowerPcAsm::stw(0, 0x18c, 1));
    asm_.append(PowerPcAsm::stw(0, 0x190, 1));
    asm_.append(PowerPcAsm::stw(0, 0x194, 1));
    asm_.append(PowerPcAsm::stw(0, 0x198, 1));
    asm_.append(PowerPcAsm::stw(0, 0x19c, 1));
    asm_.append(PowerPcAsm::stw(0, 0x1a0, 1));
    asm_.append(PowerPcAsm::stw(0, 0x1a4, 1));
    asm_.append(PowerPcAsm::stw(0, 0x1a8, 1));
    asm_.append(PowerPcAsm::stw(0, 0x1ac, 1));
    asm_.append(PowerPcAsm::stw(0, 0x160, 1));
    asm_.append(PowerPcAsm::stw(0, 0x164, 1));
    asm_.append(PowerPcAsm::stw(0, 0x168, 1));
    asm_.append(PowerPcAsm::stw(0, 0x16c, 1));
    asm_.append(PowerPcAsm::stw(0, 0x170, 1));
    asm_.append(PowerPcAsm::stw(0, 0x174, 1));
    asm_.append(PowerPcAsm::stw(0, 0x178, 1));
    asm_.append(PowerPcAsm::stw(0, 0x17c, 1));
    asm_.append(PowerPcAsm::stw(0, 0x180, 1));
    asm_.append(PowerPcAsm::stw(0, 0x184, 1));
    asm_.append(PowerPcAsm::lwz(3, 0x98, 31));
    asm_.append(PowerPcAsm::li(4, 25003));
    asm_.append(PowerPcAsm::addi(5, 1, 0x188));
    asm_.append(PowerPcAsm::li(6, 0x1));
    asm_.append(PowerPcAsm::li(7, 0x2));
    asm_.append(PowerPcAsm::bl(routineStartAddress, asm_.size(), Game_MessageWindowUI_OneShot));
    // set state=1
    asm_.append(PowerPcAsm::li(0, 0x1));
    asm_.append(PowerPcAsm::stw(0, 0x30, 31));
    asm_.append(PowerPcAsm::b(routineStartAddress, asm_.size(), addressMapper.boomStreetToStandard(0x8016b2b8)));
    // Init Save Game Slot 10 for CSMM
    int i=0,j=0;
    while(i<bytes.length()) {
        asm_.append(PowerPcAsm::lis(4, (bytes.at(i+1)<<8) + bytes.at(i)));
        i+=2;
        if(i<bytes.length())
            asm_.append(PowerPcAsm::ori(4, 4, (bytes.at(i+1)<<8) + bytes.at(i)));
        i+=2;
        asm_.append(PowerPcAsm::stw(4, 8+j, 3));
        j+=4;
    }
    // To avoid that the player accidentally corrupts his 10th save game slot by using a vanilla game, we set the isWiiMoteHorizontal property to -1. This will
    // cause the game to be soft locked when picking that save game slot. Although not ideal, it is still preferable to accidentally corrupting the CSMM save game data.
    // We also use this flag above to check if the save game slot has already been initalized as CSMM data
    asm_.append(PowerPcAsm::li(4, -0x1));
    asm_.append(PowerPcAsm::stb(4, 0x38, 3));

    asm_.append(PowerPcAsm::b(asm_.count(), ret)); // -> return normal work
    return asm_;
}

QVector<quint32> TourClearKeyTable::writeRoutine_GetGainedMapClearKeyAddress(const AddressMapper &addressMapper, quint32 routineStartAddress, quint32 tableAddr) {
    constexpr int N = 16; // the stack size
    // precondition:  r3 <- mapId
    // precondition:  r4 <- UserIndex
    // postcondition: r3 <- address of the gained ClearKey or 0 if ClearKey not gained yet

    auto Flag_SaveData_GetCurrentUser = 0x3;

    QVector<quint32> asm_;
    asm_.append(PowerPcAsm::stwu(1, -N, 1));                // move stack pointer
    asm_.append(PowerPcAsm::mflr(11));                      // store link register value in r11
    asm_.append(PowerPcAsm::stw(11, N+4, 1));               // put link register value in stack
    asm_.append(PowerPcAsm::stw(0, N-4, 1));                // save r0, etc. to stack
    asm_.append(PowerPcAsm::stw(3, N-8, 1));
    asm_.append(PowerPcAsm::stw(5, N-12, 1));
    // start
    asm_.append(PowerPcAsm::cmpwi(3, 40));
    asm_.append(PowerPcAsm::bge(5));
    asm_.append(PowerPcAsm::bge(5));
    // end
    asm_.append(PowerPcAsm::lwz(5, N-12, 1));               // pop r0, etc. from stack to restore values
    asm_.append(PowerPcAsm::lwz(3, N-8, 1));
    asm_.append(PowerPcAsm::lwz(0, N-4, 1));
    asm_.append(PowerPcAsm::lwz(11, N+4, 1));               // restore link register value from stack
    asm_.append(PowerPcAsm::addi(1, 1, N));                 // reset stack pointer
    asm_.append(PowerPcAsm::mtlr(11));                      // restore link register for return
    asm_.append(PowerPcAsm::blr());                         // return
    return asm_;
}

void TourClearKeyTable::readAsm(QDataStream &stream, QVector<MapDescriptor> &mapDescriptors, const AddressMapper &, bool isVanilla) {
    if (isVanilla) {
        stream.skipRawData(0x30);
    }
    for(int i=0;i<mapDescriptors.length();i++) {
        auto &mapDescriptor = mapDescriptors[i];
        stream >> mapDescriptor.tourClearKey;
        if (isVanilla) {
            // account for the offset (the clear flag of standard maps is stored in memory directly after easy maps)
            if(i<=20)
                mapDescriptor.tourClearKey += 20;
            // in vanilla main.dol the table has other stuff in it like bgm id, map frb files, etc.
            // this we need to skip to go the next target amount in the table
            stream.skipRawData(0x38 - 0x04);
        }
    }
}

quint32 TourClearKeyTable::readTableAddr(QDataStream &stream, const AddressMapper &addressMapper, bool) {
    stream.device()->seek(addressMapper.boomToFileAddress(0x801ccb5c));
    quint32 lisOpcode, addiOpcode;
    stream >> lisOpcode >> addiOpcode;
    return PowerPcAsm::make32bitValueFromPair(lisOpcode, addiOpcode);
}

qint16 TourClearKeyTable::readTableRowCount(QDataStream &, const AddressMapper &, bool) {
    return -1;
}

bool TourClearKeyTable::readIsVanilla(QDataStream &stream, const AddressMapper &addressMapper) {
    stream.device()->seek(addressMapper.boomToFileAddress(0x801ccb58));
    quint32 opcode; stream >> opcode;
    // mulli r0,r3,0x38
    return opcode == PowerPcAsm::mulli(0, 3, 0x38);
}

