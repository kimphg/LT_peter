#ifndef DATAPROCESSINGTHREAD_H
#define DATAPROCESSINGTHREAD_H
#include <QThread>
#include <queue>
#include <QTimer>
#include <QGeoPositionInfo>
#include "c_config.h"
#include "c_radar_data.h"
//#include "c_arpa_data.h"

#include "c_gps.h"
#include "AIS/AIS.h"
//#include "c_gps_parser.h"
#include <vector>
#include <QFile>
#include <QUdpSocket>
#include <QStringList>
#include <QtSerialPort/QSerialPort>
#include <QSerialPortInfo>
#define MAX_COMMAND_QUEUE_SIZE 100
//#define HAVE_REMOTE// for pcap
//#include "pcap.h"

#ifndef CONST_NM
#define CONST_NM 1.852
#endif
#define HR2D_UDP_PORT 5000
using namespace std;
struct GPSData
{
    double lat,lon;
    double heading,speed;
    long long timeStamp;
    bool isFixed;
};
struct DataBuff// buffer for data frame
{
    short len;
    unsigned char data[MAX_FRAME_SIZE];
};

class radarStatus_3C
{
public:
    radarStatus_3C();
    ~radarStatus_3C();
    void ReadStatusMessage(uchar* mes)
    {
        mTaiAngTen = mes[0];
        mSuyGiam = mes[1];
        mMayPhatOK = mes[2];
        mCaoApReady = mes[3];
        mCaoApKetNoi = mes[4];
        isStatChange = true;
    }
    int     mCheDoDK;
    bool    isStatChange ;
    int     mCaoApReady;
    int     mCaoApKetNoi;
    bool    mTaiAngTen;
    int     mSuyGiam;
    int     mMaHieu;
    bool    mMayPhatOK;
    bool isStatChanged()
    {
        if(isStatChange)
        {
            isStatChange = false;
            return true;
        }
        else return false;
    }
};
struct  RadarCommand// radar control commmand
{
    unsigned char bytes[8];
};
enum RadarSignMode { ModeSimpleSignal=0, ModeComplexSignal=1, ModeContinuos=2};
//extern class CGPS{};
typedef std::queue<RadarCommand> RadarCommandQueue;
class dataProcessingThread:public QThread
{
    Q_OBJECT
public:
    std::queue<GPSData> mGpsData;
    unsigned char       connect_timeout;
    RadarSignMode       mRadMode;
    unsigned short      playRate;
    DataBuff*   dataBuff;
    c_gps mGPS;
    float   k_vet;
    void SetRadarPort( unsigned short portNumber);
    void SetARPAPort( unsigned short portNumber);
    ~dataProcessingThread();
    dataProcessingThread();
    QTimer commandSendTimer;
    QTimer readUdpBuffTimer;
    QTimer readSerialTimer;
    radarStatus_3C mRadarStat;
    void PlaybackFile();
    void startRecord(QString fileName);
    void stopRecord();
    void stopThread();
    void radRequestTemp(char index);
    void radTxOn();
    void radTxOff();
    void sendCommand(unsigned char* commandBuff, short len, bool queued = true);
    void loadRecordDataFile(QString fileName);
    void togglePlayPause(bool play);

    C_radar_data* mRadarData;
//    C_ARPA_data* arpaData;
    void run();
    bool getIsDrawn();
    AIS aisMessageHandler;
    QList<AIS_object_t> m_aisList;
    bool isConnected()
    {
        return bool(connect_timeout);
    }
    void setIsDrawn(bool value);

    void setRotationSpeed(int index);
    bool getIsXuLyThuCap() const;
    void setIsXuLyThuCap(bool value);

    double getSelsynAzi() const;
    double newAzi;
    unsigned int mazi;
    unsigned int realazi1,realazi2;
    bool   isHeadingAvaible;
    double          mAntennaAzi,mAntennaAziOld;
signals:
    void HeadingDataReceived(double heading);
private:

    QSerialPort     mEncoderPort;
    double          mHeading ;

    unsigned char   failureCount;
//    bool  isDrawn;
    bool isXuLyThuCap;
    RadarCommandQueue radarComQ;
    bool isRecording;
    bool isPlaying;
    QFile signRepFile;
    QFile signRecFile;
    std::vector<QSerialPort*>     serialPorts;
    QUdpSocket      *radarSocket;
    QUdpSocket      *navSocket;
    QUdpSocket      *ARPADataSocket;
    double selsynEncoderAzi;
//    void listenToRadar();
    void initSerialComm();
    void processSerialData(QByteArray inputData);
    //    bool ProcDataAIS(BYTE *szBuff, int nLeng);
    bool checkFeedback();
    void ProcessNavData(unsigned char *mReceiveBuff, int len);
    void sendAziData();
private slots:
    void ReadDataBuffer();
    void PushCommandQueue();
    void processRadarData();
    void processARPAData(QByteArray inputdata);
    void playbackRadarData();
    void SerialDataRead();
//    void gpsupdate(QGeoPositionInfo geo);

    void ReadNavData();
public slots:
    void StopProcessing();
};

#endif // DATAPROCESSINGTHREAD_H
