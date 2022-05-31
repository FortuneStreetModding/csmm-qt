#include "uigame052.h"
#include <brlyt.h>
#include <fstream>
#include <filesystem>

namespace Ui_game_052
{

bool widenDistrictName(const QString &brlytFile) {
    bq::brlyt::Brlyt brlyt;
    {
        std::ifstream stream(std::filesystem::path(brlytFile.toStdU16String()), std::ios::binary);
        brlyt.read(stream);
        if (!stream) {
            return false;
        }
    }

    std::function<void(std::shared_ptr<bq::BasePane>)> traverse = [&](std::shared_ptr<bq::BasePane> cur) {
        // TODO actually do the widening

        for (auto &nxt: cur->children) {
            traverse(nxt);
        }
    };
    traverse(brlyt.rootPane);

    std::ofstream stream(std::filesystem::path(brlytFile.toStdU16String()), std::ios::binary);
    brlyt.write(stream);
    return !stream.fail();
}

}
