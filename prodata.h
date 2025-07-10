#ifndef PREPROCESSOR_H
#define PREPROCESSOR_H

#include <deque>
#include <cmath>
#include <algorithm>
#include <limits>
#include <QDebug>
#include <vector>
#include <mutex>

class Preprocessor {
public:
    static constexpr size_t DEFAULT_BUFFER_SIZE = 250;
    static constexpr double DEFAULT_SAMPLE_RATE = 250.0;

    Preprocessor();
    double processData(double input);

    double getCurrentMin() const { return minVal; }
    double getCurrentMax() const { return maxVal; }
    size_t getWindowSize() const { return ringCount; }

private:
    class IIRFilter {
    public:
        IIRFilter(const std::vector<double>& b_coeffs,
                  const std::vector<double>& a_coeffs);
        double process(double input);
        void reset();
    private:
        std::vector<double> b, a;
        std::deque<double> x_history;
        std::deque<double> y_history;
    };

    IIRFilter hpFilter;
    IIRFilter lpFilter;
    IIRFilter notchFilter;

    // 环形缓冲区
    std::vector<double> ringBuffer;
    size_t ringHead = 0;
    size_t ringCount = 0;

    // 增量统计
    double windowSum = 0.0;
    double windowSqSum = 0.0;
    double mean = 0.0;
    double m2 = 0.0;

    double minVal = 0.0;
    double maxVal = 1.0;
    size_t sampleCount = 0;

    std::mutex normMutex;

    void updateNormalizationParams();
    void addSample(double value);
    bool checkInputValid(double input);
};

#endif // PREPROCESSOR_H
