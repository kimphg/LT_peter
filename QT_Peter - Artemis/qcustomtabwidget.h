#ifndef QCUSTOM_TAB_WIDGET_H
#define QCUSTOM_TAB_WIDGET_H

#include <QWidget>
#include <QTabWidget>
#include <qevent.h>
#include <c_radar_thread.h>
#include <QPainter>
class QCustomTabWidget: public QTabWidget
{
    Q_OBJECT
public:
    explicit QCustomTabWidget(QWidget *parent = 0);
    dataProcessingThread *processing;
    void SetTransparent(bool transp);
    bool isDragging,mMoveable;
protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void initStyle();
    void hoverEnter(QHoverEvent *event);
    void hoverLeave(QHoverEvent *event);
    void hoverMove(QHoverEvent *event);
    bool event(QEvent *event);
    void paintEvent(QPaintEvent *);
private:
    short mMX,mMY;

signals:


};

#endif // QCUSTOM_TAB_WIDGET_H
