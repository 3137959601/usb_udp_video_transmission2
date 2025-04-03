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
    mousePositionLabel->setFixedSize(100, 20);
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
    // 获取鼠标的 x, y 坐标,默认窗口为512是原窗口的4倍，所以先除以4
    //目前受浮点精度误差累积的影响只能计算放大窗口及大于512的正确坐标

    int x = event->pos().x() /4/ m_zoomFactor+1;  // 四舍五入
    int y = event->pos().y() /4/ m_zoomFactor+1;  // 四舍五入
    // 更新 QLabel 显示坐标
    mousePositionLabel->setText(QString("X: %1, Y: %2").arg(x).arg(y));

    // 让 QLabel 贴着鼠标
    // QLabel 的大小
    int labelWidth = mousePositionLabel->width();
    int labelHeight = mousePositionLabel->height();

    // 窗口的大小
    int windowWidth = this->width();
    int windowHeight = this->height();

    // 计算 QLabel 的新位置（优先右下角）
    int newX = event->pos().x() + 10;
    int newY = event->pos().y() + 10;

    // **防止 QLabel 超出右边界**
    if (newX + labelWidth > windowWidth) {
        newX = event->pos().x() - labelWidth - 10; // 移动到鼠标左侧
    }

    // **防止 QLabel 超出下边界**
    if (newY + labelHeight > windowHeight) {
        newY = event->pos().y() - labelHeight - 10; // 移动到鼠标上方
    }

    // 更新 QLabel 的位置
    mousePositionLabel->move(newX, newY);
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
