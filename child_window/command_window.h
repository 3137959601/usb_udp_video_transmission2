#ifndef COMMAND_WINDOW_H
#define COMMAND_WINDOW_H

#include <QWidget>
#include "serialworker.h"
#include <QButtonGroup>

namespace Ui {
class command_window;
}

class command_window : public QWidget
{
    Q_OBJECT

public:
    explicit command_window(QWidget *parent = nullptr);
    ~command_window();

signals:
    void sensor_signal(unsigned char flag,QList<float>value);
private slots:
    void on_cmd_set_pB_clicked();

    void on_on_set_pB_clicked();

    void on_off_set_pB_clicked();

private:
    Ui::command_window *ui;
};

#endif // COMMAND_WINDOW_H
