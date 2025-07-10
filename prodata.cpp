#include "prodata.h"

Preprocessor::IIRFilter::IIRFilter(const std::vector<double>& b_coeffs,
                                   const std::vector<double>& a_coeffs)
    : b(b_coeffs), a(a_coeffs) {
    reset();// 初始化时清空历史数据
}

void Preprocessor::IIRFilter::reset() {
    x_history.assign(b.size(), 0.0);
    y_history.assign(a.size()-1, 0.0);
}

double Preprocessor::IIRFilter::process(double input) {
        // 更新输入历史
    x_history.push_front(input);
    if(x_history.size() > b.size()) x_history.pop_back();
   // 计算输出
    double output = 0.0;
    for(size_t i=0; i<b.size(); i++) {
        if(i < x_history.size()) {
            output += b[i] * x_history[i];
        }
    }
    for(size_t i=1; i<a.size(); i++) {
        if(i-1 < y_history.size()) {
            output -= a[i] * y_history[i-1];
        }
    }
    output /= a[0];
    // 更新输出历史
    y_history.push_front(output);
    if(y_history.size() > a.size()-1) y_history.pop_back();

    return output;
}

Preprocessor::Preprocessor()
    : hpFilter({0.9604, -0.9604}, {1.0, -0.9208}),// 1Hz高通
    lpFilter({0.0468, 0.0468}, {1.0, -0.9064}),// 40Hz低通
    notchFilter({0.9695, -1.9021, 0.9695}, {1.0, -1.9021, 0.9391})// 50Hz带阻
{
    ringBuffer.resize(DEFAULT_BUFFER_SIZE, 0.0);
    ringHead = 0;
    ringCount = 0;
    windowSum = 0.0;
    windowSqSum = 0.0;
    mean = 0.0;
    m2 = 0.0;
}
 // 典型脑电范围检查(单位:μV)
bool Preprocessor::checkInputValid(double input) {
    const double MAX_EEG_VALUE = 1000.0;
    return (std::abs(input) <= MAX_EEG_VALUE);
}

void Preprocessor::addSample(double value) {
    if (ringCount < DEFAULT_BUFFER_SIZE) {
        ringBuffer[ringHead] = value;
        windowSum += value;
        windowSqSum += value * value;
        double delta = value - mean;
        mean += delta / (ringCount + 1);
        m2 += delta * (value - mean);
        ringCount++;
    } else {
        double old = ringBuffer[ringHead];
        windowSum += value - old;
        windowSqSum += value * value - old * old;
        // Welford算法增量移除旧值
        double oldMean = mean;
        mean += (value - old) / DEFAULT_BUFFER_SIZE;
        m2 += (value - old) * (value - mean + old - oldMean);
        ringBuffer[ringHead] = value;
    }
    ringHead = (ringHead + 1) % DEFAULT_BUFFER_SIZE;
}

void Preprocessor::updateNormalizationParams() {
    std::lock_guard<std::mutex> lock(normMutex);
    if (ringCount == 0) return;
    double stddev = std::sqrt(m2 / ringCount);
    minVal = mean - 3 * stddev;
    maxVal = mean + 3 * stddev;
    const double MIN_RANGE = 0.1;
    if (maxVal - minVal < MIN_RANGE) {
        double center = (maxVal + minVal) / 2;
        minVal = center - MIN_RANGE / 2;
        maxVal = center + MIN_RANGE / 2;
    }
}

double Preprocessor::processData(double input) {
    if(!checkInputValid(input)) {
        qWarning() << "Invalid EEG input value:" << input;
        return 0.0;
    }

    double filtered = notchFilter.process(input);
    filtered = hpFilter.process(filtered);
    filtered = lpFilter.process(filtered);

    addSample(filtered);

    if(++sampleCount >= DEFAULT_BUFFER_SIZE) {
        updateNormalizationParams();
        sampleCount = 0;
    }

    double normMin, normMax;
    {
        std::lock_guard<std::mutex> lock(normMutex);
        normMin = minVal;
        normMax = maxVal;
    }
    double normalized = (filtered - normMin) / (normMax - normMin + std::numeric_limits<double>::epsilon());
    return std::clamp(normalized, 0.0, 1.0);
}
