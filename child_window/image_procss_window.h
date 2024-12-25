#ifndef IMAGE_PROCSS_WINDOW_H
#define IMAGE_PROCSS_WINDOW_H

#include <QWidget>

namespace Ui {
class image_procss_window;
}

class image_procss_window : public QWidget
{
    Q_OBJECT

public:
    explicit image_procss_window(QWidget *parent = nullptr);
    ~image_procss_window();

private:
    Ui::image_procss_window *ui;
};

#endif // IMAGE_PROCSS_WINDOW_H
