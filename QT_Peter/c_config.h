
#ifndef CONFIG_H
#define CONFIG_H

#define SCALE_MAX 80
#define SCALE_MIN 5
#define HR_APP_PATH       "D:/HR2D/"
#define HR_DATA_REC_PATH  "D:/HR2D/RecordData/"
#define HR_CONFIG_FILE    "D:/HR2D/radar_config.xml"
#define HR_CONFIG_FILE_DF "D:/HR2D/radar_config_default.xml"
#define XML_ELEM_NAME     "radar_config"
#define DEFAULT_LAT		20.707
#define DEFAULT_LONG	106.78
#include <QFile>
#include <QHash>
#include <QXmlStreamReader>
//#include <QSettings>
class CConfig
{
public:
    CConfig(void);
    ~CConfig(void);
    static double shipHeadingDeg;
    static double shipSpeed;
    static double antennaAziDeg;
    static volatile long long int time_now_ms;
    static QHash<QString, QString> mHashData;
    static void    setValue(QString key, double value);
    static void    setValue(QString key,QString value);
    static double  getDouble(QString key, double defaultValue=0);
    static QString getString(QString key, QString defaultValue="0");
    static int     getInt(QString key, int defaultValue=0);
    static void    setDefault();
    static void     SaveToFile();

    //static QXmlStreamReader xml;
private:
    static QHash<QString, QString> readFile();

};

#endif
