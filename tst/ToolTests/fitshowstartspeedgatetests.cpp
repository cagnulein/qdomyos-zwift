#include <gtest/gtest.h>

#include "devices/fitshowtreadmill/fitshowstartspeedgate.h"
#include "devices/fitshowtreadmill/fitshowstopcommand.h"

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

TEST(FitShowStopCommand, ExplicitStopAlwaysUsesPhysicalStop) {
    for (bool merach : {false, true}) {
        for (bool stopOnly : {false, true}) {
            EXPECT_EQ(0x03, fitshowStopCommand(false, merach, stopOnly));
        }
    }
}

TEST(FitShowStopCommand, OrdinaryPauseAndMerachPauseAreUnchanged) {
    EXPECT_EQ(0x06, fitshowStopCommand(true, false, false));
    EXPECT_EQ(0x0a, fitshowStopCommand(true, true, false));
}

TEST(FitShowStopCommand, BlacklordPauseUsesCapturedPhysicalStop) {
    EXPECT_EQ(0x03, fitshowStopCommand(true, false, true));
    const uint8_t command = fitshowStopCommand(true, false, true);
    EXPECT_EQ(0x50, 0x53 ^ command); // 02 53 03 50 03
}

TEST(FitShowStopCommand, StopAfterPauseDoesNotReusePauseCommand) {
    EXPECT_EQ(0x06, fitshowStopCommand(true, false, false));
    EXPECT_EQ(0x03, fitshowStopCommand(false, false, false));
}
