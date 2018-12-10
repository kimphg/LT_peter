
#include "c_radar_thread.h"

#define MAX_IREC 500
//#include <QGeoCoordinate>
#include <QNmeaPositionInfoSource>
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
//    delete arpaData;
}

void dataProcessingThread::ReadDataBuffer()
{
    if(iRec!=iRead)
    {
        connect_timeout = 4;
    }else
    {
        if(connect_timeout)connect_timeout--;
    }
    short nread = 0;
    while(iRec!=iRead)
    {
        nread++;

        if(nread>=300)
        {
            mRadarData->resetData();
            break;
        }
        uchar *pData = &(dataB[iRead].data[0]);
        unsigned short dataLen = dataB[iRead].len;
        if(!isPlaying)
        {
            mRadarData->processSocketData(pData,dataLen);

            if(isRecording)
            {
                signRecFile.write((char*)&dataLen,2);
                signRecFile.write((char*)pData,dataLen);
            }
        }
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
    mRadMode = ModeComplexSignal;
    mAntennaAzi = 0;
    failureCount = 0;
    isHeadingAvaible=false;
    selsynEncoderAzi = 0;
    isXuLyThuCap = false;
    dataBuff = &dataB[0];
    iRec=0;iRead=0;
    connect_timeout = 0;
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
    connect(&commandSendTimer, &QTimer::timeout, this, &dataProcessingThread::PushCommandQueue);
    commandSendTimer.start(100);
    connect(&readUdpBuffTimer, &QTimer::timeout, this, &dataProcessingThread::ReadDataBuffer);
    readUdpBuffTimer.start(10);
    initSerialComm();
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
void dataProcessingThread::ProcessNavData(unsigned char *mReceiveBuff,int len)
{
    if(len<7)return;
    if(mReceiveBuff[0]==0xaa&&mReceiveBuff[1]==0x55)//system messages
    {
        if(mReceiveBuff[2]==0x65)// trang thai may 2-2
        {
            mRadarStat.ReadStatusMessage(&mReceiveBuff[4]);
        }
    }
    else if(mReceiveBuff[0]==0x5a&&mReceiveBuff[1]==0xa5&&mReceiveBuff[31]==0xAA&&len>=32)//gyro messages
    {
        CConfig::shipHeadingDeg = (((mReceiveBuff[6])<<8)|mReceiveBuff[7])/182.044444444;//*360.0/65535.0;
        int vy = (((mReceiveBuff[18])<<8)|mReceiveBuff[19]);
        int vx = (((mReceiveBuff[20])<<8)|mReceiveBuff[21]);
        CConfig::shipSpeed = sqrt(vy*vy+vx*vx)*0.00388768898488120950323974082073;//*2/1000000/CONST_NM*36000; kn
    }
    else if(mReceiveBuff[0]=='!'&&mReceiveBuff[1]=='A')//AIS
    {
        processARPAData(QByteArray((char*)mReceiveBuff,len));
    }
    else if((mReceiveBuff[0]=='$')&&(mReceiveBuff[1]=='G')&&(mReceiveBuff[2]=='P'))//GPS
    {
        for(int i = 0;i<len;i++)
        {
            if(mGPS.decode(mReceiveBuff[i]))
            {
               GPSData newGPSPoint;
               mGPS.get_position(&(newGPSPoint.lat),&(newGPSPoint.lon));
               newGPSPoint.heading =  mGPS.course()/100.0;
               while(mGpsData.size()>10)
               {
                   mGpsData.pop();
               }
               mGpsData.push(newGPSPoint);
               if(mGpsData.size()>5)
               {
                   double dLat = mGpsData.back().lat - mGpsData.front().lat;
                   double dLon = mGpsData.back().lon - mGpsData.front().lon;
                   newGPSPoint.heading = degrees(ConvXYToAziRd(dLon,dLat));
               }
            }

        }
    }

    /*CGPSParser gpsParser(data.toStdString());
    if(gpsParser.isDataValid)
    {
        if(gpsParser.latitude)
        {

            if(gpsParser.longitude)
            {
                GPSData newGPSPoint;

                newGPSPoint.lat = gpsParser.latitude;
                newGPSPoint.lon = gpsParser.longitude;
                newGPSPoint.isFixed = true;
                while(mGpsData.size()>10)
                {
                    mGpsData.pop();
                }
                if(gpsParser.heading)
                {
                    newGPSPoint.heading = gpsParser.heading;
                    //printf("\nheading:%f",newGPSPoint.heading);
                }
                else if(mGpsData.size()>3)
                {
                    double dLat = mGpsData.back().lat - mGpsData.front().lat;
                    double dLon = mGpsData.back().lon - mGpsData.front().lon;
                    if(dLat!=0)
                    {
                        newGPSPoint.heading = atan(dLon/dLat)/3.1415926535489*180.0;
                        if(dLat<0)newGPSPoint.heading+=180;
                    }
                    else
                        newGPSPoint.heading = 180-90*(dLon>0);
                    if(newGPSPoint.heading<0)newGPSPoint.heading+=360;
                    if(newGPSPoint.heading>360)newGPSPoint.heading-=360;
                }
                else newGPSPoint.heading = 0;
                mGpsData.push(newGPSPoint);
            }
        }
    }*/
}
void dataProcessingThread::initSerialComm()
{
    int serialBaud ;


    // baudrate at 4800 standart for low speed encoder and ais
    serialBaud = 4800;
    QList<QSerialPortInfo> portlist = QSerialPortInfo::availablePorts();
    for(int i = 0;i<portlist.size();i++)
    {
        if(!portlist.at(i).isBusy())
        {
            QSerialPort *newport = new QSerialPort(this);
            QString qstr = portlist.at(i).portName();
            newport->setPortName(qstr);
            newport->setBaudRate(serialBaud);
            newport->open(QIODevice::ReadWrite);
            serialPorts.push_back(newport);
            printf("\nOpen serial %d:",portlist.size());
            printf((char*)qstr.data());
        }
    }

}
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
        /*if(!geoLocation)
        {
            if(responseData.contains("$GP")&&(!responseData.contains("HDT")))
            {
                geoLocation = new QNmeaPositionInfoSource(QNmeaPositionInfoSource::RealTimeMode,this);
                QString nameport( (*it)->portName() );
                (*it)->close();
                serialPorts.erase(it);
                QSerialPort *newport = new QSerialPort();
                newport->setPortName(nameport);
                newport->setBaudRate(9600);
                newport->open(QIODevice::ReadWrite);
                geoLocation->setDevice(newport);
                geoLocation->startUpdates();
                //                //connect(&geoLocation, QNmeaPositionInfoSource::positionUpdated(), this, &dataProcessingThread::gpsupdate);
                connect(geoLocation, SIGNAL(positionUpdated(QGeoPositionInfo)),
                        this, SLOT(gpsupdate(QGeoPositionInfo)));
                return;

            }
        }*/
        if(responseData.size())
        {
            processSerialData(responseData);
        }

    }
}
void dataProcessingThread::processSerialData(QByteArray inputData)
{

    unsigned short len = inputData.length();
    unsigned char* data = (unsigned char*)inputData.data();
    if(isRecording)
    {
        signRecFile.write((char*)&len,2);
        signRecFile.write((char*)data,len);
    }
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

        processARPAData(inputData);
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
void dataProcessingThread::sendAziData()
{
    unsigned char sendBuf[9];
    sendBuf[0]=0xaa;
    sendBuf[1]=0x55;
    sendBuf[2]=0x6e;
    sendBuf[3]=0x09;
    int azi = (mRadarData->getCurAziRad()/3.1415926535*2048.0);
    sendBuf[4]=azi>>8;
    sendBuf[5]=azi;
    int bearing =0;
    sendBuf[6]=bearing>>8;
    sendBuf[7]=bearing;
    sendBuf[8]=0;
    sendCommand(&sendBuf[0],9,false);
}
void dataProcessingThread::PushCommandQueue()
{
    sendAziData();
    if(radarComQ.size())
    {
        if(radarComQ.front().bytes[1]==0xab)
        {
            radarSocket->writeDatagram((char*)&radarComQ.front().bytes[0],
                    8,
                    QHostAddress("192.168.0.44"),2572
                    );
            radarSocket->writeDatagram((char*)&radarComQ.front().bytes[0],
                    8,
                    QHostAddress("127.0.0.1"),30002
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
                //togglePlayPause(false);
                return;
            }
            QByteArray buff;
            buff.resize(len);

            signRepFile.read(buff.data(),len);
            if(len>300){
                //mRadarData->assembleDataFrame((unsigned char*)buff.data(),buff.size());
                mRadarData->processSocketData((unsigned char*)buff.data(),len);
            }
            else if(len)
                ProcessNavData((unsigned char*)buff.data(),len);
            if(isRecording)
            {
                signRecFile.write((char*)&len,2);
                signRecFile.write(buff.data(),len);
            }
            if(playRate<10){togglePlayPause(false);return;}
        }
        return;
    }
}

void dataProcessingThread::StopProcessing()
{

    deleteLater();
}
void dataProcessingThread::setIsDrawn(bool value)
{
    //isDrawn = value;
}

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
QString messageStringbuffer;
void dataProcessingThread::processARPAData(QByteArray inputdata)
{
    messageStringbuffer.append(QString::fromLatin1(inputdata));
    if(messageStringbuffer.size()>100)messageStringbuffer = "";
    //printf(inputdata.data());
    QStringList strlist = messageStringbuffer.split("!");
    if(strlist.size() <= 1)return;

    for(int i = 0;i<strlist.size()-1;i++)
        if(aisMessageHandler.ProcessNMEA(strlist.at(i)))
        {
            AIS_object_t obj = aisMessageHandler.GetAisObject(); ;
            QMutableListIterator<AIS_object_t> i(m_aisList);
            int elecount = 0;
            bool objExist = false;
            while (i.hasNext())
            {
                AIS_object_t oldObj = i.next();
                elecount++;
                if(elecount>3000){i.remove();continue;}
                if(obj.mMMSI==oldObj.mMMSI)
                {
                    objExist = true;
                    oldObj.isNewest = false;
                    obj.isSelected = oldObj.isSelected;
                    if(obj.mName.isEmpty()&&(!oldObj.mName.isEmpty()))
                        obj.mName = oldObj.mName;
                    if(obj.mLat==0)obj.mLat = oldObj.mLat;
                    if(obj.mLong==0)obj.mLong = oldObj.mLong;
                    if(obj.mDst.isEmpty())
                        obj.mDst       = oldObj.mDst;
                    if(!obj.mImo)
                        obj.mImo       = oldObj.mImo;
                    if(!obj.mType)
                        obj.mType      = oldObj.mType;
                    if(!obj.mBow)
                        obj.mBow       = oldObj.mBow;
                    if(!obj.mStern)
                        obj.mStern     = oldObj.mStern;
                    if(!obj.mStarboard)
                        obj.mStarboard = oldObj.mStarboard;
                    if(!obj.mPort)
                        obj.mPort      = oldObj.mPort;
                    if(!obj.mSog)
                        obj.mSog       = oldObj.mSog;
                    if(!obj.mCog)
                        obj.mCog       = oldObj.mCog;
                    if(!obj.mCog)
                    {
                        double heading, speed;
                        C_radar_data::kmxyToPolarDeg(obj.mLat-oldObj.mLat,obj.mLong-oldObj.mLong,&heading,&speed);
                        obj.mCog       =    heading;
                    }
                    i.setValue(obj);
                    break;
                }

            }
            if(!objExist)m_aisList.push_front(obj);

        }
    messageStringbuffer=strlist.at(strlist.size()-1);

    return;
}
void dataProcessingThread::processRadarData()
{

}

static uchar mReceiveBuff[1000];
void dataProcessingThread::run()
{

    while(true)
    {
        while(radarSocket->hasPendingDatagrams())
        {
            int len = radarSocket->pendingDatagramSize();
            if(len<1000)// system packets
            {
                radarSocket->readDatagram((char*)&mReceiveBuff[0],len);

                ProcessNavData((unsigned char*)mReceiveBuff,len);
            }
            else if(len<=MAX_FRAME_SIZE)
            {
                dataB[iRec].len = len;
                radarSocket->readDatagram(( char*)&(dataB[iRec].data[0]),len);
                iRec++;
                if(iRec>=MAX_IREC)iRec = 0;
            }

        }
    }

}

bool dataProcessingThread::getIsDrawn()
{
    if(!mRadarData->isDrawn){mRadarData->isDrawn = true;return false;}
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
    //    if(1)
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
//        radarSocket->writeDatagram((char*)commandBuff,
//                len,
//                QHostAddress("127.0.0.1"),30002
//                );
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


radarStatus_3C::radarStatus_3C()
{
    isStatChange = false;
}

radarStatus_3C::~radarStatus_3C()
{

}
