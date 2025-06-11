#include "command_window.h"
#include "ui_command_window.h"

command_window::command_window(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::command_window)
{
    ui->setupUi(this);
    this->setWindowTitle("SPI指令控制");
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

    /********************************QTableWidget配置******************************/
    // 设置行列数
    ui->tableCommands->setRowCount(25);
    ui->tableCommands->setColumnCount(3);

    ui->tableCommands->setColumnWidth(1, 160);  // 将第二列宽度设为 150 像素
    ui->tableCommands->setColumnWidth(2, 150);  // 将第三列宽度设为 150 像素
    // 合并单元格（row, column, rowspan, colspan）
    ui->tableCommands->setSpan(0, 0, 2, 1);
    ui->tableCommands->setSpan(2, 0, 2, 1);
    ui->tableCommands->setSpan(4, 0, 5, 1);
    ui->tableCommands->setSpan(11, 0, 2, 1);
    ui->tableCommands->setSpan(13, 0, 4, 1);
//    ui->tableCommands->setSpan(17, 1, 4, 1);
//    ui->tableCommands->setSpan(21, 1, 2, 1);
//    ui->tableCommands->setSpan(23, 1, 2, 1);

//    ui->tableCommands->setSpan(17, 2, 4, 1);
//    ui->tableCommands->setSpan(21, 2, 2, 1);
//    ui->tableCommands->setSpan(23, 2, 2, 1);

    // 填充文本（只列出关键行，以下省略号处按同样格式补全）
    ui->tableCommands->setItem(0, 0, new QTableWidgetItem("OUT1<0:7>"));
    ui->tableCommands->setItem(0, 1, new QTableWidgetItem("ROW START<0:6>"));
    ui->tableCommands->setItem(1, 1, new QTableWidgetItem("OPEN WIN"));

    ui->tableCommands->setItem(2, 0, new QTableWidgetItem("OUT2<0:7>"));
    ui->tableCommands->setItem(2, 1, new QTableWidgetItem("COL START<0:6>"));
    ui->tableCommands->setItem(3, 1, new QTableWidgetItem("LIGHT CURRENT"));

    ui->tableCommands->setItem(4, 0, new QTableWidgetItem("OUT3<0:5>"));
    ui->tableCommands->setItem(4, 1, new QTableWidgetItem("SEL<0:1>"));

    ui->tableCommands->setItem(5, 1, new QTableWidgetItem("ADJ_TH"));
    ui->tableCommands->setItem(6, 1, new QTableWidgetItem("EN_DVS"));
    ui->tableCommands->setItem(7, 1, new QTableWidgetItem("EN_APS"));
    ui->tableCommands->setItem(8, 1, new QTableWidgetItem("SPI DVS RONGHE"));

    ui->tableCommands->setItem(9, 0, new QTableWidgetItem("OUT4<3:6>"));
    ui->tableCommands->setItem(9, 1, new QTableWidgetItem("CHOUZHEN START<0:3>"));

    ui->tableCommands->setItem(10, 0, new QTableWidgetItem("OUT5<3:6>"));
    ui->tableCommands->setItem(10, 1, new QTableWidgetItem("CHOUZHEN START<4:7>"));

    ui->tableCommands->setItem(11, 0, new QTableWidgetItem("OUT6<0:7>"));
    ui->tableCommands->setItem(11, 1, new QTableWidgetItem("IN_CTRL<0:3>"));
    ui->tableCommands->setItem(12, 1, new QTableWidgetItem("IP_CTRL<0:3>"));

    ui->tableCommands->setItem(13, 0, new QTableWidgetItem("OUT7<0:6>"));
    ui->tableCommands->setItem(13, 1, new QTableWidgetItem("MUX<0:3>"));
    ui->tableCommands->setItem(14, 1, new QTableWidgetItem("LPF_EN"));
    ui->tableCommands->setItem(15, 1, new QTableWidgetItem("PFD EN"));
    ui->tableCommands->setItem(16, 1, new QTableWidgetItem("RN"));

    ui->tableCommands->setItem(17, 0, new QTableWidgetItem("OUT8<0:7>"));
    ui->tableCommands->setItem(17, 1, new QTableWidgetItem("SEL_C1<1:8>"));

    ui->tableCommands->setItem(18, 0, new QTableWidgetItem("OUT9<0:7>"));
    ui->tableCommands->setItem(18, 1, new QTableWidgetItem("SEL_C1<9:16>"));

    ui->tableCommands->setItem(19, 0, new QTableWidgetItem("OUT10<0:7>"));
    ui->tableCommands->setItem(19, 1, new QTableWidgetItem("SEL_C1<17:24>"));

    ui->tableCommands->setItem(20, 0, new QTableWidgetItem("OUT11<0:7>"));
    ui->tableCommands->setItem(20, 1, new QTableWidgetItem("SEL_C1<25:32>"));

    ui->tableCommands->setItem(21, 0, new QTableWidgetItem("OUT12<0:7>"));
    ui->tableCommands->setItem(21, 1, new QTableWidgetItem("SEL_C2<1:8>"));

    ui->tableCommands->setItem(22, 0, new QTableWidgetItem("OUT13<0:7>"));
    ui->tableCommands->setItem(22, 1, new QTableWidgetItem("SEL_C2<9:16>"));

    ui->tableCommands->setItem(23, 0, new QTableWidgetItem("OUT14<0:7>"));
    ui->tableCommands->setItem(23, 1, new QTableWidgetItem("VCO PVT<1:8>"));

    ui->tableCommands->setItem(24, 0, new QTableWidgetItem("OUT15<0:5>"));
    ui->tableCommands->setItem(24, 1, new QTableWidgetItem("VCO PVT<9:14>"));
    // 2. 然后，遍历每一行，读取“位域”信息，决定用哪种控件
    QRegularExpression re("<(\\d+):(\\d+)>");

    for (int row = 0; row < ui->tableCommands->rowCount(); ++row) {
        auto *itm = ui->tableCommands->item(row, 1);
        if (!itm) continue;
        QString txt = itm->text();

        int bitWidth = 1;
        // 用 QRegularExpressionMatch 去做匹配
        QRegularExpressionMatch match = re.match(txt);
        if (match.hasMatch()) {
            int start = match.captured(1).toInt();
            int end   = match.captured(2).toInt();
            bitWidth  = qAbs(end - start) + 1;
        }

        // 创建一个容器 widget 放控件
        QWidget *cellWidget = new QWidget;
        auto *lay = new QHBoxLayout(cellWidget);
        lay->setContentsMargins(0,0,0,0);
        lay->setSpacing(0);


        if (bitWidth == 1) {
            // 单 bit，插入 QCheckBox
            QCheckBox *cb = new QCheckBox;
            lay->addWidget(cb);
            // 水平+垂直 居中
            lay->setAlignment(cb, Qt::AlignHCenter | Qt::AlignVCenter);

        } else if(bitWidth != 32){
            // 多 bit，插入 QSpinBox
//            QSpinBox *sb = new QSpinBox;
            // 多 bit，插入 BinarySpinBox（显示二进制）
            auto *sb = new BinarySpinBox;
            sb->setRange(0, (1 << bitWidth) - 1);
            sb->setMaximumWidth(100);
            lay->addWidget(sb);
        }else{
//            QDoubleSpinBox *sb = new QDoubleSpinBox;
//            sb->setDecimals(0);
//            sb->setRange(0, 4294967295);
            QLineEdit *edit = new QLineEdit;
            edit->setMaximumWidth(60);
            lay->addWidget(edit);
        }

        cellWidget->setLayout(lay);
        // 第三列索引是 2
        ui->tableCommands->setCellWidget(row, 2, cellWidget);
    }
    /********************************QTableWidget配置******************************/
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
//    qDebug() << "flagFloat : " << flagFloat;

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


void command_window::on_FA2415ASetpB_clicked()
{
    QList<uint32_t> values;
    int rows = ui->tableCommands->rowCount();
    for (int row = 0; row < rows; ++row) {
        QWidget *cell = ui->tableCommands->cellWidget(row, 2);
        uint32_t v = 0;
        if (cell) {
            if (auto *cb = cell->findChild<QCheckBox*>()) {
                v = cb->isChecked() ? 1 : 0;
            }
            else if (auto *sb = cell->findChild<QSpinBox*>()) {
                int x = sb->value();
                v = static_cast<uint32_t>(x);
            }
            else if (auto *le = cell->findChild<QLineEdit*>()) {
                bool ok;
                quint64 tmp = le->text().toULongLong(&ok);
                if (ok) {
                    // clamp 到 uint32 范围
                    if (tmp > 0xFFFFFFFFULL) tmp = 0xFFFFFFFFULL;
                    v = static_cast<quint32>(tmp);
                    qDebug() << "v"<<v;
                }
            }
        }
        // 若 cell 为空或无控件，v 保持 0,注意被合并的部分也会算入
        values.append(v);
    }

    // 此时 values 就包含了每一行“设置”列的数值
    // 例如你可以打印验证：
    qDebug() << "Collected values:";
    for (quint32 v : values) {
        qDebug() << v;
    }
    QList<float> packet;
    uint8_t byte = 0;
    byte = values[0]|values[1]<<7;
    packet.append(byte);

    byte = values[2]|values[3]<<7;
    packet.append(byte);

    if(m_isFa2415ArbSelected ==true){
        byte = values[4]|values[5]<<2|values[6]<<3|values[7]<<4|values[8]<<5;
        packet.append(byte);

        byte = values[9]<<3;
        packet.append(byte);
    }else{
        byte = values[4]|values[5]<<2|values[6]<<3|values[7]<<4;
        packet.append(byte);

        byte = values[8]<<2|values[9]<<3;
        packet.append(byte);
    }

    byte = values[10]<<3;
    packet.append(byte);

    byte = values[11]|values[12]<<4;
    packet.append(byte);

    byte = values[13]|values[14]<<4|values[15]<<5|values[16]<<6;
    packet.append(byte);

    byte = values[17]&0xff;
    packet.append(byte);

    byte = values[18]&0xff;
    packet.append(byte);

    byte = values[19]&0xff;
    packet.append(byte);

    byte = values[20]&0xff;
    packet.append(byte);

    byte = values[21]&0xff;
    packet.append(byte);

    byte = values[22]&0xff;
    packet.append(byte);

    byte = values[23]&0xff;
    packet.append(byte);

    byte = values[24]&0xff;
    packet.append(byte);

    emit sensor_signal(0xF3, packet);
    // 打印验证
    qDebug() << "packet:";
    qDebug() << "packet: " << packet;
}


void command_window::on_FA2415ArB_toggled(bool checked)
{
    m_isFa2415ArbSelected = checked;
    if(!checked)
    {
        ui->tableCommands->setSpan(4, 0, 4, 1);
        ui->tableCommands->item(4,0)->setText("OUT3<0:4>");
        ui->tableCommands->setItem(8,0,new QTableWidgetItem("OUT4<2>"));
        ui->tableCommands->item(8,1)->setText("DVS_CAP_SW");

    }
    else {
        ui->tableCommands->setSpan(4, 0, 5, 1);
        ui->tableCommands->item(4,0)->setText("OUT3<0:5>");
        ui->tableCommands->item(8,1)->setText("SPI DVS RONGHE");
    }
}

