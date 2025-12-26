#include "trainprogramtestsuite.h"
#include "trainprogram.h"
#include "zwiftworkout.h"
#include <QTime>
#include <QList>

TrainProgramTestSuite::TrainProgramTestSuite()
{

}

void TrainProgramTestSuite::test_totalElapsedTime_shortWorkout() {
    // Create a simple workout with one 30-second interval
    QList<trainrow> rows;
    trainrow row;
    row.duration = QTime(0, 0, 30); // 30 seconds
    row.power = 100;
    rows.append(row);

    // Create train program with null bluetooth manager (we're only testing time calculation)
    trainprogram program(rows, nullptr);

    // Start the program
    program.restart();

    // Simulate 30 seconds elapsed
    program.increaseElapsedTime(30);

    QTime elapsed = program.totalElapsedTime();

    // Should be 00:00:30
    EXPECT_EQ(elapsed.hour(), 0);
    EXPECT_EQ(elapsed.minute(), 0);
    EXPECT_EQ(elapsed.second(), 30);
}

void TrainProgramTestSuite::test_totalElapsedTime_exactly60Seconds() {
    // Create a simple workout with one 60-second interval
    QList<trainrow> rows;
    trainrow row;
    row.duration = QTime(0, 1, 0); // 60 seconds (1 minute)
    row.power = 100;
    rows.append(row);

    trainprogram program(rows, nullptr);
    program.restart();

    // Simulate 60 seconds elapsed
    program.increaseElapsedTime(60);

    QTime elapsed = program.totalElapsedTime();

    // Should be 00:01:00
    EXPECT_EQ(elapsed.hour(), 0);
    EXPECT_EQ(elapsed.minute(), 1);
    EXPECT_EQ(elapsed.second(), 0);
}

void TrainProgramTestSuite::test_totalElapsedTime_longWorkout() {
    // Create a workout with 2 minutes duration
    QList<trainrow> rows;
    trainrow row;
    row.duration = QTime(0, 2, 0); // 120 seconds
    row.power = 100;
    rows.append(row);

    trainprogram program(rows, nullptr);
    program.restart();

    // Simulate 120 seconds (2 minutes) elapsed
    program.increaseElapsedTime(120);

    QTime elapsed = program.totalElapsedTime();

    // Should be 00:02:00
    EXPECT_EQ(elapsed.hour(), 0);
    EXPECT_EQ(elapsed.minute(), 2);
    EXPECT_EQ(elapsed.second(), 0);

    // Also test the string representation
    QString timeString = elapsed.toString("hh:mm:ss");
    EXPECT_EQ(timeString.toStdString(), "00:02:00");
}

void TrainProgramTestSuite::test_totalElapsedTime_40MinuteWorkout() {
    // This tests the specific bug reported by the user
    // A 40-minute workout that was stopping at 00:39:59 instead of 00:40:00

    QList<trainrow> rows;
    trainrow row;
    row.duration = QTime(0, 40, 0); // 40 minutes
    row.power = 100;
    rows.append(row);

    trainprogram program(rows, nullptr);
    program.restart();

    // Simulate 2400 seconds (40 minutes) elapsed
    program.increaseElapsedTime(2400);

    QTime elapsed = program.totalElapsedTime();

    // Should be 00:40:00
    EXPECT_EQ(elapsed.hour(), 0);
    EXPECT_EQ(elapsed.minute(), 40);
    EXPECT_EQ(elapsed.second(), 0);

    // Verify the string representation is correct
    QString timeString = elapsed.toString("hh:mm:ss");
    EXPECT_EQ(timeString.toStdString(), "00:40:00");

    // The old buggy code would create QTime(0, 0, 2400) which is invalid
    // Let's verify that our fix creates a valid QTime
    EXPECT_TRUE(elapsed.isValid());
}

void TrainProgramTestSuite::test_totalElapsedTime_multiHourWorkout() {
    // Test a longer workout (2 hours, 30 minutes, 45 seconds)
    QList<trainrow> rows;
    trainrow row;
    row.duration = QTime(2, 30, 45);
    row.power = 100;
    rows.append(row);

    trainprogram program(rows, nullptr);
    program.restart();

    // Simulate 9045 seconds (2h 30m 45s) elapsed
    program.increaseElapsedTime(9045);

    QTime elapsed = program.totalElapsedTime();

    // Should be 02:30:45
    EXPECT_EQ(elapsed.hour(), 2);
    EXPECT_EQ(elapsed.minute(), 30);
    EXPECT_EQ(elapsed.second(), 45);

    // Verify string representation
    QString timeString = elapsed.toString("hh:mm:ss");
    EXPECT_EQ(timeString.toStdString(), "02:30:45");

    // Verify the QTime is valid
    EXPECT_TRUE(elapsed.isValid());
}
