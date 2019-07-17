#include <QCoreApplication>
#include <QDir>
#include <../QT_Peter/AIS/AIS.h>
#include <iostream>
#define FRAME_LEN_NAV 1500
using namespace std;
typedef std::pair<int,int> LatLon1000;
typedef std::map<LatLon1000, int> DensityMap;

DensityMap  targetDensityMap;
//QString     messageStringbuffer;
AIS         aisMessageHandler;

void addDensityPoint(double lat,double lon)
{
    LatLon1000 key(lat*1000,lon*1000);

    DensityMap::iterator it =targetDensityMap.find(key);
    if ( it == targetDensityMap.end() )
    {
        targetDensityMap.insert(std::pair<LatLon1000,int>(key,1));
    }
    else
    {
        it->second++;
    }
}

void AIStoDensityMap(QByteArray inputdata)
{
    QString str = QString(inputdata);
    QStringList strlist = str.split("!");
    if(strlist.size() < 1)return;
    for(int i = 0;i<strlist.size();i++)
    {
        if(strlist.at(i).length())
        if(aisMessageHandler.ProcessNMEA(strlist.at(i)))
        {
//            CConfig::mStat.cAisUpdateTime = clock();
            double mLat = aisMessageHandler.get_latitude()/600000.0;
            double mLong = aisMessageHandler.get_longitude()/600000.0;
            if(
                    abs(mLat*mLong)>0.1&&
                    (mLat)>-90&&
                    (mLong)>-180&&
                    (mLat)<90&&
                    (mLong)<180
                    )
                addDensityPoint(mLat,mLong);
            else
                continue;
        }
    }
    //messageStringbuffer=strlist.at(strlist.size()-1);
}
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QDir directory("D:/HR2D/logs/AIS/");
    QStringList logfiles = directory.entryList(QStringList() << "*.log" ,QDir::Files);
    char dataPt[FRAME_LEN_NAV];
    int nRecord = 0;
    foreach(QString filename, logfiles) {
        QFile logFile("D:/HR2D/logs/AIS/"+filename);
        logFile.open(QIODevice::ReadOnly);
        for(;;)
        {
            nRecord++;
            int len = logFile.readLine(dataPt,FRAME_LEN_NAV);
            if(len<0)break;
            AIStoDensityMap(QByteArray(dataPt,len));
        }
        logFile.close();
        if(nRecord>500000)break;
    }
    int n=0;
    for (auto it : targetDensityMap)
    {
        cout << "[ " << it.first.first << ", "<<it.first.second << ", "
             << it.second << "]"<<n++
             <<" "<<targetDensityMap.size()<<"\n";
    }
//    cout<<targetDensityMap.size();
    flushall();
    return 0;
}
