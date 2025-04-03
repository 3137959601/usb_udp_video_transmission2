#include "udp_widget_image.h"
#include "udp_thread.h"

udp_widget_image::udp_widget_image(QWidget *parent)
    : widget_image{parent}
{

}

//QImage udp_widget_image::getCurrentImage() const
//{
//    QImage image(Udp_Thread::COL_FPGA, Udp_Thread::ROW_FPGA, QImage::Format_Grayscale8);

//    // 这里需要根据您的实际UDP数据处理逻辑填充图像
//    // 示例代码：
//    uchar *bits = image.bits();
//    for (int i = 0; i < Udp_Thread::COL_FPGA * Udp_Thread::ROW_FPGA; ++i) {
//        bits[i] = Udp_Thread::pic[i];  // 假设pic是您的UDP图像数据
//    }

//    return image;
//}
QImage udp_widget_image::getCurrentImage() const
{
    // 从UDP线程获取图像数据,直接16位显示
//    QImage image(reinterpret_cast<const uchar*>(Udp_Thread::pic[Udp_Thread::valid_pic]),
//                 Udp_Thread::COL_FPGA, Udp_Thread::ROW_FPGA,
//                 Udp_Thread::COL_FPGA * sizeof(ushort),
//                 QImage::Format_Grayscale16);
    int currentWidth = Udp_Thread::COL_FPGA;
    int currentHeight = Udp_Thread::ROW_FPGA;

    // 如果尺寸变化，则重新分配图像
    if (currentWidth != m_lastWidth  || currentHeight != m_lastHeight) {
        // 创建8位灰度图像用于显示
        m_displayImage = QImage(Udp_Thread::COL_FPGA, Udp_Thread::ROW_FPGA, QImage::Format_Grayscale8);
        m_lastWidth  = currentWidth;
        m_lastHeight = currentHeight;
    }

    // 提取高八位数据
    for (int y = 0; y < m_displayImage.height(); ++y) {
        const ushort *srcLine = reinterpret_cast<const ushort*>(Udp_Thread::pic[Udp_Thread::valid_pic] + y * Udp_Thread::COL_FPGA);
        uchar *dstLine = m_displayImage.scanLine(y);
        for (int x = 0; x < m_displayImage.width(); ++x) {
            dstLine[x] = static_cast<uchar>(srcLine[x] >> 8);            // 提取高八位
//            dstLine[x] = static_cast<uchar>((srcLine[x] >> 4) & 0xFF); // 提取中间八位
        }
    }
/*************************帧率计算**********************************/
//    static qint64 lastTime = QDateTime::currentMSecsSinceEpoch(); // 获取当前时间（毫秒）

//    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
//    qint64 timeDiff = currentTime - lastTime;
//    lastTime = currentTime;

//    // 计算 FPS（避免 timeDiff = 0）
//    if (timeDiff > 0) {
//        double fps = 1000.0 / timeDiff;
//        qDebug() << "Frame Time:" << timeDiff << "ms, FPS:" << fps;
//    }
/*************************帧率计算**********************************/
    return m_displayImage;
}
void udp_widget_image::saveCurrentImage(const QString &filePath)
{
    if (!getCurrentImage().save(filePath)) {
        QMessageBox::warning(this, "错误", "保存UDP图像失败！");
    }
}
