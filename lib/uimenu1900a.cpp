#include "uimenu1900a.h"
#include <brlan.h>
#include <brlyt.h>
#include <QFileInfo>
#include <QSet>
#include <QDebug>
#include "unicodefilenameutils.h"

namespace Ui_menu_19_00a {

static bool isMapTpl(const QString &tpl) {
    return tpl.startsWith("ui_menu007_") && tpl.endsWith(".tpl") && tpl != "ui_menu007_bghatena.tpl";
}

static bool checkMapIconsForValidity(const QMap<QString, QString> &mapIconToTplName) {
    auto values = mapIconToTplName.values();
    for (auto &tpl: values) {
        if (!isMapTpl(tpl)) {
            return false;
        }
    }
    return true;
}

QString constructMapIconTplName(const QString &mapIcon) {
    auto basename = QFileInfo(mapIcon).baseName();
    if (basename == "bghatena") {
        qCritical() << "bghatena is not allowed to be used as mapIcon name!";
        return "";
    }
    return QString("ui_menu007_%1.tpl").arg(basename);
}

bool injectMapIconsLayout(const QString &brlytFile, const QMap<QString, QString> &mapIconToTplName) {
    if (!checkMapIconsForValidity(mapIconToTplName)) return false;

    bq::brlyt::Brlyt brlyt;

    {
        ufutils::unicode_ifstream stream(brlytFile);
        brlyt.read(stream);
        if (!stream) {
            return false;
        }
    }

    // TEXTURES

    auto &texs = brlyt.txl1.textures;
    auto texIt = std::remove_if(texs.begin(), texs.end(), [](const std::string &s) { return isMapTpl(QString::fromStdString(s)); });
    texs.erase(texIt, texs.end());
    auto newTplNames = mapIconToTplName.values();
    for (auto &tplName: qAsConst(newTplNames)) {
        texs.push_back(tplName.toStdString());
    }

    // MATERIALS
    QSet<QString> nonMapIconMaterials;
    auto &mats = brlyt.mat1.materials;
    auto matIsMapIcon = [](const std::shared_ptr<bq::brlyt::Material> &mat) {
        auto &textures = mat->textureMaps;
        return std::find_if(textures.begin(), textures.end(), [](const bq::brlyt::TextureRef &ref) { return isMapTpl(QString::fromStdString(ref.name)); }) != textures.end();
    };
    std::shared_ptr<bq::brlyt::Material> materialTemplate;
    for (auto &mat: mats) {
        if (matIsMapIcon(mat)) {
            materialTemplate = mat;
        } else {
            nonMapIconMaterials.insert(QString::fromStdString(mat->name));
        }
    }
    auto matIt = std::remove_if(mats.begin(), mats.end(), matIsMapIcon);
    mats.erase(matIt, mats.end());

    QMap<QString, std::shared_ptr<bq::brlyt::Material>> mapIconToMaterial;

    for (auto it = mapIconToTplName.begin(); it != mapIconToTplName.end(); ++it) {
        auto newMat = std::make_shared<bq::brlyt::Material>(*materialTemplate);
        newMat->name = it.key().toStdString();
        for (auto &textureRef: newMat->textureMaps) {
            if (isMapTpl(QString::fromStdString(textureRef.name))) {
                textureRef.name = it.value().toStdString();
            }
        }
        mats.push_back(newMat);
        mapIconToMaterial[it.key()] = newMat;
    }

    // PICS
    std::shared_ptr<bq::BasePane> mapIconParent;
    std::shared_ptr<bq::brlyt::Pic1> templatePic;

    auto picIsMapIcon = [&](std::shared_ptr<bq::BasePane> pane) {
        auto pic1 = std::dynamic_pointer_cast<bq::brlyt::Pic1>(pane);
        if (pic1) {
            return !nonMapIconMaterials.contains(QString::fromStdString(pic1->material->name));
        }
        return false;
    };
    std::function<void(std::shared_ptr<bq::BasePane>)> traverse = [&](std::shared_ptr<bq::BasePane> cur) mutable {
        for (auto &nxt: cur->children) {
            if (picIsMapIcon(nxt)) {
                mapIconParent = cur;
                templatePic = std::dynamic_pointer_cast<bq::brlyt::Pic1>(nxt);
                return;
            }
            traverse(nxt);
        }
    };
    traverse(brlyt.rootPane);

    auto picIt = std::remove_if(mapIconParent->children.begin(), mapIconParent->children.end(), picIsMapIcon);
    mapIconParent->children.erase(picIt, mapIconParent->children.end());

    auto mapIcons = mapIconToTplName.keys();
    for (auto &icon: qAsConst(mapIcons)) {
        auto newPic = std::make_shared<bq::brlyt::Pic1>(*templatePic);
        newPic->material = mapIconToMaterial[icon];
        newPic->name = icon.toStdString();
        mapIconParent->children.push_back(newPic);
        newPic->parent = mapIconParent;
    }

    // WRITE FILE

    ufutils::unicode_ofstream stream(brlytFile);
    brlyt.write(stream);
    return !stream.fail();
}

bool injectMapIconsAnimation(const QString &brlanFile, const QMap<QString, QString> &mapIconToTplName) {
    if (!checkMapIconsForValidity(mapIconToTplName)) return false;

    bq::brlan::Brlan brlan;

    {
        ufutils::unicode_ifstream stream(brlanFile);
        brlan.read(stream);
        if (!stream) {
            return false;
        }
    }

    static const QSet<QString> ignorePanes = { "p_rank_1", "n_bg_b_00_anime", "p_rank_2", "n_bg_anime", "n_hatena_anime", "n_new_anime", "p_bg_flash", "w_bg_button_00", "p_rank_1_leaf", "p_rank_2_leaf", "n_rank_leaf", "p_new", "p_rank_1", "p_rank_2", "p_rank_1_leaf", "p_rank_2_leaf" };

    auto isForMapIcon = [&](const bq::brlan::PaiEntry &entry) {
        return !ignorePanes.contains(QString::fromStdString(entry.name));
    };

    auto &entries = brlan.animationInfo.entries;
    bq::brlan::PaiEntry templateEntry;

    for (auto &entry: entries) {
        if (isForMapIcon(entry)) {
            templateEntry = entry;
            break;
        }
    }

    auto entriesIt = std::remove_if(entries.begin(), entries.end(), isForMapIcon);
    entries.erase(entriesIt, entries.end());

    auto mapIcons = mapIconToTplName.keys();
    for (auto &icon: qAsConst(mapIcons)) {
        auto newEntry = templateEntry;
        newEntry.name = icon.toStdString();
        entries.push_back(newEntry);
    }

    ufutils::unicode_ofstream stream(brlanFile);
    brlan.write(stream);
    return !stream.fail();
}

}
