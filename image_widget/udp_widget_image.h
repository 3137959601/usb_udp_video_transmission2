#ifndef UDP_WIDGET_IMAGE_H
#define UDP_WIDGET_IMAGE_H

#include "widget_image.h"
#include <QMessageBox>

class udp_widget_image : public widget_image
{
    Q_OBJECT
public:
    explicit udp_widget_image(QWidget *parent = nullptr);
public slots:
    // 接收图像处理线程的图像
    void onProcessedImage(const QImage &img);

signals:
    void image8Updated(const QImage &img8);   // 用于大窗口更新
    void raw12Updated(const QImage &img12);   // 用于大窗口更新

protected:
    QImage getCurrentImage() const override;
    void saveCurrentImage(const QString &filePath) override;

private:
    mutable QImage m_displayImage;
    mutable int m_lastWidth;
    mutable int m_lastHeight;
    QImage    m_raw12;

};

#endif // UDP_WIDGET_IMAGE_H
