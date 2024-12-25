#ifndef USBTHREAD_H
#define USBTHREAD_H
#include "qobject.h"

class usbThread :public QObject
{
    Q_OBJECT
public:
    usbThread(QObject *parent = 0);

signals:
    void complete();
    void updatapic();

public slots:
    int Xfer();//数据传输和解析
};

#endif // USBTHREAD_H
