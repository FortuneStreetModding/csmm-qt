#include "uigame013.h"
#include <QDataStream>
#include <QFile>

namespace Ui_game_013 {

bool widenDistrictName(const QString &brlytFile) {
#if 0
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
#endif

    QFile brlytFileObj(brlytFile);
    if (brlytFileObj.open(QIODevice::ReadWrite)) {
        QDataStream stream(&brlytFileObj);
        stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
        stream.setByteOrder(QDataStream::BigEndian);

        brlytFileObj.seek(22740); // /xmlyt/tag[@name='t_area01']/size/width
        stream << 300.0f;
        brlytFileObj.seek(22708); // /xmlyt/tag[@name='t_area01']/translate/x
        stream << -20.5f;
        brlytFileObj.seek(22756); // /xmlyt/tag[@name='t_area01']/font/alignment
        stream << quint8(5); // right align x center align y

        return true;
    } else return false;
}

}
