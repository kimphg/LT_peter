#ifndef DIALOGDETAILDISPLAY_H
#define DIALOGDETAILDISPLAY_H

#include <QDialog>
#include "c_radar_thread.h"
#include "dialogaisinfo.h"
namespace Ui {
class DialogDetailDisplay;
}

class DialogDetailDisplay : public QDialog
{
    Q_OBJECT

public:
    explicit DialogDetailDisplay(QWidget *parent = 0);
    ~DialogDetailDisplay();
    C_radar_data *mRadarData;
    void init(dataProcessingThread *processingThread, DialogAisInfo *tinfoPointer);
    double mLat,mLon,mScale;
    double mZoomSizeKm;
    int mMouseLastX,mMouseLastY;
    int mMousex,mMousey;
    void setCenterLonLat(double lon,double lat);
private:
    bool showAisName;
    int timerId;
    dataProcessingThread *processing;
    int target_size;
    int radCtX,radCtY;
    double trueShiftDeg;
    double mZoomSizeRg,    mZoomSizeAz;
    Ui::DialogDetailDisplay *ui;
    void DrawSignal(QPainter *p);
    void DrawRadarTargetByPainter(QPainter *p);
    PointInt ConvWGSToScrPoint(double m_Long, double m_Lat);
    C_primary_track *checkClickRadarTarget(int xclick, int yclick);
    void checkClickAIS(int xclick, int yclick);
    DialogAisInfo *dialogTargetInfo;
    void drawAisTarget(QPainter *p);
    void DrawAISMark(PointInt s, double head, QPainter *p, QString name, int size,int vectorLen);
    bool isInsideViewZone(int x, int y);
protected:
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event);
private slots:
    void timerEvent(QTimerEvent *event);
    void on_toolButton_view_IAD_clicked(bool checked);
    void on_toolButton_view_histogram_clicked(bool checked);
    void on_toolButton_view_zoom_clicked(bool checked);
};

#endif // DIALOGDETAILDISPLAY_H
