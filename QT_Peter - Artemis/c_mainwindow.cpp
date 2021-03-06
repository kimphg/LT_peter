#include "c_mainwindow.h"
#include "statuswindow.h"
#include "ui_mainwindow.h"


#define MAX_VIEW_RANGE_KM   50
//static QPen penTargetHistory(QBrush(Qt::gray),2);
//static QPen penTargetEnemy(QBrush(Qt::magenta),2);
//static QPen penTargetFriend(QBrush(QColor(0,200,200 ,255)),2);
//static QPen penTargetEnemySelected(QBrush(Qt::magenta),2);
//static QPen penTargetFriendSelected(QBrush(QColor(50,255,255 ,255)),3);
//static QPen penCyan(QBrush(QColor(50,255,255 ,255)),1);//xoay mui tau
static enum ViewMode {ZoomHiden =0,ZoomIAD=1,ZoomHistogram=2,ZoomSpectre=3,ZoomRamp=4,ZoomZoom=5} zoom_mode=ZoomHiden;
static PointAziRgkm AutoSelP1,AutoSelP2;
//static bool hideAisFishingBoat = true;

static C_arpa_area rda_main;
DialogAisInfo *dialogTargetInfo ;
DialogDetailDisplay* dialogZoom;
QPixmap                     *pMap;// painter cho ban do
DensityMap* pDensMap;
//#ifdef THEON
//static QPen penBackground(QBrush(QColor(24 ,48 ,64,255)),224+SCR_BORDER_SIZE);
//static QRect circleRect = ppiRect.adjusted(-135,-135,135,135);

//#else

//static QPen penBackground(QBrush(QColor(24 ,48 ,64,255)),150+SCR_BORDER_SIZE);
//QRect circleRect = ppiRect.adjusted(-135,-135,135,135);
//#endif
static bool isInsideProtected = false;
static QPen penYellow(QBrush(QColor(255,255,50 ,255)),2);
static QPen mGridViewPen1(QBrush(QColor(150,150,150,255)),1);
static clock_t clkBegin = clock();
static clock_t clkEnd = clock();
static clock_t paintTime = 20;
static QStringList                 commandLogList;
static QTransform                  mTrans;

static bool isShowAIS =true;
//QPixmap                     *pViewFrame=NULL;// painter cho ban do
static CMap *osmap ;
static bool toolButton_grid_checked = true;
static StatusWindow                *mstatWin;
static int                         mRangeIndex = 0;
static int                         mDistanceUnit=0;//0:NM;1:KM
static double                      mZoomSizeRg = 2;
static double                      mZoomSizeAz = 10;

//static double                      CConfig::mStat.shipHeadingDeg=20;
static bool                        isMapOutdated = true;
static bool isHeadUp = false;
static int   mMousex =0,mMousey=0;
//static QPoint points[6];
static double                      mMapOpacity;
static int                         mMaxTapMayThu=18;
static QTimer                      timerVideoUpdate,timerMetaUpdate;
static QTimer                      syncTimer1s,syncTimer5p ;
//static QTimer                      *dataPlaybackTimer ;
static short                       dxMax,dyMax;
//static C_ARPA_data                 arpa_data;
static short                       scrCtX= SCR_H/2 + SCR_LEFT_MARGIN, scrCtY= SCR_H/2+SCR_TOP_MARGIN;
static short                       dx =-100,dy=-100,dxMap=0,dyMap=0;
static short                       mZoomCenterx,mZoomCentery,mMouseLastX,mMouseLastY;
static bool                        isScaleChanged =true;
static double                       rangeRatio = 1;
//extern CConfig         mGlobbalConfig;
//static QStringList     warningList;
static QString         strDistanceUnit;
//short selectedTargetIndex;
static mouseMode mouse_mode = MouseNormal;
//static DialogCommandLog *cmLog;
static unsigned char commandMay22[]={0xaa,0x55,0x02,0x0c,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
//static enum TargetType{
//    RADAR,AIS,NOTARGET
//}selectedTargetType  = NOTARGET;


//short config.getRangeView() = 1;
static double ringStep = 1;
static double curAziRad = 3;
//static TrackPointer* currTrackPt;
//guard_zone_t gz1,gz2,gz3;
//static unsigned short cur_object_index = 0;


//PointDouble rda_main.ConvWGSToScrPoint(double m_Long,double m_Lat)
//{
//    PointDouble s;
//    double refLat = (CConfig::mLat + (m_Lat))*0.00872664625997;//pi/360
//    s.x	= rda_main.mScale*(((m_Long) - CConfig::mLon) * 111.31949079327357)*cos(refLat);// 3.14159265358979324/180.0*6378.137);//deg*pi/180*rEarth
//    s.y	= rda_main.mScale*((CConfig::mLat - (m_Lat)) * 111.132954);
//    rotateVector(rda_main.trueShiftDeg,&s.x,&s.y);
//    s.x   += rda_main.radCtX;
//    s.y   += rda_main.radCtY;
//    return s;
//}

double y2lat(short y)
{
    return (y  )/rda_main.mScale/111.31949079327357 + CConfig::mLat;
}
double x2lon(short x)
{
    double refLat = CConfig::mLat*0.01745329251994;
    return (x  )/rda_main.mScale/111.31949079327357/cos(refLat) + CConfig::mLon;
}

void Mainwindow::ConvXYradar2XYscr()
{

}
void Mainwindow::mouseDoubleClickEvent( QMouseEvent * e )
{

    if ( e->button() == Qt::LeftButton )
    {
        int posx = e->x();
        int posy = e->y();
        if(posx)mMousex= posx;
        if(posy)mMousey= posy;
        if(isInsideViewZone(mMousex,mMousey))
        {
            C_SEA_TRACK* track = rda_main.SelectRadarTarget(mMousex,mMousey);
            if(track)
            {
                track->isUserInitialised=true;
            }
            else
            {
                PointDouble point = rda_main.ConvScrPointToKMXY(mMousex,mMousey);
                rda_main.mRadarData->addManualTrack(point.x,point.y);
            }


//            {
//                int dx = mMousex-rda_main.radCtX;
//                int dy = mMousey-rda_main.radCtY;
//                double rgKM = sqrt((dx*dx)+(dy*dy));
//                rda_main.mRadarData->addDetectionZone(point.x,point.y,200/(rgKM)+1,7.0/rda_main.mScale,true);
//            }
        }
        //ui->toolButton_manual_track->setChecked(false);

    }
    else if(e->button()==Qt::RightButton)
    {
        int posx = (QCursor::pos()).x();
        int posy = (QCursor::pos()).y();
        if(posx)mMousex= posx;
        if(posy)mMousey= posy;
        PointDouble pt = rda_main.ConvScrPointToWGS(mMousex,mMousey);
        dialogZoom->rda.setCenterLonLat(pt.x,pt.y);
#ifndef THEON
        if(!isInsideViewZone(mMousex,mMousey))return;
        double azid,rg;
        C_radar_data::ConvkmxyToPolarDeg((mMousex - rda_main.radCtX)/rda_main.mScale,-(mMousey - rda_main.radCtY)/rda_main.mScale,&azid,&rg);
        int aziBinary = int(azid/360.0*4096);
        unsigned char command[]={0xaa,0x55,0x6a,0x09,
                                 static_cast<unsigned char>(aziBinary>>8),
                                 static_cast<unsigned char>(aziBinary),
                                 0x00,0x00,0x00,0x00,0x00,0x00};
        rda_main.processing->sendCommand(command,9,false);
#endif
        mZoomCenterx = mMousex;
        mZoomCentery = mMousey;

        rda_main.mRadarData->setZoomRectAR((mMousex - rda_main.radCtX)/rda_main.mScale,
                              -(mMousey - rda_main.radCtY)/rda_main.mScale,
                              mZoomSizeRg,mZoomSizeAz);
        rda_main.mRadarData->setZoomRectXY((mMousex - rda_main.radCtX),(mMousey - rda_main.radCtY));

    }
    //Test doc AIS

}
void Mainwindow::sendToRadarString(QString command)
{
    command.replace(" ", "");
    QStringList list = command.split(';');
    for(int i=0;i<list.size();i++)
    {
        QByteArray ba=list.at(i).toLatin1();
        sendToRadarHS(ba.data());
    }

}
void Mainwindow::sendToRadarHS(const char* hexdata)//todo:move to radar class
{
    size_t len = strlen(hexdata);
    if(len>16)return;
    if(len%2)return;
    len/=2;
    unsigned char sendBuff[]={0,0,0,0,0,0,0,0};
    hex2bin(hexdata,sendBuff);
    rda_main.processing->sendCommand(sendBuff,8);

}
void Mainwindow::sendToRadar(unsigned char* hexdata)
{
    m_udpSocket->writeDatagram((char*)hexdata,8,QHostAddress("192.168.0.44"),2572);
}

bool Mainwindow::checkInsideZoom(int x,int y)
{
    int dx= x-mZoomCenterx;
    int dy= y-mZoomCentery;
    if(abs(dx)<(zoom_size/2.0))
    {
        if(abs(dy)<(zoom_size/2.0))
        {
            return true;
        }
    }
    return false;
}
void Mainwindow::mouseReleaseEvent(QMouseEvent *event)
{
    setMouseMode(MouseDrag,false);
    //    if(isAddingTarget)
    //    {
    //        float xRadar = (mouseX - rda_main.radCtX)/signsize ;//coordinates in  radar xy system
    //        float yRadar = -(mouseY - rda_main.radCtY)/signsize;
    //        rda_main.mRadarData->addTrack(xRadar,yRadar);
    //        ui->actionAddTarget->toggle();
    //        isScreenUp2Date = false;
    //        return;
    //    }

    isMapOutdated = true;

}
void Mainwindow::wheelEvent(QWheelEvent *event)
{
    event = event;
    //if(event->delta()>0)ui->horizontalSlider->setValue(ui->horizontalSlider->value()+1);
    //if(event->delta()<0)ui->horizontalSlider->setValue(ui->horizontalSlider->value()-1);
}
void Mainwindow::mouseMoveEvent(QMouseEvent *event) 
{
    if((mouse_mode&MouseDrag)&&(event->buttons() & Qt::LeftButton))
    {
        {
            short olddx = dx;
            short olddy = dy;

            dx+= mMouseLastX-event->x();
            dy+= mMouseLastY-event->y();

            dxMap += mMouseLastX-event->x();
            dyMap += mMouseLastY-event->y();
            while(dx*dx+dy*dy>dxMax*dxMax)
            {
                if(abs(dx)>abs(dy))
                {
                    if(dx>0){dx--;dxMap--;}else {dx++;dxMap++;}}
                else
                {
                    if(dy>0){dy--;dyMap--;}else {dy++;dyMap++;}
                }
            }
            mZoomCenterx+= olddx - dx;
            mZoomCentery+= olddy - dy;
            mMouseLastX=event->x();
            mMouseLastY=event->y();
            //            isMapOutdated = true;
            rda_main.radCtX = scrCtX-dx;
            rda_main.radCtY = scrCtY-dy;
        }
    }
}

void Mainwindow::keyPressEvent(QKeyEvent *event)
{
    this->setFocus();
    int key = event->key();
    if(key == Qt::Key_Space)
    {

        int posx = (QCursor::pos()).x();
        int posy = (QCursor::pos()).y();
        if(posx)mMousex= posx;
        if(posy)mMousey= posy;
        PointDouble pt = rda_main.ConvScrPointToWGS(mMousex,mMousey);
        dialogZoom->rda.setCenterLonLat(pt.x,pt.y);
#ifndef THEON
        if(!isInsideViewZone(mMousex,mMousey))return;
        double azid,rg;
        C_radar_data::ConvkmxyToPolarDeg((mMousex - rda_main.radCtX)/rda_main.mScale,-(mMousey - rda_main.radCtY)/rda_main.mScale,&azid,&rg);
        int aziBinary = int(azid/360.0*4096);
        unsigned char command[]={0xaa,0x55,0x6a,0x09,
                                 static_cast<unsigned char>(aziBinary>>8),
                                 static_cast<unsigned char>(aziBinary),
                                 0x00,0x00,0x00,0x00,0x00,0x00};
        rda_main.processing->sendCommand(command,9,false);
#endif
        mZoomCenterx = mMousex;
        mZoomCentery = mMousey;

        rda_main.mRadarData->setZoomRectAR((mMousex - rda_main.radCtX)/rda_main.mScale,
                              -(mMousey - rda_main.radCtY)/rda_main.mScale,
                              mZoomSizeRg,mZoomSizeAz);
        rda_main.mRadarData->setZoomRectXY((mMousex - rda_main.radCtX),(mMousey - rda_main.radCtY));
    }
    else if(key == Qt::Key_Control)
    {
        controlPressed  = true;
    }
    else if(controlPressed)
    {
        if(key==Qt::Key_F2)
        {
            this->setGeometry(0,-120,SCR_W,SCR_H);
        }
        else if(key==Qt::Key_F1)
        {
            this->setGeometry(0,0,SCR_W,SCR_H);
        }
        else if(key==Qt::Key_1)
        {
            int posx = (QCursor::pos()).x();
            int posy = (QCursor::pos()).y();
            if(posx)mMousex= posx;
            if(posy)mMousey= posy;

            rda_main.setCenterLonLat(x2lon(mMousex - rda_main.radCtX),y2lat(-(mMousey - rda_main.radCtY)));
            CConfig::setGPSLocation(rda_main.mLat,rda_main.mLon);
        }
        else if(key==Qt::Key_2)
        {
            rda_main.mRadarData->clearPPI();
        }
        else if(key==Qt::Key_3)
        {
            int posx = (QCursor::pos()).x();
            int posy = (QCursor::pos()).y();
            if(posx)mMousex= posx;
            if(posy)mMousey= posy;

            int density = rda_main.mRadarData->getDensityLatLon(y2lat(-(mMousey - rda_main.radCtY)),
                                                   x2lon(mMousex - rda_main.radCtX)
                                                   );
            ui->label_sn_type->setText(QString::number(density));

        }
        else if(key==Qt::Key_0)
        {
            ShutDown();
        }
        else if(key==Qt::Key_9)
        {
            QProcess::startDetached("explorer.exe");
        }
        else if(key==Qt::Key_8)
        {
            on_toolButton_set_commands_clicked();
        }

    }

    else if((!controlPressed)&&key >= Qt::Key_1&&key<=Qt::Key_6)
    {
        /*int keyNum = key-Qt::Key_1;
        if(keyNum>=TARGET_TABLE_SIZE)return;
        if(!mTargetMan.selectedTrackID)return;
        /*if(keyNum>0){
            for(int i=0;i<rda_main.mRadarData->mTrackList.size();i++)
            {
                track_t* track=&( rda_main.mRadarData->mTrackList[i]);
                if(track->uniqId==mTargetMan.selectedTrackID)
                {

                    mmTargetMan.selectedTrackID=track->uniqId;
                    return;
                }
            }
        }*/
        //        for(int i=0;i<rda_main.mRadarData->mTrackList.size();i++)
        //        {
        //            track_t* track=&( rda_main.mRadarData->mTrackList[i]);
        //            if(mTargetMan.selectedTrackID==track->uniqId)
        //            {
        //                mTargetMan.targetTable[keyNum].track = track;
        //                mTargetMan.targetTable[keyNum].trackID = track->uniqId;
        //            }
        //        }
    }

}
void Mainwindow::keyReleaseEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Control)
    {
        controlPressed  = false;
    }
}

/*
short selZone_x1, selZone_x2, selZone_y1, selZone_y2;
bool isSelectingTarget = false;
void Mainwindow::detectZone()
{
    //short sx,sy;
    //float scale_ppi = rda_main.mRadarData->getScale_ppi();
    if(selZone_x1>selZone_x2)
    {
        short tmp = selZone_x1;
        selZone_x1 = selZone_x2;
        selZone_x2 = tmp;
    }
    if(selZone_y1>selZone_y2)
    {
        short tmp = selZone_y1;
        selZone_y1 = selZone_y2;
        selZone_y2 = tmp;
    }
}*/
//bool Mainwindow::isInsideViewZone(int x, int y)
//{

//    short dx = x-scrCtX;
//    short dy = y-scrCtY;
//    short range = SCR_H-60;
//    if((dx*dx+dy*dy)>(range*range/4))
//        return false;
//    else
//        return true;
//}
bool Mainwindow::isInsideIADZone(int x, int y)
{
    return mIADrect.contains(x,y);
}

void Mainwindow::mousePressEvent(QMouseEvent *event)
{
    int posx = event->x();
    int posy = event->y();
    if(posx)mMouseLastX= posx;
    if(posy)mMouseLastY= posy;
    if(event->buttons() & Qt::MiddleButton)
    {
        if(isInsideViewZone(mMousex,mMousey))
        {
            if(mouse_mode&MouseManualTrack)//add mouse manual object
            {
                PointDouble point = rda_main.ConvScrPointToKMXY(mMousex,mMousey);
                double rgKm = rda_main.mRadarData->sn_scale*80.0;
                C_SEA_TRACK*track= rda_main.mRadarData->getManualTrackzone(point.x,point.y,rgKm);
                if(track)
                {
                    track->addManualPossible(point.x,point.y);
                    return;
                }

            }
        }
    }

    else if(event->buttons() & Qt::LeftButton) {
        if(posx)mMousex= posx;
        if(posy)mMousey= posy;
        if(isInsideViewZone(mMousex,mMousey))
        {

            if(!isHeadUp)
                setMouseMode(MouseDrag,true);
            if(mouse_mode&MouseAutoSelect1)
            {
                AutoSelP1 =  rda_main.ConvScrPointToAziRgkm(mMousex,mMousey);
                setMouseMode(MouseAutoSelect2,true);
                setMouseMode(MouseAutoSelect1,false);
            }
            else if(mouse_mode&MouseAutoSelect2)
            {
                ui->toolButton_dzs_1->setChecked(false);
                AutoSelP2 =  rda_main.ConvScrPointToAziRgkm(mMousex,mMousey);
                double dazi = abs(AutoSelP1.aziRad-AutoSelP2.aziRad)/2.0;

                double dRg = abs( AutoSelP1.rg-AutoSelP2.rg)/2.0;
                double cazi = (AutoSelP1.aziRad+AutoSelP2.aziRad)/2; ;
                if(dazi>PI_CHIA2)
                {
                    dazi = PI-dazi;
                    cazi+=PI;
                    if(cazi>PI_NHAN2)cazi-=PI_NHAN2;
                }

                double cRg =( AutoSelP1.rg+AutoSelP2.rg)/2;
                setMouseMode(MouseAutoSelect2,false);
                setMouseMode(MouseAutoSelect1,false);

                //PointDouble point = ConvScrPointToKMXY(mMousex,mMousey);
                //int dx = mMousex-rda_main.radCtX;
                //int dy = mMousey-rda_main.radCtY;
                //double rgKM = sqrt((dx*dx)+(dy*dy));
                rda_main.mRadarData->addDetectionZoneAZ(cazi,cRg,dazi,dRg,false);
            }
            else
            {
            }
        }

    }
    else if(event->buttons() & Qt::RightButton)
    {
        if(isInsideViewZone(mMousex,mMousey))
            if(!rda_main.SelectRadarTarget(posx,posy))
                if(ui->toolButton_ais_show->isChecked())
                {
                    checkClickAIS(posx,posy);

                }
    }

}

void Mainwindow::checkClickAIS(int xclick, int yclick)
{
    for(std::map<int,AIS_object_t>::iterator iter = rda_main.processing->mAisVesselsList.begin();iter!=rda_main.processing->mAisVesselsList.end();iter++)
    {
        AIS_object_t *aisObj = &(iter->second);
//        if(aisObj->isSelected)continue;
        if(aisObj->isMatchToRadarTrack)continue;
        double fx,fy;
        C_radar_data::ConvWGSToKm(&fx,&fy,aisObj->mLong,aisObj->mLat);
        int x = (fx*rda_main.mScale)+rda_main.radCtX;
        int y = rda_main.radCtY-(fy*rda_main.mScale);
        if(abs(x-xclick)<5&&abs(y-yclick)<5)
        {

            dialogTargetInfo->setDataSource(aisObj,0);

            break;
        }
        if(checkInsideZoom(x,y))
        {
            int dx= x-mZoomCenterx;
            int dy= y-mZoomCentery;
            PointDouble iadPoint;
            iadPoint.x = mIADCenter.x+dx*mZoomScale;
            iadPoint.y = mIADCenter.y+dy*mZoomScale;
            if(abs(iadPoint.x-xclick)<5&&abs(iadPoint.y-yclick)<5)
            {
                DialogAisInfo *dialog = new DialogAisInfo(this);
                dialog->setAttribute( Qt::WA_DeleteOnClose, true );
                dialog->setWindowFlags(dialog->windowFlags()&(~Qt::WindowContextHelpButtonHint));
                dialog->setFixedSize(dialog->width(),dialog->height());
                dialog->setDataSource(aisObj,0);
                dialog->setGeometry(10,800,0,0);
                dialog->show();
                break;
            }
        }
    }
}

void Mainwindow::initCursor()
{
    QPixmap cursor_pixmap = QPixmap(31,31);
    cursor_pixmap.fill(Qt::transparent);
    QPainter paint(&cursor_pixmap);

    paint.setPen(QPen(QColor(255,255,50,150),1,Qt::SolidLine,Qt::FlatCap));
    paint.drawLine(15,0,15,30);
    paint.drawLine(0,15,30,15);
    paint.setPen(QPen(QColor(255,255,0,255),3,Qt::SolidLine,Qt::FlatCap));
    paint.drawLine(15,0,15,6);
    paint.drawLine(15,24,15,30);
    paint.drawLine(0,15,6,15);
    paint.drawLine(24,15,30,15);
    cursor_default = QCursor(cursor_pixmap, 15, 15);
    setCursor(cursor_default);
}
Mainwindow::Mainwindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    dialogTargetInfo = new DialogAisInfo(this);
    dialogZoom = new DialogDetailDisplay(this);
    //    mShowobjects = false;
    //    mShowLines = false;
    //    mShowTracks = false;
    InitNetwork();
    InitTimer();
    setFocusPolicy(Qt::StrongFocus);
    InitSetting();
    gotoCenter();
//    setRadarState(DISCONNECTED);

    isRadarShow = true;
    dialogZoom->init(rda_main.processing,dialogTargetInfo);
    initCursor();
    controlPressed = false;
    pMap = new QPixmap(SCR_H,SCR_H);
    //    processCuda = new QProcess();
    degreeSymbol= QString::fromLocal8Bit("\260");
    pDensMap = rda_main.mRadarData->getDensityMap();
    //cmLog = new DialogCommandLog();
    ppiRect = QRect(SCR_LEFT_MARGIN+SCR_BORDER_SIZE/2,
                    SCR_TOP_MARGIN+SCR_BORDER_SIZE/2,
                    SCR_H -SCR_BORDER_SIZE,
                    SCR_H -SCR_BORDER_SIZE);

    //    GDALAllRegister();
    //    GDALDataset       *poDS;

    //init drawing context

    //this->setFixedSize(900 + ui->toolBar_Main->width()*3,850);
    //scale = SCALE_MIN;



    //isSettingUp2Date = false;
    //UpdateSetting();

}

//void Mainwindow::DrawSignal(QPainter *p)
//{


//}

//void MainWindow::createMenus()
//{
//    m_fileMenu = menuBar()->addMenu(tr("&File"));
//    m_fileMenu->addAction(a_openShp);
//    m_fileMenu->addAction(a_openPlace);
//    m_fileMenu->addAction(a_openSignal);

//    //
//    m_connectionMenu = menuBar()->addMenu(tr("&Connect"));
//    m_connectionMenu->addAction(a_gpsOption);
//}
void Mainwindow::gpsOption()
{
    //GPSDialog *dlg = new GPSDialog;
    //dlg->show();
}

void Mainwindow::PlaybackRecFile()//
{


}


Mainwindow::~Mainwindow()
{

    delete ui;
    CConfig::SaveToFile();
    if(pMap)delete pMap;
}
//bool isMapbusy=false;
double trueShiftDegOldMap;
void DrawMap()
{
    isMapOutdated = false;
    if(!pMap)
    {
        pMap = new QPixmap(SCR_H,SCR_H);
    }
    //calculate center coordinate
    double newLat, newLong;
    C_radar_data::ConvKmToWGS((double(dx))/rda_main.mScale,
                (double(-dy))/rda_main.mScale,&newLong,&newLat);
    osmap->setCenterPos(newLat,newLong);
    trueShiftDegOldMap = rda_main.trueShiftDeg;
    //clear the map
    pMap->fill(Qt::black);
    //clear the offset
    dxMap = 0;
    dyMap = 0;
    //
    QPainter pMapPainter(pMap);

//    isMapbusy = true;
    QPixmap pix = osmap->getImage(rda_main.mScale);
#ifdef THEON
    //draw density map
    if(CConfig::getInt("isViewDensityMap"))
    {
        QPainter densityPainter(&pix);

        double minLat ,minLon, maxLat, maxLon;
        double rangeKm = pMap->width()/1.5/rda_main.mScale;
        C_radar_data::ConvKmToWGS(-rangeKm,
                    -rangeKm,
                    &minLon,
                    &minLat);
        C_radar_data::ConvKmToWGS(rangeKm,
                    rangeKm,
                    &maxLon,
                    &maxLat);
        int minLatin = minLat*1000;
        int minLonin = minLon*1000;
        int maxLatin = maxLat*1000;
        int maxLonin = maxLon*1000;
        for(auto it:(*pDensMap))
        {
            std::pair<int,int> key=it.first;
            if(key.first<maxLatin&&
                    key.first>minLatin&&
                    key.second<maxLonin&&
                    key.second>minLonin
                    )
            {
                int value = log2(it.second)*50;
                if(value>255)value=255;
                PointDouble p = rda_main.ConvWGSToScrPoint(
                            ((key.second)+0.5)/1000.0,
                            ((key.first)+0.5)/1000.0
                            );
                densityPainter.setPen(QPen(QColor(value,value,100,value),1+rda_main.mScale/15));
                densityPainter.drawPoint(p.x- scrCtX+pix.width()/2,p.y-scrCtY+pix.height()/2);
            }

        }
    }
#endif
    // rotate Map for head up mode
    if(isHeadUp)
    {
        pix=pix.transformed(mTrans);
    }
    pMapPainter.setOpacity(mMapOpacity);
    pMapPainter.drawPixmap((-pix.width()/2+pMap->width()/2),
                           (-pix.height()/2+pMap->height()/2),pix.width(),pix.height(),pix
                           );

//    isMapbusy = false;

}
void Mainwindow::DrawGrid(QPainter* p,short centerX,short centerY)
{
    p->setCompositionMode(QPainter::CompositionMode_Plus);
    //return;
    QPen pen(QColor(150,150,150,0xff));

    pen.setStyle(Qt::DashLine);

    p->setFont(QFont("Times", 10));
    p->setBrush(QBrush(Qt::NoBrush));
    p->setPen(pen);
    p->drawLine(centerX-5,centerY,centerX+5,centerY);
    p->drawLine(centerX,centerY-5,centerX,centerY+5);
    //pen.setColor(QColor(30,90,150,120));
    pen.setWidth(1);
    p->setPen(pen);
    for(short i = 1;i<6;i++)
    {
        int rad = i*ringStep*rangeRatio*rda_main.mScale;
        p->drawEllipse(QPoint(centerX,centerY),
                       (short)(rad),
                       (short)(rad));
        p->drawText(centerX+2,centerY-rad+2,100,20,0,QString::number(i*ringStep)+strDistanceUnit);
        p->drawText(centerX+2,centerY+rad+2,100,20,0,QString::number(i*ringStep)+strDistanceUnit);
        p->drawText(centerX+rad+2,centerY+2,100,20,0,QString::number(i*ringStep)+strDistanceUnit);
        p->drawText(centerX-rad+2,centerY+2,100,20,0,QString::number(i*ringStep)+strDistanceUnit);
    }
    short theta;
    short gridR = ringStep*1.852f*rda_main.mScale*5;
    for(theta=0;theta<360;theta+=90){
        QPoint point1,point2;
        short dx = gridR*cos(radians(theta));
        short dy = gridR*sin(radians(theta));
        point1.setX(centerX);
        point1.setY(centerY);
        point2.setX(centerX+dx);
        point2.setY(centerY+dy);
        p->drawLine(point1,point2);

    }
    for(theta=0;theta<360;theta+=30){
        QPoint point1,point2;
        short dx = gridR*cos(radians(theta));
        short dy = gridR*sin(radians(theta));
        point1.setX(centerX);
        point1.setY(centerY);
        point2.setX( centerX+dx);
        point2.setY(centerY+dy);
        p->drawLine(point1,point2);
        point2.setX(centerX+dx*1.02-9);
        point2.setY(centerY+dy*1.02+5);
        //                if(theta<270)p->drawText(point2,QString::number(theta+90));
        //                else p->drawText(point2,QString::number(theta-270));

    }

    //end grid
    p->setCompositionMode(QPainter::CompositionMode_SourceOver);



}

void Mainwindow::initGraphicView()
{
    //scene = new QGraphicsScene(-200, -200, 400, 400);
    //view = new jViewPort(scene,this);
    //view->setGeometry(SCR_LEFT_MARGIN,0,SCR_H,SCR_H);
    //view->lower();
    //view->setRenderHint(QPainter::Antialiasing);
    //view->setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
    //view->setBackgroundBrush(Qt::transparent);

}
//void Mainwindow::DrawTrack(track_t* track ,QPainter* p)
//{

//}


void Mainwindow::DrawDetectZones(QPainter* p)//draw radar target from rda_main.mRadarData->mTrackList
{
    p->setPen((penYellow));
    for (uint i = 0;i<rda_main.mRadarData->mDetectZonesList.size();i++)
    {
        RangeAziWindow *dw = &rda_main.mRadarData->mDetectZonesList[i];
        if(dw->isRemoved)continue;
        int dazi = dw->maxDazDeg;
        double azi = 90.0-(rda_main.trueShiftDeg+dw->aziDeg);
        int drg =  int(dw->maxDrg*rda_main.mScale);
        int rg = int(dw->rg*rda_main.mScale)-drg;
        /*short sx = dw->xkm*rda_main.mScale ;//+ rda_main.radCtX;
        short sy = -dw->ykm*rda_main.mScale ;//+ rda_main.radCtY;
        rotateVector(trueShift,&sx,&sy);*/
        int x1 = rda_main.radCtX-rg;
        int y1 = rda_main.radCtY-rg;
        p->drawArc(x1,y1,rg*2,rg*2,(azi-dazi)*5760/360,int(dazi*2)*5760/360);
        rg+=(drg*2);
        x1 = rda_main.radCtX-rg;
        y1 = rda_main.radCtY-rg;
        p->drawArc(x1,y1,rg*2,rg*2,(azi-dazi)*5760/360,int(dazi*2)*5760/360);

        azi = (rda_main.trueShiftDeg+dw->aziDeg);
        p->drawLine(rda_main.radCtX+rg*sin(radians(azi-dazi)),
                    rda_main.radCtY-rg*cos(radians(azi-dazi)),
                    rda_main.radCtX+(rg-2*drg)*sin(radians(azi-dazi)),
                    rda_main.radCtY-(rg-2*drg)*cos(radians(azi-dazi)));
        p->drawLine(rda_main.radCtX+rg*sin(radians(azi+dazi)),
                    rda_main.radCtY-rg*cos(radians(azi+dazi)),
                    rda_main.radCtX+(rg-2*drg)*sin(radians(azi+dazi)),
                    rda_main.radCtY-(rg-2*drg)*cos(radians(azi+dazi)));
    }

}


PointDouble Mainwindow::ConvKmXYToScrPoint(double x, double y)
{
    PointDouble s;
    s.x = x*rda_main.mScale ;
    s.y = -y*rda_main.mScale ;
    C_arpa_area::rotateVector(rda_main.trueShiftDeg,&s.x,&s.y);
    s.x   += rda_main.radCtX;
    s.y   += rda_main.radCtY;
    return s;
}


void Mainwindow::UpdateMouseStat(QPainter *p)
{
    mMousex = (QCursor::pos()).x();
    mMousey = (QCursor::pos()).y();
    if(!isInsideViewZone(mMousex,mMousey))return;
    //QPen penmousePointer(QColor(0x50ffffff));
    //penmousePointer.setWidth(2);
    p->setFont(QFont("Times",10));
    p->setPen(penYellow);
    p->drawText(mMousex,mMousey+25,100,15,0,ui->label_cursor_range->text());
    p->drawText(mMousex,mMousey+15,100,15,0,ui->label_cursor_azi->text());
    if(mouse_mode&MouseManualTrack)
    {
        if(isInsideViewZone(mMousex,mMousey))
        {
            PointDouble point = rda_main.ConvScrPointToKMXY(mMousex,mMousey);
            double rgKm = rda_main.mRadarData->sn_scale*80.0;
            double rgXY = rgKm*rda_main.mScale;
            //ve vong tron
            p->drawEllipse(QPoint(mMousex,mMousey),int(rgXY),int(rgXY));
            C_SEA_TRACK*track= rda_main.mRadarData->getManualTrackzone(point.x,point.y,rgKm);
            if(track)
            {
                PointDouble sTrack = rda_main.ConvWGSToScrPoint(track->lon,track->lat);
                p->drawRect(sTrack.x-9,sTrack.y-9,18,18);
            }
            //select radar target
            //select ais target
        }
        //ui->toolButton_manual_track->setChecked(false);

    }
    else if((mouse_mode&MouseVRM)||(mouse_mode&MouseELB)||(mouse_mode&MouseAutoSelect1)||(mouse_mode&MouseAutoSelect2))
    {
        short r = sqrt((mMousex - rda_main.radCtX)*(mMousex - rda_main.radCtX)+(mMousey - rda_main.radCtY)*(mMousey - rda_main.radCtY));


        if((mouse_mode&MouseAutoSelect1)||(mouse_mode&MouseAutoSelect2))
        {
            p->drawEllipse(QPoint(rda_main.radCtX,rda_main.radCtY),r,r);
            p->drawLine(QPoint(rda_main.radCtX,rda_main.radCtY),QPoint(rda_main.radCtX-(rda_main.radCtX-mMousex)*100,rda_main.radCtY-(rda_main.radCtY-mMousey)*100));
        }
        else
        {
            if(mouse_mode&MouseVRM)p->drawEllipse(QPoint(rda_main.radCtX,rda_main.radCtY),r,r);
            if(mouse_mode&MouseELB)p->drawLine(QPoint(rda_main.radCtX,rda_main.radCtY),QPoint(rda_main.radCtX-(rda_main.radCtX-mMousex)*100,rda_main.radCtY-(rda_main.radCtY-mMousey)*100));
        }
    }
    /*if(isSelectingTarget)
    {

        QPen penmousePointer(QColor(0x50ffffff));
        penmousePointer.setWidth(2);
        penmousePointer.setStyle(Qt::DashDotLine);
        p->setPen(penmousePointer);
        p->drawLine(selZone_x1,selZone_y1,mMousex,selZone_y1);
        p->drawLine(selZone_x1,selZone_y1,selZone_x1,mMousey);
        p->drawLine(selZone_x1,mMousey,mMousex,mMousey);
        p->drawLine(mMousex,selZone_y1,mMousex,mMousey);
    }*/
}
//bool fisrtTime = true;
//QRectF signRectTemp;
void Mainwindow::paintEvent(QPaintEvent *event)
{
    //CConfig::time_now_ms  = QDateTime::currentMSecsSinceEpoch();
    clkBegin = clock();

    //printf("paint:%ld\n",clkBegin);
    QPainter p(this);
    //p.setRenderHint(QPainter::Antialiasing, true);
    if(pMap)
    {
        p.drawPixmap(SCR_LEFT_MARGIN,SCR_TOP_MARGIN,SCR_H,SCR_H,
                     *pMap,
                     dxMap,dyMap,SCR_H,SCR_H);
        //printf("drawmap");_flushall();
    }
    //draw signal
    QRectF screen(0,0,SCR_W,SCR_H);
    if(isHeadUp)//isHeadUp)
    {
        QImage newImg = rda_main.mRadarData->getMimg_ppi()->transformed(mTrans);
        QRectF signRectTemp = QRectF(newImg.width()/2-(rda_main.radCtX),newImg.height()/2-(rda_main.radCtY),SCR_W,SCR_H);
        p.drawImage(screen,newImg,signRectTemp,Qt::AutoColor);

        //        pMapPainter.drawPixmap((-pix.width()/2+pMap->width()/2),
        //                     (-pix.height()/2+pMap->height()/2),pix.width(),pix.height(),pix
        //                     );
    }
    else
    {
        QRectF signRect(RAD_DISPLAY_RES-(rda_main.radCtX),RAD_DISPLAY_RES-(rda_main.radCtY),SCR_W,SCR_H);
        p.drawImage(screen,*rda_main.mRadarData->getMimg_ppi(),signRect,Qt::AutoColor);
    }
    //draw zoom rect
    p.setPen(QPen(QColor(255,255,255,200),0,Qt::DashLine));
    p.setBrush(Qt::NoBrush);
    p.drawRect(mZoomCenterx-zoom_size/2.0,mZoomCentery-zoom_size/2.0,zoom_size,zoom_size);
    //draw radar targets
    if(isRadarShow)rda_main.DrawRadarTargets(&p);
    UpdateMouseStat(&p);
    //ve luoi cu ly phuong vi
    DrawDetectZones(&p);
    if(isShowAIS)rda_main.drawARPATargets(&p);
    DrawViewFrame(&p);
    //    DrawViewFrameSquared(&p);
    //DrawIADArea(&p);

    clkEnd = clock();
    paintTime = (clkEnd-clkBegin);
    //printf("\npaint:%ldms",paintTime);
}
bool Mainwindow::isInsideViewZone(int x,int y)
{
    int range = SCR_H - 60;
    short dx = x-scrCtX;
    short dy = y-scrCtY;
    if((dx*dx+dy*dy)>(range*range/4))
        return false;
    else
        return true;
}
void Mainwindow::DrawIADArea(QPainter* p)
{
    if(zoom_mode==ZoomHiden)return;

    p->setCompositionMode(QPainter::CompositionMode_SourceOver);

    p->setBrush(QBrush(Qt::black));
    p->setPen(Qt::black);
    p->drawRect(mIADrect);
    if(zoom_mode==ZoomIAD)
    {
        //        printf("\nDraw IAD");
        if((rda_main.mRadarData->getMimg_zoom_ar()==nullptr)||(rda_main.mRadarData->getMimg_zoom_ar()->isNull()))return;
        //        printf("\nDraw IAD");
        p->setPen(QPen(Qt::white,2));


        QImage img = rda_main.mRadarData->getMimg_zoom_ar()->scaled(mIADrect.width(),mIADrect.height(),Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
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
    else if(zoom_mode==ZoomZoom)
    {
        //        printf("\nDraw ZoomZoom");
        //if((!rda_main.mRadarData->getMimg_zoom_ppi())||(rda_main.mRadarData->getMimg_zoom_ppi()->isNull()))return;
        p->drawImage(mIADrect,*rda_main.mRadarData->getMimg_zoom_ppi(),rda_main.mRadarData->getMimg_zoom_ppi()->rect());
        if(mRangeIndex>2)
        {

            p->setPen(QPen(QColor(255,255,255,200),0,Qt::DashLine));
            p->setBrush(Qt::NoBrush);
            p->drawRect(mZoomCenterx-zoom_size/2.0,mZoomCentery-zoom_size/2.0,zoom_size,zoom_size);
        }

    }
    else if(zoom_mode==ZoomHistogram)
    {

        p->drawImage(mIADrect,*rda_main.mRadarData->getMimg_histogram(),
                     rda_main.mRadarData->getMimg_histogram()->rect());

    }
    else if(zoom_mode==ZoomSpectre)
    {

        p->drawImage(mIADrect,*rda_main.mRadarData->getMimg_spectre(),
                     rda_main.mRadarData->getMimg_spectre()->rect());
    }
    else if(zoom_mode==ZoomRamp)
    {
        if(ui->toolButton_scope_2->isChecked()==false)rda_main.mRadarData->drawRamp();
        QRect rect1 = mIADrect;
        rect1.adjust(0,0,0,-mIADrect.height()/2);
        //        pengrid.setWidth(10);
        //        p->setPen(pengrid);
        p->drawImage(rect1,*rda_main.mRadarData->getMimg_RAmp());
        double rampos = ui->horizontalSlider_ramp_pos_2->value()/(double(ui->horizontalSlider_ramp_pos_2->maximum()));
        QRect rect2 = mIADrect;
        rect2.adjust(0,mIADrect.height()/2,0,0);
        int zoomw = rect2.width()/2;
        int ramposInt = (rda_main.mRadarData->getMimg_RAmp()->width()-zoomw)*rampos;
        QRect srect(ramposInt,0,zoomw,rda_main.mRadarData->getMimg_RAmp()->height());
        p->drawImage(rect2,*rda_main.mRadarData->getMimg_RAmp(),srect);
        //p->drawRect(rect1,rda_main.mRadarData->getMimg_RAmp()->width()+5,rda_main.mRadarData->getMimg_RAmp()->height()+5);
        //        pengrid.setWidth(2);
        //        pengrid.setColor(QColor(128,128,0,120));
        //        p->setPen(pengrid);
        //        for(short i=60;i<rda_main.mRadarData->getMimg_RAmp()->height();i+=50)
        //        {
        //            p->drawLine(0,height()-i,rda_main.mRadarData->getMimg_RAmp()->width()+5,height()-i);
        //        }
        //        for(short i=110;i<rda_main.mRadarData->getMimg_RAmp()->width();i+=100)
        //        {
        //            p->drawLine(i,height()-266,i,height());
        //        }
    }
}
//void MainWindow::keyPressEvent(QKeyEvent *event)
//{
//    if(event->key() == Qt::Key_F1)
//    {
//    selectobject = true;
//    }
//    switch(event->key())
//    {
//    case Qt::Key_Alt:
//        if(ui->menuBar->isVisible())
//            ui->menuBar->hide();
//        else
//            ui->menuBar->show();
//        break;
//    default:
//        break;
//    }

//}


void Mainwindow::SaveBinFile()
{
    //vnmap.SaveBinFile();

}
void Mainwindow::setDistanceUnit(int unit)//0:NM, 1:KM
{
    mDistanceUnit = unit;
    CConfig::setValue("mDistanceUnit",mDistanceUnit);
    if(mDistanceUnit==0)
    {
        rangeRatio = 1.852;
        strDistanceUnit = "NM";
        ui->toolButton_setRangeUnit->setText(QString::fromUtf8("Đơn vị đo:NM"));
        UpdateScale();
    }
    else if(mDistanceUnit==1)
    {
        rangeRatio = 1.0;
        strDistanceUnit = "KM";
        ui->toolButton_setRangeUnit->setText(QString::fromUtf8("Đơn vị đo:KM"));
        UpdateScale();
    }
    isMapOutdated = true;
}
void Mainwindow::targetTableItemMenu(int row,int col)
{
    QTableWidgetItem* item =  ui->tableWidgetTarget_2->item(row,0);
    if(!item)return;
    int selectedTrackID = item->text().toInt();
    mTargetMan.currTrackPt = mTargetMan.getTargetById(selectedTrackID);
    if(mTargetMan.currTrackPt)
    {
        QMenu contextMenu(tr("Context menu"), this);
        QAction action1(QString::fromUtf8("Bỏ chỉ thị"), this);
        connect(&action1, SIGNAL(triggered()), this, SLOT(removeTarget()));
        contextMenu.addAction(&action1);
        contextMenu.exec((QCursor::pos()));
    }
}
void Mainwindow::showTrackContext()
{

    QMenu contextMenu(tr("Context menu"), this);
    contextMenu.setStyleSheet("background-color: rgb(16, 32, 64);color:rgb(255, 255, 255);font: bold 12pt \"MS Shell Dlg 2\";");
    QAction action2(QString::fromUtf8("Đặt cờ địch"), this);
    connect(&action2, SIGNAL(triggered()), this, SLOT(setEnemy()));
    contextMenu.addAction(&action2);
    //
    QAction action3(QString::fromUtf8("Đặt cờ ta"), this);
    connect(&action3, SIGNAL(triggered()), this, SLOT(setFriend()));
    contextMenu.addAction(&action3);
#ifndef THEON
    //add to target
    QAction action0(QString::fromUtf8("Đặt chỉ thị"), this);
    connect(&action0, &QAction::triggered, this, &Mainwindow::addToTargets);
    contextMenu.addAction(&action0);
    //
    QAction action6(QString::fromUtf8("Bỏ chỉ thị"), this);
    connect(&action6, SIGNAL(triggered()), this, SLOT(removeTarget()));
    contextMenu.addAction(&action6);
#endif
    //        //change id
    QAction action4(QString::fromUtf8("Đổi số hiệu"), this);
    connect(&action4, &QAction::triggered, this, &Mainwindow::changeID);
    contextMenu.addAction(&action4);
    //delete
    QAction action1(QString::fromUtf8("Xóa"), this);
    connect(&action1, &QAction::triggered, this, &Mainwindow::removeTrack);
    contextMenu.addAction(&action1);

    contextMenu.exec(QPoint(mMousex,mMousey));
}
void Mainwindow::trackTableItemMenu(int row,int col)
{
    QTableWidgetItem* item =  ui->tableWidgetTarget->item(row,0);
    if(!item)return;
    int selectedTrackID = item->text().toInt();
    //mTargetMan.currTrackPt = mTargetMan.getTrackById(selectedTrackID);
    //mTargetMan.setSelectedTrack(selectedTrackID);
    for (uint i = 0;i<MAX_TRACKS_COUNT;i++)
    {
        C_SEA_TRACK* track = &(rda_main.mRadarData->mTrackList[i]);
        if(track->isRemoved())continue;
        if(track->uniqId==selectedTrackID)
        {
            selectedTrack = track;
            showTrackContext();
            return;
        }
    }


}
void Mainwindow::changeID()
{
    int value=1;
    DialogInputValue *dlg= new DialogInputValue(this,&value);
    dlg->exec();
    if(value<1)return;
    if(value>MAX_TRACKS_COUNT*10)value = MAX_TRACKS_COUNT*10;
    for (uint i = 0;i<MAX_TRACKS_COUNT;i++)
    {
        C_SEA_TRACK* track = &(rda_main.mRadarData->mTrackList[i]);
        if(track->isRemoved())continue;
        if(track==selectedTrack)continue;
        if(track->uniqId==value)
        {
            QMessageBox msgBox;
            msgBox.setWindowTitle("Error");
            msgBox.setText(QString::fromUtf8("Số hiệu bị trùng"));
            msgBox.exec();
            return;
        }
    }
    selectedTrack->uniqId = value;
}
void Mainwindow::setEnemy()
{
    selectedTrack->flag=1;
}
void Mainwindow::setFriend()
{
    selectedTrack->flag=-1;
}
void Mainwindow::removeTarget()
{
    if(mTargetMan.currTrackPt)
    {
        //mTargetMan.currTrackPt->track->Remove();
        mTargetMan.currTrackPt->track = nullptr;
    }

}
void Mainwindow::removeTrack()
{
    if(selectedTrack)selectedTrack->Remove();

}
void Mainwindow::addToTargets()
{
    QString error = mTargetMan.addCurrTrackToTargets(selectedTrack);
    if(error.size())
    {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Lỗi đặt chỉ thị");
        msgBox.setText(error);
        msgBox.exec();
    }
}
void Mainwindow::SetUpTheonGUILayout()
{

    ui->label_am2->hide();
    ui->toolButton_exit_4->hide();
    ui->toolButton_exit_3->hide();
    ui->toolButton_passive_mode->hide();
    ui->groupBox_gps_3->hide();
    ui->groupBox_statuses->setGeometry(1400,1145,500,60);
    ui->groupBox_25->setGeometry(10,1010,130,100);
    ui->groupBox_gps->setGeometry(10,1120,211,70);
    ui->groupBox_15->setGeometry(10,50,310,65);
    ui->groupBox_16->setGeometry(10,120,160,170);
    ui->groupBox_24->setGeometry(10,10,490,40);
    ui->tabWidget_iad->setGeometry(1380,610,530,540);
    ui->tabWidget_iad->show();
    ui->tabWidget_iad->setTabEnabled(5,false);
    ui->tabWidget_iad->mMoveable = false;
    ui->tabWidget_menu_2->setGeometry(1600,10,310,590);
    ui->tableWidgetTarget->setGeometry(0,0,308,450);
    ui->groupBox_3->setGeometry(10,460,280,100);
    ui->tabWidget_iad->setCurrentIndex(4);
    ui->bt_rg_5->setChecked(true);
    on_bt_rg_5_clicked();
    ui->textBrowser_message->setStyleSheet("background-color:black");
    ui->textBrowser_message->setFont(QFont("Times", 8));
    ui->groupBox_8->setGeometry(1230,10,360,250);
    //   ui->groupBox_14->setGeometry(1450,390,160,120);
    ui->groupBox_5->setGeometry(1430,270,160,100);

}
void Mainwindow::checkCuda()
{

    //system("taskkill /f /im cudaFFT.exe");
    //    int a=rda_main.processing->mCudaAge200ms;
    if(rda_main.processing->mCudaAge200ms<20)return;

    if(rda_main.processing->getIsPlaying())return;
    else {
        if(CConfig::getInt("runWithOutCuda",0))return;
        CConfig::AddMessage(QString::number(rda_main.processing->mCudaAge200ms));
//        printf("\nreset cuda:%d ",rda_main.processing->mCudaAge200ms);
        system("taskkill /f /im cudaFFT.exe");
        QFileInfo check_file("D:\\HR2D\\cudaFFT.exe");
        if (check_file.exists() && check_file.isFile())
        {

            system("start D:\\HR2D\\cudaFFT.exe");
            CConfig::AddMessage(QString::fromUtf8("Khởi động core FFT: OK"));
            rda_main.processing->mCudaAge200ms=-30;
        }
        else
        {
            CConfig::AddMessage(QString::fromUtf8("Không tìm thấy cudaFFT.exe"));
        }
    }

    //const char* systemCommand = "start D:\\HR2D\\cudaFFT.exe";
    //const char* exeFile = "cudaFFT.exe";
    //system("taskkill /f /im cudaFFT.exe");
    //system("start D:\\HR2D\\cudaFFT.exe");


}
void Mainwindow::InitSetting()
{

    rda_main.showAisName = false;
    rda_main.rect = this->rect();
    rda_main.dialogTargetInfo =dialogTargetInfo;
    rda_main.radCtX= SCR_H/2 + SCR_LEFT_MARGIN;
    rda_main.radCtY= SCR_H/2+SCR_TOP_MARGIN;
    CConfig::setGPSLocation(CConfig::getDouble("mLat",DEFAULT_LAT),
                            CConfig::getDouble("mLon",DEFAULT_LONG));
    rda_main.setCenterLonLat(CConfig::getDouble("mLon"),CConfig::getDouble("mLat"));
#ifndef THEON
    checkCuda();
#endif
    ui->tabWidget_iad->setGeometry(200,-800,ui->tabWidget_iad->width(),ui->tabWidget_iad->height());
    ui->tabWidget_iad->hide();
    ui->tabWidget_iad->mMoveable = true;
    ui->tabWidget_iad->raise();
    CConfig::mStat.antennaBearingTxStopDeg = CConfig::getDouble("antennaBearingTxStopDeg",150.0);
    CConfig::mStat.antennaBearingTxStartDeg = CConfig::getDouble("antennaBearingTxStartDeg",210.0);
#ifdef THEON
    SetUpTheonGUILayout();
#else
    ui->toolButton_hdsd->hide();
    ui->customButton_openCPN->hide();
    ui->toolButton_dzs_1->hide();
    ui->bt_rg_3->setChecked(true);on_bt_rg_3_clicked();
#endif
    ui->toolButton_xl_nguong_4->setChecked(CConfig::getInt("cut_noise"));
    ui->toolButton_sled->setChecked(CConfig::getInt("isShowSled"));
    updateSimTargetStatus();
    ui->tabWidget_menu_2->setCurrentIndex(0);
    //penTargetEnemySelected.setWidth(3);
    //penTarget.setWidth(2);
    //penTargetEnemy.setWidth(3);
    //penTargetEnemy.setStyle(Qt::DashLine);
    ui->tableWidgetTarget->setRowCount(24);
    ui->tableWidgetTarget->setStyleSheet("QTableView{gridline-color: gray;background-color:black}"
                                         "QTableView::item{color:white; background:#000000; font-weight:900; }"
                                         "QHeaderView::section { color:white; background-color:rgb(24, 48, 64); }");
    ui->tableWidgetTarget_2->setStyleSheet("QTableView{gridline-color: gray;}"
                                           "QTableView::item{color:white; background:#000000; font-weight:900; }"
                                           "QHeaderView::section { color:white; background-color:rgb(24, 48, 64); }");
    ui->tableWidgetTarget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableWidgetTarget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->tableWidgetTarget, SIGNAL(cellClicked(int ,int)), this, SLOT(trackTableItemMenu(int,int)));
    ui->tableWidgetTarget_2->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableWidgetTarget_2->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->tableWidgetTarget_2, SIGNAL(cellClicked(int ,int)), this, SLOT(targetTableItemMenu(int,int)));
    on_toolButton_signal_type_2_clicked();
    ui->toolButton_signal_type_2->setChecked(true);
    qApp->setStyleSheet("QToolTip { color: #ffffff; background-color: #2a82da; border: 1px solid white; }");
    ui->tabWidget_iad->SetTransparent(true);
    //    QApplication::setOverrideCursor(Qt::CrossCursor);

    mMaxTapMayThu = CConfig::getInt("mMaxTapMayThu");
    //mRangeIndex = CConfig::getInt("mRangeIndex");

    UpdateScale();
    //assert(mRangeLevel>=0&&mRangeLevel<8);
    setDistanceUnit(CConfig::getInt("mDistanceUnit"));
    //assert(mDistanceUnit>=0&&mDistanceUnit<2);

    //rda_main.mRadarData->setAziOffset(mHeadingT);
    //    ui->textEdit_heading->setText(CConfig::getString("mHeadingT"));
    //    ui->textEdit_heading_2->setText(CConfig::getString("mHeadingT2"));
    mZoomSizeAz = CConfig::getDouble("mZoomSizeAz",5);
    ui->textEdit_size_ar_a->setText(QString::number(mZoomSizeAz));
    mZoomSizeRg = CConfig::getDouble("mZoomSizeRg",2);
    ui->textEdit_size_ar_r->setText(QString::number(mZoomSizeRg));

    //load map
    osmap = new CMap();
    //SetGPS(CConfig::mLat, CConfig::mLon);
    osmap->setImgSize(SCR_H-20,SCR_H-20);
    osmap->SetType(0);
    mMapOpacity = CConfig::getDouble("mMapOpacity");
    //config.setMapOpacity(value/50.0);
    ui->horizontalSlider_map_brightness->setValue(mMapOpacity*50);
    //    ui->toolButton_xl_nguong_3->setChecked(true);
    ui->groupBox_control->setHidden(true);
    //    ui->groupBox_control_setting->setHidden(true);
    setMouseTracking(true);
    //initGraphicView();21.433170, 106.624043
    //init the guard zone
    //    gz1.isActive = 0;
    //    gz2.isActive = 0;
    //    gz3.isActive = 0;
    //    ui->groupBox_3->setCurrentIndex(0);
    ui->tabWidget_iad->setCurrentIndex(0);
    ui->tabWidget_menu->setCurrentIndex(0);
    QRect rec = QApplication::desktop()->screenGeometry(0);
    setFixedSize(SCR_W,SCR_H);
    if((rec.height()==SCR_H)&&(rec.width()==SCR_W))
    {
        this->showFullScreen();
        this->setGeometry(QApplication::desktop()->screenGeometry(0));//show on first screen
    }
    else if(rec.width()==SCR_W)
    {
        this->showFullScreen();
    }
    else
    {

        rec = QApplication::desktop()->screenGeometry(1);
        if((rec.height()==SCR_H)&&(rec.width()==SCR_W))
        {
            this->showFullScreen();
            //printf("error");
            this->setGeometry(QApplication::desktop()->screenGeometry(1));//show on second screen
            //setFixedSize(QApplication::desktop()->screenGeometry(1));
        }


    }

    dxMax = SCR_H/6-10;
    dyMax = SCR_H/6-10;
    mZoomCenterx = scrCtX ;
    mZoomCentery = scrCtY ;
    //ui->horizontalSlider_2->setValue(config.m_config.cfarThresh);

    ui->horizontalSlider_brightness->setValue(ui->horizontalSlider_brightness->maximum()/3.5);
    //    ui->horizontalSlider_gain->setValue(ui->horizontalSlider_gain->maximum());
    //    ui->horizontalSlider_rain->setValue(ui->horizontalSlider_rain->minimum());
    //    ui->horizontalSlider_sea->setValue(ui->horizontalSlider_sea->minimum());
    //ui->comboBox_radar_resolution->setCurrentIndex(0);
    //    connect(ui->textEdit_heading, SIGNAL(returnPressed()),ui->toolButton_set_heading,SIGNAL(clicked()));
    connect(ui->lineEdit_byte_1, SIGNAL(returnPressed()),ui->toolButton_send_command,SIGNAL(clicked()));
    connect(ui->lineEdit_byte_2, SIGNAL(returnPressed()),ui->toolButton_send_command,SIGNAL(clicked()));
    connect(ui->lineEdit_byte_3, SIGNAL(returnPressed()),ui->toolButton_send_command,SIGNAL(clicked()));
    connect(ui->lineEdit_byte_4, SIGNAL(returnPressed()),ui->toolButton_send_command,SIGNAL(clicked()));
    connect(ui->lineEdit_byte_5, SIGNAL(returnPressed()),ui->toolButton_send_command,SIGNAL(clicked()));
    connect(ui->lineEdit_byte_6, SIGNAL(returnPressed()),ui->toolButton_send_command,SIGNAL(clicked()));
    connect(ui->lineEdit_byte_7, SIGNAL(returnPressed()),ui->toolButton_send_command,SIGNAL(clicked()));


    //vnmap.setUp(config.m_config.lat(), config.m_config.lon(), 200,config.m_config.mapFilename.data());
    if(pMap)delete pMap;
    pMap = new QPixmap(SCR_H,SCR_H);
    //pViewFrame = new QPixmap(SCR_W,SCR_H);
    setMouseMode(MouseDrag,true);
    isMapOutdated = true;
    // hide menu
    ui->tabWidget_menu->setGeometry(200,-800,ui->tabWidget_menu->width(),ui->tabWidget_menu->height());
    ui->tabWidget_menu->hide();
    ui->tabWidget_menu->mMoveable = true;
    ui->tabWidget_menu->raise();

}
void Mainwindow::ReloadSetting()
{

}
bool Mainwindow::CalcAziContour(double theta, double d)
{
    while (theta>359.99)theta-=360.0;
    while(theta<0.0)theta+=360.0;
    double tanA = tan(theta/57.295779513);
    double sinA = sin(theta/57.295779513);
    double cosA = cos(theta/57.295779513);

    if(abs(theta)<0.01)
    {
        mBorderPoint2.setX(scrCtX  - dx);
        mBorderPoint2.setY(scrCtY - sqrt((d*d/4.0- dx*dx)));
        mBorderPoint1.setX(mBorderPoint2.x());
        mBorderPoint1.setY(mBorderPoint2.y()-5.0);
        mBorderPoint0.setX(mBorderPoint2.x());
        mBorderPoint0.setY(mBorderPoint2.y()-18.0);
    }
    else if(abs(theta-180)<0.01)
    {
        mBorderPoint2.setX(scrCtX  - dx);
        mBorderPoint2.setY(scrCtY + sqrt((d*d/4.0- dx*dx)));
        mBorderPoint1.setX(mBorderPoint2.x());
        mBorderPoint1.setY(mBorderPoint2.y()+5.0);
        mBorderPoint0.setX(mBorderPoint2.x());
        mBorderPoint0.setY(mBorderPoint2.y()+18.0);
    }
    else if (theta<180)
    {
        double a = (1.0+1.0/tanA/tanA);//4*(dy/tanA-dx)*(dy/tanA-dx) -4*(1+1/tanA)*(dx*dx+dy*dy-width()*width()/4);
        double b= 2.0*(dy/tanA - dx);
        double c= dx*dx+dy*dy-d*d/4.0;
        double delta = b*b-4.0*a*c;
        if(delta<30.0)return false;
        delta = sqrt(delta);
        double rx = (-b + delta)/2.0/a;
        double ry = -rx/tanA;
        if(abs(rx)<100&&abs(ry)<100)return false;
        mBorderPoint2.setX(scrCtX + rx -dx+1);
        mBorderPoint2.setY(scrCtY + ry-dy);
        mBorderPoint1.setX(mBorderPoint2.x()+5.0*sinA);
        mBorderPoint1.setY(mBorderPoint2.y()-5.0*cosA);
        mBorderPoint0.setX(mBorderPoint2.x()+18.0*sinA);
        mBorderPoint0.setY(mBorderPoint2.y()-18.0*cosA);
    }
    else
    {
        double a = (1.0+1.0/tanA/tanA);//4*(dy/tanA-dx)*(dy/tanA-dx) -4*(1+1/tanA)*(dx*dx+dy*dy-width()*width()/4);
        double b= 2.0*(dy/tanA - dx);
        double c= dx*dx+dy*dy-d*d/4.0;
        double delta = b*b-4.0*a*c;
        if(delta<30.0)return false;
        delta = sqrt(delta);
        double rx = (-b - delta)/2.0/a;
        double ry = -rx/tanA;
        if(abs(rx)<100&&abs(ry)<100)return false;
        mBorderPoint2.setX(scrCtX + rx - dx);
        mBorderPoint2.setY(scrCtY + ry - dy);
        mBorderPoint1.setX(mBorderPoint2.x()+5.0*sinA);
        mBorderPoint1.setY(mBorderPoint2.y()-5.0*cosA);
        mBorderPoint0.setX(mBorderPoint2.x()+18.0*sinA);
        mBorderPoint0.setY(mBorderPoint2.y()-18.0*cosA);
    }
    return true;

}

void Mainwindow::DrawViewFrameSquared(QPainter* p)
{
    printf("not implemented yet");
}
void Mainwindow::DrawViewFrame(QPainter* p)
{
    if(toolButton_grid_checked)
    {
        if(ui->toolButton_measuring->isChecked())
        {
            DrawGrid(p,mMouseLastX,mMouseLastY);
        }
        else
        {
            DrawGrid(p,rda_main.radCtX,rda_main.radCtY);
        }
    }
    //fill back ground
    p->setPen(QColor(24 ,48 ,64,255));
    p->setBrush(QBrush(QColor(24 ,48 ,64,255)));
    p->drawRect(SCR_H+SCR_LEFT_MARGIN,SCR_TOP_MARGIN,SCR_W-SCR_H-SCR_LEFT_MARGIN,SCR_H);
    p->drawRect(0,0,SCR_LEFT_MARGIN,SCR_H);
    p->setBrush(Qt::NoBrush);
#ifdef THEON
    QPen penBackground(QBrush(QColor(24 ,48 ,64,255)),224+SCR_BORDER_SIZE);
    QRect circleRect = ppiRect.adjusted(-135,-135,135,135);
    p->setPen(penBackground);
    p->drawEllipse(circleRect);
#else

    QPen penBackground(QBrush(QColor(24 ,48 ,64,255)),150+SCR_BORDER_SIZE);
    QRect circleRect = ppiRect.adjusted(-135,-135,135,135);
    p->setPen(penBackground);
    p->drawEllipse(circleRect);
#endif
    p->setPen(penYellow);
    p->drawEllipse(ppiRect);
    p->setFont(QFont("Times", 10));
    CConfig::mStat.antennaAziDeg = degrees(rda_main.mRadarData->getCurAziTrueRad());//todo
    //ve vanh goc ngoai
    for(short theta=0;theta<360;theta+=2)
    {
        if(CalcAziContour(theta+rda_main.trueShiftDeg,SCR_H - SCR_BORDER_SIZE))
        {

            if(!(theta%10))
            {
                p->drawLine(mBorderPoint1,mBorderPoint2);
                p->drawText(mBorderPoint0.x()-25,mBorderPoint0.y()-10,50,20,
                            Qt::AlignHCenter|Qt::AlignVCenter,
                            QString::number(theta));
            }
            else p->drawPoint(mBorderPoint1);
        }
    }

#ifndef THEON
    //ve vanh goc trong
    p->setPen(QPen(Qt::cyan,1,Qt::SolidLine,Qt::RoundCap));
    for(short theta=0;theta<360;theta+=10)
    {
        if(CalcAziContour(theta+rda_main.headShift,SCR_H - SCR_BORDER_SIZE-40))
        {
            int value = theta;
            if(value>180)value-=360;
            if(!value)continue;
            p->drawLine(mBorderPoint0,mBorderPoint1);
            p->drawText(mBorderPoint2.x()-25,mBorderPoint2.y()-10,50,20,
                        Qt::AlignHCenter|Qt::AlignVCenter,
                        QString::number(value));
        }
    }

    double radHeading;
    if(isHeadUp)
    {
        radHeading=0;
    }else radHeading = (CConfig::mStat.shipHeadingDeg);
    //drawn own ship

    if(CalcAziContour(radHeading,SCR_H-SCR_BORDER_SIZE-18))
    {
        QPoint p1(rda_main.radCtX+30*sin(radians(radHeading)),
                  rda_main.radCtY-30*cos(radians(radHeading)));
        QPoint p2(rda_main.radCtX+15*sin(radians(radHeading+45)),
                  rda_main.radCtY-15*cos(radians(radHeading+45)));
        QPoint p3(rda_main.radCtX+25*sin(radians(radHeading+155)),
                  rda_main.radCtY-25*cos(radians(radHeading+155)));
        QPoint p4(rda_main.radCtX+25*sin(radians(radHeading-155)),
                  rda_main.radCtY-25*cos(radians(radHeading-155)));
        QPoint p5(rda_main.radCtX+15*sin(radians(radHeading-45)),
                  rda_main.radCtY-15*cos(radians(radHeading-45)));
        p->drawLine(p1,mBorderPoint1);
        p->drawLine(p1,p2);
        p->drawLine(p2,p3);
        p->drawLine(p3,p4);
        p->drawLine(p4,p5);
        p->drawLine(p5,p1);

    }
    //antenna angle
    double antennaAngle = (CConfig::mStat.antennaAziDeg)+rda_main.trueShiftDeg;
    if(CalcAziContour(antennaAngle,SCR_H-SCR_BORDER_SIZE-20))
    {
        p->setPen(QPen(Qt::red,4,Qt::SolidLine,Qt::RoundCap));
        p->drawLine(mBorderPoint2,mBorderPoint1);
        QPoint p1(rda_main.radCtX+20*sin(radians(antennaAngle)),
                  rda_main.radCtY-20*cos(radians(antennaAngle)));
        p->drawLine(p1.x(),p1.y(),rda_main.radCtX,rda_main.radCtY);
        //draw text

    }




    //plot cur azi
    //    if(CalcAziContour(rda_main.processing->mAntennaAzi,SCR_H-SCR_BORDER_SIZE-20))
    //    {
    //        p->setPen(QPen(Qt::red,4,Qt::SolidLine,Qt::RoundCap));
    //        p->drawLine(mBorderPoint2,mBorderPoint1);
    //        //draw text
    //        //p->drawText(720,20,200,20,0,"Antenna: "+QString::number(aziDeg,'f',1));

    //    }
    if(CalcAziContour((CConfig::mStat.antennaAziDeg)+rda_main.trueShiftDeg,SCR_H-SCR_BORDER_SIZE-20))
    {
        p->setPen(QPen(Qt::red,4,Qt::SolidLine,Qt::RoundCap));
        p->drawLine(mBorderPoint2,mBorderPoint1);
        //draw text
        //p->drawText(720,20,200,20,0,"Antenna: "+QString::number(aziDeg,'f',1));

    }

#endif

}
//void Mainwindow::setScaleNM(unsigned short rangeNM)
//{
//    float oldScale = rda_main.mScale;
//    rda_main.mScale = (float)height()/((float)rangeNM*CONST_NM)*0.7f;
//    //printf("scale:%f- %d",scale,rangeNM);
//    isScaleChanged = true;// scale*SIGNAL_RANGE_KM/2048.0f;

//    dyMax = MAX_VIEW_RANGE_KM*rda_main.mScale;
//    dxMax = dyMax;
//    dx =short(rda_main.mScale/oldScale*dx);
//    dy =short(rda_main.mScale/oldScale*dy);
//    DrawMap();
//    /*currMaxRange = (sqrtf(dx*dx+dy*dy)+scrCtY)/signsize;
//    if(currMaxRange>RADAR_MAX_RESOLUTION)currMaxRange = RADAR_MAX_RESOLUTION;*/
//    isScreenUp2Date = false;
//}
short waittimer =0;
void Mainwindow::DisplayClkAdc(int clk)
{
    switch(clk)
    {
    case 0:
        ui->label_range_resolution->setText("15m");
        break;
    case 1:
        ui->label_range_resolution->setText("30m");
        break;
    case 2:
        ui->label_range_resolution->setText("60m");
        break;
    case 3:
        ui->label_range_resolution->setText("120m");
        break;
    case 4:
        ui->label_range_resolution->setText("240m");
        break;
    case 5:
        ui->label_range_resolution->setText("0x05");
        break;
    case 6:
        ui->label_range_resolution->setText("0x06");
        break;
    default:
        ui->label_range_resolution->setText("N/A");
        break;

    }
}
void Mainwindow::UpdateVideo()
{

    //clock_t ageVideo = clock()-rda_main.mRadarData->mUpdateTime;
    if(rda_main.mRadarData->UpdateData())
    {
        if(rda_main.mRadarData->isClkAdcChanged)
        {
            //ui->comboBox_radar_resolution->setCurrentIndex(rda_main.mRadarData->clk_adc);
            DisplayClkAdc(rda_main.mRadarData->clk_adc);
            rda_main.mRadarData->setScalePPI(rda_main.mScale);
            this->UpdateScale();
            //            printf("\nsetScale:%d",rda_main.mRadarData->clk_adc);
            rda_main.mRadarData->isClkAdcChanged = false;
        }
        repaint();
    }

    /*QStandardItemModel* model = new QStandardItemModel(trackListPt->size(), 5);
    for (int row = 0; row < trackListPt->size(); ++row)
    {
       for (int column = 0; column < 5; ++column)
       {
           QString text = QString('A' + row) + QString::number(column + 1);
           QStandardItem* item = new QStandardItem(text);
           model->setItem(row, column, item);
       }
    }
    ui->tableTargetList->setModel(model);*/
}
void Mainwindow::readBuffer()
{

}
void Mainwindow::InitTimer()
{
    tprocessing = new QThread();
    rda_main.processing = new dataProcessingThread();
    rda_main.mRadarData = rda_main.processing->mRadarData;
    //init simulator
    simulator = new c_radar_simulation(rda_main.processing->mRadarData);
    connect(this,SIGNAL(destroyed()),simulator,SLOT(deleteLater()));
    simulator->start(QThread::HighPriority);
    //
    connect(&syncTimer1s, SIGNAL(timeout()), this, SLOT(sync1S()));
    syncTimer1s.start(1000);
    connect(&syncTimer5p, SIGNAL(timeout()), this, SLOT(sync1p()));
    syncTimer5p.start(60000);
    //syncTimer1s.moveToThread(t);

    connect(&timerVideoUpdate, SIGNAL(timeout()), this, SLOT(UpdateVideo()));
    timerVideoUpdate.start(60);//ENVDEP
    //scrUpdateTimer.moveToThread(t2);
    //connect(t2,SIGNAL(finished()),t2,SLOT(deleteLater()));
    //    dataPlaybackTimer = new QTimer(this);
    connect(this,SIGNAL(destroyed()),rda_main.processing,SLOT(deleteLater()));
    //    connect(dataPlaybackTimer,SIGNAL(timeout()),rda_main.processing,SLOT(playbackRadarData()));
    rda_main.processing->start(QThread::TimeCriticalPriority);
    tprocessing->start(QThread::HighPriority);

    connect(&timerMetaUpdate, SIGNAL(timeout()), this, SLOT(Update100ms()));
    timerMetaUpdate.start(100);//ENVDEP

}
void Mainwindow::SetTx(bool isOn)
{
    CConfig::mStat.isTransmitting = isOn;
    if(!isOn)
    {
        sendToRadarString(CConfig::getString("mRxCommand","aaab0000; aaab0100"));
    }
    if(isOn)
    {
#ifdef THEON
        sendToRadarString(CConfig::getString("mTxCommand","aaab0201;aaab0001;16ab0b00ff;13ab0c;08ab02;01ab0104;27ab01"));
#else
        sendToRadarString(CConfig::getString("mTxCommand",
                                             "aaab0001;aaab0101;17ab0100;24ab0064;24ab0163;24ab0262;24ab035e;24ab035e;24ab045d;24ab055c;24ab0658;24ab0757;24ab0856;24ab0952;24ab0a51;24ab0b50;24ab0064"));
#endif
    }


}
void Mainwindow::Update100ms()
{
    //update rda_main.target_size


    //update info labels
    ui->label_cur_freq->setText(QString::number((rda_main.mRadarData->mFreq)+1));
    //smooth the heading
    ui->label_head_ship->setText(QString::number(CConfig::mStat.shipHeadingDeg,'f',1));
    ui->label_course_ship->setText(QString::number(CConfig::mStat.shipCourseDeg,'f',1));
    ui->label_speed_ship->setText(QString::number(CConfig::mStat.shipSpeedWater,'f',1)
                                  +"/"
                                  +QString::number(CConfig::mStat.shipSpeedGround,'f',1));

    mIADrect = ui->tabWidget_iad->geometry();
    mIADrect.adjust(4,30,-5,-5);
    mIADCenter.x = mIADrect.x()+(mIADrect.width())/2;
    mIADCenter.y = mIADrect.y()+(mIADrect.height())/2;
    mZoomScale = double(mIADrect.width())/zoom_size;
    zoom_size = mIADrect.width()/rda_main.mRadarData->getScale_zoom_ppi()*rda_main.mRadarData->getScale_ScreenPerPpi();
    if(ui->tabWidget_iad->isHidden())
    {
        zoom_mode = ZoomHiden;
    }
    else switch(ui->tabWidget_iad->currentIndex())
    {
    case 0 :
        zoom_mode = ZoomIAD;break;
    case 1 :
        zoom_mode = ZoomHistogram;break;
    case 2 :
        zoom_mode = ZoomSpectre;break;
    case 3 :
        zoom_mode = ZoomRamp;break;
    case 4 :
        zoom_mode = ZoomZoom;break;
    default:
        break;
    }
    //calculate heading
    if(isHeadUp)
    {
        rda_main.trueShiftDeg = -CConfig::mStat.shipHeadingDeg;
        rda_main.headShift = 0;
        //        rda_main.mRadarData->setAziViewOffsetDeg(rda_main.trueShiftDeg);
        mTrans.reset();
        mTrans = mTrans.rotate(rda_main.trueShiftDeg);
        isMapOutdated = true;
    }
    else
    {
        rda_main.trueShiftDeg = 0;
        rda_main.headShift = CConfig::mStat.shipHeadingDeg;
    }

    if(isMapOutdated)DrawMap();
    int posx = (QCursor::pos()).x();
    int posy = (QCursor::pos()).y();
    if(posx)mMousex= posx;
    if(posy)mMousey= posy;

#ifndef THEON
    if(CConfig::mStat.isTransmitting){
        CConfig::mStat.antennaBearingDeg = CConfig::mStat.antennaAziDeg - CConfig::mStat.shipHeadingDeg;
        //check if bearing is inside the prohibited sector
        if(CConfig::mStat.antennaBearingDeg<0)CConfig::mStat.antennaBearingDeg+=360;
        else if(CConfig::mStat.antennaBearingDeg>360)CConfig::mStat.antennaBearingDeg-=360;
        //        radarStatus_3C stat = CConfig::mStat;
        bool isInsideProtectedNew =(CConfig::mStat.antennaBearingDeg>CConfig::mStat.antennaBearingTxStopDeg&&CConfig::mStat.antennaBearingDeg<CConfig::mStat.antennaBearingTxStartDeg);
        if(isInsideProtectedNew!=isInsideProtected)
        {
            if(isInsideProtectedNew)sendToRadarString(CConfig::getString("mTxStopCommand",
                                                                         "17ab0000;17ab0000"));
            else
                sendToRadarString(CConfig::getString("mTxStartCommand",
                                                     "17ab0100;17ab0100"));
        }
        isInsideProtected =isInsideProtectedNew;
        //
    }
#endif
    if(rda_main.mRadarData->init_time)
    {
        ui->label_azi_antenna_head_true->setText(QString::number(int(CConfig::mStat.antennaAziDeg)));
    }
    else
    {
        ui->label_azi_antenna_head_true->setText(QString::number(CConfig::mStat.antennaAziDeg,'f',1));
    }
    if(isInsideViewZone(mMousex,mMousey))
    {
//        activateWindow();
        if(hasFocus())rda_main.MouseOverRadarTarget(mMousex,mMousey);
        if(mouse_mode&MouseAutoSelect1||mouse_mode&MouseAutoSelect2)
            QApplication::setOverrideCursor(Qt::DragMoveCursor);
        else if(this->hasFocus())QApplication::setOverrideCursor(cursor_default);
        else QApplication::setOverrideCursor(Qt::ArrowCursor);
        double azi,rg;
        if(ui->toolButton_measuring->isChecked())
        {
            C_radar_data::ConvkmxyToPolarDeg((mMousex - mMouseLastX)/rda_main.mScale,-(mMousey - mMouseLastY)/rda_main.mScale,&azi,&rg);

        }
        else
        {
            C_radar_data::ConvkmxyToPolarDeg((mMousex - rda_main.radCtX)/rda_main.mScale,-(mMousey - rda_main.radCtY)/rda_main.mScale,&azi,&rg);
        }
        rg/=rangeRatio;
        double headAzi ;
        if(isHeadUp)
        {
            headAzi= azi;
            azi+=CConfig::mStat.shipHeadingDeg;
            if(azi<0)azi+=360;
        }
        else
        {
            headAzi= azi-CConfig::mStat.shipHeadingDeg;
        }
        if(headAzi>180)headAzi-=360;
        if(headAzi<-180)headAzi+=360;
        ui->label_cursor_range->setText(QString::number(rg,'f',2)+strDistanceUnit);
        ui->label_cursor_azi->setText(QString::number(azi,'f',1)+degreeSymbol);
        ui->label_cursor_azi_2->setText(QString::number(headAzi,'f',1)+degreeSymbol);
        PointDouble latlon = rda_main.ConvScrPointToWGS(
                    mMousex,
                    (mMousey )
                    );
        ui->label_cursor_lat->setText(demicalDegToDegMin( latlon.y)+"'N");
        ui->label_cursor_long->setText(demicalDegToDegMin(latlon.x)+"'E");
    }
    else
    {
        QApplication::setOverrideCursor(Qt::ArrowCursor);
    }
    repaint();
}
void Mainwindow::InitNetwork()
{
    m_udpSocket = new QUdpSocket(this);
    if(!m_udpSocket->bind(8900))
    {
        if(!m_udpSocket->bind(8901))
        {
            m_udpSocket->bind(8902);
        }
    }
    m_udpSocket->setSocketOption(QAbstractSocket::MulticastTtlOption, 10);

    connect(m_udpSocket, SIGNAL(readyRead()),
            this, SLOT(processARPA()));
}
void Mainwindow::processARPA()
{// !!todo here

    while (m_udpSocket->hasPendingDatagrams())
    {
        QByteArray datagram;
        datagram.resize(m_udpSocket->pendingDatagramSize());
        m_udpSocket->readDatagram(datagram.data(), datagram.size());
        if(datagram.size()==5)
        {
            if(*((unsigned char*)datagram.data())==0xff)
            {
                curAziRad = (*((unsigned char*)datagram.data()+1)*256
                             + (*((unsigned char*)datagram.data()+2)))/2500.0*3.14159265;
            }
        }
        //printf(datagram.data());
        QString str(datagram.data());
        QStringList list = str.split(",");
        short dataStart = 0;
        for(short i=0;i<list.size()-5;i++)
        {

            if(list.at(i).contains("RATTM"))
            {
                //            short tNum = (*(list.begin()+1)).toInt();
                //            float tDistance = (*(list.begin()+2)).toFloat();
                //            float tRange = (*(list.begin()+3)).toFloat();
                //            arpa_data.adde(tNum,tDistance,tRange);
            }

            dataStart+= list.at(i).size();
        }

    }

}




//}
void Mainwindow::ShutDown()
{
    rda_main.processing->stopThread();
    rda_main.processing->wait();

    CConfig::SaveToFile();
    QApplication::quit();
#ifdef _WIN32
    system("shutdown -s -f -t 00");
#else
    //system("/sbin/halt -p");
#endif
}

//void Mainwindow::on_actionConnect_triggered()
//{

//}
void Mainwindow::sync1p()//period 1 min
{
    QString str = ui->textBrowser_message->toPlainText();
    QDateTime now = QDateTime::currentDateTime();
    /*QString dir = "D:\\HR2D\\logs\\"+now.toString("\\dd.MM\\");
    if(!QDir(dir).exists())
    {
        QDir().mkdir(dir);
    }
    if(str.size())
    {
        QFile logFile;

        QString dir = "D:\\HR2D\\logs\\"+now.toString("\\dd.MM\\");
        if(!(QDir(dir).exists()))
        {
            QDir().mkdir(dir);
        }
        logFile.setFileName(dir+("message_log")+".log");
        logFile.open(QIODevice::WriteOnly);
        logFile.write(str.toUtf8());
        logFile.close();
    }*/
    CConfig::backup();

}
void Mainwindow::saveScreenShot(QString fileName)
{
    QPixmap pixmap = QPixmap::grabWindow(QApplication::desktop()->winId());
    QFile file(fileName);
    file.open(QIODevice::WriteOnly);
    pixmap.save(&file, "PNG");
}

void Mainwindow::CheckRadarStatus()
{
#ifndef THEON
    //check Tx condition
    bool isStatOk = CheckTxCondition(false);
    //    ui->toolButton_tx->setEnabled((CConfig::mStat.getAge21()<1000));
    ui->toolButton_tx->highLight(isStatOk);
    if(rda_main.mRadarData->isTxOn)
        ui->label_am2->setText("AM2-ON");
    else
        ui->label_am2->setText("AM2-OFF");

    //tat phat khi bao hong
    if(rda_main.mRadarData->isTxOn&&(!isStatOk))
    {
        if(!ui->toolButton_tx_forced->isChecked())
        {

            //SetTx(false);
        }
    }

    //    else
    //    {
    //        if(CConfig::mStat.getAgeTempOk()>1500000)
    //        {
    //            QMessageBox msgBox;
    //            msgBox.setText(QString::fromUtf8("Máy phát quá nhiệt quá 15 phút, cấm phát tuyệt đối!"));
    //            msgBox.exec();
    //            ui->toolButton_tx_off->setChecked(true);
    //            return false;
    //        }
    //    }
#endif
    clock_t ageMay22 = CConfig::mStat.getAge22();
    if(ageMay22<5000)
    {
        if(CConfig::mStat.mMayPhatOK)
        {
            ui->label_status_may_22->setStyleSheet("color: rgb(10, 255, 10);");
            ui->label_status_may_22->setText(QString::fromUtf8("Máy phát hoạt động bình thường"));
        }
        else
        {
            ui->label_status_may_22->setStyleSheet("color: rgb(255, 10, 10);");
            ui->label_status_may_22->setText(QString::fromUtf8("Máy phát không hoàn hảo"));
        }
        if(CConfig::mStat.mTaiAngTen==1)ui->toolButton_dk_2->setChecked(true);//tai ang ten
        else if(CConfig::mStat.mTaiAngTen==0)ui->toolButton_dk_13->setChecked(true);//tai tuong duong
        if(CConfig::mStat.mMaHieu==0)ui->toolButton_dk_11->setChecked(true);//ma hieu
        else if(CConfig::mStat.mMaHieu==1)ui->toolButton_dk_16->setChecked(true);//ma hieu
        else if(CConfig::mStat.mMaHieu==2)ui->toolButton_dk_17->setChecked(true);//ma hieu
        if(CConfig::mStat.mSuyGiam==7)ui->toolButton_dk_3->setChecked(true);//suy giam
        else if(CConfig::mStat.mSuyGiam==6)ui->toolButton_dk_4->setChecked(true);//suy giam
        else if(CConfig::mStat.mSuyGiam==5)ui->toolButton_dk_5->setChecked(true);//suy giam
        else if(CConfig::mStat.mSuyGiam==4)ui->toolButton_dk_6->setChecked(true);//suy giam
        else if(CConfig::mStat.mSuyGiam==3)ui->toolButton_dk_7->setChecked(true);//suy giam
        else if(CConfig::mStat.mSuyGiam==2)ui->toolButton_dk_8->setChecked(true);//suy giam
        else if(CConfig::mStat.mSuyGiam==1)ui->toolButton_dk_9->setChecked(true);//suy giam
        else if(CConfig::mStat.mSuyGiam==0)ui->toolButton_dk_3->setChecked(true);//suy giam
        ui->groupBox_20->setTitle(QString::fromUtf8("Cao ap san sang:")+QString::number(CConfig::mStat.mCaoApReady));
        if(CConfig::mStat.mCaoApReady==2)ui->groupBox_20->setStyleSheet("background-color: rgb(70, 30, 10);color:rgb(255, 255, 255);");
        else if(CConfig::mStat.mCaoApReady==1)ui->groupBox_20->setStyleSheet("background-color: rgb(60, 60, 10);color:rgb(255, 255, 255);");
        else if(CConfig::mStat.mCaoApReady==0)ui->groupBox_20->setStyleSheet("background-color: rgb(24, 32, 64);color:rgb(255, 255, 255);");
        if(CConfig::mStat.mCaoApKetNoi==0)
        {
            ui->checkBox_cao_ap_1->setChecked(false);//cao ap
            ui->checkBox_cao_ap_2->setChecked(false);//cao ap
        }
        else if(CConfig::mStat.mCaoApKetNoi==1)
        {
            ui->checkBox_cao_ap_1->setChecked(true);//cao ap
            ui->checkBox_cao_ap_2->setChecked(false);//cao ap
        }
        else if(CConfig::mStat.mCaoApKetNoi==2)
        {
            ui->checkBox_cao_ap_1->setChecked(false);//cao ap
            ui->checkBox_cao_ap_2->setChecked(true);//cao ap
        }
        else if(CConfig::mStat.mCaoApKetNoi==3)
        {
            ui->checkBox_cao_ap_1->setChecked(true);//cao ap
            ui->checkBox_cao_ap_2->setChecked(true);//cao ap
        }
    }
    else
    {
        ui->label_status_may_22->setStyleSheet("color: rgb(255, 10, 10);");
        ui->label_status_may_22->setText(QString::fromUtf8("Mất kết nối máy phát ")+QString::number(ageMay22/1000));
    }
}
void Mainwindow::ViewTrackInfo()
{
    //find new tracks
    for(uint i =0;i<rda_main.mRadarData->mTrackList.size();i++)
    {
        C_SEA_TRACK* track = &(rda_main.mRadarData->mTrackList[i]);
        if(track->isConfirmed()&&track->isUserInitialised)
            if(!mTargetMan.checkIDExist(rda_main.mRadarData->mTrackList[i].uniqId))
                mTargetMan.addTrack(&rda_main.mRadarData->mTrackList[i]);
    }
    //track table
    int row = 0;
    for(int i=0;i<TRACK_TABLE_SIZE;i++)
    {
        TrackPointer *trackPt = mTargetMan.getTrackAt(i);
        if(trackPt)
        {

            if(row>=ui->tableWidgetTarget->rowCount())ui->tableWidgetTarget->setRowCount(row+1);
            for(int col = 0;col<5;col++)
            {
                QTableWidgetItem* item = ui->tableWidgetTarget->item(row,col);
                if(!item)
                {
                    item = new QTableWidgetItem;
                    ui->tableWidgetTarget->setItem(i,col,item);
                }
                if( trackPt->selected)item->setFont(QFont("Times", 12,QFont::Bold));
                else item->setFont(QFont("Times", 12));
                if(col==0)      item->setText(QString::number(trackPt->track->uniqId));
                else if (col==1)item->setText(QString::number(trackPt->track->aziDeg,'f',1));
                else if (col==2)item->setText(QString::number(nm(trackPt->track->rgKm),'f',2));
                else if (col==3)item->setText(QString::number(trackPt->track->courseDeg,'f',1));
                else if (col==4)item->setText(QString::number(nm(trackPt->track->mSpeedkmhFit),'f',1));
                //

            }
            row++;

        }
//        ui->tableWidgetTarget->setRowCount(row);

    }
    if(row<(ui->tableWidgetTarget->rowCount()))ui->tableWidgetTarget->setRowCount(row);
    /*for(int i=row;i<ui->tableWidgetTarget->rowCount();i++)
    {
        for(int col = 0;col<5;col++)
        {
            QTableWidgetItem* item = ui->tableWidgetTarget->item(row,col);
            if(item)item->setText("");
        }
    }*/
    //target table

    for(int i=0;i<TARGET_TABLE_SIZE;i++)
    {
        TrackPointer *trackPt = mTargetMan.getTargetAt(i);
        if(!trackPt)
        {
            for(int col = 0;col<5;col++)
            {
                QTableWidgetItem* item = ui->tableWidgetTarget_2->item(i,col);
                if(item)item->setText("");
            }
        }
        else{
            for(int col = 0;col<5;col++)
            {
                QTableWidgetItem* item = ui->tableWidgetTarget_2->item(i,col);
                if(!item)
                {
                    item = new QTableWidgetItem;
                    ui->tableWidgetTarget_2->setItem(i,col,item);
                }
                if( trackPt->selected)item->setFont(QFont("Times", 12,QFont::Bold));
                else item->setFont(QFont("Times", 12));
                if(col==0)      item->setText(QString::number(trackPt->track->uniqId));
                else if (col==1)item->setText(QString::number(trackPt->track->aziDeg,'f',1));
                else if (col==2)item->setText(QString::number((trackPt->track->rgKm),'f',2));
                else if (col==3)item->setText(QString::number(trackPt->track->courseDeg,'f',1));
                else if (col==4)item->setText(QString::number((trackPt->track->mSpeedkmhFit)/3.6,'f',1));
                //ui->tableWidgetTarget_2->setItem(i,col,item);
            }

        }

    }
    /*
    int numOfTracks = 0;
    bool selectionExist = false;
    for (uint i = 0;i<rda_main.mRadarData->mTrackList.size();i++)
    {
        track_t* track = &(rda_main.mRadarData->mTrackList[i]);
        if(!(track->isRemoved()))
        {
            if(!track->isLost())
            {
                if((mmTargetMan.selectedTrackID==track->uniqId))
                {
                    selectionExist = true;
                    if(mDistanceUnit==0)//NM
                    {
                        ui->label_data_ID->setText(QString::number(track->operatorID));
                        ui->label_data_range->setText(QString::number(nm(track->rgKm),'f',2)+"Nm");
                        ui->label_data_azi->setText(QString::number(track->aziDeg,'f',2)+degreeSymbol);
                        ui->label_data_speed->setText(QString::number(nm(track->mSpeedkmhFit),'f',1)+"Kn");
                        ui->label_data_heading->setText(QString::number(degrees(track->bearingRadFit),'f',1)+degreeSymbol);
                        ui->label_data_rg_speed->setText(QString::number(nm(-track->rgSpeedkmh),'f',1)+"Kn");
                    }
                    else
                    {
                        ui->label_data_ID->setText(QString::number(track->operatorID));
                        ui->label_data_range->setText(QString::number(track->rgKm,'f',2)+"Km");
                        ui->label_data_azi->setText(QString::number(track->aziDeg,'f',2)+degreeSymbol);
                        ui->label_data_speed->setText(QString::number(track->mSpeedkmhFit,'f',1)+"Km/h");
                        ui->label_data_heading->setText(QString::number(degrees(track->bearingRadFit),'f',1)+degreeSymbol);
                        ui->label_data_rg_speed->setText(QString::number((-track->rgSpeedkmh),'f',1)+"Km/h");
                    }
                }
            }
            numOfTracks++;
        }
    }
    if(!selectionExist)
    {
        ui->label_data_range->setText("--");
        ui->label_data_azi->setText("--");
        ui->label_data_speed->setText("--");
    }
    ui->toolButton_sled_reset_4->setText(QString::fromUtf8("Quỹ đạo(")+QString::number(numOfTracks)+")");*/
}
void Mainwindow::sync1S()//period 1 second
{
    checkCuda();

    if(CConfig::getWarningList()->size())
    {
        std::queue<WarningMessage> * listMsg = (CConfig::getWarningList());

        while(listMsg->size())
        {
            //            printf("warning added");
            WarningMessage msg = listMsg->front();
            ui->textBrowser_message->append(msg.message);
            listMsg->pop();
        }
    }
    isMapOutdated=true;
    CheckRadarStatus();
    UpdateGpsData();
    ViewTrackInfo();
    // update rate
    int sampleTime = 10*paintTime/7;
    if(sampleTime<30)sampleTime=30;
    ui->label_frame_rate->setText("SFR:"+QString::number(1000/sampleTime));

    timerVideoUpdate.start(sampleTime);
    timerMetaUpdate.start(sampleTime*4);

    ui->label_radar_fps->setText("RFR:"+QString::number(int(rda_main.processing->mFramesPerSec)));
    //target manager
    if(ui->toolButton_chi_thi_mt->isChecked())mTargetMan.OutputTargetToKasu();
    if(isScaleChanged ) {

        rda_main.mRadarData->setScalePPI(rda_main.mScale);
        isScaleChanged = false;
    }
    ui->label_speed_2->setText(QString::number(rda_main.mRadarData->rotation_per_min,'f',1)+"v/p");
    showTime();
    int nfft = pow(2,(rda_main.mRadarData->mHeader[22])+2);
    ui->label_radar_fft->setText(QString::number(nfft));
    return;
    //removed code-----------------------------


}
//void Mainwindow::setRadarState(radarSate radarState)
//{
//    if(radar_state!=radarState)
//    {
//        radar_state = radarState;
//        on_label_status_warning_clicked();
//    }
//}





void Mainwindow::on_actionOpen_rec_file_triggered()
{
    QString filename = QFileDialog::getOpenFileName(this,    tr("Open signal file"), NULL, tr("HR signal record files (*.r2d)"));
    if(!filename.size())return;
    rda_main.processing->loadRecordDataFile(filename);
    ui->label_record_file_name->setText(filename);
}



//void Mainwindow::on_actionOpen_map_triggered()
//{
//    //openShpFile();
//}
void Mainwindow::showTime()
{
    QDateTime time = QDateTime::currentDateTime();
    QString text = time.toString("hh:mm:ss");
    ui->label_date->setText(text);
    text = time.toString("dd/MM/yy");
    ui->label_time->setText(text);
}

//void Mainwindow::on_actionSaveMap_triggered()
//{
//    //vnmap.SaveBinFile();
//}

//void Mainwindow::on_actionSetting_triggered()
//{
//    //    GPSDialog *dlg = new GPSDialog(this);
//    //    dlg->setModal(false);
//    //    dlg->loadConfig(&config);
//    //    dlg->show();
//    //    dlg->setAttribute(Qt::WA_DeleteOnClose);
//    //    connect(dlg, SIGNAL(destroyed(QObject*)), SLOT(UpdateSetting()));
//    //    connect(dlg, SIGNAL(destroyed(QObject*)), SLOT(setCodeType()));
//}
//void Mainwindow::on_actionAddTarget_toggled(bool arg1)
//{
//    //isAddingTarget=arg1;

//}



void Mainwindow::on_actionPlayPause_toggled(bool arg1)
{
    rda_main.processing->togglePlayPause(arg1);
    //    if(arg1)dataPlaybackTimer->start(25);else dataPlaybackTimer->stop();

}


/*
        void MainWindow::on_pushButton_clicked()
        {

            Command_Control new_com;
            hex2bin(ui->lineEdit_byte_1->text().toStdString().data(),&new_com.bytes[0]);
            hex2bin(ui->lineEdit_byte_2->text().toStdString().data(),&new_com.bytes[1]);
            hex2bin(ui->lineEdit_byte_3->text().toStdString().data(),&new_com.bytes[2]);
            hex2bin(ui->lineEdit_byte_4->text().toStdString().data(),&new_com.bytes[3]);
            hex2bin(ui->lineEdit_byte_5->text().toStdString().data(),&new_com.bytes[4]);
            hex2bin(ui->lineEdit_byte_6->text().toStdString().data(),&new_com.bytes[5]);
            hex2bin(ui->lineEdit_byte_7->text().toStdString().data(),&new_com.bytes[6]);
            hex2bin(ui->lineEdit_byte_8->text().toStdString().data(),&new_com.bytes[7]);
            command_queue.push(new_com);
        }
        */

void Mainwindow::SendCommandControl()
{/*
              if(command_queue.size())
              {

                  if(rda_main.mRadarData->checkFeedback(&command_queue.front().bytes[0]))// check if the radar has already recieved the command
                  {


                      command_queue.pop();
                      udpFailure = 0;

                  }
                  else
                  {
                    if(udpFailure<20)//ENVDEP 20*50ms = 1s
                    {udpFailure++;}
                    else{
                        setRadarState( DISCONNECTED);
                        udpFailure = 0;
                    }
                    udpSendSocket->writeDatagram((char*)&command_queue.front().bytes[0],8,QHostAddress("192.168.0.44"),2572);
                    //
                    char xx[3];
                    xx[2]=0;
                    QString str;
                    for(short i =0;i<8;i++)
                    {
                        bin2hex(command_queue.front().bytes[i],&xx[0]);
                        str.append(xx);
                        str.append('-');
                    }

                    ui->label_command->setText(str);
                    //printf((const char*)str.data());
                    //

                  }

              }*/

}

//void Mainwindow::on_actionRecording_triggered()
//{

//}


//void Mainwindow::on_comboBox_temp_type_currentIndexChanged(int index)
//{

//}

//void RadarGui::on_horizontalSlider_brightness_actionTriggered(int action)
//{

//}

void Mainwindow::on_horizontalSlider_brightness_valueChanged(int value)
{
    rda_main.mRadarData->setBrightness(0.5f+(float)value/ ui->horizontalSlider_brightness->maximum()*4.0f);
}

/*void MainWindow::on_horizontalSlider_3_valueChanged(int value)
        {
            switch (value) {
            case 1:
                Command_Control new_com;
                new_com.bytes[0] = 4;
                new_com.bytes[1] = 0xab;
                new_com.bytes[2] = 0;
                new_com.bytes[3] = 0;
                new_com.bytes[4] = 1;
                new_com.bytes[5] = 0;
                new_com.bytes[6] = 0;
                new_com.bytes[7] = 0;//new_com.bytes[0]+new_com.bytes[1]+new_com.bytes[2]+new_com.bytes[3]+new_com.bytes[4]+new_com.bytes[5]+new_com.bytes[6];
                command_queue.push(new_com);
                break;
            case 2:
                printf("2");
                break;
            case 3:
                printf("3");
                break;
            default:
                break;
            }
        }*/



//void MainWindow::on_toolButton_toggled(bool checked)
//{
//    //if(checked)ui->toolBar_Main->show();
//    //else ui->toolBar_Main->hide();
//}

//void Mainwindow::on_actionSector_Select_triggered()
//{

//}


//void MainWindow::on_toolButton_10_clicked()
//{
//    //if(ui->frame_RadarViewOptions->isHidden())ui->frame_RadarViewOptions->show();
//    //else ui->frame_RadarViewOptions->hide();
//}




/*
        void MainWindow::on_toolButton_14_clicked()
        {
            //if(event->delta()>0)ui->horizontalSlider->setValue(ui->horizontalSlider->value()+1);
        }

        void MainWindow::on_toolButton_13_clicked()
        {
            //if(event->delta()<0)ui->horizontalSlider->setValue(ui->horizontalSlider->value()-1);
        }
        */
void Mainwindow::setScaleRange(double srange)
{
    if(mDistanceUnit==0)
    {
        rda_main.setScale( (SCR_H-SCR_BORDER_SIZE)/(rangeRatio*srange )/2);
        ringStep = srange/4;
        ui->label_range->setText(QString::number(srange)+strDistanceUnit);
    }
    else if(mDistanceUnit==1)
    {
        rda_main.setScale( (SCR_H-SCR_BORDER_SIZE)/(rangeRatio*srange )/2);
        ringStep = srange/5;
        ui->label_range->setText(QString::number(srange)+strDistanceUnit);
    }
}
void Mainwindow::UpdateScale()
{
    if(simulator->getIsPlaying())
    {
        simulator->setRange(mRangeIndex);
    }
    float oldScale = rda_main.mScale;
    if(mDistanceUnit==0)//NM
    {
        switch(mRangeIndex)
        {
        case 0:
            setScaleRange(2);
            break;
        case 1:
            setScaleRange(4);
            //if(isAdaptSn) sendToRadarString(CConfig::getString("mR2Command"));
            break;
        case 2:
            setScaleRange(8);
            //if(isAdaptSn) sendToRadarString(CConfig::getString("mR3Command"));
            break;
        case 3:
            setScaleRange(16);
            //if(isAdaptSn) sendToRadarString(CConfig::getString("mR4Command"));
            break;
        case 4:
            setScaleRange(32);
            //if(isAdaptSn) sendToRadarString(CConfig::getString("mR5Command"));
            break;
        case 5:
            setScaleRange(64);
            //if(isAdaptSn) sendToRadarString(CConfig::getString("mR6Command"));
            break;
        case 6:
            setScaleRange(128);
            //if(isAdaptSn) sendToRadarString(CConfig::getString("mR7Command"));
            break;
        case 7:
            setScaleRange(256);
            //if(isAdaptSn) sendToRadarString(CConfig::getString("mR8Command"));
            break;
        default:
            setScaleRange(48);
            break;
        }
    }
    else if(mDistanceUnit==1)
    {
        switch(mRangeIndex)
        {
        case 0:
            setScaleRange(2.5);
            //            if(isAdaptSn) sendToRadarString(CConfig::getString("mR0Command"));
            break;
        case 1:
            setScaleRange(5);
            //            if(isAdaptSn) sendToRadarString(CConfig::getString("mR1Command"));
            break;
        case 2:
            setScaleRange(10);
            //            if(isAdaptSn) sendToRadarString(CConfig::getString("mR2Command"));
            break;
        case 3:
            setScaleRange(20);
            //            if(isAdaptSn) sendToRadarString(CConfig::getString("mR3Command"));
            break;
        case 4:
            setScaleRange(40);
            //            if(isAdaptSn) sendToRadarString(CConfig::getString("mR4Command"));
            break;
        case 5:
            setScaleRange(80);
            //            if(isAdaptSn) sendToRadarString(CConfig::getString("mR5Command"));
            break;
        case 6:
            setScaleRange(160);
            //            if(isAdaptSn) sendToRadarString(CConfig::getString("mR6Command"));
            break;
        case 7:
            setScaleRange(320);
            //            if(isAdaptSn) sendToRadarString(CConfig::getString("mR7Command"));
            break;
        default:
            break;
        }
    }
    isScaleChanged = true;
    short sdx = mZoomCenterx - rda_main.radCtX;
    short sdy = mZoomCentery - rda_main.radCtY;
    sdx =(sdx*rda_main.mScale/oldScale);
    sdy =(sdy*rda_main.mScale/oldScale);
    mZoomCenterx = scrCtX+sdx-dx;
    mZoomCentery = scrCtY+sdy-dy;
}


void Mainwindow::setCodeType(short index)// chuyen ma
{
    unsigned char bytes[8];
    bytes[0] = 1;
    bytes[1] = 0xab;

    //printf("\n code:%d",index);
    switch (index)
    {
    case 0://M32
        bytes[2] = 2;
        bytes[3] = 0;
        break;
    case 1://M64
        bytes[2] = 2;
        bytes[3] = 1;
        break;
    case 2://M128
        bytes[2] = 2;
        bytes[3] = 2;
        break;
    case 3://M255
        bytes[2] = 2;
        bytes[3] = 3;
        break;
    case 4://M32x2
        bytes[2] = 2;
        bytes[3] = 4;
        break;
    case 5://M64x2
        bytes[2] = 2;
        bytes[3] = 5;
        break;
    case 6://M128x2
        bytes[2] = 2;
        bytes[3] = 6;
        break;
    case 7://baker
        bytes[2] = 1;
        bytes[3] = 1;
        break;
    case 8://single pulse
        bytes[2] = 0;
        bytes[3] = 1;

        break;
    default:
        bytes[2] = 0;
        bytes[3] = 0;
        break;
    }
    bytes[4] = 0;
    bytes[5] = 0;
    bytes[6] = 0;
    bytes[7] = 0;//new_com.bytes[0]+new_com.bytes[1]+new_com.bytes[2]+new_com.bytes[3]+new_com.bytes[4]+new_com.bytes[5]+new_com.bytes[6];
    sendToRadar(&bytes[0]);

}
//void MainWindow::on_toolButton_4_toggled(bool checked)
//{
//    if(checked)
//    {
//        this->on_actionTx_On_triggered();
//    }
//    else
//    {
//        this->on_actionTx_Off_triggered();
//    }

//}



void Mainwindow::on_horizontalSlider_gain_valueChanged(int value)
{
    rda_main.mRadarData->kgain = 8-float(value)/(ui->horizontalSlider_gain->maximum())*8;
    ui->label_gain->setText("Gain:"+QString::number(-rda_main.mRadarData->kgain,'f',2));
    //printf("rda_main.mRadarData->kgain %f \n",rda_main.mRadarData->kgain);
}

void Mainwindow::on_horizontalSlider_rain_valueChanged(int value)
{
    rda_main.mRadarData->krain = (float)value/(ui->horizontalSlider_rain->maximum());
    ui->label_rain->setText("Rain:" + QString::number(rda_main.mRadarData->krain,'f',2));
}

void Mainwindow::on_horizontalSlider_sea_valueChanged(int value)
{
    rda_main.mRadarData->ksea = (float)value/(ui->horizontalSlider_sea->maximum());
    ui->label_sea->setText("Sea:" + QString::number(rda_main.mRadarData->ksea,'f',2));
}


/*
        void MainWindow::on_pushButton_loadAis_clicked()
        {
            QString filename = QFileDialog::getOpenFileName(this,    QString::fromUtf8("M? file "), NULL, tr("ISM file (*.txt)"));
            if(!filename.size())return;
            QFile gpsfile( filename);
            if (!gpsfile.open(QIODevice::ReadOnly | QIODevice::Text))
            {
                return ;
            }
            QTextStream in(&gpsfile);
            QString line ;int k=0;
            line = in.readLine();

            while(!in.atEnd()) {
                //printf((char*)line.data());
                QStringList  list = line.split(",");

                if (list[0] == "$GPRMC")
                {

                    float mlat = (*(list.begin()+3)).toFloat()/100.0f +0.0097;
                    float mlong = (*(list.begin()+5)).toFloat()/100.0f + 0.355;
                    arpa_data.addAIS(list[0].toStdString(),mlat,mlong,0,0);

                }line = in.readLine();
                k=list.size();
                //printf("size:%d",arpa_data.ais_track_list[0].id.data());
            }

        }

        */


//void Mainwindow::on_toolButton_exit_clicked()
//{

//}

//void Mainwindow::on_toolButton_setting_clicked()
//{
//    this->on_actionSetting_triggered();
//}

/*
        void Mainwindow::on_toolButton_tx_toggled(bool checked)
        {

        //    if(checked)

        //    {   //0xaa,0xab,0x00,0x01,0x00,0x00,0x00
        //        unsigned char        bytes[8] = {0xaa,0xab,0x02,0x01,0x00,0x00,0x00};
        //        udpSendSocket->writeDatagram((char*)&bytes[0],8,QHostAddress("192.168.0.44"),2572);
        //        bytes[2] = 0x00;//{0xaa,0xab,0x00,0x01,0x00,0x00,0x00};
        //        Sleep(100);
        //        udpSendSocket->writeDatagram((char*)&bytes[0],8,QHostAddress("192.168.0.44"),2572);
        //        //ui->toolButton_tx->setChecked(false);
        //    }
        //    else
        //    {

        //        unsigned char        bytes[8] = {0xaa,0xab,0x02,0x00,0x00,0x00,0x00};
        //        udpSendSocket->writeDatagram((char*)&bytes[0],8,QHostAddress("192.168.0.44"),2572);
        //        bytes[2] = 0x00;// = {0xaa,0xab,0x00,0x01,0x00,0x00,0x00};
        //        Sleep(100);
        //        udpSendSocket->writeDatagram((char*)&bytes[0],8,QHostAddress("192.168.0.44"),2572);
        //        //ui->toolButton_tx->setChecked(true);
        //    }

        }
        */


//void Mainwindow::on_toolButton_xl_nguong_toggled(bool checked)
//{
//    rda_main.mRadarData->setAutorgs(checked);
//    if(checked)
//    {
//        //        ui->horizontalSlider_gain->setVisible(false);
//        //        ui->horizontalSlider_rain->setVisible(false);
//        //        ui->horizontalSlider_sea->setVisible(false);
//    }
//    else
//    {
//        //        ui->horizontalSlider_gain->setVisible(true);
//        //        ui->horizontalSlider_rain->setVisible(true);
//        //        ui->horizontalSlider_sea->setVisible(true);
//    }
//}

void Mainwindow::on_toolButton_replay_toggled(bool checked)
{
    this->on_actionPlayPause_toggled(checked);
}



void Mainwindow::on_toolButton_record_toggled(bool checked)
{

}

void Mainwindow::on_toolButton_open_record_clicked()
{
    this->on_actionOpen_rec_file_triggered();
}


void Mainwindow::gotoCenter()
{
    dx = 0;
    dy = 0;
    rda_main.radCtX = scrCtX-dx;
    rda_main.radCtY = scrCtY-dy;
    isMapOutdated = true;
}
void Mainwindow::on_toolButton_centerView_clicked()
{
    gotoCenter();
    //    isScreenUp2Date = false;
}

//void Mainwindow::on_comboBox_currentIndexChanged(int index)
//{
//    switch (index)
//    {
//    case 0:
//        rda_main.mRadarData->dataOver = m_only;
//        break;
//    case 1:
//        rda_main.mRadarData->dataOver = s_m_150;
//        break;
//    case 2:
//        rda_main.mRadarData->dataOver = s_m_200;
//        break;
//    case 3:
//        rda_main.mRadarData->dataOver = max_s_m_150;
//        break;
//    case 4:
//        rda_main.mRadarData->dataOver = max_s_m_200;
//        break;
//    default:
//        break;
//    }

//}

void Mainwindow::on_comboBox_img_mode_currentIndexChanged(int index)
{
    rda_main.mRadarData->setImgMode(imgDrawMode(index));
}


void Mainwindow::on_toolButton_send_command_clicked()
{

    unsigned char        bytes[8];
    hex2bin(ui->lineEdit_byte_1->text().toStdString().data(),&bytes[0]);
    hex2bin(ui->lineEdit_byte_2->text().toStdString().data(),&bytes[1]);
    hex2bin(ui->lineEdit_byte_3->text().toStdString().data(),&bytes[2]);
    hex2bin(ui->lineEdit_byte_4->text().toStdString().data(),&bytes[3]);
    hex2bin(ui->lineEdit_byte_5->text().toStdString().data(),&bytes[4]);
    hex2bin(ui->lineEdit_byte_6->text().toStdString().data(),&bytes[5]);
    hex2bin(ui->lineEdit_byte_7->text().toStdString().data(),&bytes[6]);
    bytes[7] = bytes[0]+bytes[1]+bytes[2]+bytes[3]+bytes[4]+bytes[5]+bytes[6];
    ui->lineEdit_byte_8->setText(QString::number(bytes[7]));
    //hex2bin(ui->lineEdit_byte_8->text().toStdString().data(),&bytes[7]);
    sendToRadar((unsigned char*)&bytes[0]);
    //udpSendSocket->writeDatagram((char*)&bytes[0],8,QHostAddress("192.168.0.44"),2572);

}


void Mainwindow::on_toolButton_zoom_in_clicked()
{
    if(mRangeIndex >0) mRangeIndex-=1;
    CConfig::setValue("mRangeLevel",mRangeIndex);

    UpdateScale();
    SendScaleCommand();

    isMapOutdated = true;
}
void Mainwindow::SendScaleCommand()
{
    if(!ui->toolButton_auto_adapt->isChecked())return;
    QString commandString;
    switch(mRangeIndex)
    {
    case 0://2nm
        commandString = (CConfig::getString("mR1Command"));
        break;
    case 1:
        commandString = (CConfig::getString("mR2Command"));
        break;
    case 2:
        commandString = (CConfig::getString("mR3Command"));
        break;
    case 3:
        commandString = (CConfig::getString("mR4Command"));
        break;
    case 4:
        commandString = (CConfig::getString("mR5Command"));
        break;
    case 5:
        commandString = (CConfig::getString("mR6Command"));
        break;
    case 6:
        commandString = (CConfig::getString("mR7Command"));
        break;
    case 7:
        commandString = (CConfig::getString("mR8Command"));
        break;
    case 8:
        commandString = (CConfig::getString("mR9Command"));
        break;
    default:
        break;
    }
    QStringList comm = commandString.split('*');
    if(rda_main.processing->mRadMode<comm.size())
        commandString = comm.at(rda_main.processing->mRadMode);
    sendToRadarString(commandString);

}
void Mainwindow::on_toolButton_zoom_out_clicked()
{
    if(mRangeIndex <7) mRangeIndex+=1;
    CConfig::setValue("mRangeIndex",mRangeIndex);
    UpdateScale();
    isMapOutdated = true;
}

//void Mainwindow::on_toolButton_reset_clicked()
//{
//    rda_main.mRadarData->resetSled();
//}

//void Mainwindow::on_toolButton_send_command_2_clicked()
//{
//    unsigned char        bytes[8] = {0xaa,0xab,0x02,0x02,0x0a,0,0,0};
//    udpSendSocket->writeDatagram((char*)&bytes[0],8,QHostAddress("192.168.0.44"),2572);
////    bytes[0] = 0xaa;
////    bytes[1] = 0xab;
////    bytes[2] = 0x02;
////    bytes[3] = 0x02;
////    bytes[4] = 0x0a;
////    bytes[5] = 0x00;
////    bytes[6] = 0x00;
////    bytes[7] = 0x00;

//}



//void Mainwindow::on_dial_valueChanged(int value)
//{
//    float heading = value/100.0f;
//    ui->textEdit_heading->setText(QString::number(heading));

//}

//void Mainwindow::on_toolButton_set_heading_clicked()
//{

//    mHeadingT = ui->textEdit_heading->text().toFloat();
//    mHeadingT2 = ui->textEdit_heading_2->text().toFloat();
//    CConfig::setValue("mHeadingT",mHeadingT);
//    CConfig::setValue("mHeadingT2",mHeadingT2);
//    rda_main.mRadarData->setAziOffset(mHeadingT);

//}

//void Mainwindow::on_toolButton_gps_update_clicked()
//{

//    SetGPS(ui->text_latInput_2->text().toFloat(),ui->text_longInput_2->text().toFloat());

//}




//void Mainwindow::on_toolButton_centerZoom_clicked()
//{
//    rda_main.mRadarData->updateZoomRect(mousePointerX - rda_main.radCtX,mousePointerY - rda_main.radCtY);
//}

void Mainwindow::on_toolButton_xl_dopler_clicked()
{

}

//void Mainwindow::on_toolButton_xl_dopler_toggled(bool checked)
//{
//    rda_main.mRadarData->gat_mua_dopler = checked;
//}


//void Mainwindow::on_toolButton_xl_nguong_3_toggled(bool checked)
//{
//    rda_main.mRadarData->noise_nornalize = checked;
//}

//void Mainwindow::on_groupBox_3_currentChanged(int index)
//{

//}

void Mainwindow::on_toolButton_xl_dopler_2_toggled(bool checked)
{
    rda_main.mRadarData->bo_bang_0 = checked;
}



//void Mainwindow::on_toolButton_reset_3_clicked()
//{
//    rda_main.mRadarData->resetTrack();
//    //    for(short i = 0;i<targetDisplayList.size();i++)
//    //    {
//    //        targetDisplayList.at(i)->deleteLater();
//    //    }
//    //    targetDisplayList.clear();
//}

//void Mainwindow::on_toolButton_reset_2_clicked()
//{
//    rda_main.mRadarData->resetSled();
//}
//void Mainwindow::on_toolButton_vet_clicked(bool checked)
//{
//    rda_main.mRadarData->isSled = checked;
//}

void Mainwindow::on_label_status_warning_clicked()
{
    /*if(warningList.size())warningList.removeAt(warningList.size()-1);
    if(warningList.size())
    {
        ui->label_status_warning->setText(warningList.at(warningList.size()-1));
    }
    else
    {
        ui->label_status_warning->setText(QString::fromUtf8("Không cảnh báo"));
        ui->label_status_warning->setStyleSheet("background-color: rgb(20, 40, 60,255);");
    }*/
}

//void Mainwindow::on_toolButton_delete_target_clicked()
//{
//    /*if(targetList.at(selected_target_index)->isLost)
//            {
//                targetList.at(selected_target_index)->hide();
//            }

//            else*/
//    //    rda_main.mRadarData->mTrackList.at(targetDisplayList.at(selected_target_index)->trackId).isManual = false;
//}
bool Mainwindow::CheckTxCondition(bool isPopupMsg)
{
    if(CConfig::mStat.getAge21()>5000)
    {
        if(isPopupMsg){QMessageBox msgBox;
            msgBox.setText(QString::fromUtf8("Mất kết nối đến Máy 2-1!"));
            msgBox.exec();
            //        ui->toolButton_tx_off->setChecked(true);
        }
        return false;
    }
    if(CConfig::mStat.getAgeBH()>20000)
    {
        if(isPopupMsg){
            QMessageBox msgBox;
            msgBox.setText(QString::fromUtf8("Mất kết nối đến mô đun báo hỏng, cấm phát!"));
            msgBox.exec();
            //            ui->toolButton_tx_off->setChecked(true);
        }
        return false;
    }
    if(!CConfig::mStat.isTxSwModeOk)
    {
        if(isPopupMsg){
            QMessageBox msgBox;
            msgBox.setText(QString::fromUtf8("Vị trí chuyển mạch ăng ten không đúng, cấm phát!"));
            msgBox.exec();
            //            ui->toolButton_tx_off->setChecked(true);
        }
        return false;
    }

    if(CConfig::mStat.getAgeTempOk()>300000)
    {
        if(isPopupMsg){
            QMessageBox msgBox;
            msgBox.setText(QString::fromUtf8("Máy phát quá nhiệt trên 5 phút, cấm phát!"));
            msgBox.exec();
        }
        //        ui->toolButton_tx_off->setChecked(true);
        return false;
    }
    return true;
}
void Mainwindow::on_toolButton_tx_clicked()
{

#ifndef THEON
    //check Tx condition
    if(!ui->toolButton_tx_forced->isChecked())
    {
        if(!CheckTxCondition(true))return;
    }
#endif

    //restart cuda
    //system("taskkill /f /im cudaFFT.exe");
    SetTx(true);
    unsigned char command[]={0xaa,0x55,0x67,0x0c,
                             0x00,
                             0x01,
                             0x00,
                             0x00,
                             0x00,
                             0x00,
                             0x00,
                             0x00};
    rda_main.processing->sendCommand(command,12,false);

}
void Mainwindow::closeEvent (QCloseEvent *event)
{

}

void Mainwindow::on_toolButton_tx_off_clicked()
{
    //rda_main.processing->radTxOff();

    SetTx(false);
    unsigned char command[]={0xaa,0x55,0x67,0x0c,
                             0x00,
                             0x00,
                             0x00,
                             0x00,
                             0x00,
                             0x00,
                             0x00,
                             0x00};
    rda_main.processing->sendCommand(command,12,false);

}

//void Mainwindow::on_toolButton_filter2of3_clicked(bool checked)
//{
//    rda_main.mRadarData->filter2of3 = checked;
//}




//void Mainwindow::on_comboBox_radar_resolution_currentIndexChanged(int index)
//{

//}

//void Mainwindow::on_toolButton_create_zone_2_clicked(bool checked)
//{
//    //    if(checked)
//    //        gz2.isActive = false;
//}

void Mainwindow::on_toolButton_measuring_clicked()
{
    mMouseLastX = rda_main.radCtX;
    mMouseLastY = rda_main.radCtY;
}


/*void Mainwindow::on_comboBox_2_currentIndexChanged(int index)
{
    return;

}*/

void Mainwindow::on_toolButton_measuring_clicked(bool checked)
{
    ui->toolButton_grid->setChecked(true);
}

void Mainwindow::on_toolButton_export_data_clicked(bool checked)
{
    rda_main.mRadarData->data_export = checked;
}





//void Mainwindow::on_toolButton_auto_select_toggled(bool checked)
//{
//    setMouseMode(MouseAutoSelect,checked);
//    //    if(!checked)
//    //    {
//    //        this->setCursor(Qt::ArrowCursor);
//    //    }
//    //    else
//    //    {
//    //        this->setCursor(Qt::CrossCursor);
//    //    }
//}

void Mainwindow::on_toolButton_ais_reset_clicked()
{
    rda_main.processing->mAisVesselsList.clear();
}


void Mainwindow::on_toolButton_command_dopler_clicked()
{
    ui->lineEdit_byte_1->setText("05");
    ui->lineEdit_byte_2->setText("ab");
    ui->lineEdit_byte_3->setText("02");
    ui->lineEdit_byte_4->setText("00");
    ui->lineEdit_byte_5->setText("00");
    ui->lineEdit_byte_6->setText("00");
}

void Mainwindow::on_toolButton_command_period_clicked()
{
    ui->lineEdit_byte_1->setText("14");
    ui->lineEdit_byte_2->setText("ab");
    ui->lineEdit_byte_3->setText("ff");
    ui->lineEdit_byte_4->setText("00");
    ui->lineEdit_byte_5->setText("00");
    ui->lineEdit_byte_6->setText("00");
}

void Mainwindow::on_toolButton_noise_balance_clicked()
{
    ui->lineEdit_byte_1->setText("1a");
    ui->lineEdit_byte_2->setText("ab");
    ui->lineEdit_byte_3->setText("20");
    ui->lineEdit_byte_4->setText("01");
    ui->lineEdit_byte_5->setText("00");
    ui->lineEdit_byte_6->setText("00");
}

void Mainwindow::on_toolButton_limit_signal_clicked()
{
    ui->lineEdit_byte_1->setText("17");
    ui->lineEdit_byte_2->setText("ab");
    ui->lineEdit_byte_3->setText("64");
    ui->lineEdit_byte_4->setText("00");
    ui->lineEdit_byte_5->setText("00");
    ui->lineEdit_byte_6->setText("00");
}

void Mainwindow::on_toolButton_command_amplifier_clicked()
{
    ui->lineEdit_byte_1->setText("aa");
    ui->lineEdit_byte_2->setText("ab");
    ui->lineEdit_byte_3->setText("01");
    ui->lineEdit_byte_4->setText("01");
    ui->lineEdit_byte_5->setText("1f");
    ui->lineEdit_byte_6->setText("00");
}

void Mainwindow::on_toolButton_command_dttt_clicked()
{
    ui->lineEdit_byte_1->setText("01");
    ui->lineEdit_byte_2->setText("ab");
    ui->lineEdit_byte_3->setText("04");
    ui->lineEdit_byte_4->setText("00");
    ui->lineEdit_byte_5->setText("00");
    ui->lineEdit_byte_6->setText("00");
}

void Mainwindow::on_toolButton_command_res_clicked()
{
    ui->lineEdit_byte_1->setText("08");
    ui->lineEdit_byte_2->setText("ab");
    ui->lineEdit_byte_3->setText("00");
    ui->lineEdit_byte_4->setText("00");
    ui->lineEdit_byte_5->setText("00");
    ui->lineEdit_byte_6->setText("00");
}

void Mainwindow::on_toolButton_command_antenna_rot_clicked()
{
    ui->lineEdit_byte_1->setText("aa");
    ui->lineEdit_byte_2->setText("ab");
    ui->lineEdit_byte_3->setText("03");
    ui->lineEdit_byte_4->setText("00");
    ui->lineEdit_byte_5->setText("00");
    ui->lineEdit_byte_6->setText("00");
}

void Mainwindow::on_comboBox_3_currentIndexChanged(int index)
{
    osmap->SetType(index);
    isMapOutdated = true;

}

void Mainwindow::on_horizontalSlider_map_brightness_valueChanged(int value)
{
    mMapOpacity = value/50.0;
    CConfig::setValue("mMapOpacity",mMapOpacity);
    isMapOutdated = true;
}



void Mainwindow::on_toolButton_selfRotation_toggled(bool checked)
{
    if(checked)
    {
        double rate = ui->lineEdit_selfRotationRate->text().toDouble();
        rda_main.mRadarData->SelfRotationOn(rate);
    }
    else
        rda_main.mRadarData->SelfRotationOff();
}

//void Mainwindow::on_toolButton_scope_toggled(bool checked)
//{
//    setMouseMode(MouseScope,checked);
//}

//void Mainwindow::on_toolButton_manual_track_toggled(bool checked)
//{
//    setMouseMode(MouseAddingTrack,checked);
//}
void Mainwindow::setMouseMode(mouseMode mode,bool isOn)
{
    if(isOn)
    {
        mouse_mode = static_cast<mouseMode>(mouse_mode|mode) ;
        //printf("\ntrue:%d",mouse_mode|mode);
    }
    else
    {
        mouse_mode = static_cast<mouseMode>(mouse_mode-(mode&mouse_mode));
        //printf("\nfalse:%d",mouse_mode-(mode&mouse_mode));
    }

}
void Mainwindow::on_toolButton_measuring_toggled(bool checked)
{
    setMouseMode(MouseMeasuring,checked);
}

void Mainwindow::on_toolButton_VRM_toggled(bool checked)
{
    setMouseMode(MouseVRM,checked);
}

void Mainwindow::on_toolButton_ELB_toggled(bool checked)
{
    setMouseMode(MouseELB,checked);
}

void Mainwindow::on_toolButton_record_clicked()
{

}

//void Mainwindow::on_toolButton_sharp_eye_toggled(bool checked)
//{
//    rda_main.mRadarData->setIsSharpEye(checked);
//}

void Mainwindow::on_toolButton_help_clicked()
{

    DialogDocumentation *dlg=new DialogDocumentation();
    dlg->setModal(false);
    dlg->showNormal();
    printf("\nNew windows");

}

void Mainwindow::on_toolButton_setRangeUnit_clicked()
{
    switch(mDistanceUnit)
    {
    case 0:
        this->setDistanceUnit(1);
        break;
    case 1:
        this->setDistanceUnit(0);
        break;
    default:
        break;
    }


}





//void Mainwindow::on_toolButton_gps_update_auto_clicked()
//{
//    UpdateGpsData();
//}

void Mainwindow::UpdateGpsData()
{
    if(CConfig::mStat.getAgeGps()<3000)
    {
        if(CConfig::mLat)ui->label_gps_lat->setText(demicalDegToDegMin(CConfig::mLat)+"'N");
        else ui->label_gps_lat->setText(demicalDegToDegMin(CConfig::mLat)+"'S");
        if(CConfig::mLon)ui->label_gps_lon->setText(demicalDegToDegMin(CConfig::mLon)+"'E");
        else ui->label_gps_lon->setText(demicalDegToDegMin(CConfig::mLon)+"'W");
        rda_main.setCenterLonLat(CConfig::mLon,CConfig::mLat);
    }
    else
    {
        rda_main.processing->forwardOldGps();//
    }

}
void Mainwindow::on_toolButton_set_zoom_ar_size_clicked()
{
    mZoomSizeRg = ui->textEdit_size_ar_r->text().toDouble();
    mZoomSizeAz = ui->textEdit_size_ar_a->text().toDouble();
    CConfig::setValue("mZoomSizeRg",mZoomSizeRg);
    CConfig::setValue("mZoomSizeAz",mZoomSizeAz);
}

void Mainwindow::on_toolButton_advanced_control_clicked()
{
    if(ui->lineEdit_password->text()=="ccndt3108")
    {
        ui->groupBox_control->setHidden(false);
    }
}



//void Mainwindow::on_toolButton_auto_freq_toggled(bool checked)
//{
//    if(checked) autoSwitchFreq();
//}

void Mainwindow::on_toolButton_set_default_clicked()
{
    CConfig::SaveAndSetConfigAsDefault();
}



//void Mainwindow::on_toolButton_heading_update_clicked()
//{
//    /*if(rda_main.processing->isHeadingAvaible)
//    {
//        mHeadingT = rda_main.processing->getHeading()+CConfig::getDouble("mHeadingT3");
//        if(mHeadingT>=360)mHeadingT-=360;
//        ui->label_compass_value->setText(QString::number(rda_main.processing->getHeading(),'f',1));
//        ui->textEdit_heading->setText(QString::number(mHeadingT));
//    }
//    else
//    {
//        warningList.push_back(QString::fromUtf8("Chưa kết nối la bàn"));
//    }*/
//}

void Mainwindow::on_toolButton_sled_clicked()
{

}

void Mainwindow::on_toolButton_sled_toggled(bool checked)
{
    rda_main.mRadarData->SetSled(checked);
    CConfig::setValue("isShowSled",int(checked));
}

void Mainwindow::on_toolButton_sled_reset_clicked()
{
    rda_main.mRadarData->resetSled();
}



//void Mainwindow::on_toolButton_dobupsong_toggled(bool checked)
//{
//    rda_main.mRadarData->is_do_bup_song = checked;
//    if(checked)
//    {
//        rda_main.mRadarData->setTb_tap_k(ui->textEdit_dobupsongk->text().toDouble());

//    }
//}


void Mainwindow::on_toolButton_set_commands_clicked()
{
    DialogConfig *dlg= new DialogConfig();
    dlg->setModal(false);
    dlg->showNormal();

}

void Mainwindow::on_toolButton_command_log_toggled(bool checked)
{
    if(checked)
    {
        //cmLog->show();
    }
    else
    {
        //cmLog->hide();
    }
}

void Mainwindow::on_toolButton_exit_2_clicked()
{
    mstatWin = new StatusWindow(rda_main.processing);

    mstatWin->show();
}

void Mainwindow::on_toolButton_selfRotation_2_toggled(bool checked)
{
    //rda_main.mRadarData->isEncoderAzi  = checked;
    if(checked)
    {
        double rate = ui->lineEdit_selfRotationRate->text().toDouble();
        //        rate = rate/MAX_AZIR;
        rda_main.mRadarData->SelfRotationOn(rate);
    }
    else
        rda_main.mRadarData->SelfRotationOff();
}

//void Mainwindow::on_toolButton_selfRotation_clicked()
//{

//}



void Mainwindow::on_toolButton_tx_2_clicked(bool checked)
{

}

void Mainwindow::on_toolButton_menu_clicked()
{
    if(ui->tabWidget_menu->isHidden())
    {
        ui->tabWidget_menu->setHidden(false);
        ui->tabWidget_menu->setGeometry(550,180,ui->tabWidget_menu->width(),ui->tabWidget_menu->height());
        ui->tabWidget_menu->setCurrentIndex(0);
    }
    else
    {
        ui->tabWidget_menu->setGeometry(200,-800,ui->tabWidget_menu->width(),ui->tabWidget_menu->height());
        ui->tabWidget_menu->setHidden(true);
    }
}

void Mainwindow::on_toolButton_iad_clicked()
{
    if(ui->tabWidget_iad->isHidden())
    {
        ui->tabWidget_iad->setHidden(false);
        ui->tabWidget_iad->setGeometry(5,180,ui->tabWidget_iad->width(),ui->tabWidget_iad->height());
        ui->tabWidget_iad->setCurrentIndex(0);
    }
    else
    {
        ui->tabWidget_iad->setGeometry(200,-800,ui->tabWidget_iad->width(),ui->tabWidget_iad->height());
        ui->tabWidget_iad->setHidden(true);
    }
}

void Mainwindow::on_tabWidget_menu_tabBarClicked(int index)
{
    if(ui->tabWidget_menu->count()-1==index)
    {
        ui->tabWidget_menu->setHidden(true);
    }

}

void Mainwindow::on_tabWidget_iad_tabBarClicked(int index)
{
    if(ui->tabWidget_iad->count()-1==index)
    {
        ui->tabWidget_iad->setHidden(true);
    }
}

//void Mainwindow::on_toolButton_xl_nguong_3_clicked()
//{

//}

void Mainwindow::on_toolButton_head_up_toggled(bool checked)
{
    isHeadUp = checked;
    gotoCenter();
}

//void Mainwindow::on_toolButton_delete_target_2_clicked()
//{
//    QStringList list = ui->textEdit_systemCommand->toPlainText().split(';');
//    for(int i=0;i<list.size();i++)
//    {
//        QByteArray ba=list.at(i).toLatin1();
//        sendToRadarHS(ba.data());
//    }
//}



void Mainwindow::on_toolButton_dk_13_toggled(bool checked)
{
    if(checked)
    {

    }
}

void Mainwindow::on_toolButton_dk_15_toggled(bool checked)
{
    return;

}

void Mainwindow::on_toolButton_dk_11_toggled(bool checked)
{
    return;

}

void Mainwindow::on_toolButton_dk_16_toggled(bool checked)
{
    return;
    if(checked)
    {
        commandMay22[7]=0x01;
        rda_main.processing->sendCommand(commandMay22,12,false);
    }
}

void Mainwindow::on_toolButton_dk_17_toggled(bool checked)
{
    return;
    if(checked)
    {
        commandMay22[7]=0x02;
        rda_main.processing->sendCommand(commandMay22,12,false);
    }
}

void Mainwindow::on_toolButton_grid_toggled(bool checked)
{
    toolButton_grid_checked = checked;
}

void Mainwindow::on_toolButton_dk_4_clicked()
{

    commandMay22[6]=0x06;
    rda_main.processing->sendCommand(commandMay22,12,false);
}

void Mainwindow::on_toolButton_dk_3_clicked()
{
    commandMay22[6]=0x00;
    rda_main.processing->sendCommand(commandMay22,12,false);
}

void Mainwindow::on_toolButton_dk_5_clicked()
{
    commandMay22[6]=0x05;
    rda_main.processing->sendCommand(commandMay22,12,false);
}

void Mainwindow::on_toolButton_dk_6_clicked()
{
    commandMay22[6]=0x04;
    rda_main.processing->sendCommand(commandMay22,12,false);
}

void Mainwindow::on_toolButton_dk_7_clicked()
{
    commandMay22[6]=0x03;
    rda_main.processing->sendCommand(commandMay22,12,false);
}

void Mainwindow::on_toolButton_dk_8_clicked()
{
    commandMay22[6]=0x02;
    rda_main.processing->sendCommand(commandMay22,12,false);
}

void Mainwindow::on_toolButton_dk_9_clicked()
{
    commandMay22[6]=0x01;
    rda_main.processing->sendCommand(commandMay22,12,false);
}

void Mainwindow::on_toolButton_dk_2_clicked()
{
    commandMay22[5]=0x00;
    rda_main.processing->sendCommand(commandMay22,12,false);
}

void Mainwindow::on_toolButton_dk_13_clicked()
{
    commandMay22[5]=0x01;
    rda_main.processing->sendCommand(commandMay22,12,false);
}

void Mainwindow::on_toolButton_dk_12_clicked()
{
    commandMay22[4]=0x01;
    rda_main.processing->sendCommand(commandMay22,12,false);
}

//void Mainwindow::on_toolButton_dk_10_clicked()
//{
//    commandMay22[8]=0x01;
//    rda_main.processing->sendCommand(commandMay22,12,false);
//}

//void Mainwindow::on_toolButton_dk_14_clicked()
//{
//    commandMay22[8]=0x02;
//    rda_main.processing->sendCommand(commandMay22,12,false);
//}

//void Mainwindow::on_toolButton_dk_15_clicked()
//{
//    commandMay22[8]=0x00;
//    rda_main.processing->sendCommand(commandMay22,12,false);
//}

void Mainwindow::on_toolButton_sled_time25_clicked()
{
    rda_main.mRadarData->mSledValue = 180;
}

void Mainwindow::on_toolButton_sled_time8_clicked()
{
    rda_main.mRadarData->mSledValue = 50;
}

void Mainwindow::on_toolButton_sled_time3_clicked()
{
    rda_main.mRadarData->mSledValue = 10;
}

//void Mainwindow::on_toolButton_sled_reset_2_clicked(bool checked)
//{
//    mShowobjects = checked;
//}

//void Mainwindow::on_toolButton_sled_reset_3_clicked(bool checked)
//{
//    mShowLines = checked;
//}

//void Mainwindow::on_toolButton_sled_reset_4_clicked(bool checked)
//{
//    mShowTracks = checked;
//}

//void Mainwindow::on_toolButton_sled_reset_3_toggled(bool checked)
//{

//}



void Mainwindow::on_on_toolButton_xl_nguong_3_clicked(bool checked)
{
    rda_main.mRadarData->noise_nornalize = checked;
}

void Mainwindow::on_toolButton_xl_nguong_4_clicked(bool checked)
{
    rda_main.mRadarData->setAutorgs(checked);
}

void Mainwindow::on_toolButton_sled_clicked(bool checked)
{
    rda_main.mRadarData->SetSled(checked);
}

void Mainwindow::on_toolButton_xl_dopler_clicked(bool checked)
{
    rda_main.mRadarData->gat_mua_va_dia_vat = checked;
    CConfig::setValue("gat_mua_dopler",QString::number(int(checked)));
}

void Mainwindow::on_toolButton_setRangeUnit_toggled(bool checked)
{

}

//void Mainwindow::on_toolButton_xl_dopler_3_clicked(bool checked)
//{
//    //rda_main.mRadarData->isTrueHeadingFromRadar = checked;
////    CConfig::setValue("isTrueHeadingFromRadar",checked);
//}

void Mainwindow::on_toolButton_head_up_clicked(bool checked)
{

}

void Mainwindow::on_toolButton_dk_1_clicked()
{
    commandMay22[4]=0x00;
    rda_main.processing->sendCommand(commandMay22,12,false);
}

void Mainwindow::on_toolButton_chi_thi_mt_clicked(bool checked)
{

}

void Mainwindow::on_bt_rg_1_toggled(bool checked)
{
    if(checked)
    {
        mRangeIndex=0;
        CConfig::setValue("mRangeLevel",mRangeIndex);
        UpdateScale();
        SendScaleCommand();
        isMapOutdated = true;
    }
}

void Mainwindow::on_bt_rg_2_toggled(bool checked)
{
    if(checked)
    {
        mRangeIndex=1;
        CConfig::setValue("mRangeLevel",mRangeIndex);
        UpdateScale();
        SendScaleCommand();
        isMapOutdated = true;
    }
}

void Mainwindow::on_bt_rg_3_toggled(bool checked)
{
    if(checked)
    {
        mRangeIndex=2;
        CConfig::setValue("mRangeLevel",mRangeIndex);
        UpdateScale();
        SendScaleCommand();
        isMapOutdated = true;
    }
}

void Mainwindow::on_bt_rg_4_toggled(bool checked)
{
    if(checked)
    {
        mRangeIndex=3;
        CConfig::setValue("mRangeLevel",mRangeIndex);
        UpdateScale();
        SendScaleCommand();
        isMapOutdated = true;
    }
}



void Mainwindow::on_bt_rg_6_toggled(bool checked)
{

}

void Mainwindow::on_bt_rg_8_toggled(bool checked)
{
    if(checked)
    {
        mRangeIndex=7;
        CConfig::setValue("mRangeLevel",mRangeIndex);
        UpdateScale();
        SendScaleCommand();
        isMapOutdated = true;
    }
}

void Mainwindow::on_bt_rg_7_toggled(bool checked)
{
    if(checked)
    {
        mRangeIndex=6;
        CConfig::setValue("mRangeLevel",mRangeIndex);
        UpdateScale();
        SendScaleCommand();
        isMapOutdated = true;
    }
}


void Mainwindow::on_toolButton_open_record_2_clicked()
{
    char command[100];
    QString filename = QFileDialog::getOpenFileName(this,    tr("Open signal file"), NULL, tr("HR raw record files (*.dat)"));
    system("taskkill /f /im cudaCv.exe");
    sprintf(command,"D:\\HR2D\\cudaFFT.exe %s",filename.toStdString().data());
    printf("\n%s", (const char*)&command);
    system(command);
    ui->label_record_file_name->setText(filename);
}

void Mainwindow::on_bt_rg_6_clicked()
{
    mRangeIndex=5;
    CConfig::setValue("mRangeLevel",mRangeIndex);
    UpdateScale();
    SendScaleCommand();
    isMapOutdated = true;
}

void Mainwindow::on_bt_rg_7_clicked()
{
    mRangeIndex=6;
    CConfig::setValue("mRangeLevel",mRangeIndex);
    UpdateScale();
    SendScaleCommand();
    isMapOutdated = true;
}

void Mainwindow::on_bt_rg_8_clicked()
{
    mRangeIndex=7;
    CConfig::setValue("mRangeLevel",mRangeIndex);
    UpdateScale();
    SendScaleCommand();
    isMapOutdated = true;
}

void Mainwindow::on_bt_rg_1_clicked()
{
    mRangeIndex=0;
    CConfig::setValue("mRangeLevel",mRangeIndex);
    UpdateScale();
    SendScaleCommand();
    isMapOutdated = true;
}

void Mainwindow::on_bt_rg_2_clicked()
{
    mRangeIndex=1;
    CConfig::setValue("mRangeLevel",mRangeIndex);
    UpdateScale();
    SendScaleCommand();
    isMapOutdated = true;
}

void Mainwindow::on_bt_rg_3_clicked()
{
    mRangeIndex=2;
    CConfig::setValue("mRangeLevel",mRangeIndex);
    UpdateScale();
    SendScaleCommand();
    isMapOutdated = true;
}

void Mainwindow::on_bt_rg_4_clicked()
{
    mRangeIndex=3;
    CConfig::setValue("mRangeLevel",mRangeIndex);
    UpdateScale();
    SendScaleCommand();
    isMapOutdated = true;
}

void Mainwindow::on_bt_rg_5_clicked()
{
    mRangeIndex=4;
    CConfig::setValue("mRangeLevel",mRangeIndex);
    UpdateScale();
    SendScaleCommand();
    isMapOutdated = true;
}

void Mainwindow::on_toolButton_xl_nguong_5_clicked(bool checked)
{
    rda_main.mRadarData->noise_nornalize = checked;
}

void Mainwindow::on_toolButton_second_azi_clicked(bool checked)
{
    rda_main.mRadarData->giaQuayPhanCung=checked;
    if(checked)sendToRadarHS("1dab010070");
    else sendToRadarHS("1dab000070");
}

//void Mainwindow::on_on_toolButton_xl_nguong_3_toggled(bool checked)
//{

//}

void Mainwindow::on_toolButton_signal_type_1_clicked()
{
    rda_main.processing->mRadMode = ModeSimpleSignal;
    //    ui->bt_rg_0->setEnabled(false);
    ui->bt_rg_1->setHidden(false);
    ui->bt_rg_2->setHidden(false);
    ui->bt_rg_3->setHidden(false);
    ui->bt_rg_4->setHidden(false);
    ui->bt_rg_5->setHidden(false);
    ui->bt_rg_6->setHidden(true);
    ui->bt_rg_7->setHidden(true);
    ui->bt_rg_8->setHidden(true);
    if(ui->bt_rg_1->isChecked())on_bt_rg_1_clicked();
    else if(ui->bt_rg_2->isChecked())on_bt_rg_2_clicked();
    else if(ui->bt_rg_3->isChecked())on_bt_rg_3_clicked();
    else if(ui->bt_rg_4->isChecked())on_bt_rg_4_clicked();
    else {on_bt_rg_5_clicked();ui->bt_rg_5->setChecked(true);}

}

void Mainwindow::on_toolButton_signal_type_2_clicked()
{
    rda_main.processing->mRadMode = ModeComplexSignal;
    ui->bt_rg_1->setHidden(true); //if(ui->bt_rg_1->isChecked())on_bt_rg_1_clicked();
    ui->bt_rg_2->setHidden(false);
    ui->bt_rg_3->setHidden(false);
    ui->bt_rg_4->setHidden(false);
    ui->bt_rg_5->setHidden(false);
    ui->bt_rg_6->setHidden(false);
    ui->bt_rg_7->setHidden(false);
    ui->bt_rg_8->setHidden(false);
    if(ui->bt_rg_2->isChecked())on_bt_rg_2_clicked();
    else if(ui->bt_rg_3->isChecked())on_bt_rg_3_clicked();
    else if(ui->bt_rg_4->isChecked())on_bt_rg_4_clicked();
    else if(ui->bt_rg_5->isChecked())on_bt_rg_5_clicked();
    else if(ui->bt_rg_6->isChecked())on_bt_rg_6_clicked();
    else if(ui->bt_rg_7->isChecked())on_bt_rg_7_clicked();
    else if(ui->bt_rg_8->isChecked())on_bt_rg_8_clicked();
    else {on_bt_rg_2_clicked();ui->bt_rg_2->setChecked(true);}
}

void Mainwindow::on_toolButton_signal_type_3_clicked()
{
    rda_main.processing->mRadMode = ModeContinuos;
    ui->bt_rg_1->setHidden(true);
    ui->bt_rg_2->setHidden(false);
    ui->bt_rg_3->setHidden(false);
    ui->bt_rg_4->setHidden(false);
    ui->bt_rg_5->setHidden(false);
    ui->bt_rg_6->setHidden(false);
    ui->bt_rg_7->setHidden(false);
    ui->bt_rg_8->setHidden(false);
    if(ui->bt_rg_2->isChecked())on_bt_rg_2_clicked();
    else if(ui->bt_rg_3->isChecked())on_bt_rg_3_clicked();
    else if(ui->bt_rg_4->isChecked())on_bt_rg_4_clicked();
    else if(ui->bt_rg_5->isChecked())on_bt_rg_5_clicked();
    else if(ui->bt_rg_6->isChecked())on_bt_rg_6_clicked();
    else if(ui->bt_rg_7->isChecked())on_bt_rg_7_clicked();
    else if(ui->bt_rg_8->isChecked())on_bt_rg_8_clicked();
    else {on_bt_rg_2_clicked();ui->bt_rg_2->setChecked(true);}
}

void Mainwindow::on_toolButton_del_tget_table_clicked()
{
    mTargetMan.ClearTargetTable();
}

void Mainwindow::on_toolButton_manual_tune_clicked(bool checked)
{
    if(checked)
    {
        rda_main.mRadarData->isManualTune = true;
    }
    else
    {
        rda_main.mRadarData->isManualTune = false;
    }
}

//void Mainwindow::on_dial_valueChanged(int value)
//{
//    value+=180;
//    if(value>360)value-=360;
//    ui->label_dial_value_azi->setText(QString::fromUtf8("Hướng cđ:")+QString::number(value)+degreeSymbol);
//}

//void Mainwindow::on_horizontalSlider_valueChanged(int value)
//{
//    ui->label_dial_value_rg->setText(QString::fromUtf8("Tốc độ:")+QString::number(value/2.0,'f',1)+"Kn");
//}

void Mainwindow::on_toolButton_start_simulation_start_clicked(bool checked)
{
    if(checked)
    {
        simulator->play();//todo: stop receving signal
        rda_main.processing->isSimulationMode = true;
    }
}

void Mainwindow::on_toolButton_start_simulation_set_clicked(bool checked)
{

    simulator->setIsManeuver(checked);

}
void Mainwindow::updateSimTargetStatus()
{

    if(ui->checkBox->isChecked())
    {
        ui->doubleSpinBox_1->setEnabled(false);
        ui->doubleSpinBox_2->setEnabled(false);
        ui->doubleSpinBox_3->setEnabled(false);
        ui->doubleSpinBox_4->setEnabled(false);
        simulator->setTarget(0
                             ,ui->doubleSpinBox_1->value()
                             ,ui->doubleSpinBox_2->value()
                             ,ui->doubleSpinBox_3->value()
                             ,ui->doubleSpinBox_4->value());
    }
    else
    {
        simulator->targetList[0].setEnabled(false);
        ui->doubleSpinBox_1->setEnabled(true);
        ui->doubleSpinBox_2->setEnabled(true);
        ui->doubleSpinBox_3->setEnabled(true);
        ui->doubleSpinBox_4->setEnabled(true);
    }
    //target 1
    if(ui->checkBox_2->isChecked())
    {
        ui->doubleSpinBox_11->setEnabled(false);
        ui->doubleSpinBox_12->setEnabled(false);
        ui->doubleSpinBox_13->setEnabled(false);
        ui->doubleSpinBox_14->setEnabled(false);
        simulator->setTarget(1
                             ,ui->doubleSpinBox_11->value()
                             ,ui->doubleSpinBox_12->value()
                             ,ui->doubleSpinBox_13->value()
                             ,ui->doubleSpinBox_14->value());
    }
    else
    {
        simulator->targetList[1].setEnabled(false);
        ui->doubleSpinBox_11->setEnabled(true);
        ui->doubleSpinBox_12->setEnabled(true);
        ui->doubleSpinBox_13->setEnabled(true);
        ui->doubleSpinBox_14->setEnabled(true);
    }
    //target 2
    if(ui->checkBox_3->isChecked())
    {
        ui->doubleSpinBox_21->setEnabled(false);
        ui->doubleSpinBox_22->setEnabled(false);
        ui->doubleSpinBox_23->setEnabled(false);
        ui->doubleSpinBox_24->setEnabled(false);
        simulator->setTarget(2
                             ,ui->doubleSpinBox_21->value()
                             ,ui->doubleSpinBox_22->value()
                             ,ui->doubleSpinBox_23->value()
                             ,ui->doubleSpinBox_24->value());
    }
    else
    {
        simulator->targetList[2].setEnabled(false);
        ui->doubleSpinBox_21->setEnabled(true);
        ui->doubleSpinBox_22->setEnabled(true);
        ui->doubleSpinBox_23->setEnabled(true);
        ui->doubleSpinBox_24->setEnabled(true);
    }
    //target 3
    if(ui->checkBox_4->isChecked())
    {
        ui->doubleSpinBox_31->setEnabled(false);
        ui->doubleSpinBox_32->setEnabled(false);
        ui->doubleSpinBox_33->setEnabled(false);
        ui->doubleSpinBox_34->setEnabled(false);
        simulator->setTarget(3
                             ,ui->doubleSpinBox_31->value()
                             ,ui->doubleSpinBox_32->value()
                             ,ui->doubleSpinBox_33->value()
                             ,ui->doubleSpinBox_34->value());
    }
    else
    {
        simulator->targetList[3].setEnabled(false);
        ui->doubleSpinBox_31->setEnabled(true);
        ui->doubleSpinBox_32->setEnabled(true);
        ui->doubleSpinBox_33->setEnabled(true);
        ui->doubleSpinBox_34->setEnabled(true);
    }
    //target 4
    if(ui->checkBox_5->isChecked())
    {
        ui->doubleSpinBox_41->setEnabled(false);
        ui->doubleSpinBox_42->setEnabled(false);
        ui->doubleSpinBox_43->setEnabled(false);
        ui->doubleSpinBox_44->setEnabled(false);
        simulator->setTarget(4
                             ,ui->doubleSpinBox_41->value()
                             ,ui->doubleSpinBox_42->value()
                             ,ui->doubleSpinBox_43->value()
                             ,ui->doubleSpinBox_44->value());
    }
    else
    {
        simulator->targetList[4].setEnabled(false);
        ui->doubleSpinBox_41->setEnabled(true);
        ui->doubleSpinBox_42->setEnabled(true);
        ui->doubleSpinBox_43->setEnabled(true);
        ui->doubleSpinBox_44->setEnabled(true);
    }
    //target 5
    if(ui->checkBox_6->isChecked())
    {
        ui->doubleSpinBox_51->setEnabled(false);
        ui->doubleSpinBox_52->setEnabled(false);
        ui->doubleSpinBox_53->setEnabled(false);
        ui->doubleSpinBox_54->setEnabled(false);
        simulator->setTarget(5
                             ,ui->doubleSpinBox_51->value()
                             ,ui->doubleSpinBox_52->value()
                             ,ui->doubleSpinBox_53->value()
                             ,ui->doubleSpinBox_54->value());
    }
    else
    {
        simulator->targetList[5].setEnabled(false);
        ui->doubleSpinBox_51->setEnabled(true);
        ui->doubleSpinBox_52->setEnabled(true);
        ui->doubleSpinBox_53->setEnabled(true);
        ui->doubleSpinBox_54->setEnabled(true);
    }
    //target 6
    if(ui->checkBox_7->isChecked())
    {
        ui->doubleSpinBox_61->setEnabled(false);
        ui->doubleSpinBox_62->setEnabled(false);
        ui->doubleSpinBox_63->setEnabled(false);
        ui->doubleSpinBox_64->setEnabled(false);
        simulator->setTarget(6
                             ,ui->doubleSpinBox_61->value()
                             ,ui->doubleSpinBox_62->value()
                             ,ui->doubleSpinBox_63->value()
                             ,ui->doubleSpinBox_64->value());
    }
    else
    {
        simulator->targetList[6].setEnabled(false);
        ui->doubleSpinBox_61->setEnabled(true);
        ui->doubleSpinBox_62->setEnabled(true);
        ui->doubleSpinBox_63->setEnabled(true);
        ui->doubleSpinBox_64->setEnabled(true);
    }
    //target 7
    if(ui->checkBox_8->isChecked())
    {
        ui->doubleSpinBox_71->setEnabled(false);
        ui->doubleSpinBox_72->setEnabled(false);
        ui->doubleSpinBox_73->setEnabled(false);
        ui->doubleSpinBox_74->setEnabled(false);
        simulator->setTarget(7
                             ,ui->doubleSpinBox_71->value()
                             ,ui->doubleSpinBox_72->value()
                             ,ui->doubleSpinBox_73->value()
                             ,ui->doubleSpinBox_74->value());
    }
    else
    {
        simulator->targetList[7].setEnabled(false);
        ui->doubleSpinBox_71->setEnabled(true);
        ui->doubleSpinBox_72->setEnabled(true);
        ui->doubleSpinBox_73->setEnabled(true);
        ui->doubleSpinBox_74->setEnabled(true);
    }
}
void Mainwindow::on_checkBox_stateChanged(int arg1)
{
    arg1=arg1;
    updateSimTargetStatus();
}

void Mainwindow::on_checkBox_2_stateChanged(int arg1)
{
    arg1=arg1;
    updateSimTargetStatus();
}

void Mainwindow::on_checkBox_3_stateChanged(int arg1)
{
    arg1=arg1;
    updateSimTargetStatus();
}

void Mainwindow::on_checkBox_4_stateChanged(int arg1)
{
    arg1=arg1;
    updateSimTargetStatus();
}

void Mainwindow::on_checkBox_5_stateChanged(int arg1)
{
    arg1=arg1;
    updateSimTargetStatus();
}

void Mainwindow::on_checkBox_6_stateChanged(int arg1)
{
    arg1=arg1;
    updateSimTargetStatus();
}

void Mainwindow::on_checkBox_7_stateChanged(int arg1)
{
    arg1=arg1;
    updateSimTargetStatus();
}

void Mainwindow::on_checkBox_8_stateChanged(int arg1)
{
    arg1=arg1;
    updateSimTargetStatus();
}

//void Mainwindow::on_toolButton_start_simulation_set_2_clicked(bool checked)
//{
//    simulator->target[1].setIsManeuver(checked);
//}

//void Mainwindow::on_toolButton_start_simulation_set_3_clicked(bool checked)
//{
//    simulator->target[2].setIsManeuver(checked);
//}

//void Mainwindow::on_toolButton_start_simulation_set_4_clicked(bool checked)
//{
//    simulator->target[3].setIsManeuver(checked);
//}

//void Mainwindow::on_toolButton_start_simulation_set_5_clicked(bool checked)
//{
//    simulator->target[4].setIsManeuver(checked);
//}

//void Mainwindow::on_toolButton_start_simulation_set_6_clicked(bool checked)
//{
//    simulator->target[5].setIsManeuver(checked);
//}

//void Mainwindow::on_toolButton_start_simulation_set_7_clicked(bool checked)
//{
//    simulator->target[6].setIsManeuver(checked);
//}

//void Mainwindow::on_toolButton_start_simulation_set_8_clicked(bool checked)
//{
//    simulator->target[7].setIsManeuver(checked);
//}

void Mainwindow::on_toolButton_start_simulation_stop_clicked()
{

}

void Mainwindow::on_toolButton_start_simulation_stop_clicked(bool checked)
{
    if(checked)
    {
        simulator->pause();
        rda_main.processing->isSimulationMode = false;
    }
}

//void Mainwindow::on_bt_rg_1_clicked(bool checked)
//{

//}

void Mainwindow::on_toolButton_sim_target_autogenerate_clicked()
{
    ui->doubleSpinBox_1->setValue((rand()%720)/2.0);
    ui->doubleSpinBox_11->setValue((rand()%720)/2.0);
    ui->doubleSpinBox_21->setValue((rand()%720)/2.0);
    ui->doubleSpinBox_31->setValue((rand()%720)/2.0);
    ui->doubleSpinBox_41->setValue((rand()%720)/2.0);
    ui->doubleSpinBox_51->setValue((rand()%720)/2.0);
    ui->doubleSpinBox_61->setValue((rand()%720)/2.0);
    ui->doubleSpinBox_71->setValue((rand()%720)/2.0);

    ui->doubleSpinBox_2->setValue(2+(rand()%100)/2.0);
    ui->doubleSpinBox_12->setValue(2+(rand()%100)/2.0);
    ui->doubleSpinBox_22->setValue(2+(rand()%100)/2.0);
    ui->doubleSpinBox_32->setValue(2+(rand()%100)/2.0);
    ui->doubleSpinBox_42->setValue(2+(rand()%100)/2.0);
    ui->doubleSpinBox_52->setValue(2+(rand()%100)/2.0);
    ui->doubleSpinBox_62->setValue(2+(rand()%100)/2.0);
    ui->doubleSpinBox_72->setValue(2+(rand()%100)/2.0);

    ui->doubleSpinBox_3->setValue((rand()%720)/2.0);
    ui->doubleSpinBox_13->setValue((rand()%720)/2.0);
    ui->doubleSpinBox_23->setValue((rand()%720)/2.0);
    ui->doubleSpinBox_33->setValue((rand()%720)/2.0);
    ui->doubleSpinBox_43->setValue((rand()%720)/2.0);
    ui->doubleSpinBox_53->setValue((rand()%720)/2.0);
    ui->doubleSpinBox_63->setValue((rand()%720)/2.0);
    ui->doubleSpinBox_73->setValue((rand()%720)/2.0);

    ui->doubleSpinBox_4->setValue((rand()%50)/2.0);
    ui->doubleSpinBox_14->setValue((rand()%50)/2.0);
    ui->doubleSpinBox_24->setValue((rand()%50)/2.0);
    ui->doubleSpinBox_34->setValue((rand()%50)/2.0);
    ui->doubleSpinBox_44->setValue((rand()%50)/2.0);
    ui->doubleSpinBox_54->setValue((rand()%50)/2.0);
    ui->doubleSpinBox_64->setValue((rand()%50)/2.0);
    ui->doubleSpinBox_74->setValue((rand()%50)/2.0);
    updateSimTargetStatus();

}

void Mainwindow::on_checkBox_clicked()
{

}

void Mainwindow::on_toolButton_chong_nhieu_1_clicked(bool checked)
{
    //rda_main.processing->setVaru(checked);
    if(checked)
    {
        int depth = ui->horizontalSlider_varu_depth->value();
        depth = 16384/pow(10,depth/20.0);
        int width = ui->horizontalSlider_varu_width->value();
        width =  (16384 - depth)/10/width;
        uchar comand[8];
        comand[0] = 0x29;
        comand[1] = 0xab;
        comand[2] = 0x01;
        comand[3] = width;
        comand[4] = depth;
        comand[5] = depth>>8;
        rda_main.processing->sendCommand(&comand[0]);
    }
    else
    {
        sendToRadarHS("29ab00");
    }
}

//void Mainwindow::on_toolButton_chong_nhieu_2_clicked(bool checked)
//{
//    //rda_main.processing->setSharu(checked);
//}

//void Mainwindow::on_toolButton_chong_nhieu_3_clicked(bool checked)
//{
//    //rda_main.processing->setBaru(checked);
//    if(checked)
//    {
//        sendToRadarHS("03ab2f00");
//    }
//    else
//    {
//        sendToRadarHS("03abffff");
//    }
//}

void Mainwindow::on_toolButton_auto_freq_clicked(bool checked)
{
    if(checked)
    {
        sendToRadarString(CConfig::getString("mAutoFreqOnCommand"));

    }
    else sendToRadarString(CConfig::getString("mAutoFreqOffCommand"));
}

void Mainwindow::on_toolButton_chong_nhieu_ppy_clicked(bool checked)
{
    if(checked)
    {
        uchar value = ui->horizontalSlider_ppy_gain->value();
        uchar comand[8];
        comand[0] = 0x2a;
        comand[1] = 0xab;
        comand[2] = 0x01;
        comand[3] = value;
        rda_main.processing->sendCommand(&comand[0]);
    }
    else sendToRadarHS("2aab00");
}

void Mainwindow::on_toolButton_record_clicked(bool checked)
{
    if(checked)
    {

        QDateTime now = QDateTime::currentDateTime();
        QString filename = now.toString("dd.MM_hh.mm.ss")+
                "_"+ui->label_range_resolution->text()+
                "_"+ui->label_sn_type->text()+
                "_"+ui->label_sn_param->text();
        ui->label_record_file_name->setText(filename);
        rda_main.processing->startRecord("D:/HR2D/rec_"+filename+HR_FILE_EXTENSION);
    }
    else
    {
        rda_main.processing->stopRecord();
    }
}

void Mainwindow::on_bt_rg_5_clicked(bool checked)
{

}




void Mainwindow::on_toolButton_dzs_1_clicked(bool checked)
{
    if(checked)
    {
        setMouseMode(MouseAutoSelect1,true);
        this->setCursor(Qt::ArrowCursor);
    }
}

void Mainwindow::on_toolButton_dzs_2_clicked()
{
    rda_main.mRadarData->resetTrack();

}

void Mainwindow::on_toolButton_hdsd_clicked()
{
    DialogDocumentation *dlg=new DialogDocumentation();
    dlg->setModal(false);
    dlg->showNormal();
}

void Mainwindow::on_toolButton_dz_clear_clicked()
{
    rda_main.mRadarData->mDetectZonesList.clear();
}

void Mainwindow::on_toolButton_ais_show_clicked(bool checked)
{
    isShowAIS = checked;
    CConfig::setValue("isShowAIS",int(checked));
}

void Mainwindow::on_toolButton_loc_dia_vat_clicked(bool checked)
{
    rda_main.mRadarData->cut_terrain = checked;
}

void Mainwindow::on_toolButton_loc_dia_vat_2_clicked()
{
    rda_main.mRadarData->updateTerrain();
}

void Mainwindow::on_toolButton_tx_off_2_clicked()
{
    ShutDown();
}

void Mainwindow::on_toolButton_menu_2_clicked()
{
    DialogConfig *dlg= new DialogConfig();
    dlg->setModal(false);
    dlg->showNormal();
}

//void Mainwindow::on_toolButton_antennaConfigUpdate_clicked()
//{
//    ui->textEdit_headingAdjustInverse->text().toDouble()
//}

void Mainwindow::on_toolButton_dk_18_clicked()
{
    unsigned char command[]={0xaa,0x55,0x67,0x12,
                             01,
                             00,
                             0x00,0x00,0x00,0x00,0x00,0x00};
    rda_main.processing->sendCommand(command,12,false);
}

void Mainwindow::on_toolButton_exit_3_clicked(bool checked)
{
    if(checked)
    {
        sendToRadarString(CConfig::getString("mGosZaprosOn"));
    }
    else
    {
        sendToRadarString(CConfig::getString("mGosZaprosOff"));
    }
}

void Mainwindow::on_toolButton_passive_mode_clicked(bool checked)
{
    if(checked)
    {
        sendToRadarString(CConfig::getString("mPassiveModeOn"));
    }
    else
    {
        sendToRadarString(CConfig::getString("mPassiveModeOff"));
    }
}

void Mainwindow::on_toolButton_tx_clicked(bool checked)
{

}

void Mainwindow::on_horizontalSlider_varu_width_valueChanged(int value)
{
    if(ui->toolButton_chong_nhieu_1->isChecked())
    {
        int depth = ui->horizontalSlider_varu_depth->value();
        depth = 16384/pow(10,depth/20.0);
        int width = ui->horizontalSlider_varu_width->value();
        width =  (16384 - depth)/10/width;
        uchar comand[8];
        comand[0] = 0x29;
        comand[1] = 0xab;
        comand[2] = 0x01;
        comand[3] = width;
        comand[4] = depth;
        comand[5] = depth>>8;
        rda_main.processing->sendCommand(&comand[0]);
    }
}

void Mainwindow::on_horizontalSlider_varu_depth_valueChanged(int value)
{
    if(ui->toolButton_chong_nhieu_1->isChecked())
    {
        int depth = ui->horizontalSlider_varu_depth->value();
        depth = 16384/pow(10,depth/20.0);
        int width = ui->horizontalSlider_varu_width->value();
        width =  (16384 - depth)/10/width;
        uchar comand[8];
        comand[0] = 0x29;
        comand[1] = 0xab;
        comand[2] = 0x01;
        comand[3] = width;
        comand[4] = depth;
        comand[5] = depth>>8;
        rda_main.processing->sendCommand(&comand[0]);
    }
}

void Mainwindow::on_horizontalSlider_ppy_gain_valueChanged(int value)
{
    if(ui->toolButton_chong_nhieu_ppy->isChecked())
    {
        uchar comand[8];
        comand[0] = 0x2a;
        comand[1] = 0xab;
        comand[2] = 0x01;
        comand[3] = value;
        rda_main.processing->sendCommand(&comand[0]);
    }
}

void Mainwindow::on_toolButton_tx_3_clicked(bool checked)
{

}

void Mainwindow::on_comboBox_currentIndexChanged(int index)
{

    switch (index)
    {
    case 0 :
        rda_main.mRadarData->setFreqHeadOffsetDeg(CConfig::getDouble("mFreq1Offset",0));
        sendToRadarString(CConfig::getString("mFreq1Command"));
        break;
    case 1 :
        rda_main.mRadarData->setFreqHeadOffsetDeg(CConfig::getDouble("mFreq2Offset",0));
        sendToRadarString(CConfig::getString("mFreq2Command"));break;
    case 2 :
        rda_main.mRadarData->setFreqHeadOffsetDeg(CConfig::getDouble("mFreq3Offset",0));
        sendToRadarString(CConfig::getString("mFreq3Command"));break;
    case 3 :
        rda_main.mRadarData->setFreqHeadOffsetDeg(CConfig::getDouble("mFreq4Offset",0));
        sendToRadarString(CConfig::getString("mFreq4Command"));break;
    case 4 :
        rda_main.mRadarData->setFreqHeadOffsetDeg(CConfig::getDouble("mFreq5Offset",0));
        sendToRadarString(CConfig::getString("mFreq5Command"));break;
    case 5 :
        rda_main.mRadarData->setFreqHeadOffsetDeg(CConfig::getDouble("mFreq6Offset",0));
        sendToRadarString(CConfig::getString("mFreq6Command"));break;
    case 6 :
        rda_main.mRadarData->setFreqHeadOffsetDeg(CConfig::getDouble("mFreq7Offset",0));
        sendToRadarString(CConfig::getString("mFreq7Command"));break;
    case 7 :
        rda_main.mRadarData->setFreqHeadOffsetDeg(CConfig::getDouble("mFreq8Offset",0));
        sendToRadarString(CConfig::getString("mFreq8Command"));break;
    case 8 :
        rda_main.mRadarData->setFreqHeadOffsetDeg(CConfig::getDouble("mFreq9Offset",0));
        sendToRadarString(CConfig::getString("mFreq9Command"));break;
    case 9 :
        rda_main.mRadarData->setFreqHeadOffsetDeg(CConfig::getDouble("mFreq10Offset",0));
        sendToRadarString(CConfig::getString("mFreq10Command"));break;
    case 10 :
        rda_main.mRadarData->setFreqHeadOffsetDeg(CConfig::getDouble("mFreq11Offset",0));
        sendToRadarString(CConfig::getString("mFreq11Command"));break;
    case 11 :
        rda_main.mRadarData->setFreqHeadOffsetDeg(CConfig::getDouble("mFreq12Offset",0));
        sendToRadarString(CConfig::getString("mFreq12Command"));break;
    default : break;
    }
}

void Mainwindow::on_toolButton_cao_ap_1_clicked()
{
    unsigned char is_checked_1 = ui->toolButton_cao_ap_1->isChecked();
    unsigned char is_checked_2 = ui->toolButton_cao_ap_2->isChecked();
    commandMay22[8]=is_checked_1+is_checked_2*2;
    rda_main.processing->sendCommand(commandMay22,12,false);
}

void Mainwindow::on_toolButton_cao_ap_2_clicked()
{
    unsigned char is_checked_1 = ui->toolButton_cao_ap_1->isChecked();
    unsigned char is_checked_2 = ui->toolButton_cao_ap_2->isChecked();
    commandMay22[8]=is_checked_1+is_checked_2*2;
    rda_main.processing->sendCommand(commandMay22,12,false);
}

void Mainwindow::on_toolButton_antennaConfigUpdate_clicked()
{
    CConfig::setValue("antennaHeadOffset", ui->textEdit_headingAdjust->text().toDouble());
    rda_main.mRadarData->antennaHeadOffset = ui->textEdit_headingAdjust->text().toDouble();
    CConfig::setValue("mInverseRotAziCorrection", ui->textEdit_headingAdjustInverse->text().toDouble());
    rda_main.mRadarData->mInverseRotAziCorrection= ui->textEdit_headingAdjust->text().toDouble();
}

void Mainwindow::on_toolButton_exit_4_clicked(bool checked)
{
    if(checked )
    {
        sendToRadarString(CConfig::getString("m300WNewCommand"));
    }
    else
    {
        sendToRadarString(CConfig::getString("m300WOldCommand"));
    }
}

void Mainwindow::on_horizontalSlider_ppy_gain_2_valueChanged(int value)
{
    ui->label_speed_comp_value->setText(QString::number(ui->horizontalSlider_ppy_gain_2->value()));
    if(ui->toolButton_chong_nhieu_ppy_2->isChecked())
    {
        int value = ui->horizontalSlider_ppy_gain_2->value();
        uchar comand[8];
        comand[0] = 0x2b;
        comand[1] = 0xab;
        comand[2] = value;
        comand[3] = value>>8;
        rda_main.processing->sendCommand(&comand[0]);
    }
}

void Mainwindow::on_toolButton_chong_nhieu_ppy_2_clicked()
{

}

void Mainwindow::on_toolButton_chong_nhieu_ppy_2_clicked(bool checked)
{
    if(checked)
    {
        int value = ui->horizontalSlider_ppy_gain_2->value();
        uchar comand[8];
        comand[0] = 0x2b;
        comand[1] = 0xab;
        comand[2] = value;
        comand[3] = value>>8;
        rda_main.processing->sendCommand(&comand[0]);
    }
    else sendToRadarHS("2bab00");
}

//void Mainwindow::on_toolButton_ais_hide_fishing_clicked(bool checked)
//{
//    hideAisFishingBoat =checked;
//}

void Mainwindow::on_customButton_load_density_clicked()
{
    //    rda_main.processing->loadTargetDensityMap();
}

void Mainwindow::on_customButton_openCPN_clicked()
{
    system("taskkill /f /im opencpn.exe");
    QProcess::startDetached(CConfig::getString("navManager","\"C:\\Program Files (x86)\\OpenCPN\\opencpn.exe\"").toStdString().data());
    //    system("taskkill /f /im opencpn.exe");

}

void Mainwindow::on_lineEdit_simulation_lost_editingFinished()
{
    simulator->setLostRate(ui->lineEdit_simulation_lost->text().toInt());
}

void Mainwindow::on_toolButton_manual_tracking_clicked(bool checked)
{
    setMouseMode(MouseManualTrack,checked);
    rda_main.mRadarData->setManualTracking(checked);
}

void Mainwindow::on_toolButton_start_simulation_set_all_clicked(bool checked)
{
    simulator->setAllTarget();
}

//void Mainwindow::on_toolButton_replay_clicked(bool checked)
//{

//}

//void Mainwindow::on_toolButton_dk_4_clicked(bool checked)
//{

//}

void Mainwindow::on_horizontalSlider_valueChanged(int value)
{
    rda_main.processing->playRate = value;
}


void Mainwindow::on_toolButton_autotracking_clicked(bool checked)
{
    rda_main.mRadarData->setIsAutoTracking(checked);
}

void Mainwindow::on_toolButton_radar_clicked(bool checked)
{
    isRadarShow = checked;
}

void Mainwindow::on_tableWidgetTarget_clicked(const QModelIndex &index)
{

}
