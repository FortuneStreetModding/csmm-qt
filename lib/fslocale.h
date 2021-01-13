#ifndef FSLOCALE_H
#define FSLOCALE_H

#include <QString>

static const QString FS_LOCALES[] = {
    "en", "de", "fr", "it", "jp", "su", "uk"
};

inline QString localeToUpper(const QString &locale) {
    if (locale == "jp") {
        return "";
    }
    if (locale == "su") {
        return "ES";
    }
    return locale.toUpper();
}

#endif // FSLOCALE_H
