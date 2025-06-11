#include "widget_image.h"
#include "mainwindow.h"
#include "udp_widget_image.h"

widget_image::widget_image(QWidget *parent) : QWidget(parent)
{

}

widget_image::~widget_image()
{
    if (largeWindow) {
        largeWindow->deleteLater();
    }
}

void widget_image::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e); // 这个宏用于明确指出该参数未被使用,表明故意忽略了此参数。由于 paintEvent 是一个重写的虚函数，不能简单地移除它
    QPainter painter(this);

    painter.drawImage(rect(), getCurrentImage().scaled(size(), Qt::KeepAspectRatio));

}

void widget_image::repaintImage()
{
    update();
    emit imageUpdated(getCurrentImage()); // 发射带参数的图像更新信号
}


void widget_image::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        // 检查是否已存在大窗口
//        static LargeImageWindow *largeWindow = nullptr;
        if (!largeWindow) {
            QImage dynamicImage = getCurrentImage(); // 获取当前显示图像的拷贝
            largeWindow = new LargeImageWindow(dynamicImage);
            largeWindow->setAttribute(Qt::WA_DeleteOnClose);
            largeWindow->setWindowFlag(Qt::Window);

            //连接 udp_widget_image 的12位图像信号到大窗口槽
            udp_widget_image *udp = qobject_cast<udp_widget_image*>(this);
            if (udp) {  //增加一个12位未压缩图像发送给大窗口，以定位对应坐标的像素值
                connect(udp, &udp_widget_image::raw12Updated,
                        largeWindow, &LargeImageWindow::updateImage12);
            }

            QObject::connect(this, &widget_image::imageUpdated, largeWindow, &LargeImageWindow::updateImage);
            QObject::connect(largeWindow, &QObject::destroyed, [&]() { largeWindow = nullptr; });

        }
        // 强制激活并置顶窗口
        largeWindow->raise();              // 置于顶层
        largeWindow->activateWindow();     // 激活窗口（获取焦点）
        largeWindow->show();
    }
    QWidget::mousePressEvent(event);
//    qDebug()<<"widget_image:"<<QThread::currentThreadId();
}
void widget_image::contextMenuEvent(QContextMenuEvent *event) {
    QMenu menu(this);

    // 添加菜单项
    // 强制设置亮色样式
    menu.setStyleSheet("QMenu { background-color: white; color: black; }"
                       "QMenu::item:selected { background-color: #ddd; }");
    menu.addAction("保存图像", this, [this]() {
        saveImage(); // 需要实现保存功能
    });

    // 显示菜单
    menu.exec(event->globalPos());
}


void widget_image::saveImage() {
    QString defaultName = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");

    QString filePath = QFileDialog::getSaveFileName(
        this,
        "保存图像",
        save_path+ "/" + "gray" + defaultName + ".png",
        "PNG图像 (*.png);;JPEG图像 (*.jpg)"
    );
    if (!filePath.isEmpty()) {
//        if (!getDisplayImage().save(filePath)) {
//            QMessageBox::warning(this, "错误", "保存图像失败！");
//        }
        saveCurrentImage(filePath);
    }
}
void widget_image::setSaveDir(const QString &path)
{
    save_path = path;
}
