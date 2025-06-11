#include "binaryspinbox.h"
#include <QRegularExpression>

static const QRegularExpression rx("^(\\d+)'[bB]([01]+)$");

BinarySpinBox::BinarySpinBox(QWidget *parent)
     : QSpinBox(parent) {
       // 设置文本框等宽字体（选用等宽字体更易读）
       QFont f = font();
       f.setFamily("Courier New");
       setFont(f);
       // 只显示数字，不要千分位分隔等
       setGroupSeparatorShown(false);
   }
QString BinarySpinBox::textFromValue(int val) const  {
    // 例如要固定显示 bitWidth 位，你可以动态传入 bitWidth
    // 这里只按当前最大值位宽自动计算
    int bits = 1;
    int mx   = maximum();
    while (mx > 1) { bits++; mx >>= 1; }
    QString s = QString::number(val, 2).rightJustified(bits, '0');
    // 3) 用动态前缀 "<bits>b" 代替 "0b"
    return QString("%1'b%2").arg(bits).arg(s);
}

// 把用户输入的文本解析回整数
int BinarySpinBox::valueFromText(const QString &text) const {
    QString t = text.trimmed();
    // 如果是 "<digits>'b<binary>" 格式，就去掉前缀部分
    auto match = rx.match(t);
    if (match.hasMatch()) {
        t = match.captured(2);
    } else {
        // 如果用户直接输入裸二进制，也接受
        if (t.startsWith("0b", Qt::CaseInsensitive))
            t = t.mid(2);
    }
    bool ok = false;
    int v = t.toInt(&ok, 2);
    return ok ? v : value();
}

// 验证用户每次输入保证合法
QValidator::State BinarySpinBox::validate(QString &input, int &pos) const
{
    Q_UNUSED(pos);
    // 1. 计算当前位宽 bits
    int bits = 1;
    int mx = maximum();
    while (mx > 1) { bits++; mx >>= 1; }

    // 2. 去掉前缀 "<n>'b" 或 "0b"
    QString t = input.trimmed();
    int startData = 0;
    if (t.contains('\'')) {
        // 如果有单引号，找 b 的位置
        int bpos = t.indexOf('b', t.indexOf('\'')+1, Qt::CaseInsensitive);
        if (bpos > 0) startData = bpos + 1;
    } else if (t.startsWith("0b", Qt::CaseInsensitive)) {
        startData = 2;
    }
    QString data = t.mid(startData);

    // 3. 检查长度，不得超过 bits
    if (data.length() > bits)
        return QValidator::Invalid;

    // 4. 检查每个字符都是 0/1
    for (QChar c : data) {
        if (c != '0' && c != '1')
            return QValidator::Invalid;
    }

    // 5. 如果刚好等于 bits，且全是 0/1，则 Acceptable；否则 Intermediate
    return (data.length() == bits ? QValidator::Acceptable
                                  : QValidator::Intermediate);
}

