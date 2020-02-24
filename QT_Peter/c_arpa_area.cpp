#include "c_arpa_area.h"
static QPen penTargetHistory(QBrush(Qt::gray),2);
static QPen penTargetEnemy(QBrush(Qt::magenta),3);
static QPen penYellow(QBrush(Qt::yellow),1);
static QPen penTargetFriend(QBrush(QColor(0,200,200 ,255)),3);
static QPen penTargetEnemySelected(QBrush(Qt::magenta),4);
static QPen penTargetFriendSelected(QBrush(QColor(50,255,255 ,255)),5);
static QPen penCyan(QBrush(QColor(50,255,255 ,255)),1);//xoay mui tau


void C_arpa_area::checkClickAIS(int xclick, int yclick)
{
    for(std::map<int,AIS_object_t>::iterator iter = processing->mAisVesselsList.begin();
        iter!=processing->mAisVesselsList.end();iter++)
    {
        AIS_object_t *aisObj = &(iter->second);
        //if(aisObj->isSelected)continue;
        if(aisObj->isMatchToRadarTrack)continue;

        PointDouble  pt = ConvWGSToScrPoint(aisObj->mLong,aisObj->mLat);
        if(abs(pt.x-xclick)<target_size/2&&abs(pt.y-yclick)<target_size/2)
        {

            dialogTargetInfo->setDataSource(aisObj,0);
            break;
        }

    }
}
void C_arpa_area::DrawAISBuoy(PointDouble s ,QPainter *p,QString name,int size)
{
    QPolygon poly;
    QPoint point;
    point.setX(s.x);
    point.setY(s.y-size*0.5);
    poly<<point;
    point.setX(s.x-size*0.2);
    point.setY(s.y+size*0.2);
    poly<<point;
    point.setX(s.x+size*0.2);
    point.setY(s.y+size*0.2);
    poly<<point;
    p->setPen(penYellow);
    p->drawPolygon(poly);

}
void C_arpa_area::DrawAISMark(PointDouble s ,double head,QPainter *p,QString name,int size,int vectorLen)
{
    head = radians(head)+radians(trueShiftDeg);
    QPolygon poly;
    QPoint point;
    //    double head = aisObj.mCog*PI_NHAN2/360.0;
    point.setX(s.x+size*sin(head));
    point.setY(s.y-size*cos(head));
    poly<<point;
    point.setX(s.x+size*sin(head+2.3562f));
    point.setY(s.y-size*cos(head+2.3562f));
    poly<<point;
    point.setX(s.x+size*sin(head-2.3562f));
    point.setY(s.y-size*cos(head-2.3562f));
    poly<<point;
    point.setX(s.x+size*sin(head));
    point.setY(s.y-size*cos(head));
    poly<<point;
    point.setX(s.x+vectorLen*sin(head));
    point.setY(s.y-vectorLen*cos(head));
//    poly<<point;

    p->drawPolygon(poly);

    if(size>10)
    {
        p->drawLine(s.x,s.y,point.x(),point.y());
        p->setFont(QFont("Times", 12));
        p->drawText(s.x+size,s.y+size,250,30,0,name);
    }


}
void C_arpa_area::initMarkPolygons()
{

    QPoint point;

    //   initPolyplane
    point.setX(+0);                   point.setY(-150);   polyPlane<<point;
    point.setX(+25);    point.setY(-120);   polyPlane<<point;
    point.setX(+25);    point.setY(-30);   polyPlane<<point;
    point.setX(+150);     point.setY(+60);   polyPlane<<point;
    point.setX(+150);     point.setY(+90);   polyPlane<<point;
    point.setX(+25);    point.setY(+30);   polyPlane<<point;
    point.setX(+25);    point.setY(+120);   polyPlane<<point;
    point.setX(+60);     point.setY(+150);   polyPlane<<point;
    point.setX(+60);     point.setY(+180);   polyPlane<<point;
    //                                                                polyPlane
    point.setX(+0);       point.setY(+150);   polyPlane<<point;
    //                                                                polyPlane
    point.setX(-60);     point.setY(+180);   polyPlane<<point;
    point.setX(-60);     point.setY(+150);   polyPlane<<point;
    point.setX(-25);    point.setY(+120);   polyPlane<<point;
    point.setX(-25);    point.setY(+30);   polyPlane<<point;
    point.setX(-150);     point.setY(+90);   polyPlane<<point;
    point.setX(-150);     point.setY(+60);   polyPlane<<point;
    point.setX(-25);    point.setY(-30);   polyPlane<<point;
    point.setX(-25);    point.setY(-120);   polyPlane<<point;
    point.setX(+0);                   point.setY(-150);   polyPlane<<point;
}

void C_arpa_area::DrawPlaneMark(PointDouble s ,QPainter *p,double head, QString name,int size)
{
    double viewhead = head+trueShiftDeg;
//    poly<<point;
    QPolygon poly = QTransform()
            .translate(s.x, s.y)
            .rotate(viewhead)
            .scale(target_size/100.0,target_size/100.0)
            .map(polyPlane);

    p->setPen(penYellow);
    p->drawPolygon(poly);
    //printf("\nviewhead:%f",viewhead);
    if(size>10)
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
    p->setBrush(QBrush(penTargetEnemy.color()));
    p->setPen(penTargetEnemy);
    for(std::map<int,AIS_object_t>::iterator iter = processing->mAisVesselsList.begin();iter!=processing->mAisVesselsList.end();iter++)
    {
        AIS_object_t aisObj = iter->second;

        if(aisObj.getAge()>1200000)continue;
        PointDouble s = ConvWGSToScrPoint(aisObj.mLong,aisObj.mLat);
        if(aisObj.isMatchToRadarTrack)continue;
        if(!isInsideViewRect(s.x,s.y))continue;
        int vectorLen = nm2km(aisObj.mSog)*mScale/6.0;
        if(vectorLen>target_size*10)vectorLen=target_size*10;
        DrawAISMark(s,aisObj.mCog,p,aisObj.mName,target_size,vectorLen);
    }
    p->setBrush(QBrush(penYellow.color()));
    p->setPen(penYellow);
    for(std::map<QString,AIS_object_t>::iterator iter = processing->mAisObjList.begin();iter!=processing->mAisObjList.end();iter++)
    {
        AIS_object_t aisObj = iter->second;

        PointDouble s = ConvWGSToScrPoint(aisObj.mLong,aisObj.mLat);
        if(!isInsideViewRect(s.x,s.y))continue;
        DrawAISBuoy(s,p,aisObj.mName,target_size);
    }
    //
    for(const auto&kv:processing->mPlaneList)
    {
        C_AIR_TRACK plane = kv.second;
        PointDouble s = ConvWGSToScrPoint(plane.mlon,plane.mlat);
        if(!isInsideViewRect(s.x,s.y))continue;
        DrawPlaneMark(s,p,plane.mhead,plane.registrationName,target_size);
    }
}
C_arpa_area::C_arpa_area()
{
    initMarkPolygons();

}

void C_arpa_area::setScale(double scale)
{
    mScale = scale;
    setTarget_size( 0.4*mScale);
}

void C_arpa_area::setTarget_size(int value)
{
    isDrawTargetNumber = mScale>10;
    target_size = value;
    if(target_size<3)target_size=3;
    else if(target_size>12)target_size=12;
    int penSize = 1;
    penTargetHistory          .setWidth(penSize);
    penTargetEnemy            .setWidth(penSize);
    penTargetFriend           .setWidth(penSize);
    penTargetEnemySelected    .setWidth(penSize);
    penTargetFriendSelected   .setWidth(penSize);
    penCyan                   .setWidth(penSize);
}

PointDouble C_arpa_area::ConvWGSToScrPoint(double m_Long, double m_Lat)
{
    PointDouble s;
    double refLat = (mLat + (m_Lat))*0.00872664625997;//pi/360
    s.x	= mScale*(((m_Long) - mLon) * 111.31949079327357)*cos(refLat);// 3.14159265358979324/180.0*6378.137);//deg*pi/180*rEarth
    s.y	= mScale*((mLat - (m_Lat)) * 111.132954);
    rotateVector(trueShiftDeg,&s.x,&s.y);
    s.x   += radCtX;
    s.y   += radCtY;
    return s;
}
C_SEA_TRACK* C_arpa_area::MouseOverRadarTarget(int xclick, int yclick)
{
    double minDistanceToCursorSq = sq(target_size)/4.0;
    C_SEA_TRACK* trackSel=0;
    for (uint i = 0;i<MAX_TRACKS_COUNT;i++)
    {
        C_SEA_TRACK* track = &(mRadarData->mTrackList[i]);
        if(track->mState!=TrackState::confirmed)continue;
//        track->isMouseOver = false;
        PointDouble s = ConvWGSToScrPoint(track->lon,track->lat);
        double distanceSQ = sq(s.x - xclick)+sq(s.y - yclick);

        if(distanceSQ<minDistanceToCursorSq)
        {
            minDistanceToCursorSq = distanceSQ;
            if(trackSel)
            {
                trackSel->isMouseOver=false;
            }
            track->isMouseOver=true;
            trackSel = track;
            //                tracktime = track->time;
        }
        else
        {
            track->isMouseOver=false;
//            s = ConvWGSToScrPoint(track->lon,track->lat);
//            printf("\nint xclick:%d, int yclick:%d, target_size:%d, targetx:%d",xclick,yclick,target_size,s.x);
        }
    }

    return trackSel;
}
PointDouble C_arpa_area::ConvScrPointToWGS(int x,int y)
{
    PointDouble output;
    output.y  = mLat -  ((y-radCtY)/mScale)/(111.132954);
    double refLat = (mLat +(output.y))*0.00872664625997;//3.14159265358979324/180.0/2;
    output.x = (x-radCtX)/mScale/(111.31949079327357*cos(refLat))+ mLon;
    return output;
}
PointDouble C_arpa_area::ConvScrPointToKMXY(int x, int y)
{
    PointDouble output;
    output.x = (x-radCtX)/mScale;
    output.y = -(y-radCtY)/mScale;
    C_arpa_area::rotateVector(trueShiftDeg,&output.x,&output.y);
    return output;
}
PointAziRgkm C_arpa_area::ConvScrPointToAziRgkm (int x, int y)
{
    PointDouble p;
    PointAziRgkm ouput;
    p.x = (x-radCtX)/mScale;
    p.y = -(y-radCtY)/mScale;
    C_arpa_area::rotateVector(trueShiftDeg,&p.x,&p.y);
    ouput.aziRad = ConvXYToAziRd(p.x,p.y);
    ouput.rg = sqrt(p.x*p.x+p.y*p.y);;
    return ouput;
}

C_SEA_TRACK* C_arpa_area::SelectRadarTarget(int xclick, int yclick)
{
    double minDistanceToCursorSq = sq(target_size)/4.0;
    int trackSel=-1;
    for (uint i = 0;i<MAX_TRACKS_COUNT;i++)
    {
        C_SEA_TRACK* track = &(mRadarData->mTrackList[i]);
        if(track->mState!=TrackState::confirmed)continue;
        track->isSelected = false;
        PointDouble s = ConvWGSToScrPoint(track->lon,track->lat);
        double distanceSQ = sq((s.x - xclick))+sq((s.y - yclick));
        if(distanceSQ<minDistanceToCursorSq)
        {
            minDistanceToCursorSq = distanceSQ;
            //trackMin = track->uniqId;
            trackSel = i;
            //                tracktime = track->time;
        }
    }
    if(trackSel>=0)
    {
        C_SEA_TRACK* track = &(mRadarData->mTrackList[trackSel]);
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
        PointDouble s0 = ConvWGSToScrPoint(locationHistory->at(0).first,locationHistory->at(0).second);

        for(int i=0;i<locationHistory->size();i++)
        {
            PointDouble s1 = ConvWGSToScrPoint(locationHistory->at(i).first,locationHistory->at(i).second);
            p->drawLine(s0.x,s0.y,s1.x,s1.y);
            s0=s1;
        }
    }
    //draw unconfirmed new datection
    for (uint i = 0;i<mRadarData->mTrackList.size();i++)
    {
        C_SEA_TRACK* track = &(mRadarData->mTrackList[i]);
        if(track->mState==TrackState::newDetection&&track->isUserInitialised)
        {
            PointDouble sTrack = ConvWGSToScrPoint(track->objectList.back().lon,track->objectList.back().lat);
            //p->drawPoint(sTrack.x,sTrack.y);
            p->drawRect(sTrack.x-target_size/2,sTrack.y-target_size/2,target_size,target_size);
            //p->drawText(sTrack.x+10,sTrack.y+10,100,50,0,QString::number(track->objectList.size()));
        }
    }


    bool blink = (CConfig::time_now_ms/500)%2;

    //draw all tracks
    for (uint i = 0;i<MAX_TRACKS_COUNT;i++)
    {
        C_SEA_TRACK* track = &(mRadarData->mTrackList[i]);
        if(track->mState==TrackState::removed)continue;
        if(track->mState==TrackState::newDetection)continue;
        PointDouble sTrack = ConvWGSToScrPoint(track->lon,track->lat);
        if(track->isSelected)//selected
        {
            // draw track history
            p->setPen(penTargetHistory);
            object_t* obj1 = &(track->objectHistory[0]);
            for (int j = 1;j<track->objectHistory.size();j++)
            {
                object_t* obj2 = &(track->objectHistory[j]);

                PointDouble s = ConvWGSToScrPoint(obj1->lon,obj1->lat);
                PointDouble s1 = ConvWGSToScrPoint(obj2->lon,obj2->lat);
                p->drawLine(s1.x,s1.y,s.x,s.y);
                obj1 = obj2;
            }
            object_t *obj2  = &(track->objectList.back());
            PointDouble s      = ConvWGSToScrPoint(obj1->lon,obj1->lat);
            PointDouble s1     = ConvWGSToScrPoint(obj2->lon,obj2->lat);
            p->drawLine(s1.x,s1.y,s.x,s.y);

            if(track->flag>=0)p->setPen(penTargetEnemySelected);
            else  p->setPen(penTargetFriendSelected);
        }
        else
        {

            if(track->flag>=0)p->setPen(penTargetEnemy);
            else  p->setPen(penTargetFriend);
        }
        if(track->flag==2)//targeted enemy
        {
            PointDouble s = ConvWGSToScrPoint(track->lon,track->lat);
            p->drawLine(s.x-20,s.y,s.x-10,s.y);
            p->drawLine(s.x+20,s.y,s.x+10,s.y);
            p->drawLine(s.x,s.y-20,s.x,s.y-10);
            p->drawLine(s.x,s.y+20,s.x,s.y+10);
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
            int size = 20000.0/((CConfig::time_now_ms - track->lastUpdateTimeMs)+800);
            if(size<target_size)size=target_size;//rect size depends on target age
            int lX = sTrack.x-size/2;
            int tY = sTrack.y-size/2;
            if(track->mAisConfirmedObj==0)
            {
                p->drawEllipse(lX,tY,size,size);
            }
            else {
                //p->setPen(penSelTarget);
                p->drawRect(lX,tY,size,size);
            }
            // draw speed vector
            int vectorLen = track->mSpeedkmhFit*mScale/6.0;
            int sx = sTrack.x+short(vectorLen*sinFast(track->courseRadFit+radians(trueShiftDeg)));
            int sy = sTrack.y-short(vectorLen*cosFast(track->courseRadFit+radians(trueShiftDeg)));
            p->drawLine(sx,sy,sTrack.x,sTrack.y);
            //draw mouse over
            if(track->isMouseOver)
            {
                lX = sTrack.x-size;
                tY = sTrack.y-size;
                int rX = sTrack.x+size;
                int bY = sTrack.y+size;
                p->drawLine(lX,tY,lX+size/3,tY);
                p->drawLine(lX,bY,lX+size/3,bY);
                p->drawLine(rX,tY,rX-size/3,tY);
                p->drawLine(rX,bY,rX-size/3,bY);
                p->drawLine(lX,tY,lX,tY+size/3);
                p->drawLine(rX,tY,rX,tY+size/3);
                p->drawLine(lX,bY,lX,bY-size/3);
                p->drawLine(rX,bY,rX,bY-size/3);
            }
            else
                track=track;
            //draw target number
            if(isDrawTargetNumber)p->drawText(sTrack.x+target_size/2+1,sTrack.y+target_size/2+1,100,50,0,QString::number(track->uniqId));
        }

    }



}
