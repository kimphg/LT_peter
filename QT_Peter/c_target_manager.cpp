#include "c_target_manager.h"
TrackPointer    *c_target_manager::targetTable = new TrackPointer[TARGET_TABLE_SIZE];
TrackPointer    *c_target_manager::trackTable = new TrackPointer[TRACK_TABLE_SIZE];
c_target_manager::c_target_manager()
{
    memset(trackTable,0,sizeof(TrackPointer)*TRACK_TABLE_SIZE);
    memset(targetTable,0,sizeof(TrackPointer)*TARGET_TABLE_SIZE);
    udpSocketSend = new QUdpSocket();
    initDataGram();

}

c_target_manager::~c_target_manager()
{
    delete [] trackTable;
    delete [] targetTable;
}

void c_target_manager::setCurToEnemy()
{
    if(currTrackPt)
    {
        currTrackPt->flag=1;
    }
}

void c_target_manager::setCurToFriend()
{
    if(currTrackPt)
    {
        currTrackPt->flag=-1;
    }
}

QString c_target_manager::addCurrTrackToTargets()
{
    if(currTrackPt)
    {
        if(currTrackPt->flag<0)return QString::fromUtf8("Không thể đặt chỉ thị mục tiêu gắn cờ ta");
        if(!addTarget(currTrackPt->track))
        {
            return QString::fromUtf8("Được phép đặt chỉ thị tối đa 6 mục tiêu");
        }
    }
    return QString();
}
TrackPointer* c_target_manager::getTrackById(int id)
{
    for(int i=0;i<TRACK_TABLE_SIZE;i++)
    {
        if(trackTable[i].trackUniqID==id)return getTrackAt(i);
    }
    return nullptr;
}
void c_target_manager::setSelectedTrack(int uniqID)
{
    for (uint i = 0;i<TRACK_TABLE_SIZE;i++)
    {

        if(trackTable[i].track !=nullptr)
        {
            trackTable[i].selected=(trackTable[i].track->uniqId==uniqID);
        }
    }
}
TrackPointer* c_target_manager::getTrackAt(int i)
{
    TrackPointer* trackPt = &trackTable[i];
    if(!trackPt->track)return nullptr;
    if(trackPt->track->isRemoved()||(trackPt->trackUniqID!=trackPt->track->uniqId))
    {
        trackPt->track = nullptr;
        trackPt->selected = false;
        trackPt->trackUniqID = 0;
        return nullptr;
    }
    return trackPt;
}

bool c_target_manager::addTarget(C_primary_track * track)
{
    //search for duplication
    for (uint i = 0;i<TARGET_TABLE_SIZE;i++)
    {
        if(targetTable[i].track ==track)
        {
            return true;
        }
    }
    //search for empty slot
    for (uint i = 0;i<TARGET_TABLE_SIZE;i++)
    {
        if(targetTable[i].track == nullptr)
        {
            targetTable[i].track = track;
            targetTable[i].flag = 0;
            targetTable[i].trackUniqID = track->uniqId;
            return true;
        }
    }
    //search for removed slot
    for (uint i = 0;i<TARGET_TABLE_SIZE;i++)
    {
        if(targetTable[i].track !=nullptr)
        {
            if(targetTable[i].track->isRemoved()||(targetTable[i].trackUniqID!=targetTable[i].track->uniqId))
            {
                targetTable[i].track = track;
                targetTable[i].trackUniqID = track->uniqId;
                return true;
            }
        }
    }
    return false;
}

bool c_target_manager::checkIDExist(int id)
{
    //search for empty slot
    for (uint i = 0;i<TRACK_TABLE_SIZE;i++)
    {
        if(trackTable[i].trackUniqID ==id)return true;

    }
    return false;
}

bool c_target_manager::changeCurrTrackID(int id)
{
    //check if id exist
    for (uint i = 0;i<TRACK_TABLE_SIZE;i++)
    {
        if(trackTable[i].track !=nullptr)
        {
            if(trackTable[i].trackUniqID==id)return false;
        }
    }
    if(currTrackPt->track==nullptr)return false;
    currTrackPt->trackUniqID = id;
    currTrackPt->track->uniqId = id;
    return true;

}
bool c_target_manager::addTrack(C_primary_track* track)
{
    //search for empty slot
    for (uint i = 0;i<TRACK_TABLE_SIZE;i++)
    {
        if(trackTable[i].track ==nullptr)
        {
            trackTable[i].track = track;
            trackTable[i].trackUniqID = track->uniqId;
            return true;
        }
    }
    //search for removed slot
    for (uint i = 0;i<TRACK_TABLE_SIZE;i++)
    {
        if(trackTable[i].track !=nullptr)
        {
            if(trackTable[i].track->isRemoved()||(trackTable[i].trackUniqID!=trackTable[i].track->uniqId))
            {
                trackTable[i].track = track;
                track->uniqId = C_primary_track::IDCounter++;
                trackTable[i].trackUniqID = track->uniqId;
                return true;
            }
        }
    }
    return false;
}
TrackPointer* c_target_manager::getTargetById(int id)
{
    for(int i=0;i<TARGET_TABLE_SIZE;i++)
    {
        if(targetTable[i].track==nullptr)continue;
        if(targetTable[i].trackUniqID!=targetTable[i].track->uniqId)
            targetTable[i].trackUniqID = targetTable[i].track->uniqId;
        if(targetTable[i].trackUniqID==id)return getTargetAt(i);
    }
    return nullptr;
}
TrackPointer* c_target_manager::getTargetAt(int i)
{
    TrackPointer* trackPt = &targetTable[i];
    if(!trackPt->track)return nullptr;
    if(trackPt->track->isRemoved())
    {

        trackPt->track = nullptr;
        trackPt->selected = false;
        trackPt->trackUniqID = 0;
        return nullptr;
    }
    if(trackPt->trackUniqID!=trackPt->track->uniqId)trackPt->trackUniqID = trackPt->track->uniqId;
    return trackPt;
}
void c_target_manager::initDataGram()
{
    kasudatagram[0] = 0x16;
    kasudatagram[1] = 0x16;
    kasudatagram[2] = 0x01;
    kasudatagram[3] = 0x16;
    kasudatagram[4] = 0x00;
    kasudatagram[5] = 0x00;//Циклический номер последнего правильного принятого пакета
    kasudatagram[6] = 0x00;//Циклический номер пакета
    kasudatagram[7] = 0x00;
    kasudatagram[8] = 0x16;//Контрольная сумма байт 3-8
    kasudatagram[9] = 0x16;//Контрольная сумма байт 3-8
    //end of header
    memset(&kasudatagram[10],0,22*6);
    kasudatagram[142] = 0x00;
    kasudatagram[143] = 0x00;//Контрольная сумма поля данных
}

void c_target_manager::ClearTargetTable()
{
    for(int i=0;i<6;i++)
    {
        targetTable[i].track=nullptr;
        targetTable[i].trackUniqID = 0;
    }
}
/*
 *
+--------------------------------------------------------------+----------------------+------------+--------------------------------------------------------------+--+
|                                                              |                      |            |                                                              |  |
|   Назначение поля пакета                                     |   Обозначе­ние поля   |   № байта  |   Содержимое поля                                            |  |
|                                                              |                      |            |                                                              |  |
+--------------------------------------------------------------+----------------------+------------+--------------------------------------------------------------+--+
|                                                              |                      |            |                                                              |  |
|   Флаги синхронизации пакета                                 |   SYN                |   1,2      |   0х16 0х16                                                  |  |
|                                                              |                      |            |                                                              |  |
+--------------------------------------------------------------+----------------------+------------+--------------------------------------------------------------+--+
|                                                              |                      |            |                                                              |  |
|   Тип пакета                                                 |   SOH                |   3        |   0х01 - информационный                                      |  |
|                                                              |                      |            |                                                              |  |
+--------------------------------------------------------------+----------------------+------------+--------------------------------------------------------------+--+
|                                                              |                      |            |                                                              |  |
|   Количество байт в поле данных пакета, флаги SYNC, SELECT   |   COUNT              |   4,5      |   0х16 (4-й байт) 0х00 (5-й байт)                            |  |
|                                                              |                      |            |                                                              |  |
+--------------------------------------------------------------+----------------------+------------+--------------------------------------------------------------+--+
|                                                              |                      |            |                                                              |  |
|   Циклический номер последнего правильного принятого пакета  |   RESP               |   6        |   Произвольное (безразличное для комплекса                   |  |
|                                                              |                      |            |   ЗР-60УЭ-1 241 .8)                                          |  |
|                                                              |                      |            |                                                              |  |
+--------------------------------------------------------------+----------------------+------------+--------------------------------------------------------------+--+
|                                                              |                      |            |                                                              |  |
|   Циклический номер данного                                  |   NUM                |   7        |   Циклический номер пакета                                   |  |
|   передаваемого пакета                                       |                      |            |                                                              |  |
|                                                              |                      |            |                                                              |  |
+--------------------------------------------------------------+----------------------+------------+--------------------------------------------------------------+--+
|                                                              |                      |            |                                                              |  |
|   Адрес станции, которой предназначен данный пакет           |   ADDR               |   8        |   Произвольное (безразличное для комплекса ЗР-60УЭ-1241 .8)  |  |
|                                                              |                      |            |                                                              |  |
+--------------------------------------------------------------+----------------------+------------+--------------------------------------------------------------+--+
|                                                              |                      |            |                                                              |  |
|   Контрольная сумма заголовка                                |   BLKCHK1            |   9, 10    |   Контрольная сумма байт 3-8                                 |  |
|                                                              |                      |            |                                                              |  |
+--------------------------------------------------------------+----------------------+------------+--------------------------------------------------------------+--+
|                                                              |                      |            |                                                              |  |
|   Поле данных                                                |   DATA 1             |   11-32    |   Данные о цели № 1                                          |  |
|                                                              |                      |            |                                                              |  |
+--------------------------------------------------------------+----------------------+------------+--------------------------------------------------------------+--+
|                                                              |                      |            |                                                              |  |
|                                                              |   DATA 2             |   33-54    |   Данные                                                     |  |
|                                                              |                      |            |   о цели № 2                                                 |  |
|                                                              |                      |            |                                                              |  |
+--------------------------------------------------------------+----------------------+------------+--------------------------------------------------------------+--+
|                                                              |                      |            |                                                              |  |
|                                                              |   DATA 3             |   55-76    |   Данные о цели № 3                                          |  |
|                                                              |                      |            |                                                              |  |
|                                                              |                      |            |                                                              |  |
+--------------------------------------------------------------+----------------------+------------+--------------------------------------------------------------+--+
|                                                              |                      |            |                                                              |  |
|                                                              |   DATA 4             |   77-98    |   Данные                                                     |  |
|                                                              |                      |            |   о цели № 4                                                 |  |
|                                                              |                      |            |                                                              |  |
+--------------------------------------------------------------+----------------------+------------+--------------------------------------------------------------+--+
|                                                              |                      |            |                                                              |  |
|                                                              |   DATA 5             |   99-120   |   Данные о цели № 5                                          |  |
|                                                              |                      |            |                                                              |  |
|                                                              |                      |            |                                                              |  |
+--------------------------------------------------------------+----------------------+------------+--------------------------------------------------------------+--+
|                                                              |                      |            |                                                              |  |
|                                                              |   DATA 6             |   121-142  |   Данные                                                     |  |
|                                                              |                      |            |   о цели № 6                                                 |  |
|                                                              |                      |            |                                                              |  |
+--------------------------------------------------------------+----------------------+------------+--------------------------------------------------------------+--+
|                                                              |                      |            |                                                              |  |
|   Контрольная сумма поля данных                              |   BLKCHK2            |   143-144  |   Контрольная сумма поля данных                              |  |
|                                                              |                      |            |   - байт 1 1 -1 42                                           |  |
|                                                              |                      |            |                                                              |  |
+--------------------------------------------------------------+----------------------+------------+--------------------------------------------------------------+--+

+-------------+-----------------------+--------------------------+--+--+
|             |                       |                          |          |
|   Параметр  |   Диапазон изменения  |   Цена младшего разряда  |  bytes   |
|             |                       |                          |          |
+-------------+-----------------------+--------------------------+--+--+
|             |                       |                          |   |
|   нц        |   1 -6                |   1                      |  1|
|             |                       |                          |   |
+-------------+-----------------------+--------------------------+--+--+
|             |                       |                          |   |  |
|   Дц        |   100 -260000 м       |   1 м                    | 4 |  |
|             |                       |                          |   |  |
+-------------+-----------------------+--------------------------+--+--+
|             |                       |                          |   |  |
|   Пц        |   0° - 360°           |   (180х2^-14)°           | 2 |  |
|             |                       |                          |   |  |
+-------------+-----------------------+--------------------------+--+--+
|             |                       |                          |   |  |
|   Kц        |   0° - 360°           |   (180х2^-14)°           | 2 |  |
|             |                       |                          |   |  |
+-------------+-----------------------+--------------------------+--+--+
|             |                       |                          |   |  |
|   Vц        |   0 -50 м/с           |   2'8 м/с                | 2 |  |
|             |                       |                          |   |  |
+-------------+-----------------------+--------------------------+--+--+
|             |                       |                          |   |  |
|   СКО Дц    |   0 - 20000 м         |   1 м                    | 2 |  |
|             |                       |                          |   |  |
+-------------+-----------------------+--------------------------+--+--+
|             |                       |                          |   |  |
|   СКО Пц    |   0° - 20°            |   (180х2^-14)°           | 2 |  |
|             |                       |                          |   |  |
+-------------+-----------------------+--------------------------+--+--+
|             |                       |                          |   |  |
|   СКО Кц    |   0° - 20°            |   (180х2^-14)°           |  2|  |
|             |                       |                          |   |  |
+-------------+-----------------------+--------------------------+--+--+
|             |                       |                          |   |  |
|   СКО Vц    |   0 -5 м/с            |   2^-8 м/с               |  2|  |
|             |                       |                          |   |  |
+-------------+-----------------------+--------------------------+--+--+
|             |                       |                          |   |  |
|   Тц        |   0-7                 |   1                      | 1 |  |
|             |                       |                          |   |  |
+-------------+-----------------------+--------------------------+--+--+
|             |                       |                          |   |  |
|   Ty        |   0 - 65535 с         |   1 с                    |  2|  |
|             |                       |                          |   |  |
+-------------+-----------------------+--------------------------+--+--+
*/
void c_target_manager::OutputTargetToKasu()
{
    unsigned char dataPacket[22];
    //targets:
    for(int i=0;i<6;i++)
    {
        C_primary_track* target = targetTable[i].track;
        if(target==nullptr)continue;
        dataPacket[0] = (unsigned char)(i+1);
        //distance
        int distance = int(target->rgKm*1000.0);
        if(distance<100||distance>260000)
        {
            printf("\nTarget %d wrong distance value: %d",i+1,distance);
            distance =100;
        }
        dataPacket[1] = (unsigned char)(distance);
        dataPacket[2] = (unsigned char)(distance>>8);
        dataPacket[3] = (unsigned char)(distance>>16);
        dataPacket[4] = (unsigned char)(distance>>24);
        //peleng
        int peleng = int(target->aziDeg/180.0*16384);
        dataPacket[5] = (unsigned char)(peleng);
        dataPacket[6] = (unsigned char)(peleng>>8);
        int course = int(target->courseRadFit/180.0*16384);
        dataPacket[7] = (unsigned char)(course);
        dataPacket[8] = (unsigned char)(course>>8);
        int speed = int(target->mSpeedkmhFit/3.6*256);
        if(speed>50*256)
        {
            printf("\nTarget %d too big speed value: %d",i+1,speed);
            speed = 50*256;
        }
        dataPacket[9] = (unsigned char)(speed);
        dataPacket[10] = (unsigned char)(speed>>8);
        //sko distance
        int skoD = int(target->sko_rgKm*1000);
        if(skoD>20000)
        {
            printf("\nTarget %d too big sko_rgKm value: %f",i+1,target->sko_rgKm);
            skoD=20000;

        }
        dataPacket[11] = (unsigned char)(skoD);
        dataPacket[12] = (unsigned char)(skoD>>8);
        //sko peleng
        if(target->sko_aziDeg>5)
        {
            printf("\nTarget %d too big sko_aziDeg value: %f",i+1,target->sko_aziDeg);
            target->sko_aziDeg=5;
        }
        int skoPeleng = int(target->sko_aziDeg/180.0*16384);
        dataPacket[13] = (unsigned char)(skoPeleng);
        dataPacket[14] = (unsigned char)(skoPeleng>>8);
        //sko course
        int skoCourse= int(target->sko_cour/180.0*16384);
        dataPacket[15] = (unsigned char)(skoCourse);
        dataPacket[16] = (unsigned char)(skoCourse>>8);
        //sko speed
        int skoSpd = int(target->sko_spd/3.6*256);
        if(skoSpd>5*256)
        {
            printf("\nTarget %d too big skoSpeed value: %f",i+1,target->sko_spd);
            skoSpd = 5*256;
        }
        dataPacket[17] = (unsigned char)(skoSpd);
        dataPacket[18] = (unsigned char)(skoSpd>>8);
        dataPacket[19] = 1;
        //time
        int targetAgeSec = int((CConfig::time_now_ms - target->objectList.back().timeMs)/1000);
        dataPacket[20] = (unsigned char)targetAgeSec;
        dataPacket[21] = (unsigned char)(targetAgeSec>>8);
        memcpy(kasudatagram+10+i*22,dataPacket,22);

    }
    //
    //debug code
    /*for(int i=0;i<6;i++)
    {

    }*/
    udpSocketSend->connectToHost(QHostAddress(CConfig::getString("KASU_IP_1","192.168.1.20")),30000,QIODevice::ReadWrite);
    udpSocketSend->write(reinterpret_cast<char*>(&kasudatagram[0]),KASU_DATA_SIZE);
    udpSocketSend->connectToHost(QHostAddress(CConfig::getString("KASU_IP_2","192.168.1.21")),30000,QIODevice::ReadWrite);
    udpSocketSend->write(reinterpret_cast<char*>(&kasudatagram[0]),KASU_DATA_SIZE);
    udpSocketSend->connectToHost(QHostAddress(CConfig::getString("KASU_IP_3","192.168.1.72")),30000,QIODevice::ReadWrite);
    udpSocketSend->write(reinterpret_cast<char*>(&kasudatagram[0]),KASU_DATA_SIZE);
    udpSocketSend->disconnectFromHost();
}
