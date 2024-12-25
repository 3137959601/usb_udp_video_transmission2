#include "toolwindow.h"
#include "ui_toolwindow.h"

toolwindow::toolwindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::toolwindow)
{
    ui->setupUi(this);
}

toolwindow::~toolwindow()
{
    delete ui;
}
