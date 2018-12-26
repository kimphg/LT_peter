#include "statuswindow.h"
#include "ui_statuswindow.h"
//double lookupTable5V[256];

StatusWindow::StatusWindow(dataProcessingThread *radar,QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::StatusWindow)
{
    ansTrue = false;
    ui->setupUi(this);
    mRadar = radar;
    timerId = startTimer(600);
    moduleId = 0;
    paramId = 0xaa;
    command[0]=0xaa;
    command[1]=0xab;
    command[2]=moduleId;
    command[3]=0xaa;
    command[4]=0x00;
    command[5]=0x00;
    command[6]=0x00;
    command[7]=0x00;
    this->setWindowFlags(Qt::WindowStaysOnTopHint);
    ui->tableView->
}
void StatusWindow::closeEvent(QCloseEvent *event)
{
    killTimer(timerId);
}

void StatusWindow::readGlobalStatus()
{
    radarStatus_3C* mRadarStat = mRadar->mRadarStat;

    mRadarStat->msgGlobal[0];
}
/*
1. DDS: aaab03cc

2. VCO1: aaab03bb

3. VCO2: aaab02bb

4. vào TK: aaab03dd

5. ra TK: aaab01cc

các tham số trên nằm trong 3 byte 1,2,3  trong khung truyền:
-byte 1: loại mô-đun
-byte 2: loại tham số
-byte 3: giá trị tham số

6. thu: giá trị nằm 2 byte: 7 và 8 trong khung truyền:
*/
void StatusWindow::sendReq()
{
    moduleId++;
    switch (moduleId) {
    case 1:
        command[2]=0x03;
        command[3]=0xcc;
        break;
    case 2:
        command[2]=0x03;
        command[3]=0xbb;
        break;
    case 3:
        command[2]=0x02;
        command[3]=0xbb;
        break;
    case 4:
        command[2]=0x03;
        command[3]=0xdd;
        break;
    case 5:
        command[2]=0x01;
        command[3]=0xcc;
        break;
    case 6:
        command[2]=0x00;
        command[3]=0xaa;
        break;
    default:
        moduleId=0;
        return;
    }
    mRadar->sendCommand(&command[0],8);
    mRadar->sendCommand(&command[0],8);
    mRadar->sendCommand(&command[0],8);
}
/*
1. DDS: aaab03cc

2. VCO1: aaab03bb

3. VCO2: aaab02bb

4. vào TK: aaab03dd

5. ra TK: aaab01cc
*/
bool StatusWindow::receiveRes()
{
    unsigned char* signalFrameHeader = mRadar->mRadarData->mHeader;
    int moduleIndex = signalFrameHeader[1];
    int paramIndex  = signalFrameHeader[2];
    int paramValue  = signalFrameHeader[3];
    int recvValue   = (signalFrameHeader[7]<<8)+signalFrameHeader[8];
    recvAverage.push_back(recvValue);
    if(recvAverage.size()>10)recvAverage.pop_front();
    double recvAver=0;
    int count = 0;
    for (int i=0;i<recvAverage.size();i++) {
        recvAver+=recvAverage[i];
        count++;
    }
    recvAver/=count;
    ui->label_byte_1->setText(QString::number(moduleIndex));
    ui->label_byte_2->setText(QString::number(paramIndex));
    ui->label_byte_3->setText(QString::number(paramValue));
    ui->label_byte_4->setText(QString::number(recvValue));
    if((moduleIndex==3)&&paramIndex==0xcc)ui->label_res_dds_out->setText(QString::number(paramValue));
    if((moduleIndex==3)&&paramIndex==0xbb)ui->label_vco_output_1->setText(QString::number(paramValue));
    if((moduleIndex==2)&&paramIndex==0xbb)ui->label_vco_output_2->setText(QString::number(paramValue));
    if((moduleIndex==3)&&paramIndex==0xdd)ui->label_trans_input->setText(QString::number(paramValue));
    if((moduleIndex==1)&&paramIndex==0xcc)ui->label_trans_output->setText(QString::number(paramValue));
    if((moduleIndex==0)&&paramIndex==0xaa)ui->label_res_main_temp->setText(QString::number(paramValue/4.0));
    recvAver=60+20*log10(recvAver/55.0);
    ui->label_res_receiver->setText(QString::number(recvAver,'f',1));
    return true;
    /*QString resVal;
    double hsTap = mRadar->mRadarData->get_tb_tap();
    hsTap = 20*log10(hsTap/165.0)+77.0;
    ui->label_res_receiver->setText(QString::number(hsTap,'f',1));
    int resModId = mRadar->mRadarData->tempType;
    unsigned char * pFeedBack = mRadar->mRadarData->getFeedback();
    if(   (pFeedBack[0]==command[0])
          &&(pFeedBack[1]==command[1])
          &&(pFeedBack[2]==command[2])
          &&(pFeedBack[3]==command[3])
          &&(pFeedBack[4]==command[4])
          &&(pFeedBack[5]==command[5])
          &&(pFeedBack[6]==command[6])
          &&(resModId==moduleId)
       )
    {
        ansTrue = false;
        double x =mRadar->mRadarData->moduleVal;
        switch (resModId) {
        case 0:
            if(paramId==0xaa)
            {
                resVal = QString::number(x/4.0,'f',1);
                ui->label_res_main_temp->setText(resVal);
            }
            else if(paramId==0xab)
            {
                resVal = QString::number(x-72.0,'f',1);
                if(((x-72.0)>10||(x-72.0)<4)&warningBlink)ui->label_res_dds_out->setStyleSheet("border: 3px solid red;");
                else ui->label_res_dds_out->setStyleSheet("");
                ui->label_res_dds_out->setText(resVal);
            }
            else if(paramId==0xac)
            {
                resVal = QString::number(x-78.0,'f',1);
                if(((x-78.0)>-5||(x-78.0)<-15)&warningBlink)ui->label_trans_input->setStyleSheet("border: 3px solid red;");
                else ui->label_trans_input->setStyleSheet("");
                ui->label_trans_input->setText(resVal);
            }
            break;
        case 2:
            if(paramId==0xaa)
            {
                resVal = QString::number(x,'f',1);
                ui->label_res_trans_temp->setText(resVal);
            }
            else if(paramId==0xab)
            {
                double val = (x/62.0)*(x/62.0)*100.0;
                resVal = QString::number(val,'f',1);
                if((val<80||val>100)&warningBlink)ui->label_trans_output->setStyleSheet("border: 3px solid red;");
                else ui->label_trans_output->setStyleSheet("");
                ui->label_trans_output->setText(resVal);
            }
            break;

        case 3:
            if(paramId==0xaa)
            {

            }
            else if(paramId==0xab)
            {
                resVal = QString::number((x-126.0)/1.515,'f',1);
                ui->label_vco2_output->setText(resVal);
            }
            break;
        default:
            break;
        }
        return true;
    }
    printf("wrong respond:%d;%d\n",paramId,moduleId);
    return false;*/
}

void StatusWindow::timerEvent(QTimerEvent *event)
{
    /*if(!mRadar->isConnected())
    {
        return;
    }*/
    warningBlink=!warningBlink;
    ansTrue = receiveRes();
    sendReq();
    readGlobalStatus();
}
StatusWindow::~StatusWindow()
{
    delete ui;
}
