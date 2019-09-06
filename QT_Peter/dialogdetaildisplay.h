#ifndef DIALOGDETAILDISPLAY_H
#define DIALOGDETAILDISPLAY_H

#include <QDialog>
#include "c_arpa_area.h"
#include "c_radar_thread.h"
#include "dialogaisinfo.h"
namespace Ui {
class DialogDetailDisplay;
}

class DialogDetailDisplay : public QDialog
{
    Q_OBJECT

public:
    explicit DialogDetailDisplay(QWidget *parent = 0);
    ~DialogDetailDisplay();

    void init(dataProcessingThread *processingThread, DialogAisInfo *tinfoPointer);

    C_arpa_area rda;


private:

    int timerId;
    void DrawSignal(QPainter *p);
    Ui::DialogDetailDisplay *ui;


protected:
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);
private slots:
    void timerEvent(QTimerEvent *event);
    void on_toolButton_view_IAD_clicked(bool checked);
    void on_toolButton_view_histogram_clicked(bool checked);
    void on_toolButton_view_zoom_clicked(bool checked);
};

#endif // DIALOGDETAILDISPLAY_H
