#include "doliotable.h"

void DolIOTable::readAsm(QDataStream &stream, const AddressMapper &addressMapper, std::vector<MapDescriptor> &mapDescriptors) {
    bool isVanilla = readIsVanilla(stream, addressMapper);
#if 0
    qint16 rowCount = readTableRowCount(stream, addressMapper, isVanilla);
    if (rowCount != -1 && rowCount != mapDescriptors.size()) {
        if (isVanilla) {
            // in vanilla all kinds of strange stuff is there. E.g.
            // - there are 42 venture card tables but 48 maps.
            // - there are 48 maps but the ids get mapped to different values (e.g. easy map yoshi island index is 21 but mapped to 18 in some tables and in other tables mapped to 0)
            //     so we cant really figure out the real amount of maps unless doing some complex logic
        } else {
            // should not happen as with the hacks that we apply we streamline the tables and total map count so that they should always map
        }
    }
#endif
    quint32 addr = readTableAddr(stream, addressMapper, isVanilla);
    if (addr) {
        stream.device()->seek(addressMapper.toFileAddress(addr));
    }
    readAsm(stream, mapDescriptors, addressMapper, isVanilla);
}
