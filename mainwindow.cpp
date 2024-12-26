#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QStatusBar>
#include <QTime>
#include <qpainter.h>

CCyUSBDevice* MainWindow::pUSB = new CCyUSBDevice;
CCyUSBEndPoint * MainWindow::BulkInEpt;
bool MainWindow::Acquire = false;
ushort MainWindow::pic[2][2700*2700];
int MainWindow::valid_pic = 0;
QImage MainWindow::image = QImage(640,512,QImage::Format_Grayscale16);//
bool  MainWindow::b_equalizehist = false;
bool MainWindow::b_medianblur = false;
int MainWindow::T_equalizehist[2] = {75,15};
bool MainWindow::AutoAdapt = false;
bool MainWindow::b_area_array = true;//默认面阵
double MainWindow::frame_rate = 0;

ushort MainWindow::ROW_FPGA = 512;//分辨率初始化
ushort MainWindow::COL_FPGA = 640;//分辨率初始化

bool b_fullscreen = false;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->initUI();
    this->initThreads();


}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initUI()
{
    this->setWindowTitle("双通道红外图像传输系统");

    Acquire = false;
    ui->switchBt->setIcon(QIcon(":/picture/switch_off.png"));
    ui->switchBt->setStyleSheet("border: none;"); // 去掉默认边框
    ui->switchBt->setIconSize(QSize(40, 40)); // 设置图标大小

    ui->home_tB->setIcon(QIcon(":/picture/home_1.png"));
    ui->home_tB->setIconSize(QSize(20, 20)); // 增加图标大小

    ui->image_process_tB->setIcon(QIcon(":/picture/median_filter.png"));
    ui->image_process_tB->setIconSize(QSize(20, 20)); // 增加图标大小

    ui->log_tB->setIcon(QIcon(":/picture/log.png"));
    ui->log_tB->setIconSize(QSize(20, 20)); // 增加图标大小

    ui->system_setup_tB->setIcon(QIcon(":/picture/set.png"));
    ui->system_setup_tB->setIconSize(QSize(20, 20)); // 增加图标大小

    ui->tooltB->setIcon(QIcon(":/picture/tool.png"));
    ui->tooltB->setIconSize(QSize(20, 20)); // 增加图标大小

    // 默认创建状态栏
    auto p_status_bar = this->statusBar();
    p_status_bar->showMessage("这是一个QMainWindow集成QStatusBar示例。");

    //初始时先检测一次USB接口
    this->on_camera_det_pB_clicked();

    //    zoom_ratio[0] = 0.2;
    //    zoom_ratio[1] = 0.25;
}

void MainWindow::initThreads()
{
    //为自定义的模块分配空间  不能指定父对象
    usbthread = new usbThread;
    drawthread = new drawThread;

    //为自定义的子线程分配空间  指定父对象
    thread1 = new QThread(this);//子线程1
    thread2 = new QThread(this);//子2
    //thread1->setPriority(QThread::HighPriority);

    //把自定义模块添加到子线程
    usbthread->moveToThread(thread1);
    drawthread->moveToThread(thread2);

    connect(this,&MainWindow::new_Xfer,usbthread,&usbThread::Xfer);
    connect(usbthread,SIGNAL(updatapic()),drawthread,SLOT(drawimage()));//usb线程发出pic更新信号，draw线程绘图
    connect(drawthread,&drawThread::updataimage,ui->usb_widget,&widget_image::repaintImage);//draw线程绘图结束，主线程更新界面

    //启动子线程
    thread1->start();
    thread2->start();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    Acquire = false;
    if(thread1)
    {
        thread1->quit();
        thread1->wait();
        usbthread->deleteLater();
        thread1->deleteLater();
    }
    if(thread2)
    {
        thread2->quit();
        thread2->wait();
        drawthread->deleteLater();
        thread2->deleteLater();
    }

    // 如果子窗口存在，关闭它
    if (configWindow != nullptr) {
        configWindow->close();
    }
    if (logWindow != nullptr) {
        logWindow->close();
    }
    if (image_process_Window != nullptr) {
        image_process_Window->close();
    }
    if (toolWindow != nullptr) {
        toolWindow->close();
    }
    // 执行其他关闭时的逻辑

    // 继续关闭主窗口
    event->accept(); // 通过调用 accept() 准许事件
}

void MainWindow::on_system_setup_tB_clicked()
{

    // 如果newWindow为空或者已经关闭，则创建新窗口
    if (configWindow == nullptr) {
        configWindow = new system_settings_window();
        configWindow->setAttribute(Qt::WA_DeleteOnClose); // 窗口关闭时自动删除
        // 连接主窗口的destroyed信号到新窗口的close槽
        connect(configWindow, &QWidget::destroyed, this, [this]() {
            configWindow = nullptr; // 将configWindow重置为nullptr
            // 设置工具按钮状态为按下
            ui->system_setup_tB->setChecked(false);
        });
    } else {
        // 如果窗口已经打开，则将其激活
        configWindow->raise(); // 提升窗口到最前面
        configWindow->activateWindow(); // 激活窗口
        ui->system_setup_tB->setChecked(true);
    }
        configWindow->show(); // 显示新窗口

}


void MainWindow::on_log_tB_clicked()
{
    // 如果newWindow为空或者已经关闭，则创建新窗口
    if (logWindow == nullptr) {
        logWindow = new logwindow();
        logWindow->setAttribute(Qt::WA_DeleteOnClose); // 窗口关闭时自动删除
        // 连接主窗口的destroyed信号到新窗口的close槽
        connect(logWindow, &QWidget::destroyed, this, [this]() {
            logWindow = nullptr; // 将configWindow重置为nullptr
            // 设置工具按钮状态为按下
            ui->log_tB->setChecked(false);
        });
    } else {
        // 如果窗口已经打开，则将其激活
        logWindow->raise(); // 提升窗口到最前面
        logWindow->activateWindow(); // 激活窗口
        ui->log_tB->setChecked(true);
    }
        logWindow->show(); // 显示新窗口
}


void MainWindow::on_image_process_tB_clicked()
{
    // 如果newWindow为空或者已经关闭，则创建新窗口
    if (image_process_Window == nullptr) {
        image_process_Window = new image_procss_window();
        image_process_Window->setAttribute(Qt::WA_DeleteOnClose); // 窗口关闭时自动删除
        // 连接主窗口的destroyed信号到新窗口的close槽
        connect(image_process_Window, &QWidget::destroyed, this, [this]() {
            image_process_Window = nullptr; // 将configWindow重置为nullptr
            // 设置工具按钮状态为按下
            ui->image_process_tB->setChecked(false);
        });
    } else {
        // 如果窗口已经打开，则将其激活
        image_process_Window->raise(); // 提升窗口到最前面
        image_process_Window->activateWindow(); // 激活窗口
        ui->image_process_tB->setChecked(true);
    }
        image_process_Window->show(); // 显示新窗口
}


void MainWindow::on_home_tB_clicked()
{
    if (!this->isActiveWindow()) {
        this->raise(); // 提升窗口到最前面
        this->activateWindow(); // 激活窗口
    }
        ui->home_tB->setChecked(true);
}




void MainWindow::on_tooltB_clicked()
{
    // 如果newWindow为空或者已经关闭，则创建新窗口
    if (toolWindow == nullptr) {
        toolWindow = new toolwindow();
        toolWindow->setAttribute(Qt::WA_DeleteOnClose); // 窗口关闭时自动删除
        // 连接主窗口的destroyed信号到新窗口的close槽
        connect(toolWindow, &QWidget::destroyed, this, [this]() {
            toolWindow = nullptr; // 将configWindow重置为nullptr
            // 设置工具按钮状态为按下
            ui->tooltB->setChecked(false);
        });
    } else {
        // 如果窗口已经打开，则将其激活
        toolWindow->raise(); // 提升窗口到最前面
        toolWindow->activateWindow(); // 激活窗口
        ui->tooltB->setChecked(true);
    }
        toolWindow->show(); // 显示新窗口
}


void MainWindow::on_switchBt_clicked()
{
    Acquire=!Acquire;
    if(Acquire)
    {
        ui->switchBt->setIcon(QIcon(":/picture/switch_on.png"));
        emit new_Xfer();
        ui->camera_det_pB->setEnabled(false);
        ui->camera_comboBox->setEnabled(false);
    }else{
        ui->switchBt->setIcon(QIcon(":/picture/switch_off.png"));
        ui->camera_det_pB->setEnabled(true);
        ui->camera_comboBox->setEnabled(true);
    }
}


void MainWindow::on_camera_det_pB_clicked()
{
    int nDeviceCount = pUSB->DeviceCount();
    ui->camera_comboBox->clear();

    for (int nIdx = 0; nIdx < nDeviceCount; nIdx++)
    {
        pUSB->Open(nIdx);
        qDebug("DeviceName  :%s\n", pUSB->DeviceName);
        qDebug("FriendlyName:%s\n", pUSB->FriendlyName);
        qDebug("VendorID    :%4x\n", pUSB->VendorID);
        qDebug("ProductID   :%4x\n", pUSB->ProductID);
        ui->camera_comboBox->addItem(pUSB->DeviceName);
    }
}

