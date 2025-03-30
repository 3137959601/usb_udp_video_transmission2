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

    //初始化保存路径
    ui->filepath_cB->addItem(QApplication::applicationDirPath());
    usbthread->setSaveDir(QApplication::applicationDirPath());
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
    p_status_bar->showMessage("红外双通道采集系统");

    // 初始化 QSettings
//    settings = new QSettings("MyCompany", "MyApp", this);
    // 使用INI文件存储，确保可写位置
    settings = new QSettings(QApplication::applicationDirPath() + "/config.ini",
                           QSettings::IniFormat,
                           this);

    qDebug() << "Config file:" << settings->fileName();
    // 加载数据
    loadData();

    //初始时先检测一次USB接口
    this->on_camera_det_pB_clicked();


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

//    connect(serialworker,&SerialWorker::recvDataSignal,this,&MainWindow::serial_recvDataSlot);
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
    connect(serialworker,&SerialWorker::Temp_LCDNumShow,this,&MainWindow::Temp_LCDNumShow_slot);
    connect(this,&MainWindow::InstructSettings_signal,serialworker,&SerialWorker::InstructionCode);

}


void MainWindow::closeEvent(QCloseEvent *event)
{
    usbthread->stream_save_flag = false;        //数据流图像保存
    ui->stream_save_pB->setText("开始保存");

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
    Acquire=!Acquire;                             //暂时关闭相机功能，只保留串口传输功能
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

        usbthread->stream_save_flag = false;        //数据流图像保存
        ui->stream_save_pB->setText("开始保存");
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
//原指令接收框，已被删除
//void MainWindow::serial_recvDataSlot(QString data)
//{
//    if(data[0]==QChar(0xEB)&&data[2]!=QChar(0xDA))
//    {
//        ui->serialRecvpTE->appendPlainText(data);
//    }

//    //qDebug()<<"开启recv主线程"<<QThread::currentThreadId();//查看槽函数在哪个线程运行
//}

//原指令发送按钮，已被删除
//void MainWindow::on_command_sendBt_clicked()
//{
//    emit serial_send_signal(ui->serialSendtE->toPlainText());
//}

//原指令清除按钮，已被删除
//void MainWindow::on_command_clearBt_clicked()
//{
//    ui->serialRecvpTE->clear();
//}


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


void MainWindow::LCDNumShow_slot2(std::vector<float> currents)
{
    // 检查数据有效性
    if(currents.size() < 47) {
        qWarning() << "Invalid currents data size:" << currents.size();
        return;
    }

    // 定义LCD控件和对应数据索引的映射
    struct LCDMapping {
        QLCDNumber* lcd;
        int dataIndex;
        std::function<float(float)> transform; // 可选的数据转换函数
    };

    // 所有LCD控件的配置
    const std::vector<LCDMapping> mappings = {
        // 供电电流电压
        {ui->sc_lcdNum1, 0, nullptr}, {ui->sc_lcdNum2, 2, nullptr},
        {ui->sc_lcdNum3, 4, nullptr}, {ui->sc_lcdNum4, 6, nullptr},
        {ui->sc_lcdNum5, 8, nullptr},
        // 供电电压
        {ui->sv_lcdNum1, 1, nullptr}, {ui->sv_lcdNum2, 3, nullptr},
        {ui->sv_lcdNum3, 5, nullptr}, {ui->sv_lcdNum4, 7, nullptr},
        {ui->sv_lcdNum5, 9, nullptr},
        // SUBPI (根据原理图，实际测量值需要减去2.5v的电压)
        {ui->SUBPI_lcdNum, 10, [](float v){ return v - 2.5f; }},
        // SUBPV
        {ui->SUBPV_lcdNum, 11, nullptr},
        // 可调电流
        {ui->ajc_lcdNum1, 12, nullptr}, {ui->ajc_lcdNum2, 13, nullptr},
        {ui->ajc_lcdNum3, 14, nullptr}, {ui->ajc_lcdNum4, 15, nullptr},
        {ui->ajc_lcdNum5, 46, nullptr},
        // 偏置电流(奇数索引)
        {ui->bc_lcdNum1, 17, nullptr}, {ui->bc_lcdNum2, 19, nullptr},
        {ui->bc_lcdNum3, 21, nullptr}, {ui->bc_lcdNum4, 23, nullptr},
        {ui->bc_lcdNum5, 25, nullptr}, {ui->bc_lcdNum6, 27, nullptr},
        {ui->bc_lcdNum7, 29, nullptr}, {ui->bc_lcdNum8, 31, nullptr},
        {ui->bc_lcdNum9, 33, nullptr}, {ui->bc_lcdNum10, 35, nullptr},
        {ui->bc_lcdNum11, 37, nullptr}, {ui->bc_lcdNum12, 39, nullptr},
        {ui->bc_lcdNum13, 41, nullptr}, {ui->bc_lcdNum14, 43, nullptr},
        {ui->bc_lcdNum15, 45, nullptr},
        // 偏置电压(偶数索引)
        {ui->bv_lcdNum1, 16, nullptr}, {ui->bv_lcdNum2, 18, nullptr},
        {ui->bv_lcdNum3, 20, nullptr}, {ui->bv_lcdNum4, 22, nullptr},
        {ui->bv_lcdNum5, 24, nullptr}, {ui->bv_lcdNum6, 26, nullptr},
        {ui->bv_lcdNum7, 28, nullptr}, {ui->bv_lcdNum8, 30, nullptr},
        {ui->bv_lcdNum9, 32, nullptr}, {ui->bv_lcdNum10, 34, nullptr},
        {ui->bv_lcdNum11, 36, nullptr}, {ui->bv_lcdNum12, 38, nullptr},
        {ui->bv_lcdNum13, 40, nullptr}, {ui->bv_lcdNum14, 42, nullptr},
        {ui->bv_lcdNum15, 44, nullptr}
    };

    // 统一处理所有LCD显示
    for(const auto& mapping : mappings) {
        float value = currents[mapping.dataIndex];
        if(mapping.transform) {
            value = mapping.transform(value);
        }
        mapping.lcd->display(QString::number(value, 'f', 3));
    }
}

void MainWindow::Temp_LCDNumShow_slot(std::vector<float> Temps)
{
    ui->Temp_lcdNum->display(QString::number(Temps[0], 'f', 2));
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
    // 定义所有需要保存的spinbox指针数组
    QDoubleSpinBox* spinBoxes[] = {
        // 供电电压
        ui->sv_dSB1, ui->sv_dSB2, ui->sv_dSB3, ui->sv_dSB4, ui->sv_dSB5,
        // SUBPV
        ui->SUBPV_dSB,
        // 偏置电压
        ui->bv_dSB1, ui->bv_dSB2, ui->bv_dSB3, ui->bv_dSB4, ui->bv_dSB5,
        ui->bv_dSB6, ui->bv_dSB7, ui->bv_dSB8, ui->bv_dSB9, ui->bv_dSB10,
        ui->bv_dSB11, ui->bv_dSB12, ui->bv_dSB13, ui->bv_dSB14, ui->bv_dSB15,
        // 可调电流
        ui->ajc_dSB1, ui->ajc_dSB2, ui->ajc_dSB3, ui->ajc_dSB4, ui->ajc_dSB5,
        //积分时间
        ui->itgr_dSB,
        //主时钟频率
        ui->clk_dSB,ui->clk_dSB_2
    };

    QStringList values;
    const int spinBoxCount = sizeof(spinBoxes) / sizeof(spinBoxes[0]);

    // 遍历所有spinbox获取值
    for (int i = 0; i < spinBoxCount; ++i) {
        values << QString::number(spinBoxes[i]->value());
    }

    // 保存到QSettings
    settings->setValue("spinBoxValues", values.join(","));
    settings->sync();
    qDebug() << "Data saved:" << values;

}
void MainWindow:: loadData() {

    // 定义所有需要保存的spinbox指针数组
    QDoubleSpinBox* spinBoxes[] = {
        // 供电电压
        ui->sv_dSB1, ui->sv_dSB2, ui->sv_dSB3, ui->sv_dSB4, ui->sv_dSB5,
        // SUBPV
        ui->SUBPV_dSB,
        // 偏置电压
        ui->bv_dSB1, ui->bv_dSB2, ui->bv_dSB3, ui->bv_dSB4, ui->bv_dSB5,
        ui->bv_dSB6, ui->bv_dSB7, ui->bv_dSB8, ui->bv_dSB9, ui->bv_dSB10,
        ui->bv_dSB11, ui->bv_dSB12, ui->bv_dSB13, ui->bv_dSB14, ui->bv_dSB15,
        // 可调电流
        ui->ajc_dSB1, ui->ajc_dSB2, ui->ajc_dSB3, ui->ajc_dSB4, ui->ajc_dSB5,
        //积分时间
        ui->itgr_dSB,
        //主时钟频率
        ui->clk_dSB,ui->clk_dSB_2
    };

    // 从 QSettings 中读取值并更新 QDoubleSpinBox,如果不存在，则使用defaultValues
    const int spinBoxCount = sizeof(spinBoxes) / sizeof(spinBoxes[0]);
    QString defaultValues = QString("0,").repeated(spinBoxCount-1) + "0";

    QString storedValues = settings->value("spinBoxValues", defaultValues).toString();
    QStringList loadedValues = storedValues.split(",");

    for (int i = 0; i < spinBoxCount && i < loadedValues.size(); ++i) {
        spinBoxes[i]->setValue(loadedValues[i].toDouble());
    }
    qDebug() << "Data loaded:" << loadedValues;
}

void MainWindow::on_stream_save_pB_clicked()
{
    if(Acquire)
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
    else
    {
        usbthread->stream_save_flag = false;
        ui->stream_save_pB->setText("开始保存");
    }
}


void MainWindow::on_path_sel_tB_clicked()
{
    QString directory = QDir::toNativeSeparators(QFileDialog::getExistingDirectory(this,tr("Streamer Save path" ),QDir::currentPath()));
    if ( !directory.isEmpty()){
        if (ui->filepath_cB->findText(directory) == -1)ui->filepath_cB->addItem(directory);
        ui->filepath_cB->setCurrentIndex(ui->filepath_cB->findText(directory));
    }
    usbthread->setSaveDir(directory);
}

void MainWindow::LCDNumShow_slot(unsigned char index,float value)//这种方法没有被使用到，暂时舍弃
{
    //可以通过使用 ​查找表（Lookup Table）​ 替代冗长的 switch-case  QMap<unsigned char, QLCDNumber*> lcdMap;
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
