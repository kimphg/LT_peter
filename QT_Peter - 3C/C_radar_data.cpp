#define PI 3.1415926536
#include "c_config.h"
#include "c_radar_data.h"
//#include <QElapsedTimer>
#include <cmath>
//#include <QtDebug>

#define PLOT_MAX_SIZE 80
#define PLOT_MIN_SIZE 5
#define PLOT_MAX_DR 10
#define RANGE_MIN 20
#define TERRAIN_MAX 40
#define TERRAIN_INIT 20
#define RADAR_COMMAND_FEEDBACK  6
#define RADAR_HEADER_LEN   34
#define RADAR_DATA_SPECTRE      22
#define RADAR_DATA_MAX_SIZE     2688
#define RADAR_GAIN_MIN 3.0
#define RADAR_GAIN_MAX 9.0

#define TARGET_OBSERV_PERIOD 6500//ENVAR max periods to save object in the memory
static  FILE *logfile;
int C_primary_track::IDCounter =1;
static int sumvar = 0;
static int nNoiseFrameCount = 0;
static short indexLastProcessAzi = 0;
static short indexCurrProcessAzi = 0;
static short    curIdCount = 1;
static qint64   cur_rot_timeMSecs ;//= QDateTime::currentMSecsSinceEpoch();
//static int      antennaHeadOffset;
static float    rot_period_min =0;
static short histogram[256];
#define AZI_QUEUE_SIZE 500
static int aziToProcess[AZI_QUEUE_SIZE];
static int indexCurrRecAzi = 0;
//qint64   CConfig::time_now_ms ;
typedef struct  {
    //processing dataaziQueue
    unsigned char level [MAX_AZIR][RADAR_RESOLUTION];
    unsigned char may_hoi[MAX_AZIR][RADAR_RESOLUTION];
    unsigned char level_disp [MAX_AZIR][RADAR_RESOLUTION];
    bool          detect[MAX_AZIR][RADAR_RESOLUTION];
    //unsigned char rainLevel[MAX_AZIR][RADAR_RESOLUTION];
    unsigned char dopler[MAX_AZIR][RADAR_RESOLUTION];
    unsigned int terrainMap[MAX_AZIR][RADAR_RESOLUTION];
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
static signal_map_t data_mem;
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


void C_primary_track::addPossible(object_t *obj,double score)
{
    possibleObj=(*obj);
    possibleMaxScore=score;
}

double C_primary_track::estimateScore(object_t *obj1,object_t *obj2)
{
    double dtime = int(obj1->timeMs - obj2->timeMs);
    if(dtime<TRACK_MIN_DTIME)return -1;
    if(dtime>TRACK_MAX_DTIME)return -1;
    dtime/=3600000.0;//time in hours
    double dx = obj1->xkm - obj2->xkm;
    double dy = obj1->ykm - obj2->ykm;

    double distancekm = sqrt(dx*dx+dy*dy);

    //double speedkmh = distancekm/(dtime);
    //if(speedkmh>500)return -1;
    double distanceCoeff = distancekm/(TARGET_MAX_SPEED_MARINE*dtime+ obj1->rgKm*AZI_ERROR_STD);
    if(distanceCoeff>3.0)return -1;
    double rgSpeedkmh = abs(obj1->rgKm - obj2->rgKm)/(dtime);
    //double dDopler = abs(obj1->dopler-obj2->dopler);
    //    if(dDopler>7)dDopler=16-dDopler;
    //    if(dDopler>3)
    //    {
    //        printf("\n 2 objs rejected by dopler diff:%f",dDopler);return -1;
    //    }
    //normalize params
    //speedkmh/=TARGET_MAX_SPEED_MARINE;
    rgSpeedkmh = abs(rgSpeedkmh)/(TARGET_MAX_SPEED_MARINE+obj1->rgStdEr);
    //dDopler/=2.0;
    double score =
            fastPow(CONST_E,-sq(distanceCoeff))
            //*fastPow(CONST_E,-sq(dDopler))
            *fastPow(CONST_E,-sq(rgSpeedkmh));
#ifdef DEBUGMODE
    /*if(distancekm>0.1&&score>0.1){
    fprintf(logfile,"\n%f,",score);//1.0
    fprintf(logfile,"%f,",distancekm);//1.0
    fprintf(logfile,"%f,",speedkmh);//1.0
    fprintf(logfile,"%f,",distanceCoeff);//500
    fprintf(logfile,"%f,",rgSpeedkmh);
    }*/
#endif
    return score;
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
double C_primary_track::LinearFitProbability(object_t *myobj)
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
        t[i] = int(CConfig::time_now_ms-obj[i]->timeMs);
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
    double probability = 1;
    for(int i=0;i<FIT_ELEMENTS;i++)
    {
        double x1 =abs(azRadfit[i]-obj[i]->azRad)/obj[i]->aziStdEr;
        probability*=fastPow(CONST_E,-sq(x1)/2.0);
        double x2 =abs(rgKmfit[i]  -obj[i]->rgKm)/obj[i]->rgStdEr;
        probability*=fastPow(CONST_E,-sq(x2)/2.0);
    }
    return probability;
}

double C_primary_track::estimateScore(object_t *obj1)
{
    //return estimateScore(obj1,&(this->objectList.back()));
    if(objectList.size()<2)
        return -1;
    object_t* obj2 = &(this->objectList.back());
    double dtime = int(obj1->timeMs - obj2->timeMs);
    if(dtime<TRACK_MIN_DTIME)return -1;
    if(dtime>TRACK_MAX_DTIME)return -1;
    dtime/=3600000.0;
    double dx = obj1->xkm - obj2->xkm;
    double dy = obj1->ykm - obj2->ykm;


    double distancekm = sqrt(dx*dx+dy*dy);
    double distanceCoeff = distancekm/(TARGET_MAX_SPEED_MARINE*dtime   + obj1->rgKm*AZI_ERROR_STD);
    if(distanceCoeff>3.0)
    {
        //printf("\n obj rejected by distanceCoeff");
        return -1;
    }
    //    double dDopler = abs(obj1->dopler-obj2->dopler);
    //    if(dDopler>7.0)dDopler=abs(16-dDopler);
    //    if(dDopler>4)
    //    {
    //        printf("\n obj rejected by dopler diff:%f",dDopler);
    //        return -1;
    //    }
    //    double dBearing = ConvXYToAziRd(dx,dy)-this->courseRad;
    //double speedkmh = distancekm/(dtime);
    //if(speedkmh>500.0)return -1;
    //double dSpeed = abs(speedkmh-this->mSpeedkmh);//*cosFast(-dBearing));
    //if(dSpeed>600.0)return -1;
    double rgSpeedkmh = (obj1->rgKm-obj2->rgKm)/(dtime);
    if(abs(rgSpeedkmh)>TARGET_MAX_SPEED_MARINE*2)return -1;
    double dRgSp = abs(rgSpeedkmh - this->rgSpeedkmh);
    if(abs(dRgSp)>TARGET_MAX_SPEED_MARINE)return -1;


    double linearFitProb = LinearFitProbability(obj1);
    //normalize machine learning likelihood model
    rgSpeedkmh/=(TARGET_MAX_SPEED_MARINE+obj1->rgStdEr);
    //dSpeed/=TARGET_MAX_SPEED_MARINE;
    //speedkmh/=TARGET_MAX_SPEED_MARINE;
    //dRgSp/=TARGET_MAX_SPEED_MARINE+obj1->rgStdEr;
    //    linearFit/=0.8;
    dtime/=0.011;
    double dazi = abs(obj1->azRad-obj2->azRad);
    if(dazi>PI)dazi = PI_NHAN2-dazi;
    dazi       /=(AZI_ERROR_STD+mSpeedkmh*dtime/obj1->rgKm);
    if(dazi>3)return -1;
    //    dDopler/=2.0;
    /*
    rgSpeedkmh/=50.0;
    dSpeed/=200.0;
    speedkmh/=150.0;
    dRgSp/=40.0;
    linearFit/=0.16;
    dtime/=0.004167;*/
    double score = fastPow(CONST_E,-sq(distanceCoeff))
            *fastPow(CONST_E,-sq(rgSpeedkmh))
            //*fastPow(CONST_E,-sq(dSpeed))
            //            *fastPow(CONST_E,-sq(speedkmh))
            //            *fastPow(CONST_E,-sq(dDopler))
            *fastPow(CONST_E,-sq(dazi))
            //*fastPow(CONST_E,-sq(dRgSp))
            *linearFitProb
            *fastPow(CONST_E,-sq(dtime));
#ifdef DEBUGMODE
    {
        fprintf(logfile,"\n%f,",score);//1.0
        fprintf(logfile,"%f,",distancekm);//1.0
        fprintf(logfile,"%f,",distanceCoeff);//1.0
        //fprintf(logfile,"%f,",speedkmh);//500
        //fprintf(logfile,"%f,",abs(dSpeed));//600
        fprintf(logfile,"%f,",abs(rgSpeedkmh));//150
        //    fprintf(logfile,"%f,",abs(dRgSp));//120
        fprintf(logfile,"%f,",linearFitProb);//0.5
    }
#endif
    return score;
    /*if(obj1->dopler==this->objectList.back().dopler)
    {fprintf(logfile,"1\n");
        return 1;
    }
    else
        fprintf(logfile,"-1\n");
    return -1;*/


}
void C_primary_track::update()
{
    isUpdating = true;
    ageMs=uint(CConfig::time_now_ms-lastTimeMs);
    if(mState == TrackState::removed)return;
    else if(ageMs>TRACK_DELETE_TIME)
    {mState = TrackState::removed;return;}
    else if((mState!= TrackState::lost)&&ageMs>TRACK_LOST_TIME)
    {
        if(mState==TrackState::newDetection)
        {
            mState = TrackState::removed;
        }
        else {
            mState = TrackState::lost;
            CConfig::AddMessage(QString::fromUtf8("Mất MT SH:")+
                                QString::number(uniqId)+" PV:"+
                                QString::number(aziDeg,'f',1)+" CL:"+
                                QString::number(nm(rgKm),'f',1)
                                );
            //            printf("\ntrack lost id:%d len:%llu",uniqId, objectList.size());
            //            _flushall();
        }
        return;
    }
    if(possibleMaxScore>0)
    {
        if(CConfig::time_now_ms-possibleObj.timeMs>TRACK_MIN_DTIME)
        {

            objectList.push_back(possibleObj);
            mDopler = possibleObj.dopler;
            lastTimeMs = possibleObj.timeMs;
            if(mDopler>7)mDopler-=16;
            while(objectList.size()>TRACK_STABLE_LEN)
            {
                if((lastTimeMs-objectHistory.back().timeMs)>60000)
                    objectHistory.push_back(objectList[0]);
                objectList.erase(objectList.begin());
                if(mState==TrackState::newDetection)
                {
                    if(mSpeedkmhFit<TARGET_MAX_SPEED_MARINE)
                    {
                        LinearFit(TRACK_STABLE_LEN);
                        if(fitProbability<0.5)
                        {
                            printf("\n fitProbability too small:%f",fitProbability);
                        }
                        else
                        {
                            uniqId = C_primary_track::IDCounter++;
                            mState = TrackState::confirmed;
                        }
                    }
                }
            }


            possibleMaxScore = 0;
            if(mState==TrackState::newDetection)
            {
                object_t* obj1  = &(objectList.back());
                object_t* obj2 ;
                if(objectHistory.size()>2)
                {
                    obj2 = &(objectHistory.back())-2;
                }
                else if(objectHistory.size()>1)
                {
                    obj2 = &(objectHistory.back())-1;
                }
                else
                {
                    obj2 = &(objectList.at(0));
                }
                if(obj1->timeMs != obj2->timeMs)
                {
                    double dx       = obj1->xkm - obj2->xkm;
                    double dy       = obj1->ykm - obj2->ykm;
                    double dtime    = (obj1->timeMs - obj2->timeMs)/3600000.0;
                    rgSpeedkmh      = (obj1->rgKm - obj2->rgKm)/dtime;
                    //speed param
                    mSpeedkmhFit    = sqrt(dx*dx+dy*dy)/dtime;
                    sko_spd         = mSpeedkmhFit/2.0;
                    //course param
                    courseRadFit    = ConvXYToAziRd(dx,dy);
                    courseDeg = degrees(courseRadFit);
                    sko_cour = 30.0;
                }
                else
                { printf("\ntrung object, khong the tinh van toc");}
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
            else if(mState==TrackState::confirmed)
            {

                LinearFit(TRACK_STABLE_LEN);
                object_t* obj1  = &(objectList.back());
                object_t* obj2  ;
                if(objectHistory.size()>2)
                {
                    obj2 = &(objectHistory.back())-2;
                }
                else if(objectHistory.size()>1)
                {
                    obj2 = &(objectHistory.back())-1;
                }
                else
                {
                    obj2 = &(objectList.at(0));
                }
                double dx       = obj1->xkm - obj2->xkm;
                double dy       = obj1->ykm - obj2->ykm;
                if(obj1->timeMs!=obj2->timeMs)
                {
                    double dtime    = (obj1->timeMs-obj2->timeMs)/3600000.0;
                    rgSpeedkmh      = (obj1->rgKm-obj2->rgKm)/dtime;
                    //speed param
                    double mSpeedkmhFitNew    = sqrt(dx*dx+dy*dy)/dtime;
                    double sko_spdNew = abs(mSpeedkmhFitNew-mSpeedkmhFit);
                    sko_spd         +=(sko_spdNew-sko_spd)/5.0;
                    mSpeedkmhFit    +=(mSpeedkmhFitNew-mSpeedkmhFit)/5.0;
                    //course param
                    courseRadFit   = ConvXYToAziRd(dx,dy);
                    double courseDegNew = degrees(courseRadFit);
                    double sko_courNew = abs(courseDegNew-courseDeg);
                    if(sko_courNew>180)sko_courNew-=360;
                    if(sko_courNew<-180)sko_courNew+=360;
                    courseDeg       = courseDegNew;
                    sko_cour        +=(sko_courNew-sko_cour)/5.0;
                }
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
                generateTTM();
            }
        }

    }
    isUpdating = false;
}
void C_primary_track::generateTTM()
{
    mTTM = "$RATTM,"+QString::number(uniqId)+","+
            QString::number(nm(rgKm),'f',2)+","+
            QString::number(aziDeg,'f',1)+","+
            +"T,"+
            QString::number(nm(mSpeedkmhFit),'f',1)+","+
            QString::number(courseDeg,'f',1)+","+
            +"T,"+
            "0.0"+","+
            "0.0"+","+
            "N,,Q,,,A,";
    uchar a = getCheckSum(mTTM);
    char chs[] = {0,0,0};
    bin2hex(a,&chs[0]);
    mTTM+="*"+QString(chs)+"\r\n";
}

uchar C_primary_track::getCheckSum(QString message)
{
    char* data = (char*)message.toStdString().data();
    uchar sum = 0;
    for(int i=1;i<message.size();i++)
    {
        sum^=uchar(data[i]);
    }
    return (sum);
}
void C_primary_track::LinearFit(int nEle)
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
    //uint nEle=4;
    if(this->objectList.size()<nEle)
    {
        printf("\nlinear fit error");
        return;
    }
    object_t* obj = &(this->objectList[(this->objectList.size()- nEle)]) ;
    double *y1 = new double[nEle];
    double *y2 = new double[nEle];
    double *t  = new double[nEle];
    for(int i=0;i<nEle;i++)
    {
        y1[i] = obj[i].xkm;
        y2[i] = obj[i].ykm;
        t[i]  = int(CConfig::time_now_ms- obj[i].timeMs);
    }
    double y1sum = 0;//r1+r2+r3;
    double y2sum = 0;
    double tsum = 0;
    double t2sum = 0;
    double y1tsum = 0;
    double y2tsum = 0;
    for(int i=0;i<nEle;i++)
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
    for(int i=0;i<nEle;i++)
    {
        y1[i]=y1a*t[i]+y1b;
    }
    double y2a=(nEle*y2tsum-tsum*y2sum)/(nEle*t2sum-tsum*tsum);
    double y2b=(t2sum*y2sum-tsum*y2tsum)/(t2sum*nEle-tsum*tsum);
    for(int i=0;i<nEle;i++)// !!index start from 1
    {
        y2[i]=y2a*t[i]+y2b;
    }
    for(int i=1;i<nEle;i++)// !!index start from 1
    {
        obj[i].xkm+=(y1[i]-obj[i].xkm)*i/double(nEle);
        obj[i].ykm+=(y2[i]-obj[i].ykm)*i/double(nEle);
    }
    //
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
    double probability = 1;
    for(int i=0;i<FIT_ELEMENTS;i++)
    {
        double x1 =abs(azRadfit[i]-obj[i].azRad)/obj[i].aziStdEr;
        probability*=fastPow(CONST_E,-sq(x1)/2.0);
        double x2 =abs(rgKmfit[i]  -obj[i].rgKm)/obj[i].rgStdEr;
        probability*=fastPow(CONST_E,-sq(x2)/2.0);
    }

    delete[] y1;
    delete[] y2;
    delete[] t;
    fitProbability=probability;
}

C_radar_data::C_radar_data()
{
    isTxOn = false;
    cut_terrain=false;
    mTerrainAvailable = false;
    mShipHeading2048 = 0;
    aziViewOffset = 0;
    antennaHeadOffset=int((CConfig::getDouble("antennaHeadOffset",13))/360.0*MAX_AZIR);
    while((antennaHeadOffset)>=MAX_AZIR)antennaHeadOffset-=MAX_AZIR;
    while((antennaHeadOffset)<=0)antennaHeadOffset+=MAX_AZIR;

    mInverseRotAziCorrection = CConfig::getDouble("mInverseRotAziCorrection",0)/360.0*MAX_AZIR;
    mRealAziRate = 0;
    mUpdateTime = clock();
    cur_rot_timeMSecs = QDateTime::currentMSecsSinceEpoch();
    C_primary_track track;
    mTrackList = std::vector<C_primary_track>(MAX_TRACKS_COUNT,track);
    giaQuayPhanCung = false;
    //    mShipHeading = 0;
    isTrueHeadingFromRadar = true;
    rgStdErr = sn_scale*pow(2,clk_adc);
    azi_er_rad = AZI_ERROR_STD;
    CConfig::time_now_ms = QDateTime::currentMSecsSinceEpoch();
    mFalsePositiveCount = 0;
    mSledValue = CConfig::getInt("mSledValue",200);
    isInverseRotation = 0;
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
    cut_noise = CConfig::getInt("cut_noise");
    //    doubleFilter = false;
    rotation_per_min = 0;
    bo_bang_0 = false;
    data_export = false;
    gat_mua_va_dia_vat = CConfig::getInt("gat_mua_va_dia_vat");
    noise_nornalize = false;
    filter2of3 = CConfig::getInt("filter2of3");
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
    isShowSled = CConfig::getInt("isShowSled");
    init_time = 5;
    dataOver = max_s_m_200;
    curAzirTrue2048 = 0;
    arcMaxAzi = 0;
    arcMinAzi = 0;
    //    isSharpEye = false;
    sn_scale = SIGNAL_SCALE_0;
    raw_map_init();
    raw_map_init_zoom();
    //    setAziOffset(0);
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
    double result = ((double)arcMaxAzi/(double)MAX_AZIR*PI_NHAN2);
    if(result>PI_NHAN2)result-=PI_NHAN2;
    return ( result);
}
double C_radar_data::getArcMinAziRad() const
{
    double result = ((double)arcMinAzi/(double)MAX_AZIR*PI_NHAN2);
    if(result>PI_NHAN2)result-=PI_NHAN2;
    return (result );
}

void C_radar_data::addDetectionZoneAZ(double az, double rg, double dazi, double drg,bool isOneTime)
{

    DetectionWindow dw;//todo: save dw to config
    dw.isOneTime = isOneTime;
    dw.isRemoved = false;
    dw.timeStart=CConfig::time_now_ms;
    dw.xkm=rg*sin((az));
    dw.ykm=rg*cos((az));
    dw.aziDeg = degrees(az);
    dw.rg = (rg);
    dw.maxDazDeg = degrees(dazi);
    dw.maxDrg = drg;
    addDetectionZone(dw);
}
void C_radar_data::addDetectionZone(DetectionWindow dw)
{
    for(uint i=0;i<mDetectZonesList.size();i++)
    {
        if(mDetectZonesList[i].isRemoved)
        {
            mDetectZonesList[i] = dw;
            return;
        }
    }
    mDetectZonesList.push_back(dw);
}
void C_radar_data::addDetectionZone(double x, double y, double dazi, double drg,bool isOneTime)
{
    DetectionWindow dw;
    dw.isOneTime = isOneTime;
    dw.isRemoved = false;
    dw.timeStart=CConfig::time_now_ms;
    dw.xkm=x;
    dw.ykm=y;
    kmxyToPolarDeg(x,y,&dw.aziDeg,&dw.rg);
    dw.maxDazDeg = (dazi);
    dw.maxDrg = drg;
    addDetectionZone(dw);

}
/*
void C_radar_data::setShipHeading(int shipHeading)
{
    double headingDegOld = mShipHeading*360.0/MAX_AZIR;
    mShipHeading = shipHeading;
    double headingDeg = mShipHeading*360.0/MAX_AZIR;
    mPPITrans.reset();
    mPPITrans = mPPITrans.rotate((headingDegOld-headingDeg));
    isShipHeadingChanged = true;
}
*/
double C_radar_data::getSelfRotationAzi() const
{
    return selfRotationAzi;
}

void C_radar_data::setSelfRotationAzi(int value)
{
    selfRotationAzi = value;

}
double C_radar_data::getCurAziTrueRad() const
{

    double result = ((double)curAzirTrue2048/(double)MAX_AZIR*PI_NHAN2);
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
    if(r_pos<100)pSize=1;
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

    //clear old image
    if(isInverseRotation)
    {
        //reset the display masks
        short prev_azi = azi - 20;
        if(prev_azi<0)prev_azi += MAX_AZIR;
        drawBlackAzi(prev_azi*3+2);
        drawBlackAzi(prev_azi*3+1);
        drawBlackAzi(prev_azi*3);
        //reset the drawing ray
        memset(&data_mem.display_ray[0][0],0,RAD_DISPLAY_RES*3);
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
//        if(r_pos==500)dopler = 16;
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
            if (data_mem.display_ray[display_pos][2] < data_mem.sled[azi][r_pos])
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

    if(nNoiseFrameCount<50)return;

    short histogram_max_val=1;
    short histogram_max_pos=0;
    noiseVar+=(sumvar/float(nNoiseFrameCount)-noiseVar)/2.0f;
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
    nNoiseFrameCount=0;
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
        unsigned char* pLevel = &(data_mem.level[azi][r_pos]);
        unsigned char* pSled=   &(data_mem.sled[azi][r_pos]);
        bool* pDetect=          &(data_mem.detect[azi][r_pos]);
        rainLevel += krain_auto*((*pLevel)-rainLevel);
        if(rainLevel>MAX_RAIN)rainLevel = MAX_RAIN;
        short nthresh = rainLevel + noiseVar*kgain_auto;
        threshRay[r_pos] += (nthresh-threshRay[r_pos])*0.5;
        bool underThreshold = (*pLevel)<threshRay[r_pos];
        //if(data_mem.dopler[azi][r_pos]!=data_mem.dopler[lastAzi][r_pos])underThreshold = true;
        //(*pDetect) = (!underThreshold);
        //if(!underThreshold)if(!init_time)if(r_pos>RANGE_MIN&&r_pos<(range_max-RANGE_MIN))procPix(azi,lastAzi,r_pos);
        //        if(mTerrainEnabled)
        //        {
        //            if(data_mem.terrainMap[azi][r_pos]>150)underThreshold=true;
        //        }

        if(filter2of3)
        {
            unsigned char* pHot= &(data_mem.hot[azi][r_pos]);
            if(underThreshold)
            {
                if((*pHot))(*pHot)--;
            }
            else
            {
                (*pHot)++;
                if((*pHot)>3)(*pHot)=3;
            }
            (*pDetect) = ((*pHot)>1);
        }
        else
        {
            (*pDetect) = !underThreshold;
        }
        if((*pDetect))if(!init_time)if(r_pos>RANGE_MIN&&r_pos<(range_max-RANGE_MIN))procPix(azi,lastAzi,r_pos);
        if(cut_terrain&&mTerrainAvailable)
        {
            if(data_mem.terrainMap[azi][r_pos]>300)underThreshold=true;
        }
        // display value
        if(!isManualTune)
        {
            if(noise_nornalize&&(!cut_noise))
            {
                short dif = ((*pLevel)+32+ noiseVar*kgain_auto -threshRay[r_pos]);
                if(dif<0)dif=0;
                else if(dif>255)dif=255;
                displayVal=dif;
            }
            else displayVal=(*pLevel);
            if(cut_terrain)
            {
                (*pLevel) = (data_mem.terrainMap[azi][r_pos]>150)?255:0;
            }
            else if(*pDetect)
            {
                if(!init_time)if((*pSled)<(*pLevel)) (*pSled)= (*pLevel);
            }
            else
            {
                if((*pSled)>0)  if((rand()%20)==0)    (*pSled)--;

            }
            if(cut_noise&&(!(*pDetect)))displayVal= 0;

            if(displayVal>255)displayVal=255;
            data_mem.level_disp[azi][r_pos]=displayVal;
        }
        if(data_mem.may_hoi[azi/2][r_pos])
            data_mem.dopler[azi][r_pos]|=0x10;

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

            if(displayVal>255)displayVal=255;
            data_mem.level_disp[azi][r_pos]=displayVal;

        }
    }
    return ;
}
void C_radar_data::ProcessEach90Deg()
{
    //    UpdateTrackStatistic();

    //remove old points
    int nObj = 0;
    for (uint i=0;i<mFreeObjList.size();i++)
    {

        if(!mFreeObjList.at(i).isRemoved)//todo: optimize code
        {
            if(CConfig::time_now_ms-mFreeObjList.at(i).timeMs>TRACK_MAX_DTIME)
                mFreeObjList.at(i).isRemoved = true;
            else nObj++;

        }

    }

    //
    if(nObj>1000)
    {
        if(kgain_auto<7.5)
        {
            kgain_auto*=1.05;
            printf("\ntoo many obj,kgain_auto:%f",kgain_auto);
        }
    }
    else if(nObj<20)
    {if(kgain_auto>4.2)kgain_auto/=1.05;}
    if(mFalsePositiveCount>500)//ENVAR
    {
        if(kgain_auto<10)kgain_auto*=1.05;
        printf("\ntoo many false positive kgain_auto:%f",kgain_auto);
    }
    mFalsePositiveCount = 0;

    //calculate rotation speed

    qint64 newtime = CConfig::time_now_ms;
    qint64 dtime = newtime - cur_rot_timeMSecs;
    if(dtime<60000&&dtime>0)
    {
        rot_period_min = (dtime/15000.0);// *4/60
        rotation_per_min =1/rot_period_min;

        if(isSelfRotation)
        {
            double rateError = rotation_per_min/selfRotationRate;
            selfRotationDazi/=rateError;
        }
    }
    cur_rot_timeMSecs = newtime;


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
void C_radar_data::setShipHeadingDeg(double headingDeg)
{
    if(!isTrueHeadingFromRadar)
    {
        mShipHeading2048 = (headingDeg)/360.0*MAX_AZIR;
    }

}

int C_radar_data::approximateAzi(int newAzi)
{
    double dazi = newAzi-mRealAzi;
    if(dazi>MAX_AZIR/2)dazi = dazi-MAX_AZIR;else if(dazi<(-MAX_AZIR/2))dazi = dazi+MAX_AZIR;
    if((abs(dazi)>10))
    {
        mRealAziRate=0.5;
        mRealAzi=newAzi;
        init_time+=2;
        //printf("\n newAzi:%4d init_time:%d",newAzi,init_time);
    }
    else
    {
        mRealAziRate+=(dazi-mRealAziRate)/10.0;
        if(mRealAziRate>10)mRealAziRate=10;
        else if(mRealAziRate<-10)mRealAziRate=-10;
        mRealAzi+=mRealAziRate;
    }
    // intazi
    int intAzi;
    if(mRealAziRate<0)  intAzi=int(mRealAzi+mInverseRotAziCorrection+0.5);
    else                intAzi=int(mRealAzi+0.5);
    if(intAzi>=MAX_AZIR){mRealAzi-=MAX_AZIR;intAzi-=MAX_AZIR;}
    else if(intAzi<0)  {mRealAzi+=MAX_AZIR;intAzi+=MAX_AZIR;}
    return intAzi;
}
void C_radar_data::processSocketData(unsigned char* data,short len)
{
    CConfig::mStat.mFrameCount++;
#ifndef THEON
    //bool isGo = (data[0]==4);// du lieu may hoi
    if((data[0]==4))
    {
        ProcessGOData(data, len,curAzirTrue2048/2);
        return;

    }

#else
    if((data[0]==4))
    {
        return;

    }
#endif
    
    //check data valid
    if(data[0]!=0x55&&data[0]>7)
        return;
    /*if(data[0]==5)
    {
        if(range_max!=512)

        {
            range_max = 512;
            resetData();
        }

    }
    else if((data[0]==6)||(data[0]==7))
    {
        if(range_max!=1024)

        {
            range_max = 512;
            resetData();
        }
    }
    else if((data[0]>=0)&&(data[0]<4))
    {
        if(range_max!=RADAR_RESOLUTION)

        {
            range_max = 512;
            resetData();
        }
    }*/
    memcpy(mHeader,data,FRAME_HEADER_SIZE);
    unsigned char n_clk_adc = data[4];
    mFreq = 0x0F&(data[20]);
    sn_stat = (data[5]<<8)+data[6];
    isTxOn = (data[21])>>4;
    if(clk_adc != n_clk_adc)
    {
        // clock adc
        clk_adc = n_clk_adc;
        isClkAdcChanged = true;
        UpdateData();
        resetData();
    }
    uint newAziTrue =0;

     if(isSelfRotation)
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
        newAziTrue = selfRotationAzi;
    }
    else if(data[0]==0x55)//TH tao gia
    {
        newAziTrue = (data[2]<<8)|data[3];
        //printf("\nheading:%d",heading);
        //printf(" newAzi:%d",newAzi);
    }
    else
    {
#ifdef THEON
        newAziTrue =  ((data[11]<<8)|data[12])>>5;
        newAziTrue+= (mShipHeading2048+antennaHeadOffset);
        while(newAziTrue>=MAX_AZIR)newAziTrue-=MAX_AZIR;
#else
        if(giaQuayPhanCung)
        {
            newAziTrue = ((data[11]<<8)|(data[12]))>>5;

        }
        else
        {
            newAziTrue = (data[9]<<24)|(data[10]<<16)|(data[11]<<8)|(data[12]);
            newAziTrue>>=3;
            newAziTrue&=0xffff;
            newAziTrue = ssiDecode(newAziTrue);

        }
        if(isTrueHeadingFromRadar)
        {
            mShipHeading2048 = (((data[13]<<8)|data[14]))/65536.0*MAX_AZIR;
            CConfig::mStat.inputGyro(mShipHeading2048*360.0/MAX_AZIR,0);
        }
        newAziTrue+= (mShipHeading2048+antennaHeadOffset);
        while(newAziTrue>=MAX_AZIR)newAziTrue-=MAX_AZIR;
        newAziTrue = approximateAzi(newAziTrue);


#endif
    }
    //    azi queue



    if(curAzirTrue2048==newAziTrue)return;

    int dIntAzi = newAziTrue -curAzirTrue2048;
    if(dIntAzi>=(MAX_AZIR/2))      dIntAzi = dIntAzi-MAX_AZIR;
    else if(dIntAzi<(-MAX_AZIR/2)) dIntAzi = dIntAzi+MAX_AZIR;
    if(abs(dIntAzi)>10)
    {
        curAzirTrue2048 = newAziTrue;
        return;

        indexCurrRecAzi++;
        if(indexCurrRecAzi>=AZI_QUEUE_SIZE)indexCurrRecAzi=0;
        aziToProcess[indexCurrRecAzi]=curAzirTrue2048;
    }
    while (curAzirTrue2048 != newAziTrue)
    {
        if(dIntAzi>0)
        {
            curAzirTrue2048++;if(curAzirTrue2048>=MAX_AZIR)curAzirTrue2048=0;
            memcpy(&data_mem.level[curAzirTrue2048][0],data+FRAME_HEADER_SIZE,range_max);
            memcpy(&data_mem.dopler[curAzirTrue2048][0],data+FRAME_HEADER_SIZE+range_max,range_max);
            indexCurrRecAzi++;
            if(indexCurrRecAzi>=AZI_QUEUE_SIZE)indexCurrRecAzi=0;
            aziToProcess[indexCurrRecAzi]=curAzirTrue2048;
        }
        else
        {

            curAzirTrue2048--;if(curAzirTrue2048<0)        curAzirTrue2048+=MAX_AZIR;
            memcpy(&data_mem.level[curAzirTrue2048][0],data+FRAME_HEADER_SIZE,range_max);
            memcpy(&data_mem.dopler[curAzirTrue2048][0],data+FRAME_HEADER_SIZE+range_max,range_max);
            indexCurrRecAzi++;
            if(indexCurrRecAzi>=AZI_QUEUE_SIZE)indexCurrRecAzi=0;
            aziToProcess[indexCurrRecAzi]=curAzirTrue2048;
        }
    }

    //if(curAzirTrue2048==newAzi)return;

    //if(newAzi==0)dir= !dir;
    /*double dazi = newAzi-mRealAzi;
    if(dazi>MAX_AZIR/2)dazi = dazi-MAX_AZIR;else if(dazi<(-MAX_AZIR/2))dazi = dazi+MAX_AZIR;
//    if(true)
//    {
//        printf("\n newAzi:%4d dazi:%2.2f",newAzi,dazi);
//        printf("   mRealAzi:%4.2f mRealAziRate:%2.2f",mRealAzi,mRealAziRate);
//        printf("   curAzirTrue2048:%d",curAzirTrue2048);
//    }
    if((abs(dazi)>20))
    {
        mRealAziRate=0.5;
        mRealAzi=newAzi;
        init_time+=2;
        //printf("\n newAzi:%4d init_time:%d",newAzi,init_time);
    }
    else
    {
        mRealAziRate+=(dazi-mRealAziRate)/20.0;
        if(mRealAziRate>20)mRealAziRate=20;
        else if(mRealAziRate<-20)mRealAziRate=-20;
        mRealAzi+=mRealAziRate;
    }
    int intAzi=newAzi;
//    if(mRealAziRate<0)intAzi=int(mRealAzi+mInverseRotAziCorrection+0.5);
//    else intAzi=int(mRealAzi+0.5);
    if(intAzi>=MAX_AZIR){mRealAzi-=MAX_AZIR;intAzi-=MAX_AZIR;}
    else if(intAzi<=0)  {mRealAzi+=MAX_AZIR;intAzi+=MAX_AZIR;}*/
    /*
    if(curAzirTrue2048==newAzi)return;
    curAzirTrue2048 =newAzi;
    memcpy(&data_mem.level[curAzirTrue2048][0],data+FRAME_HEADER_SIZE,range_max);
    memcpy(&data_mem.dopler[curAzirTrue2048][0],data+FRAME_HEADER_SIZE+range_max,range_max);
    return;

    int diff = intAzi -curAzirTrue2048;
    if(diff>MAX_AZIR/2)diff = diff-MAX_AZIR;else if(diff<(-MAX_AZIR/2))diff = diff+MAX_AZIR;
    int nn=0;
    while (curAzirTrue2048 != intAzi)
    {
        nn++;if(nn>10)break;
        if(abs(diff)>10)curAzirTrue2048 = intAzi;
        else if(diff>0)  {curAzirTrue2048++;if(curAzirTrue2048>=MAX_AZIR)curAzirTrue2048-=MAX_AZIR;}
        else        {curAzirTrue2048--;if(curAzirTrue2048<0)        curAzirTrue2048+=MAX_AZIR;}
        memcpy(&data_mem.level[curAzirTrue2048][0],data+FRAME_HEADER_SIZE,range_max);
        memcpy(&data_mem.dopler[curAzirTrue2048][0],data+FRAME_HEADER_SIZE+range_max,range_max);
        //aziToProcess.push(curAzirTrue2048);
    }
*/
    return;

}

void C_radar_data::SelfRotationOn( double rate)
{
    isSelfRotation = true;
    printf("\nself rotation");
    SelfRotationReset();
    selfRotationDazi = 0.2;
    cur_rot_timeMSecs =0;
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

    int leftAzi = curAzirTrue2048-1;if(leftAzi<0)leftAzi+=MAX_AZIR;
    int rightAzi = curAzirTrue2048 +1; if(rightAzi>=MAX_AZIR)rightAzi-=MAX_AZIR;
    if(newAzi == leftAzi )
    {
        if(rotDir==Right)
        {
            rotDir  = Left;
            arcMaxAzi = curAzirTrue2048;
            init_time =5;

        }
    }
    else if(newAzi == rightAzi) {
        if(rotDir==Left)
        {
            rotDir = Right;
            arcMinAzi = curAzirTrue2048;
            init_time =5;
        }
    }
    else if(newAzi ==curAzirTrue2048)
    {
        //printf("\ncurAzirTrue2048:%d",curAzirTrue2048);
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
    curAzirTrue2048 = newAzi;
    aziToProcess.push(curAzirTrue2048);
    decodeData(curAzirTrue2048);
    if(!((unsigned char)(curAzirTrue2048<<3))){
        procTracks(curAzirTrue2048);
        getNoiseLevel();

    }
    if(curAzirTrue2048==0)
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

void C_radar_data::ProcessGOData(unsigned char *data, short len, int aziMH)
{
    if(len<300)return;
    for(int i=0;i<range_max;i++)
    {
        data_mem.may_hoi[aziMH][i] = (data[i/8+RADAR_HEADER_LEN]>>(i%8))&0x01;
    }
}
void C_radar_data::setAziViewOffsetDeg(double angle)
{
    //printf("\ncurAzirTrue2048:%d",curAzirTrue2048);
    aziViewOffset = (angle*MAX_AZIR)/360;
    while(aziViewOffset<0)aziViewOffset+=MAX_AZIR;
    while(aziViewOffset>=MAX_AZIR)aziViewOffset-=MAX_AZIR;
    //    raw_map_init();
}
uint processing_azi_count = 0;
bool C_radar_data::UpdateData()
{
    CConfig::time_now_ms = (QDateTime::currentMSecsSinceEpoch());

    while (indexCurrProcessAzi!=indexCurrRecAzi)
    {
        indexLastProcessAzi = indexCurrProcessAzi;
        indexCurrProcessAzi++;
        if(indexCurrProcessAzi>=AZI_QUEUE_SIZE)indexCurrProcessAzi=0;
        int azi = aziToProcess[indexCurrProcessAzi];
        int lastAzi = aziToProcess[indexLastProcessAzi];
        int dazi = azi-lastAzi;
        if(dazi==1||dazi==-2047)isInverseRotation = false;
        else if(dazi==-1||dazi==2047)isInverseRotation = true;
        else continue;
        //        clock_t clkBegin = clock();
        ProcessData(azi,lastAzi);
        drawAzi(azi);
        //        clock_t clkEnd = clock();
        //        int ProcessingTime = (clkEnd-clkBegin);
        //        if(ProcessingTime>1)
        //        {
        //            printf("\nProcessingTime:%d azi:%d",ProcessingTime,azi);
        //        }
        mUpdateTime = clock();
        processing_azi_count++;
        if(!(processing_azi_count%16))//xu ly moi 16 chu ky
        {
            //QDateTime::currentMSecsSinceEpoch();
            //ProcessObjects();
            ProcessTracks();
            if(!(processing_azi_count%64))//xu ly moi 64 chu ky
            {
                if(!(processing_azi_count%512))//xu ly moi 512 chu ky
                {
                    ProcessEach90Deg();
                    getNoiseLevel();
                }

            }
            if(init_time>5)init_time=5;
            if(init_time)
            {
                init_time--;
            }
            for(unsigned short i = 0;i<plot_list.size();++i)
            {
                if(plot_list.at(i).isUsed)
                {
                    if((plot_list.at(i).lastA!=lastAzi)&&(plot_list.at(i).lastA!=azi))
                    {
                        procPLot(&plot_list.at(i));
                        plot_list.at(i).isUsed = false;
                    }

                }
            }
        }
        // update histogram
        nNoiseFrameCount++;
        sumvar+= abs(data_mem.level[azi][range_max-300]-data_mem.level[azi][range_max-305]);;
        unsigned char value = data_mem.level[azi][range_max-300];
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

        //        aziToProcess.pop();
    }
    return true;
}

static  unsigned int doplerHistogram[256];
void C_radar_data::procPLot(plot_t* mPlot)
{
    if(init_time)
        return;
    // remove too small obj
    if(mPlot->size<3)
    {
        mFalsePositiveCount++;
        return;
    }
    float ctA,dAz,dRg = (mPlot->maxR-mPlot->minR)+1;
    float ctR = (float)mPlot->sumR/(float)mPlot->size+0.5;//min value starts at 1
    if(dRg*sn_scale>MAX_OBJ_SIZE)
    {
        //printf("\nrejected by dRg :%fkm %d %d",dRg*sn_scale,mPlot->maxR,mPlot->minR);
        return;
    }
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
    if(ctA >= MAX_AZIR)ctA -= MAX_AZIR;
    if(mTerrainAvailable)
    {
        if(data_mem.terrainMap[int(ctA)][int(ctR)]>150)
        {
            printf("\nPlot terrain rejected:%d",data_mem.terrainMap[int(ctA)][int(ctR)]);
            return;
        }
    }
    //double objSizeAz = (dAz*PI_NHAN2/MAX_AZIR)*ctR*sn_scale;
    if(dAz<2||dAz>30)
    {

        //printf("rejected by daz:%f %d %d",dAz,mPlot->lastA,mPlot->riseA);
        return;
    }
    //if(mPlot->minR<500)return;
    if(gat_mua_va_dia_vat)
    {
        int leftA = ctA-dAz/2;
        if(leftA<0)leftA+=MAX_AZIR;
        int rightA = ctA+dAz/2;
        if(rightA>=MAX_AZIR)rightA-=MAX_AZIR;
        int maxRg = ctR+dRg*2;if(maxRg>=range_max-RANGE_MIN)  maxRg=range_max-RANGE_MIN;
        int minRg = ctR-dRg*2;if(minRg<RANGE_MIN)             minRg=RANGE_MIN;
        int engCount=0;
        for(int lAzi=leftA;lAzi!=rightA;lAzi++)
        {
            if(lAzi>=MAX_AZIR)lAzi-=MAX_AZIR;
            for(int lRg = minRg;lRg<maxRg;lRg++)
            {
                if(data_mem.detect[lAzi][lRg])engCount++;
            }
        }
        if(engCount>mPlot->size*2)
        {
            printf("\nclutter rejected: %d / %d",engCount,mPlot->size);
            return;
        }
        /*memset(doplerHistogram,0,256);
        int leftA = mPlot->riseA-15;
        if(leftA<0)leftA+=MAX_AZIR;
        int rightA = mPlot->fallA+15;
        if(rightA>=MAX_AZIR)rightA-=MAX_AZIR;
        int maxRg = ctR+15;
        int minRg = ctR-15;
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
        if(maxHis/(float)sumHis<0.75)return;*/

    }

    object_t newobject;
    newobject.timeMs = CConfig::time_now_ms;
    newobject.isRemoved = false;
    newobject.dazi = dAz;
    newobject.period = CConfig::mStat.mFrameCount;

    if(ctR<mPlot->minR||ctR>mPlot->maxR+1)printf("\nWrong ctR");
    //todo: tinh dopler histogram
    newobject.size = mPlot->size;
    newobject.energy = mPlot->sumEnergy;
    newobject.drg = (mPlot->maxR-mPlot->minR)+1;
    newobject.aziStdEr = azi_er_rad;
    newobject.rgStdEr = rgStdErr+(newobject.drg*sn_scale)/2;//km
    if(ctA<0|| ctR>=RADAR_RESOLUTION)
    {
        return;
    }
    newobject.dopler = mPlot->dopler;
    //newobject.terrain = data_mem.terrain[short(ctA)][short(ctR)];
    newobject.azRad   = ctA/float(MAX_AZIR/PI_NHAN2);
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
        unsigned char value = data_mem.level[curAzirTrue2048][r_pos];
        char dopler = data_mem.dopler[curAzirTrue2048][r_pos];

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
    azi-=aziViewOffset;
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
    for(unsigned short i=0;i<MAX_TRACKS_COUNT;i++)
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
    for(unsigned short i=0;i<MAX_TRACKS_COUNT;i++)
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
void C_radar_data::procPix(short proc_azi,short lastAzi,short range)//_______signal detected, check 4 last neighbour points for nearby mark_______________//
{

    int plotIndex =-1;
    char dopler_0 = data_mem.dopler[proc_azi][range];
    //char dopler_1 = dopler_0 +1;
    //    if(dopler_1>15)dopler_1-=16;
    //    //char dopler_2 = dopler_0 - 1;
    //    if(dopler_2<0)dopler_2+=16;

    for(int dr=-max_drange_plot;dr<=max_drange_plot;dr++)//  search lastAzi
    {
        if(data_mem.detect[lastAzi][range+dr])
        {
            int dDopler = abs(data_mem.dopler[lastAzi][range+dr]-dopler_0);
            if(dDopler>7)dDopler=16-dDopler;
            if(dDopler<3)
            {
                plotIndex = data_mem.plotIndex[lastAzi][range+dr];
                break;
            }
        }
    }
    if(plotIndex<0)
    {
        for(int dr=-max_drange_plot;dr<0;dr++)//  search proc_azi
        {
            if(data_mem.detect[proc_azi][range+dr])
            {
                int dDopler = abs(data_mem.dopler[proc_azi][range+dr]-dopler_0);
                if(dDopler>7)dDopler=16-dDopler;
                if(dDopler<3)
                {
                    plotIndex = data_mem.plotIndex[proc_azi][range+dr];
                    break;
                }
            }
        }
    }
    if((plotIndex<plot_list.size())
            &&(plotIndex>=0)
            &&(plot_list.at(plotIndex).isUsed)
            )// add to existing plot
    {
        plot_t* pPlot = &(plot_list[plotIndex]);
        data_mem.plotIndex[proc_azi][range] = plotIndex;
        pPlot->size++;
        pPlot->sumEnergy+=data_mem.level[proc_azi][range];
        if(pPlot->maxR<range)pPlot->maxR = range;
        if(pPlot->minR>range)pPlot->minR = range;
        pPlot->sumR    +=  range;
        pPlot->lastA   = proc_azi;
        //
        // get max dopler and max level of this plot
        if(pPlot->maxLevel<data_mem.level[proc_azi][range])
        {
            pPlot->riseA = proc_azi;
            pPlot->maxLevel = data_mem.level[proc_azi][range];
            pPlot->dopler = data_mem.dopler[proc_azi][range];
        }
        else
        {
            data_mem.dopler[proc_azi][range] = pPlot->dopler;
        }
        if((pPlot->maxLevel-noiseVar)<data_mem.level[proc_azi][range])
            pPlot->fallA = proc_azi;
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

//    hsTap += ((tb_tap[curAzirTrue2048])-hsTap)/5.0;
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
    else cta = atan(ctx/cty);
    if(cty<0)cta+=PI;
    if(cta<0)cta += PI_NHAN2;
    if(cta>PI_NHAN2)cta-=PI_NHAN2;
    cta=cta/PI_NHAN2*MAX_AZIR-aziViewOffset;
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
    img_zoom_ar = new QImage(zoom_ar_size_a+1,zoom_ar_size_r+1,QImage::Format_ARGB32);
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
    img_zoom_ar->setPixel(pa,zoom_ar_size_r-pr,getColor(val,dopler,sled));
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
    cut_noise = aut;
    CConfig::setValue("cut_noise",int(cut_noise));
}
void C_radar_data::raw_map_init()
{
    double theta=0;
    //    printf("\naziViewOffset:%d",aziViewOffsetRad);
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
    double theta=0;
    double dTheta = 2*PI/MAX_AZIR_DRAW;
    for(short azir = 0; azir < MAX_AZIR_DRAW; azir++)
    {

        double cost = cos(theta);
        double sint = sin(theta);
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
    rgStdErr = SIGNAL_SCALE_0*pow(2,clk_adc);
    // decode byte clock ADC
    switch(clk_adc)
    {
    case 0:
        sn_scale = SIGNAL_SCALE_0;//15m
        break;
    case 1:
        sn_scale = SIGNAL_SCALE_1;//30m
        break;
    case 2:
        sn_scale = SIGNAL_SCALE_2;//60m
        break;
    case 3:
        sn_scale = SIGNAL_SCALE_3;//120m
        break;
    case 4:
        sn_scale = SIGNAL_SCALE_4;//240m
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
    max_drange_plot = MAX_OBJ_SIZE/sn_scale/2;
    if(max_drange_plot<1)max_drange_plot=1;
    if(max_drange_plot>=10)max_drange_plot=10;
    int dataLen = RADAR_RESOLUTION*MAX_AZIR;
    memset(data_mem.level,      0,dataLen);
    memset(data_mem.dopler,     0,dataLen);
    memset(data_mem.detect,     0,dataLen);
    memset(data_mem.plotIndex,  0,dataLen);
    memset(data_mem.hot,        0,dataLen);
    memset(data_mem.may_hoi,    0,dataLen);
    memset(data_mem.sled,       0,dataLen);
    init_time = 5;

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
    if((px<pSize)||(py<pSize)||(px>=ZOOM_SIZE-pSize)||(py>=ZOOM_SIZE-pSize))return;
    for(short x = -pSize;x <= pSize;x++)
    {
        for(short y = -pSize;y <= pSize;y++)
        {
            double k =1.0/(sqrt(x*x+y*y)/8.0+1.0);

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
    unsigned short value = ((unsigned short)pvalue)*brightness;
    if(!isShowSled)sled = 0;
    else
        if(sled>=128)sled = 0xff; else sled*=2;
    if(value>0xff)
    {
        value = 0xff;
    }
    unsigned char alpha = 0xff - ((0xff - value)*0.75);;
    unsigned char red   = 0;
    unsigned char green = 0;
    unsigned char blue  = 0;
    unsigned char gradation = value<<2;
    uint color;
    if((dopler&0xF0))
    {
        color = 0xffffff|(alpha<<24);
    }
    else
    {
        dopler&=0x0F;
        switch(imgMode)
        {
        case DOPLER_3_COLOR:
            if(pvalue>1)
            {
                if(dopler==0)
                {
                    color = 0xffff00;
                }
                else
                {
                    char dDopler = dopler-1;
                    if(dDopler>7)dDopler = 15-dDopler;
                    color = 0x00ff00 | ((dDopler<<5));
                }
                //alpha = value;//0xff - ((0xff - value)*0.75);
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
                //alpha = value;//0xff - ((0xff - pvalue)*0.75);
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
    }
    return color;
}
void C_radar_data::resetTrack()
{
    init_time += 3;
    C_primary_track::IDCounter=1;
    for(int i=0;i<MAX_TRACKS_COUNT;i++)
    {
        C_primary_track* track=&(mTrackList[i]);
        track->mState = TrackState::removed;

    }
    for(int i=0;i<mFreeObjList.size();i++)
    {
        mFreeObjList[i].isRemoved=true;
    }

}

void C_radar_data::ProcessTracks()
{
    //processTracks
    for (ushort j=0;j<MAX_TRACKS_COUNT;j++)
    {
        C_primary_track* track = &(mTrackList[j]);
        if(track->mState==TrackState::removed)return;
        track->update();

    }
}
void C_radar_data::addFreeObj(object_t *obj1)
{
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
}
bool C_radar_data::checkSimilarityToExistingTracks(object_t *obj1)
{

    return false;
}
void C_radar_data::UpdateTrackStatistic()
{
    int nobj=0;
    double sumSize=0;
    double sumDazi=0;
    double sumDrg=0;
    double sumEng=0;
    for (ushort j=0;j<MAX_TRACKS_COUNT;j++)
    {
        C_primary_track* track = &(mTrackList[j]);
        if(!track->isConfirmed())continue;
        for(int i=0;i<track->objectList.size();i++)
        {
            object_t* obj = &(track->objectList[i]);
            nobj++;
            sumSize+=obj->size;
            sumDazi+=obj->dazi;
            sumDrg+=obj->drg;
            sumEng+=obj->energy;
            //obj->
        }
    }
    sumSize/=nobj;
    sumDazi/=nobj;
    sumDrg /=nobj;
    sumEng /=nobj;
    if(nobj>20)
    {
        printf("\ntracks statistic: sumSize:%f sumDazi:%f sumDrg:%f sumEng:%f",sumSize,sumDazi,sumDrg,sumEng);
    }
}
#define MAX_TERAIN 300
void C_radar_data::updateTerrain()
{
    maxTer=0;

    for(short azi=0;azi<MAX_AZIR;azi++)
    {
        for(short r_pos=0;r_pos<RADAR_RESOLUTION;r_pos++)
        {
            //int offset = azi*RADAR_RESOLUTION+r_pos
            if((data_mem.dopler[azi][r_pos]==0)&&data_mem.detect[azi][r_pos])
            {
                data_mem.terrainMap[azi][r_pos]++;
                if(maxTer<data_mem.terrainMap[azi][r_pos])maxTer=data_mem.terrainMap[azi][r_pos];
            }
        }
    }
    if((maxTer/MAX_TERAIN)>1)
    {
        int k=maxTer/MAX_TERAIN;
        for(short azi=0;azi<MAX_AZIR;azi++)
        {
            for(short r_pos=0;r_pos<RADAR_RESOLUTION;r_pos++)
            {
                data_mem.terrainMap[azi][r_pos]/=k;
            }
        }
    }
    mTerrainAvailable = maxTer>MAX_TERAIN;
    CConfig::AddMessage(QString::fromUtf8("Địa vật:")+QString::number(maxTer));
    saveTerrain();
}

void C_radar_data::saveTerrain()
{
    QFile terrainFile("d:\\HR2D\\terrain.dat");
    terrainFile.open(QIODevice::WriteOnly);
    if(terrainFile.isOpen())
    {
        terrainFile.write((char*)(&(data_mem.terrainMap[0][0])),(RADAR_RESOLUTION*MAX_AZIR*4));
        CConfig::AddMessage("Terrain saved.");
    }
    else
    {
        printf("\nCan't save terrain");
    }
    terrainFile.close();
}

void C_radar_data::loadTerrain()
{
    QFile terrainFile("d:\\HR2D\\terrain.dat");
    terrainFile.open(QIODevice::ReadOnly);
    if(terrainFile.size()==(RADAR_RESOLUTION*MAX_AZIR*4))
    {
        terrainFile.read((char*)(&(data_mem.terrainMap[0][0])),(RADAR_RESOLUTION*MAX_AZIR*4));
        CConfig::AddMessage("Terrain loaded.");
    }
    else
    {
        CConfig::AddMessage("Terrain file error, reset terrain.");
        memset((char*)(&data_mem.terrainMap[0][0]),0,(RADAR_RESOLUTION*MAX_AZIR*4));
    }
    terrainFile.close();

}
void C_radar_data::ProcessObject(object_t *obj1)
{
    //check  if object_t belonging to tracks
    if(MAX_TRACKS_COUNT)
        if(checkBelongToTrack(obj1))
            return ;
    // check if object_t belonging to another obj
    if(checkBelongToObj(obj1))return ;
    // add to mFreeObjList if inside DW
    if(checkInsideDWOneTime(degrees(obj1->azRad),obj1->rgKm)){addFreeObj(obj1); return ;}
    if(checkInsideDW(degrees(obj1->azRad),obj1->rgKm))
    {
        if(obj1->dopler!=0)
        {
            addFreeObj(obj1);
            return;
        }
        if(checkSimilarityToExistingTracks(obj1))
        {
            addFreeObj(obj1);
            return;
        }
    }

    return ;
}

bool C_radar_data::checkBelongToTrack(object_t *obj1)
{
    bool isBelongingToTrack = false;
    C_primary_track* chosenTrack =nullptr;
    double maxScore=0.2;
    for (ushort j=0;j<MAX_TRACKS_COUNT;j++)
    {
        C_primary_track* track = &(mTrackList[j]);

        if(track->mState==TrackState::removed||
                track->mState==TrackState::lost)continue;

        if(track->isUpdating)
        {
#ifdef DEBUGMODE
            printf("\n warning:track busy");
#endif
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

        //-----------------------------------

        if(score>maxScore&&score>track->possibleMaxScore)
        {

            if(track->mState==TrackState::confirmed)
            {
                maxScore=score;
                chosenTrack = track;
                isBelongingToTrack = true;
            }
            else if(chosenTrack)
            {
                if(chosenTrack->mState==TrackState::confirmed)continue;//
                maxScore=score;
                chosenTrack = track;
                isBelongingToTrack = true;

            }
            else
            {
                maxScore=score;
                chosenTrack = track;
                isBelongingToTrack = true;
            }
            if((!isBelongingToTrack)&&(track->mDopler==obj1->dopler))
            {
                score=score;
            }

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
bool C_radar_data::checkInsideDWOneTime(double aziDeg,double rgkm)
{
    for(uint i=0;i<mDetectZonesList.size();i++)
    {
        DetectionWindow *dw = &mDetectZonesList[i];
        if(dw->isRemoved)continue;
        if(dw->isOneTime)
        {
            if((CConfig::time_now_ms-dw->timeStart>TRACK_DELETE_TIME)){dw->isRemoved=true;continue;}
            if((abs(aziDeg-dw->aziDeg))<dw->maxDazDeg
                    &&(abs(rgkm-dw->rg)<dw->maxDrg)
                    )
            {
                dw->isRemoved=true;
                return true;
            }
        }
    }
    return false;
}

bool C_radar_data::checkInsideDW(double aziDeg,double rgkm)
{
    for(uint i=0;i<mDetectZonesList.size();i++)
    {
        DetectionWindow *dw = &mDetectZonesList[i];
        if(dw->isRemoved)continue;
        if(!dw->isOneTime)
        {
            double daz = abs(aziDeg-dw->aziDeg);
            if(daz>180)daz=360-daz;
            if(daz<dw->maxDazDeg
                    &&(abs(rgkm-dw->rg)<dw->maxDrg)
                    )
            {
                return true;
            }
        }
    }
    return false;
}
void C_radar_data::CreateTrack(object_t* obj1,object_t* obj2)
{
    for (ushort j=0;j<MAX_TRACKS_COUNT;j++)
    {
        if(mTrackList[j].mState==TrackState::removed)
        {
            //            track_t newTrack(obj1,obj2);
            mTrackList[j].init(obj1,obj2);
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
        double score = C_primary_track::estimateScore(obj1,obj2);
        if(score<=0)continue;
        if(score>maxScore)
        {
            maxScore = score;
            objLast = obj2;
        }
    }
    if(maxScore>0.05)//todo: find best value
    {
        //obj1->scorepObj = maxScore;
        //obj1->isRemoved=true;
        //objLast->isRemoved = true;
        CreateTrack(obj1,objLast);
        objLast->isRemoved = true;
        //obj1->isRemoved =true;
        return true;
    }
    else return false;

}

