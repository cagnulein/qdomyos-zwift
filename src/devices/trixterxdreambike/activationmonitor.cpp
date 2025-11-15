#include "activationmonitor.h"

ActivationMonitor::ActivationMonitor(const int64_t samplingPeriod, const double activationThreshold, const double deactivationThreshold) {
    this->samplingPeriod = std::max<int64_t>(samplingPeriod, 1);

    this->activationThreshold = std::clamp(activationThreshold, 0.0, 1.0);
    this->deactivationThreshold = std::clamp(deactivationThreshold, 0.0, activationThreshold);
}

void ActivationMonitor::update(const bool isActive, const int64_t now) {

    // Add new sample
    samples.push({now, isActive});
    if (isActive) {
        ++activeSampleCount;
    }

    // Remove outdated samples
    cleanupOldSamples(now);

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

void ActivationMonitor::cleanupOldSamples(const int64_t now) {
    int64_t cutoffTime = now - samplingPeriod;

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

bool ActivationMonitor::isActive() const {
    return currentState;
}

void ActivationMonitor::setActivationCallback(ActivationCallback callback) {
    this->activationCallback = callback;
}

