#include "dialogaisinfo.h"
#include "ui_dialogaisinfo.h"

DialogAisInfo::DialogAisInfo(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogAisInfo)
{
    ui->setupUi(this);
    timerId = this->startTimer(1000);
    mAisData = 0;
    mRadarData = 0;
    ui->textBrowser_2->setTextBackgroundColor(QColor(0,0,0,100));
    ui->textBrowser_2->setStyleSheet("background-color: rgba(0,0,0,0);color:rgb(255, 255, 255);font: bold 13pt \"MS Shell Dlg 2\";border : noborder");
    ui->textBrowser->setTextBackgroundColor(QColor(0,0,0,100));
    ui->textBrowser->setStyleSheet("background-color: rgba(0,0,0,0);color:rgb(255, 255, 255);font: bold 13pt \"MS Shell Dlg 2\";border : noborder");
}

DialogAisInfo::~DialogAisInfo()
{
    this->killTimer(timerId);
    delete ui;

}

void DialogAisInfo::setDataSource(AIS_object_t *aisData ,C_primary_track* radarData)
{
    mAisData = aisData;
    mRadarData = radarData;
    //dialogTargetInfo->setAttribute( Qt::WA_DeleteOnClose, true );
    this->setWindowFlags(Qt::Widget|Qt::FramelessWindowHint);
    //this->setWindowFlags(Qt::Widget);
    //this->setWindowFlags(this->windowFlags()&(~Qt::WindowContextHelpButtonHint));
    this->setFixedSize(width(),height());
    this->setGeometry(10,350,0,0);
    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_PaintOnScreen);
    setAttribute(Qt::WA_TranslucentBackground);

    this->show();
    UpdateData();
}

void DialogAisInfo::timerEvent(QTimerEvent *event)
{
    if(isVisible())
    UpdateData();
}
void DialogAisInfo::UpdateData()
{
    if(mAisData)
    {
        ui->textBrowser->setText(QString::fromUtf8("Dữ liệu AIS:\n")+mAisData->printData());

    }
    else
    {
        ui->textBrowser->clear();
    }
    if(mRadarData)
    {
        ui->textBrowser_2->show();
        ui->textBrowser_2->setText(QString::fromUtf8("Dữ liệu ra đa:\n")+mRadarData->printData());

        if(mRadarData->mAisConfirmedObj)
        {
            ui->textBrowser->setText(QString::fromUtf8("Dữ liệu AIS:\n")+mRadarData->mAisConfirmedObj->printData());
        }
        else if(mRadarData->mAisPossibleObj)
        {
            ui->textBrowser->setText(QString::fromUtf8("Dữ liệu AIS:\n")+mRadarData->mAisConfirmedObj->printData());
        }

    }
    else
    {
        ui->textBrowser_2->clear();
    }
}
