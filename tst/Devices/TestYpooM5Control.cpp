#include <gtest/gtest.h>
#include <limits>

#include "devices/horizontreadmill/ypoom5control.h"

using Action = YpooM5Control::Action;

TEST(YpooM5ControlTest, DecodesCapturedTelemetryWithRfuBitsAndExtraBytes) {
    const uint8_t speedPacket[] = {0x9c, 0x25, 0x78, 0x00, 0, 0, 0, 0, 0, 0xff, 0x7f, 0, 0,
                                   0,    0,    0,    0,    0, 0, 0, 0, 2, 0,    0,    0, 0};
    double speed = 0;
    double incline = 0;
    ASSERT_TRUE(YpooM5Control::readTelemetry(speedPacket, sizeof(speedPacket), speed, incline));
    EXPECT_DOUBLE_EQ(speed, 1.2);
    EXPECT_DOUBLE_EQ(incline, 0);
    const uint8_t inclinePacket[] = {0x9c, 0x25, 0x64, 0x00, 0, 0, 0, 0x0a, 0, 0xff, 0x7f, 0, 0,
                                     0,    0,    0,    0,    0, 0, 0, 0,    2, 0,    0,    0, 0};
    ASSERT_TRUE(YpooM5Control::readTelemetry(inclinePacket, sizeof(inclinePacket), speed, incline));
    EXPECT_DOUBLE_EQ(speed, 1);
    EXPECT_DOUBLE_EQ(incline, 1);
}

TEST(YpooM5ControlTest, RejectsTruncatedTelemetryAndSupportsSignedIncline) {
    const uint8_t packet[] = {8, 0, 100, 0, 0xc4, 0xff, 0, 0};
    double speed = 0;
    double incline = 0;
    for (size_t size = 0; size < sizeof(packet); ++size)
        EXPECT_FALSE(YpooM5Control::readTelemetry(packet, size, speed, incline));
    ASSERT_TRUE(YpooM5Control::readTelemetry(packet, sizeof(packet), speed, incline));
    EXPECT_DOUBLE_EQ(speed, 1);
    EXPECT_DOUBLE_EQ(incline, -6);
    const uint8_t moreData[] = {9, 0, 0, 0, 0, 0, 0, 0};
    EXPECT_FALSE(YpooM5Control::readTelemetry(moreData, sizeof(moreData), speed, incline));
}

TEST(YpooM5ControlTest, TargetsNeverStartAStoppedBelt) {
    YpooM5Control control;
    control.telemetry(0, 0, 0);
    EXPECT_FALSE(control.requestSpeed(1.2));
    EXPECT_FALSE(control.requestIncline(1));
    EXPECT_EQ(control.next(0).action, Action::None);
}

TEST(YpooM5ControlTest, RepeatedStartDuringCountdownDoesNotRestartTheCountdown) {
    YpooM5Control control;
    control.requestStart();
    EXPECT_EQ(control.next(0).action, Action::Start);
    control.requestStart();
    control.telemetry(0, 0, 1000);
    EXPECT_EQ(control.next(1000).action, Action::None);
}

TEST(YpooM5ControlTest, StopIsANoOpWhenFreshTelemetryAlreadyConfirmsRest) {
    YpooM5Control control;
    control.telemetry(0, 0, 0);
    control.requestStop();
    EXPECT_EQ(control.next(200).action, Action::None);
    control.requestStart();
    EXPECT_EQ(control.next(400).action, Action::Start);
}

TEST(YpooM5ControlTest, WorkoutTargetsWaitForMotionInsteadOfAThreeSecondTimer) {
    YpooM5Control control;
    control.telemetry(0, 0, 0);
    control.requestStart();
    ASSERT_TRUE(control.requestSpeed(1.2));
    ASSERT_TRUE(control.requestIncline(1));
    EXPECT_EQ(control.next(0).action, Action::Start);
    control.telemetry(0, 0, 3000);
    EXPECT_EQ(control.next(3000).action, Action::None);
    control.telemetry(1, 0, 4500);
    const auto speed = control.next(4500);
    EXPECT_EQ(speed.action, Action::Speed);
    EXPECT_DOUBLE_EQ(speed.value, 1.2);
    EXPECT_EQ(control.next(4700).action, Action::None);
    const auto incline = control.next(4800);
    EXPECT_EQ(incline.action, Action::Incline);
    EXPECT_DOUBLE_EQ(incline.value, 1);
}

TEST(YpooM5ControlTest, LatestWorkoutTargetReplacesTheCountdownTarget) {
    YpooM5Control control;
    control.requestStart();
    ASSERT_TRUE(control.requestSpeed(1.2));
    EXPECT_EQ(control.next(0).action, Action::Start);
    ASSERT_TRUE(control.requestSpeed(1.4));
    control.telemetry(1, 0, 3500);
    EXPECT_DOUBLE_EQ(control.next(3500).value, 1.4);
}

TEST(YpooM5ControlTest, CapturedSpeedFailureNeedsTelemetryNotAnAckRetryLoop) {
    YpooM5Control control;
    control.telemetry(1, 0, 0);
    ASSERT_TRUE(control.requestSpeed(1.2));
    EXPECT_EQ(control.next(0).action, Action::Speed);
    // Hardware returns 80 02 04 2a 03 yet the next data packets report 1.2.
    control.telemetry(1.2, 0, 450);
    EXPECT_EQ(control.next(450).action, Action::None);
    control.telemetry(1.2, 0, 3000);
    EXPECT_EQ(control.next(3000).action, Action::None);
}

TEST(YpooM5ControlTest, UnconfirmedTargetsHaveBoundedSpacedRetries) {
    YpooM5Control control;
    control.telemetry(1, 0, 0);
    ASSERT_TRUE(control.requestSpeed(1.2));
    EXPECT_EQ(control.next(0).action, Action::Speed);
    EXPECT_EQ(control.next(200).action, Action::None);
    for (int64_t now : {2500, 5000}) {
        control.telemetry(1, 0, now);
        EXPECT_EQ(control.next(now).action, Action::Speed);
    }
    control.telemetry(1, 0, 7500);
    EXPECT_EQ(control.next(7500).action, Action::None);
    EXPECT_EQ(control.takeFailure(), Action::Speed);
    EXPECT_EQ(control.next(8000).action, Action::None);
}

TEST(YpooM5ControlTest, StopDuringCountdownCancelsTargetsAndRetriesUntilStopped) {
    YpooM5Control control;
    control.requestStart();
    ASSERT_TRUE(control.requestSpeed(1.2));
    ASSERT_TRUE(control.requestIncline(1));
    EXPECT_EQ(control.next(0).action, Action::Start);
    control.requestStop();
    EXPECT_EQ(control.next(200).action, Action::None);
    EXPECT_EQ(control.next(300).action, Action::Stop);
    control.stopResponse(false); // The M5 rejects stop during its countdown.
    control.telemetry(0, 0, 500);
    EXPECT_EQ(control.next(500).action, Action::None);
    EXPECT_EQ(control.next(1300).action, Action::Stop);
    control.telemetry(1, 0, 3200);
    EXPECT_EQ(control.next(3200).action, Action::Stop);
    control.stopResponse(true);
    control.telemetry(0, 0, 3600);
    EXPECT_EQ(control.next(4000).action, Action::None);
    control.telemetry(1, 0, 5000); // A later physical start must not replay targets.
    EXPECT_EQ(control.next(5000).action, Action::None);
}

TEST(YpooM5ControlTest, StopAcknowledgementAloneDoesNotConfirmAStoppedBelt) {
    YpooM5Control control;
    control.telemetry(1.2, 0, 0);
    control.requestStop();
    EXPECT_EQ(control.next(0).action, Action::Stop);
    control.stopResponse(true);
    control.telemetry(1.2, 0, 500);
    EXPECT_EQ(control.next(1000).action, Action::Stop);
    control.stopResponse(true);
    control.telemetry(0, 0, 1500);
    EXPECT_EQ(control.next(2000).action, Action::None);
}

TEST(YpooM5ControlTest, StopWinsOverStartAndZeroSpeedMeansStop) {
    YpooM5Control control;
    control.requestStop();
    control.requestStart();
    EXPECT_FALSE(control.requestSpeed(1.2));
    EXPECT_EQ(control.next(0).action, Action::Stop);
    control.reset();
    control.telemetry(1, 0, 0);
    ASSERT_TRUE(control.requestSpeed(0));
    EXPECT_EQ(control.next(0).action, Action::Stop);
}

TEST(YpooM5ControlTest, MissingStartupTelemetryCancelsTargetsAndRequestsStop) {
    YpooM5Control control;
    control.requestStart();
    ASSERT_TRUE(control.requestIncline(1));
    EXPECT_EQ(control.next(0).action, Action::Start);
    EXPECT_EQ(control.next(10000).action, Action::Stop);
    EXPECT_EQ(control.takeFailure(), Action::Start);
}

TEST(YpooM5ControlTest, StaleTelemetryBlocksTargetWrites) {
    YpooM5Control control;
    control.telemetry(1, 0, 0);
    ASSERT_TRUE(control.requestSpeed(1.2));
    EXPECT_EQ(control.next(2100).action, Action::None);
    control.telemetry(1, 0, 2500);
    EXPECT_EQ(control.next(2500).action, Action::Speed);
}

TEST(YpooM5ControlTest, PhysicalStopAndDisconnectDiscardQueuedCommands) {
    YpooM5Control control;
    control.telemetry(1, 0, 0);
    ASSERT_TRUE(control.requestSpeed(1.2));
    control.telemetry(0, 0, 450);
    control.telemetry(1, 0, 900);
    EXPECT_EQ(control.next(900).action, Action::None);
    ASSERT_TRUE(control.requestIncline(1));
    control.reset();
    control.telemetry(1, 0, 1500);
    EXPECT_EQ(control.next(1500).action, Action::None);
}

TEST(YpooM5ControlTest, FailedStopDoesNotPermitUnexpectedRestarts) {
    YpooM5Control control;
    control.requestStop();
    for (int64_t now = 0; now < 6000; now += 1000)
        EXPECT_EQ(control.next(now).action, Action::Stop);
    EXPECT_EQ(control.next(6000).action, Action::None);
    EXPECT_EQ(control.takeFailure(), Action::Stop);
    control.requestStart();
    control.telemetry(1, 0, 6500);
    EXPECT_FALSE(control.requestSpeed(1.2));
    EXPECT_EQ(control.next(6500).action, Action::None);
}

TEST(YpooM5ControlTest, EnforcesAdvertisedRangesAndResolution) {
    YpooM5Control control;
    control.telemetry(1, 0, 0);
    EXPECT_FALSE(control.requestSpeed(-1));
    EXPECT_FALSE(control.requestSpeed(0.5));
    EXPECT_FALSE(control.requestSpeed(18.9));
    EXPECT_FALSE(control.requestSpeed(std::numeric_limits<double>::infinity()));
    EXPECT_FALSE(control.requestIncline(-7));
    EXPECT_FALSE(control.requestIncline(16));
    EXPECT_FALSE(control.requestIncline(std::numeric_limits<double>::quiet_NaN()));
    ASSERT_TRUE(control.requestSpeed(1.24));
    EXPECT_DOUBLE_EQ(control.next(0).value, 1.2);
    ASSERT_TRUE(control.requestIncline(1.4));
    EXPECT_DOUBLE_EQ(control.next(300).value, 1);
    ASSERT_TRUE(control.requestIncline(-6));
    EXPECT_DOUBLE_EQ(control.next(600).value, -6);
}
