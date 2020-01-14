#define C4I_VIEW_MODE

#ifdef C4I_VIEW_MODE
#include "c_mainwindowbasic.h"
#else
#include "c_mainwindow.h"
#endif

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    //a.setOverrideCursor(QCursor(QPixmap("img\crossCursor.png")));
#ifdef C4I_VIEW_MODE
    MainWindowBasic w;
#else
    Mainwindow w;
#endif
    //w.showFullScreen();
    //w.on_MainWindow_SizeChanged();
    w.show();
    //w.on_MainWindow_SizeChanged();
    a.exec();
    return EXIT_FAILURE;
}
