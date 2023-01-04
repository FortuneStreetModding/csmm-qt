#include "uigame052.h"
#include <brlyt.h>
#include <fstream>
#include <filesystem>
#include <functional>

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
        float enlargeBy = 40;
        if (cur->name == "cw_waku03") { // district name pane
            cur->width = 70 + enlargeBy;
            cur->translate.x = 5 + enlargeBy/2;
        } else if (cur->name == "cw_waku04") { // 04-06 are panes to the right
            cur->translate.x = 70 + enlargeBy;
        } else if (cur->name == "cw_waku05") {
            cur->translate.x = 140 + enlargeBy;
        } else if (cur->name == "cw_waku06") {
            cur->translate.x = 230 + enlargeBy;
        } else if (cur->name == "w_player_01_04") { // I think this and w_player_01_05 deal with interior visual elements of cw_waku03
            cur->width = 70 + enlargeBy;
        } else if (cur->name == "w_player_01_05") {
            cur->width = 66 + enlargeBy;
        } else if (cur->name.rfind("t_area_", 0) == 0) { // check if name starts with t_area_ for the textboxes
            cur->width = 62 + enlargeBy;
            cur->translate.x = 5 + enlargeBy/2;
        }

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
