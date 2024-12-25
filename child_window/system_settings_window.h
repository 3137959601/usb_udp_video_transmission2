#ifndef SYSTEM_SETTINGS_WINDOW_H
#define SYSTEM_SETTINGS_WINDOW_H

#include <QWidget>

namespace Ui {
class system_settings_window;
}

class system_settings_window : public QWidget
{
    Q_OBJECT

public:
    explicit system_settings_window(QWidget *parent = nullptr);
    ~system_settings_window();

private:
    Ui::system_settings_window *ui;
};

#endif // SYSTEM_SETTINGS_WINDOW_H
