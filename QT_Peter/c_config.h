
#ifndef CONFIG_H
#define CONFIG_H

#define SCALE_MAX 80
#define SCALE_MIN 5
#include "common.h"
#ifdef ARTEMIS
#define MAP_PATH_1       "D:/ARTEMIS/MapData/GM1/"
#define MAP_PATH_2       "D:/ARTEMIS/MapData/GM2/"
#define MAP_PATH_3       "D:/ARTEMIS/MapData/GS/"
#define HR_APP_PATH       "D:/ARTEMIS/"
#define HR_DATA_REC_PATH  "D:/ARTEMIS/RecordData/"
#define HR_CONFIG_FILE    "radar_config.xml"
#define HR_CONFIG_FILE_BACKUP_1 "radar_config_backup_1.xml"
#define HR_CONFIG_FILE_BACKUP_2 "radar_config_backup_2.xml"
#define HR_CONFIG_FILE_BACKUP_C "radar_config_backup_c.xml"
#define HR_CONFIG_FILE_DF "D:/ARTEMIS/radar_config_default.xml"
#define HR_ERROR_FILE "D:\\ARTEMIS\\errorLog.txt"
#else
#define HR_APP_PATH       ""
#define MAP_PATH_1       "MapData/GM1/"
#define MAP_PATH_2       "MapData/GM2/"
#define MAP_PATH_3       "MapData/GS/"
#define HR_DATA_REC_PATH  "RecordData/"
#define HR_CONFIG_FILE    "radar_config.xml"
#define HR_CONFIG_FILE_BACKUP_1 "radar_config_backup_1.xml"
#define HR_CONFIG_FILE_BACKUP_2 "radar_config_backup_2.xml"
#define HR_CONFIG_FILE_BACKUP_C "radar_config_backup_c.xml"
#define HR_CONFIG_FILE_DF "radar_config_default.xml"
#define HR_ERROR_FILE "errorLog.txt"
#endif
#define XML_ELEM_NAME     "radar_config"
#include <math.h>
#define DEFAULT_LAT		20.707
#define DEFAULT_LONG	106.78
#include <QFile>
#include <QHash>
#include <QXmlStreamReader>
#include <time.h>
#include <QDateTime>
#include <queue>
#include <common.h>
inline void ConvPolarToXY(double *x, double *y, double azi, double range)
{

    *x = ((sin(azi)))*range;
    *y = ((cos(azi)))*range;
}
inline void ConvkmxyToPolarDeg(double x, double y, double *azi, double *range)
{
    if(!y)
    {
        *azi = x>0? PI_CHIA2:(PI_NHAN2-PI_CHIA2);
        *azi = *azi*DEG_RAD;
        *range = abs(x);
    }
    else
    {
        *azi = atanf(x/y);
        if(y<0)*azi+=PI;
        if(*azi<0)*azi += PI_NHAN2;
        *range = sqrt(x*x+y*y);
        *azi = *azi*DEG_RAD;
    }

}
inline QString demicalDegToDegMin(double demicalDeg)
{
    return QString::number( (short)demicalDeg) +
            QString::fromLocal8Bit("\260")+
            QString::number((demicalDeg-(short)demicalDeg)*60.0,'f',2);
}
struct WarningMessage
{
    QString message;
    clock_t time;
};
class radarStatus_3C
{
public:
    radarStatus_3C();
    ~radarStatus_3C();
    bool isTxSwModeOk;
    void ReadStatus22(uchar* mes)
    {
        mTaiAngTen =    mes[0];
        mSuyGiam =      mes[1];
        mMayPhatOK =    mes[2];
        mCaoApReady =   mes[3];
        mCaoApKetNoi =  mes[4];
        c22UpdateTime = clock();
    }
    void ReadStatusGlobal(uchar* mes)
    {
        cBHUpdateTime = clock();
        memcpy(&(msgGlobal[0]),(char*)(mes),32);
        if(msgGlobal[0]==1
                &&msgGlobal[1]==1
                &&msgGlobal[2]==0
                )
        {
            isTxSwModeOk = true;
        }
        else
        {
            isTxSwModeOk = false;
        }
        if(msgGlobal[17]==0
                &&msgGlobal[18]==0)cTempOkTime = clock();

    }
//    void setGPSLocation(double lat, double lon);
    void inputGyroDeg(double heading,double headingRateDps)
    {
        // auto learning algorithm
        shipHeadingRate_dps = headingRateDps;
        shipHeadingDeg = heading;
        cGyroUpdateTime = clock();
    }
    void inputGyroDeg21(double heading,double headingRateDps)
    {
        // auto learning algorithm
        shipHeadingRate_dps = headingRateDps;
        shipHeadingDeg = heading;
        cGyroUpdateTime21 = clock();
    }
    void setcGyroUpdateTime(){cGyroUpdateTime = clock();}
    void inputHDT(double heading)
    {
        if(getAgeGyro()<3000){isGyro = true; return;}
        isGyro = false;
        double headingDiff = heading-shipHeadingDeg;
        shipHeadingDeg = heading;
        clock_t timeNow = clock();
        clock_t age = timeNow-cHDTUpdateTime;
        if(age<2000)
        {

            shipHeadingRate_dps = headingDiff/(age/1000.0);
        }
        else
            shipHeadingRate_dps = 0;
        cHDTUpdateTime = timeNow;
    }
    //2-2 status
    int     mCheDoDK;
    int     mCaoApReady;
    int     mCaoApKetNoi;
    bool    mTaiAngTen;
    int     mSuyGiam;
    int     mMaHieu;
    bool    mMayPhatOK;
    bool isTransmitting;
private:
    bool    isGyro;
    double shipHeadingRate_dps;

public:

    unsigned  long long int       mFrameCount;
//    double mLat,mLon;
    double shipSpeedWater,shipSpeedGround;
    double shipCourseDeg;
    double antennaAziDeg;
    double antennaBearingDeg;
    double antennaBearingTxStopDeg;
    double antennaBearingTxStartDeg;
    double shipHeadingDeg;
    //global Status
    char msgGlobal[32];
    // connection age
    //clock_t cGpsAge;
    clock_t cAisUpdateTime;
    clock_t cGpsUpdateTime;
    clock_t c22UpdateTime;
    clock_t c21UpdateTime;
    clock_t cBHUpdateTime;
    clock_t cGyroUpdateTime,cGyroUpdateTime21;
    clock_t cVeloUpdateTime;
    clock_t cHDTUpdateTime;
    clock_t cCourseUpdateTime;
    clock_t cTempOkTime;
//    bool isStatChanged()
//    {
//        if(isStatChange)
//        {
//            isStatChange = false;
//            return true;
//        }
//        else return false;
//    }

    clock_t getAgeAis(){return clock()-cAisUpdateTime;}
    clock_t getAgeGps(){return clock()-cGpsUpdateTime;}
    clock_t getAge22(){return clock()- c22UpdateTime;}
    clock_t getAge21(){return clock()- c21UpdateTime;}
    clock_t getAgeBH(){return clock()- cBHUpdateTime;}
    clock_t getAgeGyro(){return clock()-cGyroUpdateTime;}
    clock_t getAgeGyro21(){return clock()-cGyroUpdateTime21;}
    clock_t getAgeVelo(){return clock()-cVeloUpdateTime;}
    clock_t getAgeHDT(){return clock()-cHDTUpdateTime;}
    clock_t getAgeTempOk(){return clock()-cTempOkTime;}
    double getShipHeadingDeg();
    void setShipSpeed(double value);
    void setShipCourse(double value);
    double getshipHeadingRate_dps();
    void setShipSpeed2(double value);
    void setCGyroUpdateTime(const clock_t &value);
};
class CConfig
{
public:
    CConfig(void);
    ~CConfig(void);
    static inline void ConvWGSToKm(double* x, double *y, double m_Long,double m_Lat)
    {
        double refLat = (mLat + (m_Lat))*0.00872664625997;//pi/360
        *x	= (((m_Long) - mLon) * 111.31949079327357)*cos(refLat);// 3.14159265358979324/180.0*6378.137);//deg*pi/180*rEarth
        *y	= ((m_Lat- mLat ) * 111.132954);
        //tinh toa do xy KM so voi diem center khi biet lat-lon
    }
    static inline void ConvKmToWGS(double x, double y, double *m_Long, double *m_Lat)
    {
        *m_Lat  = CConfig::mLat +  (y)/(111.132954);
        double refLat = (CConfig::mLat +(*m_Lat))*0.00872664625997;//3.14159265358979324/180.0/2;
        *m_Long = (x)/(111.31949079327357*cos(refLat))+ CConfig::mLon;
        //tinh toa do lat-lon khi biet xy km (truong hop coi trai dat hinh cau)
    }
    static void setGPSLocation(double lat,double lon);
    static radarStatus_3C mStat;
    static double  mLat,mLon;
    static std::vector<std::pair<double,double>>* GetLocationHistory();
//    static double shipHeadingDeg;
//    static double shipCourseDeg;
//    static double shipSpeed;
//    static double antennaAziDeg;
    static volatile qint64 time_now_ms;
    static QHash<QString, QString> mHashData;

    static void    setValue(QString key, double value);
    static void    setValue(QString key,QString value);
    static double  getDouble(QString key, double defaultValue=0);
    static QString getString(QString key, QString defaultValue="0");
    static int     getInt(QString key, int defaultValue=0);
    static void    SaveAndSetConfigAsDefault();
    static void    SaveToFile();
    static void     ReportError(const char *error);
    static void     AddMessage(QString warning);
    //static QXmlStreamReader xml;
    static QHash<QString, QString> readFile();
    static std::queue<WarningMessage> *getWarningList();
    static bool    isChanged;
    static void backup();
private:
    static std::queue<WarningMessage> mWarningList;
    static QHash<QString, QString> readFile(QString fileName);
};

#endif
