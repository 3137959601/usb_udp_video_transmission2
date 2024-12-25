#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "child_window/system_settings_window.h"
#include "child_window/logwindow.h"
#include "child_window/image_procss_window.h"
#include "child_window/toolwindow.h"

#include <QGraphicsDropShadowEffect>

#include "cy_cpp/inc/CyAPI.h"
#include "drawthread.h"
#include "usbthread.h"

#include<qelapsedtimer.h>

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

    void repaintImage();

signals :
    void new_Xfer();
    void new_display();

protected:
    void closeEvent(QCloseEvent *event) override; // 重载 closeEvent

private slots:
    //void on_system_setup_tB_triggered(QAction *arg1);

    void on_system_setup_tB_clicked();

    void on_log_tB_clicked();

    void on_image_process_tB_clicked();

    void on_home_tB_clicked();


    void on_tooltB_clicked();

    void on_switchBt_clicked();

private:
    Ui::MainWindow *ui;

    QGraphicsDropShadowEffect * defaultShadow;

    system_settings_window *configWindow = nullptr;
    logwindow *logWindow = nullptr;
    image_procss_window *image_process_Window = nullptr;
    toolwindow *toolWindow = nullptr;

    bool switch_flag;
};
#endif // MAINWINDOW_H
