#include "c_radar_simulation.h"
static std::default_random_engine generator;
static std::normal_distribution<double> distribNoise(30, 8);
static std::normal_distribution<double> distribAzi(0, 1);//scale of 2048
static std::normal_distribution<double> distribRot(180, 0.5 );//deg per sec
static unsigned char n_clk_adc = 0;
double rResolution = 0.015070644;
unsigned char outputFrame[MAX_AZI][OUTPUT_FRAME_SIZE];
double ConvXYToR(double x, double y)
{
    return sqrt(x*x + y*y);

}
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
target_t::target_t()
{
    enabled = false;
    isManeuver = false;
}

void target_t::init(double tx, double ty, double tspeed, double tbearing, int dople)
{
    enabled = true;
    speed = tspeed;
    x = tx;
    y = ty;
    bearing = radians(tbearing);
    azi = ConvXYToAziRad(x, y) / 3.141592653589*1024.0;
    range = ConvXYToR(x, y);
    dopler = dople;
    targetSize = 10;
    nUpdates = 0;
    timeLast = time(nullptr);
    rot = 0;

}
void target_t::generateSignal()
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

void target_t::eraseSIgnal()
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

void target_t::update()
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
        bearing += rot*elapsed_secs;
    }
    double distance = elapsed_secs / 3600.0*speed;
    x += distance*sin(bearing);
    y += distance*cos(bearing);
    //
    azi = ConvXYToAziRad(x, y) / 3.141592653589*1024.0;// +(distribAzi(generator));
    range	= ConvXYToR(x, y) / rResolution;
    generateSignal();
}

bool target_t::getIsManeuver() const
{
    return isManeuver;
}

void target_t::setIsManeuver(bool value)
{
    isManeuver = value;
}

bool target_t::getEnabled() const
{
    return enabled;
}

void target_t::setEnabled(bool value)
{
//    eraseSIgnal();
    enabled = value;
}
void c_radar_simulation::initTargets()
{
    //target[0] = new target_t(250, -300, 4, 100,4);
    for (int i = 0; i < NUM_OF_TARG; i++)
    {
        target_t t;
        target.push_back(t);
    }

}

void c_radar_simulation::updateTargets()
{
    for (uint i = 0; i < target.size(); i++)
    {
        target[i].update();
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

c_radar_simulation::c_radar_simulation(C_radar_data *radarData)//QObject *parent)
{
    for (int i = 0; i < MAX_AZI; i++)
    {
        regenerate(i);
        outputFrame[i][2] = i >> 8;
        outputFrame[i][3] = i;
    }
    for (int i = 0; i < NUM_OF_TARG; i++)
    {
        target_t t;
        target.push_back(t);
    }
    setRange(2);
    //socketInit();
    mRadarData = radarData;
    isPlaying = false;
    //connect(&dataSendTimer, &QTimer::timeout, this, &c_radar_simulation::sendData);
    //dataSendTimer.start(50);
    azi = 200;
}

void c_radar_simulation::play()
{
    isPlaying = true;
}

void c_radar_simulation::pause()
{
    isPlaying = false;
}
void c_radar_simulation::setTarget(int id,double aziDeg, double rangeKm,  double tbearingDeg,double tspeed, int dople)
{
    //target_t newTarget(tx,ty,tspeed,tbearing,dople);
    if(id>=target.size())return;
    double tx,ty;
    tx = rangeKm*CONST_NM*sin(radians(aziDeg));
    ty = rangeKm*CONST_NM*cos(radians(aziDeg));
    target[id].init(tx,ty,tspeed*CONST_NM,tbearingDeg,dople);
}
void c_radar_simulation::setRange(int clk_adc)
{
    //15 30 60 120 240
    int nclk_adc = clk_adc-3;
    if(nclk_adc<0)nclk_adc=0;
    if(nclk_adc>4)nclk_adc=4;
    n_clk_adc = nclk_adc;
    rResolution = 0.015070644 * pow(2, n_clk_adc);
    updateTargets();
}
void c_radar_simulation::sendData()
{
    if(!isPlaying)
    {
        return;
    }
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
    }
}
void c_radar_simulation::run()
{
    while (true)
    {
        msleep(75);
        if(!isPlaying)
        {
            msleep(500);
            continue;
        }
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
}

