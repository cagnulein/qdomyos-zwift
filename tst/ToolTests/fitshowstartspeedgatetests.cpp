#include <gtest/gtest.h>

#include "devices/fitshowtreadmill/fitshowstartspeedgate.h"

// The FitShow firmware may ACK speed commands while in its start countdown,
// without applying those commands to the motor. Keep the queued workout target
// until the physical machine reports RUNNING.
TEST(FitShowStartSpeedGate, InitialWorkoutSpeedWaitsForPhysicalRunning) {
    FitShowStartSpeedGate gate;

    // Speed may arrive before the driver's update() processes requestStart.
    EXPECT_FALSE(gate.canSendTarget(true, false));
    gate.startSent();
    EXPECT_TRUE(gate.waitingForRunning());
    EXPECT_FALSE(gate.canSendTarget(false, false)); // START countdown

    gate.runningConfirmed();
    EXPECT_FALSE(gate.waitingForRunning());
    EXPECT_TRUE(gate.canSendTarget(false, false));
}

TEST(FitShowStartSpeedGate, InclinationIsHeldAlongsideSpeed) {
    FitShowStartSpeedGate gate;
    gate.startSent();

    // Both fields share the FITSHOW_CONTROL_TARGET_OR_RUN packet.
    EXPECT_FALSE(gate.canSendTarget(false, false));
    gate.runningConfirmed();
    EXPECT_TRUE(gate.canSendTarget(false, false));
}

TEST(FitShowStartSpeedGate, StopRequestPreventsMotorCommand) {
    FitShowStartSpeedGate gate;
    EXPECT_FALSE(gate.canSendTarget(false, true));
    gate.startSent();
    EXPECT_FALSE(gate.canSendTarget(false, true));
    gate.cancel();
    EXPECT_FALSE(gate.canSendTarget(false, true));
}

TEST(FitShowStartSpeedGate, CancelledStartupDoesNotRetainGate) {
    FitShowStartSpeedGate gate;
    gate.startSent();
    gate.cancel();
    EXPECT_FALSE(gate.waitingForRunning());
    EXPECT_TRUE(gate.canSendTarget(false, false));
}

TEST(FitShowStartSpeedGate, RestartRequiresNewRunningStatus) {
    FitShowStartSpeedGate gate;
    gate.startSent();
    gate.runningConfirmed();
    EXPECT_TRUE(gate.canSendTarget(false, false));

    gate.startSent();
    EXPECT_FALSE(gate.canSendTarget(false, false));
    gate.runningConfirmed();
    EXPECT_TRUE(gate.canSendTarget(false, false));
}

TEST(FitShowStartSpeedGate, OrdinaryRunningSpeedChangesAreUnchanged) {
    FitShowStartSpeedGate gate;
    // Existing users controlling an already running treadmill are unaffected.
    EXPECT_TRUE(gate.canSendTarget(false, false));
}
