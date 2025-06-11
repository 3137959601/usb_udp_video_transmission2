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

QPushButton* image_procss_window::getLowRefButton() const {
    return ui->lowRefpB;
}
QPushButton* image_procss_window::getHighRefButton() const {
    return ui->highRefpB;
}
QPushButton* image_procss_window::getTwoPointButton() const {
    return ui->two_point_pB;
}

QPushButton *image_procss_window::getHistEqualButton() const
{
    return ui->histEqual_pB;
}

void image_procss_window::on_lowRefpB_clicked()
{

}


void image_procss_window::on_highRefpB_clicked()
{

}


void image_procss_window::on_two_point_pB_toggled(bool checked)
{
    useCalibrate = checked;
    if(useCalibrate){
        ui->two_point_pB->setText("关闭");
    }else{
        ui->two_point_pB->setText("开启");
    }
}


void image_procss_window::on_histEqual_pB_toggled(bool checked)
{
    useHistEqual = checked;
    if(useHistEqual){
        ui->histEqual_pB->setText("关闭");
    }else{
        ui->histEqual_pB->setText("开启");
    }
}

//void image_procss_window::on_CollectLowTemp_pB_clicked()
//{
//    // 创建对话框
//    QDialog *dialog = new QDialog(this);
//    dialog->resize(300, 120);  // ✅ 设置合适大小
//    dialog->setWindowTitle(tr("确认操作"));

//    // 创建标签
//    QLabel *label = new QLabel("是否采集低温？", dialog);

//    // 创建按钮
//    QPushButton *yesButton = new QPushButton("Yes", dialog);
//    QPushButton *noButton = new QPushButton("No", dialog);

//    // 按钮布局
//    QHBoxLayout *buttonLayout = new QHBoxLayout;
//    buttonLayout->addStretch();
//    buttonLayout->addWidget(yesButton);
//    buttonLayout->addWidget(noButton);

//    // 总布局
//    QVBoxLayout *mainLayout = new QVBoxLayout(dialog);
//    mainLayout->addWidget(label);
//    mainLayout->addLayout(buttonLayout);

//    // 连接按钮信号
//    connect(yesButton, &QPushButton::clicked, this, [=]() {
//        qDebug() << "发送指令...";  // 这里替换为你的发送指令逻辑
//        QList<float> value;
//        value.append(0.0);
//        nonuniformity_correction_signal(0xE1,value);
//        dialog->accept();  // 关闭对话框
//    });

//    connect(noButton, &QPushButton::clicked, dialog, &QDialog::reject);  // 关闭对话框

//    // 执行对话框
//    dialog->exec();
//}


//void image_procss_window::on_CollectHighTemp_pB_clicked()
//{
//    // 创建对话框
//    QDialog *dialog = new QDialog(this);
//    dialog->resize(300, 120);  // ✅ 设置合适大小
//    dialog->setWindowTitle(tr("确认操作"));

//    // 创建标签
//    QLabel *label = new QLabel("是否采集高温？", dialog);

//    // 创建按钮
//    QPushButton *yesButton = new QPushButton("Yes", dialog);
//    QPushButton *noButton = new QPushButton("No", dialog);

//    // 按钮布局
//    QHBoxLayout *buttonLayout = new QHBoxLayout;
//    buttonLayout->addStretch();
//    buttonLayout->addWidget(yesButton);
//    buttonLayout->addWidget(noButton);

//    // 总布局
//    QVBoxLayout *mainLayout = new QVBoxLayout(dialog);
//    mainLayout->addWidget(label);
//    mainLayout->addLayout(buttonLayout);

//    // 连接按钮信号
//    connect(yesButton, &QPushButton::clicked, this, [=]() {
//        qDebug() << "发送指令...";  // 这里替换为你的发送指令逻辑
//        QList<float> value;
//        value.append(0.0);
//        nonuniformity_correction_signal(0xE2,value);
//        dialog->accept();  // 关闭对话框
//    });

//    connect(noButton, &QPushButton::clicked, dialog, &QDialog::reject);  // 关闭对话框

//    // 执行对话框
//    dialog->exec();
//}


//void image_procss_window::on_nonunicorrect_pB_clicked()
//{
//    if(nonuniformity_correction_status==false)
//    {
//        QList<float> value;
//        value.append(0.0);
//        nonuniformity_correction_signal(0xE3,value);
//        nonuniformity_correction_status = true;
//        ui->nonunicorrect_pB->setText("关闭");
//    }
//    else if(nonuniformity_correction_status==true)
//    {

//        QList<float> value;
//        value.append(0.0);
//        nonuniformity_correction_signal(0xE4,value);
//        nonuniformity_correction_status = false;
//        ui->nonunicorrect_pB->setText("启动");
//    }

//}



