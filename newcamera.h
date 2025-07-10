#ifndef NEWCAMERA_H
#define NEWCAMERA_H

#include <QWidget>
#include <QCamera>
#include <QCameraViewfinder>
#include <QCameraImageCapture>
#include <QCameraInfo>

namespace Ui {
class newcamera;
}

class newcamera : public QWidget
{
    Q_OBJECT

public:
    explicit newcamera(QWidget *parent = nullptr);
    ~newcamera();
    bool startCamera();
    void stopCamera();
    bool isCameraActive() const;

private:
    Ui::newcamera *ui;
    QCamera *usecamera;//启动/停止摄像头（start()/stop()）设置捕获模式（拍照/录像/预览）调整摄像头参数（分辨率、帧率等）


    QCameraViewfinder *view;//实时预览组件，继承自QWidget，用于显示摄像头捕获的画面。需通过m_camera->setViewfinder(m_viewfinder)绑定到摄像头。
    QCameraImageCapture *capture;//捕获静态图像（capture()方法）设置保存路径（文件或内存）通过信号imageCaptured处理捕获结果

private slots:
    void onStartCamera_clicked();
};

#endif // NEWCAMERA_H
