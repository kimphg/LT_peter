#include "c_radar_simulation.h"
static std::default_random_engine generator;
static std::normal_distribution<double> distribNoise(30, 8);
static std::normal_distribution<double> distribAzi(0, 0.01);//radian
static std::normal_distribution<double> distribRot(180, 0.5 );//deg per sec
static unsigned char n_clk_adc = 0;
bool isManeuver=false, isWithSignal=false;
double rResolution = 0.015070644;
unsigned char outputFrame[MAX_AZI][OUTPUT_FRAME_SIZE];
double ConvXYToR(double x, double y)
{
    return sqrt(x*x + y*y);

}
int lostRate =0;
double ConvXYToAziRad(double x, double y)
{
    double azi;
    if (y==0)
    {
        azi = x>0 ? PI_CHIA2 : (PI_NHAN2 - PI_CHIA2);

    }
    else
    {
        azi = atan(x / y);
        if (y<0)azi += PI;
        if (azi<0)azi += PI_NHAN2;
    }
    return azi;
}
void regenerate(int azi)
{
    unsigned char* dataPointer = &outputFrame[azi][0] + FRAME_HEADER_SIZE;
    for (int i = 0; i < FRAME_LEN; i++)
    {
        int num = int(distribNoise(generator));
        if (num < 0)num = 0;
        dataPointer[i] = num;
    }
    for (int i = FRAME_LEN; i < FRAME_LEN * 2; i++)
    {
        dataPointer[i] = rand()%16;
    }
}

sim_target_t::sim_target_t()
{
    enabled = false;

}

void sim_target_t::init()
{
    enabled = true;
    speedKmh = rand()%50+5;
    x = (rand()%80)*((rand()%2)*2-1);
    y = (rand()%80)*((rand()%2)*2-1);
    CConfig::ConvKmToWGS(x,y,&mlat,&mlon);
    mHeading = radians(rand()%360);
    azi = ConvXYToAziRad(x, y) / 3.141592653589*1024.0;
    range = ConvXYToR(x, y);
    dopler = 0;
    targetSize = 10;
    nUpdates = 0;
    timeLast = time(nullptr);
    rot = 0;
    malt=0;
}

void sim_target_t::init(double tx, double ty, double tspeedKmh, double tbearing, int dople, double talt)
{
//    lostRate = tlostRate;
    enabled = true;
    speedKmh = tspeedKmh;
    x = tx;
    y = ty;
    malt = talt;
    CConfig::ConvKmToWGS(tx,ty,&mlon,&mlat);
    mHeading = radians(tbearing);
    azi = ConvXYToAziRad(x, y) / 3.141592653589*1024.0;
    range = ConvXYToR(x, y);
    dopler = dople;
    targetSize = 10;
    nUpdates = 0;
    timeLast = time(nullptr);
    rot = 0;

}
void sim_target_t::generateSignal()
{
    if (range >= FRAME_LEN - 1)return;
    //if (rand() % 5)return;
    int azimin = azi - targetSize; if (azimin < 0)azimin += 2048;
    int azimax = azi + targetSize; if (azimax >= 2048)azimax -= 2048;
    int k = 0;
    for (int a = azimin; ; a++)
    {
        if (a < 0)a += 2048;
        if (a >= 2048) a -= 2048;
        if (a == azimax)break;
        int value = 150 * (1.0 - abs(k - targetSize) / (targetSize + 1.0));
        outputFrame[a][(int)range + FRAME_LEN + FRAME_HEADER_SIZE] = dopler;
        outputFrame[a][(int)range + FRAME_LEN + 1 + FRAME_HEADER_SIZE] = dopler;
        outputFrame[a][(int)range + FRAME_HEADER_SIZE] = value + int(distribNoise(generator));
        outputFrame[a][(int)range + 1 + FRAME_HEADER_SIZE] = value + int(distribNoise(generator));

        k++;
    }
}

void sim_target_t::eraseSIgnal()
{
    return;
    if (range >= FRAME_LEN - 1)return;
    int azimax = azi + targetSize; if (azimax >= 2048)azimax -= 2048;
    int azimin = azi - targetSize; if (azimin < 0)azimin += 2048;
    for (int a = azimin; ; a++)
    {
        if (a < 0)a += 2048;
        if (a>=2048) a -= 2048;
        if (a == azimax)break;
        int num = int(distribNoise(generator));
        if (num < 0)num = 0;
        outputFrame[a][(int)range + FRAME_HEADER_SIZE] = num;
        outputFrame[a][(int)range + FRAME_LEN + FRAME_HEADER_SIZE] =  rand() % 16;
        num = int(distribNoise(generator));
        if (num < 0)num = 0;
        outputFrame[a][(int)range + 1 + FRAME_HEADER_SIZE] = num;
        outputFrame[a][(int)range + FRAME_LEN + 1 + FRAME_HEADER_SIZE] = rand() % 16;
    }
}

void sim_target_t::update()
{
    if(!enabled)return;
    //recalculate coodinates
    time_t timenow = time(nullptr);
    double elapsed_secs = difftime(timenow, timeLast);
    timeLast = timenow;
    nUpdates++;
    if(isManeuver)
    {
        if (nUpdates%5==0)
        {
            if(isManeuver)rot = distribRot(generator) / DEG_RAD - PI;

        }
        mHeading += rot*elapsed_secs;
    }
    double distance = elapsed_secs / 3600.0*speedKmh;
    x += distance*sin(mHeading);
    y += distance*cos(mHeading);
    CConfig::ConvKmToWGS(x,y,&mlon,&mlat);
    //
    azi = (ConvXYToAziRad(x, y) + distribAzi(generator))/ 3.141592653589*1024.0;
    range	= ConvXYToR(x, y) / rResolution;
    if(isWithSignal)if(rand()%100>lostRate)generateSignal();
}

bool sim_target_t::getIsManeuver() const
{
    return isManeuver;
}

void sim_target_t::setIsManeuver(bool value)
{
    isManeuver = value;
}

bool sim_target_t::getEnabled() const
{
    return enabled;
}

void sim_target_t::setEnabled(bool value)
{
//    eraseSIgnal();
    enabled = value;
}
void c_radar_simulation::initTargets()
{
    //target[0] = new target_t(250, -300, 4, 100,4);
    for (int i = 0; i < NUM_OF_TARG; i++)
    {
        sim_target_t t;
        targetList.push_back(t);
    }

}

void c_radar_simulation::updateTargets()
{
    for (uint i = 0; i < targetList.size(); i++)
    {
        targetList[i].update();
    }
}
void c_radar_simulation::socketInit()
{
    radarSocket = new QUdpSocket(this);
}

bool c_radar_simulation::getIsPlaying() const
{
    return isPlaying;
}

void c_radar_simulation::setIsManeuver(bool checked)
{
    isManeuver = checked;
}
void c_radar_simulation::setLostRate(int rate)
{
    lostRate = rate%100;
}
c_radar_simulation::c_radar_simulation(C_radar_data *radarData)//QObject *parent)
{
    socketInit();
    int port = CConfig::getInt("simSocketPort",32000);
    while(port<31100)
    {
        if(radarSocket->bind(port))
        {
            break;
        }
        port++;
    }
    isManeuver = false;
    for (int i = 0; i < MAX_AZI; i++)
    {
        regenerate(i);
        outputFrame[i][2] = i >> 8;
        outputFrame[i][3] = i;
    }
    for (int i = 0; i < NUM_OF_TARG; i++)
    {
        sim_target_t t;
        targetList.push_back(t);
    }
    setRange(2);
    //socketInit();
    mRadarData = radarData;
    isPlaying = false;
    //connect(&dataSendTimer, &QTimer::timeout, this, &c_radar_simulation::sendData);
    //dataSendTimer.start(50);
    azi = 200;
}

void c_radar_simulation::play(bool isSig)
{
    isPlaying = true;
    isWithSignal = isSig;
}

void c_radar_simulation::pause()
{
    isPlaying = false;
}
void c_radar_simulation::setTarget(int id,double aziDeg, double rangeKm,  double tbearingDeg,double tspeedKn, int dople,int tlostRate)
{
    //target_t newTarget(tx,ty,tspeed,tbearing,dople);
    if(id>=targetList.size())id=id%targetList.size();
    double tx,ty;
    tx = rangeKm*CONST_NM*sin(radians(aziDeg));
    ty = rangeKm*CONST_NM*cos(radians(aziDeg));
    lostRate = tlostRate%100;
    targetList[id].init(tx,ty,tspeedKn*CONST_NM,tbearingDeg,dople);//(double tx, double ty, double tspeedKmh, double tbearing, int dople)
}
void c_radar_simulation::RemoveAllTargets()
{
    for(sim_target_t plane:targetList)
    {
        plane.setEnabled(false);
    }
}
void c_radar_simulation::setAirTarget(int id,double lat,double lon,double alt, double tspeedKm,double tbearingDeg)
{
    //target_t newTarget(tx,ty,tspeed,tbearing,dople);
    if(id>=targetList.size())id=id%targetList.size();
    double tx,ty;
    CConfig::ConvWGSToKm(&tx,&ty,lon,lat);
    targetList[id].init(tx,ty,tspeedKm,tbearingDeg,5,alt);//(double tx, double ty, double tspeedKmh, double tbearing, int dople)
}
void c_radar_simulation::setAllTarget()
{
    for(int i =0;i<targetList.size();i++)
    {
        targetList[i].init();
    }
}
void c_radar_simulation::setRange(int clk_adc)
{
    //15 30 60 120 240
    int nclk_adc = clk_adc-3;
    if(nclk_adc<0)nclk_adc=0;
    if(nclk_adc>3)nclk_adc=3;
    n_clk_adc = nclk_adc;
    rResolution = 0.015070644 * pow(2, n_clk_adc);
    updateTargets();
}

void c_radar_simulation::run()
{
    while (true)
    {
        msleep(50);
        if(!isPlaying)
        {
            msleep(500);
            continue;
        }
        if(isWithSignal)
        {
            int a=0;
            while(true)
            {
                if(a++>10)break;
                azi += 1;
                if (azi >= 2048)
                {
                    //nPeriod++;
                    //if (nPeriod > 50)nPeriod = 0;
                    azi = 0;
                    updateTargets();
                }
                outputFrame[azi][0] = 0x55;
                outputFrame[azi][2] = (azi >> 8);
                outputFrame[azi][3] = (azi);
                outputFrame[azi][4] = n_clk_adc;
                mRadarData->processSocketData((unsigned char*)(&outputFrame[azi][0]),OUTPUT_FRAME_SIZE);
                regenerate(azi);
            }
        }
        else
        {
            updateTargets();
            for(sim_target_t obj:targetList)
            {
                if(obj.getEnabled())
                {
                    QString sentence = "$RATIF_PLOT,"+
                            QString::number(clock()) +","+
                            +"_radar_plot,"+
                            QString::number(obj.mlat, 'f',5) +","+
                            QString::number(obj.mlon, 'f',5) +","+
                            +"0.0,"+
                            +"0.0,"+
                            +"0.0,"+
                            +"air,"
                            +"radar,"+
                            QString::number(CConfig::time_now_ms)+"*\r\n";
                    radarSocket->writeDatagram(sentence.toUtf8(),
                                               QHostAddress(CConfig::getString("OutputIP","192.168.0.80")),
                                               CConfig::getInt("TargetOutputPort3",30003)
                                               );
                }
            }
            msleep(CConfig::getInt("simTargetUpdatePeriod",10000));
        }
    }
}

