#include "dialogmodeselect.h"
#include "ui_dialogmodeselect.h"
#include "c_config.h"
DialogModeSelect::DialogModeSelect(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogModeSelect)
{
    ui->setupUi(this);
}

DialogModeSelect::~DialogModeSelect()
{
    delete ui;
}

void DialogModeSelect::on_buttonBox_accepted()
{

}

void DialogModeSelect::on_pushButton_clicked()
{
    int value = 1;
    if(ui->radioButton->isChecked())value = 1;
    if(ui->radioButton_2->isChecked())value = 2;
    if(ui->radioButton_3->isChecked())value = 3;
    if(ui->radioButton_4->isChecked())value = 4;
    CConfig::setValue("WorkMode",value);
    this->accept();
    this->close();
}

void DialogModeSelect::on_pushButton_2_clicked()
{
    this->close();
}
