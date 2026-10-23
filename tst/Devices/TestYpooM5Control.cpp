#include <gtest/gtest.h>
#include <limits>

#include "devices/horizontreadmill/ypoom5control.h"

using Action = YpooM5Control::Action;

TEST(YpooM5ControlTest, ReadinessRequiresFreshTelemetryFromTheCurrentConnection) {
    YpooM5Control control;
    EXPECT_FALSE(control.hasFreshTelemetry(0));
    control.telemetry(0, 0, 500);
    EXPECT_TRUE(control.hasFreshTelemetry(500));
    EXPECT_TRUE(control.hasFreshTelemetry(2500));
    EXPECT_FALSE(control.hasFreshTelemetry(2501));
    control.reset();
    EXPECT_FALSE(control.hasFreshTelemetry(2600));
}

TEST(YpooM5ControlTest, PhysicalMotionAllowsRecordingWithoutIssuingAnotherStart) {
    YpooM5Control control;
    EXPECT_FALSE(control.canResumeRecording(0));
    control.telemetry(0, 0, 0);
    EXPECT_FALSE(control.canResumeRecording(0));
    // Captured run: the console started the belt while QZ still considered itself stopped.
    control.telemetry(9.5, 0, 1000);
    EXPECT_TRUE(control.canResumeRecording(1000));
    EXPECT_EQ(control.next(1000).action, Action::None);
    EXPECT_FALSE(control.canResumeRecording(3001));
    control.reset();
    EXPECT_FALSE(control.canResumeRecording(3500));
}

TEST(YpooM5ControlTest, StopDecelerationCannotReopenAStoppedRecording) {
    YpooM5Control control;
    control.telemetry(9.5, 0, 0);
    control.requestStop();
    EXPECT_FALSE(control.canResumeRecording(0));
    EXPECT_EQ(control.next(0).action, Action::Stop);
    control.stopResponse(true);
    control.telemetry(8.4, 0, 500);
    EXPECT_FALSE(control.canResumeRecording(500));
    control.telemetry(1, 0, 3000);
    EXPECT_FALSE(control.canResumeRecording(3000));
    control.telemetry(0, 0, 3500);
    EXPECT_FALSE(control.canResumeRecording(3500));
    // A later physical restart is a new recording, not a replayed Start command.
    control.telemetry(1, 0, 4500);
    EXPECT_TRUE(control.canResumeRecording(4500));
    EXPECT_EQ(control.next(4500).action, Action::None);
}

TEST(YpooM5ControlTest, CancelledCountdownAndFailedStopCannotResumeRecording) {
    YpooM5Control control;
    control.telemetry(0, 0, 0);
    control.requestStart();
    EXPECT_EQ(control.next(0).action, Action::Start);
    EXPECT_FALSE(control.canResumeRecording(0));
    control.requestStop();
    control.telemetry(1, 0, 3000); // Brief motion while cancelling the countdown.
    EXPECT_FALSE(control.canResumeRecording(3000));
    for (int64_t now = 3000; now < 9000; now += 1000) {
        control.telemetry(1, 0, now);
        EXPECT_EQ(control.next(now).action, Action::Stop);
        EXPECT_FALSE(control.canResumeRecording(now));
    }
    control.telemetry(1, 0, 9000);
    EXPECT_EQ(control.next(9000).action, Action::None);
    EXPECT_EQ(control.takeFailure(), Action::Stop);
    EXPECT_FALSE(control.canResumeRecording(9000));
}

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

TEST(YpooM5ControlTest, ConvertsYpooM5InclinationLevelsToRealPercentages) {
    EXPECT_DOUBLE_EQ(YpooM5Control::percentToLevel(1), 1.8);
    EXPECT_DOUBLE_EQ(YpooM5Control::percentToLevel(6), 10.5);
    EXPECT_DOUBLE_EQ(YpooM5Control::percentToLevel(12), 21);
    EXPECT_DOUBLE_EQ(YpooM5Control::levelToPercent(10.5), 6);
    EXPECT_DOUBLE_EQ(YpooM5Control::levelToPercent(21), 12);
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

TEST(YpooM5ControlTest, CancelledCountdownUsesStableRestAfterStartupDeadlineWithoutRetryingStart) {
    YpooM5Control control;
    control.telemetry(0, 0, 0);
    control.requestStart();
    EXPECT_EQ(control.next(0).action, Action::Start);
    control.requestStop();
    for (int64_t now = 1000; now <= 6000; now += 1000) {
        control.telemetry(0, 0, now);
        EXPECT_EQ(control.next(now).action, Action::Stop);
        control.stopResponse(false);
    }
    for (int64_t now = 7000; now < 10000; now += 1000) {
        control.telemetry(0, 0, now);
        control.requestStart(); // A new start cannot interrupt an unsettled cancellation.
        EXPECT_EQ(control.next(now).action, Action::None);
        EXPECT_EQ(control.takeFailure(), Action::None);
    }
    // An update tick at the deadline must wait for the next live sample.
    EXPECT_EQ(control.next(10000).action, Action::None);
    EXPECT_EQ(control.takeFailure(), Action::None);
    control.telemetry(0, 0, 10000);
    EXPECT_EQ(control.next(10000).action, Action::None); // No automatic restart.
    control.requestStart();
    EXPECT_EQ(control.next(10500).action, Action::Start); // A new explicit start works.
}

TEST(YpooM5ControlTest, StaleRestCannotConfirmCountdownCancellation) {
    YpooM5Control control;
    control.telemetry(0, 0, 0);
    control.requestStart();
    EXPECT_EQ(control.next(0).action, Action::Start);
    control.requestStop();
    for (int64_t now = 1000; now <= 6000; now += 1000)
        EXPECT_EQ(control.next(now).action, Action::Stop);
    EXPECT_EQ(control.next(10000).action, Action::None);
    EXPECT_EQ(control.takeFailure(), Action::Stop);
    control.requestStart();
    EXPECT_EQ(control.next(10500).action, Action::None);
}

TEST(YpooM5ControlTest, TelemetryGapsRestartTheStableRestObservationForCountdownCancellation) {
    YpooM5Control control;
    control.telemetry(0, 0, 0);
    control.requestStart();
    EXPECT_EQ(control.next(0).action, Action::Start);
    control.requestStop();
    EXPECT_EQ(control.next(500).action, Action::Stop);
    control.stopResponse(false);
    control.telemetry(0, 0, 10000); // Fresh sample, but no evidence about the intervening gap.
    control.requestStart();
    EXPECT_EQ(control.next(10000).action, Action::Stop);
    control.telemetry(0, 0, 11000);
    control.requestStart();
    EXPECT_EQ(control.next(11000).action, Action::Stop);
    control.telemetry(0, 0, 12000); // Two seconds of continuous live rest samples.
    EXPECT_EQ(control.next(12000).action, Action::None);
    control.requestStart();
    EXPECT_EQ(control.next(12500).action, Action::Start);
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

TEST(YpooM5ControlTest, YpM1552ConfirmsQuantizedInclineFromCapturedFirmware) {
    YpooM5Control control;
    control.telemetry(1.2, 0, 0);
    // Percent targets and the corresponding integer levels reported by YP-M1552 firmware.
    const struct { double target; double reportedLevel; } cases[] = {
        {1, 1}, {2, 3}, {3, 5}, {4, 7}, {5, 8}, {6, 10}, {8, 14}, {12, 21}
    };
    int64_t now = 0;
    for (const auto &entry : cases) {
        ASSERT_TRUE(control.requestIncline(entry.target));
        const auto command = control.next(now, true);
        ASSERT_EQ(command.action, Action::Incline);
        EXPECT_DOUBLE_EQ(command.value, entry.target);

        const double feedback = YpooM5Control::levelToPercent(entry.reportedLevel);
        control.telemetry(1.2, feedback, now + 500);
        EXPECT_EQ(control.next(now + 500, true).action, Action::None);
        control.telemetry(1.2, feedback, now + 2600);
        EXPECT_EQ(control.next(now + 2600, true).action, Action::None);
        EXPECT_EQ(control.takeFailure(), Action::None);
        now += 3000;
    }
}

TEST(YpooM5ControlTest, StandardYptmU0InclineConfirmationRemainsUnchanged) {
    YpooM5Control control;
    control.telemetry(1.2, 0, 0);
    ASSERT_TRUE(control.requestIncline(2));
    EXPECT_EQ(control.next(0).action, Action::Incline);
    const double ypM1552Feedback = YpooM5Control::levelToPercent(3);
    control.telemetry(1.2, ypM1552Feedback, 500);
    EXPECT_EQ(control.next(500).action, Action::None);
    control.telemetry(1.2, ypM1552Feedback, 2600);
    // Without the YP-M1552 flag, the target must not be mistaken for a confirmed 2%.
    EXPECT_EQ(control.next(2600).action, Action::Incline);
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

TEST(YpooM5ControlTest, HalfStepInclineRequestsMoveInBothDirectionsIncludingDecline) {
    struct Example {
        double actual;
        double requested;
        double expected;
    };
    const Example examples[] = {{0, 0.5, 1}, {1, 0.5, 0}, {2, 2.5, 3}, {3, 2.5, 2},
                                {0, -0.5, -1}, {-1, -0.5, 0}, {-2, -2.5, -3}, {-3, -2.5, -2},
                                {-6, -5.5, -5}, {-5, -5.5, -6}, {14, 14.5, 15}, {15, 14.5, 14}};
    for (const auto &example : examples) {
        SCOPED_TRACE(example.requested);
        YpooM5Control control;
        control.telemetry(1, example.actual, 0);
        ASSERT_TRUE(control.requestIncline(example.requested));
        const auto command = control.next(0);
        EXPECT_EQ(command.action, Action::Incline);
        EXPECT_DOUBLE_EQ(command.value, example.expected);
    }
}

TEST(YpooM5ControlTest, HalfStepDuplicatesDoNotResetRetriesWhileMovementIsPending) {
    YpooM5Control control;
    control.telemetry(1, 3, 0);
    ASSERT_TRUE(control.requestIncline(2.5));
    EXPECT_DOUBLE_EQ(control.next(0).value, 2);
    for (int64_t now : {2500, 5000}) {
        control.telemetry(1, 3, now);
        ASSERT_TRUE(control.requestIncline(2.5));
        const auto command = control.next(now);
        EXPECT_EQ(command.action, Action::Incline);
        EXPECT_DOUBLE_EQ(command.value, 2);
    }
    control.telemetry(1, 3, 7500);
    ASSERT_TRUE(control.requestIncline(2.5));
    EXPECT_EQ(control.next(7500).action, Action::None);
    EXPECT_EQ(control.takeFailure(), Action::Incline);
}

TEST(YpooM5ControlTest, PlusThenMinusCanSendTheSameHalfStepAfterThePreviousMovementCompletes) {
    for (bool confirmBeforeNextRequest : {false, true}) {
        for (double halfStep : {0.5, -0.5}) {
            YpooM5Control control;
            control.telemetry(1, 0, 0);
            ASSERT_TRUE(control.requestIncline(halfStep));
            const double firstTarget = halfStep > 0 ? 1 : -1;
            EXPECT_DOUBLE_EQ(control.next(0).value, firstTarget);
            control.telemetry(1, firstTarget, 500);
            if (confirmBeforeNextRequest)
                EXPECT_EQ(control.next(500).action, Action::None);
            ASSERT_TRUE(control.requestIncline(halfStep));
            const auto command = control.next(1000);
            EXPECT_EQ(command.action, Action::Incline);
            EXPECT_DOUBLE_EQ(command.value, 0);
        }
    }
}

TEST(YpooM5ControlTest, StopAndDisconnectDiscardPendingHalfStepTargets) {
    for (bool disconnect : {false, true}) {
        YpooM5Control control;
        control.telemetry(1, 3, 0);
        ASSERT_TRUE(control.requestIncline(2.5));
        EXPECT_DOUBLE_EQ(control.next(0).value, 2);
        if (disconnect)
            control.reset();
        else {
            control.requestStop();
            EXPECT_EQ(control.next(500).action, Action::Stop);
            control.stopResponse(true);
            control.telemetry(0, 2, 1000);
        }
        control.telemetry(1, 2, 1500);
        EXPECT_EQ(control.next(1500).action, Action::None);
        ASSERT_TRUE(control.requestIncline(2.5));
        EXPECT_DOUBLE_EQ(control.next(2000).value, 3);
    }
}
