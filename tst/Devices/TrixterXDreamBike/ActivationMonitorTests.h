#include "devices/trixterxdreamv1bike/ActivationMonitor.h"
#include <gtest/gtest.h>

class ActivationMonitorTests : public ::testing::Test {
protected:
    ActivationMonitorTests() {}
};

TEST_F(ActivationMonitorTests, InitiallyInactive) {
    ActivationMonitor monitor(1000, 0.8, 0.1);

    ASSERT_FALSE(monitor.isActive());
}

TEST_F(ActivationMonitorTests, TriggersActivationWhenThresholdExceeded) {
    ActivationMonitor monitor(1000, 0.6, 0.4);

    bool eventTriggered = false;
    bool eventState = false;

    monitor.setActivationCallback([&](bool state) {
        eventTriggered = true;
        eventState = state;
    });

    int64_t now=0;
    monitor.update(true, now+=10);
    monitor.update(false, now+=10);
    monitor.update(true, now+=10);
    monitor.update(false, now+=10);
    monitor.update(true, now+=10);
    monitor.update(true, now+=10);
    monitor.update(true, now+=10);
    monitor.update(true, now+=10);

    ASSERT_FALSE(eventTriggered);
    ASSERT_FALSE(eventState);
    ASSERT_FALSE(monitor.isActive());

    monitor.update(true, now+=10);
    monitor.update(true, now+=10);

    ASSERT_TRUE(eventTriggered);
    ASSERT_TRUE(eventState);
    ASSERT_TRUE(monitor.isActive());
}

TEST_F(ActivationMonitorTests, TriggersDeactivationWhenThresholdFallsBelow) {
    ActivationMonitor monitor(1000, 0.8, 0.1);

    bool eventTriggered = false;
    bool eventState = false;

    monitor.setActivationCallback([&](bool state) {
        eventTriggered = true;
        eventState = state;
    });

    monitor.update(true, 10);
    monitor.update(true, 20);
    monitor.update(true, 30);


    ASSERT_TRUE(monitor.isActive());

    eventTriggered = false;
    
    monitor.update(false, 10);
    monitor.update(false, 20);
    monitor.update(false, 30);

    ASSERT_TRUE(eventTriggered);
    ASSERT_FALSE(eventState);
    ASSERT_FALSE(monitor.isActive());
}

TEST_F(ActivationMonitorTests, NoEventIfStateUnchanged) {
    ActivationMonitor monitor(1000, 0.8, 0.1);

    bool eventTriggered = false;

    monitor.setActivationCallback([&](bool) {
        eventTriggered = true;
    });

    monitor.update(true, 10);
    monitor.update(true, 20);
    monitor.update(true, 30);

    ASSERT_TRUE(eventTriggered);

    eventTriggered = false;

    // Send more active samples, no state change
    monitor.update(true, 40);
    monitor.update(true, 50);


    ASSERT_FALSE(eventTriggered);
    ASSERT_TRUE(monitor.isActive());
}


