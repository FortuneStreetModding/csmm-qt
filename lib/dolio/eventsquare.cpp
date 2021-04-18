#include "eventsquare.h"
#include "lib/powerpcasm.h"

void EventSquare::readAsm(QDataStream &, const AddressMapper &, QVector<MapDescriptor> &) { /* crab nothing to do crab */ }
void EventSquare::writeAsm(QDataStream &stream, const AddressMapper &addressMapper, const QVector<MapDescriptor> &) {
    // this is the virtual address for the ForceVentureCardVariable: 0x804363c4
    // instead of allocating space for the venture card variable with the free space manager, we use a fixed virtual address
    // so that this variable can be reused by other hacks outside of CSMM.
    quint32 forceVentureCardVariable = addressMapper.boomStreetToStandard(0x804363c4);
    stream.device()->seek(addressMapper.toFileAddress(forceVentureCardVariable));
    // write zeroes to it
    stream << quint32(0); // 4 bytes

    // --- Model ---
    // some examples:
    //   80087f88 = Property
    //   80088040 = Bank
    //   80088100 = Default
    //   80088050 = Take a break
    //   80088068 = Stockbroker
    //   80088048 = Arcade
    //   80088060 = Switch
    //   80088058 = Cannon
    stream.device()->seek(addressMapper.boomToFileAddress(0x80453330));
    stream << (quint32)addressMapper.boomStreetToStandard(0x80088060);

    // --- Texture ---
    quint32 customTextureHandler = allocate(writeGetTextureForCustomSquareRoutine(15, 14), "GetTextureForCustomSquareRoutine");
    quint32 virtualPos = addressMapper.boomStreetToStandard(0x80086d98);
    stream.device()->seek(addressMapper.toFileAddress(virtualPos));
    // li r15,0x1        -> bl customTextureHandler
    stream << PowerPcAsm::bl(virtualPos, customTextureHandler);

    customTextureHandler = allocate(writeGetTextureForCustomSquareRoutine(31, 30), "GetTextureForCustomSquareRoutine2");
    virtualPos = addressMapper.boomStreetToStandard(0x80087a24);
    stream.device()->seek(addressMapper.toFileAddress(virtualPos));
    // li r31,0x1        -> bl customTextureHandler
    stream << PowerPcAsm::bl(virtualPos, customTextureHandler);

    // --- Icon ---
    stream.device()->seek(addressMapper.boomToFileAddress(0x804160c8));
    stream << (quint32)addressMapper.boomStreetToStandard(0x80415ee0); // pointer to the texture name (0x80415ee0 points to the string "p_mark_21" which is the switch icon texture
    stream << (quint32)addressMapper.boomStreetToStandard(0x80415ee0); // we need to write it twice: once for each design type (Mario and DragonQuest)

    // --- Name ---
    stream.device()->seek(addressMapper.boomToFileAddress(0x80475580));
    stream << (quint32)3336; // id of the message in ui_message.csv (3336 = "Switch square")

    // --- Description ---
    quint32 customDescriptionRoutine = allocate(writeGetDescriptionForCustomSquareRoutine(addressMapper, 0), "GetDescriptionForCustomSquareRoutine");
    stream.device()->seek(addressMapper.toFileAddress(customDescriptionRoutine));
    auto routineCode = writeGetDescriptionForCustomSquareRoutine(addressMapper, customDescriptionRoutine);
    for (quint32 inst: qAsConst(routineCode)) stream << inst; // re-write the routine again since now we know where it is located in the main dol

    virtualPos = addressMapper.boomStreetToStandard(0x800f8ce4);
    stream.device()->seek(addressMapper.toFileAddress(virtualPos));
    // bl Game::uitext::get_string   -> bl customDescriptionRoutine
    stream << PowerPcAsm::bl(virtualPos, customDescriptionRoutine);

    virtualPos = addressMapper.boomStreetToStandard(0x800f8d6c);
    stream.device()->seek(addressMapper.toFileAddress(virtualPos));
    // bl Game::uitext::get_string   -> bl customDescriptionRoutine
    stream << PowerPcAsm::bl(virtualPos, customDescriptionRoutine);

    virtualPos = addressMapper.boomStreetToStandard(0x800f8dd8);
    stream.device()->seek(addressMapper.toFileAddress(virtualPos));
    // bl Game::uitext::get_string   -> bl customDescriptionRoutine
    stream << PowerPcAsm::bl(virtualPos, customDescriptionRoutine);

    virtualPos = addressMapper.boomStreetToStandard(0x800f8e5c);
    stream.device()->seek(addressMapper.toFileAddress(virtualPos));
    // bl Game::uitext::get_string   -> bl customDescriptionRoutine
    stream << PowerPcAsm::bl(virtualPos, customDescriptionRoutine);

    virtualPos = addressMapper.boomStreetToStandard(0x800f8ee4);
    stream.device()->seek(addressMapper.toFileAddress(virtualPos));
    // bl Game::uitext::get_string   -> bl customDescriptionRoutine
    stream << PowerPcAsm::bl(virtualPos, customDescriptionRoutine);

    virtualPos = addressMapper.boomStreetToStandard(0x800f8f4c);
    stream.device()->seek(addressMapper.toFileAddress(virtualPos));
    // bl Game::uitext::get_string   -> bl customDescriptionRoutine
    stream << PowerPcAsm::bl(virtualPos, customDescriptionRoutine);

    // --- Behavior ---
    // the idea is that whenever someone stops at the event square, it sets our custom variable "ForceVentureCardVariable" to the id of the venture card and runs the Venture Card Mode (0x1c).
    // The custom variable is used to remember which venture card should be played the next time a venture card is executed.
    quint32 procStopEventSquareRoutine = allocate(writeProcStopEventSquareRoutine(addressMapper, forceVentureCardVariable, 0), "procStopEventSquareRoutine");
    stream.device()->seek(addressMapper.toFileAddress(procStopEventSquareRoutine));
    routineCode = writeProcStopEventSquareRoutine(addressMapper, forceVentureCardVariable, procStopEventSquareRoutine);
    for (quint32 inst: qAsConst(routineCode)) stream << inst; // re-write the routine again since now we know where it is located in the main dol

    stream.device()->seek(addressMapper.boomToFileAddress(0x80475838));
    stream << (quint32)procStopEventSquareRoutine;

    // --- Hijack Venture Card Mode ---
    // We are hijacking the execute venture card mode (0x1f) to check if our custom variable "ForceVentureCardVariable" has been set to anything other than 0.
    // If it was, then setup that specific venture card to be executed. Also reset our custom variable "ForceVentureCardVariable" so that normal venture cards still work.
    quint32 forceFetchFakeVentureCard = allocate(writeSubroutineForceFetchFakeVentureCard(forceVentureCardVariable), "forceFetchFakeVentureCard");
    virtualPos = addressMapper.boomStreetToStandard(0x801b7f44);
    stream.device()->seek(addressMapper.toFileAddress(virtualPos));
    // li r4,-0x1   -> bl forceFetchFakeVentureCard
    stream << PowerPcAsm::bl(virtualPos, forceFetchFakeVentureCard);
    // li r8,0x3    -> nop
    stream.device()->seek(addressMapper.boomToFileAddress(0x801b7f74));
    stream << PowerPcAsm::nop();
}
QVector<quint32> EventSquare::writeGetDescriptionForCustomSquareRoutine(const AddressMapper & addressMapper, quint32 routineStartAddress) {
    quint32 gameUiTextGetString = addressMapper.boomStreetToStandard(0x800f78dc);
    quint32 gameUiTextGetCardMsg = addressMapper.boomStreetToStandard(0x800f837c);
    auto gameBoard = PowerPcAsm::make16bitValuePair(addressMapper.boomStreetToStandard(0x8054d018));
    return {
        PowerPcAsm::lis(7, gameBoard.upper),                                         //
        PowerPcAsm::addi(7, 7, gameBoard.lower),                                     // r7 <- start of gameboard table containing all squares
        PowerPcAsm::mulli(8, 24, 0x54),                                              // r8 <- squareId * 0x54 (the size of each square)
        PowerPcAsm::add(6, 7, 8),                                                    // r6 <- the current square
        PowerPcAsm::lbz(8, 0x4d, 6),                                                 // r8 <- square.squareType
        PowerPcAsm::cmpwi(8, 0x2e),                                                  // if(square.squareType == 0x2e)
        PowerPcAsm::bne(3),                                                          // {
        PowerPcAsm::lbz(4, 0x18, 6),                                                 //   r4 <- square.district_color
        PowerPcAsm::b(routineStartAddress, 8 /* asm.Count */, gameUiTextGetCardMsg), //   goto Game::uitext::get_card_message(r4)
                                                                                     // }
        PowerPcAsm::li(6, 0x0),                                                      // |
        PowerPcAsm::li(7, 0x0),                                                      // | No message arguments
        PowerPcAsm::li(8, 0x0),                                                      // |
        PowerPcAsm::b(routineStartAddress, 12 /* asm.Count */, gameUiTextGetString)  // goto Game::uitext::get_string(r4, 0, 0, 0)
    };
}

QVector<quint32> EventSquare::writeGetTextureForCustomSquareRoutine(quint8 register_textureType, quint8 register_squareType) {
    return {
        PowerPcAsm::li(register_textureType, 0x1),    // textureType = 1
        PowerPcAsm::cmpwi(register_squareType, 0x2e), // if(squareType == 0x2e)
        PowerPcAsm::beq(2),                           // {
        PowerPcAsm::blr(),                            //   return textureType;
                                                      // } else {
        PowerPcAsm::li(register_textureType, 0x5),    //   textureType = 5
        PowerPcAsm::blr()                             //   return textureType;
                                                      // }
    };
}

QVector<quint32> EventSquare::writeProcStopEventSquareRoutine(const AddressMapper & addressMapper, quint32 forceVentureCardVariable, quint32 routineStartAddress) {
    PowerPcAsm::Pair16Bit v = PowerPcAsm::make16bitValuePair(forceVentureCardVariable);
    quint32 gameProgressChangeModeRoutine = addressMapper.boomStreetToStandard(0x800c093c);
    quint32 endOfSwitchCase = addressMapper.boomStreetToStandard(0x800fac38);

    return {
        PowerPcAsm::lwz(3, 0x188, 28),         // |
        PowerPcAsm::lwz(3, 0x74, 3),           // | r3_place = gameChara.currentPlace
        PowerPcAsm::lbz(6, 0x18, 3),           // | r6_ventureCardId = r3_place.districtId

        PowerPcAsm::lis(3, v.upper),           // |
        PowerPcAsm::addi(3, 3, v.lower),       // | forceVentureCardVariable <- r6_ventureCardId
        PowerPcAsm::stw(6, 0x0, 3),            // |

        PowerPcAsm::lwz(3, 0x18, 20),          // | lwz r3,0x18(r20)
        PowerPcAsm::li(4, 0x1f),               // | li r4,0x1f  (the GameProgress mode id 0x1f is for executing a venture card)
        PowerPcAsm::li(5, -0x1),               // | li r5,-0x1
        PowerPcAsm::li(6, -0x1),               // | li r6,-0x1
        PowerPcAsm::li(7, 0x0),                // | li r7,0x0
        PowerPcAsm::bl(routineStartAddress, 11 /* asm.Count */, gameProgressChangeModeRoutine),        // | bl Game::GameProgress::changeMode
        PowerPcAsm::b(routineStartAddress, 12 /* asm.Count */, endOfSwitchCase)        // | goto end of switch case
    };
}

QVector<quint32> EventSquare::writeSubroutineForceFetchFakeVentureCard(quint32 fakeVentureCard) {
    PowerPcAsm::Pair16Bit v = PowerPcAsm::make16bitValuePair(fakeVentureCard);

    // precondition: r3 is ChanceCardUI *
    // ChanceCardUI->field_0x34 is ChanceBoard *
    // ChanceBoard->field_0x158 is current venture card id

    return {
        PowerPcAsm::lis(6, v.upper),      // |
        PowerPcAsm::addi(6, 6, v.lower),  // | r6 <- forceVentureCardVariable
        PowerPcAsm::lwz(4, 0x0, 6),       // | r4 <- forceVentureCard
        PowerPcAsm::cmpwi(4, 0x0),        // | if(forceVentureCard != 0)
        PowerPcAsm::beq(7),               // | {
        PowerPcAsm::lwz(5, 0x34, 3),      // |   r5 <- ChanceCardUI.ChanceBoard
        PowerPcAsm::stw(4, 0x158, 5),     // |   ChanceBoard.currentVentureCardId <- r4
        PowerPcAsm::li(5, 0),             // |   forceVentureCard <- 0
        PowerPcAsm::stw(5, 0x0, 6),       // |
        PowerPcAsm::li(8, 0x0),           // |   r8 <- 0 (the venture card is initialized)
        PowerPcAsm::blr(),                // |   return r4 and r8
                                          // | }
        PowerPcAsm::li(4, -0x1),          // | r4 <- -1
        PowerPcAsm::li(8, 0x3),           // | r8 <- 3 (the venture card is continued to be executed)
        PowerPcAsm::blr()                 // | return r4 and r8
    };
}
