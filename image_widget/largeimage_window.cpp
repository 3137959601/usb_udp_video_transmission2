#include "largeimage_window.h"

LargeImageWindow::LargeImageWindow(const QImage &image, QWidget *parent)
    : QWidget(parent)
    , m_image(image)
    , mousePositionLabel(new QLabel(this))
{
    setWindowTitle("放大图像");

    // 计算放大后的尺寸，保持宽高比
    setFixedSize(calculateOptimalSize(image,512));

    setMouseTracking(true); // 启用鼠标跟踪，即使不点击也能检测鼠标移动

    // 设置 QLabel 样式，使其显示在窗口左上角
    mousePositionLabel->setStyleSheet("background-color: rgba(0, 0, 0, 50); color: white; padding: 3px;");
    mousePositionLabel->setFixedSize(150, 20);
    mousePositionLabel->setVisible(false); // 隐藏坐标标签
}
void LargeImageWindow::contextMenuEvent(QContextMenuEvent *event) {
    QMenu menu;
//    menu.addAction("保存图像", this, &LargeImageWindow::saveImage);
    menu.addAction("放大", this, &LargeImageWindow::zoomIn);
    menu.addAction("缩小", this, &LargeImageWindow::zoomOut);

    menu.exec(event->globalPos());
}
void LargeImageWindow::mouseMoveEvent(QMouseEvent *event)
{
    refreshMouseLabel(event->pos());
}

void LargeImageWindow::refreshMouseLabel(const QPoint &pos)
{
    // 将窗口坐标映射到图像像素坐标
    int xscale = width() / m_image.width();
    int yscale = height() / m_image.height();
    int x = pos.x() / xscale + 1;
    int y = pos.y() / yscale + 1;
    if (x < 1 || x > m_raw12.width() || y < 1 || y > m_raw12.height())
        return;
    //    if (m_image.format() == QImage::Format_Grayscale8) {
    //        pixelInfo = QString("X: %1, Y: %2, Gray: %3").arg(x).arg(y).arg(color.red());
    //    } else {
    ////        pixelInfo = QString("X: %1, Y: %2")
    ////                        .arg(x).arg(y)
    ////                        .arg(color.red())
    ////                        .arg(color.green())
    ////                        .arg(color.blue());
    //        pixelInfo = QString("X: %1, Y: %2")
    //                        .arg(x).arg(y);
    //    }
    //    mousePositionLabel->setText(pixelInfo);
    // 读取 12 位像素值
    const ushort *buf = reinterpret_cast<const ushort*>(m_raw12.constBits());
    ushort val12 = buf[(y-1) * m_raw12.width() + (x-1)];
    mousePositionLabel->setText(
        QString("X:%1 Y:%2 Gray:%3").arg(x).arg(y).arg(val12)
    );

    // 让 QLabel 贴着鼠标
    // QLabel 的大小
    int labelWidth = mousePositionLabel->width();
    int labelHeight = mousePositionLabel->height();

    // 窗口的大小
    int windowWidth = this->width();
    int windowHeight = this->height();

    // 计算 QLabel 的新位置（优先右下角）
    int newX = pos.x() + 10;
    int newY = pos.y() + 10;

    // **防止 QLabel 超出右边界**
    if (newX + labelWidth > windowWidth) {
        newX = pos.x() - labelWidth - 10; // 移动到鼠标左侧
    }

    // **防止 QLabel 超出下边界**
    if (newY + labelHeight > windowHeight) {
        newY = pos.y() - labelHeight - 10; // 移动到鼠标上方
    }

    // 更新 QLabel 的位置
    mousePositionLabel->move(newX, newY);
    mousePositionLabel->setVisible(true);
}
void LargeImageWindow::enterEvent(QEnterEvent *event)
{
    // 鼠标进入窗口时显示 QLabel
    mousePositionLabel->setVisible(true); // 显示坐标标签
    QWidget::enterEvent(event);
}

void LargeImageWindow::leaveEvent(QEvent *event)
{

    // 鼠标离开窗口时隐藏 QLabel
    mousePositionLabel->setVisible(false); // 隐藏坐标标签
    QWidget::leaveEvent(event);
}
QSize LargeImageWindow::calculateOptimalSize(const QImage &img, int maxSize)
{
    QSize newSize = img.size();
    if (img.width() > img.height()) {
        newSize.scale(maxSize, maxSize * img.height() / img.width(), Qt::KeepAspectRatio);
    } else {
        newSize.scale(maxSize * img.width() / img.height(), maxSize, Qt::KeepAspectRatio);
    }
    return newSize;
}
void LargeImageWindow::zoomIn() {
    m_zoomFactor *= 2;
    setFixedSize(calculateOptimalSize(m_image) * m_zoomFactor);
}

void LargeImageWindow::zoomOut() {
    m_zoomFactor /= 2;
    setFixedSize(calculateOptimalSize(m_image) * m_zoomFactor);
}
void LargeImageWindow::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);

    // 使用平滑变换绘制放大后的图像
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    painter.drawImage(rect(), m_image.scaled(size(), Qt::KeepAspectRatio, Qt::FastTransformation));
}
void LargeImageWindow::updateImage(const QImage &newImage) {
    m_image = newImage; // QImage 的隐式共享机制保证高效
    update(); // 触发重绘
    //    qDebug()<<"LargeImageWindow:"<<QThread::currentThreadId();
}

void LargeImageWindow::updateImage12(const QImage &img12)
{
    m_raw12 = img12;
    // 如果鼠标此刻在窗口内，则刷新标签
    if (mousePositionLabel->isVisible()) {
        // 全局鼠标位置 --映射到-> widget 坐标
        QPoint wpos = mapFromGlobal(QCursor::pos());
        if (rect().contains(wpos))  //如果鼠标在窗口内则更新它，否则不处理
            refreshMouseLabel(wpos);
    }
}

