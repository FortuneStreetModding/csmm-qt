#include "displaymapinresults.h"
#include "lib/datafileset.h"
#include "lib/fslocale.h"
#include "lib/powerpcasm.h"
#include "lib/resultscenes.h"

static QVector<quint32> writeDesiredTextSubroutine(quint32 location, const AddressMapper &addressMapper);

void DisplayMapInResults::readAsm(QDataStream &, const AddressMapper &, std::vector<MapDescriptor> &) {
    // nothing to do
}

void DisplayMapInResults::writeAsm(QDataStream &stream, const AddressMapper &addressMapper, const std::vector<MapDescriptor> &mapDescriptors) {
    (void) mapDescriptors;

    // subroutine location not known yet so we start w/ 0
    auto textSub = writeDesiredTextSubroutine(0, addressMapper);
    quint32 textSubAddr = allocate(textSub, "Results screen map text subroutine");
    // we now know the location of the subroutine so we rewrite it w/ the location
    textSub = writeDesiredTextSubroutine(textSubAddr, addressMapper);
    stream.device()->seek(addressMapper.toFileAddress(textSubAddr));
    for (auto word: std::as_const(textSub)) stream << word;

    stream.device()->seek(addressMapper.boomToFileAddress(0x801d402c)); // 1st page of results
    stream << PowerPcAsm::bl(addressMapper.boomStreetToStandard(0x801d402c), textSubAddr);
    stream.device()->seek(addressMapper.boomToFileAddress(0x801d8a80)); // 3rd page of results
    stream << PowerPcAsm::bl(addressMapper.boomStreetToStandard(0x801d8a80), textSubAddr);
}

static QVector<quint32> writeDesiredTextSubroutine(quint32 location, const AddressMapper &addressMapper) {
    constexpr int N = 0x20; // the stack size

    // r4 will have the map name id.
    // The values of r0, r3, and r5 will be unchanged after this function is invoked.

    QVector<quint32> asm_;

    asm_.append(PowerPcAsm::stwu(1, -N, 1)); // move stack pointer
    asm_.append(PowerPcAsm::mflr(11)); // store link register value in r11
    asm_.append(PowerPcAsm::stw(11, N+4, 1)); // put link register value in stack
    asm_.append(PowerPcAsm::stw(0, N-4, 1)); // save r0, etc. to stack
    asm_.append(PowerPcAsm::stw(3, N-8, 1));
    asm_.append(PowerPcAsm::stw(5, N-12, 1));
    asm_.append(PowerPcAsm::bl(location, asm_.size(), addressMapper.boomStreetToStandard(0x8020cd78))); // r3 <- GetPlayMap()
    asm_.append(PowerPcAsm::bl(location, asm_.size(), addressMapper.boomStreetToStandard(0x801cca6c))); // r3 <- GetStageNameId(r3)
    asm_.append(PowerPcAsm::or_(4, 3, 3)); // r4 <- r3
    asm_.append(PowerPcAsm::lwz(5, N-12, 1)); // pop r0, etc. from stack to restore values
    asm_.append(PowerPcAsm::lwz(3, N-8, 1));
    asm_.append(PowerPcAsm::lwz(0, N-4, 1));
    asm_.append(PowerPcAsm::lwz(11, N+4, 1)); // restore link register value from stack
    asm_.append(PowerPcAsm::addi(1, 1, N)); // reset stack pointer
    asm_.append(PowerPcAsm::mtlr(11)); // restore link register for return
    asm_.append(PowerPcAsm::blr()); // return

    return asm_;
}


QMap<QString, ArcFileInterface::ModifyArcFunction> DisplayMapInResults::modifyArcFile()
{
    QMap<QString, ArcFileInterface::ModifyArcFunction> result;
    for (auto &locale : FS_LOCALES) {
        result[gameSequenceResultArc(locale)] = [](const QString &, GameInstance *, const ModListType &, const QString &tmpDir) {
            auto brlytFile = QDir(tmpDir).filePath("arc/blyt/ui_menu_011_scene.brlyt");

            ResultScenes::widenResultTitle(brlytFile);

            brlytFile = QDir(tmpDir).filePath("arc/blyt/ui_game_049_scene.brlyt");

            ResultScenes::widenResultTitle(brlytFile);
        };
    }
    return result;
}
