
#include "c_config.h"
//CConfig         mGlobbalConfig;
#include <iostream>
#include <ctime>
using namespace std;
//double CConfig::shipHeadingDeg=5;
//double CConfig::shipCourseDeg = 0;
//double CConfig::shipSpeed=0;
//double CConfig::antennaAziDeg=0;
std::queue<WarningMessage> CConfig::mWarningList;
bool CConfig::isChanged = false;
radarStatus_3C CConfig::mStat ;
radarStatus_3C::radarStatus_3C()
{
//    isStatChange = false;
    mFrameCount = 0;
    memset(&(msgGlobal[0]),0,32);
    cAisUpdateTime      = clock();
    cGpsUpdateTime      = clock();
    c22UpdateTime       = clock();
    c21UpdateTime       = clock();
    cBHUpdateTime       = clock();
    cGyroUpdateTime     = clock();
    cVeloUpdateTime     = clock();
    cHDTUpdateTime      = clock();
    cCourseUpdateTime   = clock();
    shipHeadingDeg = 30;
    shipHeadingRate_dps=0;
    isGyro = false;

}

radarStatus_3C::~radarStatus_3C()
{

}

void radarStatus_3C::setGPSLocation(double lat, double lon)
{
    mLat = lat;mLon = lon;
    cGpsUpdateTime = clock();
}

void radarStatus_3C::setShipCourse(double value)
{
    shipCourseDeg = value;
    cCourseUpdateTime = clock();
}

void radarStatus_3C::setShipSpeed(double value)
{
    shipSpeed = value;
    cVeloUpdateTime = clock();
}

double radarStatus_3C::getShipHeadingDeg()
{
    return shipHeadingDeg;
}
double radarStatus_3C::getshipHeadingRate_dps()
{
    return shipHeadingRate_dps;
}
QHash<QString, QString> CConfig::mHashData = CConfig::readFile();
volatile long long int CConfig::time_now_ms = 0;
void CConfig::setValue(QString key, double value)
{
    settings
    QString strValue = QString::number(value);
    mHashData.insert(key, strValue);
    isChanged = true;
    //SaveToFile();
}

void CConfig::setValue(QString key, QString value)
{
    mHashData.insert(key, value);
    isChanged = true;
    //SaveToFile();
}

double CConfig::getDouble(QString key,double defaultValue )
{
    if(mHashData.find(key)!=mHashData.end())
    return mHashData.value(key).toDouble();
    else
    {
        setValue(key,defaultValue);
        return defaultValue;
    }
}
int CConfig::getInt(QString key, int defaultValue )
{
    if(mHashData.find(key)!=mHashData.end())
    return mHashData.value(key).toInt();
    else
    {
        setValue(key,defaultValue);
        return defaultValue;
    }
}
QString CConfig::getString(QString key,QString defaultValue )
{
    if(mHashData.find(key)!=mHashData.end())
    return mHashData.value(key);
    else
    {
        setValue(key,defaultValue);
        return defaultValue;
    }
}

CConfig::CConfig(void)
{
    //hashData.;
    //shipHeadingDeg = 0;

    readFile();
}

CConfig::~CConfig(void)
{
}

void CConfig::SaveToFile()
{
    if(isChanged)isChanged = false;else return;
    QHash<QString, QString>::const_iterator it = mHashData.constBegin();
    QXmlStreamAttributes attr;
    while (it != mHashData.constEnd()) {
        attr.append(it.key(),it.value());
        ++it;
    }
    QXmlStreamWriter writer;
    if(QFile::exists(HR_CONFIG_FILE_BACKUP_1))
    {
        if (QFile::exists(HR_CONFIG_FILE_BACKUP_2))
        {
            QFile::remove(HR_CONFIG_FILE_BACKUP_2);

        }
        QFile::rename(HR_CONFIG_FILE_BACKUP_1,HR_CONFIG_FILE_BACKUP_2);
    }
    QFile::rename(HR_CONFIG_FILE,HR_CONFIG_FILE_BACKUP_1);
    QFile xmlFile(HR_CONFIG_FILE);
    xmlFile.open(QIODevice::WriteOnly);
    writer.setDevice(&xmlFile);
    writer.writeEmptyElement(XML_ELEM_NAME);
    writer.writeAttributes(attr);
    writer.writeEndElement();
    xmlFile.close();
    //QFile xmlFile(HR_CONFIG_FILE_BACKUP_1);
    //xmlFile.copy(HR_CONFIG_FILE);
}

void CConfig::ReportError(const char* error)
{
    freopen(HR_ERROR_FILE, "a", stderr );
    time_t rawtime;
    struct tm * timeinfo;
    char buffer[80];

    time (&rawtime);
    timeinfo = localtime(&rawtime);

    strftime(buffer,sizeof(buffer),"%d-%m-%Y %H:%M:%S    ",timeinfo);
    cerr << buffer;
    cerr << error;
    cerr << endl;
}

void CConfig::AddMessage(QString message)
{
    QDateTime now = QDateTime::fromMSecsSinceEpoch(time_now_ms);
    WarningMessage warning;
    warning.message = now.toString("hh:mm:ss:")+ message ;
    warning.time = clock();
    mWarningList.push(warning);
}

void CConfig::setDefault()
{
    if (QFile::exists(HR_CONFIG_FILE))
    {
        QFile::remove(HR_CONFIG_FILE);
    }

    QFile::copy(HR_CONFIG_FILE_DF, HR_CONFIG_FILE);

}
QHash<QString, QString> CConfig::readFile(QString fileName)
{
    QFile xmlFile(fileName);
    xmlFile.open(QIODevice::ReadOnly);

    QXmlStreamReader xml;
    xml.setDevice(&xmlFile);
    int nElement = 0;
    QHash<QString, QString> hashData;
    while (xml.readNextStartElement())
    {

        if(xml.name()==XML_ELEM_NAME)
        {

           for (int i=0;i<xml.attributes().size();i++)
           {
               nElement++;
               QXmlStreamAttribute attr = xml.attributes().at(i);
               hashData.insert( attr.name().toString(),
                                attr.value().toString());
           }
        }
        if (xml.tokenType() == QXmlStreamReader::Invalid)
            xml.readNext();
        // readNextStartElement() leaves the stream in
        // an invalid state at the end. A single readNext()
        // will advance us to EndDocument.
        if (xml.hasError()) {
            if(fileName==HR_CONFIG_FILE) readFile(HR_CONFIG_FILE_BACKUP_1);
            else if(fileName==HR_CONFIG_FILE_BACKUP_1)readFile(HR_CONFIG_FILE_BACKUP_2);
            else
            {
                ReportError("Config load failed");
            }
        }
    }
    if((!hashData.contains("mLon"))||(!hashData.contains("mLat")))
    {
        if(fileName==HR_CONFIG_FILE)                readFile(HR_CONFIG_FILE_BACKUP_1);
        else if(fileName==HR_CONFIG_FILE_BACKUP_1)  readFile(HR_CONFIG_FILE_BACKUP_2);
        else
        {
            ReportError("Config load failed");
        }
    }
    xmlFile.close();
    return hashData;
}
QHash<QString, QString> CConfig::readFile() {

    return readFile(HR_CONFIG_FILE);
}

std::queue<WarningMessage>* CConfig::getWarningList()
{
    return &mWarningList;
}

