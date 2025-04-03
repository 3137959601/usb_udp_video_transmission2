#include "udp_thread.h"
#include "mainwindow.h"
#include <QElapsedTimer>

bool udp_bind_flag = false;
QImage Udp_Thread::image = QImage(640,512,QImage::Format_Grayscale16);//
ushort Udp_Thread::pic[2][2700*2700];
int Udp_Thread::valid_pic = 0;
ushort Udp_Thread::ROW_FPGA = 512;//分辨率初始化
ushort Udp_Thread::COL_FPGA = 640;//分辨率初始化

// 创建 QElapsedTimer 对象
//QElapsedTimer timer;
//float elapsedMilliseconds = 0;
int current_row = 0;

Udp_Thread::Udp_Thread(QObject *parent)
    : QObject{parent}// 子线程不传递 parent
{
    Q_UNUSED(parent); // 用以消除未使用警告
//    udpSocket= new QUdpSocket(this);// 调用udp_thread中的构造函数时还在主线程中运行，此时还未将其移入子线程，所以不能在这里构造，否则是在主线程中运行
//    connect(udpSocket, &QUdpSocket::readyRead, this, &Udp_Thread::on_readyReadData);

}

Udp_Thread::~Udp_Thread()
{
    delete udpSocket;
}


void Udp_Thread::setSaveDir(const QString &path)
{
    save_path = path;
}
void Udp_Thread::udp_bind(QString ip, uint port)
{
    udpSocket= new QUdpSocket();// 子线程中不能再指定父对象，因将其移至另一个线程
    connect(udpSocket, &QUdpSocket::readyRead, this, &Udp_Thread::on_readyReadData);

    // 返回绑定函数返回值
    m_hostAddr = QHostAddress(ip);
    m_port = port;
//    qDebug()<<"m_hostAddr"<<m_hostAddr;
//    qDebug()<<"m_port"<<m_port;

    if(udpSocket->bind(QHostAddress(ip), port))
    {
        udp_bind_flag = true;
//        QByteArray data = udpSocket->readAll(); // 读取并丢弃所有数据
//        timer.start();
//        qDebug() << "UDP bind successful";
    }
    else
    {
        udp_bind_flag = false;
//        qDebug() << "UDP bind failed: " << udpSocket->errorString();
    }
//    qDebug()<<"开启udp_bind线程"<<QThread::currentThread();//查看槽函数在哪个线程运行
//    qDebug() << "udpSocket thread:" << udpSocket->thread();
}

void Udp_Thread::udp_send_data(QString sendbuf,QString ip,qint16 port)//如果想将它放入子线程中可以考虑将函数在udp_bind中用connect连接
{
    QHostAddress address;
    address.setAddress(ip);
    udpSocket->writeDatagram(sendbuf.toLocal8Bit().data(),sendbuf.length(),address,port);
    //qDebug()<<"开启udp_send_data线程"<<QThread::currentThreadId();//查看槽函数在哪个线程运行

}


void Udp_Thread::udp_close()
{
    if (udpSocket) {
        udpSocket->close();
        delete udpSocket; // 释放内存
        udpSocket = nullptr; // 防止悬空指针
        udp_bind_flag = false;
    }
}

void Udp_Thread::on_readyReadData()
{
    int row = 0;        //分辨率列
    int col = 0;        //分辨率行
    int frame = 0;      //当前帧数
    int row_index = 0;  //  当前所在行
    static int pic_index = 0;   // 静态变量保持状态

//    ushort pic2[2][270*270];
    while(udpSocket->hasPendingDatagrams())
    {
        QByteArray receivebyte;
        receivebyte.resize(udpSocket->pendingDatagramSize());
        qint64 len = udpSocket->readDatagram(receivebyte.data(),receivebyte.size());

        if (len <= 0) continue;
//        QString buf;
//        buf = receivebyte.data();
//        emit recvDataSignal(buf);
//        qDebug()<<buf;
//        receivebyte = QByteArray::fromHex(receivebyte);     //将ASCII 格式的十六进制字符串转换为十六进制
//        std::vector<uint8_t> hexArray(receivebyte.begin(), receivebyte.end());

        uint8_t hexArray[4096];
        if (receivebyte.size() > 4096) {
            qDebug() << "UDP packet too large, size:" << receivebyte.size();
            continue; // 跳过处理
        }
        memcpy(hexArray, receivebyte.constData(), receivebyte.size());

        // -----------------------------
        // 解析出每次的head
        if (!(hexArray[0] == 0x00 && hexArray[1] == 0x00 &&
              hexArray[2] == 0x90 && hexArray[3] == 0xEB)) {
            qDebug() << "Invalid UDP packet header, discarded";
            continue; // 直接丢弃，进入下一次循环
        }

        col = (static_cast<uint16_t>(hexArray[5])<<8)|hexArray[4];
        row = (static_cast<uint16_t>(hexArray[7])<<8)|hexArray[6];
        frame = (static_cast<uint16_t>(hexArray[9])<<8)|hexArray[8];
        row_index = (static_cast<uint16_t>(hexArray[11])<<8)|hexArray[10];
        // 检查参数合法性
        if (row_index < 1 || row_index > row || col <= 0 ) {
            qDebug() << "Invalid col_index or col value";
            continue;
        }
        //检测是否丢帧，但是没有加入舍弃当前帧所有行
        if((current_row+1)%row == row_index%row)
        {
            current_row = row_index;
        }
        else
        {
            qDebug()<<"frame:"<<frame<<"col error:"<<current_row<<"row_index"<<row_index;
            current_row = row_index;
        }
        Udp_Thread::ROW_FPGA = row;
        Udp_Thread::COL_FPGA = col;

        memcpy(Udp_Thread::pic[pic_index]+(row_index-1)*col,hexArray+16,2*col);
//                memcpy(pic2[pic_index]+(row_index-1)*col,hexArray+16,2*col);
        if(row_index == row)
        {
            row_index = 0;
            Udp_Thread::valid_pic = pic_index;
            pic_index = (pic_index + 1) % 2; // 双缓存，切换缓冲区

            // 获取运行时间（毫秒）
//                    elapsedMilliseconds = (double)timer.nsecsElapsed()/(double)1000000;
//                    timer.start();
            // 打印运行时间
//                    qDebug() << "Elapsed time:" << elapsedMilliseconds << "milliseconds";
            /**************数据流保存**********************/
            if(stream_save_flag==1)
            {
                if (!fileOpened)
                {
                    auto path = this->save_path + '/' + QDateTime::currentDateTime().toString("hh_mm_ss-yyyy-MM-dd") + "-udp_pic.raw";
                    file.setFileName(path);
                    if (file.open(QIODevice::WriteOnly))
                    {
                        fileOpened = true;
                        qDebug() << "File opened successfully.";
                    } else {
                        qDebug() << "Failed to open file for writing!";
                        continue; // 跳过本次循环，继续处理 UDP 数据
                    }
                }
                // 写入文件
                if (fileOpened) {
                    QDataStream aStream(&file);
                    aStream.setByteOrder(QDataStream::LittleEndian);
                    aStream.writeRawData(reinterpret_cast<const char*>(Udp_Thread::pic[Udp_Thread::valid_pic]), 128*128*2);
//                    qDebug() << "Data written to file. Length:" << len;
                }
            }
            else{
                if(fileOpened){
                    file.close();
                    fileOpened = false; // 文件已关闭
                    qDebug() << "File closed because stream_save_flag is false.";
                }
            }
            /**************数据流保存**********************/
            emit updataUDPpic();
            /**************帧率计算**********************/
//            static qint64 lastTime = QDateTime::currentMSecsSinceEpoch(); // 获取当前时间（毫秒）

//            qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
//            qint64 timeDiff = currentTime - lastTime;
//            lastTime = currentTime;

//            // 计算 FPS（避免 timeDiff = 0）
//            if (timeDiff > 0) {
//                double fps = 1000.0 / timeDiff;
//                qDebug() << "Frame Time:" << timeDiff << "ms, FPS:" << fps;
//            }
        }
    }

    //qDebug()<<"开启on_readyReadData线程"<<QThread::currentThreadId();//查看槽函数在哪个线程运行
}


