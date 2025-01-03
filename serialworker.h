#ifndef SERIALWORKER_H
#define SERIALWORKER_H

#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QThread>

class SerialWorker : public QObject
{
    Q_OBJECT
public:
    explicit SerialWorker(QObject *parent = nullptr);

    ~SerialWorker();
    QSerialPort *serialWorker;

    void SerialPortInit(QString com_name);
    void SerialOpen();

    void SerialClose();
    void SerialAnalyse(const QByteArray &data);

    bool XorCorrect(const std::vector<unsigned char>& byteArray);
    void InstructionAnalyse(const std::vector<unsigned char>& content);

    void string2Hex(QString str, QByteArray &senddata);
    char hex2Char(char ch);


public slots:
    void SerialSendData_Slot(QString buf);
    void SerialPortReadyRead_Slot();
    void ADInstructionCode(QList<float> ADSetVals);
    void DAInstructionCode(QList<float> ADSetVals);
signals:
    void recvDataSignal(QString buf);
    void LCDNumShow(unsigned char index,float value);
    void LCDNumShow2(std::vector<float>currents);
    void AD_instruction_signal(QString buf);
    void DA_instruction_signal(QString buf);
};

#endif // SERIALWORKER_H
