#ifndef CHARTWINDOW_H
#define CHARTWINDOW_H

#include <QMainWindow>
#include <QtCharts>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QScrollBar>
#include <QMainWindow>

class ChartWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit ChartWindow(QWidget *parent = nullptr);
    ~ChartWindow();

    QChart *chart;
    QChartView *chartView;
    QValueAxis *axisX;
    QValueAxis *axisY;
    QScrollBar *horizontalScrollBar;
    QVBoxLayout* mainLayout;

    static constexpr double VIEW_WINDOW = 5000;//最大横坐标范围
    static constexpr int MAX_POINTS = 5000;//最多储存数据范围
    QVector<QLineSeries*> seriesList;//曲线容器
    void updateChart(int channel, double time, double value);//更新函数
     void clearseries(int channel);//清理曲线函数

private slots:
    void onHorizontalScrollBarValueChanged(int value);//滚动条槽函数

};

#endif // CHARTWINDOW_H
