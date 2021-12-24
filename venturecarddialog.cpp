#include "venturecarddialog.h"
#include "ui_venturecarddialog.h"

#include "lib/vanilladatabase.h"

VentureCardDialog::VentureCardDialog(const MapDescriptor &descriptorVal, QWidget *parent) :
    QDialog(parent),
    descriptor(descriptorVal),
    ui(new Ui::VentureCardDialog)
{
    ui->setupUi(this);
    for (quint32 i=0; i<descriptor.ventureCards.size(); ++i) {
        if (descriptor.ventureCards[i]) {
            ui->listWidget->addItem(QString("%1)  %2").arg(i+1).arg(VanillaDatabase::getVentureCardDesc(i)));
        }
    }
}

VentureCardDialog::~VentureCardDialog()
{
    delete ui;
}
