#include "qcustomtabwidget.h"

#include <QStyleOption>

QCustomTabWidget::QCustomTabWidget(QWidget *parent) : QTabWidget(parent)
{
    isDragging = false;
    setMouseTracking(true);
    //this->setStyleSheet("background-color: rgb(30, 50, 70);color:rgb(255, 255, 255);font: 12pt \"MS Shell Dlg 2\";");
    setAttribute(Qt::WA_Hover);
    this->setCursor(Qt::ArrowCursor);
    SetTransparent(false);
    mMoveable = false;
    //this->setMovable(true);
}

void QCustomTabWidget::hoverEnter(QHoverEvent *)
{

   // highLight();
}

void QCustomTabWidget::hoverLeave(QHoverEvent *)
{
    //resetView();
}

void QCustomTabWidget::hoverMove(QHoverEvent *)
{
    //highLight();

}
void QCustomTabWidget::initStyle()
{
    setStyleSheet("QCustomTabWidget{background-color: rgb(30, 50, 70,255);color:white;font: 8pt \"MS Shell Dlg 2\"; "
                  "border-style: groove; border-width: 2px;border-color:white;}"
                  //"QTabWidget QTabBar{background-color: rgb(30, 50, 70,255);border-style: groove; border-width: 2px;}"
                  "QTabWidget QTabBar::tab{background-color: rgb(10, 20, 40,255);color:white;border-style: groove;border-right:noborder; border-width: 2px;}"
                  "QTabWidget QTabBar::tab:selected{background-color: rgb(30, 50, 70,255);color:white;border-bottom:noborder}"
                  );
    /*  setStyleSheet("QCustomTabWidget{background-color: rgb(30, 50, 70,255);color:rgb(30, 50, 70,255);font: 12pt \"MS Shell Dlg 2\"; "
                  "border-style: groove; border-width: 2px;border-color:white;}"
                  "QTabWidget QTabBar{background-color: #136ba2;width: 256px;}"
                  "QTabWidget QTabBar::tab{background-color: #136ba2;}"
                  "QTabWidget QTabBar::tab:selected{background-color: #0f5d90;}"
                  );
                  */
}
void QCustomTabWidget::paintEvent(QPaintEvent *)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);

}



void QCustomTabWidget::SetTransparent(bool transp)
{
    if(transp)
        setStyleSheet("QCustomTabWidget{background-color: rgb(0, 0, 0,0);color:rgb(30, 50, 70,255);font: 12pt \"MS Shell Dlg 2\"; "
                            "border-style: groove; border-width: 2px;border-color:white;}");
    else
        initStyle();

}

void QCustomTabWidget::mousePressEvent(QMouseEvent *event)
{
    if(!mMoveable)return;
     mMX= event->pos().x();
     mMY= event->pos().y();
     isDragging = true;
}

void QCustomTabWidget::mouseReleaseEvent(QMouseEvent *event)
{
    isDragging = false;
}

void QCustomTabWidget::mouseMoveEvent(QMouseEvent *event)
{
    if(!mMoveable)return;
    if(event->buttons() & Qt::LeftButton)
    {
//        QRect rect = this->geometry();
//        rect.setX(rect.x() + event->x()-mMX);
//        rect.setY(rect.y() + event->y()-mMY);
        short difx = event->pos().x()-mMX;
        short dify = event->pos().y()-mMY;
//        short nx = this->mapFromGlobal(event->pos()).x();
//        short ny = this->mapFromGlobal(event->pos()).y();
        this->move(this->pos().x()+difx,this->pos().y()+dify);
        //mMX = nx;
        //mMY = ny;
    }
}

bool QCustomTabWidget::event(QEvent *event)
{

    /*switch(event->type())
    {

    default:
        break;
    }*/
    return QWidget::event(event);
}
