#ifndef CONNECTWAY_H
#define CONNECTWAY_H

#include <QMainWindow>
#include<QTcpSocket>
#include <QTimer>
#include <QSerialPort>
#include <QSerialPortInfo>
namespace Ui {
class Connectway;
}

class Connectway : public QMainWindow
{
    Q_OBJECT

public:
    explicit Connectway(QWidget *parent = nullptr);
    ~Connectway();

    //连接方式
    bool cheakserialstate=false;
    bool cheakinterstate=false;
    bool interstate=false;
    //两个连接端
    QSerialPort *serial;
    QTcpSocket *client;
    //网络连接定时器
    QTimer *intertimer;
    void serialSet();//设置串口参数
    //打开串口
    void openserial();
    //网络连接
    void onConnected();
    //网络连接错误判断函数
    void onError(QAbstractSocket::SocketError error);
    //网络连接超时函数
    void ontimeout();
private slots:
    void on_scanPushButton_clicked();//扫描可用串口函数

    void on_openserialpushButton_clicked();//打开串口函数

    void on_interconpushButton_clicked();//网络连接函数

    void on_outserialpushButton_clicked();//断开串口连接函数

    void on_outinterpushButton_clicked();//断开网络衔接函数

private:
    Ui::Connectway *ui;

};

#endif // CONNECTWAY_H
