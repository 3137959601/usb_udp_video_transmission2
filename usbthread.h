#ifndef USBTHREAD_H
#define USBTHREAD_H
#include "qobject.h"
#include <QFile>
#include<QFileDialog>

class usbThread :public QObject
{
    Q_OBJECT
public:
    usbThread(QObject *parent = 0);
    void stream_save();
    void setSaveDir(const QString& path);

    bool stream_save_flag = false;
    QFile file;
    QString save_path;
    bool fileOpened = false; // 标记文件是否已打开

    static  QImage image;
    static ushort COL_FPGA;
    static ushort ROW_FPGA;
    static int valid_pic;
    static uchar usbpic[2][256*256];//双缓冲区;
signals:
    void complete();
    void updatapic();

public slots:
    int Xfer();//数据传输和解析
};

#endif // USBTHREAD_H
