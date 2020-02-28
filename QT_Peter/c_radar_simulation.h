#ifndef C_RADAR_SIMULATION_H
#define C_RADAR_SIMULATION_H

#include "C_radar_data.h"

#include <QThread>
#include <QTimer>
#include <QUdpSocket>
#include <random>
#include <time.h>
#define FRAME_HEADER_SIZE 34
#define FRAME_LEN 2048
#define OUTPUT_FRAME_SIZE FRAME_LEN*2+FRAME_HEADER_SIZE
#define PI_NHAN2                    6.2831853072
#define PI_CHIA2                    1.5707963268
#define PI                       3.14159265358979323846
#define DEG_RAD 57.295779513
#define MAX_AZI 2048
#define NUM_OF_TARG 50
using namespace std;

extern unsigned char outputFrame[MAX_AZI][OUTPUT_FRAME_SIZE];
extern double rResolution;
class sim_target_t
{
public:

    sim_target_t();
    void init();
    void init(double tx, double ty, double tspeedKmh, double tbearing, int dople =5);
    void generateSignal();
    void eraseSIgnal();
    void update();
    ~sim_target_t()
    {}
    bool getIsManeuver() const;
    void setIsManeuver(bool value);

    bool getEnabled() const;
    void setEnabled(bool value);
    double  x, y,mlat,mlon;
private:

    bool enabled;
    double speedKmh, azi, range;
    double bearing,rot;
    int dopler;
    int targetSize;
    unsigned char nUpdates;
    time_t  timeLast;
};


class c_radar_simulation:public QThread
{
    Q_OBJECT
public:
    std::vector<sim_target_t> targetList;
    c_radar_simulation(C_radar_data* radarData);
    void play(bool isSig);
    void pause();
    void run();
    void setTarget(int id, double aziDeg, double rangeKm, double tbearingDeg, double tspeed, int dople =0, int tlostRate=0);
    void setRange(int clk_adc);
    C_radar_data* mRadarData;
    bool getIsPlaying() const;
    void setIsManeuver(bool checked);
    void setLostRate(int rate);
    void setAllTarget();

    void setAirTarget(int id, double lat, double lon, double tspeedKm, double tbearingDeg);
private:
    int azi;

//    QTimer dataSendTimer;
    void updateTargets();
    void socketInit();
    QUdpSocket      *radarSocket;
    bool isPlaying;
    void initTargets();
};

#endif // C_RADAR_SIMULATION_H
