#ifndef FSLOCALE_H
#define FSLOCALE_H

#include <QString>

static const QString FS_LOCALES[] = {
    "en", "de", "fr", "it", "jp", "su", "uk"
};
inline QString localeToYamlKey(const QString &locale) {
    if (locale == "su") {
        return "es";
    }
    return locale;
}
inline QString yamlKeyToLocale(const QString &yamlKey) {
    if (yamlKey == "es") {
        return "su";
    }
    return yamlKey;
}
inline QString localeToUpper(const QString &locale) {
    if (locale == "jp") {
        return "";
    }
    if (locale == "su") {
        return "ES";
    }
    return locale.toUpper();
}
template<class MapType>
auto retrieveStr(const MapType &map, const QString &key) -> decltype(map.at(key)) {
    if (map.count(key)) {
        return map.at(key);
    }
    return map.at("en");
}

#endif // FSLOCALE_H
