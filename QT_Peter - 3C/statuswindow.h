#ifndef STATUSWINDOW_H
#define STATUSWINDOW_H

#include "c_radar_thread.h"
#include <queue>
#include <QMainWindow>

namespace Ui {
class StatusWindow;
}

class StatusWindow : public QMainWindow
{
    Q_OBJECT

public:

    explicit StatusWindow(dataProcessingThread *radar, QWidget *parent = 0);
    ~StatusWindow();

protected slots:
    void timerEvent(QTimerEvent *event);
private slots:
    void closeEvent(QCloseEvent *event);
private:
    void readGlobalStatus();
    deque<int> recvAverage;
    bool warningBlink;
    bool ansTrue;
    int timerId;
    unsigned char command[8];
    unsigned char moduleId;
    unsigned char paramId;
    dataProcessingThread *mRadar;
    Ui::StatusWindow *ui;
    void sendReq();
    bool receiveRes();
    void readConectionStat();
};

#endif // STATUSWINDOW_H
