#pragma once
#ifndef BINARYSPINBOX_H
#define BINARYSPINBOX_H
#include <QSpinBox>

class BinarySpinBox : public QSpinBox {
    Q_OBJECT
public:
    explicit BinarySpinBox(QWidget *parent = nullptr);


protected:
    QString textFromValue(int val) const override;
    int     valueFromText(const QString &text) const override;
    QValidator::State validate(QString &input, int &pos) const override;
};
#endif // BINARYSPINBOX_H
