#include "serialworker.h"
#include <QDebug>

bool serial_bind_flag = false;
//QByteArray baRcvData;
SerialWorker::SerialWorker(QObject *parent)
    : QObject{parent}
{

}

SerialWorker::~SerialWorker()
{
    if (timer) {
        timer->stop();
        delete timer;
    }
    delete serialWorker;

}

void SerialWorker::SerialPortInit(QString com_name)
{
    serialWorker = new QSerialPort;
    timer = new QTimer;

    QSerialPort::BaudRate baudRate;
    QSerialPort::DataBits dataBits;
    QSerialPort::StopBits stopBits;
    QSerialPort::Parity checkBits;

    baudRate = QSerialPort::Baud115200;
    dataBits = QSerialPort::Data8;
    stopBits = QSerialPort::OneStop;
    checkBits = QSerialPort::NoParity;

//    if(ui->baundrateCb->currentText()=="4800"){
//        baudRate = QSerialPort::Baud4800;
//    }else if(ui->baundrateCb->currentText()=="9600"){
//        baudRate = QSerialPort::Baud9600;
//    }else if(ui->baundrateCb->currentText()=="115200"){
//        baudRate = QSerialPort::Baud115200;
//    }

//    if(ui->dataCb->currentText()=="8"){
//        dataBits = QSerialPort::Data8;
//    }else if(ui->dataCb->currentText()=="7"){
//        dataBits = QSerialPort::Data7;
//    }else if(ui->dataCb->currentText()=="6"){
//        dataBits = QSerialPort::Data6;
//    }else if(ui->dataCb->currentText()=="5"){
//        dataBits = QSerialPort::Data5;
//    }

//    if(ui->stopCb->currentText()=="1"){
//        stopBits = QSerialPort::OneStop;
//    }else if(ui->stopCb->currentText()=="1.5"){
//        stopBits = QSerialPort::OneAndHalfStop;
//    }else if(ui->stopCb->currentText()=="2"){
//        stopBits = QSerialPort::TwoStop;
//    }

//    if(ui->checkCb->currentText()=="none"){
//        checkBits = QSerialPort::NoParity;
//    }

    serialWorker->setPortName(com_name);
    serialWorker->setBaudRate(baudRate);
    serialWorker->setDataBits(dataBits);
    serialWorker->setStopBits(stopBits);
    serialWorker->setParity(checkBits);

    connect(serialWorker,&QSerialPort::readyRead,this,&SerialWorker::SerialPortReadyRead_Slot);
    connect(timer, &QTimer::timeout, this, &SerialWorker::timeUpdate);

    if(serialWorker->open(QIODevice::ReadWrite)==true)
    {
        serial_bind_flag = true;
        //qDebug()<<"串口打开成功";
    }else{
        serial_bind_flag = false;
        //qDebug()<<"串口打开失败";
    }
    //qDebug()<<"serial_bind_flag"<<serial_bind_flag;

}

void SerialWorker::SerialOpen()
{

}

void SerialWorker::SerialClose()
{
    if (serialWorker) {
        serialWorker->close();
        delete serialWorker; // 释放内存
        serialWorker = nullptr; // 防止悬空指针
        serial_bind_flag = false;
    }
    //qDebug()<<"serial_bind_flag"<<serial_bind_flag;
}


void SerialWorker::ADInstructionCode(QList<float> ADSetVals)
{
    //先放大1000倍然后转换十六进制
    QString hexValues;
    for(int i=0;i<ADSetVals.size();i++)
    {
        float value = ADSetVals[i]*1000;
        //qDebug()<<"value"<<value;
        // 转换为整数,需要两字节存储
        unsigned short usValue = static_cast<unsigned short>(static_cast<int>(value));

        unsigned char lowByte = static_cast<unsigned char>(usValue&0xFF);
        unsigned char highByte = static_cast<unsigned char>((usValue>>8)&0xFF);
        // 生成十六进制字符串
        QString lowHexString = QString::number(lowByte, 16).toUpper().rightJustified(2, '0'); // 确保是2位
        QString highHexString = QString::number(highByte, 16).toUpper().rightJustified(2, '0'); // 确保是2位

        //然后插入帧头，长度，校验码，帧尾

        // 计算长度（长度始终为4个字节）
        QString lengthString = QString::number(4, 16).toUpper().rightJustified(2, '0'); // 确保长度是2位
        //计算异或校验
        QString sequenceNumber;
        unsigned char checksum = 0; // 存储校验和
        checksum ^= 0x04; // 进行异或运算
        checksum ^= 0xAD; // 进行异或运算

        if(i<5)
        {
            sequenceNumber = QString("0%1").arg(i+1, 1, 16, QLatin1Char('0')).toUpper(); // 序列号，单字节，填充0
            checksum ^= static_cast<unsigned char>(0x01 + i); // 将 0x(0i+1) 纳入异或计算

        }
        else if(i ==5)
        {
            //sequenceNumber =QString("30");
            sequenceNumber = QString("3%1").arg(0, 1, 16, QLatin1Char('0')).toUpper(); // 序列号，单字节，填充3
            checksum ^= static_cast<unsigned char>(0x30); // 将 0x(0i+1) 纳入异或计算
        }
        else if((i>5)&&(i<21))
        {
            sequenceNumber = QString("5%1").arg(i-5, 1, 16, QLatin1Char('0')).toUpper(); // 序列号，单字节，填充5
            checksum ^= static_cast<unsigned char>(0x50+i-5); // 将 0x51~0x5f 纳入异或计算
        }
        else if((i>=21)&&(i<ADSetVals.size()))
        {
            sequenceNumber = QString("6%1").arg(i-20, 1, 16, QLatin1Char('0')).toUpper(); // 序列号，单字节，填充6
            checksum ^= static_cast<unsigned char>(0x60+i-20); // 将 0x61~65 纳入异或计算
        }
        checksum ^= highByte; // 进行异或运算
        checksum ^= lowByte;
        //qDebug() << "checksum" << checksum;
        // 插入帧头和其它信息
        QString framedHexString = "EB" + lengthString + "AD" + sequenceNumber + highHexString + lowHexString + QString::number(checksum, 16).toUpper().rightJustified(2, '0') +"0A";

        // 将结果添加到 hexValues 列表
        hexValues.append(framedHexString);

    }
    //最后发送给串口输出槽函数
    emit AD_instruction_signal(hexValues);

    qDebug() << "Hex Values:" << hexValues;

}
//将所有AD的数据放入一条指令中发送
void SerialWorker::DAInstructionCode(QList<float> ADSetVals)
{
    //计算异或校验
    unsigned char checksum = 0; // 存储校验和
    // 计算长度（长度始终为53个字节=26*2+1个指令）
    QString lengthString = QString::number(53, 16).toUpper().rightJustified(2, '0'); // 确保长度是2位

    checksum ^= 0x35; // 进行异或运算,长度为53
    checksum ^= 0xDA; // 进行异或运算
    QString dataString;
    for(int i=0;i<ADSetVals.size();i++)
    {
        //先放大1000倍然后转换十六进制
        float value = ADSetVals[i]*1000;
        //qDebug()<<"value"<<value;
        // 转换为整数,需要两字节存储
        unsigned short usValue = static_cast<unsigned short>(static_cast<int>(value));

        //然后插入帧头，长度，校验码，帧尾

        unsigned char lowByte = static_cast<unsigned char>(usValue&0xFF);
        unsigned char highByte = static_cast<unsigned char>((usValue>>8)&0xFF);
        // 生成十六进制字符串
        QString lowHexString = QString::number(lowByte, 16).toUpper().rightJustified(2, '0'); // 确保是2位
        QString highHexString = QString::number(highByte, 16).toUpper().rightJustified(2, '0'); // 确保是2位

        checksum ^= highByte; // 进行异或运算
        checksum ^= lowByte;

        dataString = dataString + highHexString + lowHexString;
    }
    //qDebug() << "checksum" << checksum;
    // 插入帧头和其它信息
    QString framedHexString = "EB" + lengthString + "DA" + dataString + QString::number(checksum, 16).toUpper().rightJustified(2, '0') +"0A";

    //最后发送给串口输出槽函数
    emit DA_instruction_signal(framedHexString);

    qDebug() << "串口输出指令framedHexString:" << framedHexString;
}


void SerialWorker::SerialSendData_Slot(QString buf)
{
    QByteArray senddata;
    string2Hex(buf,senddata);
    if(serial_bind_flag == true)
        //serialWorker->write(buf.toLocal8Bit().data());
        serialWorker->write(senddata);
    //qDebug()<<"开启sendSlot线程"<<QThread::currentThreadId();//查看槽函数在哪个线程运行
}

//串口解析，包括校验指令是否传输正确，以及将正确的指令解析处理发送到主窗口中
void SerialWorker::SerialAnalyse(const QByteArray &recvdata)
{
    //将strList转换为std::vector<unsigned char>后进行数据传递，这样使用的时候不需要每次用到就进行数据转换进行转换，时间更快些
    // 开始计时
    //auto start = std::chrono::high_resolution_clock::now();
    QString str = recvdata.toHex(' ').toUpper().append(' ');
    QStringList strList = str.split(" ");
    strList.pop_back();

    std::vector<unsigned char> byteArray(strList.size());
    bool ok;
    for (int i = 0; i < strList.size(); ++i) {
        byteArray[i] = strList[i].toInt(&ok,16);
    }
    // 打印每个字节
//    for (size_t i = 0; i < byteArray.size(); ++i) {
//        qDebug() << "byteArray[" << i << "]" << QString::number(byteArray[i],16).toUpper();
//    }

    while(true)
    {

        // 检查帧头和帧尾
        if (byteArray.empty() ) {
            qDebug()<<"empty,end";
            return;
        }

        if (byteArray[0] != 0xEB ) {
            qDebug()<<"帧头不匹配";
            auto it = find(byteArray.begin(), byteArray.end(), 0xEB);
            if(it!=byteArray.end())
            {
                byteArray.erase(byteArray.begin(), it);
                continue;
            }
            else
            {
                return;
            }
        }
        int length = byteArray[1]; // 使用 unsigned char 类型
//        qDebug() << "length" << length;
        if(length+4>byteArray.size())
        {
            //末尾指令数据未接收完整或者之前获取的EB不是帧头，而是普通数据
            auto it = find(byteArray.begin()+1, byteArray.end(), 0xEB); //第一个0xEB非真正的帧头，所以不在搜索的范围内
            if(it!=byteArray.end())
            {
                byteArray.erase(byteArray.begin(), it);
                qDebug()<<"重新找到帧头";
                continue;
            }
            else
            {
                return;
            }
        }
        if (byteArray[length + 3] !=0x0A) {
            // 长度不匹配
            if(byteArray.size()>=length+4)
            {
                byteArray.erase(byteArray.begin(), byteArray.begin() + length + 4);
                qDebug()<<"重新定位长度";
                continue;
            }
            else
                return;
        }
        std::vector<unsigned char> buffer(length+4);
        copy(byteArray.begin(), byteArray.begin() + length + 4, buffer.begin());    //将byteArray第一条指令赋值给buffer
//        qDebug()<<"buffer"<<buffer;
        // 计算异或校验
        bool Xor_flag= XorCorrect(buffer);
        if(Xor_flag==0)
        {
            qDebug()<<"异或校验错误";
            return;
        }

        // 创建 content 数组并填充数据
        std::vector<unsigned char> content(buffer.begin() + 2, buffer.begin() + 3 + length - 1);
//        qDebug() << "content" << QByteArray(reinterpret_cast<char*>(content.data()), content.size());

        InstructionAnalyse(content);
        byteArray.erase(byteArray.begin(), byteArray.begin() + length + 4);
    }

    // 结束计时
//    auto end = std::chrono::high_resolution_clock::now();
//    std::chrono::duration<double> elapsed = end - start;

    // 打印运行时间
//    qDebug() << "代码执行时间：" << elapsed.count() << "秒";

}
//异或校验
bool SerialWorker::XorCorrect(const std::vector<unsigned char> &byteArray)
{
    // 计算异或校验
    unsigned char checksum = 0; // 存储校验和
    for (size_t i = 1; i < byteArray.size() - 2; ++i) { // 从长度字节开始到倒数第二个字节
        checksum ^= byteArray[i]; // 进行异或运算
    }
//    qDebug() << "checksum" << checksum;
    // 验证校验和
    return checksum == byteArray[byteArray.size() - 2];
}

//接收指令解析
void SerialWorker::InstructionAnalyse(const std::vector<unsigned char> &content)
{
    // 处理指令码为 AD 的情况
    if (content[0] == 0xAD && content.size() == 4)
    {
        // 解析电流值
        float currentMilliamp = (content[2] << 8) | (content[3] & 0xFF);
        float current = currentMilliamp / 1000;
        emit LCDNumShow(content[1],current);
        qDebug() << "current" << current;
    }
    else if(content[0] == 0xDA && content.size() == 95)
    {
        std::vector<float> currents;
        for(int i = 1;i<content.size()-1;i+=2)
        {
            // 解析电流值
            float currentMilliamp = (content[i] << 8) | (content[i+1] & 0xFF);
            float current = currentMilliamp / 1000;
            currents.push_back(current);
        }
        emit LCDNumShow2(currents);
        qDebug()<<"currents"<<currents;
    }
    //
}

void SerialWorker::SerialPortReadyRead_Slot()
{

    timer->start(200);//启动定时器，接收100毫秒数据（根据情况设定）
    baRcvData.append(serialWorker->readAll());

//    qDebug()<<baRcvData;
//    QByteArray data = serialWorker->readAll();
//    // 解析接收到的数据
//    SerialAnalyse(data);
//    QString str = data.toHex(' ').toUpper().append(' ');//十六进制显示
//    emit recvDataSignal(str);

    //qDebug()<<"开启recvSlot线程"<<QThread::currentThreadId();//查看槽函数在哪个线程运行
}

void SerialWorker::timeUpdate()
{
    timer->stop();
    if(baRcvData.length()!=0)
    {
//        qDebug()<<"baRcvData"<<baRcvData;
        // 解析接收到的数据
        SerialAnalyse(baRcvData);
        QString str = baRcvData.toHex(' ').toUpper().append(' ');//十六进制显示
        emit recvDataSignal(str);

    }
    baRcvData.clear();
}

void SerialWorker::string2Hex(QString str, QByteArray &senddata)
{
    int hexdata, lowhexdata;
    int hexdatalen = 0;
    int len = str.length();
    senddata.resize(len / 2);
    char lstr, hstr;
    for (int i = 0; i < len;) {
        // char lstr,
        // hstr=str[i].toAscii();//qt4

        hstr = str[i].toLatin1();  // qt5
        if (hstr == ' ') {
            i++;
            continue;
        }
        i++;
        if (i >= len)
            break;
        // lstr = str[i].toAscii();//qt4
        lstr = str[i].toLatin1();  // qt5
        hexdata = this->hex2Char(hstr);
        lowhexdata = this->hex2Char(lstr);
        if ((hexdata == 16) || (lowhexdata == 16))
            break;
        else
            hexdata = hexdata * 16 + lowhexdata;
        i++;
        senddata[hexdatalen] = (char)hexdata;
        hexdatalen++;
    }
    senddata.resize(hexdatalen);
}

char SerialWorker::hex2Char(char ch)
{
    if ((ch >= '0') && (ch <= '9'))
        return ch - 0x30;
    else if ((ch >= 'A') && (ch <= 'F'))
        return ch - 'A' + 10;
    else if ((ch >= 'a') && (ch <= 'f'))
        return ch - 'a' + 10;
    else
        return (-1);
}

