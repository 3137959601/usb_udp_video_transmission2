#ifndef UDP_WIDGET_IMAGE_H
#define UDP_WIDGET_IMAGE_H

#include "widget_image.h"
#include <QMessageBox>

class udp_widget_image : public widget_image
{
    Q_OBJECT
public:
    explicit udp_widget_image(QWidget *parent = nullptr);

protected:
    QImage getCurrentImage() const override;
    void saveCurrentImage(const QString &filePath) override;

private:
    mutable QImage m_displayImage;
    mutable int m_lastWidth;
    mutable int m_lastHeight;
};

#endif // UDP_WIDGET_IMAGE_H
