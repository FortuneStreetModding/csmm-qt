#include "resultscenes.h"
#include <brlyt.h>
#include <fstream>
#include <filesystem>

namespace ResultScenes {

bool widenResultTitle(const QString &brlytFile) {
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

    std::ofstream stream(std::filesystem::path(brlytFile.toStdU16String()), std::ios::binary);
    brlyt.write(stream);
    return !stream.fail();
}

}
