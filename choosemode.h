#ifndef CHOOSEMODE_H
#define CHOOSEMODE_H

#include <QSettings>
#include <QDialog>

namespace Ui {
class ChooseMode;
}

class ChooseMode : public QDialog
{
    Q_OBJECT

public:
    explicit ChooseMode(QWidget *parent = nullptr);
    ~ChooseMode();

private:
    Ui::ChooseMode *ui;
    QSettings settings;
};

#endif // CHOOSEMODE_H
