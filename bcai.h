#ifndef BCAI_H
#define BCAI_H

#include <QMainWindow>
#include <QPainter>
#include <QFile>
#include <QDateTime>
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include <QTimer>
#include <QInputDialog>
#include <QVector>
#include "chartwindow.h"
#include "connectway.h"
#include "newcamera.h"
#include "prodata.h"
#include <QRandomGenerator>
#include <QComboBox>

QT_BEGIN_NAMESPACE
namespace Ui {
class BCAI;
}
QT_END_NAMESPACE

class BCAI : public QMainWindow
{
    Q_OBJECT

public:
    BCAI(QWidget *parent = nullptr);
    ~BCAI();

    void savedate();//数据保存函数
    QList<double> parseWaveformFrame(const QByteArray &frameData);//接收数据转化函数

    QString formatByteArray(QByteArray input);//格式转换
private slots:
    void on_ConnectpushButton_clicked();//连接按钮槽函数
    void on_Start_EndpushButton_clicked();//启动停止按钮槽函数
    void DataReceiveCRC();//接收数据槽函数
    void DataReceiveCSV();

    void Dataform();

    void on_recordpushButton_clicked();//保存数据槽函数
    void on_chongzhipushButton_clicked();//重置槽函数
    void on_camerapushButton_clicked();//摄像设备函数
    void on_fqspushButton_clicked();//频率普显示函数
    void on_filterComboBox_currentIndexChanged(int index); // 滤波器选择

private:
    QVector<QVector<QPointF>> chartBuffer; // 八通道缓存
    QVector<QVector<QPointF>> pre_datas;   // 八通道待保存数据
    QVector<QVector<double>> alphaWaves;   // 阿尔法波数据
    QVector<QVector<double>> betaWaves;    // 贝塔波数据

    Ui::BCAI *ui;
    newcamera *camera;//摄像设备对象
    ChartWindow *chartwindow;//脑电波形窗口
    Connectway *connectway;//连接方式窗口
    QTimer *datatime=nullptr;//频率定时器
    int Exetime = 0;//时间记录标志
    Preprocessor *preprocessor;//预处理对象
    double lastDataTime = 0.0;//计算时间标志
    bool Start_End = false;//按钮改变标志量
    bool Recording = false;//按钮改变标志量
    QFile *file1=nullptr;//保存数据文件对象
    QTimer* dropDetectTimer = nullptr;//脱落检测定时器

    QString newData;//储存接收到的数据
    QList<double> values;//改变格式后的数据
    int pos = 0;//数据帧内位置

    //数据接收
    QByteArray incomplete; //存储不完整的数据帧
    bool findhead(const QByteArray &data, int &pos); //找开头
    bool validate(const QByteArray &frame); //验证帧有效性
    quint16 calculateCRC(const QByteArray &data); // CRC计算函数
    QList<double> parsewave(const QByteArray &frame); // 解析波形帧

    // 新增滤波函数
    void applyBandpassFilter(QVector<double>& data, double lowCut, double highCut, int sampleRate);
    void updateFilteredWaves();
};
#endif // BCAI_H
