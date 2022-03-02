#include "uigame013.h"
#include <brlyt.h>
#include "unicodefilenameutils.h"

namespace Ui_game_013 {

bool widenDistrictName(const QString &brlytFile) {
    bq::brlyt::Brlyt brlyt;

    {
        ufutils::unicode_ifstream stream(brlytFile);
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

    ufutils::unicode_ofstream stream(brlytFile);
    brlyt.write(stream);
    return !stream.fail();
}

}
