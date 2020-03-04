#ifndef DIALOGMODESELECT_H
#define DIALOGMODESELECT_H

#include <QDialog>

namespace Ui {
class DialogModeSelect;
}

class DialogModeSelect : public QDialog
{
    Q_OBJECT

public:
    explicit DialogModeSelect(QWidget *parent = 0);
    ~DialogModeSelect();

private slots:
    void on_buttonBox_accepted();

    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

private:
    Ui::DialogModeSelect *ui;
};

#endif // DIALOGMODESELECT_H
