#ifndef DIALOGAISINFO_H
#define DIALOGAISINFO_H

#include <QDialog>
#include "AIS/AIS.h"
#include "c_radar_data.h"
namespace Ui {
class DialogAisInfo;
}

class DialogAisInfo : public QDialog
{
    Q_OBJECT

public:

    explicit DialogAisInfo(QWidget *parent = 0);
    ~DialogAisInfo();
    int timerId;
    void setDataSource(AIS_object_t *aisData, C_SEA_TRACK *radarData);
    AIS_object_t * mAisData;
    C_SEA_TRACK *mRadarData;
    int radarID;
    int aisMmsi;

private:
    Ui::DialogAisInfo *ui;
    void UpdateData();
private slots:
    void timerEvent(QTimerEvent *event);


};

#endif // DIALOGAISINFO_H
