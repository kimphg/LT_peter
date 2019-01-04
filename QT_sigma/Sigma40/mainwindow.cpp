#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "time.h"
double headingRate=0;
double headingValue=0;
qint64 lastTime;
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    startTimer(30);
    lastTime = QDateTime::currentMSecsSinceEpoch();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::timerEvent(QTimerEvent *event)
{
    qint64 newTime = QDateTime::currentMSecsSinceEpoch();
    printf("\ntime diff:%d",int(newTime-lastTime));
    lastTime = newTime;

}

void MainWindow::on_horizontalSlider_valueChanged(int value)
{

}
