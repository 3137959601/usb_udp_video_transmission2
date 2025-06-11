#ifndef WIDGET_IMAGE_H
#define WIDGET_IMAGE_H

#include <QWidget>
#include <QPainter>
#include <QElapsedTimer>
#include "largeimage_window.h"
#include <QFile>
#include <QFileDialog>
#include <QMouseEvent>

class widget_image : public QWidget
{
    Q_OBJECT
public:

    explicit widget_image(QWidget *parent = nullptr);
    virtual ~ widget_image();   //如果没有虚析构函数，派生类的析构逻辑被跳过，可能导致资源泄漏

    void repaintImage();
    void setSaveDir(const QString& path);

    QElapsedTimer signalTimer; // 用于计时信号发送间隔的定时器
signals:
    void imageUpdated(const QImage &img); // 新增带参数的图像更新信号
public slots:
    void onResolutionChanged(ushort newWidth, ushort newHeight) {
        // 调整窗口大小以适应新的分辨率
        resize(newWidth, newHeight);
        // 更新图像显示
        update();
    }

protected:
    virtual QImage getCurrentImage() const = 0; //​值返回​（副本）	调用者获得独立拷贝，安全但可能有拷贝开销	需要隔离原始数据时使用
//    const QImage& getDisplayImage()const;    //​常量引用	直接返回内部引用，零拷贝但需注意生命周期	仅访问不修改时使用
    virtual void saveCurrentImage(const QString &filePath) = 0;

    void paintEvent(QPaintEvent *e);
    void mousePressEvent(QMouseEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;   //右键菜单

private:
    void saveImage();
    LargeImageWindow* largeWindow = nullptr;

//    ImageSource m_source;
//    mutable QImage m_displayImage;
//    mutable int m_lastWidth;
//    mutable int m_lastHeight;
    QString save_path;
};

#endif // WIDGET_IMAGE_H
