#include "mapdescriptorwidget.h"

#include <QCheckBox>
#include <QFileDialog>
#include <QHeaderView>
#include <QMessageBox>
#include <QPushButton>
#include <QTextStream>
#include "lib/datafileset.h"
#include "lib/patchprocess.h"
#include "venturecarddialog.h"

// Internal types for table items
static constexpr int MAP_SET_TYPE = QTableWidgetItem::UserType;
static constexpr int ZONE_TYPE = QTableWidgetItem::UserType + 1;
static constexpr int ORDER_TYPE = QTableWidgetItem::UserType + 2;

MapDescriptorWidget::MapDescriptorWidget(QWidget *parent) : QTableWidget(parent) {
    QStringList labels{"", "", "Name", "MapSet [Editable]", "Zone [Editable]", "Order [Editable]",
                       "Is Practice Board [Editable]", "Ruleset", "Initial Cash", "Target Amount",
                       "Base Salary", "Salary Increment", "Max. Dice Roll",
                       "Venture Cards", "FRB Files", "Switch Origin Points",
                       "Board Theme", "Background", "Background Music ID",
                       "Map Icon", "Looping Mode", "Looping Mode Radius",
                       "Looping Mode Horiz. Padding", "Looping Mode Vertical Square Count",
                       "Tour Bankruptcy Limit", "Tour Initial Cash", "Tour Opponents",
                       "Tour Clear Rank", "Unlock ID", "Name Msg ID", "Desc Msg ID",
                       "Description", "Internal Name"};
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
        }
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

    auto importMdButton = new QPushButton("Import .md");
    connect(importMdButton, &QPushButton::clicked, this, [=](bool) {
        auto gameDirectory = getGameDirectory();
        auto openMd = QFileDialog::getOpenFileName(this, "Import .md", "", "Map Descriptor Files (*.md)");
        if (openMd.isEmpty()) return;
        MapDescriptor newDescriptor;
        if (PatchProcess::importMd(gameDirectory, openMd, newDescriptor, tmpDir.path())) {
            descriptorPtr->setFromImport(newDescriptor);
            loadRowWithMapDescriptor(descriptors.indexOf(descriptorPtr), *descriptorPtr);
        } else {
            QMessageBox::critical(this, "Import .md", "There was an error loading the .md file");
        }
    });
    setCellWidget(row, colIdx++, importMdButton);

    auto exportMdButton = new QPushButton("Export .md");
    connect(exportMdButton, &QPushButton::clicked, this, [=](bool) {
        auto gameDirectory = getGameDirectory();
        auto saveMdTo = QFileDialog::getSaveFileName(exportMdButton, "Export .md", descriptorPtr->internalName + ".md", "Map Descriptor Files (*.md)");
        if (saveMdTo.isEmpty()) return;
        PatchProcess::exportMd(gameDirectory, saveMdTo, *descriptorPtr);
    });
    setCellWidget(row, colIdx++, exportMdButton);

    setItem(row, colIdx++, readOnlyItem(descriptor.names["en"]));

    setItem(row, colIdx++, new QTableWidgetItem(QString::number(descriptor.mapSet), MAP_SET_TYPE));
    setItem(row, colIdx++, new QTableWidgetItem(QString::number(descriptor.zone), ZONE_TYPE));
    setItem(row, colIdx++, new QTableWidgetItem(QString::number(descriptor.order), ORDER_TYPE));

    auto isPracticeBoardCheck = new QCheckBox();
    isPracticeBoardCheck->setChecked(descriptor.isPracticeBoard);
    connect(isPracticeBoardCheck, &QCheckBox::clicked, this, [&](bool isChecked) {
        descriptorPtr->isPracticeBoard = isChecked;
    });
    setCellWidget(row, colIdx++, isPracticeBoardCheck);

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
    setItem(row, colIdx++, readOnlyItem(QString::number(descriptor.bgmId)));
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
    setItem(row, colIdx++, readOnlyItem(QString::number(descriptor.unlockId)));
    setItem(row, colIdx++, readOnlyItem(QString::number(descriptor.nameMsgId)));
    setItem(row, colIdx++, readOnlyItem(QString::number(descriptor.descMsgId)));

    setItem(row, colIdx++, readOnlyItem(descriptor.descs["en"]));
    setItem(row, colIdx++, readOnlyItem(descriptor.internalName));
}

void MapDescriptorWidget::appendMapDescriptor(const MapDescriptor &descriptor) {
    descriptors.append(QSharedPointer<MapDescriptor>::create(descriptor));
    insertRow(descriptors.size()-1);
    loadRowWithMapDescriptor(descriptors.size()-1, descriptor);
}

void MapDescriptorWidget::removeSelectedMapDescriptors() {
    auto selectedRows = selectionModel()->selectedRows();
    std::sort(selectedRows.begin(), selectedRows.end(), [](const QModelIndex &A, const QModelIndex &B) { return A.row() > B.row(); });
    for (auto &selectedRow: selectedRows) {
        descriptors.remove(selectedRow.row());
        removeRow(selectedRow.row());
    }
}

void MapDescriptorWidget::clearDescriptors() {
    clearContents();
    setRowCount(0);
    descriptors.clear();
}

const QVector<QSharedPointer<MapDescriptor>> &MapDescriptorWidget::getDescriptors() {
    return descriptors;
}

void MapDescriptorWidget::setGameDirectoryFunction(const std::function<QString()> &fn) {
    getGameDirectory = fn;
}

const QTemporaryDir &MapDescriptorWidget::getTmpResourcesDir() {
    return tmpDir;
}
