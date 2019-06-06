#include "dialogmenudisplay.h"
#include "ui_dialogmenudisplay.h"

DialogMenuDisplay::DialogMenuDisplay(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogMenuDisplay)
{
    ui->setupUi(this);
}

DialogMenuDisplay::~DialogMenuDisplay()
{
    delete ui;
}
