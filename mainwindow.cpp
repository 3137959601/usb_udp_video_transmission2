#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QStatusBar>
#include <QTime>
#include <qpainter.h>

#include <iostream>
#include <iomanip>  // for std::fixed and std::setprecision
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
extern bool serial_bind_flag;


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->initUI();
    this->initThreads();
    this->initSerial();
    this->initAD();
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

    ui->serialpB->setIcon(QIcon(":/picture/serial_close.png"));
    // 默认创建状态栏
    auto p_status_bar = this->statusBar();
    p_status_bar->showMessage("这是一个QMainWindow集成QStatusBar示例。");

    // 初始化 QSettings
    settings = new QSettings("MyCompany", "MyApp", this);

    // 加载数据
    //loadData();
    ui->confsv_pB->setEnabled(false);
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

void MainWindow::initSerial()
{
    QStringList serialNamePort;
    for (const auto& it : QSerialPortInfo::availablePorts())
    {
        serialNamePort<<it.portName();

    }
    ui->serialCb->addItems(serialNamePort);

    serialworker = new SerialWorker;

    connect(this,&MainWindow::open_serial_signal,serialworker,&SerialWorker::SerialPortInit);
    connect(this,&MainWindow::close_serial_signal,serialworker,&SerialWorker::SerialClose);

    connect(serialworker,&SerialWorker::recvDataSignal,this,&MainWindow::serial_recvDataSlot);
    //多个信号对应一个槽
    connect(this,&MainWindow::serial_send_signal,serialworker,&SerialWorker::SerialSendData_Slot);

    //connect(serialworker,&SerialWorker::AD_instruction_signal,serialworker,&SerialWorker::SerialSendData_Slot);
    connect(serialworker,&SerialWorker::instruction_send_signal,serialworker,&SerialWorker::SerialSendData_Slot);
}
//电压电流控制窗口初始化，包括加载LCD配置值
void MainWindow::initAD()
{
    //connect(serialworker,&SerialWorker::LCDNumShow,this,&MainWindow::LCDNumShow_slot);
    //connect(this,&MainWindow::ADSettings_signal,serialworker,&SerialWorker::ADInstructionCode);
    connect(serialworker,&SerialWorker::LCDNumShow2,this,&MainWindow::LCDNumShow_slot2);
    connect(this,&MainWindow::InstructSettings_signal,serialworker,&SerialWorker::InstructionCode);
    LCDNumInit();
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
//    Acquire=!Acquire;                             //暂时关闭相机功能，只保留串口传输功能
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


void MainWindow::on_serialpB_clicked()
{
    if(!serial_bind_flag)
    {
        emit open_serial_signal(ui->serialCb->currentText());
        QThread::msleep(10);
        if(!serial_bind_flag)
        {
            QMessageBox::critical(this,tr("Error"),"串口已被占用，请检查是否正确连接");
        }
        else
        {
            ui->serialpB->setText("关闭串口");
            ui->serialpB->setIcon(QIcon(":/picture/serial_open.png"));
            ui->serial_det_pB->setEnabled(false);
            ui->serialCb->setEnabled(false);
        }

    }
    else if(serial_bind_flag)    //将按键与串口是否连接分开来判断，防止无可用串口时按键变灰无法选中
    {
        emit close_serial_signal();
        ui->serialpB->setText("打开串口");
        ui->serialpB->setIcon(QIcon(":/picture/serial_close.png"));
        ui->serial_det_pB->setEnabled(true);
        ui->serialCb->setEnabled(true);
    }
}

void MainWindow::serial_recvDataSlot(QString data)
{
    if(data[0]==QChar(0xEB)&&data[2]!=QChar(0xDA))
    {
        ui->serialRecvpTE->appendPlainText(data);
    }

    //qDebug()<<"开启recv主线程"<<QThread::currentThreadId();//查看槽函数在哪个线程运行
}


void MainWindow::on_command_sendBt_clicked()
{
    emit serial_send_signal(ui->serialSendtE->toPlainText());
}


void MainWindow::on_command_clearBt_clicked()
{
    ui->serialRecvpTE->clear();
}


void MainWindow::on_serial_det_pB_clicked()
{
    QString currentSerialPort  = ui->serialCb->currentText();
    ui->serialCb->clear();   //清空列表
    QStringList serialNamePort;
    for (const auto& it : QSerialPortInfo::availablePorts())
    {
        serialNamePort<<it.portName();

    }
    ui->serialCb->addItems(serialNamePort);
    // 检查是否存在之前选择的串口，如果存在则恢复选择
    if (serialNamePort.contains(currentSerialPort))
    {
        ui->serialCb->setCurrentText(currentSerialPort);
    }
}
void MainWindow::LCDNumInit()
{
//    ui->sc_lcdNum1->setFixedDecimalMode(3,0);
    QList<float>ADSetInitVals;
    ADSetInitVals.resize(26); // 分配足够的空间
    for(int i=0;i<5;i++)
    {
        ADSetInitVals[i] = 0;
    }
        ADSetInitVals[5] = 0.4f;
    for(int i=6;i<21;i++)
    {
        ADSetInitVals[i] = 1.2f;
    }
    for(int i=21;i<24;i++)
    {
        ADSetInitVals[i] = 5.0f;
    }
    for(int i=24;i<26;i++)
    {
        ADSetInitVals[i] = 0.1f;
    }
    //供电电压
    ui->sv_dSB1->setValue(ADSetInitVals[0]);
    ui->sv_dSB2->setValue(ADSetInitVals[1]);
    ui->sv_dSB3->setValue(ADSetInitVals[2]);
    ui->sv_dSB4->setValue(ADSetInitVals[3]);
    ui->sv_dSB5->setValue(ADSetInitVals[4]);
    //SUBPV
    ui->SUBPV_dSB->setValue(ADSetInitVals[5]);
    //偏置电压
    ui->bv_dSB1->setValue(ADSetInitVals[6]);
    ui->bv_dSB2->setValue(ADSetInitVals[7]);
    ui->bv_dSB3->setValue(ADSetInitVals[8]);
    ui->bv_dSB4->setValue(ADSetInitVals[9]);
    ui->bv_dSB5->setValue(ADSetInitVals[10]);
    ui->bv_dSB6->setValue(ADSetInitVals[11]);
    ui->bv_dSB7->setValue(ADSetInitVals[12]);
    ui->bv_dSB8->setValue(ADSetInitVals[13]);
    ui->bv_dSB9->setValue(ADSetInitVals[14]);
    ui->bv_dSB10->setValue(ADSetInitVals[15]);
    ui->bv_dSB11->setValue(ADSetInitVals[16]);
    ui->bv_dSB12->setValue(ADSetInitVals[17]);
    ui->bv_dSB13->setValue(ADSetInitVals[18]);
    ui->bv_dSB14->setValue(ADSetInitVals[19]);
    ui->bv_dSB15->setValue(ADSetInitVals[20]);
    //可调电流
    ui->ajc_dSB1->setValue(ADSetInitVals[21]);
    ui->ajc_dSB2->setValue(ADSetInitVals[22]);
    ui->ajc_dSB3->setValue(ADSetInitVals[23]);
    ui->ajc_dSB4->setValue(ADSetInitVals[24]);
    ui->ajc_dSB5->setValue(ADSetInitVals[25]);
}
void MainWindow::LCDNumShow_slot(unsigned char index,float value)//这种方法没有被使用到，暂时舍弃
{
    //也许有一种办法可以给LCDNumber批量赋值，例如找到它们的指针
    switch(index){
    //供电电流
        case 0x01:
            ui->sc_lcdNum1->display(value);
            break;
        case 0x02:
            ui->sc_lcdNum2->display(value);
            break;
        case 0x03:
            ui->sc_lcdNum3->display(value);
            break;
        case 0x04:
            ui->sc_lcdNum4->display(value);
            break;
        case 0x05:
            ui->sc_lcdNum5->display(value);
            break;
    //供电电压
        case 0x11:
            ui->sv_lcdNum1->display(value);
            break;
        case 0x12:
            ui->sv_lcdNum2->display(value);
            break;
        case 0x13:
            ui->sv_lcdNum3->display(value);
            break;
        case 0x14:
            ui->sv_lcdNum4->display(value);
            break;
        case 0x15:
            ui->sv_lcdNum5->display(value);
            break;
    //SUBPI
        case 0x20:
            ui->SUBPI_lcdNum->display(value);
            break;
    //SUBPI
        case 0x30:
            ui->SUBPV_lcdNum->display(value);
            break;
    //偏置电流
        case 0x41:
            ui->bc_lcdNum1->display(value);
            break;
        case 0x42:
            ui->bc_lcdNum2->display(value);
            break;
        case 0x43:
            ui->bc_lcdNum3->display(value);
            break;
        case 0x44:
            ui->bc_lcdNum4->display(value);
            break;
        case 0x45:
            ui->bc_lcdNum5->display(value);
            break;
        case 0x46:
            ui->bc_lcdNum6->display(value);
            break;
        case 0x47:
            ui->bc_lcdNum7->display(value);
            break;
        case 0x48:
            ui->bc_lcdNum8->display(value);
            break;
        case 0x49:
            ui->bc_lcdNum9->display(value);
            break;
        case 0x4a:
            ui->bc_lcdNum10->display(value);
            break;
        case 0x4b:
            ui->bc_lcdNum11->display(value);
            break;
        case 0x4c:
            ui->bc_lcdNum12->display(value);
            break;
        case 0x4d:
            ui->bc_lcdNum13->display(value);
            break;
        case 0x4e:
            ui->bc_lcdNum14->display(value);
            break;
        case 0x4f:
            ui->bc_lcdNum15->display(value);
            break;
    //偏置电压
        case 0x51:
            ui->bv_lcdNum1->display(value);
            break;
        case 0x52:
            ui->bv_lcdNum2->display(value);
            break;
        case 0x53:
            ui->bv_lcdNum3->display(value);
            break;
        case 0x54:
            ui->bv_lcdNum4->display(value);
            break;
        case 0x55:
            ui->bv_lcdNum5->display(value);
            break;
        case 0x56:
            ui->bv_lcdNum6->display(value);
            break;
        case 0x57:
            ui->bv_lcdNum7->display(value);
            break;
        case 0x58:
            ui->bv_lcdNum8->display(value);
            break;
        case 0x59:
            ui->bv_lcdNum9->display(value);
            break;
        case 0x5a:
            ui->bv_lcdNum10->display(value);
            break;
        case 0x5b:
            ui->bv_lcdNum11->display(value);
            break;
        case 0x5c:
            ui->bv_lcdNum12->display(value);
            break;
        case 0x5d:
            ui->bv_lcdNum13->display(value);
            break;
        case 0x5e:
            ui->bv_lcdNum14->display(value);
            break;
        case 0x5f:
            ui->bv_lcdNum15->display(value);
            break;

    //可调电流
        case 0x61:
            ui->ajc_lcdNum1->display(value);
            break;
        case 0x62:
            ui->ajc_lcdNum2->display(value);
            break;
        case 0x63:
            ui->ajc_lcdNum3->display(value);
            break;
        case 0x64:
            ui->ajc_lcdNum4->display(value);
            break;
        case 0x65:
            ui->ajc_lcdNum5->display(value);
            break;
    default:
        break;
    }

}

void MainWindow::LCDNumShow_slot2(std::vector<float> currents)
{
     // 使用 QString 格式化为三位小数
//     std::vector<QString> formattedNumbers(currents.size());
//     for(int i=0;i<currents.size();i++)
//        formattedNumbers[i] = QString::number(currents[i], 'f', 3);
     // 将格式化后的字符串传递给 QLCDNumber

    //按照原理图顺序,将LCDNumber小数点后固定为三位显示
//供电电流电压
        ui->sc_lcdNum1->display(QString::number(currents[0], 'f', 3));
        ui->sc_lcdNum2->display(QString::number(currents[2], 'f', 3));
        ui->sc_lcdNum3->display(QString::number(currents[4], 'f', 3));
        ui->sc_lcdNum4->display(QString::number(currents[6], 'f', 3));
        ui->sc_lcdNum5->display(QString::number(currents[8], 'f', 3));
//供电电压
        ui->sv_lcdNum1->display(QString::number(currents[1], 'f', 3));
        ui->sv_lcdNum2->display(QString::number(currents[3], 'f', 3));
        ui->sv_lcdNum3->display(QString::number(currents[5], 'f', 3));
        ui->sv_lcdNum4->display(QString::number(currents[7], 'f', 3));
        ui->sv_lcdNum5->display(QString::number(currents[9], 'f', 3));
//SUBPI
        ui->SUBPI_lcdNum->display(QString::number(currents[10]-2.5, 'f', 3));//根据原理图，实际测量值需要减去2.5v的电压
//SUBPI
        ui->SUBPV_lcdNum->display(QString::number(currents[11], 'f', 3));
//偏置电流
        ui->bc_lcdNum1->display(QString::number(currents[17], 'f', 3));
        ui->bc_lcdNum2->display(QString::number(currents[19], 'f', 3));
        ui->bc_lcdNum3->display(QString::number(currents[21], 'f', 3));
        ui->bc_lcdNum4->display(QString::number(currents[23], 'f', 3));
        ui->bc_lcdNum5->display(QString::number(currents[25], 'f', 3));
        ui->bc_lcdNum6->display(QString::number(currents[27], 'f', 3));
        ui->bc_lcdNum7->display(QString::number(currents[29], 'f', 3));
        ui->bc_lcdNum8->display(QString::number(currents[31], 'f', 3));
        ui->bc_lcdNum9->display(QString::number(currents[33], 'f', 3));
        ui->bc_lcdNum10->display(QString::number(currents[35], 'f', 3));
        ui->bc_lcdNum11->display(QString::number(currents[37], 'f', 3));
        ui->bc_lcdNum12->display(QString::number(currents[39], 'f', 3));
        ui->bc_lcdNum13->display(QString::number(currents[41], 'f', 3));
        ui->bc_lcdNum14->display(QString::number(currents[43], 'f', 3));
        ui->bc_lcdNum15->display(QString::number(currents[45], 'f', 3));

//偏置电压
        ui->bv_lcdNum1->display(QString::number(currents[16], 'f', 3));
        ui->bv_lcdNum2->display(QString::number(currents[18], 'f', 3));
        ui->bv_lcdNum3->display(QString::number(currents[20], 'f', 3));
        ui->bv_lcdNum4->display(QString::number(currents[22], 'f', 3));
        ui->bv_lcdNum5->display(QString::number(currents[24], 'f', 3));
        ui->bv_lcdNum6->display(QString::number(currents[26], 'f', 3));
        ui->bv_lcdNum7->display(QString::number(currents[28], 'f', 3));
        ui->bv_lcdNum8->display(QString::number(currents[30], 'f', 3));
        ui->bv_lcdNum9->display(QString::number(currents[32], 'f', 3));
        ui->bv_lcdNum10->display(QString::number(currents[34], 'f', 3));
        ui->bv_lcdNum11->display(QString::number(currents[36], 'f', 3));
        ui->bv_lcdNum12->display(QString::number(currents[38], 'f', 3));
        ui->bv_lcdNum13->display(QString::number(currents[40], 'f', 3));
        ui->bv_lcdNum14->display(QString::number(currents[42], 'f', 3));
        ui->bv_lcdNum15->display(QString::number(currents[44], 'f', 3));

//可调电流
        ui->ajc_lcdNum1->display(QString::number(currents[12], 'f', 3));
        ui->ajc_lcdNum2->display(QString::number(currents[13], 'f', 3));
        ui->ajc_lcdNum3->display(QString::number(currents[14], 'f', 3));
        ui->ajc_lcdNum4->display(QString::number(currents[15], 'f', 3));
        ui->ajc_lcdNum5->display(QString::number(currents[46], 'f', 3));
}
//设置下位机电压值，同时根据原理图的倍数关系对设置值进行转换
void MainWindow::on_confupd_pB_clicked()
{
    QList<float>ADSetVals;
    //根据原理图，电压电流会有2倍的放大，所以设置先除以2
    //供电电压
    ADSetVals.append(ui->sv_dSB1->value()/2);
    ADSetVals.append(ui->sv_dSB2->value()/2);
    ADSetVals.append(ui->sv_dSB3->value()/2);
    ADSetVals.append(ui->sv_dSB4->value()/2);
    ADSetVals.append(ui->sv_dSB5->value()/2);
    //SUBPV
    ADSetVals.append(ui->SUBPV_dSB->value());   //电路忘记放大了，暂时不除以2
    //偏置电压
    ADSetVals.append(ui->bv_dSB1->value()/2);
    ADSetVals.append(ui->bv_dSB2->value()/2);
    ADSetVals.append(ui->bv_dSB3->value()/2);
    ADSetVals.append(ui->bv_dSB4->value()/2);
    ADSetVals.append(ui->bv_dSB5->value()/2);
    ADSetVals.append(ui->bv_dSB6->value()/2);
    ADSetVals.append(ui->bv_dSB7->value()/2);
    ADSetVals.append(ui->bv_dSB8->value()/2);
    ADSetVals.append(ui->bv_dSB9->value()/2);
    ADSetVals.append(ui->bv_dSB10->value()/2);
    ADSetVals.append(ui->bv_dSB11->value()/2);
    ADSetVals.append(ui->bv_dSB12->value()/2);
    ADSetVals.append(ui->bv_dSB13->value()/2);
    ADSetVals.append(ui->bv_dSB14->value()/2);
    ADSetVals.append(ui->bv_dSB15->value()/2);
    //可调电流
    ADSetVals.append(ui->ajc_dSB1->value()/4);//电路放大250倍，而界面已经放大了1000倍，所以需要再除以4倍
    ADSetVals.append(ui->ajc_dSB2->value()/4);//电路放大250倍，而界面已经放大了1000倍，所以需要再除以4倍
    ADSetVals.append(ui->ajc_dSB3->value()/4);//电路放大250倍，而界面已经放大了1000倍，所以需要再除以4倍
    ADSetVals.append(ui->ajc_dSB4->value()*25);//电路放大25000倍，而界面已经放大了1000倍，所以需要再乘以25倍
    ADSetVals.append(ui->ajc_dSB5->value()*25);//电路放大25000倍，而界面已经放大了1000倍，所以需要再乘以25倍

    emit InstructSettings_signal(0xDA,ADSetVals);
    //qDebug()<<"ADSetVals"<<ADSetVals;


}
//积分时间确认，发送积分时间配置指令
void MainWindow::on_itgr_pB_clicked()
{
    QList<float>AASetVals;
    AASetVals.append(ui->itgr_dSB->value());
    emit InstructSettings_signal(0xAA,AASetVals);
}

//主时钟频率配置
void MainWindow::on_clk_pB_clicked()
{
    QList<float>BBSetVals;
    BBSetVals.append(ui->clk_dSB->value());
    emit InstructSettings_signal(0xBB,BBSetVals);
}



void MainWindow::on_confsv_pB_clicked()
{
    QStringList values; // 使用 QStringList 来存放值

    // 获取每个 QDoubleSpinBox 的值
    // 将值转换为字符串并存入列表
    //供电电压
    values << QString::number(ui->sv_dSB1->value());
    values << QString::number(ui->sv_dSB2->value());
    values << QString::number(ui->sv_dSB3->value());
    values << QString::number(ui->sv_dSB4->value());
    values << QString::number(ui->sv_dSB5->value());
    //SUBPV
    values << QString::number(ui->SUBPV_dSB->value());
    //偏置电压
    values << QString::number(ui->bv_dSB1->value());
    values << QString::number(ui->bv_dSB2->value());
    values << QString::number(ui->bv_dSB3->value());
    values << QString::number(ui->bv_dSB4->value());
    values << QString::number(ui->bv_dSB5->value());
    values << QString::number(ui->bv_dSB6->value());
    values << QString::number(ui->bv_dSB7->value());
    values << QString::number(ui->bv_dSB8->value());
    values << QString::number(ui->bv_dSB9->value());
    values << QString::number(ui->bv_dSB10->value());
    values << QString::number(ui->bv_dSB11->value());
    values << QString::number(ui->bv_dSB12->value());
    values << QString::number(ui->bv_dSB13->value());
    values << QString::number(ui->bv_dSB14->value());
    values << QString::number(ui->bv_dSB15->value());
    //可调电流
    values << QString::number(ui->ajc_dSB1->value());
    values << QString::number(ui->ajc_dSB2->value());
    values << QString::number(ui->ajc_dSB3->value());
    values << QString::number(ui->ajc_dSB4->value());
    values << QString::number(ui->ajc_dSB5->value());
    //保存到 QSettings，以逗号分隔的字符串形式存储
    settings->setValue("spinBoxValues", values.join(","));
    settings->sync();
    qDebug() << "Data saved:" << values;

}
void MainWindow:: loadData() {
    // 从 QSettings 中读取值并更新 QDoubleSpinBox
    QString storedValues = settings->value("spinBoxValues", "0,0,0").toString();
    QStringList loadedValues = storedValues.split(",");
        //供电电压
    if (loadedValues.size() > 0)
        ui->sv_dSB1->setValue(loadedValues[0].toDouble());
    if (loadedValues.size() > 1)
        ui->sv_dSB2->setValue(loadedValues[1].toDouble());
    if (loadedValues.size() > 2)
        ui->sv_dSB3->setValue(loadedValues[2].toDouble());
    if (loadedValues.size() > 3)
        ui->sv_dSB4->setValue(loadedValues[3].toDouble());
    if (loadedValues.size() > 4)
        ui->sv_dSB5->setValue(loadedValues[4].toDouble());
        //SUBPV
    if (loadedValues.size() > 5)
        ui->SUBPV_dSB->setValue(loadedValues[5].toDouble());
        //偏置电压
    if (loadedValues.size() > 6)
        ui->bv_dSB1->setValue(loadedValues[6].toDouble());
    if (loadedValues.size() > 7)
        ui->bv_dSB2->setValue(loadedValues[7].toDouble());
    if (loadedValues.size() > 8)
        ui->bv_dSB3->setValue(loadedValues[8].toDouble());
    if (loadedValues.size() > 9)
        ui->bv_dSB4->setValue(loadedValues[9].toDouble());
    if (loadedValues.size() > 10)
        ui->bv_dSB5->setValue(loadedValues[10].toDouble());
    if (loadedValues.size() > 11)
        ui->bv_dSB6->setValue(loadedValues[11].toDouble());
    if (loadedValues.size() > 12)
        ui->bv_dSB7->setValue(loadedValues[12].toDouble());
    if (loadedValues.size() > 13)
        ui->bv_dSB8->setValue(loadedValues[13].toDouble());
    if (loadedValues.size() > 14)
        ui->bv_dSB9->setValue(loadedValues[14].toDouble());
    if (loadedValues.size() > 15)
        ui->bv_dSB10->setValue(loadedValues[15].toDouble());
    if (loadedValues.size() > 16)
        ui->bv_dSB11->setValue(loadedValues[16].toDouble());
    if (loadedValues.size() > 17)
        ui->bv_dSB12->setValue(loadedValues[17].toDouble());
    if (loadedValues.size() > 18)
        ui->bv_dSB13->setValue(loadedValues[18].toDouble());
    if (loadedValues.size() > 19)
        ui->bv_dSB14->setValue(loadedValues[19].toDouble());
    if (loadedValues.size() > 20)
        ui->bv_dSB15->setValue(loadedValues[20].toDouble());
        //可调电流
    if (loadedValues.size() > 21)
        ui->ajc_dSB1->setValue(loadedValues[21].toDouble());
    if (loadedValues.size() > 22)
        ui->ajc_dSB2->setValue(loadedValues[22].toDouble());
    if (loadedValues.size() > 23)
        ui->ajc_dSB3->setValue(loadedValues[23].toDouble());
    if (loadedValues.size() > 24)
        ui->ajc_dSB4->setValue(loadedValues[24].toDouble());
    if (loadedValues.size() > 25)
        ui->ajc_dSB5->setValue(loadedValues[25].toDouble());

    qDebug() << "Data loaded:" << loadedValues;
}

void MainWindow::on_stream_save_pB_clicked()
{
    if(usbthread->stream_save_flag==false)
    {
        usbthread->stream_save_flag = true;
        ui->stream_save_pB->setText("停止保存");
    }
    else if(usbthread->stream_save_flag==true)
    {
        usbthread->stream_save_flag = false;
        ui->stream_save_pB->setText("开始保存");
    }
}

