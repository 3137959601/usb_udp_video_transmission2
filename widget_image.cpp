#include "widget_image.h"
#include "mainwindow.h"

widget_image::widget_image(QWidget *parent) : QWidget(parent)
{

}

widget_image::~widget_image()
{

}
//QImage widget_image::image = QImage(640,512,QImage::Format_Grayscale16);
//short widget_image::pic[512][640];

void widget_image::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e); // 这个宏用于明确指出该参数未被使用,表明故意忽略了此参数。由于 paintEvent 是一个重写的虚函数，不能简单地移除它
    QPainter painter(this);
//    painter.drawImage(0,0,MainWindow::image);
    //使640*512图像缩小至128*128的窗口大小
//    QImage scaledImage = MainWindow::image.scaled(size());
//    painter.drawImage(0,0,scaledImage);
    QImage image(reinterpret_cast<const uchar*>(usbThread::usbpic[usbThread::valid_pic]),
                    128, 128,
                    128 * sizeof(uchar),  // 行步长（每行的字节数）
                    QImage::Format_Grayscale8); // 16-bit 灰度图像
    painter.drawImage(0,0,image);
}
void widget_image::repaintImage()
{

    update();
}
