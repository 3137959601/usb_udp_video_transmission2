#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <qelapsedtimer.h>
#include <QThread>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QMessageBox>
#include <QSettings>

#include <QMainWindow>
#include "child_window/system_settings_window.h"
#include "child_window/logwindow.h"
#include "child_window/image_procss_window.h"
#include "child_window/toolwindow.h"

#include <QGraphicsDropShadowEffect>

#include "cy_cpp/inc/CyAPI.h"
#include "drawthread.h"
#include "usbthread.h"
#include "serialworker.h"


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    static CCyUSBDevice* pUSB;
    static CCyUSBEndPoint * BulkInEpt;
    usbThread *usbthread;
    drawThread *drawthread;

    SerialWorker *serialworker;

    QThread *thread1;
    QThread *thread2;
    QPixmap pixmap ;

    static double frame_rate;
    static  QImage image;
    static  bool Acquire;
    static  bool AutoAdapt;
    static  bool b_area_array;
    static  ushort pic[2][2700*2700];//双缓冲区
    static  int valid_pic;//有效图片index
    static bool b_equalizehist;
    static bool b_medianblur;
    static int T_equalizehist[2];//直方图阈值
    static ushort COL_FPGA;
    static ushort ROW_FPGA;

    bool zoom;
    double zoom_ratio[2];

    void initUI();
    void initThreads();
    void initSerial();
    void initAD();

    void serial_recvDataSlot(QString data);


signals :
    //USB信号
    void new_Xfer();

    //串口信号
    void open_serial_signal(QString com_name);
    void close_serial_signal();
    void serial_send_signal(QString buf);
    //电压电流控制
    void ADSettings_signal(QList<float>value);

protected:
    void closeEvent(QCloseEvent *event) override; // 重载 closeEvent

private slots:
    //void on_system_setup_tB_triggered(QAction *arg1);
    //子窗口按键
    void on_system_setup_tB_clicked();

    void on_log_tB_clicked();

    void on_image_process_tB_clicked();

    void on_home_tB_clicked();


    void on_tooltB_clicked();
    //相机开关按键
    void on_switchBt_clicked();
    //相机检测按键
    void on_camera_det_pB_clicked();

    //串口功能按键
    void on_serialpB_clicked();

    void on_command_sendBt_clicked();

    void on_command_clearBt_clicked();

    void on_serial_det_pB_clicked();
    //电压电流控制窗口
    void LCDNumInit();
    void LCDNumShow_slot(unsigned char index,float value);
    void LCDNumShow_slot2(std::vector<float>currents);
    void on_confupd_pB_clicked();

    //void getADSettings();

    void on_confsv_pB_clicked();

    void loadData();
private:
    Ui::MainWindow *ui;

    QGraphicsDropShadowEffect * defaultShadow;

    system_settings_window *configWindow = nullptr;
    logwindow *logWindow = nullptr;
    image_procss_window *image_process_Window = nullptr;
    toolwindow *toolWindow = nullptr;

    QSettings *settings;
    bool switch_flag;
};
#endif // MAINWINDOW_H
