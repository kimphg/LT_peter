#define PI 3.1415926536
#include "c_config.h"
#include "c_radar_data.h"
//#include <QElapsedTimer>
#include <cmath>
//#include <QtDebug>

#define PLOT_MAX_SIZE 80
#define PLOT_MIN_SIZE 5
#define PLOT_MAX_DR 10
#define RANGE_MIN 2
#define TERRAIN_MAX 40
#define TERRAIN_INIT 20
#define RADAR_COMMAND_FEEDBACK  6
#define RADAR_HEADER_LEN   34
#define RADAR_DATA_SPECTRE      22
#define RADAR_DATA_MAX_SIZE     2688
#define RADAR_GAIN_MIN 3.0
#define RADAR_GAIN_MAX 9.0
#define AZI_ERROR_STD 0.01746
#define TARGET_OBSERV_PERIOD 6500//ENVAR max periods to save object in the memory
static  FILE *logfile;
int C_primary_track::IDCounter =1;
static int sumvar = 0;
static int nNoiseCalculator = 0;
static short lastProcessAzi = 0;

/*
+-------+-----------+-----------------------------------------------------+
|       |           |                                                     |
|   STT |   Byte    |   Chức                                              |
|       |           |   năng                                              |
|       |           |                                                     |
+-------+-----------+-----------------------------------------------------+
|       |           |                                                     |
|   1   |   0       |   Id gói                                            |
|       |           |   tin:                                              |
|       |           |   0,1,2,3:                                          |
|       |           |   iq th mã pha (mỗi kênh 2048 byte)                 |
|       |           |   4: 256                                            |
|       |           |   byte máy hỏi, mỗi bít một o_cu_ly                 |
|       |           |   5: iq th                                          |
|       |           |   giả liên tục, 512 byte i, 512 byte q              |
|       |           |   6,7: iq                                           |
|       |           |   cho tín hiệu xung đơn, mỗi kênh 1024 byte         |
|       |           |                                                     |
+-------+-----------+-----------------------------------------------------+
|       |           |                                                     |
|   2   |   1, 2, 3 |   Byte cho                                          |
|       |           |   báo hỏng:                                         |
|       |           |   1: loại                                           |
|       |           |   mô-đun, (0, 1, 2, 3)                              |
|       |           |   2: Loại                                           |
|       |           |   tham số (bb, cc, dd)                              |
|       |           |   3: Tham                                           |
|       |           |   số mô-đun                                         |
|       |           |                                                     |
+-------+-----------+-----------------------------------------------------+
|       |           |                                                     |
|   3   |   4       |   Phân giải                                         |
|       |           |   ra đa: 0 (15m), 1 (30m)......                     |
|       |           |                                                     |
+-------+-----------+-----------------------------------------------------+
|       |           |                                                     |
|   4   |   5,6     |   Loại tín                                          |
|       |           |   hiệu phát và tham số:                             |
|       |           |   5: loại                                           |
|       |           |   th phát (0: xung đơn; 1: mã pha; 2: giả ltuc)     |
|       |           |   6: tham                                           |
|       |           |   số cho loại th trên                               |
|       |           |                                                     |
+-------+-----------+-----------------------------------------------------+
|       |           |                                                     |
|   5   |   7,8     |   Hai byte                                          |
|       |           |   trung bình tạp máy thu (ktra báo hỏng tuyến thu)  |
|       |           |                                                     |
+-------+-----------+-----------------------------------------------------+
|       |           |                                                     |
|   6   |   9, 10,  |   4 byte                                            |
|       |   11, 12  |   quay an-ten                                       |
|       |           |                                                     |
+-------+-----------+-----------------------------------------------------+
|       |           |                                                     |
|   7   |   13, 14  |   Hai byte                                          |
|       |           |   hướng tàu                                         |
|       |           |                                                     |
+-------+-----------+-----------------------------------------------------+
|       |           |                                                     |
|   8   |   15, 16  |   Hai byte                                          |
|       |           |   hướng mũi tàu                                     |
|       |           |                                                     |
+-------+-----------+-----------------------------------------------------+
|       |           |                                                     |
|   9   |   17, 18  |   Hai byte                                          |
|       |           |   tốc độ tàu                                        |
|       |           |                                                     |
+-------+-----------+-----------------------------------------------------+
|       |           |                                                     |
|   10  |   19      |   Thông                                             |
|       |           |   báo chế độ chủ đông - bị động, tốc độ quay an-ten |
|       |           |   - bít thấp                                        |
|       |           |   thông báo cđ-bđ (1: chủ động)                     |
|       |           |   - 4 bít                                           |
|       |           |   cao là tốc độ an-ten                              |
|       |           |                                                     |
+-------+-----------+-----------------------------------------------------+
|       |           |                                                     |
|   11  |   20      |   Thông                                             |
|       |           |   báo tần số phát và đặt mức tín hiệu:              |
|       |           |   - 4 bít                                           |
|       |           |   cuối là tần số phát                               |
|       |           |   - 4 bít                                           |
|       |           |   cao là đặt mức th                                 |
|       |           |                                                     |
+-------+-----------+-----------------------------------------------------+
|       |           |                                                     |
|   12  |   21      |   Thông                                             |
|       |           |   báo chọn thang cự ly và bật/tắt AM2:              |
|       |           |   - 4 bít                                           |
|       |           |   cuối là thang cự ly (0: 2 lý; 1: 4 lý.....)       |
|       |           |   - 4 bít                                           |
|       |           |   cao là báo bật/tắt AM2: 0: tắt, 1: bật            |
|       |           |                                                     |
+-------+-----------+-----------------------------------------------------+
|       |           |                                                     |
|   13  |   22      |   Thông                                             |
|       |           |   báo số điểm FFT:                                  |
|       |           |   1(fft8);                                          |
|       |           |   2(fft16) ;...;32(fft256)                          |
|       |           |                                                     |
+-------+-----------+-----------------------------------------------------+
*/
typedef struct  {
    //processing dataaziQueue
    unsigned char level [MAX_AZIR][RADAR_RESOLUTION];
    unsigned char may_hoi[MAX_AZIR][RADAR_RESOLUTION];
    unsigned char level_disp [MAX_AZIR][RADAR_RESOLUTION];
    bool          detect[MAX_AZIR][RADAR_RESOLUTION];
    //unsigned char rainLevel[MAX_AZIR][RADAR_RESOLUTION];
    unsigned char dopler[MAX_AZIR][RADAR_RESOLUTION];
    unsigned char terrain[MAX_AZIR][RADAR_RESOLUTION];
    //    unsigned char dopler_old[MAX_AZIR][RADAR_RESOLUTION];
    //    unsigned char dopler_old2[MAX_AZIR][RADAR_RESOLUTION];
    unsigned char sled[MAX_AZIR][RADAR_RESOLUTION];
    unsigned char hot[MAX_AZIR][RADAR_RESOLUTION];
    unsigned char hot_disp[MAX_AZIR][RADAR_RESOLUTION];
    short         plotIndex[MAX_AZIR][RADAR_RESOLUTION];
    //display data
    unsigned char display_ray [RAD_DISPLAY_RES][3];//0 - signal, 1- dopler, 2 - sled;
    unsigned char display_ray_zoom [DISPLAY_RES_ZOOM][3];
    unsigned char display_mask [RAD_DISPLAY_RES*2+1][RAD_DISPLAY_RES*2+1];
    unsigned char display_mask_zoom [DISPLAY_RES_ZOOM*2+1][DISPLAY_RES_ZOOM*2+1];
    short xkm[MAX_AZIR_DRAW][RAD_DISPLAY_RES+1];
    short ykm[MAX_AZIR_DRAW][RAD_DISPLAY_RES+1];
    short xzoom[MAX_AZIR_DRAW][DISPLAY_RES_ZOOM];
    short yzoom[MAX_AZIR_DRAW][DISPLAY_RES_ZOOM];
} signal_map_t;

static short curIdCount = 1;
static qint64 cur_timeMSecs = 0;//QDateTime::currentMSecsSinceEpoch();
static signal_map_t data_mem;
static float                   rot_period_sec =0;
static short histogram[256];
void C_primary_track::addPossible(object_t *obj,double score)
{
    possibleObj=(*obj);
    possibleMaxScore=score;
}

double C_primary_track::estimateScore(object_t *obj1,object_t *obj2)
{
    double dtime = (obj1->timeMs - obj2->timeMs);
    if(dtime<500)return -1;
    dtime/=3600000.0;//time in hours
    //ENVAR min time between plots in a line(1s)

    double dx = obj1->xkm - obj2->xkm;
    double dy = obj1->ykm - obj2->ykm;

    double distancekm = sqrt(dx*dx+dy*dy);

    double speedkmh = distancekm/(dtime);
    if(speedkmh>500)return -1;
    double distanceCoeff = distancekm/(TARGET_MAX_SPEED_MARINE*dtime+ obj1->rgKm*AZI_ERROR_STD);
    if(distanceCoeff>1.0)return -1;
    double rgSpeedkmh = abs(obj1->rgKm - obj2->rgKm)/(dtime);

    if(rgSpeedkmh>150.0)return -1;
    return 3.0 - speedkmh/430.0 - distanceCoeff - abs(rgSpeedkmh)/120.0;
    //return 0.4587003 - 0.00102045*speedkmh - 0.02371061*rgSpeedkmh -0.0084888*distanceCoeff;
    // calculate score using machine learning model
    /*
    if(distanceCoeff<0.285928)
    {
        if(abs(rgSpeedkmh)<44.409351)return 0.971;
        else
        {
            if(distanceCoeff<0.127)return 0.57;
            else return 0.116;
        }
    }
    else
    {
        if(speedkmh<54.774822)
        {
            return 0.926174;
        }
        else
            return 0.01;
    }*/

}
#define FIT_ELEMENTS 3
double C_primary_track::LinearFitCost(object_t *myobj)
{
    int nEle = FIT_ELEMENTS;
    //    if(this->objectList.size()<nEle-1)
    //    {
    //        printf("\nLinearFitCost error");
    //        return 0 ;
    //    }
    object_t *obj[FIT_ELEMENTS];
    obj[0] = &(objectList[this->objectList.size()-2]);
    obj[1] = &(objectList[this->objectList.size()-1]);
    obj[2] = myobj;

    double y1[FIT_ELEMENTS];
    double y2[FIT_ELEMENTS];
    double t[FIT_ELEMENTS];
    for(int i=0;i<FIT_ELEMENTS;i++)
    {
        y1[i] = obj[i]->xkm;
        y2[i] = obj[i]->ykm;
        t[i] = obj[i]->timeMs;
    }
    double y1sum = 0;//r1+r2+r3;
    double y2sum = 0;
    double tsum = 0;
    double t2sum = 0;
    double y1tsum = 0;
    double y2tsum = 0;
    for(int i=0;i<FIT_ELEMENTS;i++)
    {
        y1sum+=y1[i];
        y2sum+=y2[i];
        tsum+=t[i];
        t2sum+=t[i]*t[i];
        y1tsum+=y1[i]*t[i];
        y2tsum+=y2[i]*t[i];
    }
    //    a=(n*xysum-xsum*ysum)/(n*x2sum-xsum*xsum);            //calculate slope
    //    b=(x2sum*ysum-xsum*xysum)/(x2sum*n-xsum*xsum);            //calculate intercept
    double y1a = (nEle*y1tsum-tsum*y1sum)/(nEle*t2sum-tsum*tsum);
    double y1b = (t2sum*y1sum-tsum*y1tsum)/(t2sum*nEle-tsum*tsum);
    for(int i=0;i<FIT_ELEMENTS;i++)
    {
        y1[i]=y1a*t[i]+y1b;
    }
    double y2a=(nEle*y2tsum-tsum*y2sum)/(nEle*t2sum-tsum*tsum);
    double y2b=(t2sum*y2sum-tsum*y2tsum)/(t2sum*nEle-tsum*tsum);
    for(int i=0;i<FIT_ELEMENTS;i++)
    {
        y2[i]=y2a*t[i]+y2b;
    }
    double azRadfit[3];
    double rgKmfit[3];
    for(int i=0;i<FIT_ELEMENTS;i++)
    {
        //obj[i].xkm+=(y1[i]-obj[i].xkm)*i/float(nEle);
        //obj[i].ykm+=(y2[i]-obj[i].ykm)*i/float(nEle);
        //obj[i].xkmfit=y1[i];
        //obj[i].ykmfit=y2[i];
        azRadfit[i] = ConvXYToAziRd  (y1[i],y2[i]);
        rgKmfit[i]  = ConvXYToRg   (y1[i],y2[i]);
    }
    //    delete[] y1;
    //    delete[] y2;
    //    delete[] t;
    double costOfFit = 0;
    for(int i=0;i<FIT_ELEMENTS;i++)
    {
        costOfFit+=abs(azRadfit[i]-obj[i]->azRad)/obj[i]->aziStdEr;
        costOfFit+=abs(rgKmfit[i]  -obj[i]->rgKm)/obj[i]->rgStdEr;
    }
    costOfFit/=double(FIT_ELEMENTS);
    return costOfFit;
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
double C_primary_track::estimateScore(object_t *obj1)
{
    //return estimateScore(obj1,&(this->objectList.back()));
    if(objectList.size()<2)
        return -1;
    object_t* obj2 = &(this->objectList.back());
    double dtime = (obj1->timeMs - obj2->timeMs);
    if(dtime<300)
        return -1;//ENVAR min time between plots in a line(1s)
    if(dtime>120000)
        return -1;
    dtime/=3600000.0;
    double dx = obj1->xkm - obj2->xkm;
    double dy = obj1->ykm - obj2->ykm;

    double distancekm = sqrt(dx*dx+dy*dy);
    double distanceCoeff = distancekm/(TARGET_MAX_SPEED_MARINE*dtime   + obj1->rgKm*AZI_ERROR_STD);
    if(distanceCoeff>1.0)return 0;
    double dBearing = ConvXYToAziRd(dx,dy)-this->courseRad;
    double speedkmh = distancekm/(dtime);
    if(speedkmh>500.0)return 0;
    double dSpeed = speedkmh-(this->mSpeedkmh*cosFast(-dBearing));
    if(dSpeed>600.0)return 0;
    float rgSpeedkmh = (obj1->rgKm-obj2->rgKm)/(dtime);
    if(abs(rgSpeedkmh)>120.0)return 0;
    double dRgSp = rgSpeedkmh - this->rgSpeedkmh;
    if(abs(dRgSp)>125.0)return 0;
    if(!this)
    {
        dRgSp=dRgSp;
    }
    double linearFit = LinearFitCost(obj1);
    //normalize machine learning likelihood model
    rgSpeedkmh/=50.0;
    dSpeed/=200.0;
    speedkmh/=150.0;
    dRgSp/=40.0;
    linearFit/=0.16;
    dtime/=0.004167;
    /*fprintf(logfile,"%f,",distanceCoeff);//1.0
    fprintf(logfile,"%f,",speedkmh);//500
    fprintf(logfile,"%f,",abs(dSpeed));//600
    fprintf(logfile,"%f,",abs(rgSpeedkmh));//150
    fprintf(logfile,"%f,",abs(dRgSp));//120
    fprintf(logfile,"%f,",linearFit);//0.5*/
    return fastPow(CONST_E,-sq(distanceCoeff))
            *fastPow(CONST_E,-sq(rgSpeedkmh))
            *fastPow(CONST_E,-sq(dSpeed))
            *fastPow(CONST_E,-sq(speedkmh))
            *fastPow(CONST_E,-sq(dRgSp))
            *fastPow(CONST_E,-sq(linearFit))
            *fastPow(CONST_E,-sq(dtime));
    /*if(obj1->dopler==this->objectList.back().dopler)
    {fprintf(logfile,"1\n");
        return 1;
    }
    else
        fprintf(logfile,"-1\n");
    return -1;*/


}
void C_primary_track::LinearFit()
{
    /*
double xsum=0,x2sum=0,ysum=0,xysum=0;
    for (i=0;i<n;i++)
    {
        xsum=xsum+x[i];
        ysum=ysum+y[i];
        x2sum=x2sum+pow(x[i],2);
        xysum=xysum+x[i]*y[i];
    }
    a=(n*xysum-xsum*ysum)/(n*x2sum-xsum*xsum);
    b=(x2sum*ysum-xsum*xysum)/(x2sum*n-xsum*xsum);
    double y_fit[n];
    for (i=0;i<n;i++)
        y_fit[i]=a*x[i]+b;
*/
    uint nEle=4;
    if(this->objectList.size()<nEle)return;
    object_t* obj = &(this->objectList[(this->objectList.size()- nEle)]) ;
    double *y1 = new double[nEle];
    double *y2 = new double[nEle];
    double *t  = new double[nEle];
    for(uint i=0;i<nEle;i++)
    {
        y1[i] = obj[i].xkm;
        y2[i] = obj[i].ykm;
        t[i] = obj[i].timeMs;
    }
    double y1sum = 0;//r1+r2+r3;
    double y2sum = 0;
    double tsum = 0;
    double t2sum = 0;
    double y1tsum = 0;
    double y2tsum = 0;
    for(uint i=0;i<nEle;i++)
    {
        y1sum+=y1[i];
        y2sum+=y2[i];
        tsum+=t[i];
        t2sum+=t[i]*t[i];
        y1tsum+=y1[i]*t[i];
        y2tsum+=y2[i]*t[i];
    }
    //    a=(n*xysum-xsum*ysum)/(n*x2sum-xsum*xsum);
    //    b=(x2sum*ysum-xsum*xysum)/(x2sum*n-xsum*xsum);
    double y1a = (nEle*y1tsum-tsum*y1sum)/(nEle*t2sum-tsum*tsum);
    double y1b = (t2sum*y1sum-tsum*y1tsum)/(t2sum*nEle-tsum*tsum);
    for(uint i=0;i<nEle;i++)
    {
        y1[i]=y1a*t[i]+y1b;
    }
    double y2a=(nEle*y2tsum-tsum*y2sum)/(nEle*t2sum-tsum*tsum);
    double y2b=(t2sum*y2sum-tsum*y2tsum)/(t2sum*nEle-tsum*tsum);
    for(uint i=0;i<nEle;i++)// !!index start from 1
    {
        y2[i]=y2a*t[i]+y2b;
    }
    for(uint i=1;i<nEle;i++)// !!index start from 1
    {
        obj[i].xkm+=(y1[i]-obj[i].xkm)*i/double(nEle);
        obj[i].ykm+=(y2[i]-obj[i].ykm)*i/double(nEle);
    }
    //    rgSpeedkmh = (obj[nEle-1].rgKmfit-obj[nEle-2].rgKmfit)/
    //            ((obj[nEle-1].timeMs-obj[nEle-2].timeMs)/3600000.0);

    delete[] y1;
    delete[] y2;
    delete[] t;

}

C_radar_data::C_radar_data()
{
    C_primary_track track;
    mTrackList = std::vector<C_primary_track>(MAX_TRACKS,track);
    giaQuayPhanCung = false;
    //    mShipHeading = 0;
    isTrueHeading = true;
    rgStdErr = sn_scale*pow(2,clk_adc);
    azi_er_rad = CConfig::getDouble("azi_er_rad",AZI_ERROR_STD);
    time_start_ms = QDateTime::currentMSecsSinceEpoch();
    mFalsePositiveCount = 0;
    mSledValue = 180;
    rotDir = 0;
    mPeriodCount = 0;
    logfile = fopen("logfile.dat", "wt");
    isMarineMode = true;
    range_max = RADAR_RESOLUTION;
    imgMode = VALUE_ORANGE_BLUE;
    brightness = 1.5;
    hsTap = 0;
    tb_tap=new unsigned short[MAX_AZIR];
    img_histogram=new QImage(257,101,QImage::Format_Mono);
    img_histogram->fill(0);
    img_ppi = new QImage(RAD_DISPLAY_RES*2+1,RAD_DISPLAY_RES*2+1,QImage::Format_ARGB32);
    img_RAmp = new QImage(RADAR_RESOLUTION,256,QImage::Format_ARGB32);
    img_spectre = new QImage(16,256,QImage::Format_Mono);
    img_spectre->fill(0);
    img_zoom_ppi = new QImage(ZOOM_SIZE+1,ZOOM_SIZE+1,QImage::Format_ARGB32);
    img_zoom_ar = nullptr;

    tb_tap_k = 1;
    setZoomRectAR(10,10,1.852,10);
    //    mEncoderAzi = 0;
    //img_zoom_ar->setColorTable(colorTable);
    img_ppi->fill(Qt::transparent);
    isSelfRotation = false;
    isProcessing = true;
    //    isEncoderAzi  =false;
    isManualTune = false;
    isVtorih = true;
    cut_noise = false;
    //    doubleFilter = false;
    rotation_per_min = 0;
    bo_bang_0 = false;
    data_export = false;
    xl_dopler = true;
    noise_nornalize = false;
    filter2of3 = false;
    is_do_bup_song = false;
    clk_adc = 1;
    noiseAverage = 30;
    noiseVar = 8;
    krain_auto = 0.4;
    kgain_auto  = 4.2;
    ksea_auto = 0;
    kgain = 1;
    krain  = ksea = 0.2;
    brightness = 1;
    avtodetect = true;
    isClkAdcChanged = true;
    isSled = false;
    init_time = 5;
    dataOver = max_s_m_200;
    curAzir = 0;
    arcMaxAzi = 0;
    arcMinAzi = 0;
//    isSharpEye = false;
    sn_scale = SIGNAL_SCALE_0;
    raw_map_init();
    raw_map_init_zoom();
    setAziOffset(0);
    setScalePPI(1);
    resetData();
    setScaleZoom(4);
    //setZoomRectAR(0,0);
}
C_radar_data::~C_radar_data()
{
    delete img_ppi;
    fclose(logfile);
    //if(pFile)fclose(pFile);
    //    if(pScr_map)
    //    {
    //        delete[] pScr_map;
    //    }
}

double C_radar_data::getArcMaxAziRad() const
{
    double result = (aziOffsetRad+(double)arcMaxAzi/(double)MAX_AZIR*PI_NHAN2);
    if(result>PI_NHAN2)result-=PI_NHAN2;
    return ( result);
}
double C_radar_data::getArcMinAziRad() const
{
    double result = (aziOffsetRad+(double)arcMinAzi/(double)MAX_AZIR*PI_NHAN2);
    if(result>PI_NHAN2)result-=PI_NHAN2;
    return (result );
}

void C_radar_data::addDetectionZone(double x, double y, double dazi, double drg)
{
    DetectionWindow dw;
    dw.trackCount=1;
    dw.timeStart=now_ms;
    dw.xkm=x;
    dw.ykm=y;
    kmxyToPolarDeg(x,y,&dw.aziDeg,&dw.rg);
    dw.maxDazDeg = (dazi);
    dw.maxDrg = drg;
    for(uint i=0;i<mDetectZonesList.size();i++)
    {
        if(mDetectZonesList[i].trackCount==0||(now_ms-dw.timeStart>80000))
        {
            mDetectZonesList[i] = dw;
            return;
        }
    }
    mDetectZonesList.push_back(dw);
}

double C_radar_data::getSelfRotationAzi() const
{
    return selfRotationAzi;
}

void C_radar_data::setSelfRotationAzi(int value)
{
    selfRotationAzi = value;

}
double C_radar_data::getCurAziRad() const
{

    double result = (aziOffsetRad+(double)curAzir/(double)MAX_AZIR*PI_NHAN2);
    if(result>PI_NHAN2)result-=PI_NHAN2;
    return ( result);
}
bool C_radar_data::getIsVtorih() const
{
    return isVtorih;
}

void C_radar_data::setIsVtorih(bool value)
{
    isVtorih = value;
}
void C_radar_data::setProcessing(bool onOff)
{
    if(onOff)
    {

        //initData(true);
        isProcessing = true;
        printf("\nSecondary processing mode - on.");
    }
    else
    {
        isProcessing = false;
        printf("\nSecondary processing mode - off.");
    }
}

bool C_radar_data::checkFeedback(unsigned char *command)
{
    for (short i=0;i<8;i++)
    {if(command[i]!=command_feedback[i])return false;}
    memset(&command_feedback[0],0,8);
    return true;
}


void C_radar_data::drawSgn(short azi_draw, short r_pos)
{
    unsigned char value = data_mem.display_ray[r_pos][0];
    unsigned char dopler    = data_mem.display_ray[r_pos][1];
    unsigned char sled     = data_mem.display_ray[r_pos][2];
    short px = data_mem.xkm[azi_draw][r_pos];
    short py = data_mem.ykm[azi_draw][r_pos];
    if(px<=0||py<=0)return;
    short pSize = 1;
    if(r_pos<200)pSize=0;
    else if(r_pos>800)pSize=2;
    if((px<pSize)||(py<pSize)||(px>=img_ppi->width()-pSize)||(py>=img_ppi->height()-pSize))return;
    for(short x = -pSize;x <= pSize;x++)
    {
        for(short y = -pSize;y <= pSize;y++)
        {
            double k =1.0/((x*x+y*y)/3.0+1.0);
            unsigned char pvalue = value*k;
            if( data_mem.display_mask[px+x][py+y] <= pvalue)
            {
                data_mem.display_mask[px+x][py+y] = pvalue;
                img_ppi->setPixel(px+x,py+y,getColor(pvalue,dopler,sled));//todo: set color table

            }
        }
    }

}

void C_radar_data::drawBlackAzi(short azi_draw)
{
    for (short r_pos = 1;r_pos < RAD_DISPLAY_RES;r_pos++)
    {

        short px = data_mem.xkm[azi_draw][r_pos];
        short py = data_mem.ykm[azi_draw][r_pos];
        if(px<=0||py<=0)continue;
        short pSize = 1;
        if(r_pos<200)pSize=0;
        else if(r_pos>800)pSize=2;

        if((px<pSize)||(py<pSize)||(px>=img_ppi->width()-pSize)||(py>=img_ppi->height()-pSize))continue;

        for(short x = -pSize;x <= pSize;x++)
        {
            for(short y = -pSize;y <= pSize;y++)
            {
                //                if(r_pos<100)data_mem.display_mask[px+x][py+y] *=0.9;
                //                else
                data_mem.display_mask[px+x][py+y] =0;
            }
        }
    }
    for (short r_pos = 1;r_pos<DISPLAY_RES_ZOOM;r_pos++)
    {

        short px = data_mem.xzoom[azi_draw][r_pos];
        short py = data_mem.yzoom[azi_draw][r_pos];
        if(px<=0||py<=0)continue;
        short pSize = 1;
        if(r_pos<200)pSize=0;
        else if(r_pos>800)pSize=2;
        if((px<pSize)||(py<pSize)||(px>=img_zoom_ppi->width()-pSize)||(py>=img_zoom_ppi->height()-pSize))continue;

        for(short x = -pSize;x <= pSize;x++)
        {
            for(short y = -pSize;y <= pSize;y++)
            {

                data_mem.display_mask_zoom[px+x][py+y] = 0;
            }
        }
    }
}
void C_radar_data::drawAzi(short azi)
{

    int leftAzi = azi-1;if(leftAzi<0)leftAzi+=MAX_AZIR;
    int rightAzi = azi +1; if(rightAzi>=MAX_AZIR)rightAzi-=MAX_AZIR;
    if(lastProcessAzi==leftAzi)
    {
        if(rotDir>-3)
        {
            rotDir--;
            if(rotDir==-1)init_time+=2;
        }
    }
    else if(lastProcessAzi==rightAzi)
    {
        if(rotDir<2)
        {
            rotDir++;
            if(rotDir==0)init_time+=2;
        }
    }


    if(rotDir<0)
    {
        //reset the display masks
        short prev_azi = azi - 20;
        if(prev_azi<0)prev_azi += MAX_AZIR;
        drawBlackAzi(prev_azi*3+2);
        drawBlackAzi(prev_azi*3+1);
        drawBlackAzi(prev_azi*3);
        //reset the drawing ray
        memset(&data_mem.display_ray[0][0],0,RAD_DISPLAY_RES*3);
        //memset(&signal_map.display_zoom[0][0],0,DISPLAY_RES_ZOOM*3);
        //set data to the drawing ray

    }
    else
    {
        //reset the display masks
        short prev_azi = azi + 20;
        if(prev_azi>=MAX_AZIR)prev_azi -= MAX_AZIR;
        drawBlackAzi(prev_azi*3);
        drawBlackAzi(prev_azi*3+1);
        drawBlackAzi(prev_azi*3+2);
        //reset the drawing ray
        memset(&data_mem.display_ray[0][0],0,RAD_DISPLAY_RES*3);

    }


    unsigned short  lastDisplayPos =0;
    for (short r_pos = 0;r_pos<range_max-1;r_pos++)
    {
        unsigned short value = data_mem.level_disp[azi][r_pos];
        unsigned short dopler = data_mem.dopler[azi][r_pos];
        if(DrawZoomAR(azi,r_pos,
                      value,
                      dopler,
                      data_mem.sled[azi][r_pos]))
        {
            value+=30;
            if(value>255)value=255;
        }
        //zoom to view scale
        short display_pos = r_pos*scale_ppi;
        short display_pos_next = (r_pos+1)*scale_ppi;
        for(;;)
        {
            if(display_pos>=RAD_DISPLAY_RES)break;
            if(data_mem.display_ray[display_pos][0]<value)
            {
                data_mem.display_ray[display_pos][0] = value;
                data_mem.display_ray[display_pos][1] = dopler;

            }
            if(data_mem.display_ray[display_pos][2] < data_mem.sled[azi][r_pos])
            {
                data_mem.display_ray[display_pos][2] = data_mem.sled[azi][r_pos];
            }
            display_pos++;
            if(display_pos>=display_pos_next)break;
        }
        if(lastDisplayPos<display_pos_next)lastDisplayPos = display_pos_next;
        //zoom to zoom scale !
        short display_pos_zoom = r_pos*scale_zoom_ppi;
        short display_pos_next_zoom  = (r_pos+1)*scale_zoom_ppi;
        for(;;)
        {
            if(display_pos_zoom>=DISPLAY_RES_ZOOM)break;
            if(true)
            {
                data_mem.display_ray_zoom[display_pos_zoom][0] += (value-data_mem.display_ray_zoom[display_pos_zoom][0])/1.4;
                data_mem.display_ray_zoom[display_pos_zoom][1] = dopler;

            }
            if(true)//signal_map.display_zoom[display_pos_zoom][2] < signal_map.sled[azi][r_pos])
            {
                data_mem.display_ray_zoom[display_pos_zoom][2] = data_mem.sled[azi][r_pos];
            }
            display_pos_zoom++;
            if(display_pos_zoom>=display_pos_next_zoom)break;
        }

    }
    if (lastDisplayPos<RAD_DISPLAY_RES)
    {
        for(;lastDisplayPos<RAD_DISPLAY_RES;lastDisplayPos++)
        {

            data_mem.display_ray[lastDisplayPos][0] = 0;
            data_mem.display_ray[lastDisplayPos][1] = 0;
            data_mem.display_ray[lastDisplayPos][2] = 0;
        }
    }
    //smooothing the image
    float k  = scale_ppi/2;
    //printf("\nviewScale:%f",viewScale);
    for(short display_pos = 1;display_pos<DISPLAY_RES_ZOOM; display_pos++)
    {
        data_mem.display_ray_zoom[display_pos][0] = data_mem.display_ray_zoom[display_pos-1][0]
                + ((float)data_mem.display_ray_zoom[display_pos][0]
                -(float)data_mem.display_ray_zoom[display_pos-1][0])/2;
        //signal_map.display_zoom[display_pos][1] = signal_map.display_zoom[display_pos-1][1] + ((float)signal_map.display_zoom[display_pos][1]-(float)signal_map.display_zoom[display_pos-1][1])/3;
        drawSgnZoom(azi*3,display_pos);
        drawSgnZoom(azi*3+1,display_pos);
        drawSgnZoom(azi*3+2,display_pos);
    }


    if(k<=2)
    {
        for(short display_pos = 1;display_pos<RAD_DISPLAY_RES;display_pos++)
        {
            drawSgn(azi*3,display_pos);
            drawSgn(azi*3+1,display_pos);
            drawSgn(azi*3+2,display_pos);
        }

    }
    else
    {
        for(short display_pos = 1;display_pos<RAD_DISPLAY_RES;display_pos++)
        {
            data_mem.display_ray[display_pos][0] = data_mem.display_ray[display_pos-1][0] + ((float)data_mem.display_ray[display_pos][0]-(float)data_mem.display_ray[display_pos-1][0])/k;
            //signal_map.display[display_pos][1] = signal_map.display[display_pos-1][1] + ((float)signal_map.display[display_pos][1]-(float)signal_map.display[display_pos-1][1])/k;
            drawSgn(azi*3,display_pos);
            drawSgn(azi*3+1,display_pos);
            drawSgn(azi*3+2,display_pos);

        }

    }
    //drawingDone = true;

}

void  C_radar_data::getNoiseLevel()
{


    if(nNoiseCalculator<50)return;

    short histogram_max_val=1;
    short histogram_max_pos=0;
    noiseVar+=(sumvar/float(nNoiseCalculator)-noiseVar)/2.0f;
    if(noiseVar<7)noiseVar = 7;
    for(short i = 0;i<256;i++)
    {
        if(histogram[i]>histogram_max_val)
        {
            histogram_max_val = histogram[i];
            histogram_max_pos = i;
        }

    }

    if(noiseAverage)
    {
        noiseAverage += (histogram_max_pos-noiseAverage)/3.0f;
    }
    else
    {
        noiseAverage = 31;//histogram_max_pos;
    }
    img_histogram->fill(0);
    for(short i = 0;i<256;i++)
    {
        histogram[i] = histogram[i]*100/histogram_max_val;
        img_histogram->setPixel(i,100-histogram[i],1);
    }
    short thresh = noiseAverage+(short)noiseVar*kgain;
    if(thresh<0)thresh=0;
    for(short j = 99;j>100-histogram[histogram_max_pos];j--)
    {
        img_histogram->setPixel(histogram_max_pos,j,1);
        if(j>50)img_histogram->setPixel(thresh,j,1);
    }
    //printf("\ntrung binh tap:%f, var:%f",noiseAverage,noiseVar);
    nNoiseCalculator=0;
    memset(histogram,0,256);
    sumvar = 0;

}
/*void C_radar_data::SetHeaderLen( short len)
{
    headerLen = len;
}*/
//bool C_radar_data::getDoubleFilter() const
//{
//    return doubleFilter;
//}

//void C_radar_data::setDoubleFilter(bool value)
//{
//    doubleFilter = value;
//}
/*void C_radar_data::decodeData(int azi)
{
    //read spectre
    memcpy((char*)&spectre,(char*)&dataBuff[RADAR_DATA_HEADER_MAX],16);
    img_spectre->fill(0);
    //draw spectre

    for(short i=0;i<16;i++)
    {
        for(short j=255;j>255-spectre[i];j--)
        {
            img_spectre->setPixel(i,j,1);
        }
    }
    // sharpyeye dopler procession

    short i_m  = headerLen;
    short i_s  = i_m + range_max;
    short i_md = i_s + RAD_S_PULSE_RES;
    short i_sd = i_md+ range_max/2;
    for(short r_pos = 0;r_pos<range_max;r_pos++)
    {
        //data_mem.dopler_old[azi][r_pos] = data_mem.dopler[azi][r_pos];
        if(r_pos<RAD_S_PULSE_RES)
        {
            switch (dataOver) {
            case m_only:
                data_mem.level[azi][r_pos] = dataBuff[i_m+r_pos];
                data_mem.dopler[azi][r_pos] = dataBuff[i_md+r_pos/2];
                break;
            case s_m_200:
                if(r_pos<200)
                {
                    data_mem.level[azi][r_pos] = dataBuff[i_s+r_pos];
                    data_mem.dopler[azi][r_pos] = dataBuff[i_sd+r_pos/2];
                }
                else
                {
                    data_mem.level[azi][r_pos] = dataBuff[i_m+r_pos];
                    data_mem.dopler[azi][r_pos] = dataBuff[i_md+r_pos/2];
                }
                break;
            case max_s_m_200:
                if(r_pos<200&&(dataBuff[i_s+r_pos]>dataBuff[i_m+r_pos]))
                {
                    data_mem.level[azi][r_pos]  = dataBuff[i_s  + r_pos];
                    data_mem.dopler[azi][r_pos] = dataBuff[i_sd + r_pos/2];
                }
                else
                {
                    data_mem.level[azi][r_pos] = dataBuff[i_m+r_pos];
                    data_mem.dopler[azi][r_pos] = dataBuff[i_md+r_pos/2];
                }
                break;
            default:
                break;

            }
        }
        else
        {
            data_mem.level[azi][r_pos] = dataBuff[i_m+r_pos];
            data_mem.dopler[azi][r_pos] = dataBuff[i_md+r_pos/2];
        }
        //unzip the dopler data
        if(!(r_pos&0x01))
        {

            data_mem.dopler[azi][r_pos] = 0x0f&data_mem.dopler[azi][r_pos];

        }
        else
        {
            data_mem.dopler[azi][r_pos] = data_mem.dopler[azi][r_pos]>>4;
        }
    }
}*/
short threshRay[RADAR_RESOLUTION];
#define MAX_RAIN  180//noiseAverage+noiseVar*15;
void C_radar_data::ProcessData(unsigned short azi,unsigned short lastAzi)
{

    rainLevel = noiseAverage;

    for(short r_pos=0;r_pos<range_max;r_pos++)
    {
        // RGS threshold
        short displayVal;
        rainLevel += krain_auto*(data_mem.level[azi][r_pos]-rainLevel);
        if(rainLevel>MAX_RAIN)rainLevel = MAX_RAIN;
        short nthresh = rainLevel + noiseVar*kgain_auto;
        threshRay[r_pos] += (nthresh-threshRay[r_pos])*0.5;
        bool underThreshold = data_mem.level[azi][r_pos]<threshRay[r_pos];
        if(data_mem.dopler[azi][r_pos]!=data_mem.dopler[lastAzi][r_pos])underThreshold = true;
        data_mem.detect[azi][r_pos] = (!underThreshold);
        if(!underThreshold)if(!init_time)if(r_pos>RANGE_MIN)procPix(azi,r_pos);

        // display value
        if(!isManualTune)
        {

            if(noise_nornalize&&(!cut_noise))
            {
                short dif = (data_mem.level[azi][r_pos]+32+ noiseVar*kgain_auto -threshRay[r_pos]);
                if(dif<0)dif=0;
                else if(dif>255)dif=255;
                displayVal=dif;
            }
            else
                displayVal=data_mem.level[azi][r_pos];
            if(!underThreshold)
            {
                if(!init_time)data_mem.sled[azi][r_pos] = mSledValue;
            }
            else
            {
                data_mem.sled[azi][r_pos] -= (data_mem.sled[azi][r_pos])/20.0f;
                if(cut_noise)displayVal= 0;
            }
            if(data_mem.may_hoi[azi][r_pos])displayVal+=80;
            if(displayVal>255)displayVal=255;
            data_mem.level_disp[azi][r_pos]=displayVal;
        }


    }
    // display value if isManualTune
    if(isManualTune)
    {
        for(short r_pos=0;r_pos<range_max;r_pos++)
        {
            // RGS threshold
            short displayVal;
            rainLevel += krain*(data_mem.level[azi][r_pos]-rainLevel);
            if(rainLevel>MAX_RAIN)rainLevel = MAX_RAIN;
            int nthresh = int(rainLevel + noiseVar*kgain+pow(2.7, int(ksea*4.0/(r_pos*sn_scale))));

            //if(data_mem.dopler[azi][r_pos]!=data_mem.dopler[lastAzi][r_pos])cutoff = true;
            if(noise_nornalize)
            {
                short dif = (data_mem.level[azi][r_pos]+32+ noiseVar*kgain - nthresh);
                if(dif<0)dif=0;
                else if(dif>255)dif=255;
                displayVal=dif;
            }
            else displayVal=data_mem.level[azi][r_pos];
            if(data_mem.level[azi][r_pos]<nthresh)displayVal=0;
            if(data_mem.may_hoi[azi][r_pos])displayVal+=80;
            /*
            data_mem.detect[azi][r_pos] = (!cutoff);
            //data_mem.detect[azi][r_pos] = (!cutoff);
            //detect = vuot nguong & dopler lap lai

            if(data_mem.detect[azi][r_pos])
            {
                if(!init_time)
                {data_mem.sled[azi][r_pos] = mSledValue;
                    if(r_pos>RANGE_MIN)procPix(azi,r_pos);}
            }
            else
            {
                data_mem.sled[azi][r_pos] -= (data_mem.sled[azi][r_pos])/20.0f;
                if(cut_noise)displayVal= 0;
            }*/
            if(displayVal>255)displayVal=255;
            data_mem.level_disp[azi][r_pos]=displayVal;

        }
    }
    return ;
}
void C_radar_data::ProcessEach90Deg()
{
    //DetectTracks();
    getNoiseLevel();
    //remove old points
    int nObj = 0;
    for (uint i=0;i<mFreeObjList.size();i++)
    {

        if(!mFreeObjList.at(i).isRemoved)
        {
            if((now_ms-mFreeObjList.at(i).timeMs)>80000)
                mFreeObjList.at(i).isRemoved = true;
            else nObj++;
        }

    }
    //
    if(nObj>500)
    {
        if(kgain_auto<7.5)kgain_auto*=1.05;
        printf("\ntoo many obj,kgain_auto:%f",kgain_auto);
    }
    else if(nObj<20)
    {if(kgain_auto>4.2)kgain_auto/=1.05;}
    if(mFalsePositiveCount>100)//ENVAR
    {
        if(kgain_auto<10)kgain_auto*=1.05;
        printf("\ntoo many false positive kgain_auto:%f",kgain_auto);
    }
    mFalsePositiveCount = 0;

    //calculate rotation speed
    if(cur_timeMSecs)
    {
        qint64 newtime = now_ms;
        qint64 dtime = newtime - cur_timeMSecs;
        if(dtime<50000&&dtime>0)
        {
            rot_period_sec = (dtime/1000.0);
            rotation_per_min += (15.0/rot_period_sec-rotation_per_min)/2.0;
            cur_timeMSecs = newtime;
            if(isSelfRotation)
            {
                double rateError = rotation_per_min/selfRotationRate;
                selfRotationDazi/=rateError;
            }
        }
    }
    else
    {
        cur_timeMSecs = now_ms;
    }


}
int gray_decode(ushort n) {
    int p = n;
    while (n >>= 1) p ^= n;
    return p;
}
unsigned char inverseBit =0;
bool isInverseSSI = false;
//short dirSSI ;
int C_radar_data::ssiDecode(ushort nAzi)
{
    inverseBit<<=1;
    inverseBit&=0x03;
    nAzi = gray_decode(nAzi);//&0x1fff
    inverseBit |= (nAzi&0x01);
    if(inverseBit==3)isInverseSSI = false;
    if(inverseBit==0)isInverseSSI = true;
    nAzi&=0x1fff;

    nAzi>>=1;
    if(isInverseSSI)nAzi = 4096-nAzi;
    mEncoderVal = nAzi;
    return nAzi>>1;
}
void C_radar_data::ProcessGOData(unsigned char* data,short len, int azi)
{
    if(len<300)return;
    for(int i=0;i<range_max;i++)
    {
        data_mem.may_hoi[azi][i] = (data[i/8+RADAR_HEADER_LEN]>>(i%8))&0x01;
    }
}
void C_radar_data::processSocketData(unsigned char* data,short len)
{

    if(len==MAX_FRAME_SIZE)
        range_max = RADAR_RESOLUTION;
    else if(len==MAX_FRAME_SIZE_HALF)
        range_max = RADAR_RESOLUTION_HALF;

    memcpy(mHeader,data,FRAME_HEADER_SIZE);
    unsigned char n_clk_adc = data[4];
    sn_stat = (data[5]<<8)+data[6];
    if(clk_adc != n_clk_adc)
    {
        // clock adc
        clk_adc = n_clk_adc;
        isClkAdcChanged = true;
        UpdateData();
        resetData();
    }
    uint newAzi =0;
    if(giaQuayPhanCung)
    {
        newAzi = ((data[11]<<8)|data[12])>>5;
    }
    else if(isSelfRotation)
    {
        selfRotationAzi+=selfRotationDazi;
        if(selfRotationAzi>=MAX_AZIR)
        {
            selfRotationAzi -= MAX_AZIR;
            //ProcessRound();
        }
        if(selfRotationAzi<0)
        {
            selfRotationAzi += MAX_AZIR;
            //ProcessRound();
        }
        newAzi = selfRotationAzi;
    }
    else
    {
        if(data[0]!=0x55)// tao gia bang simulator
        {
            if(isTrueHeading)
            {
                newAzi = (data[9]<<24)|(data[10]<<16)|(data[11]<<8)|(data[12]);
                newAzi>>=3;
                newAzi&=    0xffff;
                int heading = ((data[15]<<8)|data[16])>>5;
                CConfig::shipHeadingDeg = heading/double(MAX_AZIR)*180.0;
                newAzi = ssiDecode(newAzi);
                newAzi += heading;
                if(newAzi>=MAX_AZIR)newAzi-=MAX_AZIR;
            }
            else
            {
                int heading = (CConfig::shipHeadingDeg)/360.0*MAX_AZIR;
                newAzi = (data[9]<<24)|(data[10]<<16)|(data[11]<<8)|(data[12]);
                newAzi>>=3;
                newAzi&=0xffff;
                newAzi = ssiDecode(newAzi);
                newAzi+= heading;
                if(newAzi>=MAX_AZIR)newAzi-=MAX_AZIR;
            }

        }
        else
        {
            newAzi = (data[2]<<8)|data[3];

        }

    }
    newAzi&=0x07ff;
    if(data[0]==4)// du lieu may hoi
    {
        ProcessGOData(data, len,newAzi);
        return;
    }
    if(curAzir==newAzi)return;
    //if(newAzi==0)dir= !dir;
    int dazi = newAzi-curAzir;
    if(abs(dazi)>10&&((MAX_AZIR -abs(dazi))>10))
    {
        init_time+=2;
        curAzir=newAzi;
        memcpy(&data_mem.level[curAzir][0],data+FRAME_HEADER_SIZE,range_max);
        memcpy(&data_mem.dopler[curAzir][0],data+FRAME_HEADER_SIZE+range_max,range_max);
        aziToProcess.push(curAzir);
    }
    else if(dazi<0&&(-dazi)<MAX_AZIR/2)//quay nguoc
    {
        while(curAzir != newAzi)
        {
            curAzir--;
            if(curAzir<0)curAzir+=MAX_AZIR;
            memcpy(&data_mem.level[curAzir][0],data+FRAME_HEADER_SIZE,range_max);
            memcpy(&data_mem.dopler[curAzir][0],data+FRAME_HEADER_SIZE+range_max,range_max);
            aziToProcess.push(curAzir);
        }
    }
    else if(dazi>0&&(dazi)<MAX_AZIR/2)//quay thuan
    {

        while(curAzir != newAzi)
        {

            curAzir++;
            if(curAzir>=MAX_AZIR)curAzir-=MAX_AZIR;
            memcpy(&data_mem.level[curAzir][0],data+FRAME_HEADER_SIZE,range_max);
            memcpy(&data_mem.dopler[curAzir][0],data+FRAME_HEADER_SIZE+range_max,range_max);
            aziToProcess.push(curAzir);
        }
    }
    else if(dazi<0&&(-dazi)>MAX_AZIR/2)//quay thuan qua diem 0
    {
        while(curAzir != newAzi)
        {

            curAzir++;
            if(curAzir>=MAX_AZIR)curAzir-=MAX_AZIR;
            memcpy(&data_mem.level[curAzir][0],data+FRAME_HEADER_SIZE,range_max);
            memcpy(&data_mem.dopler[curAzir][0],data+FRAME_HEADER_SIZE+range_max,range_max);
            aziToProcess.push(curAzir);
        }
    }
    else if(dazi>0&&(dazi)>MAX_AZIR/2)//quay nguoc qua diem 0
    {
        while(curAzir != newAzi)
        {

            curAzir--;
            if(curAzir<0)curAzir+=MAX_AZIR;
            memcpy(&data_mem.level[curAzir][0],data+FRAME_HEADER_SIZE,range_max);
            memcpy(&data_mem.dopler[curAzir][0],data+FRAME_HEADER_SIZE+range_max,range_max);
            aziToProcess.push(curAzir);
        }
    }

    isDrawn = false;
    return;
}
void C_radar_data::SelfRotationOn( double rate)
{
    isSelfRotation = true;
    printf("\nself rotation");
    SelfRotationReset();
    selfRotationDazi = 0.2;
    cur_timeMSecs =0;
    selfRotationRate = rate;
    if(selfRotationRate<1)selfRotationRate=1;
    //ProcessEach90Deg();
}
void C_radar_data::SelfRotationReset()
{
    //selfRotationAzi = 0;

    selfRotationAzi = 0;
}
void C_radar_data::SelfRotationOff()
{
    isSelfRotation = false;
}

//int C_radar_data::getNewAzi()
//{
//    int newAzi;
//    if(isSelfRotation)
//    {
//        selfRotationAzi-=selfRotationDazi;
//        if(selfRotationAzi>=MAX_AZIR)selfRotationAzi = 0;
//        if(selfRotationAzi<0)selfRotationAzi += MAX_AZIR;
//        newAzi = selfRotationAzi;
//    }
//    else
//    {
//        newAzi = (0xfff & (dataBuff[4] << 8 | dataBuff[5]))>>1;
//    }
//    if(newAzi>MAX_AZIR||newAzi<0)
//        return 0;
//    return newAzi;
//}
void C_radar_data::ProcessDataFrame()
{/*
    int newAzi = getNewAzi();

    int leftAzi = curAzir-1;if(leftAzi<0)leftAzi+=MAX_AZIR;
    int rightAzi = curAzir +1; if(rightAzi>=MAX_AZIR)rightAzi-=MAX_AZIR;
    if(newAzi == leftAzi )
    {
        if(rotDir==Right)
        {
            rotDir  = Left;
            arcMaxAzi = curAzir;
            init_time =5;

        }
    }
    else if(newAzi == rightAzi) {
        if(rotDir==Left)
        {
            rotDir = Right;
            arcMinAzi = curAzir;
            init_time =5;
        }
    }
    else if(newAzi ==curAzir)
    {
        //printf("\ncurAzir:%d",curAzir);
        return;
    }
    else
    {
        //clearPPI();
    }
    rotation_speed = dataBuff[1];
    overload = dataBuff[4]>>7;
    unsigned char n_clk_adc = (dataBuff[4]&(0xe0))>>5;
    if(clk_adc != n_clk_adc)
    {
        // clock adc

        clk_adc = n_clk_adc;
        isClkAdcChanged = true;
        resetData();

    }
    moduleVal = dataBuff[3];//
    tempType = dataBuff[2]&0x0f;
    if(tempType>4)printf("Wrong temperature\n");
    sn_stat = dataBuff[14]<<8|dataBuff[15];
    chu_ky = dataBuff[16]<<8|dataBuff[17];
    tb_tap[newAzi] = dataBuff[18]<<8|dataBuff[19];
    memcpy(command_feedback,&dataBuff[RADAR_COMMAND_FEEDBACK],8);
    memcpy(noise_level,&dataBuff[RADAR_COMMAND_FEEDBACK+8],8);
    curAzir = newAzi;
    aziToProcess.push(curAzir);
    decodeData(curAzir);
    if(!((unsigned char)(curAzir<<3))){
        procTracks(curAzir);
        getNoiseLevel();

    }
    if(curAzir==0)
    {
        ProcessEach90Deg();

    }*/
}
void C_radar_data::clearPPI()
{
    img_ppi->fill(0);

}



#define POLY_DEG 2
void C_radar_data::LeastSquareFit(C_primary_track* track)
{
    /*
    uint  nElement = 6;
    int lastPost = track->objectList.size()-1;
    double *x = new double[nElement];
    double *y = new double[nElement];
    int i,j,k;
    for(i =0;i<nElement;i++)
    {
        x[i] = track->objectList[lastPost-i].timeMs;
        y[i] = track->objectList[lastPost-i].azRad;
    }
    // POLY_DEG is the degree of Polynomial
    int n = POLY_DEG,N=nElement;
    double *X = new double[2*n+1];                        //Array that will store the values of sigma(xi),sigma(xi^2),sigma(xi^3)....sigma(xi^2n)
        for (i=0;i<2*n+1;i++)
        {
            X[i]=0;
            for (j=0;j<N;j++)
                X[i]=X[i]+pow(x[j],i);        //consecutive positions of the array will store N,sigma(xi),sigma(xi^2),sigma(xi^3)....sigma(xi^2n)
        }
        double B[3][4];
        double a[3];            //B is the Normal matrix(augmented) that will store the equations, 'a' is for value of the final coefficients
        for (i=0;i<=n;i++)
            for (j=0;j<=n;j++)
                B[i][j]=X[i+j];            //Build the Normal matrix by storing the corresponding coefficients at the right positions except the last column of the matrix
        double Y[3];                    //Array to store the values of sigma(yi),sigma(xi*yi),sigma(xi^2*yi)...sigma(xi^n*yi)
        for (i=0;i<n+1;i++)
        {
            Y[i]=0;
            for (j=0;j<N;j++)
            Y[i]=Y[i]+pow(x[j],i)*y[j];        //consecutive positions will store sigma(yi),sigma(xi*yi),sigma(xi^2*yi)...sigma(xi^n*yi)
        }
        for (i=0;i<=n;i++)
            B[i][n+1]=Y[i];                //load the values of Y as the last column of B(Normal Matrix but augmented)
        n=n+1;                //n is made n+1 because the Gaussian Elimination part below was for n equations, but here n is the degree of polynomial and for n degree we get n+1 equations
        for (i=0;i<n;i++)                    //From now Gaussian Elimination starts(can be ignored) to solve the set of linear equations (Pivotisation)
            for (k=i+1;k<n;k++)
                if (B[i][i]<B[k][i])
                    for (j=0;j<=n;j++)
                    {
                        double temp=B[i][j];
                        B[i][j]=B[k][j];
                        B[k][j]=temp;
                    }

        for (i=0;i<n-1;i++)            //loop to perform the gauss elimination
            for (k=i+1;k<n;k++)
                {
                    double t=B[k][i]/B[i][i];
                    for (j=0;j<=n;j++)
                        B[k][j]=B[k][j]-t*B[i][j];    //make the elements below the pivot elements equal to zero or elimnate the variables
                }
        for (i=n-1;i>=0;i--)                //back-substitution
        {                        //x is an array whose values correspond to the values of x,y,z..
            a[i]=B[i][n];                //make the variable to be calculated equal to the rhs of the last equation
            for (j=0;j<n;j++)
                if (j!=i)            //then subtract all the lhs values except the coefficient of the variable whose value                                   is being calculated
                    a[i]=a[i]-B[i][j]*a[j];
            a[i]=a[i]/B[i][i];            //now finally divide the rhs by the coefficient of the variable to be calculated
        }


    for ( i=0;i<nElement;i++)            //loop over data points to perform the estimation
    {
        y[i]=0;
        for ( j=0;j<n;j++)            //loop over all degree of polynom
        {
            y[i]+=a[j]*pow(x[i],j);
        }
    }

    for ( i =0;i<nElement;i++)
    {
        //track->objectList[lastPost-i].azRad = x[i];
        track->objectList[lastPost-i].azRad = y[i] ;
        track->objectList[lastPost-i].xkmfit = track->objectList[lastPost-i].rgKm*sin( track->objectList[lastPost-i].azRad);
        track->objectList[lastPost-i].ykmfit = track->objectList[lastPost-i].rgKm*cos( track->objectList[lastPost-i].azRad);
    }
    delete[] x;
    delete[] y;
    delete[] X;
//    delete[] B;
//    delete[] a;
*/
}
ushort mulOf16Azi = 0;
void C_radar_data::UpdateData()
{
    while(aziToProcess.size()>3)
    {

        int azi = aziToProcess.front();
        //printf("\nAzi:%d",azi);
        if(azi==lastProcessAzi)
        {
            aziToProcess.pop();
            continue;
        }
        ProcessData(azi,lastProcessAzi);
        mPeriodCount++;
        drawAzi(azi);
        if(!((unsigned char)(((unsigned char)mPeriodCount)<<3)))//xu ly moi 32 chu ky
        {
            now_ms = (QDateTime::currentMSecsSinceEpoch()-time_start_ms);//QDateTime::currentMSecsSinceEpoch();
            //ProcessObjects();
            ProcessTracks();
            mulOf16Azi++;
            if(mulOf16Azi>16)// 1/4 vong quet
            {
                mulOf16Azi=0;
                ProcessEach90Deg();
            }
            if(init_time)init_time--;
            for(unsigned short i = 0;i<plot_list.size();++i)
            {
                if(plot_list.at(i).isUsed)
                {
                    if((plot_list.at(i).lastA!=lastProcessAzi)&&(plot_list.at(i).lastA!=azi))
                    {
                        procPLot(&plot_list.at(i));
                        plot_list.at(i).isUsed = false;
                    }

                }
            }
        }
        // update histogram
        nNoiseCalculator++;
        sumvar+= abs(data_mem.level[azi][range_max-200]-data_mem.level[azi][range_max-205]);;
        unsigned char value = data_mem.level[azi][range_max-200];
        if(value>5&&value<200)
        {
            histogram[value-3]+=1;
            histogram[value-2]+=2;
            histogram[value-1]+=3;
            histogram[value  ]+=4;
            histogram[value+1]+=3;
            histogram[value+2]+=2;
            histogram[value+3]+=1;
        }//

        lastProcessAzi = azi;
        aziToProcess.pop();
    }

}

static  unsigned int doplerHistogram[256];
void C_radar_data::procPLot(plot_t* mPlot)
{
    if(init_time)
        return;
    // remove too big or too small
    if(mPlot->sumEnergy<100)
    {
        mFalsePositiveCount++;
        return;
    }
    else if(mPlot->sumEnergy>20000)
    {
        return;
    }
    float ctA,dAz,dRg = (mPlot->maxR-mPlot->minR)*rgStdErr;
    if(dRg>200)return;
    dAz = abs(mPlot->lastA-mPlot->riseA);
    if(dAz>(MAX_AZIR/2))//quay qua diem 0
    {
        dAz = MAX_AZIR-dAz;
        ctA = (mPlot->fallA+mPlot->riseA+MAX_AZIR)/2.0;
    }
    else
    {
        ctA = (mPlot->riseA + mPlot->fallA)/2.0;
    }
    if(dAz>30)return;//ENVDEP
    if(ctA >= MAX_AZIR)ctA -= MAX_AZIR;

    //if(mPlot->minR<500)return;
    if(xl_dopler)
    {
        memset(doplerHistogram,0,256);
        int leftA = mPlot->riseA-15;
        if(leftA<0)leftA+=MAX_AZIR;
        int rightA = mPlot->riseA+15;
        if(rightA>=MAX_AZIR)rightA-=MAX_AZIR;
        int maxRg = mPlot->maxR+15;
        int minRg = mPlot->maxR-15;
        if(minRg<0)minRg=0;if(maxRg>=range_max)maxRg=range_max-1;
        for(int lAzi=leftA;;lAzi++)
        {
            if(lAzi>=MAX_AZIR)lAzi-=MAX_AZIR;
            if(lAzi==rightA)break;
            for(int lRg = minRg;lRg<maxRg;lRg++)
            {
                if(data_mem.detect[lAzi][lRg])
                    doplerHistogram[data_mem.dopler[lAzi][lRg]]++;
            }
        }
        //printf("\n");
        int sumHis = 0;
        int maxHis = 0;
        for(int i=0;i<16;i++)
        {
            //printf(" %d:%d ",i,doplerHistogram[i]);
            //sumHis+=doplerHistogram[i];
            //if(doplerHistogram[i]>20)
            sumHis+=doplerHistogram[i];
            if(doplerHistogram[i]>maxHis)maxHis = doplerHistogram[i];
            //            if((doplerHistogram[i]-mPlot->size)>50&&mPlot->dopler==i)
            //            {
            //                printf("mPlot->size: %d",mPlot->size);
            //                return;
            //            }
        }
        //printf("mPlot->size: %d",mPlot->size);
        //printf("\n maxhis/sumHis: %f",maxHis/(float)sumHis);
        if(maxHis/(float)sumHis<0.75)return;


    }

    object_t newobject;
    newobject.timeMs = now_ms;
    newobject.isRemoved = false;
    newobject.dazi = dAz;
    newobject.period = mPeriodCount;
    float ctR = (float)mPlot->sumR/(float)mPlot->size+0.5;//min value starts at 1
    if(ctR<mPlot->minR||ctR>mPlot->maxR+1)printf("\nWrong ctR");
    //todo: tinh dopler histogram


    newobject.size = mPlot->size;
    newobject.drg = mPlot->maxR-mPlot->minR;
    newobject.aziStdEr = azi_er_rad;
    newobject.rgStdEr = rgStdErr;
    if(ctA<0|| ctR>=RADAR_RESOLUTION)
    {
        return;
    }
    newobject.dopler = mPlot->dopler;
    //newobject.terrain = data_mem.terrain[short(ctA)][short(ctR)];
    newobject.azRad   = ctA/float(MAX_AZIR/PI_NHAN2)+aziOffsetRad;
    if(newobject.azRad>PI_NHAN2)newobject.azRad-=PI_NHAN2;
    newobject.rg   = ctR;
    newobject.rgKm =  ctR*sn_scale;
    //    newobject.p   = -1;
    newobject.xkm = newobject.rgKm*sin( newobject.azRad);
    newobject.ykm = newobject.rgKm*cos( newobject.azRad);
    ProcessObject(&newobject);
    mPlot->isUsed = false;
}

void C_radar_data::kmxyToPolarDeg(double x, double y, double *azi, double *range)
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



void C_radar_data::drawRamp()
{
    img_RAmp->fill(Qt::black);
    for (short r_pos = 0;r_pos<range_max;r_pos++)
    {
        unsigned char value = data_mem.level[curAzir][r_pos];
        char dopler = data_mem.dopler[curAzir][r_pos];

        uint color ;
        if(dopler==0)
        {
            color = 0xffff00;
        }else
        {
            char dDopler = dopler-1;
            if(dDopler>7)dDopler = 15-dDopler;
            color = 0x00ff00 | ((dDopler<<5));
        }
        color = color|(0xff000000);

        if(((r_pos%100)==0)||(((r_pos+1)%100)==0)||(((r_pos-1)%100)==0))
        {
            for(short i=0;i<128;i++)
            {
                img_RAmp->setPixel(r_pos,i,0xffffffff);
            }
        }
        for(short i=255;i>255 - value;i--)
        {
            img_RAmp->setPixel(r_pos,i,color);
        }
    }

}

void C_radar_data::drawRamp(double azi)
{
    img_RAmp->fill(Qt::black);
    //newobject.az   = ctA/MAX_AZIR*PI_NHAN2+trueN;
    azi/=DEG_RAD;
    azi-=aziOffsetRad;
    if(azi<0)azi+=PI_NHAN2;
    int az = azi/PI_NHAN2*MAX_AZIR;
    for (short r_pos = 0;r_pos<range_max;r_pos++)
    {
        unsigned char value = data_mem.level[az][r_pos];
        char dopler = data_mem.dopler[az][r_pos];

        uint color ;
        if(dopler==0)
        {
            color = 0xffff00;
        }else
        {
            char dDopler = dopler-1;
            if(dDopler>7)dDopler = 15-dDopler;
            color = 0x00ff00 | ((dDopler<<5));
        }
        color = color|(0xff000000);

        if(((r_pos%100)==0)||(((r_pos+1)%100)==0)||(((r_pos-1)%100)==0))
        {
            for(short i=0;i<128;i++)
            {
                img_RAmp->setPixel(r_pos,i,0xffffffff);
            }
        }
        for(short i=255;i>255 - value;i--)
        {
            img_RAmp->setPixel(r_pos,i,color);
        }

    }

}
/*bool C_radar_data::procObjectAvto(object_t* pObject)
{
    bool newtrack = true;
    short trackId = -1;
    short max_length = 0;
    for(unsigned short i=0;i<mTrackList.size();i++)
    {
        if(mTrackList.at(i).isManual)continue;
        if(mTrackList.at(i).state&&(! mTrackList.at(i).isProcessed))
        {
            if(mTrackList.at(i).checkProb(pObject)){
                if(max_length<mTrackList.at(i).object_list.size())
                {
                    max_length=mTrackList.at(i).object_list.size();
                    trackId = i;
                    newtrack = false;
                }
            }
        }
    }
    if(!newtrack)
    {
        //add object to a processing track
        mTrackList.at(trackId).suspect_list.push_back(*pObject);
        return true;
    }
    else
    {

        return false;
    }


    return false;
}*/
/*bool C_radar_data::procObjectManual(object_t* pObject)// !!!
{

    short trackId = -1;
    ushort max_length = 0;
    for(unsigned short i=0;i<mTrackList.size();i++)
    {
        if(!mTrackList.at(i).isManual)continue;
        if(mTrackList.at(i).state&&(! mTrackList.at(i).isProcessed))
        {
            if(mTrackList.at(i).checkProb(pObject)){
                if(max_length<mTrackList.at(i).object_list.size())
                {
                    max_length = mTrackList.at(i).object_list.size();
                    trackId = i;
                }
            }
        }
    }
    if(trackId>=0)
    {
        //add object to a processing track
        mTrackList.at(trackId).suspect_list.push_back(*pObject);
        return true;
    }
    else return false;

    return false;
}*/
void C_radar_data::procPix(short proc_azi,short range)//_______signal detected, check 4 last neighbour points for nearby mark_______________//
{
    short pr_proc_azi = lastProcessAzi;
    short plotIndex =-1;
    char dopler_0 = data_mem.dopler[proc_azi][range];
    char dopler_1 = dopler_0 +1;
    if(dopler_1>15)dopler_1-=16;
    char dopler_2 = dopler_0 - 1;
    if(dopler_2<0)dopler_2+=16;
    if(data_mem.detect[pr_proc_azi][range]
            &&(data_mem.dopler[pr_proc_azi][range]==dopler_0
               ||data_mem.dopler[pr_proc_azi][range]==dopler_1
               ||data_mem.dopler[pr_proc_azi][range]==dopler_2)
            )
    {
        plotIndex = data_mem.plotIndex[pr_proc_azi][range];

    }else if(data_mem.detect[proc_azi][range-1]
             &&(data_mem.dopler[proc_azi][range-1]==dopler_0
                ||data_mem.dopler[proc_azi][range-1]==dopler_1
                ||data_mem.dopler[proc_azi][range-1]==dopler_2)
             )
    {
        plotIndex = data_mem.plotIndex[proc_azi][range-1];
    }
    else if(data_mem.detect[pr_proc_azi][range-1]
            &&(data_mem.dopler[pr_proc_azi][range-1]==dopler_0
               ||data_mem.dopler[pr_proc_azi][range-1]==dopler_1
               ||data_mem.dopler[pr_proc_azi][range-1]==dopler_2)
            )
    {
        plotIndex = data_mem.plotIndex[pr_proc_azi][range-1];

    }
    else if(data_mem.detect[pr_proc_azi][range+1]
            &&(data_mem.dopler[pr_proc_azi][range+1]==dopler_0
               ||data_mem.dopler[pr_proc_azi][range+1]==dopler_1
               ||data_mem.dopler[pr_proc_azi][range+1]==dopler_2)
            )
    {
        plotIndex = data_mem.plotIndex[pr_proc_azi][range+1];
    }
    if((plotIndex<plot_list.size())
            &&(plotIndex>=0)
            &&(plot_list.at(plotIndex).isUsed)
            )// add to existing plot
    {
        /*if(((plot_list.at(plotIndex).maxLevel*0.8)>data_mem.level[proc_azi][range]))//cut fall of target plot
        {
            data_mem.detect[proc_azi][range] = 0;
            return;
        }*/
        data_mem.plotIndex[proc_azi][range] = plotIndex;
        plot_list.at(plotIndex).size++;
        plot_list.at(plotIndex).sumEnergy+=data_mem.level[proc_azi][range];
        if(plot_list.at(plotIndex).maxR<range)plot_list.at(plotIndex).maxR= range;
        if(plot_list.at(plotIndex).minR>range)plot_list.at(plotIndex).minR= range;
        plot_list.at(plotIndex).sumR    +=  range;
        plot_list.at(plotIndex).lastA   = proc_azi;
        //
        // get max dopler and max level of this plot
        if(plot_list.at(plotIndex).maxLevel<data_mem.level[proc_azi][range])
        {
            plot_list.at(plotIndex).riseA = proc_azi;
            plot_list.at(plotIndex).maxLevel = data_mem.level[proc_azi][range];
            plot_list.at(plotIndex).dopler = data_mem.dopler[proc_azi][range];
        }
        if((plot_list.at(plotIndex).maxLevel-noiseVar)<data_mem.level[proc_azi][range])
            plot_list.at(plotIndex).fallA = proc_azi;
    }
    else//_________new plot_____________//
    {

        plot_t                  new_plot;
        new_plot.isUsed = true;
        new_plot.lastA =        new_plot.riseA  = new_plot.fallA= proc_azi;
        new_plot.maxLevel =     data_mem.level[proc_azi][range];
        new_plot.sumEnergy =    data_mem.level[proc_azi][range];
        new_plot.dopler =       data_mem.dopler[proc_azi][range];
        new_plot.size =         1;
        new_plot.sumR =         range;
        new_plot.maxR =         range;
        new_plot.minR =         range;
        bool listFull = true;

        for(unsigned short i = 0;i<plot_list.size();++i)
        {
            //  overwrite
            if(!plot_list.at(i).isUsed)
            {
                data_mem.plotIndex[proc_azi][range] =  i;

                plot_list.at(i) = new_plot;
                listFull = false;
                break;
            }
        }
        if(listFull)
        {
            plot_list.push_back(new_plot);
            data_mem.plotIndex[proc_azi][range]  = plot_list.size()-1;
        }

    }

}
/*void C_radar_data::polarToSnXY(short *xsn, short *ysn, short azi, short range)
{
    *xsn = signal_map.frame[azi].raw_map[range].x;
    *ysn = signal_map.frame[azi].raw_map[range].y;
}
//static short ctX=0,ctY=0;
//static float dr = 0;
*/
void C_radar_data::polarToXY(float *x, float *y, float azi, float range)
{

    *x = ((sinf(azi)))*range;
    *y = ((cosf(azi)))*range;
}

float C_radar_data::getNoiseAverage() const
{
    return noiseAverage;
}

void C_radar_data::setNoiseAverage(float value)
{
    noiseAverage = value;
}

//bool C_radar_data::getIsSharpEye() const
//{
//    return isSharpEye;
//}

//void C_radar_data::setIsSharpEye(bool value)
//{
//    isSharpEye = value;
//}
short zoomXmax,zoomYmax,zoomXmin,zoomYmin;
short zoomCenterX=RAD_DISPLAY_RES,zoomCenterY=RAD_DISPLAY_RES;
void C_radar_data::setZoomRectXY(float ctx, float cty)
{
    zoomXmax = 2*ctx*2.0/scale_ppi+ZOOM_SIZE/2;
    zoomYmax = 2*cty*2.0/scale_ppi+ZOOM_SIZE/2;
    zoomXmin = 2*ctx*2.0/scale_ppi-ZOOM_SIZE/2;
    zoomYmin = 2*cty*2.0/scale_ppi-ZOOM_SIZE/2;
    raw_map_init_zoom();
}


//int C_radar_data::get_tb_tap(){

//    hsTap += ((tb_tap[curAzir])-hsTap)/5.0;
//    return int(hsTap);
//}

void C_radar_data::setTb_tap_k(double value)
{
    tb_tap_k = value;
    if(tb_tap_k<=0)tb_tap_k=1;
}
void C_radar_data::setZoomRectAR(float ctx, float cty,double sizeKM,double sizeDeg)// unit km
{
    double cta,ctr = sqrt(ctx*ctx+cty*cty);
    if(cty==0)
    {
        if(ctx>0)cta = PI_CHIA2;
        else cta = -PI_CHIA2;
    }
    else cta = atan(ctx/cty)-aziOffsetRad;
    if(cty<0)cta+=PI;
    if(cta<0)cta += PI_NHAN2;
    if(cta>PI_NHAN2)cta-=PI_NHAN2;
    cta=cta/PI_NHAN2*MAX_AZIR;
    ctr/=sn_scale;
    zoom_ar_size_a = MAX_AZIR/360.0*sizeDeg;
    zoom_ar_size_r = sizeKM/sn_scale;
    zoom_ar_a0 = cta-zoom_ar_size_a/2.0;
    zoom_ar_a1 = zoom_ar_a0+zoom_ar_size_a;
    if(zoom_ar_a1>MAX_AZIR)zoom_ar_a1-=MAX_AZIR;
    if(zoom_ar_a0<0)zoom_ar_a0+=MAX_AZIR;
    zoom_ar_r0 = ctr-zoom_ar_size_r/2.0;
    if(zoom_ar_r0 <0)zoom_ar_r0=0;
    zoom_ar_r1 = zoom_ar_r0+zoom_ar_size_r;
    if(zoom_ar_r1>range_max)zoom_ar_r1=range_max;
    img_zoom_ar = new QImage(zoom_ar_size_r+1,zoom_ar_size_a+1,QImage::Format_ARGB32);
    //img_zoom_ar->// toto:resize
    //drawZoomAR(a0,r0);

}
bool C_radar_data::DrawZoomAR(int a,int r,short val,short dopler,short sled)
{
    //return true if point is on the edges of the zone
    //if(a<zoom_ar_size_a)a+=MAX_AZIR;
    if(!img_zoom_ar)return false;
    int pa= a-zoom_ar_a0;
    if(pa>=MAX_AZIR)pa-=MAX_AZIR;
    if(pa<0)pa+=MAX_AZIR;
    if(pa>zoom_ar_size_a)return false;
    int pr = r-zoom_ar_r0;
    if(pr>zoom_ar_size_r)return false;
    if(pr<0)return false;
    img_zoom_ar->setPixel(pr,pa,getColor(val,dopler,sled));
    if(pa==zoom_ar_size_a)return true;
    if(pr==zoom_ar_size_r)return true;
    if(pa==0)return true;
    if(pr==0)return true;
    return false;

}
void C_radar_data::resetGain()
{
    //krain_auto = 0.3;
    kgain_auto  = 4;
    //ksea_auto = 0;
}
void C_radar_data::setAutorgs(bool aut)
{
    if(aut)
    {
        cut_noise = true;
        //krain_auto = 0.3;
        //kgain_auto  = 4;
        //ksea_auto = 0;
    }else
    {
        cut_noise = false;
    }

}
void C_radar_data::raw_map_init()
{
    double theta=aziOffsetRad;
    double dTheta = 2*PI/MAX_AZIR_DRAW;
    for(short azir = 0; azir < MAX_AZIR_DRAW; azir++)
    {
        double cost = cos(theta);
        double sint = sin(theta);
        for(short range = 0;range<RAD_DISPLAY_RES;range++)
        {
            data_mem.xkm[azir][range]     =  short(sint*(range+1))+RAD_DISPLAY_RES;
            data_mem.ykm[azir][range]    =  -short(cost*(range+1))+RAD_DISPLAY_RES;
            if(data_mem.xkm[azir][range]<0||data_mem.xkm[azir][range]>=img_ppi->width()||data_mem.ykm[azir][range]<0||data_mem.ykm[azir][range]>=img_ppi->height())
            {
                data_mem.xkm[azir][range] = 0;
                data_mem.ykm[azir][range] = 0;
            }
        }
        theta+=dTheta;
    }
}
void C_radar_data::raw_map_init_zoom()
{
    img_zoom_ppi->fill(Qt::black);
    float theta=aziOffsetRad;
    float dTheta = 2*PI/MAX_AZIR_DRAW;
    for(short azir = 0; azir < MAX_AZIR_DRAW; azir++)
    {

        float cost = cosf(theta);
        float sint = sinf(theta);
        for(short range = 0;range<DISPLAY_RES_ZOOM;range++)
        {
            data_mem.xzoom[azir][range]     =  short(sint*(range+1)) - zoomXmin;
            data_mem.yzoom[azir][range]    =  -short(cost*(range+1)) - zoomYmin;
            if(data_mem.xzoom[azir][range]<0||
                    data_mem.yzoom[azir][range]<0||
                    data_mem.xzoom[azir][range]>ZOOM_SIZE||
                    data_mem.yzoom[azir][range]>ZOOM_SIZE)
            {
                data_mem.xzoom[azir][range] = 0;
                data_mem.yzoom[azir][range] = 0;
            }
        }
        theta += dTheta;
    }
}
void C_radar_data::resetData()
{
    rgStdErr = sn_scale*pow(2,clk_adc);
    switch(clk_adc)
    {
    case 0:
        sn_scale = SIGNAL_SCALE_0;
        break;
    case 1:
        sn_scale = SIGNAL_SCALE_1;//printf("1");
        break;
    case 2:
        sn_scale = SIGNAL_SCALE_2;//printf("2");
        break;
    case 3:
        sn_scale = SIGNAL_SCALE_3;//printf("2");
        break;
    case 4:
        sn_scale = SIGNAL_SCALE_4;//printf("2");
        break;
    case 5:
        sn_scale = SIGNAL_SCALE_5;//printf("2");
        break;
    case 6:
        sn_scale = SIGNAL_SCALE_6;//printf("2");
        break;
    case 7:
        sn_scale = SIGNAL_SCALE_7;//printf("2");
        break;
    default:
        sn_scale = SIGNAL_SCALE_0;
    }
    int dataLen = RADAR_RESOLUTION*MAX_AZIR;
    memset(data_mem.level,      0,dataLen);
    memset(data_mem.dopler,     0,dataLen);
    memset(data_mem.detect,     0,dataLen);
    memset(data_mem.plotIndex,  0,dataLen);
    memset(data_mem.hot,        0,dataLen);
    memset(data_mem.may_hoi,    0,dataLen);
    std::queue<int> empty;
    std::swap( aziToProcess, empty );
    //memset(data_mem.terrain,    TERRAIN_INIT,dataLen);
    //memset(data_mem.rainLevel,  0,dataLen);
    //noiseAverage = 30;
    resetSled();
    init_time += 5;

}
void C_radar_data::resetSled()
{
    memset(data_mem.sled,0,RADAR_RESOLUTION*MAX_AZIR);
}
void C_radar_data::setScalePPI(float scale)
{
    scale_ppi = sn_scale*scale;
    //setScaleZoom(scale/4.0);
    //scale_zoom_ppi = scale_ppi*4;
    //updateZoomRect();
}
void C_radar_data::setScaleZoom(float scale)
{

    scale_zoom_ppi = scale;//SIGNAL_SCALE_0*scale/scale_ppi;
    //updateZoomRect();
}

//void C_radar_data::drawZoomAR()
//{

//      //memcpy(imgAR->bits(),(unsigned char *)&data_mem.level[0][0],MAX_AZIR*RADAR_RESOLUTION);
//      QImage* imgAR = new QImage((unsigned char *)&data_mem.level[0][0],RADAR_RESOLUTION,MAX_AZIR,QImage::Format_Indexed8);
//        imgAR->setColorTable(colorTable);
//      QRect rect(zoom_ar_r0, zoom_ar_a0, 225, 225);
//      *img_zoom_ar = imgAR->copy(rect);//.convertToFormat(QImage::Format_Indexed8,colorTable,Qt::ColorOnly);

//}
void C_radar_data::drawSgnZoom(short azi_draw, short r_pos)
{


    short px = data_mem.xzoom[azi_draw][r_pos];
    short py = data_mem.yzoom[azi_draw][r_pos];
    if(!(px*py))return;
    unsigned char value    = data_mem.display_ray_zoom[r_pos][0];
    unsigned char dopler    = data_mem.display_ray_zoom[r_pos][1];
    unsigned char sled     = data_mem.display_ray_zoom[r_pos][2];
    short pSize = r_pos/150;if(pSize>4)pSize=4;

    //if(pSize>2)pSize = 2;
    if((px<pSize)||(py<pSize)||(px>=img_zoom_ppi->width()-pSize)||(py>=img_zoom_ppi->height()-pSize))return;
    for(short x = -pSize;x <= pSize;x++)
    {
        for(short y = -pSize;y <= pSize;y++)
        {
            double k =1.0/((x*x+y*y)/60.0+1.0);

            unsigned char pvalue = value*k;
            if( data_mem.display_mask_zoom[px+x][py+y] <= pvalue)
            {
                data_mem.display_mask_zoom[px+x][py+y] = pvalue;
                img_zoom_ppi->setPixel(px+x,py+y,getColor(pvalue,dopler,sled));
                //DrawZoom(px,py,pvalue);
            }
        }
    }

}
uint C_radar_data::getColor(unsigned char pvalue,unsigned char dopler,unsigned char sled)
{
    dopler = dopler%16;
    unsigned short value = ((unsigned short)pvalue)*brightness;
    if(!isSled)sled = 0;
    else
        if(sled>=32)sled = 0xff; else sled*=8;
    if(value>0xff)
    {
        value = 0xff;
    }
    unsigned char alpha;
    unsigned char red   = 0;
    unsigned char green = 0;
    unsigned char blue  = 0;
    unsigned char gradation = value<<2;
    uint color;
    switch(imgMode)
    {
    case DOPLER_3_COLOR:
        if(pvalue>1)
        {
            if(dopler==0)
            {
                color = 0xffff00;
            }else
            {
                char dDopler = dopler-1;
                if(dDopler>7)dDopler = 15-dDopler;
                color = 0x00ff00 | ((dDopler<<5));
            }
            alpha = value;//0xff - ((0xff - value)*0.75);
            color = color|(alpha<<24);
        }
        else
        {
            color = (sled<<24)|(0xff);
        }
        //

        break;

    case VALUE_ORANGE_BLUE:
        if(pvalue>1)
        {
            alpha = 0xff - ((0xff - value)*0.75);
            //pvalue-=(pvalue/10);
            switch(value>>6)
            {
            case 3:
                red = 0xff;
                green = 0xff - gradation;
                break;
            case 2:
                red = gradation;
                green = 0xff;
                break;
            case 1:
                green = 0xff ;
                blue = 0xff - gradation;
                break;
            case 0:
                green = gradation ;
                blue = 0xff;
                break;
            }
            color = (alpha<<24)|(red<<16)|(green<<8)|blue;
        }
        else
        {
            color = (sled<<24)|(0xff);
        }

        break;
    case VALUE_YELLOW_SHADES:
        if(pvalue>1)
        {
            alpha = value;//0xff - ((0xff - pvalue)*0.75);
            color = (value<<24)|(0xff<<16)|(0xff<<8);
        }
        else
        {
            color = (sled<<24)|(0xff);
        }
        break;
    default:
        break;
    }
    return color;
}
void C_radar_data::resetTrack()
{
    init_time += 3;
    curIdCount = 1;
    //mTrackList.clear();
    for(unsigned short i=0;i<mTrackList.size();i++)
    {

        mTrackList[i].mState = TrackState::removed;

    }
}

void C_radar_data::ProcessTracks()
{
    //processTracks

    for (ushort j=0;j<mTrackList.size();j++)
    {
        C_primary_track* track = &(mTrackList[j]);
        if(track->mState==TrackState::removed)continue;
        if(track->mState==TrackState::lost)continue;
        track->update(now_ms);


    }
}
bool C_radar_data::ProcessObject(object_t *obj1)
{
    //check  if object_t belonging to tracks
    if(mTrackList.size())
        if(checkBelongToTrack(obj1))
            return true;
    // check if object_t belonging to lines
    //if(checkBelongToLine(obj1))return;
    // check if object_t belonging to another obj
    if(checkBelongToObj(obj1))return true;
    // add to mObjList
    bool full = true;
    for(uint i=0;i<mFreeObjList.size(); i++)
    {
        if(mFreeObjList.at(i).isRemoved)
        {
            mFreeObjList.at(i) = *obj1;
            full = false;
            break;
        }
    }
    if(full&&mFreeObjList.size()<2000)
    {
        mFreeObjList.push_back(*obj1);
    }
    return false;

}

bool C_radar_data::checkBelongToTrack(object_t *obj1)
{
    bool isBelongingToTrack = false;
    C_primary_track* chosenTrack =nullptr;
    double maxScore=0;
    for (ushort j=0;j<mTrackList.size();j++)
    {
        C_primary_track* track = &(mTrackList[j]);

        if(track->mState==TrackState::removed||
                track->mState==TrackState::lost)continue;
        if(track->isUpdating)
        {
            continue;
        }
        //object_t *obj2 = &(track->objectList.back());
        double score = track->estimateScore(obj1);//todo: optimize this score
        //object_t* obj2 = &(track->objectList.back());
        //unsigned int dtime = (obj1->timeMs - obj2->timeMs);
        /*if(dtime<500)
            continue;//ENVAR min time between plots in a line(1s)
        if(dtime>40000)
            continue;//ENVAR max time between plots in a line(40s)
        double rgSpeedkmh = (obj1->rgKm-obj2->rgKm)/(dtime/3600000.0);
        double dRgSp = (rgSpeedkmh - track->rgSpeedkmh)/40.0;*/
        //double score1 = estimateScore(obj1,obj2);
        //double score2 = powl(CONST_E,-dRgSp*dRgSp);
        //score = score1*score2;
        //printf("\ndtime:%dscore:%f,%f,%f",dtime,score1,score2,score);
        //-----------------------------------

        if(score>maxScore&&score>track->possibleMaxScore)
        {
            maxScore=score;
            chosenTrack = track;
            isBelongingToTrack = true;
        }

    }
    if(isBelongingToTrack)
    {
        if(chosenTrack->possibleMaxScore>0)
        {
            //reprocess
            object_t tempObj = chosenTrack->possibleObj;
            chosenTrack->addPossible(obj1,maxScore);
            ProcessObject(&tempObj);
        }
        else
        {
            chosenTrack->addPossible(obj1,maxScore);
        }
        obj1->isRemoved = true;//free the object
        return true;
    }
    else
    {
        //printf(" maxScore:%f",maxScore);
        return false;
    }
}

void C_radar_data::CreateTrack(object_t* obj1,object_t* obj2)
{
    bool detectConfirmed = false;
    for(uint i=0;i<mDetectZonesList.size();i++)
    {
        DetectionWindow *dw = &mDetectZonesList[i];
        if(!dw->trackCount)continue;
        if((now_ms-dw->timeStart>80000)){dw->trackCount=0;continue;}
        if((abs(degrees(obj2->azRad)-dw->aziDeg))<dw->maxDazDeg
            &&(abs(obj2->rgKm-dw->rg)<dw->maxDrg)
                )
        {
            detectConfirmed = true;
            if(dw->trackCount>0)dw->trackCount--;
        }
    }
    if(!detectConfirmed)return;
    int id = C_primary_track::IDCounter++;
    for (ushort j=0;j<mTrackList.size();j++)
    {
        if(mTrackList[j].mState==TrackState::removed)
        {
            //            track_t newTrack(obj1,obj2);
            mTrackList[j].init(obj1,obj2,id);
            return;
        }
    }
    for (ushort j=0;j<mTrackList.size();j++)
    {
        if(mTrackList[j].mState==TrackState::lost)
        {
            //            track_t newTrack(obj1,obj2);
            mTrackList[j].init(obj1,obj2,id);
            return;
        }
    }

    printf("\nmTrackList is full");

}
bool C_radar_data::checkBelongToObj(object_t* obj1)
{
    object_t *objLast = nullptr;
    double maxScore=0;
    for (ushort j=0;j<mFreeObjList.size();j++)
    {
        object_t *obj2 = &(mFreeObjList.at(j));
        //find new line
        if(obj2->isRemoved)continue;
        //        uint dtime = (obj1->timeMs - obj2->timeMs);
        uint dperiod = (obj1->period-obj2->period);
        if(dperiod<300)
        {
            continue;
        }
        if(dperiod>MAX_AZIR*1.5)
        {
            obj2->isRemoved = true;
            continue;
        }
        //float rgSpeedkmh = (obj1->rgKm-obj2->rgKm)/(dtime/3600000.0);
        //if(rgSpeedkmh>85)continue;//ENVAR
        double score = C_primary_track::estimateScore(obj1,obj2);
        if(score<=0)continue;
        if(score>maxScore)
        {
            maxScore = score;
            objLast = obj2;
        }
    }
    if(maxScore>0.5)//todo: find best value
    {
        //obj1->scorepObj = maxScore;
        //obj1->isRemoved=true;
        //objLast->isRemoved = true;
        CreateTrack(obj1,objLast);objLast->isRemoved = true;
        return true;
    }
    else return false;

}

