#include "activationmonitor.h"

ActivationMonitor::ActivationMonitor(TimeProvider timeProvider)
    : timeProvider(std::move(timeProvider)) {}

void ActivationMonitor::update(bool isActive) {
    int64_t now = timeProvider();

    // Add new sample
    samples.push({now, isActive});
    if (isActive) {
        ++activeSampleCount;
    }

    // Remove outdated samples
    cleanupOldSamples();

    // Calculate mean activation
    double mean = calculateMeanActivation();

    // Trigger activation or deactivation event
    if (!currentState && mean >= activationThreshold) {
        currentState = true;
        if (activationCallback) {
            activationCallback(true);
        }
    } else if (currentState && mean < deactivationThreshold) {
        currentState = false;
        if (activationCallback) {
            activationCallback(false);
        }
    }
}

void ActivationMonitor::cleanupOldSamples() {
    int64_t cutoffTime = timeProvider() - samplingPeriod;

    while (!samples.empty() && samples.front().timestamp < cutoffTime) {
        if (samples.front().isActive) {
            --activeSampleCount;
        }
        samples.pop();
    }
}

double ActivationMonitor::calculateMeanActivation() const {
    if (samples.empty()) {
        return 0.0;
    }
    return static_cast<double>(activeSampleCount) / samples.size();
}

void ActivationMonitor::setActivationThreshold(double threshold) {
    activationThreshold = std::clamp(threshold, 0.0, 1.0);
}

void ActivationMonitor::setDeactivationThreshold(double threshold) {
    deactivationThreshold = std::clamp(threshold, 0.0, 1.0);
}

void ActivationMonitor::setSamplingPeriod(int64_t milliseconds) {
    samplingPeriod = std::max<int64_t>(milliseconds, 1);
}

bool ActivationMonitor::isActive() const {
    return currentState;
}

void ActivationMonitor::setActivationCallback(ActivationCallback callback) {
    activationCallback = std::move(callback);
}

int64_t ActivationMonitor::DefaultTimeProvider() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
               std::chrono::system_clock::now().time_since_epoch())
        .count();
}
