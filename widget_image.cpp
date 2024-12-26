#include "widget_image.h"
#include "mainwindow.h"

widget_image::widget_image(QWidget *parent) : QWidget(parent)
{

}

widget_image::~widget_image()
{

}
QImage widget_image::image = QImage(640,512,QImage::Format_Grayscale16);
short widget_image::pic[512][640];

void widget_image::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);
    painter.drawImage(0,0,MainWindow::image);
    //使640*512图像缩小至128*128的窗口大小
//    QImage scaledImage = MainWindow::image.scaled(size());
//    painter.drawImage(0,0,scaledImage);

}
void widget_image::repaintImage()
{

    update();
}
