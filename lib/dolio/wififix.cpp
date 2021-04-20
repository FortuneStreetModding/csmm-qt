#include "wififix.h"
#include "lib/powerpcasm.h"

void WifiFix::readAsm(QDataStream &, const AddressMapper &, QVector<MapDescriptor> &) { /* crab nothing to do crab */ }
void WifiFix::writeAsm(QDataStream &stream, const AddressMapper &addressMapper, const QVector<MapDescriptor> &mapDescriptors) {
    // --- Skip "Random or Friend" Wifi Menu ---
    // we disable that menu, since random cannot work with added maps anymore
    stream.device()->seek(addressMapper.boomToFileAddress(0x802405b8));
    stream << PowerPcAsm::li(0, 0x1be); // set state of WifiMenuScene to "ExecButton" after connecting to wifi server
    // instead of checking which button was pressed (random or friend), check whether we are going forward or backward
    quint32 GameSceneBase_IsForward = addressMapper.boomStreetToStandard(0x80167bcc);
    stream.device()->seek(addressMapper.boomToFileAddress(0x80240b1c));
    stream << PowerPcAsm::bl(addressMapper.boomStreetToStandard(0x80240b1c), GameSceneBase_IsForward);
    // if going forward -> go to Friend Menu
    // if going backward -> disconnect
    stream.device()->seek(addressMapper.boomToFileAddress(0x80240b3c));
    stream << PowerPcAsm::li(4, 0x16d);        //
    stream << PowerPcAsm::stw(4, 0x1d8, 31);   // this->wifiState = WIFI_CloseWifi (0x16d);
    stream << PowerPcAsm::b(addressMapper.boomStreetToStandard(0x80240b44), addressMapper.boomStreetToStandard(0x80240b78));

    // --- Fix Wifi Map Selection ---
    // bl GameSequenceDataAdapter::GetMapOrigin(r3)                               -> nop
    stream.device()->seek(addressMapper.toFileAddress(0x80185ac4)); stream << PowerPcAsm::nop();
    // or r31,r3,r3                                                               -> or r31,r4,r4
    stream.device()->seek(addressMapper.toFileAddress(0x80185ac8)); stream << PowerPcAsm::or_(31, 4, 4);
    // li r5,0x1                                                                  -> li r5,0x2
    stream.device()->seek(addressMapper.toFileAddress(0x80185b10)); stream << PowerPcAsm::li(5, 2);
    // bl GameSequenceDataAdapter::GetMapOrigin(r3)                               -> nop
    stream.device()->seek(addressMapper.toFileAddress(0x8024b1b8)); stream << PowerPcAsm::nop();
    // bl GameSequenceDataAdapter::GetMapOrigin(r3)                               -> nop
    stream.device()->seek(addressMapper.toFileAddress(0x802498a8));
    for (int i = 0; i < 8; i++) stream << PowerPcAsm::nop();
    // --- Default selected map button in wifi ---
    // since Standard Mode is selected on default, we use this mapset to find the default map
    // TODO need asm hack to determine currently selected MapSet and rather use that ID
    short defaultMap = 0;
    for (short i = 0; i < mapDescriptors.count(); i++) {
        MapDescriptor mapDescriptor = mapDescriptors.at(i);
        if (mapDescriptor.mapSet == 1 && mapDescriptor.zone == 0 && mapDescriptor.order == 0) {
            defaultMap = i;
        }
    }
    // 0x9 = Castle Trodain
    // li r3,0x9                                                                  -> li r3,0x9
    stream.device()->seek(addressMapper.toFileAddress(0x8024afc8)); stream << PowerPcAsm::li(3, defaultMap);
    // TODO 0x13 is some special handling map id. Need to check what is going on with it
    // li r3,0x13                                                                 -> li r3,0x13
    stream.device()->seek(addressMapper.toFileAddress(0x80243ae4)); stream << PowerPcAsm::li(5, 0x13);
    // -- fix text of selected map if the map id is 0x12 or 0x13 --
    // beq                                                                 -> nop
    stream.device()->seek(addressMapper.toFileAddress(0x8023ee28)); stream << PowerPcAsm::nop();
    // beq                                                                 -> nop
    stream.device()->seek(addressMapper.toFileAddress(0x8023ee30)); stream << PowerPcAsm::nop();
    // -- send normal map id instead of bitset of map ids --
    /*
     * The game normally always sends a bitset of mapids over wifi. The reason for that is that in random match making mode
     * you can select preferred maps you like. In this case the netcode sends the bitsets of maps you prefer to make a match
     * with people which have the same maps selected. However, it also sends a bitset of map ids in friend wifi mode, where you
     * select just a single map. Not only that, but it also packs other settings into the bitset as well, thus limiting us
     * to only 16 maps. With this hack we send the id of the map not as a bitset but as the map id itself, breaking compatibility
     * with random match making at the same time.
     */
    stream.device()->seek(addressMapper.toFileAddress(0x8020ef34)); stream << PowerPcAsm::nop();


}

