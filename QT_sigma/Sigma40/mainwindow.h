#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#define degrees(x) (x*57.295779513)
#define radians(x) (x/57.295779513)
#include <QMainWindow>
#include <QDateTime>
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
protected slots:
    void timerEvent(QTimerEvent *event);
private slots:
    void on_horizontalSlider_valueChanged(int value);

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
