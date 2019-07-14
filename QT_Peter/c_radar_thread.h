#ifndef DATAPROCESSINGTHREAD_H
#define DATAPROCESSINGTHREAD_H
#include <QThread>
#include <queue>
#include <QTimer>
//#include <QGeoPositionInfo>
#include "c_config.h"
#include "C_radar_data.h"
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
//struct GPSData
//{
//    double lat,lon;
//    double course,speed;
//    bool isFixed;
//};
struct DataBuff// buffer for data frame
{
    short len;
    unsigned char data[MAX_FRAME_SIZE];
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
    bool isSimulationMode;
    int mCudaAge200ms;
    QFile aisLogFile;
//    std::queue<GPSData> mGpsData;
//    unsigned char       connect_timeout;
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
    double mFramesPerSec;
    void PlaybackFile();
    void startRecord(QString fileName);
    void stopRecord();
    void stopThread();
    void radRequestTemp(char index);
    void radTxOn();
    void radTxOff();
//    void setVaru(bool isOn);
//    void setSharu(bool isOn);
//    void setBaru(bool isOn);
    void sendCommand(unsigned char* commandBuff, short len=8, bool queued = true);
    void loadRecordDataFile(QString fileName);
    void togglePlayPause(bool play);

    C_radar_data* mRadarData;
//    C_ARPA_data* arpaData;
    void run();
    bool getIsDrawn();
    AIS aisMessageHandler;
    QList<AIS_object_t> m_aisList;


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
    QString messageStringbuffer;
    void CalculateRFR();
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
//    QFile signTTMFile;
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
    void sendRATTM();
    bool readGyroMsg(unsigned char *mReceiveBuff, int len);
    bool readNmea(unsigned char *mReceiveBuff,int len);
    bool readMay22Msg(unsigned char *mReceiveBuff, int len);
    void ProcessData(unsigned char *data, unsigned short len);
private slots:
    void ReadDataBuffer();
    void Timer200ms();
//    void processRadarData();
    void inputAISData(QByteArray inputdata);
    void playbackRadarData();
    void SerialDataRead();
//    void gpsupdate(QGeoPositionInfo geo);

    void ReadNavData();
public slots:
    void StopProcessing();

};

#endif // DATAPROCESSINGTHREAD_H
