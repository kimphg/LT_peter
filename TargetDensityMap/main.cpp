//#include <QCoreApplication>
#include <QDir>
#include <../QT_Peter/AIS/AIS.h>
#include <iostream>
#include <fstream>
#include <QDirIterator>
#include <vector>
#include <map>
#define FRAME_LEN_NAV 1500
using namespace std;
typedef struct
{
    int lon,lat,level;
}CellKey;
typedef std::pair<double,double> PointD;
class DensityMapCell
{
public:
    DensityMapCell(int isizeLevel)
    {
        isOverflow = false;
        sizeLevel = isizeLevel;
    }
    bool addPoint(double lon,double lat)
    {
        PointD point(lon,lat);
        data.push_back(point);
        if(data.size()>=1000)isOverflow =true;
        return isOverflow;
    }

    int sizeLevel;
    bool isOverflow;
    std::vector<PointD> data;
} ;
struct CellKeyCompare
{
   bool operator() (const CellKey& lhs, const CellKey& rhs) const
   {
       if( lhs.lon < rhs.lon)return true;
       if( lhs.lon > rhs.lon)return false;
       if( lhs.lat < rhs.lat)return true;
       if( lhs.lat > rhs.lat)return false;
       if( lhs.level < rhs.level)return true;
       if( lhs.level > rhs.level)return false;
       return false;

   }
};
typedef std::map<CellKey, DensityMapCell,CellKeyCompare> DensityMap;
int nRecord = 0;
int nLine = 0;
int nMsg = 0;
int nPoint = 0;
DensityMap  targetDensityMap;
//QString     messageStringbuffer;
AIS         aisMessageHandler;

void addDensityPoint(double lon,double lat,int level = 50)
{

    CellKey key;
    key.lon = lon*level;
    key.lat = lat*level;
    key.level = level;
    DensityMap::iterator it =targetDensityMap.find(key);
    if ( it == targetDensityMap.end() )
    {
        DensityMapCell newcell(level);
        newcell.addPoint(lon,lat);
        targetDensityMap.insert(std::pair<CellKey, DensityMapCell>(key,newcell));
    }
    else
    {
        if(it->second.isOverflow)
        {
            addDensityPoint(lon,lat,level*2);
        }
        else
        {
            if(it->second.addPoint(lon,lat))//overflow
            {
                if(level>3200)
                {
                    it->second.isOverflow = false;
                    return;
                }
                for(PointD dataPoint : it->second.data)
                {
                    addDensityPoint(dataPoint.first,dataPoint.second,level*2);
                }
            }
        }
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
            if(aisMessageHandler.get_SOG()/10.0>3)
            {
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
                    addDensityPoint(mLong,mLat);
                }

            }
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
        if(it.second.isOverflow)continue;
        datafile <<it.first.lon << ","<<it.first.lat << ","<<it.first.level << ","
                << it.second.data.size() <<"\n";
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
