#include "trainprogramtestsuite.h"
#include "trainprogram.h"
#include "verticalascentrate.h"
#include <QTemporaryDir>

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

void TrainProgramTestSuite::test_gpxInclinationFilterIsDisabledAtZero() {
    EXPECT_TRUE(trainprogram::shouldSendGpxInclination(1.0, 1.0, 0.0));
}

void TrainProgramTestSuite::test_gpxInclinationFilterSuppressesSmallChanges() {
    EXPECT_FALSE(trainprogram::shouldSendGpxInclination(1.0, 1.49, 0.5));
}

void TrainProgramTestSuite::test_gpxInclinationFilterAllowsThresholdChanges() {
    EXPECT_TRUE(trainprogram::shouldSendGpxInclination(1.0, 1.5, 0.5));
}

void TrainProgramTestSuite::test_pidHrModeRoundTripsKnownValues() {
    EXPECT_EQ(trainprogram::pidHrModeFromString(QStringLiteral("default")), treadmill_pid_hr_mode::Default);
    EXPECT_EQ(trainprogram::pidHrModeFromString(QStringLiteral("speed")), treadmill_pid_hr_mode::Speed);
    EXPECT_EQ(trainprogram::pidHrModeFromString(QStringLiteral("inclination")), treadmill_pid_hr_mode::Inclination);
    EXPECT_EQ(trainprogram::pidHrModeToString(treadmill_pid_hr_mode::Inclination), QStringLiteral("inclination"));
}

void TrainProgramTestSuite::test_pidHrModeWorkoutOverrideTakesPrecedence() {
    trainrow row;
    row.pidHrMode = treadmill_pid_hr_mode::Inclination;

    EXPECT_EQ(trainprogram::effectivePidHrMode(row, treadmill_pid_hr_mode::Speed),
              treadmill_pid_hr_mode::Inclination);

    row.pidHrMode = treadmill_pid_hr_mode::Default;
    EXPECT_EQ(trainprogram::effectivePidHrMode(row, treadmill_pid_hr_mode::Inclination),
              treadmill_pid_hr_mode::Inclination);
}

void TrainProgramTestSuite::test_pidHrModeAndMaxInclinationRoundTripThroughXml() {
    QTemporaryDir temporaryDir;
    ASSERT_TRUE(temporaryDir.isValid());

    trainrow row;
    row.duration = QTime(0, 0, 10);
    row.pidHrMode = treadmill_pid_hr_mode::Inclination;
    row.maxInclination = 12.5;

    const QString filename = temporaryDir.filePath(QStringLiteral("pid-incline.xml"));
    ASSERT_TRUE(trainprogram::saveXML(filename, {row}, TREADMILL));

    const QList<trainrow> loaded = trainprogram::loadXML(filename, TREADMILL);
    ASSERT_EQ(loaded.size(), 1);
    EXPECT_EQ(loaded.first().pidHrMode, treadmill_pid_hr_mode::Inclination);
    EXPECT_DOUBLE_EQ(loaded.first().maxInclination, 12.5);
}


void TrainProgramTestSuite::test_elevationTargetProgressAndCarry() {
    EXPECT_FALSE(trainprogram::elevationTargetReached(199.9, 200.0));
    EXPECT_TRUE(trainprogram::elevationTargetReached(202.5, 200.0));
    EXPECT_FALSE(trainprogram::elevationTargetReached(0.0, -1.0));
    EXPECT_DOUBLE_EQ(trainprogram::elevationCarryOver(202.5, 200.0), 2.5);
    EXPECT_DOUBLE_EQ(trainprogram::elevationCarryOver(190.0, 200.0), 0.0);
}

void TrainProgramTestSuite::test_elevationTargetRoundTripThroughXml() {
    QTemporaryDir temporaryDir;
    ASSERT_TRUE(temporaryDir.isValid());

    trainrow climb;
    climb.elevationGain = 200.0;
    climb.speed = 5.0;
    climb.inclination = 10.0;
    trainrow recovery = timedRow(120);
    trainrow secondClimb;
    secondClimb.elevationGain = 300.0;

    const QString filename = temporaryDir.filePath(QStringLiteral("mixed-elevation.xml"));
    ASSERT_TRUE(trainprogram::saveXML(filename, {climb, recovery, secondClimb}, TREADMILL));
    const QList<trainrow> loaded = trainprogram::loadXML(filename, TREADMILL);
    ASSERT_EQ(loaded.size(), 3);
    EXPECT_DOUBLE_EQ(loaded.at(0).elevationGain, 200.0);
    EXPECT_DOUBLE_EQ(loaded.at(0).distance, -1.0);
    EXPECT_DOUBLE_EQ(loaded.at(0).speed, 5.0);
    EXPECT_EQ(loaded.at(1).duration, QTime(0, 2, 0));
    EXPECT_DOUBLE_EQ(loaded.at(1).elevationGain, -1.0);
    EXPECT_DOUBLE_EQ(loaded.at(2).elevationGain, 300.0);
}

void TrainProgramTestSuite::test_vamCalculations() {
    EXPECT_DOUBLE_EQ(instantaneousVam(5.0, 10.0, false), 500.0);
    EXPECT_DOUBLE_EQ(instantaneousVam(5.0, -5.0, false), 0.0);
    EXPECT_DOUBLE_EQ(instantaneousVam(5.0, 10.0, true), 0.0);
    EXPECT_DOUBLE_EQ(instantaneousVam(0.0, 10.0, false), 0.0);
    EXPECT_DOUBLE_EQ(averageVam(200.0, 1800.0), 400.0);
    EXPECT_DOUBLE_EQ(averageVam(200.0, 0.0), 0.0);
}
