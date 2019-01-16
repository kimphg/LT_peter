#include "c_mainwindow.h"
#include "statuswindow.h"
#include "ui_mainwindow.h"

#include <QMenu>
#include <QMessageBox>

#define MAX_VIEW_RANGE_KM   50
static QPen penTargetHistory(QBrush(Qt::gray),1);
static QPen penTargetEnemy(QBrush(Qt::magenta),2);
static QPen penTargetFriend(QBrush(QColor(0,200,200 ,255)),2);
static QPen penTargetEnemySelected(QBrush(Qt::magenta),3);
static QPen penTargetFriendSelected(QBrush(QColor(50,255,255 ,255)),3);

static QPen penCyan(QBrush(QColor(50,255,255 ,255)),1);//xoay mui tau
static QRect ppiRect(SCR_LEFT_MARGIN+SCR_BORDER_SIZE/2,
              SCR_TOP_MARGIN+SCR_BORDER_SIZE/2,
              SCR_H -SCR_BORDER_SIZE,
              SCR_H -SCR_BORDER_SIZE);
static PointAziRgkm AutoSelP1,AutoSelP2;
#ifdef THEON
static QPen penBackground(QBrush(QColor(24 ,48 ,64,255)),224+SCR_BORDER_SIZE);
static QRect circleRect = ppiRect.adjusted(-135,-135,135,135);

#else

static QPen penBackground(QBrush(QColor(24 ,48 ,64,255)),150+SCR_BORDER_SIZE);
QRect circleRect = ppiRect.adjusted(-135,-135,135,135);
#endif
static QRect mIADrect;
static QPen penYellow(QBrush(QColor(255,255,50 ,255)),1);
static QPen mGridViewPen1(QBrush(QColor(150,150,150,255)),1);
static clock_t clkBegin = clock();
static clock_t clkEnd = clock();
static clock_t paintTime = 20;
static QStringList                 commandLogList;
static QTransform                  mTrans;
static QPixmap                     *pMap=nullptr;// painter cho ban do
static bool isShowAIS =true;
//QPixmap                     *pViewFrame=NULL;// painter cho ban do
static CMap *osmap ;
static bool toolButton_grid_checked = true;
static StatusWindow                *mstatWin;
static double                      mHeadingT2,mHeadingT,mAziCorrecting;
static int                         mRangeIndex = 0;
static int                         mDistanceUnit=0;//0:NM;1:KM
static double                      mZoomSizeRg = 2;
static double                      mZoomSizeAz = 10;
static double                      mLat=DEFAULT_LAT,mLon = DEFAULT_LONG;
//static double                      CConfig::mStat.shipHeadingDeg=20;
static bool                        isMapOutdated = true;
static bool isHeadUp = false;
static int   mMousex =0,mMousey=0;
static dataProcessingThread        *processing;// thread xu ly du lieu radar
static c_radar_simulation          *simulator;// thread tao gia tin hieu
static C_radar_data                *pRadar;
static QThread                     *tprocessing;
static QPoint points[6];
static double                      mMapOpacity;
static int                         mMaxTapMayThu=18;
static QTimer                      timerVideoUpdate,timerMetaUpdate;
static QTimer                      syncTimer1s,syncTimer5p ;
static QTimer                      dataPlaybackTimer ;
static short                       dxMax,dyMax;
//static C_ARPA_data                 arpa_data;
static short                       scrCtX= SCR_H/2 + SCR_LEFT_MARGIN, scrCtY= SCR_H/2+SCR_TOP_MARGIN;
static short                       dx =0,dy=0,dxMap=0,dyMap=0;
static short                       radCtX= SCR_H/2 + SCR_LEFT_MARGIN;
static short                       radCtY= SCR_H/2+SCR_TOP_MARGIN;
static short                       mZoomCenterx,mZoomCentery,mMouseLastX,mMouseLastY;
static bool                        isScaleChanged =true;
static double                      mScale;
static double      rangeRatio = 1;
//extern CConfig         mGlobbalConfig;
static QStringList     warningList;
static QString         strDistanceUnit;
//short selectedTargetIndex;
static mouseMode mouse_mode = MouseNormal;
static DialogCommandLog *cmLog;
static unsigned char commandMay22[]={0xaa,0x55,0x02,0x0c,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
static enum TargetType{
    RADAR,AIS,NOTARGET
}selectedTargetType  = NOTARGET;

//short config.getRangeView() = 1;
static double ringStep = 1;
static double curAziRad = 3;
//static TrackPointer* currTrackPt;
//guard_zone_t gz1,gz2,gz3;
//static unsigned short cur_object_index = 0;
short lon2x(float lon)
{
    double refLat = mLat*0.00872664625997;
    return  short(- dx + scrCtX + ((lon - mLon) * 111.31949079327357*cos(refLat))*mScale);
}
short lat2y(float lat)
{

    return (- dy + scrCtY - ((lat - mLat) * 111.31949079327357)*mScale);
}
double y2lat(short y)
{
    return (y  )/mScale/111.31949079327357 + mLat;
}
double x2lon(short x)
{
    double refLat = mLat*0.00872664625997;
    return (x  )/mScale/111.31949079327357/cos(refLat) + mLon;
}
inline QString demicalDegToDegMin(double demicalDeg)
{
    return QString::number( (short)demicalDeg) +
            QString::fromLocal8Bit("\260")+
            QString::number((demicalDeg-(short)demicalDeg)*60.0,'f',2);
}
void Mainwindow::ConvXYradar2XYscr()
{

}
void Mainwindow::mouseDoubleClickEvent( QMouseEvent * e )
{
    if ( e->button() == Qt::LeftButton )
    {
        mMousex=this->mapFromGlobal(QCursor::pos()).x();
        mMousey=this->mapFromGlobal(QCursor::pos()).y();
        if(isInsideViewZone(mMousex,mMousey))
        {
            PointDouble point = ConvScrPointToKMXY(mMousex,mMousey);
            int dx = mMousex-radCtX;
            int dy = mMousey-radCtY;
            double rgKM = sqrt((dx*dx)+(dy*dy));
            pRadar->addDetectionZone(point.x,point.y,200/(rgKM)+1,7.0/mScale,true);
            //select radar target
            //select ais target
        }
        //ui->toolButton_manual_track->setChecked(false);

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
    processing->sendCommand(sendBuff,8);

}
void Mainwindow::sendToRadar(unsigned char* hexdata)
{
    m_udpSocket->writeDatagram((char*)hexdata,8,QHostAddress("192.168.0.44"),2572);
}
//ham test ve tu AIS
void Mainwindow::drawAisTarget(QPainter *p)
{
    //draw targets
    QPen penTarget(QColor(250,100,250));
    penTarget.setWidth(1);
    QPen penSelectedtarget = penTarget;
    penSelectedtarget.setWidth(2);
    p->setFont(QFont("Times", 6));
    QList<AIS_object_t>::iterator iter = processing->m_aisList.begin();
    while(iter!=processing->m_aisList.end())
    {
        AIS_object_t aisObj = *iter;
        iter++;
        if(aisObj.mUpdateTime>300000)continue;
        /*double fx,fy;
        ConvWGSToKm(&fx,&fy,aisObj.mLong,aisObj.mLat);
        short x = (fx*mScale);//+;
        short y = (fy*mScale);//+radCtY;
        rotateVector(trueShiftDeg,&x,&y);*/
        PointInt s = ConvWGSToScrPoint(aisObj.mLong,aisObj.mLat);

        if((aisObj.mType/10)==3)continue;
        if(aisObj.isNewest)
        {
            //printf("ais draw\n");
            //draw ais mark
            QPolygon poly;
            QPoint point;
            double head = aisObj.mCog*PI_NHAN2/360.0;
            point.setX(s.x+8*sinFast(head));
            point.setY(s.y-8*cosFast(head));
            poly<<point;
            point.setX(s.x+8*sinFast(head+2.3562f));
            point.setY(s.y-8*cosFast(head+2.3562f));
            poly<<point;
            point.setX(s.x+8*sinFast(head-2.3562f));
            point.setY(s.y-8*cosFast(head-2.3562f));
            poly<<point;
            point.setX(s.x+8*sinFast(head));
            point.setY(s.y-8*cosFast(head));
            poly<<point;
            point.setX(s.x+16*sinFast(head));
            point.setY(s.y-16*cosFast(head));
            poly<<point;
            if(aisObj.isSelected)
            {
                p->setPen(penSelectedtarget);
                p->drawPolygon(poly);
                p->drawText(s.x,s.y,100,20,0,aisObj.mName);
            }
            else
            {
                p->setPen(penTarget);
                p->drawPolygon(poly);
                p->drawText(s.x,s.y,100,20,0,aisObj.mName);
            }
        }
        else
        {
            p->drawPoint(s.x,s.y);
        }
    }

}
void Mainwindow::mouseReleaseEvent(QMouseEvent *event)
{
    setMouseMode(MouseDrag,false);
    //    if(isAddingTarget)
    //    {
    //        float xRadar = (mouseX - radCtX)/signsize ;//coordinates in  radar xy system
    //        float yRadar = -(mouseY - radCtY)/signsize;
    //        pRadar->addTrack(xRadar,yRadar);
    //        ui->actionAddTarget->toggle();
    //        isScreenUp2Date = false;
    //        return;
    //    }

    isMapOutdated = true;
    //    isScreenUp2Date = false;
    //isDraging = false;
    /*currMaxRange = (sqrtf(dx*dx+dy*dy)+scrCtY)/signsize;
    if(currMaxRange>RADAR_MAX_RESOLUTION)currMaxRange = RADAR_MAX_RESOLUTION;
    if((dx*dx+dy*dy)*3>scrCtX*scrCtX)
    {
        if(dx<0)
        {
            currMaxAzi = (unsigned short)((atanf((float)dy/(float)dx)-pRadar->trueN)/PI_NHAN2*4096.0f);
            if(currMaxAzi<0)currMaxAzi+=MAX_AZIR;
            if(currMaxAzi>MAX_AZIR)currMaxAzi-=MAX_AZIR;
        }
        if(dx>0)
        {
            currMaxAzi = (unsigned short)(((atanf((float)dy/(float)dx)+PI-pRadar->trueN))/PI_NHAN2*4096.0f);
            if(currMaxAzi>MAX_AZIR)currMaxAzi-=MAX_AZIR;
            if(currMaxAzi<0)currMaxAzi+=MAX_AZIR;
        }
        currMinAzi = currMaxAzi - MAX_AZIR/2;
        if(currMinAzi<0)currMinAzi+=MAX_AZIR;
        //printf("\n currMinAzi:%d currMaxAzi:%d ",currMinAzi,currMaxAzi);
    }else
    {
        currMaxAzi = MAX_AZIR;
        currMinAzi = 0;
    }*/
}
void Mainwindow::wheelEvent(QWheelEvent *event)
{
    event = event;
    //if(event->delta()>0)ui->horizontalSlider->setValue(ui->horizontalSlider->value()+1);
    //if(event->delta()<0)ui->horizontalSlider->setValue(ui->horizontalSlider->value()-1);
}
void Mainwindow::mouseMoveEvent(QMouseEvent *event) {

    if((mouse_mode&MouseDrag)&&(event->buttons() & Qt::LeftButton)) {

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
            isMapOutdated = true;
            radCtX = scrCtX-dx;
            radCtY = scrCtY-dy;
        }
    }
}
bool controlPressed = false;
void Mainwindow::keyPressEvent(QKeyEvent *event)
{
    this->setFocus();
    int key = event->key();
    if(key == Qt::Key_Control)
    {
        controlPressed  = true;
    }
    else if(controlPressed)
    {
        if(key==Qt::Key_1)
        {
            mMousex=this->mapFromGlobal(QCursor::pos()).x();
            mMousey=this->mapFromGlobal(QCursor::pos()).y();

            SetGPS(y2lat(-(mMousey - radCtY)),
                   x2lon(mMousex - radCtX)
                   );

        }
        else if(key==Qt::Key_2)
        {
            pRadar->clearPPI();
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
    else if(key == Qt::Key_Space)
    {
        mMousex=this->mapFromGlobal(QCursor::pos()).x();
        mMousey=this->mapFromGlobal(QCursor::pos()).y();
#ifndef THEON
        if(!isInsideViewZone(mMousex,mMousey))return;
        double azid,rg;
        C_radar_data::kmxyToPolarDeg((mMousex - radCtX)/mScale,-(mMousey - radCtY)/mScale,&azid,&rg);
        int aziBinary = int(azid/360.0*4096);
        unsigned char command[]={0xaa,0x55,0x6a,0x09,
                                 static_cast<unsigned char>(aziBinary>>8),
                                 static_cast<unsigned char>(aziBinary),
                                 0x00,0x00,0x00,0x00,0x00,0x00};
        processing->sendCommand(command,9,false);
#endif
        mZoomCenterx = mMousex;
        mZoomCentery = mMousey;

        pRadar->setZoomRectAR((mMousex - radCtX)/mScale,
                              -(mMousey - radCtY)/mScale,
                              mZoomSizeRg,mZoomSizeAz);
        pRadar->setZoomRectXY((mMousex - radCtX),(mMousey - radCtY));
    }
    else if((!controlPressed)&&key >= Qt::Key_1&&key<=Qt::Key_6)
    {
        /*int keyNum = key-Qt::Key_1;
        if(keyNum>=TARGET_TABLE_SIZE)return;
        if(!mTargetMan.selectedTrackID)return;
        /*if(keyNum>0){
            for(int i=0;i<pRadar->mTrackList.size();i++)
            {
                track_t* track=&( pRadar->mTrackList[i]);
                if(track->uniqId==mTargetMan.selectedTrackID)
                {

                    mmTargetMan.selectedTrackID=track->uniqId;
                    return;
                }
            }
        }*/
        //        for(int i=0;i<pRadar->mTrackList.size();i++)
        //        {
        //            track_t* track=&( pRadar->mTrackList[i]);
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
    //float scale_ppi = pRadar->scale_ppi;
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
bool Mainwindow::isInsideViewZone(int x, int y)
{
    short dx = x-scrCtX;
    short dy = y-scrCtY;
    if((dx*dx+dy*dy)>(SCR_H*SCR_H/4))
        return false;
    else
        return true;
}
void Mainwindow::mousePressEvent(QMouseEvent *event)
{
    mMouseLastX = (event->x());
    mMouseLastY = (event->y());
    if(!isInsideViewZone(mMouseLastX,mMouseLastY))return;
    if(event->buttons() & Qt::LeftButton) {

        mMousex=this->mapFromGlobal(QCursor::pos()).x();
        mMousey=this->mapFromGlobal(QCursor::pos()).y();
        if(isInsideViewZone(mMousex,mMousey))
        {
            if(!isHeadUp)setMouseMode(MouseDrag,true);
            if(mouse_mode&MouseAutoSelect1)
            {
                AutoSelP1 =  ConvScrPointToAziRgkm(mMousex,mMousey);
                setMouseMode(MouseAutoSelect2,true);
                setMouseMode(MouseAutoSelect1,false);
            }
            else if(mouse_mode&MouseAutoSelect2)
            {
                ui->toolButton_dzs_1->setChecked(false);
                AutoSelP2 =  ConvScrPointToAziRgkm(mMousex,mMousey);
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

                //PointDouble point = ConvScrPointToKMXY(mMousex,mMousey);
                //int dx = mMousex-radCtX;
                //int dy = mMousey-radCtY;
                //double rgKM = sqrt((dx*dx)+(dy*dy));
                pRadar->addDetectionZoneAZ(cazi,cRg,dazi,dRg,false);
            }
            else
            {//select radar target
                int minDistanceToCursor = 10;
                //unsigned long long trackMin = 0;
                int trackSel=0;
                for (uint i = 0;i<TRACK_TABLE_SIZE;i++)
                {
                    TrackPointer* ptrack = mTargetMan.getTrackAt(i);
                    if(ptrack==nullptr)continue;
                    PointInt s = ConvKmXYToScrPoint(ptrack->track->xkm,ptrack->track->ykm);

                    int dsx   = abs(s.x - mMousex);
                    int dsy   = abs(s.y - mMousey);
                    if(dsx+dsy<minDistanceToCursor)
                    {
                        minDistanceToCursor = dsx+dsy;
                        //trackMin = track->uniqId;
                        trackSel = ptrack->track->uniqId;

                        //                tracktime = track->time;
                    }

                }
                if(trackSel!=0)
                {
                    mTargetMan.setSelectedTrack(trackSel);
                    //mTargetMan.currTrackPt = mTargetMan.getTrackById(trackSel);
                    showTrackContext();
                }
            }
        }
        /*if(mouse_mode&MouseScope)
        {
            double azid,rg;
            C_radar_data::kmxyToPolarDeg((mMouseLastX - radCtX)/mScale,-(mMouseLastY - radCtY)/mScale,&azid,&rg);

            pRadar->drawRamp(azid);
        }*/
        /*else if(ui->toolButton_create_zone->isChecked())
        {
            gz1.isActive = 1;
            gz1.x1 = event->x();
            gz1.y1 = event->y();
        }
        else if(ui->toolButton_create_zone_2->isChecked())
        {
            gz2.isActive = 1;
            gz2.x1 = event->x();
            gz2.y1 = event->y();
        }
        else if(ui->toolButton_create_zone_3->isChecked())
        {
            gz3.isActive = 1;
            gz3.x1 = event->x();
            gz3.y1 = event->y();
        }
        else*/
        {

            //mouse_mode=MouseDrag;//isDraging = true;
        }
    }
    else
    {
        if(ui->toolButton_ais_show->isChecked())
        {
            checkClickAIS(event->x(),event->y());
        }
    }

}
void Mainwindow::checkClickAIS(int xclick, int yclick)
{
    QList<AIS_object_t>::iterator iter = processing->m_aisList.begin();
    while(iter!=processing->m_aisList.end())
    {
        AIS_object_t aisObj = *iter;
        iter++;
        if(aisObj.isSelected)continue;
        if(!aisObj.isNewest)continue;
        double fx,fy;
        ConvWGSToKm(&fx,&fy,aisObj.mLong,aisObj.mLat);
        int x = (fx*mScale)+radCtX;
        int y = (fy*mScale)+radCtY;
        if(abs(x-xclick)<5&&abs(y-yclick)<5)
        {
            DialogAisInfo *dialog = new DialogAisInfo(this);
            dialog->setAttribute( Qt::WA_DeleteOnClose, true );
            dialog->setWindowFlags(dialog->windowFlags()&(~Qt::WindowContextHelpButtonHint));
            dialog->setFixedSize(dialog->width(),dialog->height());
            dialog->setAisData(&processing->m_aisList,aisObj.mMMSI);
            dialog->show();
            break;
        }
    }
}
/*void MainWindow::wheelEvent(QWheelEvent *event)
{
//    if(event->delta()>0)ui->horizontalSlider->raise();
//    if(event->delta()<0)ui->horizontalSlider->setValue(3);
//    if(scale>SCALE_MAX)scale=SCALE_MAX;
//    if(scale<SCALE_MIN)scale=SCALE_MIN;
//    //signsize = SIGNAL_SCALE/scale;
//    DrawMap();
//    update();
}*/
Mainwindow::Mainwindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    degreeSymbol= QString::fromLocal8Bit("\260");
    //ui->frame_RadarViewOptions->hide();
    QFont font;
    font.setPointSize(12);
    cmLog = new DialogCommandLog();
    //    mShowobjects = false;
    //    mShowLines = false;
//    mShowTracks = false;
    InitNetwork();
    InitTimer();
    setFocusPolicy(Qt::StrongFocus);
    InitSetting();
    setRadarState(DISCONNECTED);
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
//void MainWindow::createActions()
//{
//    a_openShp = new QAction(tr("&Open Shp"), this);
//    a_openShp->setShortcuts(QKeySequence::Open);
//    a_openShp->setStatusTip(tr("Open shp file"));
//    connect(a_openShp, SIGNAL(triggered()), this, SLOT(openShpFile()));
//    //______________________________________//
//    a_openPlace = new QAction(tr("&Open place file"), this);
//    a_openPlace->setStatusTip(tr("Open place file"));
//    connect(a_openPlace, SIGNAL(triggered()), this, SLOT(openPlaceFile()));
//    //______________________________________//
//    a_gpsOption = new QAction(tr("&GPS option"), this);
//    a_gpsOption->setStatusTip(tr("GPS option"));
//    connect(a_gpsOption, SIGNAL(triggered()), this, SLOT(gpsOption()));
//    //______________________________________//
//    a_openSignal = new QAction(tr("&Open signal file"), this);
//    a_openSignal->setStatusTip(tr("Mở file tín hiệu đã lưu."));
//    connect(a_openSignal, SIGNAL(triggered()), this, SLOT(openSignalFile()));

//}
//void MainWindow::openSignalFile()
//{
//    //printf("shp file max ");
//    QString fileName = QFileDialog::getOpenFileName(this,
//        tr("Open signal file"), NULL, tr("Signal data Files (*.dat)"));
//    rawData.OpenFile(fileName.toStdString().c_str());

//    //SHPHandle hSHP = SHPOpen(fileName.toStdString().c_str(), "rb" );
//    //if(hSHP == NULL) return;
//}
/*
static short curMapLayer=0;

void MainWindow::openShpFile()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Open SHP file"), NULL, tr("Shp Files (*.shp)"));
    if(!fileName.size())return;
    vnmap.OpenShpFile(fileName.toStdString().c_str(), curMapLayer );
    vnmap.LoadPlaces(fileName.toStdString().c_str());
    curMapLayer++;
    DrawMap();
    //DrawToPixmap(pPixmap);
    update();

}*/

Mainwindow::~Mainwindow()
{
    processing->stopThread();
    processing->wait();

    delete ui;
    CConfig::SaveToFile();
    if(pMap)delete pMap;
}

void Mainwindow::DrawMap()
{
    if(!isMapOutdated)return;

    isMapOutdated = false;
    if(!pMap) return;
    pMap->fill(Qt::black);
    dxMap = 0;
    dyMap = 0;
    //
    QPainter pMapPainter(pMap);
    double dLat, dLong;
    ConvKmToWGS((double(dx))/mScale,(double(-dy))/mScale,&dLong,&dLat);
    osmap->setCenterPos(dLat,dLong);
    QPixmap pix = osmap->getImage(mScale);

    if(isHeadUp)
    {
        pix=pix.transformed(mTrans);
    }
    pMapPainter.setOpacity(mMapOpacity);
    pMapPainter.drawPixmap((-pix.width()/2+pMap->width()/2),
                           (-pix.height()/2+pMap->height()/2),pix.width(),pix.height(),pix
                           );
    repaint();
    //view frame
    //fill back ground
    //p.setBrush(QColor(40,60,100,255));
    //p.drawRect(scrCtX+scrCtY,0,width()-scrCtX-scrCtY,height());
    //p.drawRect(0,0,scrCtX-scrCtY,height());
    //p.setCompositionMode(QPainter::CompositionMode_SourceOver);
    //pMapPainter.setOpacity(1);
    //grid
    /*if(toolButton_grid_checked)
    {

        if(ui->toolButton_measuring->isChecked())
        {
            DrawGrid(&pMapPainter,mMouseLastX-SCR_LEFT_MARGIN,mMouseLastY);
        }
        else
        {
            DrawGrid(&pMapPainter,scrCtX- SCR_LEFT_MARGIN-dx,radCtY);
        }
    }
    //frame
    pMapPainter.setBrush(Qt::NoBrush);
    pMapPainter.setPen(penBackground);
    short i=200;
    pMapPainter.drawEllipse(-i/2,-i/2,SCR_H+i,SCR_H+i);*/

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
        int rad = i*ringStep*rangeRatio*mScale;
        p->drawEllipse(QPoint(centerX,centerY),
                       (short)(rad),
                       (short)(rad));
        p->drawText(centerX+2,centerY-rad+2,100,20,0,QString::number(i*ringStep)+strDistanceUnit);
        p->drawText(centerX+2,centerY+rad+2,100,20,0,QString::number(i*ringStep)+strDistanceUnit);
        p->drawText(centerX+rad+2,centerY+2,100,20,0,QString::number(i*ringStep)+strDistanceUnit);
        p->drawText(centerX-rad+2,centerY+2,100,20,0,QString::number(i*ringStep)+strDistanceUnit);
    }
    short theta;
    short gridR = ringStep*1.852f*mScale*5;
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
void Mainwindow::rotateVector(double angle,int* x,int* y)
{
    if(abs(angle)<0.1)return;
    double theta = radians(angle);
    double cs = cos(theta);
    double sn = sin(theta);

    double px = (*x) * cs - (*y) * sn;
    double py = (*x) * sn + (*y) * cs;
    (*x) = px;
    (*y) = py;
}
void Mainwindow::rotateVector(double angle,double* x,double* y)
{
    if(abs(angle)<0.1)return;
    double theta = radians(angle);
    double cs = cos(theta);
    double sn = sin(theta);
    double px = (*x) * cs - (*y) * sn;
    double py = (*x) * sn + (*y) * cs;
    (*x) = px;
    (*y) = py;
}
void Mainwindow::DrawDetectZones(QPainter* p)//draw radar target from pRadar->mTrackList
{
    p->setPen((penYellow));
    for (uint i = 0;i<pRadar->mDetectZonesList.size();i++)
    {
        DetectionWindow *dw = &pRadar->mDetectZonesList[i];
        if(dw->isRemoved)continue;
        int dazi = dw->maxDazDeg;
        double azi = 90.0-(trueShiftDeg+dw->aziDeg);
        int drg =  int(dw->maxDrg*mScale);
        int rg = int(dw->rg*mScale)-drg;
        /*short sx = dw->xkm*mScale ;//+ radCtX;
        short sy = -dw->ykm*mScale ;//+ radCtY;
        rotateVector(trueShift,&sx,&sy);*/
        int x1 = radCtX-rg;
        int y1 = radCtY-rg;
        p->drawArc(x1,y1,rg*2,rg*2,(azi-dazi)*5760/360,int(dazi*2)*5760/360);
        rg+=(drg*2);
        x1 = radCtX-rg;
        y1 = radCtY-rg;
        p->drawArc(x1,y1,rg*2,rg*2,(azi-dazi)*5760/360,int(dazi*2)*5760/360);

        azi = (trueShiftDeg+dw->aziDeg);
        p->drawLine(radCtX+rg*sin(radians(azi-dazi)),
                    radCtY-rg*cos(radians(azi-dazi)),
                    radCtX+(rg-2*drg)*sin(radians(azi-dazi)),
                    radCtY-(rg-2*drg)*cos(radians(azi-dazi)));
        p->drawLine(radCtX+rg*sin(radians(azi+dazi)),
                    radCtY-rg*cos(radians(azi+dazi)),
                    radCtX+(rg-2*drg)*sin(radians(azi+dazi)),
                    radCtY-(rg-2*drg)*cos(radians(azi+dazi)));
    }

}

#define TARG_SIZE 12
void Mainwindow::DrawRadarTargetByPainter(QPainter* p)//draw radar target from pRadar->mTrackList
{
    p->setFont(QFont("Times", 8));


    //penTargetBlue.setStyle(Qt::DashLine);
    //QPen penARPATrack(Qt::darkYellow);
    //draw radar targets
    //float x,y;
//    short sx,sy;
//    short sx1=0,sy1=0;
    //float scale_ppi = pRadar->scale_ppi;
    //short targetId = 0;
    p->setPen(penCyan);
    std::vector<object_t>* pObjList = &(pRadar->mFreeObjList);
    for (uint i = 0;i<pObjList->size();i++)
    {
        if(pObjList->at(i).isRemoved)continue;
        PointInt sTrack = ConvKmXYToScrPoint(pObjList->at(i).xkm,pObjList->at(i).ykm);
        //p->drawPoint(sTrack.x,sTrack.y);
        p->drawRect(sTrack.x-5,sTrack.y-5,10,10);
    }
    for (uint i = 0;i<pRadar->mTrackList.size();i++)
    {
        C_primary_track* track = &(pRadar->mTrackList[i]);
        if(track->mState==TrackState::newDetection)
        {
            PointInt sTrack = ConvKmXYToScrPoint(track->objectList.back().xkm,track->objectList.back().ykm);
            //p->drawPoint(sTrack.x,sTrack.y);
            p->drawRect(sTrack.x-5,sTrack.y-5,10,10);
            //p->drawText(sTrack.x+10,sTrack.y+10,100,50,0,QString::number(track->objectList.size()));
        }
    }
    //    p->setPen(penTargetBlue);

    bool blink = (CConfig::time_now_ms/500)%2;
    //draw targeted tracks
    p->setPen(penTargetEnemy);
    for (uint i = 0;i<TARGET_TABLE_SIZE;i++)
    {
        TrackPointer* trackPt = mTargetMan.getTargetAt(i);
        if(!trackPt)continue;
        C_primary_track* track = trackPt->track;
        PointInt s = ConvKmXYToScrPoint(track->xkm,track->ykm);
        p->drawLine(s.x-20,s.y,s.x-10,s.y);
        p->drawLine(s.x+20,s.y,s.x+10,s.y);
        p->drawLine(s.x,s.y-20,s.x,s.y-10);
        p->drawLine(s.x,s.y+20,s.x,s.y+10);
    }
    //draw all tracks
    for (uint i = 0;i<TRACK_TABLE_SIZE;i++)
    {
        TrackPointer* trackPt = mTargetMan.getTrackAt(i);
        if(!trackPt)continue;

        C_primary_track* track = trackPt->track;
        PointInt sTrack = ConvKmXYToScrPoint(track->xkm,track->ykm);
        if(trackPt->selected)//selected
        {
            // draw track history
            p->setPen(penTargetHistory);
            for (int j = 0;j<track->objectHistory.size()-1;j++)
            {
                object_t* obj1 = &(track->objectHistory[j]);
                object_t* obj2 = &(track->objectHistory[j+1]);
                PointInt s = ConvKmXYToScrPoint(obj1->xkm,obj1->ykm);
                PointInt s1 = ConvKmXYToScrPoint(obj2->xkm,obj2->ykm);
                p->drawLine(s1.x,s1.y,s.x,s.y);
            }
            if(trackPt->flag>=0)p->setPen(penTargetEnemySelected);
            else  p->setPen(penTargetFriendSelected);
        }
        else
        {

            if(trackPt->flag>=0)p->setPen(penTargetEnemy);
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
        //nornal targets
        else
        {

            int size = 18000.0/(CConfig::time_now_ms - track->lastTimeMs+400);
            if(size<TARG_SIZE)size=TARG_SIZE;//rect size depend to time
            if(track->isConfirmed())
            {

                p->drawEllipse(sTrack.x-size/2,sTrack.y-size/2,size,size);
                if(track->mSpeedkmhFit>10){
                    int sx = sTrack.x+short(10*sinFast(track->courseRadFit));
                    int sy = sTrack.y-short(10*cosFast(track->courseRadFit));
                    p->drawLine(sx,sy,sTrack.x,sTrack.y);
                }

            }
            else {
                //p->setPen(penSelTarget);
                p->drawRect(sTrack.x-size/2,sTrack.y-size/2,size,size);
            }
            //draw target number
            p->drawText(sTrack.x+6,sTrack.y+6,100,50,0,QString::number(track->uniqId));
        }

    }



}
void Mainwindow::ConvWGSToKm(double* x, double *y, double m_Long,double m_Lat)
{

    double refLat = (mLat + (m_Lat))*0.00872664625997;//pi/360
    *x	= (((m_Long) - mLon) * 111.31949079327357)*cos(refLat);// 3.14159265358979324/180.0*6378.137);//deg*pi/180*rEarth
    *y	= ((mLat - (m_Lat)) * 111.132954);
    //tinh toa do xy KM so voi diem center khi biet lat-lon
}
PointInt Mainwindow::ConvWGSToScrPoint(double m_Long,double m_Lat)
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
PointInt Mainwindow::ConvKmXYToScrPoint(double x, double y)
{
    PointInt s;
    s.x = x*mScale ;
    s.y = -y*mScale ;
    rotateVector(trueShiftDeg,&s.x,&s.y);
    s.x   += radCtX;
    s.y   += radCtY;
    return s;
}
PointDouble Mainwindow::ConvScrPointToKMXY(int x, int y)
{
    PointDouble output;
    output.x = (x-radCtX)/mScale;
    output.y = -(y-radCtY)/mScale;
    rotateVector(trueShiftDeg,&output.x,&output.y);
    return output;
}
PointAziRgkm Mainwindow::ConvScrPointToAziRgkm (int x, int y)
{
    PointDouble p;
    PointAziRgkm ouput;
    p.x = (x-radCtX)/mScale;
    p.y = -(y-radCtY)/mScale;
    rotateVector(trueShiftDeg,&p.x,&p.y);
    ouput.aziRad = ConvXYToAziRd(p.x,p.y);
    ouput.rg = sqrt(p.x*p.x+p.y*p.y);;
    return ouput;
}
void Mainwindow::ConvKmToWGS(double x, double y, double *m_Long, double *m_Lat)
{
    *m_Lat  = mLat +  (y)/(111.132954);
    double refLat = (mLat +(*m_Lat))*0.00872664625997;//3.14159265358979324/180.0/2;
    *m_Long = (x)/(111.31949079327357*cos(refLat))+ mLon;
    //tinh toa do lat-lon khi biet xy km (truong hop coi trai dat hinh cau)
}

void Mainwindow::UpdateMouseStat(QPainter *p)
{

    if(!isInsideViewZone(mMousex,mMousey))return;
    if((mouse_mode&MouseVRM)||(mouse_mode&MouseELB)||(mouse_mode&MouseAutoSelect1)||(mouse_mode&MouseAutoSelect2))
    {
        QPen penmousePointer(QColor(0x50ffffff));
        penmousePointer.setWidth(2);

        short r = sqrt((mMousex - radCtX)*(mMousex - radCtX)+(mMousey - radCtY)*(mMousey - radCtY));
        p->setPen(penmousePointer);

        if((mouse_mode&MouseAutoSelect1)||(mouse_mode&MouseAutoSelect2))
        {
            p->drawEllipse(QPoint(radCtX,radCtY),r,r);
            p->drawLine(QPoint(radCtX,radCtY),QPoint(radCtX-(radCtX-mMousex)*100,radCtY-(radCtY-mMousey)*100));
        }
        else
        {
            if(mouse_mode&MouseVRM)p->drawEllipse(QPoint(radCtX,radCtY),r,r);
            if(mouse_mode&MouseELB)p->drawLine(QPoint(radCtX,radCtY),QPoint(radCtX-(radCtX-mMousex)*100,radCtY-(radCtY-mMousey)*100));
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
    p.setRenderHint(QPainter::Antialiasing, true);
    if(pMap)
    {
        p.drawPixmap(SCR_LEFT_MARGIN,SCR_TOP_MARGIN,SCR_H,SCR_H,
                     *pMap,
                     dxMap,dyMap,SCR_H,SCR_H);
    }
    //draw signal
    QRectF screen(0,0,SCR_W,SCR_H);
    if(isHeadUp)//isHeadUp)
    {
        QImage newImg = pRadar->img_ppi->transformed(mTrans);
        QRectF signRectTemp = QRectF(newImg.width()/2-(radCtX),newImg.height()/2-(radCtY),SCR_W,SCR_H);
        p.drawImage(screen,newImg,signRectTemp,Qt::AutoColor);

        //        pMapPainter.drawPixmap((-pix.width()/2+pMap->width()/2),
        //                     (-pix.height()/2+pMap->height()/2),pix.width(),pix.height(),pix
        //                     );
    }
    else
    {
        QRectF signRect(RAD_DISPLAY_RES-(radCtX),RAD_DISPLAY_RES-(radCtY),SCR_W,SCR_H);
        p.drawImage(screen,*pRadar->img_ppi,signRect,Qt::AutoColor);
    }

    //    QPixmap dstPix = QPixmap::fromImage(*pRadar->img_ppi);



    //    p.drawPixmap(screen,dstPix,signRect);
    DrawRadarTargetByPainter(&p);
    //if(ui->toolButton_ais_show->isChecked())drawAisTarget(&p);
    //draw cursor
    //    QPen penmousePointer(QColor(0x50ffffff));

    //    penmousePointer.setWidth(2);
    //    p.setPen(penmousePointer);
    //    p.drawLine(mousePointerX-15,mousePointerY,mousePointerX-10,mousePointerY);
    //    p.drawLine(mousePointerX+15,mousePointerY,mousePointerX+10,mousePointerY);
    //    p.drawLine(mousePointerX,mousePointerY-10,mousePointerX,mousePointerY-15);
    //    p.drawLine(mousePointerX,mousePointerY+10,mousePointerX,mousePointerY+15);
    //draw mouse coordinates

    UpdateMouseStat(&p);

    if(isShowAIS)drawAisTarget(&p);
    //ve luoi cu ly phuong vi
    DrawDetectZones(&p);
    DrawViewFrame(&p);
    DrawIADArea(&p);
    clkEnd = clock();
    paintTime = (clkEnd-clkBegin);
}
void Mainwindow::DrawIADArea(QPainter* p)
{
    if(ui->tabWidget_iad->isHidden())return;
    p->setCompositionMode(QPainter::CompositionMode_SourceOver);
    mIADrect = ui->tabWidget_iad->geometry();
    mIADrect.adjust(4,30,-5,-5);
    p->setBrush(QBrush(Qt::black));
    p->setPen(Qt::black);
    p->drawRect(mIADrect);
    if(ui->tabWidget_iad->currentIndex()==0)
    {
        if((pRadar->img_zoom_ar==nullptr)||(pRadar->img_zoom_ar->isNull()))return;
        p->setPen(QPen(Qt::white,2));
        QPoint p1(mIADrect.x(),mIADrect.y());
        //QPoint p2(rect.x(),rect.y());
        QPoint p11(mIADrect.x()+mIADrect.width(),mIADrect.y());
        QPoint p22(mIADrect.x(),mIADrect.y()+mIADrect.height());
        p->drawLine(p1,p11);
        p->drawLine(p1,p22);
        int step = mIADrect.width()/5;
        for(int i = 0;i<5;i++)
        {
            p->drawLine(mIADrect.x()+step*i,mIADrect.y(),mIADrect.x()+step*i,mIADrect.y()+5);
            p->drawLine(mIADrect.x(),mIADrect.y()+step*i,mIADrect.x()+5,mIADrect.y()+step*i);
        }
        p->setFont(QFont("Times",10));
        p->drawText(mIADrect.x()+mIADrect.width()-50,mIADrect.y()+15,QString::number(mZoomSizeRg/0.1852,'f',1)+QString::fromUtf8(" Liên"));
        p->drawText(mIADrect.x()+5,mIADrect.y()+mIADrect.height()-5,QString::number(mZoomSizeAz,'f',1)+QString::fromUtf8(" Độ"));
        QImage img = pRadar->img_zoom_ar->scaled(mIADrect.width(),mIADrect.height(),Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
        p->drawImage(mIADrect,img);//todo:resize



    }
    else if(ui->tabWidget_iad->currentIndex()==4)
    {
        //if((!pRadar->img_zoom_ppi)||(pRadar->img_zoom_ppi->isNull()))return;
        p->drawImage(mIADrect,*pRadar->img_zoom_ppi,pRadar->img_zoom_ppi->rect());
        if(mRangeIndex>2)
        {
            short zoom_size = ui->tabWidget_iad->width()/pRadar->scale_zoom_ppi*pRadar->scale_ppi;
            p->setPen(QPen(QColor(255,255,255,200),0,Qt::DashLine));
            p->setBrush(Qt::NoBrush);
            p->drawRect(mZoomCenterx-zoom_size/2.0,mZoomCentery-zoom_size/2.0,zoom_size,zoom_size);
        }

    }
    else if(ui->tabWidget_iad->currentIndex()==1)
    {

        p->drawImage(mIADrect,*pRadar->img_histogram,
                     pRadar->img_histogram->rect());

    }
    else if(ui->tabWidget_iad->currentIndex()==2)
    {

        p->drawImage(mIADrect,*pRadar->img_spectre,
                     pRadar->img_spectre->rect());
    }
    else if(ui->tabWidget_iad->currentIndex()==3)
    {
        if(ui->toolButton_scope_2->isChecked()==false)pRadar->drawRamp();
        QRect rect1 = mIADrect;
        rect1.adjust(0,0,0,-mIADrect.height()/2);
        //        pengrid.setWidth(10);
        //        p->setPen(pengrid);
        p->drawImage(rect1,*pRadar->img_RAmp);
        double rampos = ui->horizontalSlider_ramp_pos_2->value()/(double(ui->horizontalSlider_ramp_pos_2->maximum()));
        QRect rect2 = mIADrect;
        rect2.adjust(0,mIADrect.height()/2,0,0);
        int zoomw = rect2.width()/2;
        int ramposInt = (pRadar->img_RAmp->width()-zoomw)*rampos;
        QRect srect(ramposInt,0,zoomw,pRadar->img_RAmp->height());
        p->drawImage(rect2,*pRadar->img_RAmp,srect);
        //p->drawRect(rect1,pRadar->img_RAmp->width()+5,pRadar->img_RAmp->height()+5);
        //        pengrid.setWidth(2);
        //        pengrid.setColor(QColor(128,128,0,120));
        //        p->setPen(pengrid);
        //        for(short i=60;i<pRadar->img_RAmp->height();i+=50)
        //        {
        //            p->drawLine(0,height()-i,pRadar->img_RAmp->width()+5,height()-i);
        //        }
        //        for(short i=110;i<pRadar->img_RAmp->width();i+=100)
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
        //delete
        QAction action2(QString::fromUtf8("Xóa"), this);
        connect(&action2, SIGNAL(triggered()), this, SLOT(removeTrack()));
        contextMenu.addAction(&action2);
        contextMenu.exec((QCursor::pos()));
    }
}
void Mainwindow::showTrackContext()
{
    if(!mTargetMan.currTrackPt)return;
    QMenu contextMenu(tr("Context menu"), this);
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
    C_primary_track* track = mTargetMan.currTrackPt->track;
    //Ph. vị
    QAction action7(QString::fromUtf8("Ph. vị:    ")+QString::number(track->aziDeg,'f',1) , this);
    //connect(&action1, &QAction::triggered, this, &Mainwindow::removeTrack);
    contextMenu.addAction(&action7);
    //Cự ly
    QAction action8(QString::fromUtf8("Cự ly(Nm): ")+QString::number(nm(track->rgKm),'f',2) , this);
    //connect(&action1, &QAction::triggered, this, &Mainwindow::removeTrack);
    contextMenu.addAction(&action8);
    //Hướng cđ:
    QAction action9(QString::fromUtf8("Hướng cđ:  ")+QString::number(track->courseDeg,'f',1)  , this);
    //connect(&action1, &QAction::triggered, this, &Mainwindow::removeTrack);
    contextMenu.addAction(&action9);
    //Tốc độ:
    QAction action10(QString::fromUtf8("Tốc độ:   ")+QString::number(nm(track->mSpeedkmhFit),'f',1) , this);
    //connect(&action1, &QAction::triggered, this, &Mainwindow::removeTrack);
    contextMenu.addAction(&action10);
    //Dopler
    QAction action5(QString::fromUtf8("Dopler:    ")+QString::number(track->mDopler), this);
    //connect(&action1, &QAction::triggered, this, &Mainwindow::removeTrack);
    contextMenu.addAction(&action5);
    //Dopler
    int secs = int(track->lastTimeMs-track->startTime)/1000;
    int minutes = secs/60;
    secs = secs%60;
    QAction action11(QString::fromUtf8("T.gian theo dõi: ")+QString::number(minutes)+"p"+QString::number(secs)+QString::fromUtf8("giây"), this);
    //connect(&action1, &QAction::triggered, this, &Mainwindow::removeTrack);
    contextMenu.addAction(&action11);

    contextMenu.exec((QCursor::pos()));
}
void Mainwindow::trackTableItemMenu(int row,int col)
{
    QTableWidgetItem* item =  ui->tableWidgetTarget->item(row,0);
    if(!item)return;
    int selectedTrackID = item->text().toInt();
    //mTargetMan.currTrackPt = mTargetMan.getTrackById(selectedTrackID);
    mTargetMan.setSelectedTrack(selectedTrackID);
    showTrackContext();

}
void Mainwindow::changeID()
{
    int value=1;
    DialogInputValue *dlg= new DialogInputValue(this,&value);
    dlg->exec();
    if(value<1)return;
    if(!mTargetMan.changeCurrTrackID(value))
    {
        QMessageBox msgBox;
        msgBox.setText(QString::fromUtf8("Số hiệu bị trùng!"));
        msgBox.exec();
    }
    else
    {
        if(C_primary_track::IDCounter<=value)C_primary_track::IDCounter = value+1;
    }
}
void Mainwindow::setEnemy()
{
    mTargetMan.setCurToEnemy();
}
void Mainwindow::setFriend()
{
    mTargetMan.setCurToFriend();
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
    if(mTargetMan.currTrackPt)
    {
        mTargetMan.currTrackPt->track->Remove();
        mTargetMan.currTrackPt->track = nullptr;
    }

}
void Mainwindow::addToTargets()
{
    QString error = mTargetMan.addCurrTrackToTargets();
    if(error.size())
    {
        QMessageBox msgBox;
        msgBox.setText(QString::fromUtf8("Lỗi:")+error);
        msgBox.exec();
    }
}
void Mainwindow::SetUpTheonGUILayout()
{
   ui->groupBox_gps_3->hide();
   ui->groupBox_statuses->setGeometry(1430,1165,480,30);
   ui->groupBox_25->setGeometry(10,1010,130,100);
   ui->groupBox_gps->setGeometry(10,1120,211,70);
   ui->groupBox_5->setGeometry(1430,10,160,100);
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
}
void Mainwindow::InitSetting()
{
    //hide iad
    ui->tabWidget_iad->setGeometry(200,-800,ui->tabWidget_iad->width(),ui->tabWidget_iad->height());
    ui->tabWidget_iad->hide();
    ui->tabWidget_iad->mMoveable = true;
    ui->tabWidget_iad->raise();
#ifdef THEON
    SetUpTheonGUILayout();
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
    qApp->setStyleSheet("QToolTip { color: #ffffff; background-color: #2a82da; border: 1px solid white; }");
    ui->tabWidget_iad->SetTransparent(true);
    QApplication::setOverrideCursor(Qt::CrossCursor);
    QString systemCommand = CConfig::getString("systemCommand","D:\\HR2D\\cudaFFT.exe");
    if(systemCommand.size()){
        systemCommand= "start "+systemCommand;
        system((char*)systemCommand.toStdString().data());}
    mMaxTapMayThu = CConfig::getInt("mMaxTapMayThu");
    mRangeIndex = CConfig::getInt("mRangeLevel");
    //assert(mRangeLevel>=0&&mRangeLevel<8);
    setDistanceUnit(CConfig::getInt("mDistanceUnit"));
    //assert(mDistanceUnit>=0&&mDistanceUnit<2);

    mHeadingT2 = CConfig::getDouble("mHeadingT2",0);
    mHeadingT = CConfig::getDouble("mHeadingT",0);
    mAziCorrecting = CConfig::getDouble("mAziCorrecting",0);
    //pRadar->setAziOffset(mHeadingT);
//    ui->textEdit_heading->setText(CConfig::getString("mHeadingT"));
//    ui->textEdit_heading_2->setText(CConfig::getString("mHeadingT2"));
    mZoomSizeAz = CConfig::getDouble("mZoomSizeAz",5);
    ui->textEdit_size_ar_a->setText(QString::number(mZoomSizeAz));
    mZoomSizeRg = CConfig::getDouble("mZoomSizeRg",2);
    ui->textEdit_size_ar_r->setText(QString::number(mZoomSizeRg));

    //load map
    osmap = new CMap();
    SetGPS(CConfig::getDouble("mLat"), CConfig::getDouble("mLon"));
    CConfig::mStat.mLat = mLat;
    CConfig::mStat.mLon = mLon;
    osmap->setCenterPos(mLat,mLon);
    osmap->setImgSize(SCR_H,SCR_H);
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
bool Mainwindow::CalcAziContour(double theta, int d)
{
    while (theta>=360.0)theta-=360.0;
    while(theta<0)theta+=360.0;
    double tanA = tan(theta/57.295779513);
    double sinA = sin(theta/57.295779513);
    double cosA = cos(theta/57.295779513);

    if(theta==0)
    {
        points[2].setX(scrCtX  - dx);
        points[2].setY(scrCtY - sqrt((d*d/4.0- dx*dx)));
        points[1].setX(points[2].x());
        points[1].setY(points[2].y()-5.0);
        points[0].setX(points[2].x());
        points[0].setY(points[2].y()-18.0);
    }
    else if(theta==180)
    {
        points[2].setX(scrCtX  - dx);
        points[2].setY(scrCtY + sqrt((d*d/4.0- dx*dx)));
        points[1].setX(points[2].x());
        points[1].setY(points[2].y()+5.0);
        points[0].setX(points[2].x());
        points[0].setY(points[2].y()+18.0);
    }
    else if (theta<180)
    {
        double a = (1.0+1.0/tanA/tanA);//4*(dy/tanA-dx)*(dy/tanA-dx) -4*(1+1/tanA)*(dx*dx+dy*dy-width()*width()/4);
        double b= 2.0*(dy/tanA - dx);
        double c= dx*dx+dy*dy-d*d/4.0;
        double delta = b*b-4.0*a*c;
        if(delta<30.0)return false;
        delta = sqrt(delta);
        int rx = (-b + delta)/2.0/a;
        int ry = -rx/tanA;
        if(abs(rx)<100&&abs(ry)<100)return false;
        points[2].setX(scrCtX + rx -dx);
        points[2].setY(scrCtY + ry-dy);
        points[1].setX(points[2].x()+5.0*sinA);
        points[1].setY(points[2].y()-5.0*cosA);
        points[0].setX(points[2].x()+18.0*sinA);
        points[0].setY(points[2].y()-18.0*cosA);
    }
    else
    {
        double a = (1+1.0/tanA/tanA);//4*(dy/tanA-dx)*(dy/tanA-dx) -4*(1+1/tanA)*(dx*dx+dy*dy-width()*width()/4);
        double b= 2.0*(dy/tanA - dx);
        double c= dx*dx+dy*dy-d*d/4.0;
        double delta = b*b-4.0*a*c;
        if(delta<30.0)return false;
        delta = sqrt(delta);
        int rx;
        int ry;
        rx =  (-b - delta)/2.0/a;
        ry = -rx/tanA;
        if(abs(rx)<100&&abs(ry)<100)return false;
        points[2].setX(scrCtX + rx - dx);
        points[2].setY(scrCtY + ry - dy);
        points[1].setX(points[2].x()+5.0*sinA);
        points[1].setY(points[2].y()-5.0*cosA);
        points[0].setX(points[2].x()+18.0*sinA);
        points[0].setY(points[2].y()-18.0*cosA);
    }
    return true;

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
            DrawGrid(p,radCtX,radCtY);
        }
    }
    //fill back ground
    p->setPen(QColor(24 ,48 ,64,255));
    p->setBrush(QBrush(QColor(24 ,48 ,64,255)));
    p->drawRect(SCR_H+SCR_LEFT_MARGIN,SCR_TOP_MARGIN,SCR_W-SCR_H-SCR_LEFT_MARGIN,SCR_H);
    //p->drawRect(0,0,SCR_LEFT_MARGIN,SCR_H);
    p->setBrush(Qt::NoBrush);
    p->setPen(penBackground);
    //    for (short i=60;i<650;i+=110)
    //    {
    //        p->drawEllipse(-i/2+(scrCtX-scrCtY)+25,-i/2+25,SCR_H -50+i,SCR_H -50+i);
    //    }

    p->drawEllipse(circleRect);
    p->setPen(penYellow);
    p->drawEllipse(ppiRect);
    p->setFont(QFont("Times", 10));
    //ve vanh goc ngoai
    for(short theta=0;theta<360;theta+=10)
    {
        if(CalcAziContour(theta+trueShiftDeg,SCR_H - SCR_BORDER_SIZE))
        {
            p->drawLine(points[1],points[2]);
            //if(!(theta%10))
                p->drawText(points[0].x()-25,points[0].y()-10,50,20,
                    Qt::AlignHCenter|Qt::AlignVCenter,
                    QString::number(theta));
        }
    }
    if(CalcAziContour((CConfig::mStat.antennaAziDeg)+trueShiftDeg,SCR_H-SCR_BORDER_SIZE-20))
    {
        p->setPen(QPen(Qt::red,4,Qt::SolidLine,Qt::RoundCap));
        p->drawLine(points[2],points[1]);
        //draw text

    }
#ifndef THEON
    //ve vanh goc trong
    p->setPen(penCyan);
    for(short theta=0;theta<360;theta+=10)
    {
        if(CalcAziContour(theta+headShift,SCR_H - SCR_BORDER_SIZE-40))
        {
            int value = theta;
            if(value>180)value-=360;
            if(!value)continue;
            p->drawLine(points[0],points[1]);
            p->drawText(points[2].x()-25,points[2].y()-10,50,20,
                    Qt::AlignHCenter|Qt::AlignVCenter,
                    QString::number(value));
        }
    }

    double radHeading;
    if(isHeadUp)
    {
        radHeading=0;
    }else radHeading = (CConfig::mStat.shipHeadingDeg);
    p->setPen(QPen(Qt::cyan,1,Qt::SolidLine,Qt::RoundCap));
    if(CalcAziContour(radHeading,SCR_H-SCR_BORDER_SIZE-18))
    {
        QPoint p1(radCtX+23*sin(radians(radHeading)),
                  radCtY-23*cos(radians(radHeading)));
        QPoint p2(radCtX+15*sin(radians(radHeading+30)),
                  radCtY-15*cos(radians(radHeading+30)));
        QPoint p3(radCtX+15*sin(radians(radHeading+150)),
                  radCtY-15*cos(radians(radHeading+150)));
        QPoint p4(radCtX+15*sin(radians(radHeading-150)),
                  radCtY-15*cos(radians(radHeading-150)));
        QPoint p5(radCtX+15*sin(radians(radHeading-30)),
                  radCtY-15*cos(radians(radHeading-30)));
        p->drawLine(p1,points[1]);
        p->drawLine(p1,p2);
        p->drawLine(p2,p3);
        p->drawLine(p3,p4);
        p->drawLine(p4,p5);
        p->drawLine(p5,p1);

    }





    //plot cur azi
    //    if(CalcAziContour(processing->mAntennaAzi,SCR_H-SCR_BORDER_SIZE-20))
    //    {
    //        p->setPen(QPen(Qt::red,4,Qt::SolidLine,Qt::RoundCap));
    //        p->drawLine(points[2],points[1]);
    //        //draw text
    //        //p->drawText(720,20,200,20,0,"Antenna: "+QString::number(aziDeg,'f',1));

    //    }
    if(CalcAziContour((CConfig::mStat.antennaAziDeg)+trueShiftDeg,SCR_H-SCR_BORDER_SIZE-20))
    {
        p->setPen(QPen(Qt::red,4,Qt::SolidLine,Qt::RoundCap));
        p->drawLine(points[2],points[1]);
        //draw text
        //p->drawText(720,20,200,20,0,"Antenna: "+QString::number(aziDeg,'f',1));

    }

#endif

}
//void Mainwindow::setScaleNM(unsigned short rangeNM)
//{
//    float oldScale = mScale;
//    mScale = (float)height()/((float)rangeNM*CONST_NM)*0.7f;
//    //printf("scale:%f- %d",scale,rangeNM);
//    isScaleChanged = true;// scale*SIGNAL_RANGE_KM/2048.0f;

//    dyMax = MAX_VIEW_RANGE_KM*mScale;
//    dxMax = dyMax;
//    dx =short(mScale/oldScale*dx);
//    dy =short(mScale/oldScale*dy);
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

    //clock_t ageVideo = clock()-pRadar->mUpdateTime;
    if(pRadar->UpdateData())
    {
        if(pRadar->isClkAdcChanged)
        {
            //ui->comboBox_radar_resolution->setCurrentIndex(pRadar->clk_adc);
            DisplayClkAdc(pRadar->clk_adc);
            pRadar->setScalePPI(mScale);
            this->UpdateScale();
            //            printf("\nsetScale:%d",pRadar->clk_adc);
            pRadar->isClkAdcChanged = false;
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
    processing = new dataProcessingThread();
    pRadar = processing->mRadarData;
    //init simulator
    simulator = new c_radar_simulation(processing->mRadarData);
    connect(this,SIGNAL(destroyed()),simulator,SLOT(deleteLater()));
    simulator->start(QThread::HighPriority);
    //
    connect(&syncTimer1s, SIGNAL(timeout()), this, SLOT(sync1S()));
    syncTimer1s.start(1000);
    connect(&syncTimer5p, SIGNAL(timeout()), this, SLOT(sync5p()));
    syncTimer5p.start(300000);
    //syncTimer1s.moveToThread(t);

    connect(&timerVideoUpdate, SIGNAL(timeout()), this, SLOT(UpdateVideo()));
    timerVideoUpdate.start(60);//ENVDEP
    //scrUpdateTimer.moveToThread(t2);
    //connect(t2,SIGNAL(finished()),t2,SLOT(deleteLater()));

    connect(this,SIGNAL(destroyed()),processing,SLOT(deleteLater()));
    connect(&dataPlaybackTimer,SIGNAL(timeout()),processing,SLOT(playbackRadarData()));
    processing->start(QThread::TimeCriticalPriority);
    tprocessing->start(QThread::HighPriority);

    connect(&timerMetaUpdate, SIGNAL(timeout()), this, SLOT(Update100ms()));
    timerMetaUpdate.start(100);//ENVDEP

}
void Mainwindow::Update100ms()
{

    //smooth the heading
    ui->label_head_ship->setText(QString::number(CConfig::mStat.shipHeadingDeg,'f',1));
    ui->label_course_ship->setText(QString::number(CConfig::mStat.shipCourseDeg,'f',1));
    ui->label_speed_ship->setText(QString::number(CConfig::mStat.shipSpeed,'f',1));
    /*double headingDiff = CConfig::shipHeadingDeg-CConfig::mStat.shipHeadingDeg;
    if(abs(headingDiff)>0.5)
    {
        if(headingDiff<-180)headingDiff+=360;
        if(headingDiff>180)headingDiff-=360;
        CConfig::mStat.shipHeadingDeg+=headingDiff/3.0;
        isMapOutdated = true;
    }else CConfig::mStat.shipHeadingDeg = CConfig::shipHeadingDeg;*/

    //calculate heading
    if(isHeadUp)
    {
        trueShiftDeg = -CConfig::mStat.shipHeadingDeg;
        headShift = 0;
//        pRadar->setAziViewOffsetDeg(trueShiftDeg);
        mTrans.reset();
        mTrans = mTrans.rotate((-CConfig::mStat.shipHeadingDeg));
    }
    else
    {
        trueShiftDeg = 0;
        headShift = CConfig::mStat.shipHeadingDeg;
    }

    DrawMap();
    mMousex=this->mapFromGlobal(QCursor::pos()).x();
    mMousey=this->mapFromGlobal(QCursor::pos()).y();
    CConfig::mStat.antennaAziDeg = degrees(pRadar->getCurAziRad());

    if(pRadar->init_time)
    {
        ui->label_azi_antenna_head_true->setText(QString::number(int(CConfig::mStat.antennaAziDeg)));
    }
    else
    {
        ui->label_azi_antenna_head_true->setText(QString::number(CConfig::mStat.antennaAziDeg,'f',1));
    }
    if(isInsideViewZone(mMousex,mMousey))
    {
        if(mouse_mode&MouseAutoSelect1||mouse_mode&MouseAutoSelect2)QApplication::setOverrideCursor(Qt::DragMoveCursor);
        else QApplication::setOverrideCursor(Qt::CrossCursor);
        double azi,rg;
        if(ui->toolButton_measuring->isChecked())
        {
            C_radar_data::kmxyToPolarDeg((mMousex - mMouseLastX)/mScale,-(mMousey - mMouseLastY)/mScale,&azi,&rg);

        }
        else
        {
            C_radar_data::kmxyToPolarDeg((mMousex - radCtX)/mScale,-(mMousey - radCtY)/mScale,&azi,&rg);
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
        ui->label_cursor_lat->setText(demicalDegToDegMin( y2lat(-(mMousey - radCtY)))+"'N");
        ui->label_cursor_long->setText(demicalDegToDegMin(x2lon(mMousex - radCtX))+"'E");
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
{

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
    //config.SaveToFile();
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
void Mainwindow::sync5p()//period 10 second
{

    if(radar_state!=DISCONNECTED)
    {
        QFile logFile;
        QDateTime now = QDateTime::currentDateTime();
        if(!QDir("D:\\logs\\"+now.toString("\\dd.MM\\")).exists())
        {
            QDir().mkdir("D:\\logs\\"+now.toString("\\dd.MM\\"));
        }
        logFile.setFileName("D:\\logs\\"+now.toString("\\dd.MM\\")+now.toString("dd.MM-hh.mm.ss")+"_radar_online.log");
        logFile.open(QIODevice::WriteOnly);

        logFile.close();

    }

}
void Mainwindow::updateTargetInfo()
{
    if(selectedTargetType==RADAR)
    {/*
        trackList* trackListPt = &pRadar->mTrackList;
        for(uint trackId=0;trackId<trackListPt->size();trackId++)
        {

            if(!trackListPt->at(trackId).isConfirmed)continue;
            if(selectedTargetIndex == trackId)
            {
                //printf("\ntrackId:%d",trackId);
                double mLat,mLon;
                this->ConvKmToWGS(trackListPt->at(trackId).estX*pRadar->scale_ppi/mScale,trackListPt->at(trackId).estY*pRadar->scale_ppi/mScale,&mLon,&mLat);
                //ui->label_data_id->setText(QString::number(trackListPt->at(trackId).idCount));
                float tmpazi = trackListPt->at(trackId).estA*DEG_RAD;
                if(tmpazi<0)tmpazi+=360;
                //ui->label_data_type->setText("Radar");
                ui->label_data_range->setText(QString::number(trackListPt->at(trackId).estR*pRadar->scale_ppi/mScale/1.852f,'f',2)+"Nm");
                ui->label_data_azi->setText( QString::number(tmpazi,'f',2)+degreeSymbol);
                ui->label_data_lat->setText( QString::number((short)mLat)+degreeSymbol+QString::number((mLat-(short)mLat)*60,'f',2)+"'N");
                ui->label_data_long->setText(QString::number((short)mLon)+degreeSymbol+QString::number((mLon-(short)mLon)*60,'f',2)+"'E");
                ui->label_data_speed->setText(QString::number(trackListPt->at(trackId).speed,'f',2)+"Kn");
                ui->label_data_heading->setText(QString::number(trackListPt->at(trackId).heading*DEG_RAD)+degreeSymbol);
                // ui->label_data_dopler->setText(QString::number(trackListPt->at(trackId).dopler));
            }
        }*/

    }
    else if(selectedTargetType == AIS){
        /*
    C2_Track *selectedTrack = &processing->m_AISList.at(selectedTargetIndex);
    double azi,rg;
    double fx,fy;
    ConvWGSToKm(&fx,&fy,selectedTrack->getLon(),selectedTrack->getLat());
    C_radar_data::kmxyToPolarDeg(fx,fy,&azi,&rg);
    ui->label_data_id->setText(QString::fromUtf8((char*)(&selectedTrack->m_MMSI),9));
    ui->label_data_range->setText(QString::number(rg,'f',2));
    ui->label_data_azi->setText(QString::number(azi,'f',2));
    ui->label_data_type->setText("AIS");
    ui->label_data_lat->setText( QString::number((short)selectedTrack->getLat())+degreeSymbol+QString::number((selectedTrack->getLat()-(short)selectedTrack->getLat())*60,'f',2)+"N");
    ui->label_data_long->setText(QString::number((short)selectedTrack->getLon())+degreeSymbol+QString::number((selectedTrack->getLon()-(short)selectedTrack->getLon())*60,'f',2)+"E");
    ui->label_data_speed->setText(QString::number(selectedTrack->m_Speed,'f',2)+"Kn");
    ui->label_data_heading->setText(QString::number(selectedTrack->getHead()*DEG_RAD)+degreeSymbol);
    */}
    else if(selectedTargetType==NOTARGET)
    {
        //ui->label_data_id->setText("--");
        //ui->label_data_type->setText("--");
        ui->label_data_range->setText("--");
        ui->label_data_azi->setText( "--");
        ui->label_data_lat->setText( "--");
        ui->label_data_long->setText("--");
        ui->label_data_speed->setText("--");
        ui->label_data_heading->setText("--");
        //ui->label_data_dopler->setText("--");
    }
}
void Mainwindow::autoSwitchFreq()
{
    int newFreq = rand()%6;
    if(newFreq==0)
    {
        ui->toolButton_tx_2->setChecked(true);
    }
    else  if(newFreq==1)
    {
        ui->toolButton_tx_3->setChecked(true);
    }
    else if(newFreq==2)
    {
        ui->toolButton_tx_4->setChecked(true);
    }
    else if(newFreq==3)
    {
        ui->toolButton_tx_5->setChecked(true);
    }
    else if(newFreq==4)
    {
        ui->toolButton_tx_6->setChecked(true);
    }
    else if(newFreq==5)
    {
        ui->toolButton_tx_7->setChecked(true);
    }


}//label_data_range_2
void Mainwindow::UpdateMay22Status()
{
    clock_t ageMay22 = CConfig::mStat.getAge22();
    if(ageMay22<3000)
    {
        if(CConfig::mStat.mMayPhatOK)
        {
            ui->label_status_may_22->setStyleSheet("color: rgb(10, 255, 10);");
            ui->label_status_may_22->setText(QString::fromUtf8("Máy phát hoạt động bình thường"));
        }
        else
        {
            ui->label_status_may_22->setStyleSheet("color: rgb(255, 10, 10);");
            ui->label_status_may_22->setText(QString::fromUtf8("Máy phát không hoạt động"));
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
        if(CConfig::mStat.mCaoApKetNoi==0)ui->toolButton_dk_15->setChecked(true);//cao ap
        else if(CConfig::mStat.mCaoApKetNoi==1)ui->toolButton_dk_10->setChecked(true);//cao ap
        else if(CConfig::mStat.mCaoApKetNoi==2)ui->toolButton_dk_14->setChecked(true);//cao ap
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
    for(uint i =0;i<pRadar->mTrackList.size();i++)
    {
        C_primary_track* track = &(pRadar->mTrackList[i]);
        if(track->isConfirmed())
        if(!mTargetMan.checkIDExist(pRadar->mTrackList[i].uniqId))
            mTargetMan.addTrack(&pRadar->mTrackList[i]);
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

    }
    if(row<(ui->tableWidgetTarget->rowCount()-1))ui->tableWidgetTarget->setRowCount(row+1);
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
                else if (col==2)item->setText(QString::number(nm(trackPt->track->rgKm),'f',2));
                else if (col==3)item->setText(QString::number(trackPt->track->courseDeg,'f',1));
                else if (col==4)item->setText(QString::number(nm(trackPt->track->mSpeedkmhFit),'f',1));
                //ui->tableWidgetTarget_2->setItem(i,col,item);
            }

        }

    }
    /*
    int numOfTracks = 0;
    bool selectionExist = false;
    for (uint i = 0;i<pRadar->mTrackList.size();i++)
    {
        track_t* track = &(pRadar->mTrackList[i]);
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

    UpdateMay22Status();
    UpdateGpsData();
    ViewTrackInfo();
    // update rate
    int sampleTime = 10*paintTime/7;
    if(sampleTime<20)sampleTime=20;
    ui->label_frame_rate->setText("SFR:"+QString::number(1000/sampleTime));

    timerVideoUpdate.start(sampleTime);
    timerMetaUpdate.start(sampleTime*4);

    ui->label_radar_fps->setText("RFR:"+QString::number(int(processing->mFramesPerSec)));
    //target manager
    if(ui->toolButton_chi_thi_mt->isChecked())mTargetMan.OutputTargetToKasu();
    if(isScaleChanged ) {

        pRadar->setScalePPI(mScale);
        isScaleChanged = false;
    }
    ui->label_speed_2->setText(QString::number(pRadar->rotation_per_min,'f',1)+"v/p");
    showTime();
    return;

    ui->label_debug_data->setText("Chu ky: "+QString::number(pRadar->chu_ky));
    unsigned int chuKy = 1000000/(pRadar->chu_ky*(pow(2,pRadar->clk_adc))/10.0);

    ui->label_sn_freq->setText(QString::number(chuKy));
    /*ui->label_he_so_tap->setText(QString::fromUtf8("Hệ số tạp: ")+QString::number(pRadar->get_tb_tap()));
    if(ui->toolButton_auto_freq->isChecked())
    {
        if(pRadar->get_tb_tap()>mMaxTapMayThu)
        {
            this->autoSwitchFreq();
        }
    }*/
    int value;
    switch((pRadar->sn_stat>>8)&0x07)
    {
    case 4:
        ui->label_sn_type->setText("DTTT");
        //ui->label_sn_param->setText(QString::number(32<<());
        if(((pRadar->sn_stat)&0xff)==0)
        {
            ui->label_sn_param->setText("32");
        }
        else if(((pRadar->sn_stat)&0xff)==1)
        {
            ui->label_sn_param->setText("48");
        }
        else if(((pRadar->sn_stat)&0xff)==2)
        {
            ui->label_sn_param->setText("64");
        }
        else if(((pRadar->sn_stat)&0xff)==3)
        {
            ui->label_sn_param->setText("96");
        }
        else if(((pRadar->sn_stat)&0xff)==4)
        {
            ui->label_sn_param->setText("128");
        }
        else if(((pRadar->sn_stat)&0xff)==5)
        {
            ui->label_sn_param->setText("192");

        }
        else
        {
            ui->label_sn_param->setText("256");
        }
        break;
    case 0:
        ui->label_sn_type->setText(QString::fromUtf8("Xung đơn"));
        ui->label_sn_param->setText(QString::number((((pRadar->sn_stat)&0xff))));
        break;
    case 2:
        ui->label_sn_type->setText(QString::fromUtf8("Giả liên tục"));
        value = ((pRadar->sn_stat)&0xff);
        if(value<5)
        {
            ui->label_sn_param->setText(QString::number(16*pow(2,value)));

            ui->label_sn_pulsew->setText(QString::number(((16*pow(2,value))*(pow(2,pRadar->clk_adc))/10.0),'f',1));
        }
        else
        {
            ui->label_sn_param->setText(QString::number(16*pow(2,value-5))+"x2");
            ui->label_sn_pulsew->setText(QString::number(((32*(value-5))*(pow(2,pRadar->clk_adc))/10.0),'f',1));
        }

        break;
    case 3:
        ui->label_sn_type->setText(QString::fromUtf8("Mã 2 pha"));//todo:change back to "ma ngau nhien" later
        value = ((pRadar->sn_stat)&0xff);
        ui->label_sn_param->setText(QString::number(value-15.0));
        ui->label_sn_pulsew->setText(QString::number(((value-15.0)*(pow(2,pRadar->clk_adc))/10.0),'f',1));
        break;
    case 1:
        ui->label_sn_type->setText(QString::fromUtf8("Mã xung"));
        ui->label_sn_param->setText(QString::number((((pRadar->sn_stat)&0xff))));
        break;
    default:
        ui->label_sn_param->setText(QString::number(((pRadar->sn_stat)&0xff)));
        break;
    }
    /*switch((pRadar->rotation_speed)&0x07)
            {
            case 0:
                ui->label_speed->setText(QString::fromUtf8("Dừng quay"));break;
            case 1:
                ui->label_speed->setText("5 v/p");break;
            case 2:
                ui->label_speed->setText("8 v/p");break;
            case 3:
                ui->label_speed->setText("12 v/p");break;
            case 4:
                ui->label_speed->setText("15 v/p");break;
            case 5:
                ui->label_speed->setText("18 v/p");break;
            default:

                break;
            }*/




}
void Mainwindow::setRadarState(radarSate radarState)
{
    if(radar_state!=radarState)
    {
        radar_state = radarState;
        on_label_status_warning_clicked();
    }
}





void Mainwindow::on_actionOpen_rec_file_triggered()
{
    QString filename = QFileDialog::getOpenFileName(this,    tr("Open signal file"), NULL, tr("HR signal record files (*.r2d)"));
    if(!filename.size())return;
    processing->loadRecordDataFile(filename);
    ui->label_record_file_name->setText(filename);
}



void Mainwindow::on_actionOpen_map_triggered()
{
    //openShpFile();
}
void Mainwindow::showTime()
{
    QDateTime time = QDateTime::currentDateTime();
    QString text = time.toString("hh:mm:ss");
    ui->label_date->setText(text);
    text = time.toString("dd/MM/yy");
    ui->label_time->setText(text);
}

void Mainwindow::on_actionSaveMap_triggered()
{
    //vnmap.SaveBinFile();
}

void Mainwindow::on_actionSetting_triggered()
{
    //    GPSDialog *dlg = new GPSDialog(this);
    //    dlg->setModal(false);
    //    dlg->loadConfig(&config);
    //    dlg->show();
    //    dlg->setAttribute(Qt::WA_DeleteOnClose);
    //    connect(dlg, SIGNAL(destroyed(QObject*)), SLOT(UpdateSetting()));
    //    connect(dlg, SIGNAL(destroyed(QObject*)), SLOT(setCodeType()));
}
void Mainwindow::on_actionAddTarget_toggled(bool arg1)
{
    //isAddingTarget=arg1;

}




void Mainwindow::on_actionClear_data_triggered()
{
    pRadar->resetData();
    //    isScreenUp2Date = false;
}

//void Mainwindow::on_actionView_grid_triggered(bool checked)
//{
//    gridOff = checked;
//    dx=0;dy=0;
//    DrawMap();
//    //UpdateSetting();
//}


void Mainwindow::on_actionPlayPause_toggled(bool arg1)
{
    processing->togglePlayPause(arg1);
    if(arg1)dataPlaybackTimer.start(25);else dataPlaybackTimer.stop();

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

                  if(pRadar->checkFeedback(&command_queue.front().bytes[0]))// check if the radar has already recieved the command
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

void Mainwindow::on_actionRecording_triggered()
{

}


//void Mainwindow::on_comboBox_temp_type_currentIndexChanged(int index)
//{

//}

//void RadarGui::on_horizontalSlider_brightness_actionTriggered(int action)
//{

//}

void Mainwindow::on_horizontalSlider_brightness_valueChanged(int value)
{
    pRadar->brightness = 0.5f+(float)value/ ui->horizontalSlider_brightness->maximum()*4.0f;
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

void Mainwindow::on_actionSector_Select_triggered()
{

}


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
        mScale = (SCR_H-SCR_BORDER_SIZE)/(rangeRatio*srange )/2;
        ringStep = srange/4;
        ui->label_range->setText(QString::number(srange)+strDistanceUnit);
    }
    else if(mDistanceUnit==1)
    {
        mScale = (SCR_H-SCR_BORDER_SIZE)/(rangeRatio*srange )/2;
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
    float oldScale = mScale;
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
    short sdx = mZoomCenterx - radCtX;
    short sdy = mZoomCentery - radCtY;
    sdx =(sdx*mScale/oldScale);
    sdy =(sdy*mScale/oldScale);
    mZoomCenterx = scrCtX+sdx-dx;
    mZoomCentery = scrCtY+sdy-dy;
}




//void MainWindow::on_toolButton_10_toggled(bool checked)
//{

//}

//void MainWindow::on_actionRotateStart_toggled(bool arg1)
//{
//    if(arg1)
//    {
//        Command_Control new_com;
//        new_com.bytes[0] = 0xaa;
//        new_com.bytes[1] = 0xab;
//        new_com.bytes[2] = 0x03;
//        new_com.bytes[3] = 0x02;
//        new_com.bytes[4] = 0x00;
//        new_com.bytes[5] = 0x00;
//        new_com.bytes[6] = 0x00;
//        new_com.bytes[7] = 0;//new_com.bytes[0]+new_com.bytes[1]+new_com.bytes[2]+new_com.bytes[3]+new_com.bytes[4]+new_com.bytes[5]+new_com.bytes[6];
//        command_queue.push(new_com);
//    }
//    else
//    {

//        Command_Control new_com;
//        new_com.bytes[0] = 0xaa;
//        new_com.bytes[1] = 0xab;
//        new_com.bytes[2] = 0x03;
//        new_com.bytes[3] = 0x00;
//        new_com.bytes[4] = 0x00;
//        new_com.bytes[5] = 0x00;
//        new_com.bytes[6] = 0x00;
//        new_com.bytes[7] = 0;//new_com.bytes[0]+new_com.bytes[1]+new_com.bytes[2]+new_com.bytes[3]+new_com.bytes[4]+new_com.bytes[5]+new_com.bytes[6];
//        command_queue.push(new_com);
//    }
//}


//void MainWindow::on_comboBox_temp_type_2_currentIndexChanged(int index)
//{



//}

//void MainWindow::on_toolButton_11_toggled(bool checked)
//{


//}

//void MainWindow::on_pushButton_removeTarget_2_clicked()
//{

//}

//void MainWindow::on_pushButton_removeTarget_2_released()
//{
//    pRadar->resetTrack();
//}

//void MainWindow::on_pushButton_avtodetect_toggled(bool checked)
//{
//    isDrawSubTg = !checked;
//    pRadar->avtodetect = checked;
//    pRadar->terrain_init_time = 3;
//}


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
    pRadar->kgain = 8-float(value)/(ui->horizontalSlider_gain->maximum())*8;
    ui->label_gain->setText("Gain:"+QString::number(-pRadar->kgain,'f',2));
    //printf("pRadar->kgain %f \n",pRadar->kgain);
}

void Mainwindow::on_horizontalSlider_rain_valueChanged(int value)
{
    pRadar->krain = (float)value/(ui->horizontalSlider_rain->maximum());
    ui->label_rain->setText("Rain:" + QString::number(pRadar->krain,'f',2));
}

void Mainwindow::on_horizontalSlider_sea_valueChanged(int value)
{
    pRadar->ksea = (float)value/(ui->horizontalSlider_sea->maximum());
    ui->label_sea->setText("Sea:" + QString::number(pRadar->ksea,'f',2));
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
//    pRadar->setAutorgs(checked);
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


void Mainwindow::on_toolButton_replay_fast_toggled(bool checked)
{
    if(checked)
    {
        processing->playRate = 200;
    }else
    {
        processing->playRate = 50;
    }
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
    radCtX = scrCtX-dx;
    radCtY = scrCtY-dy;
    isMapOutdated = true;
}
void Mainwindow::on_toolButton_centerView_clicked()
{
    gotoCenter();
    //    isScreenUp2Date = false;
}

void Mainwindow::on_comboBox_currentIndexChanged(int index)
{
    switch (index)
    {
    case 0:
        pRadar->dataOver = m_only;
        break;
    case 1:
        pRadar->dataOver = s_m_150;
        break;
    case 2:
        pRadar->dataOver = s_m_200;
        break;
    case 3:
        pRadar->dataOver = max_s_m_150;
        break;
    case 4:
        pRadar->dataOver = max_s_m_200;
        break;
    default:
        break;
    }

}

void Mainwindow::on_comboBox_img_mode_currentIndexChanged(int index)
{
    pRadar->imgMode = imgDrawMode(index) ;
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
    case 0:
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
    if(processing->mRadMode<comm.size())
        commandString = comm.at(processing->mRadMode);
    sendToRadarString(commandString);

}
void Mainwindow::on_toolButton_zoom_out_clicked()
{
    if(mRangeIndex <7) mRangeIndex+=1;
    CConfig::setValue("mRangeLevel",mRangeIndex);
    UpdateScale();
    isMapOutdated = true;
}

//void Mainwindow::on_toolButton_reset_clicked()
//{
//    pRadar->resetSled();
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

void Mainwindow::SetGPS(double lat,double lon)
{
    mLat = lat;
    mLon = lon;
    //    if(heading!=0)
    //    {
    //        mHeadingGPSNew = heading;
    //    }
    CConfig::setValue("mLat",mLat);
    CConfig::setValue("mLon",mLon);
    //CConfig::setValue("mHeadingGPS",heading);
    ui->label_gps_lat->setText(demicalDegToDegMin(lat)+"'N");
    ui->label_gps_lon->setText(demicalDegToDegMin(lon)+"'E");
    //ui->label_gps_heading->setText(QString::number(mHeadingGPSNew,'f',2));
    //ui->label_azi_heading_gps->setText(QString::number(mHeadingGPSNew,'f',2));
    osmap->setCenterPos(mLat, mLon);
    isMapOutdated = true;
    repaint();
}

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
//    pRadar->setAziOffset(mHeadingT);

//}

void Mainwindow::on_toolButton_gps_update_clicked()
{

    SetGPS(ui->text_latInput_2->text().toFloat(),ui->text_longInput_2->text().toFloat());

}




//void Mainwindow::on_toolButton_centerZoom_clicked()
//{
//    pRadar->updateZoomRect(mousePointerX - radCtX,mousePointerY - radCtY);
//}

void Mainwindow::on_toolButton_xl_dopler_clicked()
{

}

//void Mainwindow::on_toolButton_xl_dopler_toggled(bool checked)
//{
//    pRadar->gat_mua_dopler = checked;
//}


//void Mainwindow::on_toolButton_xl_nguong_3_toggled(bool checked)
//{
//    pRadar->noise_nornalize = checked;
//}

//void Mainwindow::on_groupBox_3_currentChanged(int index)
//{

//}

void Mainwindow::on_toolButton_xl_dopler_2_toggled(bool checked)
{
    pRadar->bo_bang_0 = checked;
}



//void Mainwindow::on_toolButton_reset_3_clicked()
//{
//    pRadar->resetTrack();
//    //    for(short i = 0;i<targetDisplayList.size();i++)
//    //    {
//    //        targetDisplayList.at(i)->deleteLater();
//    //    }
//    //    targetDisplayList.clear();
//}

//void Mainwindow::on_toolButton_reset_2_clicked()
//{
//    pRadar->resetSled();
//}
//void Mainwindow::on_toolButton_vet_clicked(bool checked)
//{
//    pRadar->isSled = checked;
//}

void Mainwindow::on_label_status_warning_clicked()
{
    if(warningList.size())warningList.removeAt(warningList.size()-1);
    if(warningList.size())
    {
        ui->label_status_warning->setText(warningList.at(warningList.size()-1));
    }
    else
    {
        ui->label_status_warning->setText(QString::fromUtf8("Không cảnh báo"));
        ui->label_status_warning->setStyleSheet("background-color: rgb(20, 40, 60,255);");
    }
}

//void Mainwindow::on_toolButton_delete_target_clicked()
//{
//    /*if(targetList.at(selected_target_index)->isLost)
//            {
//                targetList.at(selected_target_index)->hide();
//            }

//            else*/
//    //    pRadar->mTrackList.at(targetDisplayList.at(selected_target_index)->trackId).isManual = false;
//}

void Mainwindow::on_toolButton_tx_clicked()
{
    //processing->radTxOn();
    sendToRadarString(CConfig::getString("mTxCommand"));
}


void Mainwindow::on_toolButton_tx_off_clicked()
{
    //processing->radTxOff();
    sendToRadarString(CConfig::getString("mRxCommand"));
}

//void Mainwindow::on_toolButton_filter2of3_clicked(bool checked)
//{
//    pRadar->filter2of3 = checked;
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
    mMouseLastX = radCtX;
    mMouseLastY = radCtY;
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
    pRadar->data_export = checked;
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
    processing->m_aisList.clear();
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
        pRadar->SelfRotationOn(rate);
    }
    else
        pRadar->SelfRotationOff();
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
//    pRadar->setIsSharpEye(checked);
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



void Mainwindow::on_toolButton_tx_2_clicked()
{
    sendToRadarString(CConfig::getString("mFreq1Command"));

}

void Mainwindow::on_toolButton_tx_3_clicked()
{
    sendToRadarString(CConfig::getString("mFreq2Command"));
}

void Mainwindow::on_toolButton_tx_4_clicked()
{
    sendToRadarString(CConfig::getString("mFreq3Command"));
}

void Mainwindow::on_toolButton_tx_5_clicked()
{
    sendToRadarString(CConfig::getString("mFreq4Command"));
}

void Mainwindow::on_toolButton_tx_6_clicked()
{
    sendToRadarString(CConfig::getString("mFreq5Command"));
}

void Mainwindow::on_toolButton_tx_7_clicked()
{
    sendToRadarString(CConfig::getString("mFreq6Command"));
}

void Mainwindow::on_toolButton_gps_update_auto_clicked()
{
    UpdateGpsData();
}

void Mainwindow::UpdateGpsData()
{
    if(CConfig::mStat.getAgeGps()<3000)
    {
        SetGPS(CConfig::mStat.mLat, CConfig::mStat.mLon);
    }
    else
    {

    }
    /*if(processing->mGpsData.size())
    {

        GPSData data = processing->mGpsData.back();
        if(processing->mGpsData.size()>10)processing->mGpsData.pop();
        SetGPS(data.lat, data.lon);
        clock_t gpsAge = clock()-CConfig::mStat.cGpsUpdateTime;
        //CConfig::mStat.cAisAge = gpsAge;
        if(gpsAge<3000)   ui->groupBox_gps->setTitle(QString::fromUtf8("GPS"));
        else              ui->groupBox_gps->setTitle(QString::fromUtf8("GPS(mất kết nối ")+QString::number(gpsAge/1000)+")");
    }*/
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



/*void Mainwindow::on_toolButton_set_command_clicked()
{
    CConfig::getString("mR0Command") = ui->plainTextEdit_range_0->toPlainText();
    CConfig::getString("mR1Command") = ui->plainTextEdit_range_1->toPlainText();
    CConfig::getString("mR2Command") = ui->plainTextEdit_range_2->toPlainText();
    CConfig::getString("mR3Command") = ui->plainTextEdit_range_3->toPlainText();
    CConfig::getString("mR4Command") = ui->plainTextEdit_range_4->toPlainText();
    CConfig::getString("mR5Command") = ui->plainTextEdit_range_5->toPlainText();
    CConfig::getString("mR6Command") = ui->plainTextEdit_range_6->toPlainText();
    CConfig::getString("mR7Command") = ui->plainTextEdit_range_7->toPlainText();
    CConfig::getString("mRxCommand") = ui->plainTextEdit_command_rx->toPlainText();
    CConfig::getString("mTxCommand") = ui->plainTextEdit_command_tx->toPlainText();
    CConfig::setValue("CConfig::getString("mR0Command")",CConfig::getString("mR0Command"));
    CConfig::setValue("CConfig::getString("mR1Command")",CConfig::getString("mR1Command"));
    CConfig::setValue("CConfig::getString("mR2Command")",CConfig::getString("mR2Command"));
    CConfig::setValue("CConfig::getString("mR3Command")",CConfig::getString("mR3Command"));
    CConfig::setValue("CConfig::getString("mR4Command")",CConfig::getString("mR4Command"));
    CConfig::setValue("CConfig::getString("mR5Command")",CConfig::getString("mR5Command"));
    CConfig::setValue("CConfig::getString("mR6Command")",CConfig::getString("mR6Command"));
    CConfig::setValue("CConfig::getString("mR7Command")",CConfig::getString("mR7Command"));
    CConfig::setValue("CConfig::getString("mRxCommand")",CConfig::getString("mRxCommand"));
    CConfig::setValue("CConfig::getString("mTxCommand")",CConfig::getString("mTxCommand"));
    //
    CConfig::getString("mFreq1Command") =  ui->plainTextEdit_freq_1->toPlainText();
    CConfig::getString("mFreq2Command") =  ui->plainTextEdit_freq_2->toPlainText();
    CConfig::getString("mFreq3Command") =  ui->plainTextEdit_freq_3->toPlainText();
    CConfig::getString("mFreq4Command") =  ui->plainTextEdit_freq_4->toPlainText();
    CConfig::getString("mFreq5Command") =  ui->plainTextEdit_freq_5->toPlainText();
    CConfig::getString("mFreq6Command") =  ui->plainTextEdit_freq_6->toPlainText();
    //
    CConfig::setValue("CConfig::getString("mFreq1Command")",CConfig::getString("mFreq1Command"));
    CConfig::setValue("CConfig::getString("mFreq2Command")",CConfig::getString("mFreq2Command"));
    CConfig::setValue("CConfig::getString("mFreq3Command")",CConfig::getString("mFreq3Command"));
    CConfig::setValue("CConfig::getString("mFreq4Command")",CConfig::getString("mFreq4Command"));
    CConfig::setValue("CConfig::getString("mFreq5Command")",CConfig::getString("mFreq5Command"));
    CConfig::setValue("CConfig::getString("mFreq6Command")",CConfig::getString("mFreq6Command"));
    ui->groupBox_control_setting->setHidden(true);
    ui->toolButton_set_commands->setChecked(false);

}
*/

void Mainwindow::on_toolButton_auto_freq_toggled(bool checked)
{
    if(checked) autoSwitchFreq();
}

void Mainwindow::on_toolButton_set_default_clicked()
{
    CConfig::setDefault();
}



//void Mainwindow::on_toolButton_heading_update_clicked()
//{
//    /*if(processing->isHeadingAvaible)
//    {
//        mHeadingT = processing->getHeading()+CConfig::getDouble("mHeadingT3");
//        if(mHeadingT>=360)mHeadingT-=360;
//        ui->label_compass_value->setText(QString::number(processing->getHeading(),'f',1));
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
    pRadar->isShowSled=checked;
    CConfig::setValue("isShowSled",int(checked));
}

void Mainwindow::on_toolButton_sled_reset_clicked()
{
    pRadar->resetSled();
}



void Mainwindow::on_toolButton_dobupsong_toggled(bool checked)
{
    pRadar->is_do_bup_song = checked;
    if(checked)
    {
        pRadar->setTb_tap_k(ui->textEdit_dobupsongk->text().toDouble());

    }
}


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
        cmLog->show();
    }
    else
    {
        cmLog->hide();
    }
}

void Mainwindow::on_toolButton_exit_2_clicked()
{
    mstatWin = new StatusWindow(processing);

    mstatWin->show();
}

void Mainwindow::on_toolButton_selfRotation_2_toggled(bool checked)
{
    //pRadar->isEncoderAzi  = checked;
    if(checked)
    {
        double rate = ui->lineEdit_selfRotationRate->text().toDouble();
        //        rate = rate/MAX_AZIR;
        pRadar->SelfRotationOn(rate);
    }
    else
        pRadar->SelfRotationOff();
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
        processing->sendCommand(commandMay22,12,false);
    }
}

void Mainwindow::on_toolButton_dk_17_toggled(bool checked)
{
    return;
    if(checked)
    {
        commandMay22[7]=0x02;
        processing->sendCommand(commandMay22,12,false);
    }
}

void Mainwindow::on_toolButton_grid_toggled(bool checked)
{
    toolButton_grid_checked = checked;
}

void Mainwindow::on_toolButton_dk_4_clicked()
{

    commandMay22[6]=0x06;
    processing->sendCommand(commandMay22,12,false);
}

void Mainwindow::on_toolButton_dk_3_clicked()
{
    commandMay22[6]=0x00;
    processing->sendCommand(commandMay22,12,false);
}

void Mainwindow::on_toolButton_dk_5_clicked()
{
    commandMay22[6]=0x05;
    processing->sendCommand(commandMay22,12,false);
}

void Mainwindow::on_toolButton_dk_6_clicked()
{
    commandMay22[6]=0x04;
    processing->sendCommand(commandMay22,12,false);
}

void Mainwindow::on_toolButton_dk_7_clicked()
{
    commandMay22[6]=0x03;
    processing->sendCommand(commandMay22,12,false);
}

void Mainwindow::on_toolButton_dk_8_clicked()
{
    commandMay22[6]=0x02;
    processing->sendCommand(commandMay22,12,false);
}

void Mainwindow::on_toolButton_dk_9_clicked()
{
    commandMay22[6]=0x01;
    processing->sendCommand(commandMay22,12,false);
}

void Mainwindow::on_toolButton_dk_2_clicked()
{
    commandMay22[5]=0x00;
    processing->sendCommand(commandMay22,12,false);
}

void Mainwindow::on_toolButton_dk_13_clicked()
{
    commandMay22[5]=0x01;
    processing->sendCommand(commandMay22,12,false);
}

void Mainwindow::on_toolButton_dk_12_clicked()
{
    commandMay22[4]=0x01;
    processing->sendCommand(commandMay22,12,false);
}

void Mainwindow::on_toolButton_dk_10_clicked()
{
    commandMay22[8]=0x01;
    processing->sendCommand(commandMay22,12,false);
}

void Mainwindow::on_toolButton_dk_14_clicked()
{
    commandMay22[8]=0x02;
    processing->sendCommand(commandMay22,12,false);
}

void Mainwindow::on_toolButton_dk_15_clicked()
{
    commandMay22[8]=0x00;
    processing->sendCommand(commandMay22,12,false);
}

void Mainwindow::on_toolButton_sled_time25_clicked()
{
    pRadar->mSledValue = 180;
}

void Mainwindow::on_toolButton_sled_time8_clicked()
{
    pRadar->mSledValue = 50;
}

void Mainwindow::on_toolButton_sled_time3_clicked()
{
    pRadar->mSledValue = 10;
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
    pRadar->noise_nornalize = checked;
}

void Mainwindow::on_toolButton_xl_nguong_4_clicked(bool checked)
{
    pRadar->setAutorgs(checked);
}

void Mainwindow::on_toolButton_sled_clicked(bool checked)
{
    pRadar->isShowSled=checked;
}

void Mainwindow::on_toolButton_xl_dopler_clicked(bool checked)
{
    pRadar->gat_mua_va_dia_vat = checked;
    CConfig::setValue("gat_mua_dopler",QString::number(int(checked)));
}

void Mainwindow::on_toolButton_setRangeUnit_toggled(bool checked)
{

}

void Mainwindow::on_toolButton_xl_dopler_3_clicked(bool checked)
{
    pRadar->isTrueHeadingFromRadar = checked;
}

void Mainwindow::on_toolButton_head_up_clicked(bool checked)
{

}

void Mainwindow::on_toolButton_dk_1_clicked()
{
    commandMay22[4]=0x00;
    processing->sendCommand(commandMay22,12,false);
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

void Mainwindow::on_toolButton_xl_dopler_3_toggled(bool checked)
{

}

void Mainwindow::on_toolButton_tx_14_clicked()
{
    sendToRadarString(CConfig::getString("mFreq7Command"));
}

void Mainwindow::on_toolButton_tx_15_clicked()
{
    sendToRadarString(CConfig::getString("mFreq8Command"));
}

void Mainwindow::on_toolButton_tx_16_clicked()
{
    sendToRadarString(CConfig::getString("mFreq9Command"));
}

void Mainwindow::on_toolButton_tx_17_clicked()
{
    sendToRadarString(CConfig::getString("mFreq10Command"));
}

void Mainwindow::on_toolButton_tx_18_clicked()
{
    sendToRadarString(CConfig::getString("mFreq11Command"));
}

void Mainwindow::on_toolButton_tx_19_clicked()
{
    sendToRadarString(CConfig::getString("mFreq12Command"));
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
    pRadar->noise_nornalize = checked;
}

void Mainwindow::on_toolButton_second_azi_clicked(bool checked)
{
    pRadar->giaQuayPhanCung=checked;
    if(checked)sendToRadarHS("1dab010070");
    else sendToRadarHS("1dab000070");
}

//void Mainwindow::on_on_toolButton_xl_nguong_3_toggled(bool checked)
//{

//}

void Mainwindow::on_toolButton_signal_type_1_clicked()
{
    processing->mRadMode = ModeSimpleSignal;
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
    processing->mRadMode = ModeComplexSignal;
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
    processing->mRadMode = ModeContinuos;
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
        pRadar->isManualTune = true;
    }
    else
    {
        pRadar->isManualTune = false;
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
        processing->isSimulationMode = true;
    }
}

void Mainwindow::on_toolButton_start_simulation_set_clicked(bool checked)
{

    simulator->target[0].setIsManeuver(checked);

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
        simulator->target[0].setEnabled(false);
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
        simulator->target[1].setEnabled(false);
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
        simulator->target[2].setEnabled(false);
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
        simulator->target[3].setEnabled(false);
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
        simulator->target[4].setEnabled(false);
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
        simulator->target[5].setEnabled(false);
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
        simulator->target[6].setEnabled(false);
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
        simulator->target[7].setEnabled(false);
        ui->doubleSpinBox_71->setEnabled(true);
        ui->doubleSpinBox_72->setEnabled(true);
        ui->doubleSpinBox_73->setEnabled(true);
        ui->doubleSpinBox_74->setEnabled(true);
    }
}
void Mainwindow::on_checkBox_stateChanged(int arg1)
{
    updateSimTargetStatus();
}

void Mainwindow::on_checkBox_2_stateChanged(int arg1)
{
    updateSimTargetStatus();
}

void Mainwindow::on_checkBox_3_stateChanged(int arg1)
{
    updateSimTargetStatus();
}

void Mainwindow::on_checkBox_4_stateChanged(int arg1)
{
    updateSimTargetStatus();
}

void Mainwindow::on_checkBox_5_stateChanged(int arg1)
{
    updateSimTargetStatus();
}

void Mainwindow::on_checkBox_6_stateChanged(int arg1)
{
    updateSimTargetStatus();
}

void Mainwindow::on_checkBox_7_stateChanged(int arg1)
{
    updateSimTargetStatus();
}

void Mainwindow::on_checkBox_8_stateChanged(int arg1)
{
    updateSimTargetStatus();
}

void Mainwindow::on_toolButton_start_simulation_set_2_clicked(bool checked)
{
    simulator->target[1].setIsManeuver(checked);
}

void Mainwindow::on_toolButton_start_simulation_set_3_clicked(bool checked)
{
    simulator->target[2].setIsManeuver(checked);
}

void Mainwindow::on_toolButton_start_simulation_set_4_clicked(bool checked)
{
    simulator->target[3].setIsManeuver(checked);
}

void Mainwindow::on_toolButton_start_simulation_set_5_clicked(bool checked)
{
    simulator->target[4].setIsManeuver(checked);
}

void Mainwindow::on_toolButton_start_simulation_set_6_clicked(bool checked)
{
    simulator->target[5].setIsManeuver(checked);
}

void Mainwindow::on_toolButton_start_simulation_set_7_clicked(bool checked)
{
    simulator->target[6].setIsManeuver(checked);
}

void Mainwindow::on_toolButton_start_simulation_set_8_clicked(bool checked)
{
    simulator->target[7].setIsManeuver(checked);
}

void Mainwindow::on_toolButton_start_simulation_stop_clicked()
{

}

void Mainwindow::on_toolButton_start_simulation_stop_clicked(bool checked)
{
    if(checked)
    {
        simulator->pause();
        processing->isSimulationMode = false;
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

    ui->doubleSpinBox_2->setValue(5+(rand()%300)/2.0);
    ui->doubleSpinBox_12->setValue(5+(rand()%300)/2.0);
    ui->doubleSpinBox_22->setValue(5+(rand()%300)/2.0);
    ui->doubleSpinBox_32->setValue(5+(rand()%300)/2.0);
    ui->doubleSpinBox_42->setValue((rand()%300)/2.0);
    ui->doubleSpinBox_52->setValue((rand()%300)/2.0);
    ui->doubleSpinBox_62->setValue((rand()%300)/2.0);
    ui->doubleSpinBox_72->setValue((rand()%300)/2.0);

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
    //processing->setVaru(checked);
    if(checked)
    {
        uchar depth = ui->horizontalSlider_varu_depth->value();
        uchar width = ui->horizontalSlider_varu_width->value();
        uchar comand[8];
        comand[0] = 0x29;
        comand[1] = 0xab;
        comand[2] = 0x01;
        comand[3] = width;
        comand[4] = depth;
        processing->sendCommand(&comand[0]);
        //sendToRadarHS((const char*)comand);
    }
    else
    {
        sendToRadarHS("29ab00");
    }
}

void Mainwindow::on_toolButton_chong_nhieu_2_clicked(bool checked)
{
    //processing->setSharu(checked);
}

void Mainwindow::on_toolButton_chong_nhieu_3_clicked(bool checked)
{
    //processing->setBaru(checked);
    if(checked)
    {
        sendToRadarHS("03ab2f00");
    }
    else
    {
        sendToRadarHS("03abffff");
    }
}

void Mainwindow::on_toolButton_auto_freq_clicked(bool checked)
{
    if(checked)
    {
        sendToRadarHS("28ab2001");
        sendToRadarHS("1aab01");
    }
    else sendToRadarHS("1aab00");
}

void Mainwindow::on_toolButton_chong_nhieu_ppy_clicked(bool checked)
{
    if(checked)
    {
        uchar gain = 100-ui->horizontalSlider_ppy_gain->value();
        uchar comand[8];
        comand[0] = 0x03;
        comand[1] = 0xab;
        if(gain<=63)comand[2] = gain;
        else        comand[2] = 63;
        if(gain<=63)comand[3] = 0;
        else        comand[3] = gain-63;

        comand[4] = 0;
        processing->sendCommand(&comand[0],8);
    }
    else sendToRadarHS("03abffff");
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
        processing->startRecord("D:/HR2D/rec_"+filename+HR_FILE_EXTENSION);
    }
    else
    {
        processing->stopRecord();
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
    pRadar->resetTrack();

}

void Mainwindow::on_toolButton_hdsd_clicked()
{
    DialogDocumentation *dlg=new DialogDocumentation();
    dlg->setModal(false);
    dlg->showNormal();
}

void Mainwindow::on_toolButton_dz_clear_clicked()
{
    pRadar->mDetectZonesList.clear();
}

void Mainwindow::on_toolButton_ais_show_clicked(bool checked)
{
    isShowAIS = checked;
    CConfig::setValue("isShowAIS",int(checked));
}
