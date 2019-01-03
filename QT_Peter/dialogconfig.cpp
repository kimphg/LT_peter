#include "dialogconfig.h"
#include "ui_dialogconfig.h"

DialogConfig::DialogConfig(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogConfig)
{
    ui->setupUi(this);
    int nItems = CConfig::mHashData.size();
    ui->tableWidget->setRowCount(nItems);
}

DialogConfig::~DialogConfig()
{
    delete ui;
}

void DialogConfig::on_pushButton_load_clicked()
{
    int nItems = CConfig::mHashData.size();
    ui->tableWidget->setRowCount(nItems);
    int rowcount = 0;
    for(QHash<QString ,QString>::iterator i=CConfig::mHashData.begin();i!=CConfig::mHashData.end();++i)
    {
        QString id = i.key();
        QString value = i.value();

        ui->tableWidget->setItem(rowcount,0,new QTableWidgetItem(id));
        ui->tableWidget->setItem(rowcount,1,new QTableWidgetItem(value));
        rowcount++;
    }
}
