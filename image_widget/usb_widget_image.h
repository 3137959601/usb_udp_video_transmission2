#ifndef USB_WIDGET_IMAGE_H
#define USB_WIDGET_IMAGE_H

#include "widget_image.h"
#include <QMessageBox>
#include <QMutexLocker>

class usb_widget_image : public widget_image
{
    Q_OBJECT
public:
    explicit usb_widget_image(QWidget *parent = nullptr);

protected:
    QImage getCurrentImage() const override;
    void saveCurrentImage(const QString &filePath) override;

private:
    // 缓存图像（mutable允许const方法修改）
    mutable QImage m_displayImage;
//    mutable QMutex m_cacheMutex;  // 保护缓存
    mutable int m_lastWidth;
    mutable int m_lastHeight;
};

#endif // USB_WIDGET_IMAGE_H
