#include "featureextractor.h"
#include <complex>
#include <numeric>
#include <algorithm>
#include <cmath>

FeatureExtractor::FeatureExtractor(FeatureChart* chart)
    : featureChart(chart) {
    timeDomainBuffer.resize(FeatureExtractor::WINDOW_SIZE);
}

void  FeatureExtractor::updateFeatures( QVector<double> samples) {
    // 更新时域缓冲区
    for(double sample : samples) {
        timeDomainBuffer.push_back(sample);
        if(timeDomainBuffer.size() > WINDOW_SIZE) {
            timeDomainBuffer.pop_front();
        }
    }

    if(timeDomainBuffer.size() < WINDOW_SIZE/2) return;

    // 计算时域特征
    QVector<double> features;
    features.append(calculateMean());
    features.append(calculateVariance());
    features.append(calculateStdDev());
    features.append(calculatePeakToPeak());
    features.append(calculateRMS());
    features.append(calculateZeroCrossingRate());

    // 计算非线性特征
    features.append(calculateActivity());
    features.append(calculateMobility());
    features.append(calculateComplexity());

    // 计算频域特征
    std::vector<double> fftMagnitudes;
    calculateFFT(fftMagnitudes);

    features.append(calculateBandPower(4, 8, fftMagnitudes));   // Theta
    features.append(calculateBandPower(8, 13, fftMagnitudes));  // Alpha
    features.append(calculateBandPower(13, 30, fftMagnitudes)); // Beta
    features.append(calculateBandPower(30, 45, fftMagnitudes)); // Gamma

    // 计算特征比值
    double alpha = features[10];
    double theta = features[9];
    double beta = features[11];
    double gamma = features[12];

    features.append(alpha / (theta + 1e-6)); // A/T
    features.append((beta + gamma) / (alpha + theta + 1e-6)); // (B+G)/(A+


    // 更新图表
    if(featureChart){
    QMetaObject::invokeMethod(featureChart, [=](){
        featureChart->updateFeatures(features);
        }, Qt::QueuedConnection);
    }
}



// 辅助函数：计算信号的均值
double FeatureExtractor::calculateMean() const {
    if(timeDomainBuffer.empty()) return 0.0;
    return std::accumulate(timeDomainBuffer.begin(), timeDomainBuffer.end(), 0.0) / timeDomainBuffer.size();
}

// 计算方差
double FeatureExtractor::calculateVariance() const {
    if(timeDomainBuffer.size() < 2) return 0.0;
    double mean = calculateMean();
    double variance = 0.0;
    for(double val : timeDomainBuffer) {
        variance += (val - mean) * (val - mean);
    }
    return variance / (timeDomainBuffer.size() - 1);
}

// 计算标准差
double FeatureExtractor::calculateStdDev() const {
    return std::sqrt(calculateVariance());
}

// 计算峰峰值(最大值-最小值)
double FeatureExtractor::calculatePeakToPeak() const {
    if(timeDomainBuffer.empty()) return 0.0;
    auto minmax = std::minmax_element(timeDomainBuffer.begin(), timeDomainBuffer.end());
    return *minmax.second - *minmax.first;
}

// 计算均方根(RMS)
double FeatureExtractor::calculateRMS() const {
    if(timeDomainBuffer.empty()) return 0.0;
    double sum = 0.0;
    for(double val : timeDomainBuffer) {
        sum += val * val;
    }
    return std::sqrt(sum / timeDomainBuffer.size());
}

// 计算过零率
double FeatureExtractor::calculateZeroCrossingRate() const {
    if(timeDomainBuffer.size() < 2) return 0.0;

    int crossings = 0;
    for(size_t i = 1; i < timeDomainBuffer.size(); ++i) {
        if((timeDomainBuffer[i-1] < 0 && timeDomainBuffer[i] >= 0) ||
            (timeDomainBuffer[i-1] > 0 && timeDomainBuffer[i] <= 0)) {
            crossings++;
        }
    }
    return static_cast<double>(crossings) / (timeDomainBuffer.size() - 1);
}

// 计算活动性(方差)
double FeatureExtractor::calculateActivity() const {
    return calculateVariance();
}

// 计算移动性(一阶导数方差/原始信号方差)
double FeatureExtractor::calculateMobility() const {
    if(timeDomainBuffer.size() < 2) return 0.0;

    // 计算一阶差分
    std::vector<double> diffs;
    for(size_t i = 1; i < timeDomainBuffer.size(); ++i) {
        diffs.push_back(timeDomainBuffer[i] - timeDomainBuffer[i-1]);
    }

    // 计算差分方差
    double diffMean = std::accumulate(diffs.begin(), diffs.end(), 0.0) / diffs.size();
    double diffVar = 0.0;
    for(double d : diffs) {
        diffVar += (d - diffMean) * (d - diffMean);
    }
    diffVar /= diffs.size();

    // 计算移动性
    double var = calculateVariance();
    return std::sqrt(diffVar / (var + 1e-6));
}

// 计算复杂性(二阶导数方差/一阶导数方差)
double FeatureExtractor::calculateComplexity() const {
    if(timeDomainBuffer.size() < 3) return 0.0;

    // 计算一阶差分
    std::vector<double> firstDiffs;
    for(size_t i = 1; i < timeDomainBuffer.size(); ++i) {
        firstDiffs.push_back(timeDomainBuffer[i] - timeDomainBuffer[i-1]);
    }

    // 计算二阶差分
    std::vector<double> secondDiffs;
    for(size_t i = 1; i < firstDiffs.size(); ++i) {
        secondDiffs.push_back(firstDiffs[i] - firstDiffs[i-1]);
    }

    // 计算二阶差分方差
    double secondDiffMean = std::accumulate(secondDiffs.begin(), secondDiffs.end(), 0.0) / secondDiffs.size();
    double secondDiffVar = 0.0;
    for(double d : secondDiffs) {
        secondDiffVar += (d - secondDiffMean) * (d - secondDiffMean);
    }
    secondDiffVar /= secondDiffs.size();

    // 计算一阶差分方差
    double firstDiffMean = std::accumulate(firstDiffs.begin(), firstDiffs.end(), 0.0) / firstDiffs.size();
    double firstDiffVar = 0.0;
    for(double d : firstDiffs) {
        firstDiffVar += (d - firstDiffMean) * (d - firstDiffMean);
    }
    firstDiffVar /= firstDiffs.size();

    return std::sqrt(secondDiffVar / (firstDiffVar + 1e-6));
}

// 计算FFT(使用Cooley-Tukey算法)
void FeatureExtractor::calculateFFT(std::vector<double>& magnitudes) {
    magnitudes.clear();
    if(timeDomainBuffer.size() < 2) return;

    // 补零到FFT_SIZE
    size_t n = std::min(timeDomainBuffer.size(), FFT_SIZE);
    std::vector<std::complex<double>> fftInput(FFT_SIZE);
    for(size_t i = 0; i < n; ++i) {
        fftInput[i] = std::complex<double>(timeDomainBuffer[i], 0.0);
    }
    for(size_t i = n; i < FFT_SIZE; ++i) {
        fftInput[i] = std::complex<double>(0.0, 0.0);
    }

    // 执行FFT
    fft(fftInput);

    // 计算幅度谱(只取前一半)
    magnitudes.resize(FFT_SIZE / 2);
    for(size_t i = 0; i < FFT_SIZE / 2; ++i) {
        magnitudes[i] = std::abs(fftInput[i]) / (FFT_SIZE / 2);
    }
}

// FFT实现(原地计算)
void FeatureExtractor::fft(std::vector<std::complex<double>>& x) {
    const size_t N = x.size();
    if(N <= 1) return;

    // 分离偶数和奇数项
    std::vector<std::complex<double>> even(N/2);
    std::vector<std::complex<double>> odd(N/2);
    for(size_t i = 0; i < N/2; ++i) {
        even[i] = x[i*2];
        odd[i] = x[i*2 + 1];
    }

    // 递归计算
    fft(even);
    fft(odd);

    // 合并结果
    for(size_t k = 0; k < N/2; ++k) {
        std::complex<double> t = std::polar(1.0, -2 * M_PI * k / N) * odd[k];
        x[k] = even[k] + t;
        x[k + N/2] = even[k] - t;
    }
}

// 计算频带能量
double FeatureExtractor::calculateBandPower(double lowFreq, double highFreq,
                                            const std::vector<double>& magnitudes) const {
    if(magnitudes.empty()) return 0.0;

    double deltaFreq = SAMPLE_RATE / FFT_SIZE;
    size_t lowBin = static_cast<size_t>(lowFreq / deltaFreq);
    size_t highBin = static_cast<size_t>(highFreq / deltaFreq);

    // 确保不越界
    lowBin = std::min(lowBin, magnitudes.size() - 1);
    highBin = std::min(highBin, magnitudes.size() - 1);

    if(lowBin >= highBin) return 0.0;

    double power = 0.0;
    for(size_t i = lowBin; i <= highBin; ++i) {
        power += magnitudes[i] * magnitudes[i];
    }

    return power / (highBin - lowBin + 1);
}
