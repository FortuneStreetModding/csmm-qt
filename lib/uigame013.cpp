#include "uigame013.h"
#include <brlyt.h>
#include <fstream>
#include <filesystem>
#include <functional>

namespace Ui_game_013 {

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
        auto txt1 = std::dynamic_pointer_cast<bq::brlyt::Txt1>(cur);
        if (txt1 && txt1->name == "t_area01") {
            txt1->width = 300;
            txt1->translate.x = -20.5;
            // textAlign%3 <- 2 (right align)
            txt1->textAlign = 3 * (txt1->textAlign / 3) + 2;
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
