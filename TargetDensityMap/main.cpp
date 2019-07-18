//#include <QCoreApplication>
#include <QDir>
#include <../QT_Peter/AIS/AIS.h>
#include <iostream>
#include <fstream>
#include <QDirIterator>
#define FRAME_LEN_NAV 1500
using namespace std;
typedef std::pair<int,int> LatLon1000;
typedef std::map<LatLon1000, int> DensityMap;
int nRecord = 0;
int nLine = 0;
int nMsg = 0;
int nPoint = 0;
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
    nLine++;
    QString str = QString(inputdata);
    QStringList strlist = str.split("!");
    if(strlist.size() < 1)return;
    for(int i = 0;i<strlist.size();i++)
    {

        if(!strlist.at(i).length())continue;
        nMsg++;
        if(aisMessageHandler.ProcessNMEA(strlist.at(i)))
        {
            nRecord++;
//            CConfig::mStat.cAisUpdateTime = clock();
            double mLat  = aisMessageHandler.get_latitude()/600000.0;
            double mLong = aisMessageHandler.get_longitude()/600000.0;
            if(
                    abs(mLat*mLong)>0.1&&
                    (mLat)>-90&&
                    (mLong)>-180&&
                    (mLat)<90&&
                    (mLong)<180
                    )
            {
                nPoint++;
                addDensityPoint(mLat,mLong);
            }
            else
                continue;
        }
    }
    //messageStringbuffer=strlist.at(strlist.size()-1);
}
int main(int argc, char *argv[])
{
    ofstream datafile;
    datafile.open("D:/HR2D/target_density.txt");
//    QCoreApplication a(argc, argv);
    char dataPt[FRAME_LEN_NAV];

    QDirIterator it("D:/HR2D/logs/",
                    QStringList() << "*.log",
                    QDir::Files,
                    QDirIterator::Subdirectories);
    while (it.hasNext())
    {
        QFile logFile = it.next();
        if(!logFile.fileName().contains("ais"))continue;
        logFile.open(QIODevice::ReadOnly);
        for(;;)
        {

            int len = logFile.readLine(dataPt,FRAME_LEN_NAV);
            if(len<0)break;
            AIStoDensityMap(QByteArray(dataPt,len));
        }
        logFile.close();
    }
//    int n=0;
    for (auto it : targetDensityMap)
    {
        datafile <<it.first.first << ","<<it.first.second << ","
             << it.second <<"\n";
    }
    cout<<"\nPoints loaded:"<<nPoint
          <<"\nRecord loaded:"<<nRecord
          <<"\nMessage loaded:"<<nMsg
            <<"\nLine loaded:"<<nLine
       <<"\nPositions:"<<targetDensityMap.size();
    flushall();
    datafile.close();
    return 0;
}
