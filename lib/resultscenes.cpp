#include "resultscenes.h"
#include "pugixml/pugixml.hpp"

namespace ResultScenes {

bool widenResultTitle(const QString &xmlytFile) {
    pugi::xml_document doc;
    auto result = doc.load_file(xmlytFile.toUtf8());
    if (result.status != pugi::status_ok) {
        return false;
    }

    pugi::xpath_node node;
    node = doc.select_node("/xmlyt/tag[@name='t_m_00']/size/width");
    node.node().text().set(280);
    node = doc.select_node("/xmlyt/tag[@name='t_m_00']/translate/x");
    node.node().text().set(node.node().text().as_double() + 50);

    node = doc.select_node("/xmlyt/tag[@name='t_m_01' or @name='t_00']/translate/x");
    node.node().text().set(node.node().text().as_double() + 50);

    return doc.save_file(xmlytFile.toUtf8());
}

}
