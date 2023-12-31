#include "mapdescriptorwidget.h"

#include <QCheckBox>
#include <QFileDialog>
#include <QHeaderView>
#include <QMessageBox>
#include <QPushButton>
#include <QTextStream>
#include "lib/datafileset.h"
#include "lib/importexportutils.h"
#include "lib/getordefault.h"
#include "lib/progresscanceled.h"
#include "venturecarddialog.h"
#include "csmmprogressdialog.h"

// Internal types for table items
static constexpr int MAP_SET_TYPE = QTableWidgetItem::UserType;
static constexpr int ZONE_TYPE = QTableWidgetItem::UserType + 1;
static constexpr int ORDER_TYPE = QTableWidgetItem::UserType + 2;
static constexpr int UNLOCK_ID_TYPE = QTableWidgetItem::UserType + 3;

MapDescriptorWidget::MapDescriptorWidget(QWidget *parent) : QTableWidget(parent) {
    QStringList labels{"", "", "Name", "MapSet [Editable]", "Zone [Editable]", "Order [Editable]",
                       "Is Practice Board [Editable]", "Unlock ID [Editable]", "Ruleset", "Initial Cash", "Target Amount",
                       "Base Salary", "Salary Increment", "Max. Dice Roll",
                       "Venture Cards", "FRB Files", "Switch Origin Points",
                       "Board Theme", "Background", "Background Music ID",
                       "Map Icon", "Looping Mode", "Looping Mode Radius",
                       "Looping Mode Horiz. Padding", "Looping Mode Vertical Square Count",
                       "Tour Bankruptcy Limit", "Tour Initial Cash", "Tour Opponents",
                       "Tour Clear Rank", "Name Msg ID", "Desc Msg ID",
                       "Description", "Internal Name", "District Names", "District Name IDs"};
    setColumnCount(labels.size());
    setHorizontalHeaderLabels(labels);
    horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

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

    auto importYamlButton = new QPushButton("Import .yaml or .zip");
    connect(importYamlButton, &QPushButton::clicked, this, [=](bool) {
        auto openYaml = QFileDialog::getOpenFileName(this, "Import .yaml or .zip", QString(), "Map Descriptor Files (*.yaml *.zip)");
        if (openYaml.isEmpty()) return;
        MapDescriptor newDescriptor;
        try {
            CSMMProgressDialog dialog("Importing yaml", QString(), 0, 100, nullptr, Qt::WindowFlags(), true);
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
            QMessageBox::critical(this, "Import .yaml", QString("Error loading the map: %1").arg(exception.what()));
        }
    });
    setCellWidget(row, colIdx++, importYamlButton);

    auto exportYamlButton = new QPushButton("Export .yaml");
    connect(exportYamlButton, &QPushButton::clicked, this, [=](bool) {
        auto gameDirectory = getGameDirectory();
        auto saveYamlTo = QFileDialog::getSaveFileName(exportYamlButton, "Export .yaml", descriptorPtr->internalName + ".yaml", "Map Descriptor Files (*.yaml)");
        if (saveYamlTo.isEmpty()) return;
        try {
            ImportExportUtils::exportYaml(gameDirectory, saveYamlTo, *descriptorPtr);
        } catch (const std::runtime_error &exception) {
            QMessageBox::critical(this, "Export .yaml", QString("Error exporting the map: %1").arg(exception.what()));
        }
    });
    setCellWidget(row, colIdx++, exportYamlButton);

    setItem(row, colIdx++, readOnlyItem(getOrDefault(descriptor.names, "en", QString())));

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

    setItem(row, colIdx++, readOnlyItem(descriptor.ruleSet == Easy ? "Easy" : "Standard"));
    setItem(row, colIdx++, readOnlyItem(QString::number(descriptor.initialCash)));
    setItem(row, colIdx++, readOnlyItem(QString::number(descriptor.targetAmount)));
    setItem(row, colIdx++, readOnlyItem(QString::number(descriptor.baseSalary)));
    setItem(row, colIdx++, readOnlyItem(QString::number(descriptor.salaryIncrement)));
    setItem(row, colIdx++, readOnlyItem(QString::number(descriptor.maxDiceRoll)));

    QPushButton *ventureButton = new QPushButton("View Venture Cards"); // note: setCellWidget takes control of button
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

    setItem(row, colIdx++, readOnlyItem(getOrDefault(descriptor.descs, "en", QString())));
    setItem(row, colIdx++, readOnlyItem(descriptor.internalName));
    auto distNames = getOrDefault(descriptor.districtNames, "en", std::vector<QString>());
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
