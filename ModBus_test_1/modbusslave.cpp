#include "modbusslave.h"
#include "ui_modbusslave.h"

#include <QModbusRtuSerialSlave>
#include <QDebug>

ModBusSlave::ModBusSlave(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ModBusSlave)
{
    ui->setupUi(this);

    //开启定时器
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(qtimeSlot()));
    timer->start(1000);

    ui->textEdit->append(tr("检测到端口列表:"));
    //获取串口信息
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        QSerialPort serial;
        serial.setPort(info);
        if (serial.open(QIODevice::ReadWrite))
        {
            ui->comboBox_2->addItem(info.portName());//记录端口名
            ui->textEdit->append(info.portName());//端口名
            ui->textEdit->append(info.description());//端口描述
            ui->textEdit->append(info.manufacturer());//制造商
            serial.close();//关闭
        }
    }
    ui->comboBox->setCurrentIndex(7);//默认115200
    ui->pushButton->setText(tr("打开串口"));

    modbusDevice = new QModbusRtuSerialSlave(this);
    QModbusDataUnitMap reg;//设置可接收的寄存器数据类型
    reg.insert(QModbusDataUnit::Coils, { QModbusDataUnit::Coils, 0, 10 });//
    reg.insert(QModbusDataUnit::DiscreteInputs, { QModbusDataUnit::DiscreteInputs, 0, 10 });
    reg.insert(QModbusDataUnit::InputRegisters, { QModbusDataUnit::InputRegisters, 0, 10 });
    reg.insert(QModbusDataUnit::HoldingRegisters, { QModbusDataUnit::HoldingRegisters, 0, 10 });

    modbusDevice->setMap(reg);//
//    connect(modbusDevice, &QModbusServer::stateChanged,
//            this, &modbusSlave::onStateChanged);//连接状态发生改变时改变connect按钮上的显示文字（connect or discennect）
    connect(modbusDevice, &QModbusServer::dataWritten,
            this, &ModBusSlave::updateWidgets);//modbus客户端向服务器写入数据时dataWritten信号被触发
//    connect(modbusDevice, &QModbusServer::errorOccurred,
//            this, &modbusSlave::handleDeviceError);
}

ModBusSlave::~ModBusSlave()
{
    delete ui;
}
void ModBusSlave::updateWidgets(QModbusDataUnit::RegisterType table, int address, int size)//dataWriten信号被处发后，得到数据做出显示
{
    for (int i = 0; i < size; ++i) {
        quint16 value;
        switch (table) {
        case QModbusDataUnit::Coils:
            modbusDevice->data(QModbusDataUnit::Coils, address + i, &value);
            qDebug()<<value;
            break;
        case QModbusDataUnit::HoldingRegisters:
            modbusDevice->data(QModbusDataUnit::HoldingRegisters, address + i, &value);//得到数据后做出显示
            qDebug()<<value;
            break;
        default:
            break;
        }
        QString s = QString::number(value, 16);
        qDebug("%d",value);
        ui->textEdit->append(s);
    }
}

//时间槽函数
void ModBusSlave::qtimeSlot()
{
    QTime qtimeObj = QTime::currentTime();
    QString strTime = qtimeObj.toString("hh:mm:ss");
    ui->label_7->setText (strTime);
}
//打开串口槽函数
void ModBusSlave::on_pushButton_clicked()
{
    if(ui->pushButton->text()==tr("打开串口")){
        my_serialport= new QSerialPort();
        my_serialport->setPortName(ui->comboBox_2->currentText());//获取端口名
        my_serialport->open(QIODevice::ReadWrite);//打开串口
        my_serialport->setBaudRate(ui->comboBox->currentText().toInt());//设置波特率
        //校验设置
        switch(ui->comboBox_3->currentIndex()){
        case 0: my_serialport->setParity(QSerialPort::NoParity);break;
        case 1: my_serialport->setParity(QSerialPort::OddParity);break;
        case 2: my_serialport->setParity(QSerialPort::EvenParity);break;
        default: break;
        }
        //数据位数
        switch(ui->comboBox_4->currentIndex()){
        case 0: my_serialport->setDataBits(QSerialPort::Data8);break;
        case 1: my_serialport->setDataBits(QSerialPort::Data7);break;
        case 2: my_serialport->setDataBits(QSerialPort::Data6);break;
        default: break;
        }
        //停止位
        switch(ui->comboBox_5->currentIndex()){
        case 0: my_serialport->setStopBits(QSerialPort::OneStop);break;
        case 1: my_serialport->setStopBits(QSerialPort::TwoStop);break;
        default: break;
        }
        my_serialport->setFlowControl(QSerialPort::NoFlowControl);
        connect(my_serialport,SIGNAL(readyRead()),this,SLOT(my_readuart()));

        ui->comboBox->setEnabled(false);//波特率设置失效
        ui->comboBox_2->setEnabled(false);//端口名设置失效
        ui->comboBox_3->setEnabled(false);//校验设置失效
        ui->comboBox_4->setEnabled(false);//数据位数设置失效
        ui->comboBox_5->setEnabled(false);//停止位设置失效

        ui->label_6->setStyleSheet("background-color:red");
        ui->pushButton->setText(tr("关闭串口"));
//        ui->pushButton_18->setEnabled(true);
    }
    else {
        my_serialport->clear();
        my_serialport->deleteLater();
        ui->comboBox->setEnabled(true);
        ui->comboBox_2->setEnabled(true);
        ui->comboBox_3->setEnabled(true);
        ui->comboBox_4->setEnabled(true);
        ui->comboBox_5->setEnabled(true);

        ui->label_6->setStyleSheet("background-color:rgb(130,130,130)");
        ui->pushButton->setText(tr("打开串口"));
//        ui->pushButton_18->setEnabled(false);//发送按钮有效
    }
}
//接收数据槽函数
void ModBusSlave::my_readuart()
{
    QByteArray requestData;
    QByteArray temp5bytes;
    temp5bytes.resize(5);
    requestData = my_serialport->readAll();
    int count = requestData.size();

    while(count!=0)
    {
        //取5个字节
        temp5bytes = requestData.left(5);

        //协议判断，暂时实用的时哆啦A梦的协议
        if((requestData[0] = 0xf7) && (requestData[4] = 0xf8))
        {
            if( requestData[1] == 0x01 )
            {

            }
            else if(requestData[1] == 0x02)
            {
                if(requestData[2] == 0x01)
                {
                    if(requestData[3] == 0x00)
                    {
                        //ui->textEdit->append(tr(requestData.toHex()));
                        ui->textEdit->append(tr("收到呼吸灯开始"));
                    }
                    else if(requestData[3] == 0x01)
                    {
                        //ui->textEdit->append(tr(requestData.toHex()));
                        ui->textEdit->append(tr("收到呼吸灯关闭"));
                    }
                }
                else if(requestData[2] == 0x02)
                {
                    if(requestData[3] == 0x00)
                    {
                        // ui->textEdit->append(tr(requestData.toHex()));
                        ui->textEdit->append(tr("收到闪烁开始"));
                    }
                    else if(requestData[3] == 0x01)
                    {
                        // ui->textEdit->append(tr(requestData.toHex()));
                        ui->textEdit->append(tr("收到闪烁关闭"));
                    }
                }
                else if(requestData[2] == 0x03)
                {
                    if(requestData[3] == 0x00)
                    {
                        // ui->textEdit->append(tr(requestData.toHex()));
                        ui->textEdit->append(tr("收到全亮开始"));
                    }
                    else if(requestData[3] == 0x01)
                    {
                        // ui->textEdit->append(tr(requestData.toHex()));
                        ui->textEdit->append(tr("收到全亮关闭"));
                    }
                }
            }
            else if(requestData[1] == 0x03)
            {

            }
        }

        //去除判断过的数据组
        requestData.remove(0,5);
        //重新计算数据长度
        count = requestData.size();
    }

}
