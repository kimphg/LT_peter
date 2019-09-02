#include "dialogdetaildisplay.h"
#include "ui_dialogdetaildisplay.h"

#include <QPainter>
static QPen penTargetHistory(QBrush(Qt::gray),2);
static QPen penTargetEnemy(QBrush(Qt::magenta),3);
static QPen penTargetFriend(QBrush(QColor(0,200,200 ,255)),3);
static QPen penTargetEnemySelected(QBrush(Qt::magenta),4);
static QPen penTargetFriendSelected(QBrush(QColor(50,255,255 ,255)),5);
static QPen penCyan(QBrush(QColor(50,255,255 ,255)),1);//xoay mui tau
static enum ViewMode {ZoomHiden =0,ZoomIAD=1,ZoomHistogram=2,ZoomSpectre=3,ZoomRamp=4,ZoomZoom=5} view_mode=ZoomHiden;
PointInt DialogDetailDisplay::ConvWGSToScrPoint(double m_Long,double m_Lat)
{
    PointInt s;
    double refLat = (mLat + (m_Lat))*0.00872664625997;//pi/360
    s.x	= mScale*(((m_Long) - mLon) * 111.31949079327357)*cos(refLat);// 3.14159265358979324/180.0*6378.137);//deg*pi/180*rEarth
    s.y	= mScale*((mLat - (m_Lat)) * 111.132954);

    s.x   += radCtX;
    s.y   += radCtY;
    return s;
}
DialogDetailDisplay::DialogDetailDisplay(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogDetailDisplay)
{
    ui->setupUi(this);
    trueShiftDeg = 0;

    showAisName = true;
}
void DialogDetailDisplay::init(dataProcessingThread *processingThread,DialogAisInfo *tinfoPointer)
{
    dialogTargetInfo =tinfoPointer;
    timerId = this->startTimer(1000);
    mZoomSizeRg=CConfig::getDouble("mZoomSizeRg",2);
    mZoomSizeAz=CConfig::getDouble("mZoomSizeAz",2);

    setCenterLonLat(CConfig::getDouble("mLonZoom",DEFAULT_LONG),CConfig::getDouble("mLatZoom",DEFAULT_LAT));
    processing=processingThread;
    this->mRadarData = (processing->mRadarData);
    this->setFixedSize(ZOOM_SIZE,ZOOM_SIZE);
    this->setGeometry(1360,600,0,0);
    radCtX = width()/2;
    radCtY = height()/2;
    this->setWindowFlags(this->windowFlags()|Qt::FramelessWindowHint);
    //this->setWindowFlags(this->windowFlags()&(~Qt::WindowContextHelpButtonHint));
    this->setStyleSheet("background-color: rgb(0, 0, 0);color:rgb(255, 255, 255);font: bold 12pt \"MS Shell Dlg 2\";");
    this->show();

}

void DialogDetailDisplay::setCenterLonLat(double lon, double lat)
{
    mLon = lon;
    mLat = lat;
    CConfig::setValue("mLonZoom",mLon);
    CConfig::setValue("mLatZoom",mLat);
}

void DialogDetailDisplay::paintEvent(QPaintEvent *event)
{
    mScale  = mRadarData->getScale_PpiPerKm()*mRadarData->getScale_zoom_ppi();
    mZoomSizeKm = this->width()/mScale;
    target_size = mScale;
    QPainter p(this);
    DrawSignal(&p);
    drawAisTarget(&p);
    DrawRadarTargetByPainter(&p);
}

void DialogDetailDisplay::timerEvent(QTimerEvent *event)
{
    repaint();
}


void DialogDetailDisplay::DrawRadarTargetByPainter(QPainter* p)//draw radar target from pRadar->mTrackList
{
    p->setFont(QFont("Times", 8));
    //draw location history
    p->setBrush(Qt::NoBrush);
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
    //draw unconfirmed new detection
    for (uint i = 0;i<mRadarData->mTrackList.size();i++)
    {
        C_primary_track* track = &(mRadarData->mTrackList[i]);
        if(track->mState==TrackState::newDetection&&track->isUserInitialised)
        {
            PointInt sTrack = ConvWGSToScrPoint(track->objectList.back().lon,track->objectList.back().lat);
            //p->drawPoint(sTrack.x,sTrack.y);
            p->drawRect(sTrack.x-5,sTrack.y-5,10,10);
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
        if(track->mState!=TrackState::confirmed)continue;

        //check inside screen
        PointInt sTrack = ConvWGSToScrPoint(track->lon,track->lat);
        if(!rect().contains(sTrack.x,sTrack.y))continue;
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
                p->drawRect(sTrack.x-5,sTrack.y-5,10,10);
                p->drawLine(sTrack.x-7,sTrack.y-3,sTrack.x+7,sTrack.y+3);

            }
            p->drawText(sTrack.x+6,sTrack.y+6,100,50,0,QString::number(track->uniqId));
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
            if(track->mSpeedkmhFit>10){
                int sx = sTrack.x+short(10*sinFast(track->courseRadFit+radians(trueShiftDeg)));
                int sy = sTrack.y-short(10*cosFast(track->courseRadFit+radians(trueShiftDeg)));
                p->drawLine(sx,sy,sTrack.x,sTrack.y);
            }
            //draw target number
            p->drawText(sTrack.x+6,sTrack.y+6,100,50,0,QString::number(track->uniqId));
        }

    }

}
void DialogDetailDisplay::mousePressEvent(QMouseEvent *event)
{
    int posx = event->x();
    int posy = event->y();
    if(posx)mMouseLastX= posx;
    if(posy)mMouseLastY= posy;
   /* if(event->buttons() & Qt::MiddleButton)
    {
        if(isInsideViewZone(mMousex,mMousey))
        {
            if(mouse_mode&MouseManualTrack)//add mouse manual object
            {
                PointDouble point = ConvScrPointToKMXY(mMousex,mMousey);
                double rgKm = mRadarData->sn_scale*80.0;
                C_primary_track*track= mRadarData->getManualTrackzone(point.x,point.y,rgKm);
                if(track)
                {
                    track->addManualPossible(point.x,point.y);
                    return;
                }

            }
        }
    }

    else*/ if(event->buttons() & Qt::RightButton) {
        if(posx)mMousex= posx;
        if(posy)mMousey= posy;

        if(!checkClickRadarTarget(posx,posy))
            checkClickAIS(posx,posy);

    }

}
void DialogDetailDisplay::checkClickAIS(int xclick, int yclick)
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
void DialogDetailDisplay::DrawAISMark(PointInt s ,double head,QPainter *p,QString name,int size,int vectorLen)
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
        p->drawText(s.x+size,s.y+size,150,20,0,name);
    }


}
bool DialogDetailDisplay::isInsideViewZone(int x,int y)
{
    return rect().contains(x,y);
}
void DialogDetailDisplay::drawAisTarget(QPainter *p)
{
    p->setBrush(Qt::NoBrush);
    for(std::map<int,AIS_object_t>::iterator iter = processing->mAisData.begin();iter!=processing->mAisData.end();iter++)
    {
        AIS_object_t aisObj = iter->second;

        if(aisObj.getAge()>300000)continue;
        PointInt s = ConvWGSToScrPoint(aisObj.mLong,aisObj.mLat);
        if(aisObj.isMatchToRadarTrack)continue;
        if(!isInsideViewZone(s.x,s.y))continue;
        int vectorLen = nm2km(aisObj.mSog)*mScale/6.0;
        DrawAISMark(s,radians(aisObj.mCog),p,aisObj.mName,target_size,vectorLen);

    }

}
C_primary_track* DialogDetailDisplay::checkClickRadarTarget(int xclick, int yclick)
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
void DialogDetailDisplay::DrawSignal(QPainter*p)
{
    if(view_mode==ZoomHiden)return;

    p->setCompositionMode(QPainter::CompositionMode_SourceOver);

    p->setBrush(QBrush(Qt::black));
    p->setPen(Qt::black);
    QRect mIADrect = this->rect();
    if(view_mode==ZoomIAD)
    {
        //        printf("\nDraw IAD");
        if((mRadarData->getMimg_zoom_ar()==nullptr)||(mRadarData->getMimg_zoom_ar()->isNull()))return;
        //        printf("\nDraw IAD");
        p->setPen(QPen(Qt::white,2));


        QImage img = mRadarData->getMimg_zoom_ar()->scaled(mIADrect.width(),mIADrect.height(),Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
        p->drawImage(mIADrect,img);//todo:resize
        p->setFont(QFont("Times",10));
        p->drawText(mIADrect.x()+mIADrect.width()-50,mIADrect.y()+mIADrect.height()-10,
                    QString::number(mZoomSizeAz,'f',1)+
                    QString::fromUtf8(" Độ"));
        p->drawText(mIADrect.x()+5,mIADrect.y()+15,
                    QString::number(mZoomSizeRg/1.852,'f',1)+
                    QString::fromUtf8(" Lý"));
        QPoint p1(mIADrect.x(),mIADrect.y());
        //QPoint p2(rect.x(),rect.y());
        QPoint p11(mIADrect.x()+mIADrect.width(),mIADrect.y());
        QPoint p22(mIADrect.x(),mIADrect.y()+mIADrect.height());
        p->drawLine(p1,p11);
        p->drawLine(p1,p22);
        int step = mIADrect.width()/5;
        for(int i = 0;i<5;i++)
        {
            p->drawLine(mIADrect.x()+step*i,mIADrect.y()+mIADrect.height(),mIADrect.x()+step*i,mIADrect.y()+mIADrect.height()-15);
            p->drawLine(mIADrect.x(),mIADrect.y()+step*i,mIADrect.x()+15,mIADrect.y()+step*i);
        }



    }
    else if(view_mode==ZoomZoom)
    {
        //        printf("\nDraw ZoomZoom");
        //if((!mRadarData->getMimg_zoom_ppi())||(mRadarData->getMimg_zoom_ppi()->isNull()))return;
        p->drawImage(mIADrect,*mRadarData->getMimg_zoom_ppi(),mRadarData->getMimg_zoom_ppi()->rect());
//        if(mRangeIndex>2)
//        {

//            p->setPen(QPen(QColor(255,255,255,200),0,Qt::DashLine));
//            p->setBrush(Qt::NoBrush);
//            p->drawRect(mZoomCenterx-zoom_size/2.0,mZoomCentery-zoom_size/2.0,zoom_size,zoom_size);
//        }

    }
    else if(view_mode==ZoomHistogram)
    {

        p->drawImage(mIADrect,*mRadarData->getMimg_histogram(),
                     mRadarData->getMimg_histogram()->rect());

    }


}
DialogDetailDisplay::~DialogDetailDisplay()
{
    delete ui;
}

void DialogDetailDisplay::on_toolButton_view_IAD_clicked(bool checked)
{
    if(checked)view_mode = ViewMode::ZoomIAD;
}

void DialogDetailDisplay::on_toolButton_view_histogram_clicked(bool checked)
{
    if(checked)view_mode = ViewMode::ZoomHistogram;
}

void DialogDetailDisplay::on_toolButton_view_zoom_clicked(bool checked)
{
    if(checked)view_mode = ViewMode::ZoomZoom;
}
