#include "usb_widget_image.h"
#include "usbthread.h"


usb_widget_image::usb_widget_image(QWidget *parent)
    : widget_image{parent}
{

}


QImage usb_widget_image::getCurrentImage() const
{
//    QImage image(reinterpret_cast<const uchar*>(usbThread::usbpic[usbThread::valid_pic]),
//                usbThread::COL_FPGA, usbThread::ROW_FPGA,
//                usbThread::COL_FPGA * sizeof(uchar),
//                QImage::Format_Indexed8);
////     设置调色板
//    image.setColorTable(s_colorTable);
    // 1. 加锁保护缓存操作
//    QMutexLocker locker(&m_cacheMutex);
    // 2.若缓存无效或尺寸变化，重建图像
    if (m_displayImage.isNull() ||
        m_displayImage.width() != usbThread::COL_FPGA ||
        m_displayImage.height() != usbThread::ROW_FPGA)
    {
        m_displayImage = QImage(
            reinterpret_cast<const uchar*>(usbThread::usbpic[usbThread::valid_pic]),
            usbThread::COL_FPGA,
            usbThread::ROW_FPGA,
            usbThread::COL_FPGA * sizeof(uchar),
            QImage::Format_Indexed8
        );
        QVector<QRgb> colorTable;
        colorTable.append(qRgb(0, 0, 0));       // 值 0: 黑色
        colorTable.append(qRgb(255, 0, 0));     // 值 1: 红色
        colorTable.append(qRgb(0, 255, 0));     // 值 2: 绿色
        colorTable.append(qRgb(0, 0, 255));     // 值 3: 蓝色
        m_displayImage.setColorTable(colorTable);  // 应用静态调色板
    }
    // 3.直接更新缓存数据（避免重复构造QImage）
    else {
        memcpy(m_displayImage.bits(),
               usbThread::usbpic[usbThread::valid_pic],
               m_displayImage.sizeInBytes());
    }

    return m_displayImage;
}

void usb_widget_image::saveCurrentImage(const QString &filePath)
{
    if (!getCurrentImage().save(filePath)) {
        QMessageBox::warning(this, "错误", "保存USB图像失败！");
    }
}
