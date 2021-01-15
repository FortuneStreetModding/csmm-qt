#include "uimenu1900a.h"

#include <QFileInfo>
#include <QSet>
#include <QTemporaryFile>
#include "pugixml/pugixml.hpp"

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
        return ""; // TODO alert that bghatena is a bad basename
    }
    return QString("ui_menu007_%1.tpl").arg(basename);
}

bool injectMapIconsLayout(const QString &xmlytFile, const QMap<QString, QString> &mapIconToTplName) {
    if (!checkMapIconsForValidity(mapIconToTplName)) return false;
    pugi::xml_document doc;
    auto result = doc.load_file(xmlytFile.toUtf8());
    if (result.status != pugi::status_ok) {
        return false;
    }

    auto tplEntriesNode = doc.select_node("/xmlyt/tag[@type='txl1']/entries");
    auto tplEntryNameList = doc.select_nodes("/xmlyt/tag[@type='txl1']/entries/name");
    for (auto &tplEntryName: tplEntryNameList) {
        if (isMapTpl(tplEntryName.node().value())) {
            tplEntriesNode.node().remove_child(tplEntryName.node());
        }
    }
    auto values = mapIconToTplName.values();
    for (auto &tpl: values) {
        tplEntriesNode.node().append_child("name").set_value(tpl.toUtf8());
    }

    auto materialNodesTexture = doc.select_nodes("/xmlyt/tag[@type='mat1']/entries/texture");
    pugi::xml_node templateMaterialNode;
    QSet<QString> nonMapIconMatNames;
    for (auto materialNodeTexture: materialNodesTexture) {
        auto materialNode = materialNodeTexture.parent();
        auto tplName = materialNodeTexture.node().attribute("name").value();
        if (isMapTpl(tplName)) {
            if (templateMaterialNode.type() == pugi::node_null) {
                templateMaterialNode = materialNode;
            }
            materialNode.parent().remove_child(materialNode);
        } else {
            nonMapIconMatNames.insert(materialNode.attribute("name").value());
        }
    }
    auto matEntriesNode = doc.select_node("/xmlyt/tag[@type='mat1']");
    for (auto it=mapIconToTplName.begin(); it!=mapIconToTplName.end(); ++it) {
        auto newElem = matEntriesNode.node().prepend_copy(templateMaterialNode);
        newElem.attribute("name").set_value(it.key().toUtf8().data());
        newElem.child("texture").attribute("name").set_value(it.value().toUtf8().data());
    }

    auto picNodesMaterial = doc.select_nodes("/xmlyt/tag[@type='pic1']/material");
    pugi::xml_node templatePicNode;
    pugi::xml_node insertionPoint;
    for (auto picNodeMaterial: picNodesMaterial) {
        auto picNode = picNodeMaterial.parent();
        auto matName = picNodeMaterial.node().attribute("name").value();
        if (!nonMapIconMatNames.contains(matName)) {
            if (insertionPoint.type() == pugi::node_null) {
                insertionPoint = picNode.previous_sibling();
            }
            if (templatePicNode.type() == pugi::node_null) {
                templatePicNode = picNode;
            }
            picNode.parent().remove_child(picNode);
        }
    }
    for (auto &mapIcon: mapIconToTplName) {
        auto newElem = insertionPoint.append_copy(templatePicNode);
        newElem.attribute("name").set_value(mapIcon.toUtf8().data());
        newElem.child("material").attribute("name").set_value(mapIcon.toUtf8().data());
    }

    return doc.save_file(xmlytFile.toUtf8());
}

bool injectMapIconsAnimation(const QString &xmlanFile, const QMap<QString, QString> &mapIconToTplName) {
    if (!checkMapIconsForValidity(mapIconToTplName)) return false;
    pugi::xml_document doc;
    auto result = doc.load_file(xmlanFile.toUtf8());
    if (result.status != pugi::status_ok) {
        return false;
    }

    static const QSet<QString> ignorePanes = { "p_rank_1", "n_bg_b_00_anime", "p_rank_2", "n_bg_anime", "n_hatena_anime", "n_new_anime", "p_bg_flash", "w_bg_button_00", "p_rank_1_leaf", "p_rank_2_leaf", "n_rank_leaf", "p_new", "p_rank_1", "p_rank_2", "p_rank_1_leaf", "p_rank_2_leaf" };

    auto paneNodes = doc.select_nodes("/xmlan/pai1/pane");
    pugi::xml_node animationPaneTemplate;
    for (auto paneNode: paneNodes) {
        if (!ignorePanes.contains(paneNode.node().attribute("name").value())) {
            if (animationPaneTemplate.type() == pugi::node_null) {
                animationPaneTemplate = paneNode.node();
            }
            paneNode.parent().remove_child(paneNode.node());
        }
    }
    auto animationEntriesRefNode = doc.select_node("/xmlan/pai1/pane[@name='n_rank_leaf']");
    auto animationEntriesNodeParent = doc.select_node("/xmlan/pai1");
    for (auto &mapIcon: mapIconToTplName) {
        auto newElem = animationEntriesNodeParent.node().insert_copy_after(animationPaneTemplate, animationEntriesRefNode.node());
        newElem.attribute("name").set_value(mapIcon.toUtf8().data());
    }

    return doc.save_file(xmlanFile.toUtf8());
}

}
