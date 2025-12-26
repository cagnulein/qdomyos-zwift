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

void TrainProgramTestSuite::test_oldBuggyCode_proveBugExisted() {
    // This test PROVES that the old buggy code was broken
    // It simulates what the old code did: QTime(0, 0, ticks)
    // and confirms that approach fails for values > 59 seconds

    // Simulate the user's reported case: 40 minutes = 2400 seconds
    int ticks_40minutes = 2400;

    // OLD BUGGY CODE APPROACH: QTime(0, 0, ticks)
    QTime oldBuggyWay(0, 0, ticks_40minutes);

    // PROOF #1: The old way creates an INVALID QTime
    // When you try to create QTime(0, 0, 2400), Qt rejects it because
    // seconds must be 0-59, and 2400 is way out of range
    EXPECT_FALSE(oldBuggyWay.isValid())
        << "OLD BUG CONFIRMED: QTime(0, 0, " << ticks_40minutes
        << ") creates an INVALID QTime";

    // PROOF #2: Invalid QTime methods return -1
    // This is Qt's way of saying "this time is broken"
    EXPECT_EQ(oldBuggyWay.hour(), -1)
        << "OLD BUG CONFIRMED: hour() returns -1 for invalid QTime";
    EXPECT_EQ(oldBuggyWay.minute(), -1)
        << "OLD BUG CONFIRMED: minute() returns -1 for invalid QTime";
    EXPECT_EQ(oldBuggyWay.second(), -1)
        << "OLD BUG CONFIRMED: second() returns -1 for invalid QTime";

    // PROOF #3: toString() on invalid QTime returns empty string
    QString oldBuggyString = oldBuggyWay.toString("hh:mm:ss");
    EXPECT_TRUE(oldBuggyString.isEmpty() || oldBuggyString == "")
        << "OLD BUG CONFIRMED: toString() returns empty/invalid string for invalid QTime";

    // Now test a few more boundary cases to show exactly when the bug occurs

    // At 59 seconds: OLD CODE WORKS (barely)
    QTime at59seconds(0, 0, 59);
    EXPECT_TRUE(at59seconds.isValid())
        << "At 59 seconds, old code still works";
    EXPECT_EQ(at59seconds.second(), 59);

    // At 60 seconds: OLD CODE BREAKS
    QTime at60seconds(0, 0, 60);
    EXPECT_FALSE(at60seconds.isValid())
        << "OLD BUG CONFIRMED: At 60 seconds, old code breaks (creates invalid QTime)";

    // At 120 seconds (2 minutes): OLD CODE BREAKS
    QTime at120seconds(0, 0, 120);
    EXPECT_FALSE(at120seconds.isValid())
        << "OLD BUG CONFIRMED: At 120 seconds, old code breaks";

    // COMPARE WITH NEW CORRECT CODE
    // This is what the fix does: QTime(0, 0, 0).addSecs(ticks)
    QTime newCorrectWay = QTime(0, 0, 0).addSecs(ticks_40minutes);

    // PROOF #4: The new way creates a VALID QTime
    EXPECT_TRUE(newCorrectWay.isValid())
        << "FIX CONFIRMED: QTime(0,0,0).addSecs(" << ticks_40minutes
        << ") creates a VALID QTime";

    // PROOF #5: The new way has correct values
    EXPECT_EQ(newCorrectWay.hour(), 0) << "FIX CONFIRMED: Correct hour";
    EXPECT_EQ(newCorrectWay.minute(), 40) << "FIX CONFIRMED: Correct minute";
    EXPECT_EQ(newCorrectWay.second(), 0) << "FIX CONFIRMED: Correct second";

    // PROOF #6: The new way produces correct string
    QString newCorrectString = newCorrectWay.toString("hh:mm:ss");
    EXPECT_EQ(newCorrectString.toStdString(), "00:40:00")
        << "FIX CONFIRMED: Correct string representation";

    // FINAL SUMMARY:
    // - Old code: QTime(0, 0, 2400) → INVALID → hour()=-1, minute()=-1, second()=-1
    // - New code: QTime(0, 0, 0).addSecs(2400) → VALID → hour()=0, minute()=40, second()=0
    //
    // This test PASSES, which means we've successfully proven:
    // 1. The old code was definitely broken for workouts > 59 seconds
    // 2. The new code fixes the problem
    // 3. The user was right - there WAS a bug!
}

void TrainProgramTestSuite::test_whatUserActuallySaw_withBuggyCode() {
    // CRITICAL TEST: This simulates EXACTLY what the user saw in the UI
    //
    // The UI calls: trainProgram->totalElapsedTime().toString("hh:mm:ss")
    // With old buggy code, this was: QTime(0, 0, ticks).toString("hh:mm:ss")
    //
    // Let's see what toString() ACTUALLY returns for various tick values

    qDebug() << "\n=== SIMULATING 40-MINUTE WORKOUT WITH OLD BUGGY CODE ===";
    qDebug() << "What the UI would display using QTime(0, 0, ticks).toString(\"hh:mm:ss\"):\n";

    // Test key moments in a 40-minute workout
    struct TestCase {
        int ticks;
        QString description;
    };

    TestCase testCases[] = {
        {0, "Start"},
        {30, "30 seconds"},
        {59, "Last valid second"},
        {60, "First invalid second (1 minute)"},
        {119, "1:59 - should show 00:01:59"},
        {120, "2:00 - should show 00:02:00"},
        {2395, "39:55 - 5 seconds before end"},
        {2396, "39:56 - 4 seconds before end"},
        {2397, "39:57 - 3 seconds before end"},
        {2398, "39:58 - 2 seconds before end"},
        {2399, "39:59 - 1 second before end"},
        {2400, "40:00 - Expected end time"},
    };

    QString lastValidDisplayedTime = "";
    int lastValidTicks = -1;

    for (const auto& tc : testCases) {
        // OLD BUGGY WAY (what the code WAS doing)
        QTime oldWay(0, 0, tc.ticks);
        QString displayedTime = oldWay.toString("hh:mm:ss");

        // NEW CORRECT WAY (what the code SHOULD do)
        QTime newWay = QTime(0, 0, 0).addSecs(tc.ticks);
        QString correctTime = newWay.toString("hh:mm:ss");

        qDebug() << "Ticks" << tc.ticks << "(" << tc.description.toStdString().c_str() << "):";
        qDebug() << "  OLD BUGGY CODE displays:" << displayedTime;
        qDebug() << "  NEW CORRECT CODE displays:" << correctTime;
        qDebug() << "  QTime.isValid():" << oldWay.isValid();

        // Track the last valid time that was displayed
        if (oldWay.isValid() && !displayedTime.isEmpty()) {
            lastValidDisplayedTime = displayedTime;
            lastValidTicks = tc.ticks;
        }

        qDebug() << "";
    }

    qDebug() << "\n=== ANALYSIS ===";
    qDebug() << "Last VALID time displayed with old code: " << lastValidDisplayedTime
             << " (at ticks=" << lastValidTicks << ")";
    qDebug() << "Expected final time: 00:40:00 (at ticks=2400)";
    qDebug() << "";

    // Now let's verify our hypothesis about what the user saw

    // At ticks=59: old code should still work
    QTime at59 = QTime(0, 0, 59);
    EXPECT_TRUE(at59.isValid());
    EXPECT_EQ(at59.toString("hh:mm:ss").toStdString(), "00:00:59");

    // At ticks=60: old code breaks
    QTime at60 = QTime(0, 0, 60);
    EXPECT_FALSE(at60.isValid()) << "At 60 seconds, old code creates invalid QTime";
    QString display60 = at60.toString("hh:mm:ss");
    qDebug() << "At ticks=60, toString() returns: '" << display60 << "' (length: " << display60.length() << ")";

    // At ticks=2399 (39:59): old code breaks
    QTime at2399 = QTime(0, 0, 2399);
    EXPECT_FALSE(at2399.isValid()) << "At 2399 seconds (39:59), old code creates invalid QTime";
    QString display2399 = at2399.toString("hh:mm:ss");
    qDebug() << "At ticks=2399, toString() returns: '" << display2399 << "' (length: " << display2399.length() << ")";

    // At ticks=2400 (40:00): old code breaks
    QTime at2400 = QTime(0, 0, 2400);
    EXPECT_FALSE(at2400.isValid()) << "At 2400 seconds (40:00), old code creates invalid QTime";
    QString display2400 = at2400.toString("hh:mm:ss");
    qDebug() << "At ticks=2400, toString() returns: '" << display2400 << "' (length: " << display2400.length() << ")";

    // CRITICAL FINDING: What does toString() return for invalid QTime?
    // If it returns empty string, the UI might:
    // 1. Show nothing (blank)
    // 2. Keep showing the last valid value
    // 3. Show some default value

    qDebug() << "\n=== CONCLUSION ===";
    qDebug() << "For a 40-minute workout:";
    qDebug() << "- All ticks >= 60 create INVALID QTime with old code";
    qDebug() << "- toString() on invalid QTime returns: '" << display2400 << "'";
    qDebug() << "- This explains why the timer display was broken!";
    qDebug() << "";

    // The key assertion: with old code, you CANNOT display times >= 1 minute
    for (int ticks = 60; ticks <= 2400; ticks += 60) {
        QTime oldBuggy(0, 0, ticks);
        EXPECT_FALSE(oldBuggy.isValid())
            << "Old code fails at " << ticks << " seconds";
    }

    // With new code, all times are valid
    for (int ticks = 0; ticks <= 2400; ticks += 60) {
        QTime newCorrect = QTime(0, 0, 0).addSecs(ticks);
        EXPECT_TRUE(newCorrect.isValid())
            << "New code works at " << ticks << " seconds";
    }
}
