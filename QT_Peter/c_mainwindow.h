#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCKAPI_
//#define SCR_W 1920
//#define SCR_H 1080


#define HR_FILE_EXTENSION ".r2d"
//#include <qse>
#include <c_config.h>
#include "dialogdetaildisplay.h"
#include "dialogdocumentation.h"
#include "c_radar_data.h"
#include "c_radar_simulation.h"
#include <QtNetwork>
#include <QMainWindow>
#include <QtCore>
#include <QtGui>
#include "pkp.h"
#include <time.h>
#include <dialogaisinfo.h>
//#include <CLocal.h>
#include "dialoginputvalue.h"
#include <c_radar_thread.h>
#include <QDesktopWidget>
#include <QFileDialog>
#include <QFile>
#include <QImage>
#include <QHostAddress>
//#include <jtarget.h>
#include "c_target_manager.h"
#include "Cmap/cmap.h"
//#include "dialogcommandlog.h"
#include "dialogconfig.h"
//#include <QtConcurrent/QtConcurrent>
#include <QMenu>
#include <QMessageBox>
#ifdef THEON
#define SCR_W 1920
#define SCR_H 1200
#define SCR_LEFT_MARGIN 150
#define SCR_TOP_MARGIN 0
#define SCR_BORDER_SIZE 48
#else
#define SCR_W 1280
#define SCR_H 1024
#define SCR_LEFT_MARGIN -29
#define SCR_TOP_MARGIN 25
#define SCR_BORDER_SIZE 120
#endif
namespace Ui {
class MainWindow;
class DialogDocumentation;
//class QLabel;
//class QPushButton;
//class QUdpSocket;
}

class Mainwindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit Mainwindow(QWidget *parent = nullptr);
    ~Mainwindow();
    PointDouble ConvKmXYToScrPoint(double x, double y);
//    C_primary_track* checkClickRadarTarget(int xclick, int yclick, bool isDoubleClick = false);
protected:
    //void contextMenuEvent(QContextMenuEvent *event);
//    void keyPressEvent(QKeyEvent *event);
    void paintEvent(QPaintEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseClickEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void mouseDoubleClickEvent( QMouseEvent * e );
//    enum radarSate   { DISCONNECTED,CONNECTED,CONNECTED_ROTATE9_TXOFF,CONNECTED_ROTATE12_TXOFF, CONNECTED_ROTATE9_TXON,CONNECTED_ROTATE12_TXON } radar_state;
private:

    QCursor cursor_default;
    QRect mIADrect;
    PointDouble mIADCenter;
    double mZoomScale;
    QRect ppiRect;
    c_radar_simulation          *simulator;// thread tao gia tin hieu
    QThread                     *tprocessing;

    QString degreeSymbol ;
    bool controlPressed;
    c_target_manager mTargetMan;
    int zoom_size;
    void DrawGrid(QPainter* p,short centerX,short centerY);
//    void detectZone();
    void InitSetting();
    void sendToRadarHS(const char *hexdata);
    void sendToRadar(unsigned char* hexdata);
//    void SetSnScale(short value);
    Ui::MainWindow* ui;
//    QMenu   *       m_fileMenu;
//    QMenu   *       m_connectionMenu;
//    QAction *       a_openShp;
//    QAction *       a_openPlace;
//    QAction *       a_savePlace;
//    QAction *       a_gpsOption;
//    QAction *       a_openSignal;

    //network
    //QUdpSocket      *udpSocket;//raymarine
//    QUdpSocket      *udpARPA;//ARPA
    QUdpSocket      *m_udpSocket;//socket for radar control
    QString                     mTxCommand,mRxCommand;
    QString                     mR0Command,mR1Command,mR2Command,mR3Command,
                                mR4Command,mR5Command,mR6Command,mR7Command;
    QString                     mFreq1Command,mFreq2Command,mFreq3Command,mFreq4Command,mFreq5Command,mFreq6Command;
    //
    //CConfig         m_config;
    //CpView  *       m_view;

    QTimer          *drawTimer;
    QPoint          view_pos;

    void SaveBinFile();
    void InitNetwork();
    //void sendFrame(const char* hexdata,QHostAddress host,int port );
    void InitTimer();
    void showTime();
    void DrawViewFrame(QPainter *p);
//    void DrawSignal(QPainter *p);
//    void drawAisTarget(QPainter *p);
//    void DrawRadarTargetByPainter(QPainter* p);

    void ReloadSetting();
    void SendCommandControl();
//    void SetGPS(double lat, double lon);
public slots:
//    void UpdateSetting();
//    void UpdateSignScale();
    void UpdateScale();
    void setCodeType(short index);
    void trackTableItemMenu(int row, int col);
    void removeTrack();
    void addToTargets();
    void targetTableItemMenu(int row, int col);
    void setEnemy();
    void setFriend();
    void removeTarget();
    void changeID();
protected slots:
    void closeEvent(QCloseEvent *event);
private:

//    void setRadarState(radarSate radarState);
//    bool ProcDataAIS(BYTE *szBuff, int nLeng );
//public:
//    void setScaleNM(unsigned short rangeNM);
//    void drawAisTarget2(QPainter *p);
    void on_actionOpen_rec_file_triggered();
private slots:
//    void DrawMap();
    void readBuffer();
    void sync1S();
    void sync1p();
    void ShutDown();
    void gpsOption();
    void processARPA();
//    void on_actionExit_triggered();
    void UpdateVideo();
    void PlaybackRecFile();

//    void on_actionRecording_toggled(bool arg1);
//
//    void on_actionOpen_map_triggered();
//    void on_actionSaveMap_triggered();
//    void on_actionSetting_triggered();
//    void on_actionAddTarget_toggled(bool arg1);
//    void on_actionClear_data_triggered();
//    void on_actionView_grid_triggered(bool checked);
    void on_actionPlayPause_toggled(bool arg1);
//    void on_actionRecording_triggered();
//    void on_comboBox_temp_type_currentIndexChanged(int index);
//    void on_horizontalSlider_brightness_actionTriggered(int action);
    void on_horizontalSlider_brightness_valueChanged(int value);

//    void on_horizontalSlider_signal_scale_valueChanged(int value);

//    void on_actionSector_Select_triggered();

    //void on_toolButton_13_clicked();

    //void on_toolButton_14_clicked();

//    void on_actionRotateStart_toggled(bool arg1);

    void on_horizontalSlider_gain_valueChanged(int value);

    void on_horizontalSlider_rain_valueChanged(int value);

    void on_horizontalSlider_sea_valueChanged(int value);

//    void on_toolButton_exit_clicked();

    //void on_toolButton_setting_clicked();

//    void on_toolButton_scan_clicked();

    //void on_toolButton_tx_toggled(bool checked);

//    void on_toolButton_scan_toggled(bool checked);

//    void on_toolButton_xl_nguong_toggled(bool checked);

    void on_toolButton_replay_toggled(bool checked);

//    void on_toolButton_replay_fast_toggled(bool checked);

    void on_toolButton_record_toggled(bool checked);

    void on_toolButton_open_record_clicked();

//    void on_toolButton_alphaView_toggled(bool checked);

    //void on_toolButton_replay_2_clicked();

    void on_toolButton_centerView_clicked();

//    void on_comboBox_currentIndexChanged(int index);

    void on_comboBox_img_mode_currentIndexChanged(int index);

    void on_toolButton_send_command_clicked();

    //void on_toolButton_map_toggled(bool checked);

    void on_toolButton_zoom_in_clicked();

    void on_toolButton_zoom_out_clicked();

//    void on_toolButton_reset_clicked();


//    void on_toolButton_send_command_2_clicked();


//    void on_toolButton_map_select_clicked();

//    void on_dial_valueChanged(int value);

//    void on_toolButton_set_heading_clicked();

//    void on_toolButton_gps_update_clicked();

    //void on_comboBox_code_type_currentIndexChanged(const QString &arg1);

    //void on_comboBox_code_type_currentIndexChanged(int index);

//    void on_toolButton_centerZoom_clicked();

    void on_toolButton_xl_dopler_clicked();

//    void on_toolButton_xl_dopler_toggled(bool checked);

//    void on_toolButton_xl_nguong_3_toggled(bool checked);

//    void on_groupBox_3_currentChanged(int index);

    void on_toolButton_xl_dopler_2_toggled(bool checked);



//    void on_toolButton_reset_3_clicked();

//    void on_toolButton_reset_2_clicked();

//    void on_toolButton_vet_clicked(bool checked);

    void on_label_status_warning_clicked();

//    void on_toolButton_delete_target_clicked();

    void on_toolButton_tx_clicked();

    void on_toolButton_tx_off_clicked();

//    void on_toolButton_filter2of3_clicked(bool checked);

   // void on_comboBox_radar_resolution_currentIndexChanged(int index);

//    void on_toolButton_create_zone_2_clicked(bool checked);

    void on_toolButton_measuring_clicked();

    //void on_toolButton_map_2_clicked();

//    void on_comboBox_2_currentIndexChanged(int index);

    void on_toolButton_measuring_clicked(bool checked);

    void on_toolButton_export_data_clicked(bool checked);

//    void on_toolButton_auto_select_toggled(bool checked);

    void on_toolButton_ais_reset_clicked();

    //void on_toolButton_2x_zoom_clicked(bool checked);

//    void on_toolButton_auto_adapt_clicked();

//    void on_toolButton_set_header_size_clicked();

//    void on_toolButton_xl_nguong_clicked();

//    void on_toolButton_xl_nguong_clicked(bool checked);

//    void on_toolButton_filter2of3_2_clicked(bool checked);

    void on_toolButton_command_dopler_clicked();

    void on_toolButton_command_period_clicked();

    void on_toolButton_noise_balance_clicked();

    void on_toolButton_limit_signal_clicked();

    void on_toolButton_command_amplifier_clicked();

    void on_toolButton_command_dttt_clicked();

    void on_toolButton_command_res_clicked();

    void on_toolButton_command_antenna_rot_clicked();

    void on_comboBox_3_currentIndexChanged(int index);

    void on_horizontalSlider_map_brightness_valueChanged(int value);


    void on_toolButton_selfRotation_toggled(bool checked);

//    void on_toolButton_scope_toggled(bool checked);

//    void on_toolButton_manual_track_toggled(bool checked);

    void on_toolButton_measuring_toggled(bool checked);

    void on_toolButton_VRM_toggled(bool checked);

    void on_toolButton_ELB_toggled(bool checked);

    void on_toolButton_record_clicked();

//    void on_toolButton_sharp_eye_toggled(bool checked);

    void on_toolButton_help_clicked();

    void on_toolButton_setRangeUnit_clicked();



//    void on_toolButton_gps_update_auto_clicked();



    void on_toolButton_set_zoom_ar_size_clicked();

    void on_toolButton_advanced_control_clicked();

//    void on_toolButton_set_commands_clicked(bool checked);

//    void on_toolButton_set_command_clicked();

//    void on_toolButton_grid_clicked(bool checked);

//    void on_toolButton_auto_freq_toggled(bool checked);

    void on_toolButton_set_default_clicked();

//    void on_toolButton_gps_update_auto_2_clicked();

//    void on_toolButton_heading_update_clicked();

    void on_toolButton_sled_clicked();

    void on_toolButton_sled_toggled(bool checked);

    void on_toolButton_sled_reset_clicked();

//    void on_toolButton_ais_name_toggled(bool checked);

//    void on_toolButton_filter2of3_toggled(bool checked);

//    void on_toolButton_dobupsong_clicked();

//    void on_toolButton_dobupsong_toggled(bool checked);

//    void on_toolButton_set_commands_toggled(bool checked);

    void on_toolButton_set_commands_clicked();

    void on_toolButton_command_log_toggled(bool checked);

    void on_toolButton_exit_2_clicked();

    void on_toolButton_selfRotation_2_toggled(bool checked);

//    void on_toolButton_selfRotation_clicked();

//    void on_toolButton_tx_2_toggled(bool checked);

    void on_toolButton_tx_2_clicked(bool checked);

    void on_toolButton_menu_clicked();

    void on_toolButton_iad_clicked();

    void on_tabWidget_menu_tabBarClicked(int index);

    void on_tabWidget_iad_tabBarClicked(int index);

//    void on_toolButton_xl_nguong_3_clicked();

    void on_toolButton_head_up_toggled(bool checked);

//    void on_toolButton_delete_target_2_clicked();

//    void on_toolButton_dk_1_toggled(bool checked);

//    void on_toolButton_dk_2_toggled(bool checked);

//    void on_toolButton_dk_3_toggled(bool checked);

//    void on_toolButton_dk_4_toggled(bool checked);

//    void on_toolButton_dk_5_toggled(bool checked);

//    void on_toolButton_dk_6_toggled(bool checked);

//    void on_toolButton_dk_7_toggled(bool checked);

//    void on_toolButton_dk_8_toggled(bool checked);

//    void on_toolButton_dk_9_toggled(bool checked);

//    void on_toolButton_dk_10_toggled(bool checked);

    void Update100ms();
//    void on_toolButton_dk_12_toggled(bool checked);

//    void on_toolButton_dk_14_toggled(bool checked);

    void on_toolButton_dk_13_toggled(bool checked);

    void on_toolButton_dk_15_toggled(bool checked);

    void on_toolButton_dk_11_toggled(bool checked);

    void on_toolButton_dk_16_toggled(bool checked);

    void on_toolButton_dk_17_toggled(bool checked);

    void on_toolButton_grid_toggled(bool checked);

    void on_toolButton_dk_4_clicked();

    void on_toolButton_dk_3_clicked();

    void on_toolButton_dk_5_clicked();

    void on_toolButton_dk_6_clicked();

    void on_toolButton_dk_7_clicked();

    void on_toolButton_dk_8_clicked();

    void on_toolButton_dk_9_clicked();

    void on_toolButton_dk_2_clicked();

    void on_toolButton_dk_13_clicked();

    void on_toolButton_dk_12_clicked();

//    void on_toolButton_dk_10_clicked();

//    void on_toolButton_dk_14_clicked();

//    void on_toolButton_dk_15_clicked();

    void on_toolButton_sled_time25_clicked();

    void on_toolButton_sled_time8_clicked();

    void on_toolButton_sled_time3_clicked();

//    void on_toolButton_sled_reset_2_clicked(bool checked);

//    void on_toolButton_sled_reset_3_clicked(bool checked);

//    void on_toolButton_sled_reset_4_clicked(bool checked);

//    void on_toolButton_sled_reset_3_toggled(bool checked);



    void on_on_toolButton_xl_nguong_3_clicked(bool checked);

    void on_toolButton_xl_nguong_4_clicked(bool checked);

    void on_toolButton_sled_clicked(bool checked);

    void on_toolButton_xl_dopler_clicked(bool checked);

    void on_toolButton_setRangeUnit_toggled(bool checked);

//    void on_toolButton_xl_dopler_3_clicked(bool checked);

    void on_toolButton_head_up_clicked(bool checked);

    void on_toolButton_dk_1_clicked();

    void on_toolButton_chi_thi_mt_clicked(bool checked);

    void on_bt_rg_1_toggled(bool checked);

    void on_bt_rg_2_toggled(bool checked);

    void on_bt_rg_3_toggled(bool checked);

    void on_bt_rg_4_toggled(bool checked);

//    void on_bt_rg_5_toggled(bool checked);

    void on_bt_rg_6_toggled(bool checked);

    void on_bt_rg_8_toggled(bool checked);

    void on_bt_rg_7_toggled(bool checked);

//    void on_toolButton_xl_dopler_3_toggled(bool checked);



    void on_toolButton_open_record_2_clicked();

    void on_bt_rg_6_clicked();

    void on_bt_rg_7_clicked();

    void on_bt_rg_8_clicked();

    void on_bt_rg_1_clicked();

    void on_bt_rg_2_clicked();

    void on_bt_rg_3_clicked();

    void on_bt_rg_4_clicked();

    void on_bt_rg_5_clicked();

    void on_toolButton_xl_nguong_5_clicked(bool checked);

    void on_toolButton_second_azi_clicked(bool checked);

//    void on_on_toolButton_xl_nguong_3_toggled(bool checked);

    void on_toolButton_signal_type_1_clicked();

    void on_toolButton_signal_type_2_clicked();

    void on_toolButton_signal_type_3_clicked();

    void on_toolButton_del_tget_table_clicked();

    void on_toolButton_manual_tune_clicked(bool checked);

//    void on_dial_valueChanged(int value);

//    void on_horizontalSlider_valueChanged(int value);

    void on_toolButton_start_simulation_start_clicked(bool checked);

    void on_toolButton_start_simulation_set_clicked(bool checked);

    void on_checkBox_stateChanged(int arg1);

    void on_checkBox_2_stateChanged(int arg1);

    void on_checkBox_3_stateChanged(int arg1);

    void on_checkBox_4_stateChanged(int arg1);

    void on_checkBox_5_stateChanged(int arg1);

    void on_checkBox_6_stateChanged(int arg1);

    void on_checkBox_7_stateChanged(int arg1);

    void on_checkBox_8_stateChanged(int arg1);

//    void on_toolButton_start_simulation_set_2_clicked(bool checked);

//    void on_toolButton_start_simulation_set_3_clicked(bool checked);

//    void on_toolButton_start_simulation_set_4_clicked(bool checked);

//    void on_toolButton_start_simulation_set_5_clicked(bool checked);

//    void on_toolButton_start_simulation_set_6_clicked(bool checked);

//    void on_toolButton_start_simulation_set_7_clicked(bool checked);

//    void on_toolButton_start_simulation_set_8_clicked(bool checked);

    void on_toolButton_start_simulation_stop_clicked();

    void on_toolButton_start_simulation_stop_clicked(bool checked);

//    void on_bt_rg_1_clicked(bool checked);

    void on_toolButton_sim_target_autogenerate_clicked();

    void on_checkBox_clicked();

    void on_toolButton_chong_nhieu_1_clicked(bool checked);

//    void on_toolButton_chong_nhieu_2_clicked(bool checked);

//    void on_toolButton_chong_nhieu_3_clicked(bool checked);

    void on_toolButton_auto_freq_clicked(bool checked);

    void on_toolButton_chong_nhieu_ppy_clicked(bool checked);

    void on_toolButton_record_clicked(bool checked);

    void on_bt_rg_5_clicked(bool checked);

//    void on_pushButton_dzs_1_clicked();

//    void on_pushButton_dzs_1_clicked(bool checked);

    void on_toolButton_dzs_1_clicked(bool checked);

    void on_toolButton_dzs_2_clicked();

    void on_toolButton_hdsd_clicked();

    void on_toolButton_dz_clear_clicked();

    void on_toolButton_ais_show_clicked(bool checked);

    void on_toolButton_loc_dia_vat_clicked(bool checked);

    void on_toolButton_loc_dia_vat_2_clicked();

    void on_toolButton_tx_off_2_clicked();

    void on_toolButton_menu_2_clicked();

//    void on_toolButton_antennaConfigUpdate_clicked();

    void on_toolButton_dk_18_clicked();

    void on_toolButton_exit_3_clicked(bool checked);

    void on_toolButton_passive_mode_clicked(bool checked);

    void on_toolButton_tx_clicked(bool checked);

    void on_horizontalSlider_varu_width_valueChanged(int value);

    void on_horizontalSlider_varu_depth_valueChanged(int value);

    void on_horizontalSlider_ppy_gain_valueChanged(int value);

    void on_toolButton_tx_3_clicked(bool checked);

    void on_comboBox_currentIndexChanged(int index);

    void on_toolButton_cao_ap_1_clicked();

    void on_toolButton_cao_ap_2_clicked();

    void on_toolButton_antennaConfigUpdate_clicked();

    void on_toolButton_exit_4_clicked(bool checked);

    void on_horizontalSlider_ppy_gain_2_valueChanged(int value);

    void on_toolButton_chong_nhieu_ppy_2_clicked();

    void on_toolButton_chong_nhieu_ppy_2_clicked(bool checked);

//    void on_toolButton_ais_hide_fishing_clicked(bool checked);

    void on_customButton_load_density_clicked();

    void on_customButton_openCPN_clicked();

    void on_lineEdit_simulation_lost_editingFinished();

    void on_toolButton_manual_tracking_clicked(bool checked);

    void on_toolButton_start_simulation_set_all_clicked(bool checked);

//    void on_toolButton_replay_clicked(bool checked);

//    void on_toolButton_dk_4_clicked(bool checked);

    void on_horizontalSlider_valueChanged(int value);

//    void on_toolButton_replay_fast_clicked(bool checked);

    void on_toolButton_autotracking_clicked(bool checked);

    void on_toolButton_radar_clicked(bool checked);

    void on_tableWidgetTarget_clicked(const QModelIndex &index);

private:
    bool isRadarShow;
    C_SEA_TRACK *selectedTrack;
//    int target_size;
//    bool mShowobjects,
//    bool mShowTracks;

//    unsigned long long mSelectedTrackId;
//    uint mSelectedTrackTime;
    void initActionsConnections();
    void initGraphicView();
//    void updateTargetInfo();
//    void ConvWGSToKm(double *x, double *y, double m_Long, double m_Lat);
//    void ConvKmToWGS(double x, double y, double *m_Long, double *m_Lat);
    void setScaleRange(double srange);
    void DrawIADArea(QPainter *p);
//    bool isInsideViewZone(int x, int y);
    void UpdateMouseStat(QPainter *p);
    void setMouseMode(mouseMode mode, bool isOn);
    bool CalcAziContour(double theta, double d);
    void DisplayClkAdc(int clk);
    void setDistanceUnit(int unit);
    void keyReleaseEvent(QKeyEvent *event);
    void sendToRadarString(QString command);
//    void autoSwitchFreq();
    void checkClickAIS(int xclick, int yclick);
    void UpdateGpsData();
    void CheckRadarStatus();
    void ViewTrackInfo();
    void gotoCenter();
//    void rotateVector(double angle, int *x, int *y);
    void SendScaleCommand();
    void DrawDetectZones(QPainter *p);
    void updateSimTargetStatus();
    void ConvXYradar2XYscr();
//    PointDouble ConvWGSToScrPoint(double m_Long, double m_Lat);
//    PointDouble ConvScrPointToKMXY(int x, int y);
    QPoint mBorderPoint2,mBorderPoint1,mBorderPoint0;
//    void rotateVector(double angle, double *x, double *y);
    void SetUpTheonGUILayout();
//    PointAziRgkm ConvScrPointToAziRgkm(int x, int y);
    void showTrackContext();
    void DrawViewFrameSquared(QPainter *p);
    void checkCuda();
    void initCursor();
    void DrawAISMark(PointDouble s, double head, QPainter *p, QString name, int size, int vectorLen);
    bool isInsideIADZone(int x, int y);
    bool checkInsideZoom(int x, int y);
    void saveScreenShot(QString fileName);
    void SetTx(bool onoff);
    bool CheckTxCondition(bool isPopupMsg);
//    PointDouble ConvScrPointToWGS(int x, int y);
    bool isInsideViewZone(int x, int y);
};

#endif // MAINWINDOW_H
