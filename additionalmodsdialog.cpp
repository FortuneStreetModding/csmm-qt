#include "additionalmodsdialog.h"
#include "ui_additionalmodsdialog.h"
#include <QDialogButtonBox>
#include <QFileDialog>


QVector<QString> modpackZips;

AdditionalModsDialog::AdditionalModsDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AdditionalModsDialog)
{
    ui->setupUi(this);
    this->resize(100,100);

    connect(ui->addModZip, &QPushButton::clicked, this, [this](bool) {
        auto files = QFileDialog::getOpenFileNames(this, "Import additional modpack(s)", QString(), "Modpack zip file (*.zip)", nullptr);
        if (!files.isEmpty()) {
            ui->additionalMods->addItems(files);
        }
    });
    connect(ui->removeModZip, &QPushButton::clicked, this, [this](bool) {
        for (auto item: ui->additionalMods->selectedItems()) {
            delete item;
        }
    });

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    setWindowTitle(QString("Add Modpacks"));
}

QVector<QString> AdditionalModsDialog::GetModpackZips(){
    QVector<QString> modpackZips;
    for (int i=0; i<ui->additionalMods->count(); ++i) {
        modpackZips.append(ui->additionalMods->item(i)->text());
    }

    return modpackZips;
}

AdditionalModsDialog::~AdditionalModsDialog()
{
    delete ui;
}
