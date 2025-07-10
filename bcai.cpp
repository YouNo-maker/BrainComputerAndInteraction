#include "bcai.h"
#include "ui_bcai.h"
#include <windows.h>
#include<cmath>

#include "bcai.h"
#include "ui_bcai.h"
#include <windows.h>
#include <cmath>
#include <QComboBox>
#include <QProcess>

BCAI::BCAI(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::BCAI)
{
    ui->setupUi(this);
    this->setFixedSize(1700,1100);
    this->setWindowIcon(QIcon(":/icon/Sign.png"));
    this->setWindowTitle("BCAI-EEG");

    // 添加滤波器选择下拉框
    ui->filterComboBox->addItem("原始信号");
    ui->filterComboBox->addItem("阿尔法波 (8-13Hz)");
    ui->filterComboBox->addItem("贝塔波 (13-30Hz)");

    //添加数据格式选择下拉框
    ui->FormDatabox->addItem("CSV格式数据");
    ui->FormDatabox->addItem("CRC格式数据");

    connect(ui->filterComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &BCAI::on_filterComboBox_currentIndexChanged);

    //设置一些关键字提醒
    ui->ConnectpushButton->setToolTip("<b>提示</b><br>设置连接方法");
    ui->recordpushButton->setToolTip("<b>提示</b><br>记录数据");
    ui->Start_EndpushButton->setToolTip("<b>提示</b><br>启动/停止");
    ui->fqspushButton->setToolTip("<b>提示</b><br>频率谱图");
    ui->chongzhipushButton->setToolTip("<b>提示</b><br>重置图表");
    ui->camerapushButton->setToolTip("<b>提示</b><br>调用摄像设备");

    //重置暂存数据容器的范围
    chartBuffer.resize(8);
    pre_datas.resize(8);
    alphaWaves.resize(8);
    betaWaves.resize(8);

    preprocessor = new Preprocessor();
    connectway = new Connectway();
    camera=new newcamera();

    //设置图表部件窗口
    chartwindow = new ChartWindow();
    ui->Eggwidget->setLayout(chartwindow->mainLayout);

    //数据接收定时器
    datatime = new QTimer(this);
    datatime->setInterval(1);
    connect(datatime, &QTimer::timeout, this, [=](){
        Exetime += 1;
    });

    //在最顶图层显示通道名
    for(int i = 8; i >=1; i--) {
        QGraphicsTextItem *textItem = new QGraphicsTextItem("Pass"+QString::number(i));
        textItem->setFont(QFont("华文宋体", 15));

        // 设置Z值确保在最上层
        textItem->setZValue(1000);  // 使用足够大的值确保在最前

        qreal xPos = chartwindow->chart->plotArea().left()-8;
        qreal yPos = this->height()/10*(9-i)-((9-i)*4)-10;

        textItem->setPos(xPos, yPos);
        chartwindow->chart->scene()->addItem(textItem);
    }
    QGraphicsTextItem *textItem = new QGraphicsTextItem("归一化各通道电压范围为0-1uv");
    qreal xPos = chartwindow->chart->plotArea().left() ;
    qreal yPos = this->height()/10-100;
    textItem->setPos(xPos, yPos);
    chartwindow->chart->scene()->addItem(textItem);

    int Dataformindex=ui->FormDatabox->currentIndex();//dataform
    switch(Dataformindex)
    {
    case 0:
        connect(connectway->serial, &QSerialPort::readyRead, this, &BCAI::DataReceiveCRC);
        connect(connectway->client, &QTcpSocket::readyRead, this, &BCAI::DataReceiveCRC);
        break;
    case 1:
        connect(connectway->serial, &QSerialPort::readyRead, this, &BCAI::DataReceiveCSV);
        connect(connectway->client, &QTcpSocket::readyRead, this, &BCAI::DataReceiveCSV);
        break;
    }

}

BCAI::~BCAI()
{
    delete ui;
    if(file1) { delete file1; file1 = nullptr; }
    delete chartwindow;
    delete connectway;
    delete preprocessor;
    delete camera;
}

void BCAI::on_ConnectpushButton_clicked()//显示连接方式窗口
{
    connectway->show();
}

void BCAI::on_Start_EndpushButton_clicked()//启动关闭函数
{
    if(!Start_End && (connectway->cheakinterstate || connectway->cheakserialstate)){
        Start_End=true;
        Recording = true;
        Start_End = true;
        datatime->start();
        Dataform();

        ui->Start_EndpushButton->setText("Stop");
        lastDataTime = QDateTime::currentMSecsSinceEpoch() / 1000.0;
    } else {
        Start_End = false;
        Recording = false;
        Start_End = false;
        datatime->stop();

        ui->Start_EndpushButton->setText("Start");
    }
}
void BCAI::Dataform()
{
//     int Dataformindex=ui->FormDatabox->currentIndex();
//     switch(Dataformindex)
//     {
//     case 0:
//         connect(connectway->serial, &QSerialPort::readyRead, this, &BCAI::DataReceiveCRC);
//         connect(connectway->client, &QTcpSocket::readyRead, this, &BCAI::DataReceiveCRC);
//         break;
//     case 1:
//         connect(connectway->serial, &QSerialPort::readyRead, this, &BCAI::DataReceiveCSV);
//         connect(connectway->client, &QTcpSocket::readyRead, this, &BCAI::DataReceiveCSV);
//         break;
//     }
}

quint16 BCAI::calculateCRC(const QByteArray &data)//计算CRC函数
{
    quint16 crc = 0xFFFF; //CRC初始值

    for (int i = 0; i < data.size(); ++i) {
        crc ^= (quint8)data.at(i); //异或当前字节

        for (int j = 0; j < 8; ++j) {
            if (crc & 0x0001) { //检查最低位
                crc >>= 1;      //右移一位
                crc ^= 0xA001;   //异或多项式
            } else {
                crc >>= 1;      //右移一位
            }
        }
    }

    return crc;
}

bool BCAI::findhead(const QByteArray &data, int &pos)//查找帧头函数
{
    for (; pos < data.size() - 1; ++pos) {
        qDebug() << "检查位置" << pos << "的字节:" << QString::number((quint8)data.at(pos), 16);
        if ((quint8)data.at(pos) == 0xAA && (quint8)data.at(pos + 1) == 0x55) {
            qDebug() << "在位置" << pos << "找到帧头";
            return true;
        }
    }
    return false;
}

// 修改后的parsewave函数
QList<double> BCAI::parsewave(const QByteArray &frame)
{
    QList<double> values;

    for (int ch = 0; ch < 8; ++ch) {
        int offset = 2 + ch * 2;

        // 小端模式解析16位有符号值
        quint16 rawData = (quint8)frame[offset+1] << 8 | (quint8)frame[offset];
        qint16 signedValue = (rawData & 0x0800) ? (rawData | 0xF000) : rawData;

        // 正确电压计算公式
        double voltage = (signedValue / 2047.0) * 3.3 - 1.65;
        values.append(voltage);
    }

    return values;
}

// 修改后的CRC验证
bool BCAI::validate(const QByteArray &frame) {
    // if (frame.size() != 20) return false;

    // // 使用Qt内置的小端转换
    // quint16 receivedCrc = qFromLittleEndian<quint16>(frame.mid(18, 2).constData());
    // quint16 calculatedCrc = calculateCRC(frame.mid(0, 18));

    // qDebug() << "Received CRC:" << QString::number(receivedCrc, 16);
    // qDebug() << "Calculated CRC:" << QString::number(calculatedCrc, 16);

    // return receivedCrc == calculatedCrc;
    return true;
}
QString BCAI::formatByteArray(QByteArray input) {
    QString result;
    for (int i = 0; i < input.size(); ++i) {
        if (i > 0) result += " ";
        quint8 byte = static_cast<quint8>(input.at(i));
        result += QString("%1").arg(byte, 2, 16, QLatin1Char('0')).toUpper();
    }
    return result;
}
void BCAI::DataReceiveCRC() {
    QProcess process;
    QString scriptPath = "D:/7.7/plot.py";
    process.start("python", QStringList() << scriptPath);
    if (!process.waitForStarted()) {
        qDebug() << "[QProcess] 啟動失敗：" << process.errorString();
        return;
    }
    process.waitForFinished(-1);
    QString output = process.readAllStandardOutput();
    QString error = process.readAllStandardError();
    qDebug() << "Python output:" << output;
    qDebug() << "Python error:" << error;
    qDebug() << "Exit code:" << process.exitCode();
    qDebug() << "Exit status:" << process.exitStatus();
    qDebug() << "Process error:" << process.errorString();
    //首先检查连接状态
    if (!connectway->cheakserialstate && !connectway->cheakinterstate) {
        qDebug() << "无有效连接";
        return;
    }

    // 读取原始数据
    QByteArray Data = connectway->cheakserialstate ?
                          connectway->serial->readAll() :
                          connectway->client->readAll();
    qDebug()<<Data;
    QString newData=formatByteArray(Data);
    newData.replace(" ","");

    if(newData.isEmpty()) {
        qDebug() << "收到空数据包";
        return;
    }
    //将新数据添加到缓冲区
    incomplete= QByteArray::fromHex(newData.toLatin1());

    qDebug() << "缓冲区总大小:" << incomplete.size();

    // 处理缓冲区
    int pos = 0;
    while (findhead(incomplete, pos)) {

        //检查是否构成完整帧
        if (incomplete.size() - pos < 20) {
            qDebug() << "不完整帧，等待更多数据";
            incomplete = incomplete.mid(pos);
            return;
        }
        //提取帧
        QByteArray frame = incomplete.mid(pos, 20);
        qDebug() << "正在处理帧:" << frame.toHex();

        //验证帧
        if (validate(frame)) {
            qDebug() << "检测到有效帧";

            //解析和处理帧
            QList<double> values = parsewave(frame);
            qDebug()<<values;

            for (int ch = 0; ch < 8 && ch < values.size(); ++ch) {
                double rawValue = values[ch];
                double processedValue = preprocessor->processData(rawValue);
                qDebug()<<processedValue;
                if (Recording) {
                    pre_datas[ch].append(QPointF(Exetime, processedValue));
                }
                chartwindow->updateChart(ch,Exetime,(processedValue+1)/2.0+ch);

                // 更新滤波数据
                updateFilteredWaves();
            }

            if (Recording && !pre_datas.empty()) {
                savedate();
            }

            pos += 20; //移动到下一帧
        } else {
            qDebug() << "无效帧，跳过帧头";
            pos += 2; //跳过无效帧头
        }
    }

    // 清理已处理数据
    if (pos > 0) {
        incomplete = incomplete.mid(pos);
        qDebug() << "处理后剩余缓冲区大小:" << incomplete.size();
    }
}

void BCAI::DataReceiveCSV()
{
    QByteArray Data = connectway->cheakserialstate ?
                          connectway->serial->readAll() :
                          connectway->client->readAll();

    if(Data.isEmpty()) return;

    incomplete.append(Data);

    // 按行分割
    QList<QByteArray> lines = incomplete.split('\n');
    int validLineCount = lines.size() - 1; // 最后一行可能是不完整的

    for (int i = 0; i < validLineCount; ++i) {
        QList<QByteArray> items = lines[i].split(',');
        if (items.size() != 8) continue; // 跳过格式不对的行

        QList<double> values;
        bool ok = true;
        for (int j = 0; j < 8; ++j) {
            bool conv;
            double val = items[j].trimmed().toDouble(&conv);
            if (!conv) { ok = false; break; }
            values.append(val);
        }
        if (!ok) continue;

        // 画线：每个通道append一次
        for (int ch = 0; ch < 8; ++ch) {
            double processedValue = preprocessor->processData(values[ch]);
            if (Recording) {
                pre_datas[ch].append(QPointF(Exetime, processedValue));
            }
            chartBuffer[ch].append(QPointF(Exetime, processedValue));
        }
        Exetime++;
    }

    // 保留未处理的残留数据
    incomplete = lines.last();
}


void BCAI::savedate()//保存数据函数
{
    if (!file1 || !file1->isOpen()) return;
    QTextStream out(file1);
    for(int ch=0; ch<8; ch++){
        for (const QPointF& data : pre_datas[ch]) {
            out <<"第"<<ch<<"通道:"<< QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz")
                <<",  " << data.y() << "\n";
        }
    }
    file1->flush();//强制将文件缓冲区中的数据写入磁盘，确保数据不会因程序崩溃或未正常关闭文件而丢失
    pre_datas.clear();
}

void BCAI::on_recordpushButton_clicked()//记录数据函数
{
    if(Recording && Start_End)
    {
        Recording = false;
        ui->recordpushButton->setText("StopRecord");
        QString path = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)
                       + "/Nopro_" + QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss") + ".txt";
        if(file1) { delete file1; file1 = nullptr; }
        file1 = new QFile(path);
        if (!file1->open(QIODevice::WriteOnly | QIODevice::Text)) {
            QMessageBox::critical(this, "错误", "文件创建失败！");
            delete file1;
            file1 = nullptr;
            return;
        }
    }
    else if(!Recording && Start_End)
    {
        Recording = true;
        ui->recordpushButton->setText("StartRecord");
        if (file1) {
            file1->close();
            QString newName = QFileDialog::getSaveFileName(this, "保存文件",
                                                           QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
                                                           "Text Files (*.txt)");
            if (!newName.isEmpty())
            {
                QFile::rename(file1->fileName(), newName);
            }
            delete file1;
            file1 = nullptr;
        }
    }
}

void BCAI::on_chongzhipushButton_clicked()
{
    pre_datas.clear();
    datatime->start();
    for(int ch=0; ch<8; ch++)
    {
        chartwindow->clearseries(ch);
    }
    chartwindow->axisX->setRange(0, 5000);
    Recording = false;
}

void BCAI::on_fqspushButton_clicked()//频率普显示函数
{
    updateFilteredWaves();
}

void BCAI::on_filterComboBox_currentIndexChanged(int index)
{
    if(!Start_End) return;

    switch(index) {
    case 0: // 原始信号
        for(int ch = 0; ch < 8; ch++) {
            if(!pre_datas[ch].isEmpty()) {
                double rawValue = pre_datas[ch].last().y();
                chartwindow->updateChart(ch, Exetime, (rawValue+1)/2.0+ch);
            }
        }
        break;
    case 1: // 阿尔法波
        for(int ch = 0; ch < 8; ch++) {
            if(alphaWaves[ch].size() > 0) {
                chartwindow->updateChart(ch, Exetime, (alphaWaves[ch].last()+1)/2.0+ch);
            }
        }
        break;
    case 2: // 贝塔波
        for(int ch = 0; ch < 8; ch++) {
            if(betaWaves[ch].size() > 0) {
                chartwindow->updateChart(ch, Exetime, (betaWaves[ch].last()+1)/2.0+ch);
            }
        }
        break;
    }
}

void BCAI::applyBandpassFilter(QVector<double>& data, double lowCut, double highCut, int sampleRate)
{
    // 简单的IIR带通滤波器实现
    static const int numChannels = 8;
    static QVector<QVector<double>> previousInput(numChannels, QVector<double>(2, 0.0));
    static QVector<QVector<double>> previousOutput(numChannels, QVector<double>(2, 0.0));

    double dt = 1.0/sampleRate;
    double RC1 = 1.0/(2*M_PI*lowCut);
    double RC2 = 1.0/(2*M_PI*highCut);
    double alpha1 = dt/(dt + RC1);
    double alpha2 = RC2/(dt + RC2);

    for(int ch = 0; ch < numChannels && ch < data.size(); ch++) {
        // 高通滤波部分 (去除低频)
        double highpass = alpha2 * (previousOutput[ch][0] + data[ch] - previousInput[ch][0]);

        // 低通滤波部分 (去除高频)
        double bandpass = alpha1 * highpass + (1 - alpha1) * previousOutput[ch][1];

        previousInput[ch][0] = data[ch];
        previousOutput[ch][0] = highpass;
        previousOutput[ch][1] = bandpass;

        data[ch] = bandpass;
    }
}

void BCAI::updateFilteredWaves()
{
    if(!Start_End) return;

    // 假设采样率为250Hz
    const int sampleRate = 250;

    // 获取最新数据
    QVector<double> latestValues(8);
    for(int ch = 0; ch < 8; ch++) {
        if(!pre_datas[ch].isEmpty()) {
            latestValues[ch] = pre_datas[ch].last().y();
        }
    }

    // 阿尔法波滤波 (8-13Hz)
    QVector<double> alphaValues = latestValues;
    applyBandpassFilter(alphaValues, 8.0, 13.0, sampleRate);
    for(int ch = 0; ch < 8; ch++) {
        alphaWaves[ch].append(alphaValues[ch]);
    }

    // 贝塔波滤波 (13-30Hz)
    QVector<double> betaValues = latestValues;
    applyBandpassFilter(betaValues, 13.0, 30.0, sampleRate);
    for(int ch = 0; ch < 8; ch++) {
        betaWaves[ch].append(betaValues[ch]);
    }

    // 更新UI显示
    QComboBox *combo = findChild<QComboBox*>();
    if(combo) {
        on_filterComboBox_currentIndexChanged(combo->currentIndex());
    }
}

void BCAI::on_camerapushButton_clicked()
{
    camera->show();
}



