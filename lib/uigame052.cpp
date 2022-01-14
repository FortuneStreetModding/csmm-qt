#include "uigame052.h"
#include <QDataStream>
#include <QFile>
#include "pugixml/pugixml.hpp"

namespace Ui_game_052
{

bool widenDistrictName(const QString &xmlytFile) {
    pugi::xml_document doc;
    auto result = doc.load_file(xmlytFile.toUtf8());
    if (result.status != pugi::status_ok) {
        return false;
    }

    return doc.save_file(xmlytFile.toUtf8());
}

}
