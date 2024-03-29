#ifndef UIMENU1900A_H
#define UIMENU1900A_H

#include <QFile>
#include <QMap>
#include <QString>

namespace Ui_menu_19_00a {
    QString constructMapIconTplName(const QString &mapIcon);
    bool injectMapIconsLayout(const QString &brlytFile, const QMap<QString, QString> &mapIconToTplName);
    bool injectMapIconsAnimation(const QString &brlanFile, const QMap<QString, QString> &mapIconToTplName);
}

#endif // UIMENU1900A_H
