#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "time.h"

#include <QtNetwork/QUdpSocket>
double headingRatedps=0;
double headingValue=0;
qint64 lastTime;
char gyroFrame[35];
QUdpSocket udpSocket;
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    startTimer(10);
    lastTime = QDateTime::currentMSecsSinceEpoch();
    gyroFrame[0]=0x5a;
    gyroFrame[1]=0xa5;
    gyroFrame[2]=0x1a;
    gyroFrame[3]=0x01;
    gyroFrame[4]=0x00;
    gyroFrame[5]=0x00;
    gyroFrame[6]=int(0)>>8;
    gyroFrame[7]=int(0);
    gyroFrame[8]=0;
    gyroFrame[9]=0;
    gyroFrame[10]=0;
    gyroFrame[11]=0;
    gyroFrame[12]=int(radians(0))>>8;
    gyroFrame[13]=int(radians(0));
    gyroFrame[14]=0;
    gyroFrame[15]=0;
    gyroFrame[16]=0;
    gyroFrame[17]=0;
    gyroFrame[18]=0;
    gyroFrame[19]=200;
    gyroFrame[20]=0;
    gyroFrame[21]=100;
    gyroFrame[22]=0;
    gyroFrame[23]=0;
    gyroFrame[24]=0;
    gyroFrame[25]=0;
    gyroFrame[26]=0;
    gyroFrame[27]=0;
    gyroFrame[28]=0;
    gyroFrame[29]=0;
    gyroFrame[30]=0xab;
    gyroFrame[31]=0xaa;
    ui->horizontalSlider->setValue(ui->horizontalSlider->maximum()/2);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::timerEvent(QTimerEvent *event)
{
   /* qint64 newTime = QDateTime::currentMSecsSinceEpoch();
    printf("\ntime diff:%d",int(newTime-lastTime));
    lastTime = newTime;*/
    headingValue+=headingRatedps*0.01;
    int heading = headingValue /360.0*65536;
    short headingRate = radians(headingRatedps)*32768.0;
    gyroFrame[6]=int(heading)>>8;
    gyroFrame[7]=int(heading);
    gyroFrame[12]=short(headingRate)>>8;
    gyroFrame[13]=short(headingRate);
    ui->label->setText(QString::number(headingValue));

    udpSocket.writeDatagram((char*)&gyroFrame[0],
            32,
            QHostAddress("127.0.0.1"),31000
            );
}

void MainWindow::on_horizontalSlider_valueChanged(int value)
{
       headingRatedps  = (value-50)/2.0;
}
