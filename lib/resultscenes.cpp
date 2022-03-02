#include "resultscenes.h"
#include <brlyt.h>
#include "unicodefilenameutils.h"

namespace ResultScenes {

bool widenResultTitle(const QString &brlytFile) {
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
        if (txt1) {
            if (txt1->name == "t_m_00") {
                txt1->width = 280;
                txt1->translate.x = 40;
            } else if (txt1->name == "t_m_01" || txt1->name == "t_00" ) {
                txt1->translate.x = 60;
            }
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
