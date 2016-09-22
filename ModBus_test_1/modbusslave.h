#ifndef MODBUSSLAVE_H
#define MODBUSSLAVE_H

#include <QWidget>
#include <QModbusServer>
#include <QDateTime>
#include <QTimer>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>

namespace Ui {
class ModBusSlave;
}

class ModBusSlave : public QWidget
{
    Q_OBJECT

public:
    explicit ModBusSlave(QWidget *parent = 0);
    ~ModBusSlave();

private:
    Ui::ModBusSlave *ui;
    QModbusServer *modbusDevice;
    QSerialPort *my_serialport;

private slots:
    void updateWidgets(QModbusDataUnit::RegisterType table, int address, int size);//dataWriten信号被处发后，得到数据做出显示
    void qtimeSlot();//时间改变槽函数
    void on_pushButton_clicked();
    void my_readuart();
};

#endif // MODBUSSLAVE_H
