#include "connectway.h"
#include "ui_connectway.h"

#include <qmessagebox.h>

Connectway::Connectway(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Connectway)
{
    ui->setupUi(this);

    //设置图标标题
    this->setWindowIcon(QIcon(":/icon/connect.png"));
    this->setWindowTitle("Connectway");

    serial =new QSerialPort(this);
    ui->baudRateComboBox->setCurrentIndex(5);

    client=new QTcpSocket(this);

    //设置连接方式选择状态
    ui->scanPushButton->setEnabled(false);
    ui->serialPortComboBox->setEnabled(false);
    ui->baudRateComboBox->setEnabled(false);
    ui->stopComboBox->setEnabled(false);
    ui->dataComboBox->setEnabled(false);
    ui->checkComboBox->setEnabled(false);
    ui->openserialpushButton->setEnabled(false);
    ui->outserialpushButton->setEnabled(false);
    ui->IPlineEdit->setEnabled(false);
    ui->duankoulineEdit->setEnabled(false);
    ui->interconpushButton->setEnabled(false);
    ui->outinterpushButton->setEnabled(false);

    // 当serial复选框被点击时
    QObject::connect(ui->checkBoxserial, &QCheckBox::clicked, this, [=](){
        if(ui->checkBoxserial->isChecked()) {
            cheakserialstate=true;
            ui->checkBoxinter->setChecked(false);
            ui->checkBoxinter->setEnabled(false);
            ui->scanPushButton->setEnabled(true);
            ui->serialPortComboBox->setEnabled(true);
            ui->baudRateComboBox->setEnabled(true);
            ui->stopComboBox->setEnabled(true);
            ui->dataComboBox->setEnabled(true);
            ui->checkComboBox->setEnabled(true);
            ui->openserialpushButton->setEnabled(true);

        } else {
            cheakserialstate=false;
            ui->checkBoxinter->setEnabled(true);
            ui->scanPushButton->setEnabled(false);
            ui->serialPortComboBox->setEnabled(false);
            ui->baudRateComboBox->setEnabled(false);
            ui->stopComboBox->setEnabled(false);
            ui->dataComboBox->setEnabled(false);
            ui->checkComboBox->setEnabled(false);
            ui->openserialpushButton->setEnabled(false);

        }
    });
    // 当inter复选框被点击时
    QObject::connect(ui->checkBoxinter, &QCheckBox::clicked, this, [=](){
        if(ui->checkBoxinter->isChecked()) {
            cheakinterstate=true;
            ui->checkBoxserial->setChecked(false);
            ui->checkBoxserial->setEnabled(false);
            ui->IPlineEdit->setEnabled(true);
            ui->duankoulineEdit->setEnabled(true);
            ui->interconpushButton->setEnabled(true);

        } else {
            cheakinterstate=false;
            ui->checkBoxserial->setEnabled(true);
            ui->IPlineEdit->setEnabled(false);
            ui->duankoulineEdit->setEnabled(false);
            ui->interconpushButton->setEnabled(false);

        }
    });
}

Connectway::~Connectway()
{
    delete ui;
    delete serial;
    delete client;
    delete intertimer;
}
void Connectway::on_scanPushButton_clicked()//扫描串口
{
    ui->serialPortComboBox->clear();
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
        QSerialPort _com;
        _com.setPort(info);
        if (_com.portName() == serial->portName() || _com.open(QIODevice::ReadWrite)) {
            ui->serialPortComboBox->addItem(_com.portName());
            _com.close();
        }
    }
}
void Connectway::serialSet()//串口参数设置
{
    serial->setPortName(ui->serialPortComboBox->currentText());
    serial->setBaudRate(ui->baudRateComboBox->currentText().toLong());
    switch (ui->stopComboBox->currentIndex()) {
    case 0: serial->setStopBits(QSerialPort::OneStop); break;
    case 1: serial->setStopBits(QSerialPort::TwoStop); break;
    case 2: serial->setStopBits(QSerialPort::OneAndHalfStop); break;
    }
    switch (ui->dataComboBox->currentText().toLong()) {
    case 5: serial->setDataBits(QSerialPort::Data5); break;
    case 6: serial->setDataBits(QSerialPort::Data6); break;
    case 7: serial->setDataBits(QSerialPort::Data7); break;
    case 8: serial->setDataBits(QSerialPort::Data8); break;
    }
    switch (ui->checkComboBox->currentIndex()) {
    case 0: serial->setParity(QSerialPort::NoParity); break;
    case 1: serial->setParity(QSerialPort::OddParity); break;
    case 2: serial->setParity(QSerialPort::EvenParity); break;
    }
}
void Connectway::on_openserialpushButton_clicked()//打开串口函数
{
    if (!serial->isOpen()) {
        serialSet();
        if (serial->open(QIODevice::ReadWrite)) {
            ui->openserialpushButton->setEnabled(false);
            ui->serialPortComboBox->setEnabled(false);
            ui->baudRateComboBox->setEnabled(false);
            ui->stopComboBox->setEnabled(false);
            ui->dataComboBox->setEnabled(false);
            ui->checkComboBox->setEnabled(false);
            ui->outserialpushButton->setEnabled(true);
        } else {
            QMessageBox::critical(this, "错误", "串口打开失败！");
        }
    }
}
void Connectway::on_outserialpushButton_clicked()//串口断开函数
{
    serial->close();
    QMessageBox::critical(this, "提示", "串口断开！");
    ui->openserialpushButton->setEnabled(true);
    ui->serialPortComboBox->setEnabled(true);
    ui->baudRateComboBox->setEnabled(true);
    ui->stopComboBox->setEnabled(true);
    ui->dataComboBox->setEnabled(true);
    ui->checkComboBox->setEnabled(true);
    ui->outserialpushButton->setEnabled(false);

}
void Connectway::on_interconpushButton_clicked()//网络连接函数
{
    client->connectToHost(ui->IPlineEdit->text(),ui->duankoulineEdit->text().toInt());

    intertimer=new QTimer(this);
    intertimer->setSingleShot(true);
    intertimer->setInterval(5000);
    connect(intertimer,&QTimer::timeout,this,&Connectway::ontimeout);//超时定时器
    connect(client,&QTcpSocket::connected,this,&Connectway::onConnected);
    connect(client,SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(onError(QAbstractSocket::SocketError)));
    intertimer->start();
}
void Connectway::onConnected()//网络连接成功函数
{
    interstate=true;
    ui->interconpushButton->setEnabled(false);
    ui->duankoulineEdit->setEnabled(false);
    ui->IPlineEdit->setEnabled(false);
    ui->outinterpushButton->setEnabled(true);
}

void Connectway::on_outinterpushButton_clicked()//网络断开函数
{
    client->close();
    interstate=false;
    QMessageBox::critical(this, "提示", "连接断开！");
    ui->interconpushButton->setEnabled(true);
    ui->duankoulineEdit->setEnabled(true);
    ui->IPlineEdit->setEnabled(true);
    ui->outinterpushButton->setEnabled(false);
}
void Connectway::onError(QAbstractSocket::SocketError error)//判断连接错误信息函数
{
    QMessageBox::warning(this, "错误","连接失败："+client->errorString() );
    ui->interconpushButton->setEnabled(true);
}

void Connectway::ontimeout()//定时器判断连接超时
{
    if(!interstate){
        client->abort();
    }
}


