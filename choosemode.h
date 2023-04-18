#ifndef CHOOSEMODE_H
#define CHOOSEMODE_H

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
};

#endif // CHOOSEMODE_H
