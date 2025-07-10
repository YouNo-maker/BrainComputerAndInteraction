#include "chartwindow.h"

ChartWindow::ChartWindow(QWidget *parent)
    : QMainWindow{parent}
{
    chart = new QChart();
    axisX = new QValueAxis();
    axisY=new QValueAxis();
    chartView = new QChartView(chart);

    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY,Qt::AlignLeft);

    //y轴相关设置
    axisY->setTitleText("电压(uv)");
    axisY->setRange(0, 8);
    axisY->setGridLineVisible(false);
    axisY->setLabelsVisible(false);//隐藏坐标
    axisY->setTickCount(9);
    QPen axisYPen_one(QColor("#5c5c5c"));
    axisYPen_one.setWidth(2);
    axisY->setLinePen(axisYPen_one);

    //x轴相关设置
    axisX->setTitleText("时间(ms)");
    axisX->setRange(0,5000);
    axisX->setGridLineVisible(false);  // 隐藏 X 轴网格线
    axisX->setTickCount(11);
    QPen axisXPen(QColor("#5c5c5c"));  // 颜色
    axisXPen.setWidth(2);    // 线宽
    axisX->setLinePen(axisXPen);

    // 创建第一个通道-----------------
    QLineSeries *firstSeries = new QLineSeries();
    firstSeries->setName("通道一");
    chart->addSeries(firstSeries);
    firstSeries->attachAxis(axisX);
    firstSeries->attachAxis(axisY);
    seriesList.append(firstSeries);

    //----------------------

    // 创建第二个通道-----------------
    QLineSeries *secondSeries = new QLineSeries();
    secondSeries->setName("通道二");
    chart->addSeries(secondSeries);
    secondSeries->attachAxis(axisX);
    secondSeries->attachAxis(axisY);
    seriesList.append(secondSeries);
    //----------------------

    // 创建第三个通道-----------------
    QLineSeries *thirdSeries = new QLineSeries();
    thirdSeries->setName("通道三");
    chart->addSeries(thirdSeries);
    thirdSeries->attachAxis(axisX);
    thirdSeries->attachAxis(axisY);
    seriesList.append(thirdSeries);
    //----------------------

    // 创建第四个通道-----------------
    QLineSeries *fourthSeries = new QLineSeries();
    fourthSeries->setName("通道四");
    chart->addSeries(fourthSeries);
    fourthSeries->attachAxis(axisX);
    fourthSeries->attachAxis(axisY);
    seriesList.append(fourthSeries);
    //----------------------

    // 创建第五个通道-----------------
    QLineSeries *fifthSeries = new QLineSeries();
    fifthSeries->setName("通道五");
    chart->addSeries(fifthSeries);
    fifthSeries->attachAxis(axisX);
    fifthSeries->attachAxis(axisY);
    seriesList.append(fifthSeries);
    //----------------------

    // 创建第六个通道-----------------
    QLineSeries *sixthSeries = new QLineSeries();
    sixthSeries->setName("通道六");
    chart->addSeries(sixthSeries);
    sixthSeries->attachAxis(axisX);
    sixthSeries->setColor(QColor("#aaaaff"));
    sixthSeries->attachAxis(axisY);
    seriesList.append(sixthSeries);
    //----------------------

    // 创建第七个通道-----------------
    QLineSeries *seventhSeries = new QLineSeries();
    seventhSeries->setName("通道七");
    chart->addSeries(seventhSeries);
    seventhSeries->attachAxis(axisX);
    seventhSeries->attachAxis(axisY);
    seventhSeries->setColor(QColor("#ff5500"));
    seriesList.append(seventhSeries);
    //----------------------

    // 创建第八个通道-----------------
    QLineSeries *eighthSeries = new QLineSeries();
    eighthSeries->setName("通道八");
    chart->addSeries(eighthSeries);
    eighthSeries->attachAxis(axisX);
    eighthSeries->attachAxis(axisY);
    eighthSeries->setColor(QColor("#00aa00"));
    seriesList.append(eighthSeries);


    chart->setBackgroundBrush(QBrush(QColor("#f7f7f7")));//设置背景色
    chartView->setRenderHint(QPainter::Antialiasing, true);//启用抗锯齿
    chart->setAnimationOptions(QChart::NoAnimation);//禁用动画效果
    chartView->setContentsMargins(0, 0, 0, 0);  //消除边距
    chart->layout()->setContentsMargins(0, 0, 0, 0);  // 布局边距归零
    chart->setMargins(QMargins(0, 0, 0, 0));          // 图表边距归零


    //布局设置
    QWidget* mainWidget = new QWidget();
    horizontalScrollBar = new QScrollBar(Qt::Horizontal, this);
    horizontalScrollBar->setMinimum(0);
    horizontalScrollBar->setMaximum(5000);
    horizontalScrollBar->setSingleStep(100);
    horizontalScrollBar->setPageStep(100);

    mainLayout = new QVBoxLayout(mainWidget);
    mainLayout->setContentsMargins(10,10, 10, 10);
    mainLayout->setSpacing(10);

    mainLayout->addWidget(chartView);
    mainLayout->addWidget(horizontalScrollBar);
    this->setCentralWidget(mainWidget);

    // QPainter painter(this);
    // painter.setFont(QFont("华文宋体",20));
    // for(int i=1;i<=8;i++)
    // {
    //     painter.drawText(height()/2,QChar::Wide,QString::number(i));
    // }
    // 在初始化或需要添加文字时调用


    //绑定滑动条滚动槽函数
    connect(horizontalScrollBar, &QScrollBar::valueChanged, this, &ChartWindow::onHorizontalScrollBarValueChanged);
}

void ChartWindow::clearseries(int channel)
{
    if (seriesList.isEmpty() || channel < 0 || channel >= seriesList.size()) {
        return;
    }
    seriesList[channel]->clear();
}

ChartWindow::~ChartWindow()
{
    qDeleteAll(seriesList);  // 删除所有系列对象
    seriesList.clear();
    delete chart;
    delete chartView;
    delete axisX;
    delete axisY;  // 原代码中未删除axisY，此处需补充
}

void ChartWindow::updateChart(int channel, double time, double value)
{
    if (channel < 0 || channel >= seriesList.size()) return;

    if (seriesList[channel]->count() > MAX_POINTS) {
        // 直接截断头部多余数据
        seriesList[channel]->remove(0, seriesList[channel]->count() - MAX_POINTS);
    }

    if(time > axisX->max())
    {
        axisX->setMax(time);
        axisX->setMin(time - VIEW_WINDOW);
        horizontalScrollBar->setMaximum(time);
    }
    seriesList[channel]->append(time, value);

    chart->update();
}

void ChartWindow::onHorizontalScrollBarValueChanged(int value)
{
    double viewWidth = axisX->max() - axisX->min();
    double minX = value;
    double maxX = minX + viewWidth;
    axisX->setRange(minX, maxX);
}
