#include "system_settings_window.h"
#include "ui_system_settings_window.h"

system_settings_window::system_settings_window(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::system_settings_window)
{
    ui->setupUi(this);
    this->setWindowTitle("设置");
}

system_settings_window::~system_settings_window()
{
    delete ui;
}
