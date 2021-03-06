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
    timerId = startTimer(500);
    moduleId = 0;
    paramId = 0xaa;
    command[0] =0xaa;
    command[1] =0xab;
    command[2] =moduleId;
    command[3] =0xaa;
    command[4] =0x00;
    command[5] =0x00;
    command[6] =0x00;
    command[7] =0x00;
    this->setWindowFlags(Qt::WindowStaysOnTopHint);
    //

#ifdef THEON
    ui->tableWidget->hide();
    ui->label_module_cong_suat->setText(QString::fromUtf8("Mô đun công suất"));
    ui->groupBox_2->hide();
#else
    ui->tableWidget->setColumnCount(2);
    ui->tableWidget->setColumnWidth(0,250);
    ui->tableWidget->setColumnWidth(1,20);
    ui->tableWidget->setRowCount(23);
    //
    QTableWidgetItem* item = new QTableWidgetItem(QString::fromUtf8("Chuyển mạch ăng ten chủ động/thụ động"));   ui->tableWidget->setItem(0,0,item);
    item = new QTableWidgetItem(QString::fromUtf8("Chuyển mạch ăng ten - đang ở vị trí A2"));                    ui->tableWidget->setItem(1,0,item);
    item = new QTableWidgetItem(QString::fromUtf8("Chuyển mạch ăng ten - đang ở vị trí A3"));                    ui->tableWidget->setItem(2,0,item);
    item = new QTableWidgetItem(QString::fromUtf8("Chuyển mạch ăng ten - đang ở vị trí A4"));                    ui->tableWidget->setItem(3,0,item);
    item = new QTableWidgetItem(QString::fromUtf8("Trạng thái hệ thống quạt gió VEN1"));                                ui->tableWidget->setItem(4,0,item);
    item = new QTableWidgetItem(QString::fromUtf8("Trạng thái hệ thống quạt gió VEN2"));                                ui->tableWidget->setItem(5,0,item);
    item = new QTableWidgetItem(QString::fromUtf8("Hệ thống làm mát - nhiệt độ"));                                      ui->tableWidget->setItem(6,0,item);
    item = new QTableWidgetItem(QString::fromUtf8("Hệ thống làm mát - lưu lượng"));                                     ui->tableWidget->setItem(7,0,item);
    item = new QTableWidgetItem(QString::fromUtf8("Mức dầu"));                                                          ui->tableWidget->setItem(8,0,item);
    item = new QTableWidgetItem(QString::fromUtf8("Báo nối máy 18"));                                                   ui->tableWidget->setItem(9,0,item);
    item = new QTableWidgetItem(QString::fromUtf8("Báo tốt máy 18"));                                                   ui->tableWidget->setItem(10,0,item);
    item = new QTableWidgetItem(QString::fromUtf8("Báo nguồn Máy 2-2"));                                                ui->tableWidget->setItem(11,0,item);
    item = new QTableWidgetItem(QString::fromUtf8("Lỗi nguồn Máy 2-2"));                                                ui->tableWidget->setItem(12,0,item);
    item = new QTableWidgetItem(QString::fromUtf8("Báo nguồn Máy 4"));                                                  ui->tableWidget->setItem(13,0,item);
    item = new QTableWidgetItem(QString::fromUtf8("Lỗi nguồn máy 4"));                                                  ui->tableWidget->setItem(14,0,item);
    item = new QTableWidgetItem(QString::fromUtf8("Báo nguồn máy 3P"));                                                 ui->tableWidget->setItem(15,0,item);
    item = new QTableWidgetItem(QString::fromUtf8("Báo nguồn máy 2-1"));                                                ui->tableWidget->setItem(16,0,item);
    item = new QTableWidgetItem(QString::fromUtf8("Nhiệt không khí 1"));                                                ui->tableWidget->setItem(17,0,item);
    item = new QTableWidgetItem(QString::fromUtf8("Nhiệt không khí 2"));                                                ui->tableWidget->setItem(18,0,item);
    item = new QTableWidgetItem(QString::fromUtf8("Báo nguồn  ПМ"));                                                    ui->tableWidget->setItem(19,0,item);
    item = new QTableWidgetItem(QString::fromUtf8("Báo tốc độ quay 1V/P"));                                             ui->tableWidget->setItem(20,0,item);
    item = new QTableWidgetItem(QString::fromUtf8("Báo tốc độ quay 4V/P"));                                             ui->tableWidget->setItem(21,0,item);

    //
    ui->tableWidget_connection->setColumnCount(2);
    ui->tableWidget_connection->setColumnWidth(0,200);
    ui->tableWidget_connection->setColumnWidth(1,20);
    ui->tableWidget_connection->setRowCount(8);
    item = new QTableWidgetItem(QString::fromUtf8("Switch Ethernet 1"));ui->tableWidget_connection->setItem(0,0,item);
    item = new QTableWidgetItem(QString::fromUtf8("Switch Ethernet 2"));ui->tableWidget_connection->setItem(1,0,item);
    item = new QTableWidgetItem(QString::fromUtf8("Switch Ethernet 3"));ui->tableWidget_connection->setItem(2,0,item);
    item = new QTableWidgetItem(QString::fromUtf8("Định vị GPS"));      ui->tableWidget_connection->setItem(3,0,item);
    item = new QTableWidgetItem(QString::fromUtf8("AIS"));              ui->tableWidget_connection->setItem(4,0,item);
    item = new QTableWidgetItem(QString::fromUtf8("Con quay t.tiếp"));  ui->tableWidget_connection->setItem(5,0,item);
    item = new QTableWidgetItem(QString::fromUtf8("Con quay máy 2.1")); ui->tableWidget_connection->setItem(6,0,item);
    item = new QTableWidgetItem(QString::fromUtf8("Tính đường"));       ui->tableWidget_connection->setItem(7,0,item);
    item = new QTableWidgetItem(QString::fromUtf8("Máy 2-1"));          ui->tableWidget_connection->setItem(8,0,item);
    item = new QTableWidgetItem(QString::fromUtf8("Máy 2-2"));          ui->tableWidget_connection->setItem(9,0,item);
    item = new QTableWidgetItem(QString::fromUtf8("Mô đun báo hỏng"));  ui->tableWidget_connection->setItem(10,0,item);
#endif
}
void StatusWindow::closeEvent(QCloseEvent *event)
{
    killTimer(timerId);
}
void StatusWindow::readConectionStat()
{
    QTableWidgetItem* item;
    item = new QTableWidgetItem(QString::number((int)(CConfig::mStat.getAgeGps()<3000)));   ui->tableWidget_connection->setItem(3,1,item);
    item = new QTableWidgetItem(QString::number((int)(CConfig::mStat.getAgeAis()<3000)));   ui->tableWidget_connection->setItem(4,1,item);
    item = new QTableWidgetItem(QString::number((int)(CConfig::mStat.getAgeGyro()<3000)));  ui->tableWidget_connection->setItem(5,1,item);
    item = new QTableWidgetItem(QString::number((int)(CConfig::mStat.getAgeGyro21()<3000)));ui->tableWidget_connection->setItem(6,1,item);
    item = new QTableWidgetItem(QString::number((int)(CConfig::mStat.getAgeVelo()<10000))); ui->tableWidget_connection->setItem(7,1,item);
    item = new QTableWidgetItem(QString::number((int)(CConfig::mStat.getAge21()<3000)));    ui->tableWidget_connection->setItem(8,1,item);
    item = new QTableWidgetItem(QString::number((int)(CConfig::mStat.getAge22()<10000)));   ui->tableWidget_connection->setItem(9,1,item);
    item = new QTableWidgetItem(QString::number((int)(CConfig::mStat.getAgeBH()<3000)));    ui->tableWidget_connection->setItem(10,1,item);
}
void StatusWindow::readGlobalStatus()
{
    radarStatus_3C* mRadarStat = &(CConfig::mStat);
    unsigned long gAge = (clock()-mRadarStat->cBHUpdateTime)/1000;
    if(gAge>3)
    {
        ui->groupBox_2->setTitle(QString::fromUtf8("Trạng thái đài ra đa (mất kết nối)")+QString::number(gAge));
        return;
    }
    else
    {
        ui->groupBox_2->setTitle(QString::fromUtf8("Trạng thái đài ra đa"));
    }
    QTableWidgetItem* item;
    for(int i=0;i<22;i++)
    {
            item = new QTableWidgetItem(QString::number(mRadarStat->msgGlobal[i]));   ui->tableWidget->setItem(i,1,item);
    }
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
#ifdef THEON
    switch (moduleId) {
    case 1:
        command[2]=0x01;
        command[3]=0xaa;
        break;
    case 2:
        command[2]=0x01;
        command[3]=0xbb;
        break;
    case 3:
        command[2]=0x01;
        command[3]=0xcc;
        break;
    case 4:
        command[2]=0x01;
        command[3]=0xdd;
        break;
    case 5:
        command[2]=0x00;
        command[3]=0xbb;
        break;
    case 6:
        command[2]=0x00;
        command[3]=0xdd;
        break;
    default:
        moduleId=0;
        return;
    }
#else
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
#endif
    mRadar->sendCommand(&command[0],8,true);
    mRadar->sendCommand(&command[0],8,true);
//    mRadar->sendCommand(&command[0],8);
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
    //check connection state
    int ageVideo = CConfig::mStat.getAge21()/1000;
    if(ageVideo)
    {
        ui->groupBox->setTitle(QString::fromUtf8("Trạng thái máy 2-1(mất kết nối) ")+QString::number(ageVideo));
        //ui->groupBox->setStyleSheet("color: 3px solid red;");

    }
    else
    {
        ui->groupBox->setTitle(QString::fromUtf8("Trạng thái máy 2-1(đã kết nối)"));
        //ui->groupBox->setStyleSheet("color: 3px solid green;");
    }
        //
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
#ifdef THEON
    /*
1. kiểm tra nhiệt độ hệ thống: aaab01aa
2. kiểm tra công suất VCO1: aaab01bb
3. kiểm tra công suất DDS: aaab01cc
4. kiểm tra công suất VCO2: aaab01dd

5. kiểm tra công suất  ra KĐCS: aaab00bb
6. kiểm tra công suất  vao KĐCS: aaab00dd
*/
    if((moduleIndex==1)&&paramIndex==0xcc)ui->label_res_dds_out->setText(QString::number(paramValue));
    if((moduleIndex==1)&&paramIndex==0xbb)ui->label_vco_output_1->setText(QString::number(paramValue));
    if((moduleIndex==1)&&paramIndex==0xdd)ui->label_vco_output_2->setText(QString::number(paramValue));
    if((moduleIndex==0)&&paramIndex==0xdd)ui->label_trans_input->setText(QString::number(paramValue));
    if((moduleIndex==0)&&paramIndex==0xbb)ui->label_trans_output->setText(QString::number(paramValue));
    if((moduleIndex==1)&&paramIndex==0xaa)ui->label_res_main_temp->setText(QString::number(paramValue/4.0));
#else
    if((moduleIndex==3)&&paramIndex==0xcc)ui->label_res_dds_out->setText(QString::number(paramValue));
    if((moduleIndex==3)&&paramIndex==0xbb)ui->label_vco_output_1->setText(QString::number(paramValue));
    if((moduleIndex==2)&&paramIndex==0xbb)ui->label_vco_output_2->setText(QString::number(paramValue));
    if((moduleIndex==3)&&paramIndex==0xdd)ui->label_trans_input->setText(QString::number(paramValue));
    if((moduleIndex==1)&&paramIndex==0xcc)ui->label_trans_output->setText(QString::number(paramValue));
    if((moduleIndex==0)&&paramIndex==0xaa)ui->label_res_main_temp->setText(QString::number(paramValue/4.0));
    recvAver=60+20*log10(recvAver/55.0);
    ui->label_res_receiver->setText(QString::number(recvAver,'f',1));
#endif
    return true;
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
    readConectionStat();
}
StatusWindow::~StatusWindow()
{
    delete ui;
}
