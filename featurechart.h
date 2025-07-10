#ifndef FEATURECHART_H
#define FEATURECHART_H

#include <QMainWindow>
#include <QtCharts>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>

class FeatureChart : public QMainWindow
{
    Q_OBJECT
public:
    explicit FeatureChart(QWidget *parent = nullptr);
    ~ FeatureChart();

    QVBoxLayout* FeaturemainLayout;
    QChart *m_featureChart;
    QChartView *m_featureChartview;
    QBarSeries *m_featureSeries;
    QBarSet *m_timeDomainSet;
    QBarSet *m_freqDomainSet;
    QValueAxis *m_featureAxisY;
    QBarCategoryAxis *m_featureAxisX;

    void updateFeatures(const QVector<double> &features);
signals:
private:
    void setChart(QChart *);
};

#endif // FEATURECHART_H
