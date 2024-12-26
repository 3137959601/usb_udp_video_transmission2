#ifndef WIDGET_IMAGE_H
#define WIDGET_IMAGE_H

#include <QWidget>
#include <QPainter>
#include <QElapsedTimer>

class widget_image : public QWidget
{
    Q_OBJECT
public:
    explicit widget_image(QWidget *parent = nullptr);
    ~ widget_image();
    void paintEvent(QPaintEvent *e);
    void repaintImage();
    static QImage image;
    static  short pic[512][640];

    QElapsedTimer signalTimer; // 用于计时信号发送间隔的定时器
signals:

};

#endif // WIDGET_IMAGE_H
