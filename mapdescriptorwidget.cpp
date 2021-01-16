#include "mapdescriptorwidget.h"

#include <QHeaderView>
#include <QPushButton>
#include "venturecarddialog.h"

MapDescriptorWidget::MapDescriptorWidget(QWidget *parent) : QTableWidget(parent) {
    QStringList labels{"", "", "Name", "MapSet", "Zone", "Order",
                       "Is Practice Board", "Ruleset", "Initial Cash", "Target Amount",
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
    setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void MapDescriptorWidget::loadRowWithMapDescriptor(int row, const MapDescriptor &descriptor) {
    descriptors[row] = QSharedPointer<MapDescriptor>::create(descriptor);
    auto descriptorPtr = descriptors[row];
    int colIdx = 0;

    QPushButton *importMdButton = new QPushButton("Import .md");
    // TODO implement importMd
    setCellWidget(row, colIdx++, importMdButton);

    QPushButton *exportMdButton = new QPushButton("Export .md");
    // TODO implement exportMd
    setCellWidget(row, colIdx++, exportMdButton);

    setItem(row, colIdx++, new QTableWidgetItem(descriptor.names["en"]));

    setItem(row, colIdx++, new QTableWidgetItem(QString::number(descriptor.mapSet)));
    setItem(row, colIdx++, new QTableWidgetItem(QString::number(descriptor.zone)));
    setItem(row, colIdx++, new QTableWidgetItem(QString::number(descriptor.order)));
    setItem(row, colIdx++, new QTableWidgetItem(QString::number(descriptor.isPracticeBoard)));
    setItem(row, colIdx++, new QTableWidgetItem(QString::number(descriptor.ruleSet)));
    setItem(row, colIdx++, new QTableWidgetItem(QString::number(descriptor.initialCash)));
    setItem(row, colIdx++, new QTableWidgetItem(QString::number(descriptor.targetAmount)));
    setItem(row, colIdx++, new QTableWidgetItem(QString::number(descriptor.baseSalary)));
    setItem(row, colIdx++, new QTableWidgetItem(QString::number(descriptor.salaryIncrement)));
    setItem(row, colIdx++, new QTableWidgetItem(QString::number(descriptor.maxDiceRoll)));

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
    setItem(row, colIdx++, new QTableWidgetItem(frbFilesList.join("; ")));

    QStringList originPointsStrList;
    for (auto &originPoint: descriptor.switchRotationOrigins) originPointsStrList.append(QString(originPoint));
    setItem(row, colIdx++, new QTableWidgetItem(originPointsStrList.join("; ")));

    setItem(row, colIdx++, new QTableWidgetItem(QString::number(descriptor.theme)));
    setItem(row, colIdx++, new QTableWidgetItem(descriptor.background));
    setItem(row, colIdx++, new QTableWidgetItem(QString::number(descriptor.bgmId)));
    setItem(row, colIdx++, new QTableWidgetItem(descriptor.mapIcon));
    setItem(row, colIdx++, new QTableWidgetItem(QString::number(descriptor.loopingMode)));
    setItem(row, colIdx++, new QTableWidgetItem(QString::number(descriptor.loopingModeRadius)));
    setItem(row, colIdx++, new QTableWidgetItem(QString::number(descriptor.loopingModeHorizontalPadding)));
    setItem(row, colIdx++, new QTableWidgetItem(QString::number(descriptor.loopingModeVerticalSquareCount)));
    setItem(row, colIdx++, new QTableWidgetItem(QString::number(descriptor.tourBankruptcyLimit)));
    setItem(row, colIdx++, new QTableWidgetItem(QString::number(descriptor.tourInitialCash)));

    QStringList tourOpponentsList;
    for (auto character: descriptor.tourCharacters) tourOpponentsList.append(tourCharacterToString(character));
    setItem(row, colIdx++, new QTableWidgetItem(tourOpponentsList.join("; ")));

    setItem(row, colIdx++, new QTableWidgetItem(QString::number(descriptor.tourClearRank)));
    setItem(row, colIdx++, new QTableWidgetItem(QString::number(descriptor.unlockId)));
    setItem(row, colIdx++, new QTableWidgetItem(QString::number(descriptor.nameMsgId)));
    setItem(row, colIdx++, new QTableWidgetItem(QString::number(descriptor.descMsgId)));

    setItem(row, colIdx++, new QTableWidgetItem(descriptor.descs["en"]));
    setItem(row, colIdx++, new QTableWidgetItem(descriptor.internalName));
}

void MapDescriptorWidget::appendMapDescriptor(const MapDescriptor &descriptor) {
    descriptors.append(QSharedPointer<MapDescriptor>::create(descriptor));
    insertRow(descriptors.size()-1);
    loadRowWithMapDescriptor(descriptors.size()-1, descriptor);
}

const QVector<QSharedPointer<MapDescriptor> > &MapDescriptorWidget::getDescriptors() {
    return descriptors;
}
