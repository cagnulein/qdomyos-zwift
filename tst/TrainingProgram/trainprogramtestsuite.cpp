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
