#ifndef ACTIVATION_MONITOR_H
#define ACTIVATION_MONITOR_H

#include <queue>
#include <functional>
#include <cstdint>

class ActivationMonitor {
public:
    using ActivationCallback = std::function<void(bool)>;

    /**
     * @brief ActivationMonitor
     * @param samplingPeriod The sampling period length in the units that will be used for th eupdate method.
     */
    explicit ActivationMonitor(const int64_t samplingPeriod, const double activationThreshold, const double deactivationThreshold);

    void update(const bool isActive, const int64_t now);

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
    int64_t samplingPeriod = 1000;
    bool currentState = false;

    ActivationCallback activationCallback = nullptr;

    void cleanupOldSamples(const int64_t now);
    double calculateMeanActivation() const;

};

#endif // ACTIVATION_MONITOR_H
