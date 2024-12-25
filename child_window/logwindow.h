#ifndef LOGWINDOW_H
#define LOGWINDOW_H

#include <QWidget>

namespace Ui {
class logwindow;
}

class logwindow : public QWidget
{
    Q_OBJECT

public:
    explicit logwindow(QWidget *parent = nullptr);
    ~logwindow();

private slots:
    void on_browseBt_clicked();

    void on_saveBt_clicked();

private:
    Ui::logwindow *ui;
};

#endif // LOGWINDOW_H
