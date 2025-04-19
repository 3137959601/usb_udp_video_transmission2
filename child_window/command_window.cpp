#include "command_window.h"
#include "ui_command_window.h"

command_window::command_window(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::command_window)
{
    ui->setupUi(this);
    QButtonGroup *group1 = new QButtonGroup(this);
    QButtonGroup *group2 = new QButtonGroup(this);
    QButtonGroup *group3 = new QButtonGroup(this);
    QButtonGroup *group4 = new QButtonGroup(this);

    group1->addButton(ui->on_Denos_on_rB);
    group1->addButton(ui->on_Denos_off_rB);
    group2->addButton(ui->on_SlidWD_3_rB);
    group2->addButton(ui->on_SlidWD_5_rB);

    group3->addButton(ui->off_Denos_on_rB);
    group3->addButton(ui->off_Denos_off_rB);
    group4->addButton(ui->off_SlidWD_3_rB);
    group4->addButton(ui->off_SlidWD_5_rB);

    ui->on_Denos_off_rB->setChecked(true);
    ui->on_SlidWD_3_rB->setChecked(true);
    ui->off_Denos_off_rB->setChecked(true);
    ui->off_SlidWD_3_rB->setChecked(true);
}

command_window::~command_window()
{
    delete ui;
}


void command_window::on_cmd_set_pB_clicked()
{
    QList<float> value;
    // 获取六个 QCheckBox 状态
    unsigned char flag = 0;

    flag |= (ui->EN_DVS_CB->isChecked() ? 1 << 5 : 0); // bit 5
    flag |= (ui->EN_APS_CB->isChecked()   ? 1 << 4 : 0); // bit 4
    flag |= (ui->DVS_GAIN_CB->isChecked()  ? 1 << 3 : 0); // bit 3
    flag |= (ui->DI_CAP_SW_CB->isChecked()     ? 1 << 2 : 0); // bit 2
    flag |= (ui->VGPOL_SW_CB->isChecked()     ? 1 << 1 : 0); // bit 1
    flag |= (ui->DVS_TOP_RSTN_CB->isChecked()   ? 1 << 0 : 0); // bit 0
    float flagFloat = static_cast<float>(flag);
//    qDebug() << "flagFloat : " << flagFloat;
    value.append(flagFloat);
//    qDebug() << "Value List: " << value;

    // 发送信号
    emit sensor_signal(0xEE, value);

}


void command_window::on_on_set_pB_clicked()
{
    unsigned char flag = 0;
    QList<float> value;
    flag |= (ui->on_Denos_on_rB->isChecked()?1<<6:0);
    flag |= (ui->on_SlidWD_5_rB->isChecked()?1<<5:0);
    flag |= ui->on_DenosTrsd_lE->text().toInt()&0x1F;   //输出超过5位就溢出
    float flagFloat = static_cast<float>(flag);
    qDebug() << "flagFloat : " << flagFloat;

    value.append(flagFloat);
//    qDebug() << "Value List: " << value;
    // 发送信号
    emit sensor_signal(0xF1, value);
}


void command_window::on_off_set_pB_clicked()
{
    unsigned char flag = 0;
    QList<float> value;
    flag |= (ui->off_Denos_on_rB->isChecked()?1<<6:0);
    flag |= (ui->off_SlidWD_5_rB->isChecked()?1<<5:0);
    flag |= ui->off_DenosTrsd_lE->text().toInt()&0x1F;   //输出超过5位就溢出
    float flagFloat = static_cast<float>(flag);
//    qDebug() << "flagFloat : " << flagFloat;

    value.append(flagFloat);
//    qDebug() << "Value List: " << value;
    // 发送信号
    emit sensor_signal(0xF2, value);
}

