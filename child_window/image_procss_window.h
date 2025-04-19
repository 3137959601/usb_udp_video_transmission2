#ifndef IMAGE_PROCSS_WINDOW_H
#define IMAGE_PROCSS_WINDOW_H

#include <QWidget>
#include <QDialog>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QDebug>

namespace Ui {
class image_procss_window;
}

class image_procss_window : public QWidget
{
    Q_OBJECT

public:
    explicit image_procss_window(QWidget *parent = nullptr);
    ~image_procss_window();

    bool nonuniformity_correction_status = false;
signals:
    void nonuniformity_correction_signal(unsigned char flag,QList<float>value);
private slots:
    void on_CollectLowTemp_pB_clicked();

    void on_CollectHighTemp_pB_clicked();

    void on_nonunicorrect_pB_clicked();

private:
    Ui::image_procss_window *ui;
    QDialog *dialog;

};

#endif // IMAGE_PROCSS_WINDOW_H
