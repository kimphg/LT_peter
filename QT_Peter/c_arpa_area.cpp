#include "c_arpa_area.h"
static QPen penTargetHistory(QBrush(Qt::gray),2);
static QPen penTargetEnemy(QBrush(Qt::magenta),3);
static QPen penTargetFriend(QBrush(QColor(0,200,200 ,255)),3);
static QPen penTargetEnemySelected(QBrush(Qt::magenta),4);
static QPen penTargetFriendSelected(QBrush(QColor(50,255,255 ,255)),5);
static QPen penCyan(QBrush(QColor(50,255,255 ,255)),1);//xoay mui tau


void C_arpa_area::checkClickAIS(int xclick, int yclick)
{
    for(std::map<int,AIS_object_t>::iterator iter = processing->mAisData.begin();
        iter!=processing->mAisData.end();iter++)
    {
        AIS_object_t *aisObj = &(iter->second);
        //if(aisObj->isSelected)continue;
        if(aisObj->isMatchToRadarTrack)continue;

        PointInt  pt = ConvWGSToScrPoint(aisObj->mLong,aisObj->mLat);
        if(abs(pt.x-xclick)<target_size/2&&abs(pt.y-yclick)<target_size/2)
        {

            dialogTargetInfo->setDataSource(aisObj,0);
            break;
        }

    }
}
void C_arpa_area::DrawAISMark(PointInt s ,double head,QPainter *p,QString name,int size,int vectorLen)
{

    QPolygon poly;
    QPoint point;
    //    double head = aisObj.mCog*PI_NHAN2/360.0;
    point.setX(s.x+size*sinFast(head));
    point.setY(s.y-size*cosFast(head));
    poly<<point;
    point.setX(s.x+size*sinFast(head+2.3562f));
    point.setY(s.y-size*cosFast(head+2.3562f));
    poly<<point;
    point.setX(s.x+size*sinFast(head-2.3562f));
    point.setY(s.y-size*cosFast(head-2.3562f));
    poly<<point;
    point.setX(s.x+size*sinFast(head));
    point.setY(s.y-size*cosFast(head));
    poly<<point;
    point.setX(s.x+vectorLen*sinFast(head));
    point.setY(s.y-vectorLen*cosFast(head));
//    poly<<point;


    p->setPen(penTargetEnemy);
    p->drawPolygon(poly);
    p->drawLine(s.x,s.y,point.x(),point.y());
    if(showAisName)
    {
        p->setFont(QFont("Times", 12));
        p->drawText(s.x+size,s.y+size,250,30,0,name);
    }


}
bool C_arpa_area::isInsideViewRect(int x, int y)
{
    return rect.contains(x,y);
}

void C_arpa_area::drawAisTarget(QPainter *p)
{
    p->setBrush(Qt::NoBrush);
    for(std::map<int,AIS_object_t>::iterator iter = processing->mAisData.begin();iter!=processing->mAisData.end();iter++)
    {
        AIS_object_t aisObj = iter->second;

        if(aisObj.getAge()>300000)continue;
        PointInt s = ConvWGSToScrPoint(aisObj.mLong,aisObj.mLat);
        if(aisObj.isMatchToRadarTrack)continue;
        if(!isInsideViewRect(s.x,s.y))continue;
        int vectorLen = nm2km(aisObj.mSog)*mScale/6.0;
        DrawAISMark(s,radians(aisObj.mCog)+radians(trueShiftDeg),p,aisObj.mName,target_size,vectorLen);
    }
}
C_arpa_area::C_arpa_area()
{

}

void C_arpa_area::setScale(double scale)
{
    mScale = scale;
    setTarget_size( 0.5*mScale);
}

void C_arpa_area::setTarget_size(int value)
{
    isDrawTargetNumber = mScale>10;
    target_size = value;
    if(target_size<8)target_size=8;
    else if(target_size>20)target_size=20;
    int penSize = target_size/4;
    if(penSize<1)penSize=1;
    penTargetHistory          .setWidth(penSize);
    penTargetEnemy            .setWidth(penSize);
    penTargetFriend           .setWidth(penSize);
    penTargetEnemySelected    .setWidth(penSize);
    penTargetFriendSelected   .setWidth(penSize);
    penCyan                   .setWidth(penSize);
}

PointInt C_arpa_area::ConvWGSToScrPoint(double m_Long, double m_Lat)
{
    PointInt s;
    double refLat = (mLat + (m_Lat))*0.00872664625997;//pi/360
    s.x	= mScale*(((m_Long) - mLon) * 111.31949079327357)*cos(refLat);// 3.14159265358979324/180.0*6378.137);//deg*pi/180*rEarth
    s.y	= mScale*((mLat - (m_Lat)) * 111.132954);
    rotateVector(trueShiftDeg,&s.x,&s.y);
    s.x   += radCtX;
    s.y   += radCtY;
    return s;
}

C_primary_track* C_arpa_area::checkClickRadarTarget(int xclick, int yclick)
{
    int minDistanceToCursor = target_size/2;
    int trackSel=-1;
    for (uint i = 0;i<MAX_TRACKS_COUNT;i++)
    {
        C_primary_track* track = &(mRadarData->mTrackList[i]);
        if(track->mState!=TrackState::confirmed)continue;
        track->isSelected = false;
        PointInt s = ConvWGSToScrPoint(track->lon,track->lat);
        int dsx   = abs(s.x - xclick);
        int dsy   = abs(s.y - yclick);
        if(dsx<minDistanceToCursor&&dsy<minDistanceToCursor)
        {
            minDistanceToCursor = dsx+dsy;
            //trackMin = track->uniqId;
            trackSel = i;
            //                tracktime = track->time;
        }
    }
    if(trackSel>=0)
    {
        C_primary_track* track = &(mRadarData->mTrackList[trackSel]);
        track->isSelected = true;
        dialogTargetInfo->setDataSource(0,track);
        return track;
    }
    return 0;
}

void C_arpa_area::DrawRadarTargets(QPainter* p)//draw radar target from pRadar->mTrackList
{
    p->setFont(QFont("Times", 8));
    //draw location history of own ship
    p->setPen(penCyan);
    std::vector<std::pair<double, double> > *locationHistory = CConfig::GetLocationHistory();
    if(locationHistory->size())
    {
        PointInt s0 = ConvWGSToScrPoint(locationHistory->at(0).first,locationHistory->at(0).second);

        for(int i=0;i<locationHistory->size();i++)
        {
            PointInt s1 = ConvWGSToScrPoint(locationHistory->at(i).first,locationHistory->at(i).second);
            p->drawLine(s0.x,s0.y,s1.x,s1.y);
            s0=s1;
        }
    }
    //draw unconfirmed new datection
    for (uint i = 0;i<mRadarData->mTrackList.size();i++)
    {
        C_primary_track* track = &(mRadarData->mTrackList[i]);
        if(track->mState==TrackState::newDetection&&track->isUserInitialised)
        {
            PointInt sTrack = ConvWGSToScrPoint(track->objectList.back().lon,track->objectList.back().lat);
            //p->drawPoint(sTrack.x,sTrack.y);
            p->drawRect(sTrack.x-target_size/2,sTrack.y-target_size/2,target_size,target_size);
            //p->drawText(sTrack.x+10,sTrack.y+10,100,50,0,QString::number(track->objectList.size()));
        }
    }


    bool blink = (CConfig::time_now_ms/500)%2;
#ifndef THEON
    //draw targeted tracks
    p->setPen(penTargetEnemy);
    for (uint i = 0;i<TARGET_TABLE_SIZE;i++)
    {
        TrackPointer* trackPt = mTargetMan.getTargetAt(i);
        if(!trackPt)continue;
        C_primary_track* track = trackPt->track;
        PointInt s = ConvWGSToScrPoint(track->lon,track->lat);
        p->drawLine(s.x-20,s.y,s.x-10,s.y);
        p->drawLine(s.x+20,s.y,s.x+10,s.y);
        p->drawLine(s.x,s.y-20,s.x,s.y-10);
        p->drawLine(s.x,s.y+20,s.x,s.y+10);
    }
#endif
    //draw all tracks
    for (uint i = 0;i<MAX_TRACKS_COUNT;i++)
    {
        C_primary_track* track = &(mRadarData->mTrackList[i]);
        if(track->mState==TrackState::removed)continue;
        if(track->mState==TrackState::newDetection)continue;
        PointInt sTrack = ConvWGSToScrPoint(track->lon,track->lat);
        if(track->isSelected)//selected
        {
            // draw track history
            p->setPen(penTargetHistory);
            object_t* obj1 = &(track->objectHistory[0]);
            for (int j = 1;j<track->objectHistory.size();j++)
            {
                object_t* obj2 = &(track->objectHistory[j]);

                PointInt s = ConvWGSToScrPoint(obj1->lon,obj1->lat);
                PointInt s1 = ConvWGSToScrPoint(obj2->lon,obj2->lat);
                p->drawLine(s1.x,s1.y,s.x,s.y);
                obj1 = obj2;
            }
            object_t *obj2  = &(track->objectList.back());
            PointInt s      = ConvWGSToScrPoint(obj1->lon,obj1->lat);
            PointInt s1     = ConvWGSToScrPoint(obj2->lon,obj2->lat);
            p->drawLine(s1.x,s1.y,s.x,s.y);

            if(track->isEnemy)p->setPen(penTargetEnemySelected);
            else  p->setPen(penTargetFriendSelected);
        }
        else
        {

            if(track->isEnemy)p->setPen(penTargetEnemy);
            else  p->setPen(penTargetFriend);
        }
        if(track->isLost())
        {

            if(blink)
            {
                p->drawRect(sTrack.x-target_size/2,sTrack.y-target_size/2,target_size,target_size);
                p->drawLine(sTrack.x-target_size/2,sTrack.y-target_size/2,sTrack.x+target_size,sTrack.y+target_size);

            }
            p->drawText(sTrack.x+target_size/2+1,sTrack.y+target_size/2+1,100,50,0,QString::number(track->uniqId));
            continue;
        }
        //all targets
        else
        {
            int size = 10000.0/(CConfig::time_now_ms - track->lastUpdateTimeMs+400);
            if(size<target_size)size=target_size;//rect size depend to time
            if(track->mAisConfirmedObj==0)
            {
                p->drawEllipse(sTrack.x-size/2,sTrack.y-size/2,size,size);
            }
            else {
                //p->setPen(penSelTarget);
                p->drawRect(sTrack.x-size/2,sTrack.y-size/2,size,size);
            }
            // draw speed vector
            int vectorLen = track->mSpeedkmhFit*mScale/6.0;
            int sx = sTrack.x+short(vectorLen*sinFast(track->courseRadFit+radians(trueShiftDeg)));
            int sy = sTrack.y-short(vectorLen*cosFast(track->courseRadFit+radians(trueShiftDeg)));
            p->drawLine(sx,sy,sTrack.x,sTrack.y);

            //draw target number
            if(isDrawTargetNumber)p->drawText(sTrack.x+target_size/2+1,sTrack.y+target_size/2+1,100,50,0,QString::number(track->uniqId));
        }

    }



}
