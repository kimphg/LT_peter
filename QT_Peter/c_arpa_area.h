#ifndef C_ARPA_AREA_H
#define C_ARPA_AREA_H

#include "c_radar_thread.h"
#include "c_radar_data.h"
#include "dialogaisinfo.h"

#include <QPainter>


class C_arpa_area
{
public:
    C_arpa_area();

    QRect           rect;
    DialogAisInfo *dialogTargetInfo;
    dataProcessingThread *processing;
    C_radar_data *mRadarData;
    double mLat,mLon,mScale;
    double mZoomSizeKm;
    int mMouseLastX,mMouseLastY;
    int mMousex,mMousey;
    bool showAisName;
    bool isDrawTargetNumber;
    int target_size;
    double trueShiftDeg,headShift;
    int radCtX,radCtY;
    double mZoomSizeRg,    mZoomSizeAz;
    PointInt ConvWGSToScrPoint(double m_Long, double m_Lat);
    C_primary_track *SelectRadarTarget(int xclick, int yclick);
    void checkClickAIS(int xclick, int yclick);
    void drawAisTarget(QPainter *p);
    void DrawAISMark(PointInt s, double head, QPainter *p, QString name, int size,int vectorLen);
    bool isInsideViewRect(int x, int y);
    void setCenterLonLat(double lon,double lat);
    void DrawRadarTargets(QPainter *p);
//    bool isInsideViewZone(int x, int y, int scrCtX, int scrCtY, int range);
    inline static void rotateVector(double angle,int* x,int* y)
    {
        if(abs(angle)<0.1)return;
        double theta = radians(angle);
        double cs = cos(theta);
        double sn = sin(theta);

        double px = (*x) * cs - (*y) * sn;
        double py = (*x) * sn + (*y) * cs;
        (*x) = px;
        (*y) = py;
    }
    inline static void rotateVector(double angle,double* x,double* y)
    {
        if(abs(angle)<0.1)return;
        double theta = radians(angle);
        double cs = cos(theta);
        double sn = sin(theta);
        double px = (*x) * cs - (*y) * sn;
        double py = (*x) * sn + (*y) * cs;
        (*x) = px;
        (*y) = py;
    }

    void setTarget_size(int value);
    void setScale(double scale);
    C_primary_track *MouseOverRadarTarget(int xclick, int yclick);
    PointDouble ConvScrPointToWGS(int x, int y);
    PointDouble ConvScrPointToKMXY(int x, int y);
    PointAziRgkm ConvScrPointToAziRgkm(int x, int y);
};

#endif // C_ARPA_AREA_H
