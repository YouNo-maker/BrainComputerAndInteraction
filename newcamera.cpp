#include "newcamera.h"
#include "ui_newcamera.h"
#include <QDebug>
#include <QVBoxLayout>
#include <QCameraInfo>

newcamera::newcamera(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::newcamera)
{
    ui->setupUi(this);
    this->setWindowTitle("摄像窗口");
    this->setWindowIcon(QIcon(":/icon/camera.png"));
    setMinimumSize(640, 480);
    QVBoxLayout *layout = new QVBoxLayout(this);
    view = new QCameraViewfinder(this);//创建视图查找器
    layout->addWidget(view);
    layout->addWidget(ui->btnStartCamera);
    ui->btnStartCamera->setStyleSheet(
        "QPushButton {"
        "   font: 9pt \"Mongolian Baiti\";"
        "   border: 2px solid #cccccc;"
        "   border-radius: 10px;"
        "   padding: 5px 15px;"
        "   background-color: rgb(255, 255, 255);"
        "}"
        "QPushButton:hover {"
        "   background-color: rgb(184, 184, 184);"
        "}"
        );
    this->setLayout(layout);

    usecamera = nullptr;//初始化摄像头

    capture = nullptr;//初始化照片获取

    ui->btnStartCamera->setText("查找可用摄像设备");
    connect(ui->btnStartCamera, &QPushButton::clicked, this, &newcamera::onStartCamera_clicked);
}

newcamera::~newcamera()
{
    delete ui;
    stopCamera();
}


bool newcamera::startCamera()
{
    if(QCameraInfo::availableCameras().isEmpty()) {
        qDebug() << "没有可用的摄像头设备";
        return false;

    }

    if(!usecamera) {
        QCameraInfo cameraInfo = QCameraInfo::defaultCamera();// 获取默认摄像头
        usecamera = new QCamera(cameraInfo, this);// 创建并设置摄像头
        usecamera->setViewfinder(view);

        capture = new QCameraImageCapture(usecamera, this);
        usecamera->setCaptureMode(QCamera::CaptureViewfinder);// 设置捕获模式
    }

    // 启动摄像头
    usecamera->start();
    return true;
}

void newcamera::stopCamera()
{
    if (usecamera) {
        usecamera->stop();
        delete usecamera;
        usecamera = nullptr;
    }

    if (capture) {
        delete capture;
        capture = nullptr;
    }
}

bool newcamera::isCameraActive() const
{
    return usecamera && usecamera->state() == QCamera::ActiveState;

}
void newcamera::onStartCamera_clicked()
{
    if(!usecamera) {
        if(!startCamera()) {  // 尝试初始化摄像头
            qDebug() << "无法启动摄像头";
            return;
        }
    }

    if(usecamera->state() != QCamera::ActiveState) {
        usecamera->start();
        ui->btnStartCamera->setText("停止摄像头");
    } else {
        usecamera->stop();
        ui->btnStartCamera->setText("启动摄像头");
    }
}


