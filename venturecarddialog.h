#ifndef VENTURECARDDIALOG_H
#define VENTURECARDDIALOG_H

#include <QDialog>
#include "lib/mapdescriptor.h"

namespace Ui {
class VentureCardDialog;
}

class VentureCardDialog : public QDialog
{
    Q_OBJECT

public:
    explicit VentureCardDialog(const MapDescriptor &descriptorVal, QWidget *parent = nullptr);
    ~VentureCardDialog();

private:
    const MapDescriptor descriptor;
    Ui::VentureCardDialog *ui;
};

#endif // VENTURECARDDIALOG_H
