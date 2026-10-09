#include "trainprogramtestsuite.h"
#include "trainprogram.h"

namespace {
trainrow timedRow(int seconds) {
    trainrow row;
    row.duration = QTime(0, 0, 0, 0).addSecs(seconds);
    return row;
}

trainrow lapButtonRow() {
    trainrow row;
    row.waitForLap = true;
    return row;
}

trainrow heartRateAboveRow(int bpm) {
    trainrow row;
    row.HRabove = bpm;
    return row;
}
} // namespace

void TrainProgramTestSuite::test_lapButtonBarrierBlocksSkippedZeroDurationRow() {
    const QList<trainrow> rows = {timedRow(120), lapButtonRow(), timedRow(30)};

    EXPECT_EQ(trainprogram::firstBlockingLapButtonRow(rows, 0, 2), 1)
        << "A zero-duration lap-button row must block the transition to the later timed row.";
}

void TrainProgramTestSuite::test_lapButtonBarrierFindsRowBeforeWorkoutEnd() {
    const QList<trainrow> rows = {timedRow(120), lapButtonRow()};

    EXPECT_EQ(trainprogram::firstBlockingLapButtonRow(rows, 0, rows.length()), 1)
        << "A lap-button row must block the workout from ending just because its duration is zero.";
}

void TrainProgramTestSuite::test_lapButtonBarrierIgnoresRowsOutsideCandidateRange() {
    const QList<trainrow> rows = {timedRow(120), timedRow(30), lapButtonRow()};

    EXPECT_EQ(trainprogram::firstBlockingLapButtonRow(rows, 0, 1), -1)
        << "Only lap-button rows crossed by the candidate transition should block it.";
}

void TrainProgramTestSuite::test_heartRateThresholdBarrierBlocksSkippedZeroDurationRow() {
    const QList<trainrow> rows = {timedRow(120), heartRateAboveRow(155), timedRow(30)};

    EXPECT_EQ(trainprogram::firstBlockingTransitionRow(rows, 0, 2), 1)
        << "A zero-duration heart-rate threshold row must block the transition to the later timed row.";
}

void TrainProgramTestSuite::test_heartRatePidSuppressionRequiresStartedTrainingProgram() {
    EXPECT_FALSE(trainprogram::isHeartRatePidSuppressed(false, true, 5, true))
        << "HR PID suppression must not affect ordinary workouts when no training program is active.";
}

void TrainProgramTestSuite::test_heartRatePidSuppressionCoversForcedSpeedRows() {
    EXPECT_TRUE(trainprogram::isHeartRatePidSuppressed(true, true, 120, false))
        << "A forcespeed row must own treadmill speed for its entire duration.";
}

void TrainProgramTestSuite::test_heartRatePidSuppressionCoversRowBoundaryWindow() {
    EXPECT_TRUE(trainprogram::isHeartRatePidSuppressed(true, false, 10, false));
    EXPECT_TRUE(trainprogram::isHeartRatePidSuppressed(true, false, 0, false));
    EXPECT_FALSE(trainprogram::isHeartRatePidSuppressed(true, false, 11, false))
        << "Rows must regain HR PID control outside the ten-second pre-boundary window.";
}

void TrainProgramTestSuite::test_heartRatePidSuppressionCoversPostTransitionWindow() {
    EXPECT_TRUE(trainprogram::isHeartRatePidSuppressed(true, false, 120, true))
        << "The post-transition settling window must suppress HR PID commands.";
}
