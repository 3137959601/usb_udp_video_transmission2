#include "image_procss_window.h"
#include "ui_image_procss_window.h"

image_procss_window::image_procss_window(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::image_procss_window)
{
    ui->setupUi(this);
    this->setWindowTitle("图像处理");
}

image_procss_window::~image_procss_window()
{
    delete ui;
}
