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
    connect(serialworker,&SerialWorker::DA_instruction_signal,serialworker,&SerialWorker::SerialSendData_Slot);
}
//电压电流控制窗口初始化，包括加载LCD配置值
void MainWindow::initAD()
{
    //connect(serialworker,&SerialWorker::LCDNumShow,this,&MainWindow::LCDNumShow_slot);
    //connect(this,&MainWindow::ADSettings_signal,serialworker,&SerialWorker::ADInstructionCode);
    connect(serialworker,&SerialWorker::LCDNumShow2,this,&MainWindow::LCDNumShow_slot2);
    connect(this,&MainWindow::ADSettings_signal,serialworker,&SerialWorker::DAInstructionCode);

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
        }

    }
    else if(serial_bind_flag)    //将按键与串口是否连接分开来判断，防止无可用串口时按键变灰无法选中
    {
        emit close_serial_signal();
        ui->serialpB->setText("打开串口");
        ui->serialpB->setIcon(QIcon(":/picture/serial_close.png"));
        ui->serial_det_pB->setEnabled(true);
    }
}

void MainWindow::serial_recvDataSlot(QString data)
{
    ui->serialRecvpTE->appendPlainText(data);
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

void MainWindow::LCDNumShow_slot(unsigned char index,float value)
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
//供电电流
        ui->sc_lcdNum1->display(currents[0]);
        ui->sc_lcdNum2->display(currents[1]);
        ui->sc_lcdNum3->display(currents[2]);
        ui->sc_lcdNum4->display(currents[3]);
        ui->sc_lcdNum5->display(currents[4]);
//供电电压
        ui->sv_lcdNum1->display(currents[5]);
        ui->sv_lcdNum2->display(currents[6]);
        ui->sv_lcdNum3->display(currents[7]);
        ui->sv_lcdNum4->display(currents[8]);
        ui->sv_lcdNum5->display(currents[9]);
//SUBPI
        ui->SUBPI_lcdNum->display(currents[10]);
//SUBPI
        ui->SUBPV_lcdNum->display(currents[11]);
//偏置电流
        ui->bc_lcdNum1->display(currents[12]);
        ui->bc_lcdNum2->display(currents[13]);
        ui->bc_lcdNum3->display(currents[14]);
        ui->bc_lcdNum4->display(currents[15]);
        ui->bc_lcdNum5->display(currents[16]);
        ui->bc_lcdNum6->display(currents[17]);
        ui->bc_lcdNum7->display(currents[18]);
        ui->bc_lcdNum8->display(currents[19]);
        ui->bc_lcdNum9->display(currents[20]);
        ui->bc_lcdNum10->display(currents[21]);
        ui->bc_lcdNum11->display(currents[22]);
        ui->bc_lcdNum12->display(currents[23]);
        ui->bc_lcdNum13->display(currents[24]);
        ui->bc_lcdNum14->display(currents[25]);
        ui->bc_lcdNum15->display(currents[26]);

//偏置电压
        ui->bv_lcdNum1->display(currents[27]);
        ui->bv_lcdNum2->display(currents[28]);
        ui->bv_lcdNum3->display(currents[29]);
        ui->bv_lcdNum4->display(currents[30]);
        ui->bv_lcdNum5->display(currents[31]);
        ui->bv_lcdNum6->display(currents[32]);
        ui->bv_lcdNum7->display(currents[33]);
        ui->bv_lcdNum8->display(currents[34]);
        ui->bv_lcdNum9->display(currents[35]);
        ui->bv_lcdNum10->display(currents[36]);
        ui->bv_lcdNum11->display(currents[37]);
        ui->bv_lcdNum12->display(currents[38]);
        ui->bv_lcdNum13->display(currents[39]);
        ui->bv_lcdNum14->display(currents[40]);
        ui->bv_lcdNum15->display(currents[41]);

//可调电流
        ui->ajc_lcdNum1->display(currents[42]);
        ui->ajc_lcdNum2->display(currents[43]);
        ui->ajc_lcdNum3->display(currents[44]);
        ui->ajc_lcdNum4->display(currents[45]);
        ui->ajc_lcdNum5->display(currents[46]);
}

void MainWindow::on_confupd_pB_clicked()
{
    QList<float>ADSetVals;
    //供电电压
    ADSetVals.append(ui->sv_dSB1->value());
    ADSetVals.append(ui->sv_dSB2->value());
    ADSetVals.append(ui->sv_dSB3->value());
    ADSetVals.append(ui->sv_dSB4->value());
    ADSetVals.append(ui->sv_dSB5->value());
    //SUBPV
    ADSetVals.append(ui->SUBPV_dSB->value());
    //偏置电压
    ADSetVals.append(ui->bv_dSB1->value());
    ADSetVals.append(ui->bv_dSB2->value());
    ADSetVals.append(ui->bv_dSB3->value());
    ADSetVals.append(ui->bv_dSB4->value());
    ADSetVals.append(ui->bv_dSB5->value());
    ADSetVals.append(ui->bv_dSB6->value());
    ADSetVals.append(ui->bv_dSB7->value());
    ADSetVals.append(ui->bv_dSB8->value());
    ADSetVals.append(ui->bv_dSB9->value());
    ADSetVals.append(ui->bv_dSB10->value());
    ADSetVals.append(ui->bv_dSB11->value());
    ADSetVals.append(ui->bv_dSB12->value());
    ADSetVals.append(ui->bv_dSB13->value());
    ADSetVals.append(ui->bv_dSB14->value());
    ADSetVals.append(ui->bv_dSB15->value());
    //可调电流
    ADSetVals.append(ui->ajc_dSB1->value());
    ADSetVals.append(ui->ajc_dSB2->value());
    ADSetVals.append(ui->ajc_dSB3->value());
    ADSetVals.append(ui->ajc_dSB4->value());
    ADSetVals.append(ui->ajc_dSB5->value());

    emit ADSettings_signal(ADSetVals);
    //qDebug()<<"ADSetVals"<<ADSetVals;


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
        ui->sv_dSB1->setValue(loadedValues[1].toDouble());
    if (loadedValues.size() > 2)
        ui->sv_dSB2->setValue(loadedValues[2].toDouble());
    if (loadedValues.size() > 3)
        ui->sv_dSB3->setValue(loadedValues[3].toDouble());
    if (loadedValues.size() > 4)
        ui->sv_dSB4->setValue(loadedValues[4].toDouble());
    if (loadedValues.size() > 5)
        ui->sv_dSB5->setValue(loadedValues[5].toDouble());
        //SUBPV
    if (loadedValues.size() > 6)
        ui->SUBPV_dSB->setValue(loadedValues[6].toDouble());
        //偏置电压
    if (loadedValues.size() > 7)
        ui->bv_dSB1->setValue(loadedValues[7].toDouble());
    if (loadedValues.size() > 8)
        ui->bv_dSB2->setValue(loadedValues[8].toDouble());
    if (loadedValues.size() > 9)
        ui->bv_dSB3->setValue(loadedValues[9].toDouble());
    if (loadedValues.size() > 10)
        ui->bv_dSB4->setValue(loadedValues[10].toDouble());
    if (loadedValues.size() > 11)
        ui->bv_dSB5->setValue(loadedValues[11].toDouble());
    if (loadedValues.size() > 12)
        ui->bv_dSB6->setValue(loadedValues[12].toDouble());
    if (loadedValues.size() > 13)
        ui->bv_dSB7->setValue(loadedValues[13].toDouble());
    if (loadedValues.size() > 14)
        ui->bv_dSB8->setValue(loadedValues[14].toDouble());
    if (loadedValues.size() > 15)
        ui->bv_dSB9->setValue(loadedValues[15].toDouble());
    if (loadedValues.size() > 16)
        ui->bv_dSB10->setValue(loadedValues[16].toDouble());
    if (loadedValues.size() > 17)
        ui->bv_dSB11->setValue(loadedValues[17].toDouble());
    if (loadedValues.size() > 18)
        ui->bv_dSB12->setValue(loadedValues[18].toDouble());
    if (loadedValues.size() > 19)
        ui->bv_dSB13->setValue(loadedValues[19].toDouble());
    if (loadedValues.size() > 20)
        ui->bv_dSB14->setValue(loadedValues[20].toDouble());
    if (loadedValues.size() > 21)
        ui->bv_dSB15->setValue(loadedValues[21].toDouble());
        //可调电流
    if (loadedValues.size() > 22)
        ui->ajc_dSB1->setValue(loadedValues[22].toDouble());
    if (loadedValues.size() > 23)
        ui->ajc_dSB2->setValue(loadedValues[23].toDouble());
    if (loadedValues.size() > 24)
        ui->ajc_dSB3->setValue(loadedValues[24].toDouble());
    if (loadedValues.size() > 25)
        ui->ajc_dSB4->setValue(loadedValues[25].toDouble());
    if (loadedValues.size() > 26)
        ui->ajc_dSB5->setValue(loadedValues[26].toDouble());

    qDebug() << "Data loaded:" << loadedValues;
}
