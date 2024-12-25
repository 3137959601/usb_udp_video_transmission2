#include "logwindow.h"
#include "ui_logwindow.h"
#include <QFileDialog>
#include <QTextStream>
#include <QFile>


logwindow::logwindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::logwindow)
{
    ui->setupUi(this);
    this->setWindowTitle("日志");
}

logwindow::~logwindow()
{
    delete ui;
}

void logwindow::on_browseBt_clicked()
{
    QString filename = QFileDialog::getOpenFileName(this,"文件对话框",".","*.txt");
    ui->fileEdit->setText(filename);

    QFile file(filename);
    if(!file.open(QIODevice::ReadOnly|QIODevice::Text))
    {
        ui->plainTextEdit->appendPlainText("文件打开失败");
        return;
    }
    QTextStream readStream(&file);
    // 直接读取文件内容，QTextStream 默认是 UTF-8
    QString content = readStream.readAll();
    // 显示内容
    ui->plainTextEdit->setPlainText(content);

}


void logwindow::on_saveBt_clicked()
{
//    QFile file(ui->fileEdit->text());
//    if(!file.open(QIODevice::WriteOnly|QIODevice::Text))
//    {
//        ui->plainTextEdit->appendPlainText("文件打开失败");
//        return;
//    }
//    QString context= ui->plainTextEdit->toPlainText();
////    const char *sTemp = context.toStdString().c_str();
////    int len = strlen(sTemp);
////    if( len>0 )
////    {
////        file.write(sTemp, len);
////    }
//    // 使用 QTextStream 处理文本文件的写入
//    QTextStream out(&file);
//    out << context; // 将 QString 直接写入流中
//    out.setCodec("UTF-8"); // 设定编码为 UTF-8
//    file.flush();
//    file.close();
}

