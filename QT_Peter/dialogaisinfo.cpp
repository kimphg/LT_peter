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
    this->setWindowFlags(this->windowFlags()&(~Qt::WindowContextHelpButtonHint));
    this->setFixedSize(width(),height());
    this->setGeometry(10,800,0,0);
//    setAttribute(Qt::WA_TranslucentBackground);
//    this->setStyleSheet();
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
    if(mAisData)ui->textBrowser->setText(mAisData->printData());
    else ui->textBrowser->clear();
    if(mRadarData)
    {
        ui->textBrowser_2->setText(mRadarData->printData());
        if(mRadarData->mAisConfirmedObj)
        {
            ui->textBrowser->setText(mRadarData->mAisConfirmedObj->printData());
        }
    }
    else ui->textBrowser_2->clear();

}
