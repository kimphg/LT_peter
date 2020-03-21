#-------------------------------------------------
#
# Project created by QtCreator 2015-06-20T15:53:12
# Author Phung Kim Phuong pawnc7@gmail.com
#-------------------------------------------------
#Features:
#Control and process data from HR2D radar
#view ARPA data
#-------------------------------------------------
# Version 4.0.1
#-------------------------------------------------
#data file type defined as .r2d,
#CONFIG += ARTEMIS
#define ARTEMIS for C4I view mode
#define THEON for raytheon radar
#define
ARTEMIS {
  SOURCES  += c_mainwindowbasic.cpp
  HEADERS  += c_mainwindowbasic.h
    TARGET = Artemis
} else {
  SOURCES  += c_mainwindow.cpp
  HEADERS  += c_mainwindow.h
    TARGET = Peter
}
@CONFIG  += debug_and_release@
QT       += core gui
QT       += network
#QT       += serialport
#QT	    += positioning
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
QMAKE_CXXFLAGS_WARN_OFF -= -Wunused-parameter

TEMPLATE = app
SOURCES += main.cpp\
    qcustombutton.cpp \
    qcustomframe.cpp \
    qcustomcombobox.cpp \
    qcustomgroupbox.cpp \
    qcustomtabwidget.cpp \
#    ctarget.cpp \
    Cmap/cmap.cpp\
    c_config.cpp \
    c_radar_data.cpp \
    c_radar_thread.cpp \
    dialogdocumentation.cpp \
    AIS/AIS.cpp \
    dialogaisinfo.cpp \
    statuswindow.cpp \
    dialogmenudisplay.cpp \
#    c_mainwindow.cpp \
#    c_decision_tree.cpp
    c_gps.cpp \
    c_target_manager.cpp \
#    c_radar_simulation.cpp
    c_radar_simulation.cpp \
    dialoginputvalue.cpp \
    dialogconfig.cpp \
    dialogdetaildisplay.cpp \
    c_arpa_area.cpp \
    dialogmodeselect.cpp
HEADERS  += \
    qcustombutton.h \
    qcustomframe.h \
    qcustomcombobox.h \
    qcustomgroupbox.h \
    qcustomtabwidget.h \
    pkp.h \
    Cmap/cmap.h\
    c_config.h \
    c_radar_data.h \
    c_radar_thread.h \
    dialogdocumentation.h \
    AIS/AIS.h \
    dialogaisinfo.h \
    statuswindow.h \
    dialogmenudisplay.h \
#    c_mainwindow.h \
#    c_decision_tree.h
    c_gps.h \
    c_target_manager.h \
    c_radar_simulation.h \
    c_radar_simulation.h \
    c_sim_target.h \
    dialoginputvalue.h \
    dialogconfig.h \
    common.h \
    dialogdetaildisplay.h \
    c_arpa_area.h \
    dialogmodeselect.h
#    pch_file.h
#PRECOMPILED_HEADER = "pch_file.h"
#CONFIG += precompile_header
FORMS    += \
    mainwindow.ui \
    dialogdocumentation.ui \
    dialogaisinfo.ui \
    statuswindow.ui \
    dialogmenudisplay.ui \
    dialoginputvalue.ui \
    dialogconfig.ui \
    dialogdetaildisplay.ui \
    c_mainwindowbasic.ui \
    dialogmodeselect.ui
#win32:CONFIG(release, debug|release): LIBS += -L$$PWD/shapelib/ -lshapelib
#else:unix: LIBS += -L$$PWD\shapelib\ -lshapelib
#INCLUDEPATH += $$PWD/shapelib
#DEPENDPATH += $$PWD/shapelib
#unix: LIBS += -L/usr/lib  -ld
#unix: INCLUDEPATH += /usr/lib
INCLUDEPATH += $$PWD/tinyxml/
DEPENDPATH += $$PWD/tinyxml/
INCLUDEPATH += $$PWD/
DEPENDPATH += $$PWD/
#INCLUDEPATH += $$PWD/mitab/mitab/
#INCLUDEPATH += $$PWD/mitab/cpl/
#INCLUDEPATH += $$PWD/mitab/ogr/
#RESOURCES +=
#INCLUDEPATH += $$PWD/../armadillo/include
#DEPENDPATH += $$PWD/../armadillo/include
#win32:LIBS += -L$$PWD/../armadilloWin32/lib_winx86/ -lblas_win32_MT
#win32:LIBS += -L$$PWD/../armadilloWin32/lib_winx86/ -llapack_win32_MT
#win32:LIBS += -L$$PWD/../armadillo/lib_winx86/ -lcbia.lib.blas.dyn.rel.x86.12
#win32:LIBS += -L$$PWD/../armadillo/lib_winx86/ -lcbia.lib.lapack.dyn.rel.x86.12

#unix: LIBS += -larmadillo

#LIBS += -lQt5Concurrent
#win32:LIBS += -L$$PWD/mitab/ -lmitab
#LIBS += -ltinyxml
#INCLUDEPATH += $$PWD/../opencv/build/include
#win32:LIBS += -L$$PWD/../opencv/build/x86/vc10/lib/ -lopencv_core249
#win32:LIBS += -L$$PWD/../opencv/build/x86/vc10/lib/ -lopencv_highgui249
#win32:LIBS += -L$$PWD/../opencv/build/x86/vc10/lib/ -lopencv_imgproc249
#win32:LIBS += -L$$PWD/../opencv/build/x86/vc10/lib/ -lopencv_videoio249

#LIBS +=

#win32:CONFIG(release, debug|release):INCLUDEPATH += $$PWD/armadillo/lib_winx86
#win32:CONFIG(release, debug|release):DEPENDPATH += $$PWD/armadillo/lib_winx86

#unix:QMAKE_CXXFLAGS += -O2
#unix:QMAKE_CXXFLAGS += -l -DARMA_DONT_USE_WRAPPER -lblas -llapack
DISTFILES += \
    appIcon.rc
win32:RC_FILE += appIcon.rc
 *msvc* { # visual studio spec filter
      QMAKE_CXXFLAGS += -MP
  }
