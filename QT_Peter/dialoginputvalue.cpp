#include "dialoginputvalue.h"
#include "ui_dialoginputvalue.h"

DialogInputValue::DialogInputValue(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogInputValue)
{
    ui->setupUi(this);
}

DialogInputValue::~DialogInputValue()
{
    delete ui;
}

DialogInputValue::DialogInputValue(QWidget *parent, int *returnValue):
    QDialog(parent),
    ui(new Ui::DialogInputValue)
{
    ui->setupUi(this);
    this->setModal(true);
    retValue = returnValue;

}

void DialogInputValue::on_buttonBox_accepted()
{
    *retValue = ui->spinBox->value();
}

void DialogInputValue::on_buttonBox_rejected()
{
    *retValue = 0;
}
