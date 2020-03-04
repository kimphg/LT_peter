#ifndef DIALOGINPUTVALUE_H
#define DIALOGINPUTVALUE_H

#include <QDialog>

namespace Ui {
class DialogInputValue;
}

class DialogInputValue : public QDialog
{
    Q_OBJECT

public:
    explicit DialogInputValue(QWidget *parent = 0);
    ~DialogInputValue();
    DialogInputValue(QWidget *parent,int* returnValue);
    int* retValue;
private slots:
    void on_buttonBox_accepted();

    void on_buttonBox_rejected();

private:
    Ui::DialogInputValue *ui;
};

#endif // DIALOGINPUTVALUE_H
