#ifndef UDP_THREAD_H
#define UDP_THREAD_H

#include <QObject>
#include <QMessageBox>
#include <QDebug>
#include <QHostAddress>
#include <QUdpSocket>
#include <QThread>

#include <QFile>
#include <QFileDialog>
class Udp_Thread : public QObject
{
    Q_OBJECT
public:
    explicit Udp_Thread(QObject *parent = nullptr);

    ~Udp_Thread();
    QUdpSocket *udpSocket;

    void udp_bind(QString ip, uint port);
    void udp_send_data(QString data,QString ip,qint16 port);
    void udp_close();

    void setSaveDir(const QString& path);

    static  QImage image;
    static  int valid_pic;//有效图片index
    static ushort COL_FPGA;
    static ushort ROW_FPGA;
    static  ushort pic[2][2700*2700];//双缓冲区

    bool stream_save_flag= false;
    QFile file;
    QString save_path;
    bool fileOpened = false; // 标记文件是否已打开
    //bool udp_bind_flag;
signals:
    //通过该信号传递接收到的数据
    void recvDataSignal(QString sendbuf);
    void updataUDPpic();
public slots:
    //读取数据的槽函数
    void on_readyReadData();

private:
    QHostAddress    m_hostAddr;     //保存目标的地址对象
    QHostAddress    m_targetAddr;     //保存目标的地址对象
    quint16         m_port;         //保存目标的端口号(类型一致)

};

#endif // UDP_THREAD_H
