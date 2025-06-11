#include "udp_widget_image.h"
#include "udp_thread.h"

udp_widget_image::udp_widget_image(QWidget *parent)
    : widget_image{parent}
{
    int w = Udp_Thread::COL_FPGA;
    int h = Udp_Thread::ROW_FPGA;
    // 一开始就分配个空灰度图，避免 null 警告
    m_displayImage = QImage(w, h, QImage::Format_Grayscale8);
    m_displayImage.fill(Qt::black);
    m_lastWidth = w;
    m_lastHeight = h;
}
void udp_widget_image::onProcessedImage(const QImage &img)
{
    // img 为 12 位灰度图
    m_raw12 = img;

    // 转 8 位
    int w = img.width();
    int h = img.height();
    if (m_displayImage.size() != img.size()) {
        m_displayImage = QImage(w, h, QImage::Format_Grayscale8);
    }
    // 直接写入 m_display8 的 bits，避免中间缓冲
    const ushort *src = reinterpret_cast<const ushort*>(img.constBits());
    uchar *dst = m_displayImage.bits();
    int total = w * h;
    //感觉这样写效率比较低，存在优化的空间
    for (int i = 0; i < total; ++i) {
        dst[i] = static_cast<uchar>(src[i] >> 4);
    }

    // 刷新自己,通知 widget_image
    repaintImage();

    // 通知大窗口，发送12位未压缩的数据
//    emit image8Updated(m_displayImage);
    emit raw12Updated(m_raw12);
}

//void udp_widget_image::onProcessedImage(const QImage &img)
//{
//    m_displayImage = img;
////    update();
//    repaintImage();
//}

QImage udp_widget_image::getCurrentImage() const
{
    // 从UDP线程获取图像数据,直接16位显示
//    QImage image(reinterpret_cast<const uchar*>(Udp_Thread::pic[Udp_Thread::valid_pic]),
//                 Udp_Thread::COL_FPGA, Udp_Thread::ROW_FPGA,
//                 Udp_Thread::COL_FPGA * sizeof(ushort),
//                 QImage::Format_Grayscale16);

//    int currentWidth = Udp_Thread::COL_FPGA;
//    int currentHeight = Udp_Thread::ROW_FPGA;

//    // 如果尺寸变化，则重新分配图像
//    if (currentWidth != m_lastWidth  || currentHeight != m_lastHeight) {
//        // 创建8位灰度图像用于显示
//        m_displayImage = QImage(Udp_Thread::COL_FPGA, Udp_Thread::ROW_FPGA, QImage::Format_Grayscale8);
//        m_lastWidth  = currentWidth;
//        m_lastHeight = currentHeight;
//    }

//    // 提取高八位数据
//    for (int y = 0; y < m_displayImage.height(); ++y) {
//        const ushort *srcLine = reinterpret_cast<const ushort*>(Udp_Thread::pic[Udp_Thread::valid_pic] + y * Udp_Thread::COL_FPGA);
//        uchar *dstLine = m_displayImage.scanLine(y);
//        for (int x = 0; x < m_displayImage.width(); ++x) {
//            dstLine[x] = static_cast<uchar>(srcLine[x] >> 8);            // 提取高八位
//        }
//    }
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
