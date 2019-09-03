#include "dialogdetaildisplay.h"
#include "ui_dialogdetaildisplay.h"

#include <QPainter>
static enum ViewMode {ZoomHiden =0,ZoomIAD=1,ZoomHistogram=2,ZoomSpectre=3,ZoomRamp=4,ZoomZoom=5} view_mode=ZoomHiden;

DialogDetailDisplay::DialogDetailDisplay(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogDetailDisplay)
{
    ui->setupUi(this);
    rda.trueShiftDeg = 0;

    rda.showAisName = true;
}
void DialogDetailDisplay::init(dataProcessingThread *processingThread,DialogAisInfo *tinfoPointer)
{
    on_toolButton_view_zoom_clicked(true);
    rda.dialogTargetInfo =tinfoPointer;
    timerId = this->startTimer(1000);
    rda.mZoomSizeRg=CConfig::getDouble("mZoomSizeRg",2);
    rda.mZoomSizeAz=CConfig::getDouble("mZoomSizeAz",2);

    rda.setCenterLonLat(CConfig::getDouble("mLonZoom",DEFAULT_LONG),CConfig::getDouble("mLatZoom",DEFAULT_LAT));
    rda.processing=processingThread;
    rda.mRadarData = (rda.processing->mRadarData);
    this->setFixedSize(ZOOM_SIZE,ZOOM_SIZE);
    this->setGeometry(1360,600,0,0);
    rda.radCtX = width()/2;
    rda.radCtY = height()/2;
    rda.rect = this->rect();
    this->setWindowFlags(this->windowFlags()|Qt::FramelessWindowHint);
    //this->setWindowFlags(this->windowFlags()&(~Qt::WindowContextHelpButtonHint));
    this->setStyleSheet("background-color: rgb(0, 0, 0);color:rgb(255, 255, 255);font: bold 12pt \"MS Shell Dlg 2\";");
    this->show();

}

void C_arpa_area::setCenterLonLat(double lon, double lat)
{
    mLon = lon;
    mLat = lat;
    CConfig::setValue("mLonZoom",mLon);
    CConfig::setValue("mLatZoom",mLat);
}

void DialogDetailDisplay::paintEvent(QPaintEvent *event)
{
    rda.mScale  = rda.mRadarData->getScale_PpiPerKm()*rda.mRadarData->getScale_zoom_ppi();
    rda.mZoomSizeKm = this->width()/rda.mScale;
    rda.target_size = rda.mScale*0.3;
    QPainter p(this);
    DrawSignal(&p);
    rda.drawAisTarget(&p);
    rda.DrawRadarTargetByPainter(&p);
}

void DialogDetailDisplay::timerEvent(QTimerEvent *event)
{
    repaint();
}


void DialogDetailDisplay::mousePressEvent(QMouseEvent *event)
{
    int posx = event->x();
    int posy = event->y();
    if(posx)rda.mMouseLastX= posx;
    if(posy)rda.mMouseLastY= posy;
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
        if(posx)rda.mMousex= posx;
        if(posy)rda.mMousey= posy;

        if(!rda.checkClickRadarTarget(posx,posy))
            rda.checkClickAIS(posx,posy);

    }

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
        if((rda.mRadarData->getMimg_zoom_ar()==nullptr)||(rda.mRadarData->getMimg_zoom_ar()->isNull()))return;
        //        printf("\nDraw IAD");
        p->setPen(QPen(Qt::white,2));


        QImage img = rda.mRadarData->getMimg_zoom_ar()->scaled(mIADrect.width(),mIADrect.height(),Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
        p->drawImage(mIADrect,img);//todo:resize
        p->setFont(QFont("Times",10));
        p->drawText(mIADrect.x()+mIADrect.width()-50,mIADrect.y()+mIADrect.height()-10,
                    QString::number(rda.mZoomSizeAz,'f',1)+
                    QString::fromUtf8(" Độ"));
        p->drawText(mIADrect.x()+5,mIADrect.y()+15,
                    QString::number(rda.mZoomSizeRg/1.852,'f',1)+
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
        p->drawImage(mIADrect,*(rda.mRadarData->getMimg_zoom_ppi()),rda.mRadarData->getMimg_zoom_ppi()->rect());
//        if(mRangeIndex>2)
//        {

//            p->setPen(QPen(QColor(255,255,255,200),0,Qt::DashLine));
//            p->setBrush(Qt::NoBrush);
//            p->drawRect(mZoomCenterx-zoom_size/2.0,mZoomCentery-zoom_size/2.0,zoom_size,zoom_size);
//        }

    }
    else if(view_mode==ZoomHistogram)
    {

        p->drawImage(mIADrect,*(rda.mRadarData->getMimg_histogram()),
                     rda.mRadarData->getMimg_histogram()->rect());

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
