#include "c_mainwindow.h"
#include <QApplication>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    //a.setOverrideCursor(QCursor(QPixmap("img\crossCursor.png")));
    Mainwindow w;
    //w.showFullScreen();
    //w.on_MainWindow_SizeChanged();
    w.show();
    //w.on_MainWindow_SizeChanged();
    a.exec();
    return EXIT_FAILURE;
}
