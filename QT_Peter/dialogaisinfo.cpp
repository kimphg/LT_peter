#include "dialogaisinfo.h"
#include "ui_dialogaisinfo.h"

DialogAisInfo::DialogAisInfo(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogAisInfo)
{
    ui->setupUi(this);
    timerId = this->startTimer(1000);
}

DialogAisInfo::~DialogAisInfo()
{
    this->killTimer(timerId);
    delete ui;

}

void DialogAisInfo::setAisData(std::map<int,AIS_object_t> *data, int mmsi)
{
    aisData = data;
    aisMmsi = mmsi;
    UpdateData();
}

void DialogAisInfo::timerEvent(QTimerEvent *event)
{
    UpdateData();
}
void DialogAisInfo::UpdateData()
{
    if(aisData->find(aisMmsi)!=aisData->end())
    {
        AIS_object_t *obj = &(aisData->at(aisMmsi));

            ui->textBrowser->setText(obj->printData());
            obj->isSelected = true;


    }


}
