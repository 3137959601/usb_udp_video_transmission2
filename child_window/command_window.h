#ifndef COMMAND_WINDOW_H
#define COMMAND_WINDOW_H

#include <QWidget>
#include "serialworker.h"
#include <QButtonGroup>
#include <QSpinBox>
#include "../toolbox/binaryspinbox.h"

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

    void on_FA2415ASetpB_clicked();

    void on_FA2415ArB_toggled(bool checked);

private:
    Ui::command_window *ui;
    bool m_isFa2415ArbSelected = true;
};

#endif // COMMAND_WINDOW_H
