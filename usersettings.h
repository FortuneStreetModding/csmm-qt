#ifndef USERSETTINGS_H
#define USERSETTINGS_H

#include "qsettings.h"
#include <QApplication>
#include <QJsonObject>
#include <QPalette>
#include <QObject>
#include <QStyleFactory>
#include <QJsonDocument>
#include <QIODevice>
#include <QFile>//

class UserSettings : public QObject
{
    Q_OBJECT
public:
    explicit UserSettings(QObject *parent = nullptr);
};

inline void setChosenPalette(QJsonObject colors){
    // create the palette from the colors in the QJsonObject
    QPalette palette;
    palette.setColor(QPalette::Window, colors.value("window").toString());
    palette.setColor(QPalette::WindowText, colors.value("window_text").toString());
    palette.setColor(QPalette::Base, colors.value("base").toString());
    palette.setColor(QPalette::AlternateBase, colors.value("alternate_base").toString());
    palette.setColor(QPalette::Text, colors.value("text").toString());

    palette.setColor(QPalette::Button, colors.value("button").toString());
    palette.setColor(QPalette::ButtonText, colors.value("button_text").toString());
    palette.setColor(QPalette::BrightText, colors.value("bright_text").toString());
    palette.setColor(QPalette::Link, colors.value("link").toString());
    palette.setColor(QPalette::Highlight, colors.value("highlight").toString());
    palette.setColor(QPalette::HighlightedText, colors.value("highlighted_text").toString());
    palette.setColor(QPalette::ToolTipBase, colors.value("tooltip_base").toString());
    palette.setColor(QPalette::ToolTipText, colors.value("tooltip_text").toString());

    palette.setColor(QPalette::Disabled, QPalette::ButtonText, colors.value("disabled_button").toString());
    palette.setColor(QPalette::Disabled, QPalette::Text, colors.value("disabled_text").toString());
    palette.setColor(QPalette::Disabled, QPalette::HighlightedText, colors.value("disabled_highlighted_text").toString());

    // apply the palette immediately
    qApp->setStyle(QStyleFactory::create("Fusion"));
    qApp->setPalette(palette);
    qApp->setStyleSheet("QToolTip { color: #ffffff; background-color: #2a82da; border: 1px solid white; }");
}

inline void saveUserWindowPalette(QString paletteName, QJsonObject paletteColors){
    QSettings settings;
    // this group adds a prefix to all of the values below, resulting in path/to/keys
    // like window_palette/colors/window.
    settings.beginGroup("window_palette");

    // save the palette name
    settings.setValue("name", paletteName);

    // save the palette colors. if we add keys to our palette schema, we will need
    // to add entries here and in returnPalettePreference().
    settings.setValue("colors/window", paletteColors.value("window"));
    settings.setValue("colors/window_text", paletteColors.value("window_text"));
    settings.setValue("colors/base", paletteColors.value("base"));
    settings.setValue("colors/alternate_base", paletteColors.value("alternate_base"));
    settings.setValue("colors/text", paletteColors.value("text"));

    settings.setValue("colors/button", paletteColors.value("button"));
    settings.setValue("colors/button_text", paletteColors.value("button_text"));
    settings.setValue("colors/bright_text", paletteColors.value("bright_text"));

    settings.setValue("colors/link", paletteColors.value("link"));
    settings.setValue("colors/highlight", paletteColors.value("highlight"));
    settings.setValue("colors/highlighted_text", paletteColors.value("highlighted_text"));

    settings.setValue("colors/tooltip_base", paletteColors.value("tooltip_base"));
    settings.setValue("colors/tooltip_text", paletteColors.value("tooltip_text"));

    settings.setValue("colors/disabled_button", paletteColors.value("disabled_button"));
    settings.setValue("colors/disabled_highlighted_text", paletteColors.value("disabled_highlighted_text"));
    settings.setValue("colors/disabled_text", paletteColors.value("disabled_text"));
    settings.endGroup();
}

inline QJsonObject getSavedUserWindowPalette(){
    QSettings settings;
    QJsonObject palette_colors;

    palette_colors.insert("name",settings.value("window_palette/name").toString());

    palette_colors.insert("window", settings.value("window_palette/colors/window").toString());
    palette_colors.insert("window_text", settings.value("window_palette/colors/window_text").toString());
    palette_colors.insert("base", settings.value("window_palette/colors/base").toString());
    palette_colors.insert("alternate_base", settings.value("window_palette/colors/alternate_base").toString());
    palette_colors.insert("text", settings.value("window_palette/colors/text").toString());

    palette_colors.insert("button", settings.value("window_palette/colors/button").toString());
    palette_colors.insert("button_text", settings.value("window_palette/colors/button_text").toString());
    palette_colors.insert("bright_text", settings.value("window_palette/colors/bright_text").toString());

    palette_colors.insert("link", settings.value("window_palette/colors/link").toString());
    palette_colors.insert("highlight", settings.value("window_palette/colors/highlight").toString());
    palette_colors.insert("highlighted_text", settings.value("window_palette/colors/highlighted_text").toString());

    palette_colors.insert("tooltip_base", settings.value("window_palette/colors/tooltip_base").toString());
    palette_colors.insert("tooltip_text", settings.value("window_palette/colors/tooltip_text").toString());

    palette_colors.insert("disabled_button", settings.value("window_palette/colors/disabled_button").toString());
    palette_colors.insert("disabled_highlighted_text", settings.value("window_palette/colors/disabled_highlighted_text").toString());
    palette_colors.insert("disabled_text", settings.value("window_palette/colors/disabled_text").toString());

    return palette_colors;
}

inline QJsonDocument readJsonFile(QString path, QString fileName) {
    QFile file(path + fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Failed to open file:" << path + fileName;
    }

    // read JSON data from file
    QByteArray jsonData = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(jsonData);
    if (doc.isNull()) {
        qDebug() << "Failed to create JSON document from data";
    }
    file.close();
    return doc;
}

#endif // USERSETTINGS_H
