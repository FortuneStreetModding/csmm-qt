#include "uigame013.h"
#include "pugixml/pugixml.hpp"

namespace Ui_game_013 {

bool widenDistrictName(const QString &xmlytFile) {
    pugi::xml_document doc;
    auto result = doc.load_file(xmlytFile.toUtf8());
    if (result.status != pugi::status_ok) {
        return false;
    }

    pugi::xpath_node node;
    node = doc.select_node("/xmlyt/tag[@name='t_area01']/size/width");
    node.node().text().set(300);
    node = doc.select_node("/xmlyt/tag[@name='t_area01']/translate/x");
    node.node().text().set(-20.5);
    node = doc.select_node("/xmlyt/tag[@name='t_area01']/font/alignment/@x");
    node.attribute().set_value("Right");

    return doc.save_file(xmlytFile.toUtf8());
}

}
