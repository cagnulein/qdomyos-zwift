#ifndef ACTIVATION_MONITOR_H
#define ACTIVATION_MONITOR_H

#include <queue>
#include <functional>
#include <cstdint>
#include <chrono>
#include <algorithm>

class ActivationMonitor {
public:
    using TimeProvider = std::function<int64_t()>;
    using ActivationCallback = std::function<void(bool)>;

    // Constructor with time provider
    explicit ActivationMonitor(TimeProvider timeProvider = DefaultTimeProvider);

    void update(bool isActive);
    void setActivationThreshold(double threshold);
    void setDeactivationThreshold(double threshold);
    void setSamplingPeriod(int64_t milliseconds);
    bool isActive() const;

    void setActivationCallback(ActivationCallback callback);

private:
    struct Sample {
        int64_t timestamp;
        bool isActive;
    };

    std::queue<Sample> samples;
    int activeSampleCount = 0;
    double activationThreshold = 0.7;
    double deactivationThreshold = 0.3;
    int64_t samplingPeriod = 1000;  // in milliseconds
    bool currentState = false;

    TimeProvider timeProvider;
    ActivationCallback activationCallback = nullptr;

    void cleanupOldSamples();
    double calculateMeanActivation() const;

    static int64_t DefaultTimeProvider();
};

#endif // ACTIVATION_MONITOR_H
