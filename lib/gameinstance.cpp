#include "gameinstance.h"

#include "lib/await.h"
#include "lib/datafileset.h"
#include "lib/exewrapper.h"
#include "lib/powerpcasm.h"

GameInstance::GameInstance(const std::vector<MapDescriptor> &descriptors, const AddressMapper &addressMapper, const FreeSpaceManager &freeSpaceManager)
    : descriptors(descriptors), mapper(addressMapper), fsm(freeSpaceManager), curUiMessageId(MIN_CSMM_UI_MESSAGE_ID) {
}

std::vector<MapDescriptor> &GameInstance::mapDescriptors() {
    return descriptors;
}

const std::vector<MapDescriptor> &GameInstance::mapDescriptors() const {
    return descriptors;
}

const AddressMapper &GameInstance::addressMapper() const {
    return mapper;
}

FreeSpaceManager &GameInstance::freeSpaceManager() {
    return fsm;
}

const FreeSpaceManager &GameInstance::freeSpaceManager() const {
    return fsm;
}

int GameInstance::nextUiMessageId() {
    return curUiMessageId++;
}

GameInstance GameInstance::fromGameDirectory(const QString &dir, const std::vector<MapDescriptor> &descriptors)
{
    QString mainDol = QDir(dir).filePath(MAIN_DOL);
    auto fileMappingSections = await(ExeWrapper::readSections(mainDol));
    AddressMapper addressMapperVal(fileMappingSections);
    QFile mainDolFile(mainDol);
    if (mainDolFile.open(QFile::ReadOnly)) {
        QDataStream stream(&mainDolFile);
        stream.device()->seek(addressMapperVal.toFileAddress(0x8007a314));
        quint32 inst;
        stream >> inst;
        if (inst == PowerPcAsm::lwz(0, -0x547c, 13)) {
            // Boom Street detected
            addressMapperVal.setVersionMapper(AddressSectionMapper({ AddressSection() }), GameVersion::BOOM);
        } else {
            // Fortune Street
            stream.device()->seek(addressMapperVal.toFileAddress(0x8007a2c0));
            // check PowerPcAsm::lwz(0, -0x547c, 13)?
            addressMapperVal.setVersionMapper(AddressSectionMapper({
                {0x80000100, 0x8007a283, 0x0, ".text, .data0, .data1 and beginning of .text1 until InitSoftLanguage"},
                {0x8007a2f4, 0x80268717, 0x54, "continuation of .text1 until AIRegisterDMACallback"},
                {0x80268720, 0x8040d97b, 0x50, "continuation of .text1"},
                {0x8040d980, 0x8041027f, 0x40, ".data2, .data3 and beginning of .data4 until Boom Street / Fortune Street strings"},
                {0x804105f0, 0x8044ebe7, 0x188, "continuation of .data4"},
                {0x8044ec00, 0x804ac804, 0x1A0, ".data5"},
                {0x804ac880, 0x8081f013, 0x200, ".uninitialized0, .data6, .uninitialized1, .data7, .uninitialized2"}
            }), GameVersion::FORTUNE);
        }
    }
    return GameInstance(descriptors, addressMapperVal, FreeSpaceManager());
}
