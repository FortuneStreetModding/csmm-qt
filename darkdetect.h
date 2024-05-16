#ifndef DARKDETECT_H
#define DARKDETECT_H

#include "qapplication.h"
#include "qjsondocument.h"
#include "qpalette.h"
#include "qsettings.h"
#include "usersettings.h"

inline void initWindowPaletteSettings(Qt::ColorScheme scheme) {
    QSettings settings;
    QString saved_palette = settings.value("window_palette/name", "not set").toString();
    if(saved_palette == "not set"){
        // We set these two palettes in like this to ensure the
        // system works even if the user never interacts with the
        // theme system. In this scenario, it will automatically
        // set either Classic Light or Classic Dark on the first
        // boot of this version based on what the user's OS is
        // set to. In every subsequent boot, if the user does not
        // change the theme, they will skip straight to the
        // setChosenPalette() call below to load whichever palette
        // got set here during the first boot.
        QJsonDocument light_doc = readJsonFile(":/palettes/", "classic_light.json");
        QJsonObject light_root_obj = light_doc.object();
        QJsonObject light_palette = light_root_obj.value("colors").toObject();

        QJsonDocument dark_doc = readJsonFile(":/palettes/", "classic_dark.json");
        QJsonObject dark_root_obj = dark_doc.object();
        QJsonObject dark_palette = dark_root_obj.value("colors").toObject();

        if(scheme == Qt::ColorScheme::Light){
            setChosenPalette(light_palette);
            saveUserWindowPalette("Classic Light", light_palette);
        }
        else if(scheme == Qt::ColorScheme::Dark){
            setChosenPalette(dark_palette);
            saveUserWindowPalette("Classic Dark", dark_palette);
        }
        else{
            // if it's unknown, just set Classic Dark
            setChosenPalette(dark_palette);
            saveUserWindowPalette("Classic Dark", dark_palette);
        }
    }
    else{
        // if a palette is set
        bool useHighlightColors = settings.value("window_palette/use_highlight_colors", 0).toBool();
        setChosenPalette(getSavedUserWindowPalette(), useHighlightColors);
    }
}

inline bool isDarkMode() {
    return QApplication::palette().color(QPalette::Base).lightnessF() < 0.5;
}

#endif // DARKDETECT_H
