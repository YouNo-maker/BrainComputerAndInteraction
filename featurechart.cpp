#include "featurechart.h"

FeatureChart::FeatureChart(QWidget *parent)
    : QMainWindow{parent}
{

    // 创建特征图表
    m_featureChart = new QChart();
    m_featureChart->setTitle("脑电特征");

    // 创建条形图系列
    m_featureSeries = new QBarSeries();
    m_featureChart->addSeries(m_featureSeries);
    m_featureChartview =new  QChartView(m_featureChart);
    m_timeDomainSet = new QBarSet("时域特征");
   // m_freqDomainSet = new QBarSet("频域特征");

    // 添加特征类别
    QStringList categories;
    categories << "均值" << "方差" << "标准差" << "峰峰值" << "RMS" << "过零率"
               << "活动性" << "移动性" << "复杂性" << "Theta" << "Alpha"
               << "Beta" << "Gamma" << "A/T" << "(B+G)/(A+T)";

    m_featureAxisX = new QBarCategoryAxis();
    m_featureAxisX->append(categories);
    m_featureChart->addAxis(m_featureAxisX, Qt::AlignBottom);

    m_featureAxisY = new QValueAxis();
    m_featureAxisY->setRange(0, 1);
    m_featureChart->addAxis(m_featureAxisY, Qt::AlignLeft);

    m_featureSeries->attachAxis(m_featureAxisX);
    m_featureSeries->attachAxis(m_featureAxisY);

     QWidget* mainWidget = new QWidget();

    // 添加到布局
    QChartView *featureView = new QChartView(m_featureChart);
    FeaturemainLayout=new QVBoxLayout(mainWidget);
    FeaturemainLayout->addWidget(featureView);
    this->setCentralWidget(mainWidget);

    // 设置柱状图样式
    //m_timeDomainSet->setColor(QColor(65, 105, 225)); // 时域特征 - 蓝色
    //m_freqDomainSet->setColor(QColor(220, 20, 60));  // 频域特征 - 红色

    // 设置图例
    m_featureChart->legend()->setVisible(true);
    m_featureChart->legend()->setAlignment(Qt::AlignBottom);
}

FeatureChart::~FeatureChart()
{

    delete m_featureChart;

}
// 修改FeatureChart的updateFeatures方法
void FeatureChart::updateFeatures(const QVector<double>& features) {
    // 确保特征数量匹配
    if(features.size() != 15) return;

    // 清除旧数据
    m_featureSeries->clear();

    // 创建新的数据集
    m_timeDomainSet = new QBarSet("时域特征");
    m_freqDomainSet = new QBarSet("频域特征");

    // 添加时域特征数据 (前9个)
    for(int i=0; i<15; ++i) {
        if(features[i]<0)
        {
            m_timeDomainSet->append(abs(features[i]));
        }
        else m_timeDomainSet->append(features[i]);
    }

    // 添加频域特征数据 (后6个)


    // 添加到系列
    m_featureSeries->append(m_timeDomainSet);


    // 设置柱状图样式
    m_timeDomainSet->setColor(QColor(65, 105, 225)); // 特征 - 蓝色
    // 自动调整Y轴范围
    double maxValue = *std::max_element(features.begin(), features.end());
    //double minValue=  *std::min_element(features.begin(), features.end());
    m_featureAxisY->setRange(0, maxValue * 1.2);

    // 更新图表
    m_featureChart->update();
}
