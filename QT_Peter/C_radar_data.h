#ifndef C_RAW_DATA_H
#define C_RAW_DATA_H
//  |----------------------------------------------------------|
//  |HR2D signal processing class and tracking algorithm       |
//  |First release: Nov 2015                                   |
//  |Last update: Sep 2019                                     |
//  |Author: Phung Kim Phuong                                  |
//  |----------------------------------------------------------|
#include <common.h>
//#define DEBUGMODE
#ifdef THEON
#define TRACK_LOST_TIME 110000
#define TRACK_DELETE_TIME 180000
#define TRACK_MAX_DTIME 100000
#define TRACK_MIN_DTIME 500
#define TRACK_STABLE_LEN          15
#define TRACK_START_DOPLER      2.0
#define TARGET_MAX_SPEED_MARINE     80.0//kmh
//#define AZI_ERROR_STD 0.05//1.5 deg
#define MAX_TRACKS_COUNT          2000
#define RAD_DISPLAY_RES             700//768
#else
#define TRACK_STABLE_LEN          3
#define TRACK_MAX_DTIME     130000
#define TRACK_MIN_DTIME     500
#define TRACK_LOST_TIME     150000
#define TRACK_DELETE_TIME   180000
#define TRACK_LOW_SCORE_TIME 90000
//#define AZI_ERROR_STD 0.06
#define MAX_TRACKS_COUNT                  99
#define RAD_DISPLAY_RES             650//768
#endif
#define AIS_UPDATE_TIME 20000
#define MAX_OBJ_SIZE 0.45//400m

#define ARMA_USE_LAPACK
#define ARMA_USE_BLAS
#define ARMA_BLAS_UNDERSCORE
#define MODE_MARINE

#define MIN_TERRAIN                 10
//#define TRACK_CONFIRMED_SIZE        3
//#define TRACK_INIT_STATE            3


#define FRAME_HEADER_SIZE 34
#define RADAR_RESOLUTION 2048
#define MAX_FRAME_SIZE RADAR_RESOLUTION*2+FRAME_HEADER_SIZE
#define RADAR_RESOLUTION_HALF 1024
#define MAX_FRAME_SIZE_HALF RADAR_RESOLUTION_HALF*2+FRAME_HEADER_SIZE
#define CONST_E 2.71828182846
#define MAX_TRACK_LEN               400
#define ENCODER_RES                 5000
#define MAX_AZIR                    2048
#define MAX_AZIR_DRAW               6144
//#define RAD_M_PULSE_RES             1536
#define RAD_S_PULSE_RES             256
//#define RAD_DISPLAY_RES             700//768
#define RAD_FULL_RES                1792
#define SIGNAL_SCALE_0              0.01536//0.015070644 //15.384615384615384615384615384615
#define SIGNAL_SCALE_7              (SIGNAL_SCALE_0*128)//1.92904238624 //215.38461538461538461538461538461
#define SIGNAL_SCALE_6              (SIGNAL_SCALE_0*64)//0.96452119312 //184.61538461538461538461538461538
#define SIGNAL_SCALE_5              (SIGNAL_SCALE_0*32)//0.48226059656 //153.84615384615384615384615384615
#define SIGNAL_SCALE_4              (SIGNAL_SCALE_0*16)//0.24113029828 // 123.07692307692307692307692307692
#define SIGNAL_SCALE_3              (SIGNAL_SCALE_0*8)//0.12056514914 //92.307692307692307692307692307692
#define SIGNAL_SCALE_2              (SIGNAL_SCALE_0*4)//0.06028257456 //61.538461538461538461538461538462
#define SIGNAL_SCALE_1              (SIGNAL_SCALE_0*2)//0.03014128728 //30.769230769230769230769230769231

#define TERRAIN_GAIN                0.9f
#define TERRAIN_GAIN_1              0.1f
#define TERRAIN_THRESH              0.5f

#define ZOOM_SIZE                   550
#define DISPLAY_RES_ZOOM            8192
#define DISPLAY_SCALE_ZOOM          4

#include "c_config.h"
#include <vector>
#include <QImage>
#include <QDateTime>
#include <QFile>
#include <time.h>
#include <queue>
#include <AIS/AIS.h>

struct PointInt
{
    int x,y;
};
struct PointDouble
{
    double x,y;
};
struct PointAziRgkm
{
    double aziRad,rg;
};
inline double dlon2xkm(double lon1,double lon2,double refLatDeg)
{
    return  (((lon1 - lon2) * 111.31949079327357*cos(radians(refLatDeg))));
}
inline double dlat2ykm(double lat1,double lat2)
{
    return (((lat1 - lat2) * 111.132954));
}

inline double sinFast(double a)
{
    while (a>PI) {
        a-=PI_NHAN2;
    }
    double a2 = a*a;
    return a-a2*a/6.0+a2*a2*a/120.0-a2*a2*a2*a/5040.0;
}
inline void bin2hex(unsigned char byte, char* str)
{
    switch (byte>>4) {
    case 0:
        *str = '0';
        break;
    case 1:
        *str = '1';
        break;
    case 2:
        *str = '2';
        break;
    case 3:
        *str = '3';
        break;
    case 4:
        *str = '4';
        break;
    case 5:
        *str = '5';
        break;
    case 6:
        *str = '6';
        break;
    case 7:
        *str = '7';
        break;
    case 8:
        *str = '8';
        break;
    case 9:
        *str = '9';
        break;
    case 10:
        *str = 'A';
        break;
    case 11:
        *str = 'B';
        break;
    case 12:
        *str = 'C';
        break;
    case 13:
        *str = 'D';
        break;
    case 14:
        *str = 'E';
        break;
    case 15:
        *str = 'F';
        break;
    default:
        break;
    }
    switch (byte&(0x0F)) {
    case 0:
        *(str+1) = '0';
        break;
    case 1:
        *(str+1) = '1';
        break;
    case 2:
        *(str+1) = '2';
        break;
    case 3:
        *(str+1) = '3';
        break;
    case 4:
        *(str+1) = '4';
        break;
    case 5:
        *(str+1) = '5';
        break;
    case 6:
        *(str+1) = '6';
        break;
    case 7:
        *(str+1) = '7';
        break;
    case 8:
        *(str+1) = '8';
        break;
    case 9:
        *(str+1) = '9';
        break;
    case 10:
        *(str+1) = 'A';
        break;
    case 11:
        *(str+1) = 'B';
        break;
    case 12:
        *(str+1) = 'C';
        break;
    case 13:
        *(str+1) = 'D';
        break;
    case 14:
        *(str+1) = 'E';
        break;
    case 15:
        *(str+1) = 'F';
        break;
    default:
        break;
    }

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
inline double fastPow(double a, double b) {
    union {
        double d;
        int x[2];
    } u = { a };
    u.x[1] = (int)(b * (u.x[1] - 1072632447) + 1072632447);
    u.x[0] = 0;
    return u.d;
}
inline double likelihood(double e, double sigma)
{
    return fastPow(CONST_E,-sq(e/sigma));
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

typedef std::map<std::pair<int,int>, int> DensityMap;

typedef struct
{
    bool isRemoved;
    bool isAllowDetection;
    qint64 timeStart;
    double xkm,ykm;
    double aziDeg,rg;
    double maxDrg,maxDazDeg;
}RangeAziWindow;
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
    double          azRad ,rg,xkm,ykm,lat,lon;
    bool isUserInitialized;
    //    double          xkmfit,ykmfit;
    //    double          azRadfit,rgKmfit;
    double          rgKm;
    short           dazi,drg;
    int             energy;
    short           size;
    char            dopler;
    //    bool            isProcessed;
    bool            isRemoved;
    //    float           p;
    //    float          terrain;
    double           rgStdEr;
    double           aziStdEr;
    qint64          timeMs;
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
enum class TrackState {newDetection = 0,
                       confirmed    = 3,
                       lost         = 2,
                       manualTrack  = 4,
                       removed      = 1};
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
    QString printData()
    {
        QString output;
        output.append(QString::fromUtf8("Số hiệu:")+QString::number(uniqId)+"\n");
        output.append(QString::fromUtf8("Cự ly(hải lý):")+QString::number(nm(rgKm),'f',2)+"\n");
        output.append(QString::fromUtf8("Ph. vị(độ):")+QString::number(aziDeg,'f',2)+"\n");
        output.append(QString::fromUtf8("Tốc độ(hải lý/giờ):")+QString::number(nm(mSpeedkmhFit),'f',1)+"\n");
        output.append(QString::fromUtf8("Hướng di chuyển(độ):")+QString::number(courseDeg,'f',1)+"\n");
        output.append(QString::fromUtf8("Kinh độ:")+demicalDegToDegMin(lon)+"\n");
        output.append(QString::fromUtf8("Vỹ độ:")+demicalDegToDegMin(lat)+"\n");
        output.append(QString::fromUtf8("Cập nhật cách đây:")+QString::number(int(CConfig::time_now_ms-lastUpdateTimeMs)/1000)+"s"+"\n");
        output.append(QString::fromUtf8("Độ hợp lý:")+QString::number(fitProbability,'f',2)+"\n");
        output.append(QString::fromUtf8("Mật độ:")+QString::number(posDensityFit,'f',2)+"\n");
        return output;
    }
    bool isEnemy;
//    bool isDoplerShifted(){return (abs(mDoplerFit)>TRACK_START_DOPLER);}
    bool isHighDensityPos();
    bool isConfirmed(){return mState==TrackState::confirmed;}
    qint64 startTime;
    bool isSelected;
    AIS_object_t     *mAisPossibleObj,*mAisConfirmedObj;
    double  mAisMaxPosibility;
    qint64  mAisMaxPosibilityTimeMs;
    double  fitProbability;
    bool isUserInitialised;
    void init(double txkm,double tykm);
    void init(object_t* obj1,object_t* obj2,int id=-1);

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
    QString mTTM;
    uint ageMs;
    //    int operatorID;
//    uint time;
    int uniqId;
    bool isUpdating;
    double posDensityFit;
    void update();
    //uint  dtime;
    double lineScore;
    double mSpeedkmhFit;
    double mDoplerFit;
    std::vector<object_t> objectList;
    std::vector<object_t> objectHistory;
    object_t                possibleObj;
    double                   possibleMaxScore;
    int                     waitingForBetterScore;
    double                  courseRadFit;
    double                  courseDeg;
    double                  rgSpeedkmh;
    double                  xkm,ykm;
    double                  lat,lon;
    double                  sko_aziDeg;
    double                  sko_rgKm;
    double                  sko_spdKmh;
    double                  sko_cour;
    qint64                  lastUpdateTimeMs;
    int                     mDopler;
    void LinearFit(int nEle);
    void addPossible(object_t *obj, double score);
    void addManualPossible(double xkm,double ykm);
    double LinearFitProbability(object_t *myobj);
    double estimateScore(object_t *obj1);
    static double estimateScore(object_t *obj1, object_t *obj2);
    double aziDeg,rgKm;
    int getPosDensity();
    void checkNewObject();
private:
    double courseRad;
//    double mSpeedkmh;
    void generateTTM();
    uchar getCheckSum(QString message);
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
    bool                        isTxOn;

    bool                        cut_terrain;
    double                      mInverseRotAziCorrection;
    double                      rotation_per_min ;
//    double                      azi_er_rad;
    std::vector<C_primary_track>mTrackList;
    std::vector<plot_t>         plot_list;
    std::vector<object_t>       mFreeObjList;
    bool isManualTracking;
    void setManualTracking(bool isManual)// manual tracking
    {
        isManualTracking = isManual;
    }
    void addManualTrack(double xkm,double ykm);
    C_primary_track*  getManualTrackzone(double xkm,double ykm,double rgkm);
//    double manualTrackX,manualTracky,manualTrackR;
    int      antennaHeadOffset;
    int     freqHeadOffset;
    double rgStdErrKm;
//    qint64 time_start_ms;
    double sn_scale;
//    bool isTrueHeadingFromRadar;
    clock_t mUpdateTime;
    //    double mShipHeading;

    //    bool                    isEncoderAzi;
    //    int                     mEncoderAzi;
    unsigned char           mHeader[FRAME_HEADER_SIZE];
    unsigned char           overload, init_time, clk_adc;

    short                   curAzirTrue2048,arcMinAzi,arcMaxAzi,arcWidth;
    double                  mRealAziRate,mRealAzi;
    void                    setZoomRectAR(float ctx, float cty, double sizeKM, double sizeDeg);
    void                    setZoomRectXY(float ctx, float cty);
    unsigned int            mFreq,sn_stat,chu_ky;
    unsigned short          *tb_tap;
    double                  tb_tap_k;
    //    int                     get_tb_tap();
    bool                    is_do_bup_song;
    bool                    isClkAdcChanged,gat_mua_va_dia_vat,noise_nornalize,filter2of3;
    bool                    isManualTune,cut_noise,bo_bang_0,data_export;
    bool                    isSelfRotation;
    double                   krain,kgain,ksea;
    double                   krain_auto,kgain_auto,ksea_auto;
    void setAutorgs( bool aut);
    void                    clearPPI();
    void setBrightness(double value);
    void setImgMode(imgDrawMode mode);
    void SetSled(bool sled);
    bool                    integrateAisPoint(AIS_object_t *obj);
    unsigned char           moduleVal;
    int                  aziViewOffset;
    double                  aziRotCorrection;
    DataOverLay             dataOver;
    //    unsigned char           noise_level[8];
    unsigned char           rotation_speed;

//    QImage     *mimg_ppi,*mimg_RAmp,*mimg_zoom_ppi,*mimg_histogram,*mimg_spectre,*mimg_zoom_ar;


    //double                  sn_scale;
    //    void deleteTrack(ushort trackNum);
    void drawRamp();

    //______________________________________//
    //    void        assembleDataFrame(unsigned char *data, unsigned short dataLen);
    bool UpdateData();
    void        ProcessDataFrame();
    void        ProcessData(unsigned short azi, unsigned short lastAzi);
    void        raw_map_init();
    void        raw_map_init_zoom();
//    void        drawAzi(short azi);
    void        drawBlackAzi(short azi_draw);
//    void        DrawZoom(short azi_draw, short r_pos);
    //    void        blackLine(short x0, short y0, short x1, short y1);
    //    void        addTrackManual(double x, double y);
    //    void        addTrack(object_t *mObject);
//    static    void        ConvkmxyToPolarDeg(double x, double y, double *azi, double *range);
//    void        setAziOffset(double trueN_deg){

//        while(trueN_deg<0)trueN_deg+=360;
//        while(trueN_deg>=360)trueN_deg-=360;
//        aziViewOffsetRad =radians(trueN_deg);
//        raw_map_init();
//        resetTrack();
//    }
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
    double getCurAziTrueRad() const;

    bool getIsVtorih() const;
    void setIsVtorih(bool value);

    bool getIsSharpEye() const;
//    void setIsSharpEye(bool value);

    double getArcMaxAziRad() const;
    double getArcMinAziRad() const;
    void addDetectionZone(double x, double y,double dazi,double drg, bool isOneTime);
    std::vector<RangeAziWindow> mDetectZonesList;
private:

    bool isAutoTracking;
    int max_drange_plot;
    QTransform mPPITrans;
//    bool isShipHeadingChanged;
    int mShipHeading2048;
    int mFalsePositiveCount;
    float hsTap ;
//    std::queue<int>  aziToProcess;//hàng chờ các frame cần xử lý
    //QVector<QRgb> colorTable;
    double      selfRotationDazi,selfRotationRate;
    double      selfRotationAzi;
    bool        isVtorih;
    bool        avtodetect;
    //    bool        doubleFilter;
    uint        getColor(unsigned char pvalue, unsigned char dopler, unsigned char sled);
//    void        drawSgn(short azi_draw, short r_pos);
//    void        drawSgnZoom(short azi_draw, short r_pos);
    unsigned char command_feedback[8];
//    void        ConvPolarToXY(double *x, double *y, double azi, double range);
    bool        isProcessing;
//    bool        isSharpEye;
    float       noiseAverage,rainLevel,noiseVar;
    void        getNoiseLevel();
    void        procPix(short proc_azi, short lastAzi, short range);
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
    void ProcessGOData(unsigned char *data, short len, int aziMH);
    void addDetectionZone(RangeAziWindow dw);
    int approximateAzi(int newAzi);
    bool checkInsideDWAvoid(double aziDeg, double rgkm);
    bool checkInsideDWAllow(double aziDeg, double rgkm);
    void addFreeObj(object_t *obj1);
    bool checkSimilarityToExistingTracks(object_t *obj1);
    void UpdateTrackStatistic();
    void getDensity(double azi, double range);
public:
    void updateTerrain();
    void saveTerrain();
    void loadTerrain();
    unsigned int maxTer;
    bool mTerrainAvailable;
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
    void ProcessObject(object_t *obj1);
    //    void ProcessObjects();
    //    inline static double ConvXYToRange(double x, double y);
    //    inline static double ConvXYToAziRad(double x, double y);
    void resetGain();
//    void setShipHeading(int shipHeading);
//    void setShipHeadingDeg(double headingDeg);
    void setAziViewOffsetDeg(double angle);
    void addDetectionZoneAZ(double az, double rg, double dazi, double drg, bool isAllowDetection);
    void setFreqHeadOffsetDeg(double offset);
    QImage *getMimg_ppi() const;
    QImage *getMimg_RAmp() const;
    QImage *getMimg_zoom_ppi() const;
    QImage *getMimg_histogram() const;
    QImage *getMimg_spectre() const;
    QImage *getMimg_zoom_ar() const;
    double getScale_ScreenPerPpi() const;
    double getScale_zoom_ppi() const;
    DensityMap *getDensityMap();
    void addDensityPoint(double lat, double lon);
    void loadDensityMap();
    int getDensityLatLon(double lat, double lon);
//    static void ConvWGSToKm(double *x, double *y, double m_Long, double m_Lat);
    //    bool CheckInsideManualZone(double xkm, double ykm);
    static void ConvKmToWGS(double x, double y, double *m_Long, double *m_Lat);
    static void ConvPolarToXY(double *x, double *y, double azi, double range);
    static void ConvkmxyToPolarDeg(double x, double y, double *azi, double *range);
    static void ConvWGSToKm(double *x, double *y, double m_Long, double m_Lat);
    void setIsAutoTracking(bool value);
    double getScale_PpiPerKm() const;
};

//extern C_radar_data radarData;

#endif
