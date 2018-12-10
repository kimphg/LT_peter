#ifndef C_RAW_DATA_H
#define C_RAW_DATA_H
//  |----------------------------------------------------------|
//  |HR2D signal processing class and tracking algorithm       |
//  |First release: November 2015                              |
//  |Last update: Sept 2018                                    |
//  |Author: Phung Kim Phuong                                  |
//  |----------------------------------------------------------|
#define ARMA_USE_LAPACK
#define ARMA_USE_BLAS
#define ARMA_BLAS_UNDERSCORE
#define MODE_MARINE
#define TRACK_STABLE_STATE          5
#define MIN_TERRAIN                 10
#define TRACK_CONFIRMED_SIZE        3
#define TRACK_INIT_STATE            3
#define DEG_RAD 57.295779513
#define sq(x) (x*x)
#define degrees(x) (x*57.295779513)
#define radians(x) (x/57.295779513)
#ifndef CONST_NM
#define CONST_NM 1.852// he so chuyen doi tu km sang hai ly
#endif
#define PI_NHAN2                    6.2831853072
#define PI_CHIA2                    1.5707963268
#ifndef PI
#define PI                       3.14159265358979323846
#endif
#define FRAME_HEADER_SIZE 34
#define RADAR_RESOLUTION 2048
#define MAX_FRAME_SIZE RADAR_RESOLUTION*2+FRAME_HEADER_SIZE
#define RADAR_RESOLUTION_HALF 1024
#define MAX_FRAME_SIZE_HALF RADAR_RESOLUTION_HALF*2+FRAME_HEADER_SIZE
#define CONST_E 2.71828182846
#define MAX_TRACK_LEN               400
#define MAX_TRACKS                  400
#define ENCODER_RES                 5000
#define MAX_AZIR                    2048
#define MAX_AZIR_DRAW               6144
//#define RAD_M_PULSE_RES             1536
#define RAD_S_PULSE_RES             256
#define RAD_DISPLAY_RES             600//768
#define RAD_FULL_RES                1792
#define SIGNAL_SCALE_7              0.24113 //215.38461538461538461538461538461
#define SIGNAL_SCALE_6              0.24113//184.61538461538461538461538461538
#define SIGNAL_SCALE_5              0.24113 //153.84615384615384615384615384615
#define SIGNAL_SCALE_4              0.24113 // 123.07692307692307692307692307692
#define SIGNAL_SCALE_3              0.12056514914 //92.307692307692307692307692307692
#define SIGNAL_SCALE_2              0.06028257456 //61.538461538461538461538461538462
#define SIGNAL_SCALE_1              0.03014128728 //30.769230769230769230769230769231
#define SIGNAL_SCALE_0              0.015070644 //15.384615384615384615384615384615
#define TERRAIN_GAIN                0.9f
#define TERRAIN_GAIN_1              0.1f
#define TERRAIN_THRESH              0.5f
#define TARGET_MAX_SPEED_MARINE     50.0
#define ZOOM_SIZE                   550
#define DISPLAY_RES_ZOOM            8192
#define DISPLAY_SCALE_ZOOM          4
#define nm(x) (x/1.852)
#include "c_config.h"
#include <vector>
#include <QImage>
#include <QDateTime>
#include <QFile>
//#include <Eigen/Dense>
#include <queue>
//#include <c_target_manager.h>
inline double sinFast(double a)
{
    while (a>PI) {
        a-=PI_NHAN2;
    }
    double a2 = a*a;
    return a-a2*a/6.0+a2*a2*a/120.0-a2*a2*a2*a/5040.0;
}
inline double cosFast(double a)
{
    while (a>PI) {
        a-=PI_NHAN2;
    }
    double a2 = a*a;
    return 1.0-a2/2.0+a2*a2/24.0-a2*a2*a2/720.0;
}
inline double ConvXYToRg(double x, double y)
{
    return sqrt(x*x + y*y);

}
inline double ConvXYToAziRd(double x, double y)
{
    if (!y)        return (x>0 ? PI_CHIA2 : (PI_NHAN2 - PI_CHIA2));
    else
    {
        double azi = atan(x / y);
        if (y<0)azi += PI;
        if (azi<0)azi += PI_NHAN2;
        return azi;
    }
}
typedef struct
{
    int trackCount;
    uint timeStart;
    double xkm,ykm;
    double aziDeg,rg;
    double maxDrg,maxDazDeg;
}DetectionWindow;
typedef struct  {
    short lastA,riseA,fallA;
    short maxA1,maxA2;
    unsigned short maxR,minR;
    unsigned int sumR;
    unsigned short size;
    unsigned int sumEnergy;
    unsigned char dopler,maxLevel;
    bool isUsed;
} plot_t;
typedef struct  {
    //    int uniqID;
    double          azRad ,rg,xkm,ykm;
    //    double          xkmfit,ykmfit;
    //    double          azRadfit,rgKmfit;
    double          rgKm;
    short           dazi,drg;
    short           size;
    char            dopler;
    //    bool            isProcessed;
    bool            isRemoved;
    //    float           p;
    //    float          terrain;
    double           rgStdEr;
    double           aziStdEr;
    long long          timeMs;
    //    float           scorepObj,scorep2;
    //    float scoreTrack;
    unsigned long int period;
    //    uint len;
}object_t;
//struct object_line
//{
//    double      pointScore,trackScore;
//    float       dtimeMSec;
//    object_t    obj1;
//    object_t    obj2;
////    float distanceCoeff;
//    float rgSpeedkmh;
////    float speedkmh;
////    float bearingRad;
//    float dx,dy;
//    bool isProcessed;
//    bool isRemoved;
//};

//using Eigen::MatrixXf;
enum class TrackState {newDetection=0,
                       confirmed=3,
                       lost = 2,
                       removed=1};
class C_primary_track
{
public:
    C_primary_track()
    {
        mState=TrackState::removed;
        isUpdating = false;
        uniqId =-1;
    }
    void Remove()
    {
        mState=TrackState::removed;
        isUpdating = false;
        uniqId =-1;
    }
    void init(object_t* obj1,object_t* obj2,int id=-1)
    {

        double dtime = (obj1->timeMs-obj2->timeMs)/3600000.0;
        double dx = obj1->xkm - obj2->xkm;
        double dy = obj1->ykm - obj2->ykm;
        rgSpeedkmh = (obj1->rgKm-obj2->rgKm)/dtime;
        //        isRemoved  = false;
        mSpeedkmh  = sqrt(dx*dx+dy*dy)/dtime;
        //        isLost     = false;
        courseRad = ConvXYToAziRd(dx,dy);
        mSpeedkmhFit    = sqrt(dx*dx+dy*dy)/dtime;
        courseRadFit   = ConvXYToAziRd(dx,dy);
        possibleMaxScore = 0;
        xkm             = obj1->xkm;
        ykm             = obj1->ykm;
        rgKm            = ConvXYToRg(xkm,ykm);
        aziDeg          = degrees(ConvXYToAziRd(xkm,ykm));
        lastTimeMs = obj1->timeMs;
        objectList.push_back(*obj2);
        objectList.push_back(*obj1);
        objectHistory.push_back(*obj1);
//        time=obj2->timeMs;
        uniqId =id;
        isUpdating = false;
        mState = TrackState::newDetection;
        //        operatorID = 0;
    }
    ~C_primary_track()
    {

    }
    static int IDCounter ;
    bool isRemoved()
    {
        return mState==TrackState::removed;
    }
    bool isLost()
    {
        return mState==TrackState::lost;
    }
    TrackState mState;
    uint ageMs;
    //    int operatorID;
//    uint time;
    int uniqId;
    bool isUpdating;
    void update(uint now_ms)
    {
        isUpdating = true;
        ageMs=now_ms-lastTimeMs;
        if(ageMs>180000)
            mState = TrackState::removed;
        else
            if(ageMs>120000)
                mState = TrackState::lost;
        if(possibleMaxScore>0)
        {
            if(now_ms-possibleObj.timeMs>300)
            {
                objectList.push_back(possibleObj);
                while(objectList.size()>4)
                {
                    objectList.erase(objectList.begin());

                }
                if(objectList.size()>3)         mState = TrackState::confirmed;
                lastTimeMs = possibleObj.timeMs;
                if((lastTimeMs-objectHistory.back().timeMs)>60000)
                    objectHistory.push_back(possibleObj);
                possibleMaxScore = 0;
                if(objectList.size()<4)
                {//new target
                    object_t* obj1  = &(objectList.back());
                    object_t* obj2  = &(objectList.back())-1;
                    double dx       = obj1->xkm - obj2->xkm;
                    double dy       = obj1->ykm - obj2->ykm;
                    double dtime    = (obj1->timeMs-obj2->timeMs)/3600000.0;
                    rgSpeedkmh      = (obj1->rgKm-obj2->rgKm)/dtime;
                    //speed param
                    mSpeedkmhFit    = sqrt(dx*dx+dy*dy)/dtime;
                    sko_spd         = mSpeedkmhFit/2.0;
                    //course param
                    courseRadFit    = ConvXYToAziRd(dx,dy);
                    courseDeg = degrees(courseRadFit);
                    sko_cour = 30.0;
                    //xy coordinates
                    xkm             = obj1->xkm;
                    ykm             = obj1->ykm;
                    //range
                    rgKm            = ConvXYToRg(xkm,ykm);
                    sko_rgKm          = obj1->rgStdEr;
                    //azi
                    aziDeg          = degrees(ConvXYToAziRd(xkm,ykm));
                    sko_aziDeg         = degrees((obj1->aziStdEr));
                }
                else
                {
                    LinearFit();
                    object_t* obj1  = &(objectList.back());
                    object_t* obj2  = &(objectList.back())-3;
                    double dx       = obj1->xkm - obj2->xkm;
                    double dy       = obj1->ykm - obj2->ykm;
                    double dtime    = (obj1->timeMs-obj2->timeMs)/3600000.0;
                    rgSpeedkmh      = (obj1->rgKm-obj2->rgKm)/dtime;
                    //speed param
                    double mSpeedkmhFitNew    = sqrt(dx*dx+dy*dy)/dtime;
                    double sko_spdNew = abs(mSpeedkmhFitNew-mSpeedkmhFit);
                    sko_spd         +=(sko_spdNew-sko_spd)/5.0;
                    mSpeedkmhFit    +=(mSpeedkmhFitNew-mSpeedkmhFit)/2.0;
                    //course param
                    courseRadFit   = ConvXYToAziRd(dx,dy);
                    double courseDegNew = degrees(courseRadFit);
                    double sko_courNew = abs(courseDegNew-courseDeg);
                    courseDeg       +=(courseDegNew-courseDeg)/2.0;
                    sko_cour        +=(sko_courNew-sko_cour)/5.0;
                    //xy
                    xkm             = obj1->xkm;
                    ykm             = obj1->ykm;
                    //range
                    rgKm            = ConvXYToRg(xkm,ykm);
                    double sko_rgNew         = abs(rgKm-obj1->rgKm);
                    sko_rgKm+=(sko_rgNew-sko_rgKm)/5.0;
                    //azi
                    aziDeg          = degrees(ConvXYToAziRd(xkm,ykm));
                    double sko_aziNew         = abs(aziDeg-degrees(obj1->azRad));
                    sko_aziDeg += (sko_aziNew-sko_aziDeg)/5.0;
                }
            }

        }
        isUpdating = false;
    }
    //uint  dtime;
    double lineScore;
    double mSpeedkmhFit;
    std::vector<object_t> objectList;
    std::vector<object_t> objectHistory;
    object_t possibleObj;
    float                   possibleMaxScore;
    double                  courseRadFit;
    double                  courseDeg;
    double                  rgSpeedkmh;
    double                  xkm,ykm;
    double                  sko_aziDeg;
    double                  sko_rgKm;
    double                  sko_spd;
    double                  sko_cour;
    uint          lastTimeMs;
    void LinearFit();
    void addPossible(object_t *obj, double score);
    double LinearFitCost(object_t *myobj);
    double estimateScore(object_t *obj1);
    static double estimateScore(object_t *obj1, object_t *obj2);
    double aziDeg,rgKm;
private:
    double courseRad;
    double mSpeedkmh;
};

//______________________________________//
enum imgDrawMode
{
    VALUE_ORANGE_BLUE = 0,
    VALUE_YELLOW_SHADES = 1,
    DOPLER_3_COLOR = 2,
};
enum DataOverLay { m_only, s_m_200, s_m_150 , max_s_m_200, max_s_m_150};
//enum RotationDir { Right , Left};
class C_radar_data {
public:

    C_radar_data();
    ~C_radar_data();
//    float k_vet;// !!!!

    short                       rotDir;
    double                      rotation_per_min ;
    double                      azi_er_rad;
    std::vector<C_primary_track>        mTrackList;
    std::vector<plot_t>         plot_list;
    std::vector<object_t>       mFreeObjList;
    unsigned     long int       mPeriodCount;
    float rgStdErr;
    qint64 time_start_ms;
    double sn_scale;
    bool isTrueHeading;
    bool isDrawn;
    //    double mShipHeading;
    uint   now_ms ;
    //    bool                    isEncoderAzi;
    //    int                     mEncoderAzi;
    unsigned char           mHeader[FRAME_HEADER_SIZE];
    unsigned char           overload, init_time, clk_adc;
    float                   scale_ppi,scale_zoom_ppi;
    short                   curAzir,arcMinAzi,arcMaxAzi,arcWidth;
    void                    setZoomRectAR(float ctx, float cty, double sizeKM, double sizeDeg);
    void                    setZoomRectXY(float ctx, float cty);
    unsigned int            sn_stat,chu_ky;
    unsigned short          *tb_tap;
    double                  tb_tap_k;
    //    int                     get_tb_tap();
    bool                    is_do_bup_song;
    bool                    isClkAdcChanged,xl_dopler,noise_nornalize,isSled,filter2of3;
    bool                    isManualTune,cut_noise,bo_bang_0,data_export;
    bool                    isSelfRotation;
    double                   krain,kgain,ksea,brightness;
    double                   krain_auto,kgain_auto,ksea_auto;
    void setAutorgs( bool aut);
    void                    clearPPI();
    unsigned char           moduleVal;
    double                   aziOffsetRad;
    DataOverLay             dataOver;
    //    unsigned char           noise_level[8];
    unsigned char           rotation_speed;
    unsigned short          range_max;
    QImage                  *img_ppi,*img_RAmp,*img_zoom_ppi,*img_histogram,*img_spectre,*img_zoom_ar;
    int                     zoom_ar_w,zoom_ar_h,zoom_ar_a0,zoom_ar_r0,zoom_ar_a1,zoom_ar_r1;
    int                     zoom_ar_size_a,zoom_ar_size_r;
    imgDrawMode             imgMode;
    //double                  sn_scale;
    //    void deleteTrack(ushort trackNum);
    void drawRamp();

    //______________________________________//
    //    void        assembleDataFrame(unsigned char *data, unsigned short dataLen);
    void        UpdateData();
    void        ProcessDataFrame();
    void        ProcessData(unsigned short azi, unsigned short lastAzi);
    void        raw_map_init();
    void        raw_map_init_zoom();
    void        drawAzi(short azi);
    void        drawBlackAzi(short azi_draw);
    void        DrawZoom(short azi_draw, short r_pos);
    //    void        blackLine(short x0, short y0, short x1, short y1);
    //    void        addTrackManual(double x, double y);
    //    void        addTrack(object_t *mObject);
    static    void        kmxyToPolarDeg(double x, double y, double *azi, double *range);
    void        setAziOffset(double trueN_deg){

        while(trueN_deg<0)trueN_deg+=360;
        while(trueN_deg>=360)trueN_deg-=360;
        aziOffsetRad =(trueN_deg/360.0*PI_NHAN2);
        raw_map_init();
        resetTrack();
    }
    void        setScalePPI(float scale);
    void        setScaleZoom(float scale);
    void        resetData();
    void        resetSled();
    void        setProcessing(bool onOff);
    //bool        getDataOverload(){if(isDataTooLarge) {isDataTooLarge =false;return true;} else return false;}
    bool        checkFeedback(unsigned char* command);
    bool giaQuayPhanCung;
    unsigned char* getFeedback()
    {

        return (unsigned char*)&command_feedback[0];
    }
    void        resetTrack();
    //    void SetHeaderLen(short len);
    //    bool getDoubleFilter() const;
    //    void setDoubleFilter(bool value);

    void SelfRotationOn(double rate);
    void SelfRotationOff();
    void SelfRotationReset();
    void drawRamp(double azi);
    double getCurAziRad() const;

    bool getIsVtorih() const;
    void setIsVtorih(bool value);

    bool getIsSharpEye() const;
//    void setIsSharpEye(bool value);

    double getArcMaxAziRad() const;
    double getArcMinAziRad() const;
    void addDetectionZone(double x, double y,double dazi,double drg);
    std::vector<DetectionWindow> mDetectZonesList;
private:

    int mFalsePositiveCount;
    float hsTap ;
    std::queue<int>  aziToProcess;//hàng chờ các frame cần xử lý
    //QVector<QRgb> colorTable;
    double      selfRotationDazi,selfRotationRate;
    double      selfRotationAzi;
    bool        isVtorih;
    bool        avtodetect;
    //    bool        doubleFilter;
    uint        getColor(unsigned char pvalue, unsigned char dopler, unsigned char sled);
    void        drawSgn(short azi_draw, short r_pos);
    void        drawSgnZoom(short azi_draw, short r_pos);
    unsigned char command_feedback[8];
    void        polarToXY(float *x, float *y, float azi, float range);
    bool        isProcessing;
//    bool        isSharpEye;
    float       noiseAverage,rainLevel,noiseVar;
    void        getNoiseLevel();
    void        procPix(short proc_azi,short range);
    //    void        procTracks(unsigned short curA);
    void        procPLot(plot_t* mPlot);
    //    bool procObjectAvto(object_t* pObject);
    //    bool procObjectManual(object_t* pObject);
    //void status_start();
    //FILE *pFile;

    //    void decodeData(int azi);
    //void initZoomAR(int a0, int r0);
    bool DrawZoomAR(int a,int r,short val,short dopler,short sled);
    //    int getNewAzi();
    void ProcessEach90Deg();
    int ssiDecode(ushort nAzi);
    //    void DetectTracks();
    //    double estimateScore(object_t *obj1, object_t *obj2);
    void ProcessTracks();
    bool checkBelongToTrack(object_t *obj1);
    bool checkBelongToObj(object_t *obj1);
    //    double estimateScore(object_t *obj1, track_t *track);
    void CreateTrack(object_t *obj1, object_t *obj2);
    //    void LinearFit(track_t *track);
    void LeastSquareFit(C_primary_track* track);
    //    double LinearFitCost(track_t *track, object_t *myobj);
    void ProcessGOData(unsigned char *data, short len, int azi);
public:
    unsigned char mSledValue;
    int mEncoderVal;
    bool isMarineMode;
    //void drawZoomAR();
    float getNoiseAverage() const;
    void setNoiseAverage(float value);
    void setTb_tap_k(double value);
    double getSelfRotationAzi() const;
    void setSelfRotationAzi(int value);
    void processSocketData(unsigned char *data, short len);
    bool ProcessObject(object_t *obj1);
    //    void ProcessObjects();
    //    inline static double ConvXYToRange(double x, double y);
    //    inline static double ConvXYToAziRad(double x, double y);
    void resetGain();
};

//extern C_radar_data radarData;

#endif
