#include "mapdescriptorwidget.h"

#include <QCheckBox>
#include <QFileDialog>
#include <QHeaderView>
#include <QMessageBox>
#include <QPushButton>
#include <QTextStream>
#include <QSettings>
#include "lib/importexportutils.h"
#include "lib/getordefault.h"
#include "lib/region.h"
#include "lib/progresscanceled.h"
#include "venturecarddialog.h"
#include "csmmprogressdialog.h"

// Internal types for table items
static constexpr int MAP_SET_TYPE = QTableWidgetItem::UserType;
static constexpr int ZONE_TYPE = QTableWidgetItem::UserType + 1;
static constexpr int ORDER_TYPE = QTableWidgetItem::UserType + 2;
static constexpr int UNLOCK_ID_TYPE = QTableWidgetItem::UserType + 3;

MapDescriptorWidget::MapDescriptorWidget(QWidget *parent) : QTableWidget(parent)
{
    setWidgetLabels();

    connect(this, &MapDescriptorWidget::cellChanged, this, [&](int row, int col) {
        auto theItem = item(row, col);

        if (theItem->type() < QTableWidgetItem::UserType) return;

        bool ok;
        int value = theItem->text().toInt(&ok);
        if (!ok) return;
        if (theItem->type() == MAP_SET_TYPE) {
            descriptors[row]->mapSet = value;
        } else if (theItem->type() == ZONE_TYPE) {
            descriptors[row]->zone = value;
        } else if (theItem->type() == ORDER_TYPE) {
            descriptors[row]->order = value;
        } else if (theItem->type() == UNLOCK_ID_TYPE) {
            descriptors[row]->unlockId = value;
        } else {
            return;
        }
        dirty = true;
    });

    setSelectionBehavior(QAbstractItemView::SelectRows);
}

static QTableWidgetItem *readOnlyItem(const QString &str) {
    auto item = new QTableWidgetItem(str);
    item->setFlags(item->flags() & ~Qt::ItemIsEditable);
    return item;
}

void MapDescriptorWidget::loadRowWithMapDescriptor(int row, const MapDescriptor &descriptor) {
    descriptors[row] = QSharedPointer<MapDescriptor>::create(descriptor);
    // Descriptor pointer is so that the button event handlers work properly when descriptors
    // are shifted
    auto descriptorPtr = descriptors[row];
    int colIdx = 0;

    setVerticalHeaderItem(row, readOnlyItem(QString::number(row)));

    auto importYamlButton = new QPushButton(tr("Import"));
    connect(importYamlButton, &QPushButton::clicked, this, [=](bool) {
        auto openYaml = QFileDialog::getOpenFileName(this, tr("Import .yaml or .zip"), QString(), tr("Map Descriptor Files (*.yaml *.zip)"), nullptr);
        if (openYaml.isEmpty()) return;
        MapDescriptor newDescriptor;
        try {
            CSMMProgressDialog dialog(tr("Importing yaml"), QString(), 0, 100, nullptr, Qt::WindowFlags(), true);
            ImportExportUtils::importYaml(openYaml, newDescriptor, getImportDirectory(), [&](double progress) {
                dialog.setValue(100 * progress);
            });
            descriptorPtr->setFromImport(newDescriptor);
            descriptorPtr->mapDescriptorFilePath = openYaml;
            dirty = true;
            loadRowWithMapDescriptor(descriptors.indexOf(descriptorPtr), *descriptorPtr);
        } catch (const ProgressCanceled &) {
            // nothing to do
        } catch (const std::runtime_error &exception) {
            QMessageBox::critical(this, tr("Import .yaml"), QString(tr("Error loading the map: %1")).arg(exception.what()));
        }
    });
    setCellWidget(row, colIdx++, importYamlButton);

    auto exportYamlButton = new QPushButton(tr("Export"));
    connect(exportYamlButton, &QPushButton::clicked, this, [=](bool) {
        auto gameDirectory = getGameDirectory();
        auto saveYamlTo = QFileDialog::getSaveFileName(exportYamlButton, tr("Export .yaml"), descriptorPtr->internalName + ".yaml", tr("Map Descriptor Files (*.yaml)"), nullptr, QFileDialog::DontUseNativeDialog);
        if (saveYamlTo.isEmpty()) return;
        try {
            ImportExportUtils::exportYaml(gameDirectory, saveYamlTo, *descriptorPtr);
        } catch (const std::runtime_error &exception) {
            QMessageBox::critical(this, tr("Export .yaml"), QString(tr("Error exporting the map: %1")).arg(exception.what()));
        }
    });
    setCellWidget(row, colIdx++, exportYamlButton);

    //QString prioritized_language = "en"; // pull from QSettings once we're saving that there.
    QSettings settings;
    QString prioritized_language = Region::instance().getGameCodeFromTerritory(settings.value("preferredGameTerritory").toString());
    if(settings.contains("preferredGameTerritory")){
        qInfo() << QString("settings contains preferredGameTerritory: %1").arg(settings.value("preferredGameTerritory").toString());
    }
    else{
        qInfo() << QString("settings does not contain PreferredGameTerritory, reverting to English");
    }

    qInfo() << QString("MapDescriptorWidget: preferredGameTerritory: %1").arg(settings.value("preferredGameTerritory").toString());
    qInfo() << QString("MapDescriptorWidget: prioritized_language %1").arg(prioritized_language);

    QString en_name = getOrDefault(descriptor.names, "en", QString());
    QString es_name = getOrDefault(descriptor.names, "su", QString());
    QString de_name = getOrDefault(descriptor.names, "de", QString());
    QString fr_name = getOrDefault(descriptor.names, "fr", QString());
    QString it_name = getOrDefault(descriptor.names, "it", QString());
    QString jp_name = getOrDefault(descriptor.names, "jp", QString());

    if(!getOrDefault(descriptor.names, prioritized_language, QString()).isEmpty()){

    }
    else {
        if(!en_name.isEmpty()){
            prioritized_language = "en";
        }
        else if(!es_name.isEmpty()){
            prioritized_language = "su";
        }
        else if(!de_name.isEmpty()){
            prioritized_language = "de";
        }
        else if(!fr_name.isEmpty()){
            prioritized_language = "fr";
        }
        else if(!it_name.isEmpty()){
            prioritized_language = "it";
        }
        else if(!jp_name.isEmpty()){
            prioritized_language = "jp";
        }
    }

    setItem(row, colIdx++, readOnlyItem(getOrDefault(descriptor.names, prioritized_language, QString())));

    setItem(row, colIdx++, new QTableWidgetItem(QString::number(descriptor.mapSet), MAP_SET_TYPE));
    setItem(row, colIdx++, new QTableWidgetItem(QString::number(descriptor.zone), ZONE_TYPE));
    setItem(row, colIdx++, new QTableWidgetItem(QString::number(descriptor.order), ORDER_TYPE));

    auto isPracticeBoardCheck = new QCheckBox();
    isPracticeBoardCheck->setChecked(descriptor.isPracticeBoard);
    connect(isPracticeBoardCheck, &QCheckBox::clicked, this, [=](bool isChecked) {
        descriptorPtr->isPracticeBoard = isChecked;
        dirty = true;
    });
    setCellWidget(row, colIdx++, isPracticeBoardCheck);

    setItem(row, colIdx++, new QTableWidgetItem(QString::number(descriptor.unlockId), UNLOCK_ID_TYPE));

    setItem(row, colIdx++, readOnlyItem(descriptor.ruleSet == Easy ? tr("Easy") : tr("Standard")));
    setItem(row, colIdx++, readOnlyItem(QString::number(descriptor.initialCash)));
    setItem(row, colIdx++, readOnlyItem(QString::number(descriptor.targetAmount)));
    setItem(row, colIdx++, readOnlyItem(QString::number(descriptor.baseSalary)));
    setItem(row, colIdx++, readOnlyItem(QString::number(descriptor.salaryIncrement)));
    setItem(row, colIdx++, readOnlyItem(QString::number(descriptor.maxDiceRoll)));

    QPushButton *ventureButton = new QPushButton(tr("View Venture Cards")); // note: setCellWidget takes control of button
    connect(ventureButton, &QPushButton::clicked, this, [=](bool) {
        (new VentureCardDialog(*descriptorPtr /* maintain descriptor when other descs are removed */, this))->exec();
    });
    setCellWidget(row, colIdx++, ventureButton);

    QStringList frbFilesList;
    for (auto &str: descriptor.frbFiles) {
        if (!str.isEmpty()) {
            frbFilesList.append(str);
        }
    }
    setItem(row, colIdx++, readOnlyItem(frbFilesList.join("; ")));

    QStringList originPointsStrList;
    for (auto &originPoint: descriptor.switchRotationOrigins) originPointsStrList.append(QString(originPoint));
    setItem(row, colIdx++, readOnlyItem(originPointsStrList.join("; ")));

    setItem(row, colIdx++, readOnlyItem(descriptor.theme == Mario ? "Mario" : "DragonQuest"));
    setItem(row, colIdx++, readOnlyItem(descriptor.background));
    setItem(row, colIdx++, readOnlyItem(Bgm::bgmIdToString(descriptor.bgmId)));
    setItem(row, colIdx++, readOnlyItem(descriptor.mapIcon));
    setItem(row, colIdx++, readOnlyItem(QString::number(descriptor.loopingMode)));
    setItem(row, colIdx++, readOnlyItem(QString::number(descriptor.loopingModeRadius)));
    setItem(row, colIdx++, readOnlyItem(QString::number(descriptor.loopingModeHorizontalPadding)));
    setItem(row, colIdx++, readOnlyItem(QString::number(descriptor.loopingModeVerticalSquareCount)));
    setItem(row, colIdx++, readOnlyItem(QString::number(descriptor.tourBankruptcyLimit)));
    setItem(row, colIdx++, readOnlyItem(QString::number(descriptor.tourInitialCash)));

    QStringList tourOpponentsList;
    for (auto character: descriptor.tourCharacters) tourOpponentsList.append(tourCharacterToString(character));
    setItem(row, colIdx++, readOnlyItem(tourOpponentsList.join("; ")));

    setItem(row, colIdx++, readOnlyItem(QString::number(descriptor.tourClearRank)));
    setItem(row, colIdx++, readOnlyItem(QString::number(descriptor.nameMsgId)));
    setItem(row, colIdx++, readOnlyItem(QString::number(descriptor.descMsgId)));

    setItem(row, colIdx++, readOnlyItem(getOrDefault(descriptor.descs, prioritized_language, QString())));
    setItem(row, colIdx++, readOnlyItem(descriptor.internalName));
    auto distNames = getOrDefault(descriptor.districtNames, prioritized_language, std::vector<QString>());
    setItem(row, colIdx++, readOnlyItem(QStringList(distNames.begin(), distNames.end()).join("; ")));
    QStringList districtNameIdStrs;
    for (quint32 v: descriptor.districtNameIds) {
        districtNameIdStrs.push_back(QString::number(v));
    }
    setItem(row, colIdx++, readOnlyItem(districtNameIdStrs.join("; ")));
}

void MapDescriptorWidget::appendMapDescriptor(const MapDescriptor &descriptor) {
    descriptors.append(QSharedPointer<MapDescriptor>::create(descriptor));
    insertRow(descriptors.size()-1);
    loadRowWithMapDescriptor(descriptors.size()-1, descriptor);
    dirty = true;
}

void MapDescriptorWidget::duplicateSelectedMapDescriptors() {
    auto selectedRows = selectionModel()->selectedRows();
    for (auto &selectedRow: selectedRows) {
        appendMapDescriptor(*descriptors[selectedRow.row()]); // TODO handle mutators with care when duplicating
    }
}

void MapDescriptorWidget::removeSelectedMapDescriptors() {
    auto selectedRows = selectionModel()->selectedRows();
    std::sort(selectedRows.begin(), selectedRows.end(), [](const QModelIndex &A, const QModelIndex &B) { return A.row() > B.row(); });
    for (auto &selectedRow: selectedRows) {
        descriptors.remove(selectedRow.row());
        removeRow(selectedRow.row());
        dirty = true;
    }
}

void MapDescriptorWidget::clearDescriptors() {
    clearContents();
    setRowCount(0);
    descriptors.clear();
    dirty = false;
}

void MapDescriptorWidget::retranslate()
{
    // Reapply translated strings for row item buttons (import/export/view venture cards)
    for (int row = 0; row < this->rowCount(); ++row) {
        for (int col = 0; col < this->columnCount(); ++col){
            QWidget* widget = this->cellWidget(row, col);
            if (auto button = qobject_cast<QPushButton*>(widget)) {
                button->setText(tr(button->text().toUtf8().constData()));
            }
        }
    }
    // Reapply translated strings for columns
    for(int col = 0; col < this->columnCount(); ++col){
        auto column = this->horizontalHeaderItem(col)->text();
        this->setHorizontalHeaderItem(col, new QTableWidgetItem(tr(column.toUtf8().constData())));
    }

    setWidgetLabels();
}

void MapDescriptorWidget::setWidgetLabels(){
    QStringList labels{"", "", tr("Name"), tr("MapSet [E]"), tr("Zone [E]"), tr("Order [E]"),
                       tr("Tutorial Map? [E]"), tr("Unlock ID [E]"), tr("Ruleset"), tr("Initial Cash"), tr("Target Amount"),
                       tr("Base Salary"), tr("Salary Increment"), tr("Maximum Dice Roll"),
                       tr("Venture Cards"), tr("FRB Files"), tr("Switch Origin Points"),
                       tr("Board Theme"), tr("Background"), tr("Background Music ID"),
                       tr("Map Icon"), tr("Looping Mode"), tr("Looping Mode Radius"),
                       tr("Looping Mode Horizontal Padding"), tr("Looping Mode Vertical Square Count"),
                       tr("Tour Bankruptcy Limit"), tr("Tour Initial Cash"), tr("Tour Opponents"),
                       tr("Tour Clear Rank"), tr("Name Msg ID"), tr("Desc Msg ID"),
                       tr("Description"), tr("Internal Name"), tr("District Names"), tr("District Name IDs")};
    setColumnCount(labels.size());
    setHorizontalHeaderLabels(labels);
    horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
}

const QVector<QSharedPointer<MapDescriptor>> &MapDescriptorWidget::getDescriptors() {
    return descriptors;
}

void MapDescriptorWidget::setGameDirectoryFunction(const std::function<QString()> &fn) {
    getGameDirectory = fn;
}

void MapDescriptorWidget::setImportDirectoryFunction(const std::function<QString ()> &fn)
{
    getImportDirectory = fn;
}
