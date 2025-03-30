#include "ActivationMonitor.h"
#include <gtest/gtest.h>

class ActivationMonitorTests : public ::testing::Test {
protected:
    int64_t currentTime = 0;

    // Mock time provider function
    int64_t mockTimeProvider() {
        return currentTime;
    }

    void advanceTimeByMilliseconds(int64_t ms) {
        currentTime += ms;
    }
};

TEST_F(ActivationMonitorTests, InitiallyInactive) {
    ActivationMonitor monitor([this]() { return mockTimeProvider(); });

    ASSERT_FALSE(monitor.isActive());
}

TEST_F(ActivationMonitorTests, TriggersActivationWhenThresholdExceeded) {
    ActivationMonitor monitor([this]() { return mockTimeProvider(); });

    bool eventTriggered = false;
    bool eventState = false;

    monitor.setActivationCallback([&](bool state) {
        eventTriggered = true;
        eventState = state;
    });

    monitor.setActivationThreshold(0.6);
    monitor.setDeactivationThreshold(0.4);
    monitor.setSamplingPeriod(1000);

    monitor.update(true);
    monitor.update(true);
    monitor.update(true);

    advanceTimeByMilliseconds(10);

    ASSERT_TRUE(eventTriggered);
    ASSERT_TRUE(eventState);
    ASSERT_TRUE(monitor.isActive());
}

TEST_F(ActivationMonitorTests, TriggersDeactivationWhenThresholdFallsBelow) {
    ActivationMonitor monitor([this]() { return mockTimeProvider(); });

    bool eventTriggered = false;
    bool eventState = false;

    monitor.setActivationCallback([&](bool state) {
        eventTriggered = true;
        eventState = state;
    });

    monitor.update(true);
    monitor.update(true);
    monitor.update(true);

    advanceTimeByMilliseconds(10);
    ASSERT_TRUE(monitor.isActive());

    eventTriggered = false;
    
    monitor.update(false);
    monitor.update(false);
    monitor.update(false);

    advanceTimeByMilliseconds(10);

    ASSERT_TRUE(eventTriggered);
    ASSERT_FALSE(eventState);
    ASSERT_FALSE(monitor.isActive());
}

TEST_F(ActivationMonitorTests, NoEventIfStateUnchanged) {
    ActivationMonitor monitor([this]() { return mockTimeProvider(); });

    bool eventTriggered = false;

    monitor.setActivationCallback([&](bool) {
        eventTriggered = true;
    });

    monitor.update(true);
    monitor.update(true);
    monitor.update(true);

    advanceTimeByMilliseconds(10);

    ASSERT_TRUE(eventTriggered);

    eventTriggered = false;

    // Send more active samples, no state change
    monitor.update(true);
    monitor.update(true);

    advanceTimeByMilliseconds(10);

    ASSERT_FALSE(eventTriggered);
    ASSERT_TRUE(monitor.isActive());
}

TEST_F(ActivationMonitorTests, ThresholdsCanBeModified) {
    ActivationMonitor monitor([this]() { return mockTimeProvider(); });

    monitor.setActivationThreshold(0.8);
    monitor.setDeactivationThreshold(0.2);

    monitor.update(true);
    monitor.update(true);

    advanceTimeByMilliseconds(10);

    ASSERT_TRUE(monitor.isActive());
}

TEST_F(ActivationMonitorTests, SamplingPeriodCanBeModified) {
    ActivationMonitor monitor([this]() { return mockTimeProvider(); });

    monitor.setSamplingPeriod(500);

    ASSERT_EQ(monitor.isActive(), false);
}
