#ifndef LARGEIMAGEWINDOW_H
#define LARGEIMAGEWINDOW_H

#include <QObject>
#include <QWidget>
#include <QImage>
#include <QPainter>
#include <QThread>
#include <QMenu>
#include <QContextMenuEvent>
#include <QLabel>
#include <QElapsedTimer>
#include <QDateTime>

#include "usbthread.h"

class LargeImageWindow: public QWidget
{
    Q_OBJECT
public:
    explicit LargeImageWindow(const QImage &image, QWidget *parent = nullptr);
    static QSize calculateOptimalSize(const QImage& img, int maxSize = 512);
    void zoomIn();
    void zoomOut();

protected:
    void paintEvent(QPaintEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;  // 监听鼠标移动事件
    void refreshMouseLabel(const QPoint &pos);
    void enterEvent(QEnterEvent *event) override;//离开窗口
    void leaveEvent(QEvent *event) override;//进入窗口

private:
    QImage m_image;
    double m_zoomFactor = 1.0;
    QLabel *mousePositionLabel; // 用于显示鼠标坐标

    QImage m_image8;
    QImage m_raw12;

public slots:
    void updateImage(const QImage &newImage); // 新增更新方法
    void updateImage12(const QImage &img12);
};

#endif // LARGEIMAGEWINDOW_H
