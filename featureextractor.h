// 防止头文件被多次包含的宏定义
#ifndef FEATUREEXTRACTOR_H
#define FEATUREEXTRACTOR_H

// 包含必要的标准库头文件
#include <vector>    // 向量容器
#include <deque>     // 双端队列容器
#include <cmath>     // 数学函数
#include <algorithm> // 算法函数
#include <complex>   // 复数支持
#include "featurechart.h" // 特征图表显示类


// 特征提取器类定义
class FeatureExtractor {
public:
    // 静态常量定义（单位：Hz）
    static constexpr size_t WINDOW_SIZE = 250; // 分析窗口大小（1秒数据，250Hz采样率）
    static constexpr size_t FFT_SIZE = 256;    // FFT计算点数（补零到256点）
    static constexpr double SAMPLE_RATE = 250.0; // 采样频率（250Hz）

    // 构造函数
    FeatureExtractor(FeatureChart* chart); // 参数：特征图表显示对象指针

    // 主更新接口
   void   updateFeatures( QVector<double> samples); // 参数：输入信号样本

    // FFT计算函数
    void fft(std::vector<std::complex<double> > &x); // 参数：复数信号（输入/输出）

private:
    // 成员变量
   QPointer<FeatureChart> featureChart;   // 特征图表显示对象指针
    std::deque<double> timeDomainBuffer; // 时域数据缓冲区（滑动窗口）

    // ========== 时域特征计算 ==========
    double calculateMean() const;       // 计算信号均值
    double calculateVariance() const;   // 计算信号方差
    double calculateStdDev() const;     // 计算标准差（方差的平方根）
    double calculatePeakToPeak() const; // 计算峰峰值（最大值-最小值）
    double calculateRMS() const;       // 计算均方根值（信号能量）
    double calculateZeroCrossingRate() const; // 计算过零率

    // ========== 频域特征计算 ==========
    void calculateFFT(std::vector<double>& magnitudes); // 计算FFT幅度谱
    double calculateBandPower(double lowFreq, double highFreq,
                              const std::vector<double>& magnitudes) const; // 计算频带能量

    // ========== 非线性特征计算 ==========
    double calculateActivity() const;   // 计算活动性（信号变化强度）
    double calculateMobility() const;   // 计算移动性（主频变化）
    double calculateComplexity() const;  // 计算复杂性（信号不规则度）
};

// 头文件结束宏
#endif // FEATUREEXTRACTOR_H
