
#include "c_radar_thread.h"

#include <QDir>
#include <QJsonArray>
#include <QJsonObject>
#include <QNetworkReply>
#define FRAME_LEN_NAV 1500
#define FRAME_LEN_MAX 5000
#define MAX_IREC 2000
//#include <QGeoCoordinate>
//#include <QNmeaPositionInfoSource>
DataBuff dataB[MAX_IREC];
//uchar udpFrameBuffer[MAX_IREC][OUTPUT_FRAME_SIZE];
short iRec=0,iRead=0;
//bool *pIsDrawn;
bool *pIsPlaying;
//CConfig         mGlobbalConfig;
//QNmeaPositionInfoSource *geoLocation = NULL;
//QTimer readDataBuff;
dataProcessingThread::~dataProcessingThread()
{
    delete mRadarData;
    logFile.close();
    delete networkManager;
//    signTTMFile.close();
    //    delete arpaData;
}

void dataProcessingThread::ReadDataBuffer()
{

    if(isPlaying)
    {
        playbackRadarData();
        return;
    }
    short nread = 0;
    if(iRec!=iRead)
    {
        if(isSimulationMode)return;

    }
    while(iRec!=iRead)
    {
        nread++;
//        if(nread>=600)
//        {
//            mRadarData->resetData();
//            break;
//        }
        uchar *pData = &(dataB[iRead].data[0]);
        unsigned short dataLen = dataB[iRead].len;
        mRadarData->processSocketData(pData,dataLen);
//        if(dataLen==1058)
//            dataLen=dataLen;
//        if(isRecording&&dataLen)
//        {
//            signRecFile.write((char*)&dataLen,2);
//            signRecFile.write((char*)pData,dataLen);
//        }
        iRead++;
        if(iRead>=MAX_IREC)iRead=0;
    }
}
void dataProcessingThread::setRotationSpeed(int index)
{
    unsigned char command[7];
    command[0]=0xaa;
    command[1]=0xab;
    command[2]=0x03;
    command[3]=index;
    command[4]=0x00;
    command[5]=0x00;
    command[6]=0x00;
    sendCommand(&command[0],7);
    sendCommand(&command[0],7);
    sendCommand(&command[0],7);
}

bool dataProcessingThread::getIsXuLyThuCap() const
{
    return isXuLyThuCap;
}

void dataProcessingThread::setIsXuLyThuCap(bool value)
{
    isXuLyThuCap = value;
    mRadarData->setIsVtorih(isXuLyThuCap);
}

double dataProcessingThread::getSelsynAzi() const
{
    return selsynEncoderAzi;
}
dataProcessingThread::dataProcessingThread()
{

    networkManager = new QNetworkAccessManager();
        QObject::connect(networkManager, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(managerFinished(QNetworkReply*)));
    //set initial time
    QDateTime now = QDateTime::currentDateTime();
    QString logDirName = "D:\\HR2D\\logs\\"+now.toString("\\yy.MM\\");
    if(!QDir(logDirName).exists())
    {
        QDir().mkdir(logDirName);
    }
    logFile.setFileName(logDirName+now.toString("yy.MM.dd-hh.mm.ss")+".log");
    logFile.open(QIODevice::WriteOnly);

    mCudaAge200ms=50;
    mFramesPerSec=0;
    isSimulationMode = false;
    mRadMode = ModeComplexSignal;
    mAntennaAzi = 0;
    failureCount = 0;
    isHeadingAvaible=false;
    selsynEncoderAzi = 0;
    isXuLyThuCap = false;
    dataBuff = &dataB[0];
    iRec=0;
    iRead=0;
    //    pIsDrawn = &isDrawn;
    //    isDrawn = true;
    pIsPlaying = &isPlaying;
    playRate = 10;
    //    arpaData = new C_ARPA_data();
    isRecording = false;
    mRadarData = new C_radar_data();
    isPlaying = false;
    radarSocket = new QUdpSocket(this);
    navSocket = new QUdpSocket(this);
    int port = CConfig::getInt("radarSocketPort",31000);
    while(port<31100)
    {
        if(radarSocket->bind(port))
        {
            break;
        }
        port++;
    }
    port = CConfig::getInt("navSocketPort",30000);
    while(port<30100)
    {
        if(navSocket->bind(port))
        {
            connect(navSocket,SIGNAL(readyRead()),this, SLOT(ReadNavData()));
            break;
        }
        port++;
    }
    connect(&commandSendTimer, &QTimer::timeout, this, &dataProcessingThread::Timer200ms);
    commandSendTimer.start(200);
    connect(&readUdpBuffTimer, &QTimer::timeout, this, &dataProcessingThread::ReadDataBuffer);
    readUdpBuffTimer.start(20);
//    initSerialComm();
//    QDateTime now = QDateTime::currentDateTime();
//    QString filename = "D:/HR2D/ttm_rec_"+ now.toString("dd.MM.yy_hh.mm")+ ".log";

//    signTTMFile.setFileName(filename);


}

void dataProcessingThread::forwardOldGps()
{
    QString lastGpsMsg = CConfig::getString("lastGpsMsg","$GPGGA,030227.900,2042.373,N,10646.827,E,1,12,1.0,0.0,M,0.0,M,,*68");
    radarSocket->writeDatagram(lastGpsMsg.toLocal8Bit(),QHostAddress("127.0.0.1"),30001);
}
void dataProcessingThread::ReadNavData()
{
    while(navSocket->hasPendingDatagrams())
    {
        int len = navSocket->pendingDatagramSize();
        QByteArray data;
        data.resize(len);
        navSocket->readDatagram(data.data(),len);
        //ProcessNavData(data);
    }

    return;
}
bool dataProcessingThread::readGyroMsg(unsigned char *mReceiveBuff,int len)
{
    int n=0;
    while(n<len-31)
    {
        unsigned char *databegin =&mReceiveBuff[n];
        n++;
        if(databegin[0]==0x5a&&databegin[1]==0xa5&&databegin[31]==0xAA)
        {
            double heading = (((databegin[6])<<8)|databegin[7])/182.04444444444444444444444;//65536/360.0
            //double headingRate = degrees((((mReceiveBuff[12])<<8)|mReceiveBuff[13])/10430.21919552736);//deg per sec
            double headingRate = ((databegin[12])<<8)+databegin[13];
            if(headingRate>32768.0)headingRate=headingRate-65536.0;
            headingRate/=32768.0;
            if(CConfig::mStat.getAgeGyro21()>3000)
            {
                CConfig::mStat.inputGyroDeg(heading,degrees(headingRate));

//                mRadarData->setShipHeadingDeg(heading);
            }
            //CConfig::mStat.setcGyroUpdateTime();
            return true;
        }
    }
    n=0;
    while(n<len-30)//NAVGP 10/50 Hz message
    {
        unsigned char *databegin =&mReceiveBuff[n];
        n++;
        if(databegin[0]==0x5a&&databegin[1]==0xa5&&databegin[4]==0x7F)
        {
            for(int k = 10;k<26;k++)
            {
                if(databegin[k]==0x00&&databegin[k+1]==0x65)
                {
                    double heading = (((databegin[k+2])<<8)|databegin[k+3])/182.0444444444444444444444444444444444444444444444444;//65536/360.0
                    //double headingRate = degrees((((mReceiveBuff[12])<<8)|mReceiveBuff[13])/10430.21919552736);//deg per sec
                    //double headingRate = ((databegin[12])<<8)+databegin[13];
                    //if(headingRate>32768.0)headingRate=headingRate-65536.0;
                    //headingRate/=32768.0;
                    if(CConfig::mStat.getAgeGyro21()>3000)
                    {
                        CConfig::mStat.inputGyroDeg(heading,0);
                    }
                    else CConfig::mStat.setcGyroUpdateTime();
                    return true;
                }
            }
        }
    }
    return false;
}
bool dataProcessingThread::readNmea(unsigned char *mReceiveBuff,int len)
{
    int n=0;
    while(n<len-10)
    {

        unsigned char *databegin =&mReceiveBuff[n];
        n++;
        if(databegin[0]=='$')
        {
            if(databegin[3]=='V'
                    &&databegin[4]=='B'
                    &&databegin[5]=='W')//speed message
            {
                QString message((char*)&databegin[0]);
                QStringList tokens = message.split(',');
                if(tokens.size()<7)return true;
                if(tokens[6]=="A")CConfig::mStat.setShipSpeed2( tokens[4].toDouble());

                if(tokens[3]=="A")CConfig::mStat.setShipSpeed( tokens[1].toDouble());
                return true;
            }
            else if(databegin[3]=='V'
                    &&databegin[4]=='H'
                    &&databegin[5]=='W')//speed message
            {
                QString message((char*)&databegin[0]);
                QStringList tokens = message.split(',');
                if(tokens.size()<9)return true;
                if(tokens[6]=="N")CConfig::mStat.setShipSpeed( tokens[4].toDouble());
                if(tokens[2]=="T")CConfig::mStat.setShipCourse( tokens[1].toDouble());
                return true;
            }
            else if(databegin[3]=='H'
                    &&databegin[4]=='D'
                    &&databegin[5]=='T')//true heading message xxHDT
            {
                if(CConfig::mStat.getAgeGyro()<10000)return true;// only work when no gyro available
                QString message((char*)&databegin[0]);
                QStringList tokens = message.split(',');
                if(tokens.size()<3)return true;
                CConfig::mStat.inputHDT(tokens[1].toDouble());
                return true;
            }
            else if((databegin[1]=='G')&&(databegin[2]=='P'))//GPS
            {
                CConfig::setValue("lastGpsMsg",QString((char*)mReceiveBuff));
                for(int i = 0;i<=len;i++)
                {
                    bool endValid ;
                    if(i<len)endValid = mGPS.decode(databegin[i]);
                    else endValid = mGPS.decode('\r');//in case the sentence has no ending
                    if(endValid)
                    {

                        //mStat.cGpsUpdateTime = clock();
                        double mLat,mLon;
                        mGPS.get_position(&mLat,&mLon);
                        if(mLat>-90&&mLat<90&&mLon>-180&&mLon<180)
                            CConfig::setGPSLocation(mLat,mLon);
                        else printf("\nwrong GPS value");
                        logFile.write(QByteArray((char*)(databegin),len));
                        return true;
                    }

                }
            }
        }
        if(databegin[0]=='!'&&databegin[1]=='A')//AIS
        {
            QByteArray ba=QByteArray((char*)databegin,len);
            inputAISData(ba);
//            if (!isPlaying)aisLogFile.write(ba);
            return true;
        }

    }
    return false;

}
bool dataProcessingThread::readMay22Msg(unsigned char *mReceiveBuff,int len)
{
    int n=0;
    while(n<len-8)
    {

        unsigned char *databegin =&mReceiveBuff[n];
        n++;
        if(databegin[0]==0xaa&&databegin[1]==0x55)//system messages
        {
            if(databegin[2]==0x65)// trang thai may 2-2
            {

                CConfig::mStat.ReadStatus22(&databegin[4]);
                return true;
            }
            if(databegin[2]==0x03)// trang thai bao hong toan dai
            {
                if(len<24)return false;
                CConfig::mStat.ReadStatusGlobal(&databegin[4]);
                return true;

            }

        }
    }
    return false;
}
void dataProcessingThread::ProcessNavData(unsigned char *mReceiveBuff,int len)
{
    if(len<7)return;
//    unsigned short dataLen = len;
    if(isSimulationMode)return;

    if(readNmea(mReceiveBuff,len))
    {
        radarSocket->writeDatagram((const char*)mReceiveBuff,len,QHostAddress("127.0.0.1"),30001);

        return;
    }
    else if(readMay22Msg(mReceiveBuff,len))//system messages
    {
        return;
    }

    else if(readGyroMsg(mReceiveBuff,len)) //gyro messages
    {
        return;
    }

}
//void dataProcessingThread::initSerialComm()
//{
//    int serialBaud ;
//    // baudrate at 4800 standart for low speed encoder and ais
//    serialBaud = 38400;
//    QList<QSerialPortInfo> portlist = QSerialPortInfo::availablePorts();
//    for(int i = 0;i<portlist.size();i++)
//    {
//        if(!portlist.at(i).isBusy())
//        {
//            QSerialPort *newport = new QSerialPort(this);
//            QString qstr = portlist.at(i).portName();
//            newport->setPortName(qstr);
//            newport->setBaudRate(serialBaud);
//            newport->open(QIODevice::ReadWrite);
//            if(newport->isOpen())
//            {serialPorts.push_back(newport);
//                CConfig::AddMessage("Open serial:"+qstr+" at:"+QString::number(serialBaud));}
//        }
//    }


//}
/*bool  dataProcessingThread::getPosition(double *lat,double *lon, double *heading)
{
    if(!geoLocation)return false;

    QGeoPositionInfo location = geoLocation->lastKnownPosition();
    if(!location.isValid())return false ;
    *lat = location.coordinate().latitude();
    *lon = location.coordinate().longitude();
    *heading = location.attribute(QGeoPositionInfo::Direction);

    return true;
}*/


void dataProcessingThread::SerialDataRead()
{
    for(std::vector<QSerialPort*>::iterator it = serialPorts.begin() ; it != serialPorts.end(); ++it)
    {
        QByteArray responseData = (*it)->readAll();

        if(responseData.size())
        {
            processSerialData(responseData);
        }

    }
}
void dataProcessingThread::processSerialData(QByteArray inputData)
{

//    unsigned short dataLen = inputData.length();
    unsigned char* data = (unsigned char*)inputData.data();
//    if(isRecording&&dataLen)
//    {
//        signRecFile.write((char*)&dataLen,2);
//        signRecFile.write((char*)data,dataLen);
//    }
    if(data[0]==0xff)//encoder data
    {
        mazi = (data[1]<<16) + (data[2]<<8)+(data[3]);
        realazi1 = (data[4]);
        realazi2 = (data[5]);
        newAzi = mazi*360.0/262144.0*3.0;
        while(newAzi>=360)newAzi-=360;
        selsynEncoderAzi = newAzi;
    }
    //    else if(data[0]==0x24)//NMEA
    //    {
    //         QString s_data = QString::fromLatin1(inputData.data());
    //         if(s_data.contains("HDT"))
    //         {
    //              mHeading = s_data.split(',').at(1).toDouble();
    //              isHeadingAvaible = true;
    //         }
    //    }
    else
    {

        inputAISData(inputData);
        //printf("arpa ais\n");

    }

}
bool dataProcessingThread::checkFeedback()
{
    unsigned char * pFeedBack = mRadarData->getFeedback();
    unsigned char * command = &radarComQ.front().bytes[0];
    if(   (pFeedBack[0]==command[0])
          &&(pFeedBack[1]==command[1])
          &&(pFeedBack[2]==command[2])
          &&(pFeedBack[3]==command[3])
          &&(pFeedBack[4]==command[4])
          &&(pFeedBack[5]==command[5])
          &&(pFeedBack[6]==command[6])
          )
    {
        return true;
    }
    else return false;
}
#define MAX_AZIR_SYSTEM 4096
void dataProcessingThread::sendAziData()
{
    int azitrue = (mRadarData->getCurAziTrueRad()/PI_NHAN2*MAX_AZIR_SYSTEM);
    int heading = CConfig::mStat.shipHeadingDeg/360.0*MAX_AZIR_SYSTEM;
    int azi=azitrue-heading;
    if(azi<0)azi+=MAX_AZIR_SYSTEM;
    if(azi>=MAX_AZIR_SYSTEM)azi-=MAX_AZIR_SYSTEM;
    unsigned char sendBuf[9];
    sendBuf[0]=0xaa;
    sendBuf[1]=0x55;
    sendBuf[2]=0x6e;
    sendBuf[3]=0x09;

    sendBuf[4]=azi>>8;
    sendBuf[5]=azi;

    sendBuf[6]=heading>>8;
    sendBuf[7]=heading;
    sendBuf[8]=0;
    sendCommand(&sendBuf[0],9,false);

}
void dataProcessingThread::sendRATTM()
{

    for(int i=0;i<mRadarData->mTrackList.size();i++)
    {
        C_primary_track *track = &(mRadarData->mTrackList.at(i));
        if(track->isRemoved())continue;
        int len = track->mTTM.size();
        if(len)
        {

            radarSocket->writeDatagram(track->mTTM.toLatin1(),
                                       QHostAddress(CConfig::getString("TTMOutputIP","192.168.1.252")),
                                       CConfig::getInt("TTMOutputPort",30001)
                                       );

            radarSocket->writeDatagram(track->mTIF.toLatin1(),
                                       QHostAddress(CConfig::getString("TIFOutputIP","192.168.0.80")),
                                       CConfig::getInt("TIFOutputPort",30001)
                                       );
            track->mTTM.clear();

        }

    }
    QByteArray plotOutput;
    while(mRadarData->object_output_queue.size())
    {
        object_t *obj = &(mRadarData->object_output_queue.back());
        QString sentence = "$RAPLT,"+
                QString::number(obj->lat, 'f',5) +","+
                QString::number(obj->lon, 'f',5) +","+
                QString::number(obj->dazi,'f',5) +","+
                QString::number(obj->drg, 'f',5) +","+
                QString::number(obj->timeMs)+"\r\n";
        plotOutput.append(sentence);
        mRadarData->object_output_queue.pop();
    }
    if(plotOutput.size())
    {
        radarSocket->writeDatagram(plotOutput,
                                   QHostAddress(CConfig::getString("PLTOutputIP","192.168.0.80")),
                                   CConfig::getInt("PLTOutputPort",30001)
                                   );
    }
}
void dataProcessingThread::Timer200ms()
{
//    printf("\nmCudaAge200ms++:%d",mCudaAge200ms);
    mCudaAge200ms++;
    CalculateRFR();
    sendAziData();
    SerialDataRead();
    sendRATTM();
    if(radarComQ.size())
    {
        if(radarComQ.front().bytes[1]==0xab)
        {
            radarSocket->writeDatagram((char*)&radarComQ.front().bytes[0],
                    8,
                    QHostAddress("192.168.0.44"),2572
                    );

        }

        radarComQ.pop();
        /*if(checkFeedback())
        {
            radarComQ.pop();
            failureCount = 0;
        }
        else
        {
            failureCount++;
            if(failureCount>3)
            {
                radarComQ.pop();
                failureCount = 0;
            }
        }*/


    }

    //    mRadarData->ProcessObjects();
    /*while(false)// no sophia yet
    {
        object_t obj= mRadarData->ProcesstRadarObjects();
        if(!obj.size)break;
        QString outputData("$RATAR");

        outputData+= ","+ QString::number(obj.time)
                +","+ QString::number(obj.az/PI*180.0,'f',2)//azi
                +","+ QString::number(1.0)//maxerr of azi
                +","+ QString::number(obj.rgKm)//
                +","+ QString::number(obj.rangeRes);
                //+","+ QString::number(this->mGpsData.back().lat);
                //+","+ QString::number(this->mGpsData.back().lon);

        radarSocket->writeDatagram((char*)outputData.toStdString().data(),
                outputData.size(),
                QHostAddress("127.0.0.1"),31001
                );

    }*/
}
void dataProcessingThread::playbackRadarData()
{
    if(isPlaying) {
        //isDrawn = false;
        unsigned short len;
        if(!signRepFile.isOpen())return;
        for(unsigned short i=0;i<playRate;i++)
        {
            //QMutexLocker locker(&mutex);

            if(!signRepFile.read((char*)&len,2))
            {
                signRepFile.seek(0);
                mRadarData->SelfRotationReset();
                CConfig::AddMessage("Reset file replay");
                //togglePlayPause(false);
                return;
            }
            if(!len)
                continue;
            if(len>5000)
                continue;
            QByteArray buff;
            buff.resize(len);

            signRepFile.read(buff.data(),len);
            ProcessData((unsigned char*)buff.data(),len);

            if(playRate<10){togglePlayPause(false);return;}
        }
        return;
    }
}

void dataProcessingThread::StopProcessing()
{

    deleteLater();
}
/*void dataProcessingThread::setIsDrawn(bool value)
{
    //isDrawn = value;
}*/

void dataProcessingThread::SetRadarPort( unsigned short portNumber)
{
    radarSocket->bind(portNumber, QUdpSocket::ShareAddress);
}
void dataProcessingThread::SetARPAPort( unsigned short portNumber)
{
    ARPADataSocket->bind(portNumber, QUdpSocket::ShareAddress);
}

void dataProcessingThread::loadRecordDataFile(QString fileName)//
{
    if(signRepFile.isOpen()) signRepFile.close();
    signRepFile.setFileName(fileName);
    signRepFile.open(QIODevice::ReadOnly);

    isPlaying = false;
}

void dataProcessingThread::togglePlayPause(bool play)
{
    isPlaying = play;

}
void dataProcessingThread::addAisObj(AIS_object_t obj)
{
    int mmsi = obj.mMMSI;

    if(mAisData.find(mmsi)!=mAisData.end())
    {
        obj.merge(mAisData[mmsi]);

    }

    mAisData[mmsi] = obj;
    mRadarData->integrateAisPoint(&(mAisData[mmsi]));
}
void dataProcessingThread::inputAISData(QByteArray inputdata)
{
    messageStringbuffer.append(QString::fromLatin1(inputdata));
    if(messageStringbuffer.size()>1500)messageStringbuffer = "";
    //printf(inputdata.data());
    QStringList strlist = messageStringbuffer.split("!");
    if(strlist.size() <= 1)return;

    for(int i = 0;i<strlist.size()-1;i++)
        if(aisMessageHandler.ProcessNMEA(strlist.at(i)))
        {

            CConfig::mStat.cAisUpdateTime = clock();
            addAisObj(aisMessageHandler.GetAisObject());
        }
    messageStringbuffer=strlist.at(strlist.size()-1);

    return;
}

static unsigned long int lastFrameCount=0;

static uchar mReceiveBuff[FRAME_LEN_MAX];
void dataProcessingThread::CalculateRFR()
{
    double fDrame = double(CConfig::mStat.mFrameCount-lastFrameCount);
    mFramesPerSec = fDrame*5;
    lastFrameCount=CConfig::mStat.mFrameCount;
}

bool dataProcessingThread::getIsPlaying() const
{
    return isPlaying;
}
void dataProcessingThread::ProcessData(unsigned char* data,unsigned short len)
{
    if(isRecording)
    {

        signRecFile.write((char*)&len,2);
        signRecFile.write((char*)data,len);
    }
    if(len==4)
    {
//                radarSocket->readDatagram((char*)&mReceiveBuff[0],len);
        if(mReceiveBuff[0]==0xAA&&
                mReceiveBuff[1]==0xAA&&
                mReceiveBuff[2]==0xAA&&
                mReceiveBuff[3]==0xAA)
        {
            mCudaAge200ms = 0;
        }
        return;
    }
    if(len<FRAME_LEN_NAV&&(data[0]!=4))// rs485 packets
    {
//        radarSocket->readDatagram((char*)&mReceiveBuff[0],len);
        ProcessNavData(data,len);
        return;

    }
    else  if(len<=MAX_FRAME_SIZE)
    {
        if(isPlaying)
        {
            mRadarData->processSocketData(data,len);
        }
        else
        {
            memcpy(&(dataB[iRec].data[0]),data,len);
            dataB[iRec].len = len;
            iRec++;
            if(iRec>=MAX_IREC)iRec = 0;
        }
        return;
        //nframe++;
    }
}

void dataProcessingThread::managerFinished(QNetworkReply *reply)
{
    if (reply->error()) {
        qDebug() << reply->errorString();
        return;
    }

    QString answer = reply->readAll();
    QJsonDocument temp = QJsonDocument::fromJson(answer.toUtf8());
        if (temp.isArray()){
            //printf(answer.toUtf8().data());
        }
        if (temp.isObject()){
            //qDebug() << " It is an Object" <<endl;
            //printf(answer.toUtf8().data());
            QJsonObject jObject = temp.object();
            if(jObject["ships"].isArray())
            {
                QJsonArray jsonArray = jObject["ships"].toArray();
                foreach (const QJsonValue & value, jsonArray) {
                    if(value.isObject())
                    {
                       QJsonObject jObj  =  value.toObject();
                       AIS_object_t obj;
                       obj.mMMSI = jObj["mmsi"].toString().toInt();
                       obj.mLat = jObj ["lat"].toDouble();
                       obj.mLong = jObj["lng"].toDouble();
                       obj.mName = jObj["vsnm"].toString();
                       obj.mCog = jObj ["cog"].toDouble();
                       obj.mSog = jObj ["sog"].toDouble();
                       obj.mLut = QDateTime::currentMSecsSinceEpoch();
                       obj.mUpdateTime = clock();
                       addAisObj(obj);
                    }
                    //QJsonObject obj = value.toObject();
                    //propertyNames.append(obj["PropertyName"].toString());
                    //propertyKeys.append(obj["key"].toString());
                }
            }
        }
    //delete networkManager;
    //requestAISData();

}
void dataProcessingThread::requestAISData()
{

    networkRequest.setUrl(QUrl("http://quanlytau.vishipel.vn/Vishipel.VTS/TrackingHandler.ashx?cmd=live&range=101.042863,7.185843,111.430319,22.379662&areaW=0.055579335027628574,0.056304931640625&zoom=10&c=0&l=all"));
    networkManager->get(networkRequest);

}
void dataProcessingThread::run()
{
//    loadTargetDensityMap();
    int frameCount=0;
    while(true)
    {
        frameCount=0;
        while(radarSocket->hasPendingDatagrams())
        {
            frameCount++;
            unsigned short len = radarSocket->pendingDatagramSize();
            QHostAddress sender;
            radarSocket->readDatagram((char*)&mReceiveBuff[0],len,&sender);
            if(!len)
            {
                continue;
            }
            else
                if(!isPlaying)ProcessData(mReceiveBuff,len);


        }
        if(!frameCount)
            usleep(1);

    }

}

bool dataProcessingThread::getIsDrawn()
{
    if(clock()-mRadarData->mUpdateTime<2000){mRadarData->mUpdateTime = true;return false;}
    else return true;


}
void dataProcessingThread::stopThread()
{

    terminate();
}

void dataProcessingThread::radRequestTemp( char index)
{
    RadarCommand command;
    command.bytes[0] = 0xaa;
    command.bytes[1] = 0xab;
    command.bytes[2] = index;
    command.bytes[3] = 0xaa;
    command.bytes[4] = 0x00;
    command.bytes[5] = 0x00;
    command.bytes[6] = 0x00;
    command.bytes[7] = 0;
    if(radarComQ.size()<MAX_COMMAND_QUEUE_SIZE)
        radarComQ.push(command);
}

void dataProcessingThread::radTxOn()
{
    unsigned char bytes[8]={0,0,0,0,0,0,0,0};
    //rotation on
    bytes[1] = 0xab;
    //setRotationSpeed(3);
    //    //tx off
    //    command.bytes[0] = 0xaa;
    //    command.bytes[2] = 0x02;
    //    command.bytes[3] = 0x00;
    //    if(radarComQ.size()<MAX_COMMAND_QUEUE_SIZE)radarComQ.push(command);

    //thich nghi
    bytes[0] = 0xaa;
    bytes[2] = 0x02;
    bytes[3] = 0x01;
    sendCommand(&bytes[0],7);
    sendCommand(&bytes[0],7);
    sendCommand(&bytes[0],7);
    //if(radarComQ.size()<MAX_COMMAND_QUEUE_SIZE)radarComQ.push(command);
    //do trong
    bytes[0] = 0xaa;
    bytes[2] = 0x00;
    bytes[3] = 0x07;
    sendCommand(&bytes[0],7);
    sendCommand(&bytes[0],7);
    sendCommand(&bytes[0],7);
    //if(radarComQ.size()<MAX_COMMAND_QUEUE_SIZE)radarComQ.push(command);
    //0x18 - 0xab - 0x01 - 0x0f
    //dttt 192
    bytes[0] = 0x18;
    bytes[2] = 0x01;
    bytes[3] = 0x0f;
    sendCommand(&bytes[0],7);

    //if(radarComQ.size()<MAX_COMMAND_QUEUE_SIZE)radarComQ.push(command);
    //dttt 192
    bytes[0] = 0x14;
    bytes[2] = 0xff;
    bytes[3] = 0x02;
    sendCommand(&bytes[0],7);
    //if(radarComQ.size()<MAX_COMMAND_QUEUE_SIZE)radarComQ.push(command);
    //dttt 192
    bytes[0] = 0x01;
    bytes[2] = 0x02;
    bytes[3] = 0x07;
    sendCommand(&bytes[0],7);
    //if(radarComQ.size()<MAX_COMMAND_QUEUE_SIZE)radarComQ.push(command);
    //dttt 192
    bytes[0] = 0x15;
    bytes[2] = 0x01;
    bytes[3] = 0x00;
    sendCommand(&bytes[0],7);
    //if(radarComQ.size()<MAX_COMMAND_QUEUE_SIZE)radarComQ.push(command);
    //set resolution 60m
    /*command.bytes[0] = 0x08;
    command.bytes[2] = 0x02;
    command.bytes[3] = 0x00;
    if(radarComQ.size()<MAX_COMMAND_QUEUE_SIZE)radarComQ.push(command);*/
    //    if(radarComQ.size()<MAX_COMMAND_QUEUE_SIZE)radarComQ.push(command);
    //    if(radarComQ.size()<MAX_COMMAND_QUEUE_SIZE)radarComQ.push(command);
    //    if(radarComQ.size()<MAX_COMMAND_QUEUE_SIZE)radarComQ.push(command);
    //    if(radarComQ.size()<MAX_COMMAND_QUEUE_SIZE)radarComQ.push(command);
    //    //tat thich nghi
    //    command.bytes[0] = 0x1a;
    //    command.bytes[2] = 0x20;
    //    command.bytes[3] = 0x00;
    //    if(radarComQ.size()<MAX_COMMAND_QUEUE_SIZE)radarComQ.push(command);
    /*
    //tx on 1
    command.bytes[0] = 0xaa;
    command.bytes[2] = 0x02;
    command.bytes[3] = 0x01;
    if(radarComQ.size()<MAX_COMMAND_QUEUE_SIZE)radarComQ.push(command);
    if(radarComQ.size()<MAX_COMMAND_QUEUE_SIZE)radarComQ.push(command);
    if(radarComQ.size()<MAX_COMMAND_QUEUE_SIZE)radarComQ.push(command);
    //tx on 2
    command.bytes[2] = 0x00;
    if(radarComQ.size()<MAX_COMMAND_QUEUE_SIZE)radarComQ.push(command);
    if(radarComQ.size()<MAX_COMMAND_QUEUE_SIZE)radarComQ.push(command);
    if(radarComQ.size()<MAX_COMMAND_QUEUE_SIZE)radarComQ.push(command);
    //tx on tien khuech
    command.bytes[0] = 0x11;
    command.bytes[2] = 0xff;
    command.bytes[3] = 0x0f;
    command.bytes[4] = 0xff;
    command.bytes[5] = 0x05;
    if(radarComQ.size()<MAX_COMMAND_QUEUE_SIZE)radarComQ.push(command);

*/
    //    if(1){
    //        QFile logFile;
    //        QDateTime now = QDateTime::currentDateTime();
    //        if(!QDir("C:\\logs\\"+now.toString("\\dd.MM\\")).exists())
    //        {
    //            QDir().mkdir("C:\\logs\\"+now.toString("\\dd.MM\\"));
    //        }
    //        logFile.setFileName("C:\\logs\\"+now.toString("\\dd.MM\\")+now.toString("dd.MM-hh.mm.ss")+"_tx_on.log");

    //        logFile.open(QIODevice::WriteOnly);
    //        //logFile.p
    //        logFile.close();
    //    }


}

//void dataProcessingThread::loadTargetDensityMap()
//{
//    QDir directory("D:/HR2D/logs/AIS/");
//    QStringList logfiles = directory.entryList(QStringList() << "*.log" ,QDir::Files);
//    char dataPt[FRAME_LEN_NAV];
//    int nRecord = 0;
//    foreach(QString filename, logfiles) {
//        QFile logFile("D:/HR2D/logs/AIS/"+filename);
//        logFile.open(QIODevice::ReadOnly);
//        for(;;)
//        {
//            nRecord++;
//            int len = logFile.readLine(dataPt,FRAME_LEN_NAV);
//            if(len<0)break;
//            AIStoDensityMap(QByteArray(dataPt,len));
//        }
//        logFile.close();
//        if(nRecord>20000)break;
//    }

//}
void dataProcessingThread::radTxOff()
{
    //RadarCommand command;
    unsigned char bytes[8]={0xaa,0xab,0,0,0,0,0,0};

    //if(radarComQ.size()<MAX_COMMAND_QUEUE_SIZE)radarComQ.push(command);

    bytes[3] = 0x06;
    sendCommand(&bytes[0],7);
    sendCommand(&bytes[0],7);
    sendCommand(&bytes[0],7);
    bytes[2] = 0x02;
    bytes[3] = 0x00;
    sendCommand(&bytes[0],7);
    sendCommand(&bytes[0],7);
    sendCommand(&bytes[0],7);

    //    if(1)4
    //    {
    //        QFile logFile;
    //        QDateTime now = QDateTime::currentDateTime();
    //        if(!QDir("C:\\logs\\"+now.toString("\\dd.MM\\")).exists())
    //        {
    //            QDir().mkdir("C:\\logs\\"+now.toString("\\dd.MM\\"));
    //        }
    //        logFile.setFileName("C:\\logs\\"+now.toString("\\dd.MM\\")+now.toString("dd.MM-hh.mm.ss")+"_tx_off.log");
    //        logFile.open(QIODevice::WriteOnly);
    //        //logFile.p
    //        logFile.close();

    //    }
}

//void dataProcessingThread::setVaru(bool isOn)
//{

//}

//void dataProcessingThread::setSharu(bool isOn)
//{

//}

//void dataProcessingThread::setBaru(bool isOn)
//{

//}

void dataProcessingThread::sendCommand(unsigned char *commandBuff, short len,bool queued )
{
    // checksum byte calculation
    commandBuff[len-1] = 0;
    for(int i=0;i<len-1;i++)
    {
        commandBuff[len-1]+=commandBuff[i];
    }
    //queued command
    if(queued)
    {
        RadarCommand command;
        memset(&command.bytes[0],0,8);
        memcpy(&command.bytes[0],commandBuff,len);
        if(radarComQ.size()<MAX_COMMAND_QUEUE_SIZE)radarComQ.push(command);
    }
    else// realtime command
    {
        radarSocket->writeDatagram((char*)commandBuff,
                                   len,
                                   QHostAddress("192.168.1.71"),31000
                                   );
        radarSocket->writeDatagram((char*)commandBuff,
                                   len,
                                   QHostAddress("192.168.1.72"),31000
                                   );
        radarSocket->writeDatagram((char*)commandBuff,
                                   len,
                                   QHostAddress("192.168.1.253"),30001
                                   );
        radarSocket->writeDatagram((char*)commandBuff,
                                   len,
                                   QHostAddress("192.168.1.253"),30002
                                   );
        radarSocket->writeDatagram((char*)commandBuff,
                                   len,
                                   QHostAddress("192.168.1.253"),30003
                                   );
        radarSocket->writeDatagram((char*)commandBuff,
                                   len,
                                   QHostAddress("192.168.1.253"),30004
                                   );
                radarSocket->writeDatagram((char*)commandBuff,
                        len,
                        QHostAddress("127.0.0.1"),8001
                        );
    }
}
void dataProcessingThread::startRecord(QString fileName)
{
    //QByteArray array("aa");
    //radarSocket->writeDatagram()
    signRecFile.setFileName(fileName);
    signRecFile.open(QIODevice::WriteOnly);
    isRecording = true;
}
void dataProcessingThread::stopRecord()
{
    signRecFile.close();
    isRecording = false;
}


