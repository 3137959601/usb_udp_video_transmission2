#ifndef TOOLWINDOW_H
#define TOOLWINDOW_H

#include <QWidget>

namespace Ui {
class toolwindow;
}

class toolwindow : public QWidget
{
    Q_OBJECT

public:
    explicit toolwindow(QWidget *parent = nullptr);
    ~toolwindow();

private:
    Ui::toolwindow *ui;
};

#endif // TOOLWINDOW_H
