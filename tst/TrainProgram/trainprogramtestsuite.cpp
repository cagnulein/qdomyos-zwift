#include "trainprogramtestsuite.h"
#include "trainprogram.h"
#include "zwiftworkout.h"
#include <QTime>
#include <QList>

TrainProgramTestSuite::TrainProgramTestSuite()
{

}

void TrainProgramTestSuite::test_singleRow40MinuteWorkout_shouldReach40Minutes() {
    // Reproduce the exact bug: single row workout stops at 39:59 instead of 40:00

    qDebug() << "\n=== TEST: Single Row 40-Minute Workout ===";

    // Create a workout with 1 single row of 40 minutes (2400 seconds)
    QList<trainrow> rows;
    trainrow row;
    row.duration = QTime(0, 40, 0); // 40 minutes
    row.power = 100;
    rows.append(row);

    // Create train program with null bluetooth manager (we're only testing time logic)
    trainprogram program(rows, nullptr);

    qDebug() << "Created workout: 1 row of" << row.duration.toString("hh:mm:ss");
    qDebug() << "Expected duration:" << program.duration().toString("hh:mm:ss");
    qDebug() << "Row should complete at ticks=2400 (40:00)";
    qDebug() << "";

    // Start the program
    program.restart();

    // Simulate the workout advancing second by second
    // We'll check key moments: 2398, 2399, 2400

    // Advance to 2398 seconds (39:58)
    program.increaseElapsedTime(2398);
    qDebug() << "At ticks=2398 (39:58):";
    qDebug() << "  totalElapsedTime():" << program.totalElapsedTime().toString("hh:mm:ss");
    qDebug() << "  isStarted():" << program.isStarted();
    qDebug() << "";

    // Advance to 2399 seconds (39:59)
    program.increaseElapsedTime(1); // now at 2399
    qDebug() << "At ticks=2399 (39:59):";
    qDebug() << "  totalElapsedTime():" << program.totalElapsedTime().toString("hh:mm:ss");
    qDebug() << "  isStarted():" << program.isStarted();
    qDebug() << "";

    // CRITICAL: Advance to 2400 seconds (40:00) - the expected end
    program.increaseElapsedTime(1); // now at 2400
    qDebug() << "At ticks=2400 (40:00 - SHOULD BE THE END):";
    qDebug() << "  totalElapsedTime():" << program.totalElapsedTime().toString("hh:mm:ss");
    qDebug() << "  isStarted():" << program.isStarted();
    qDebug() << "";

    // THE BUG: The workout should have completed at ticks=2400, NOT at 2399
    QTime finalTime = program.totalElapsedTime();
    EXPECT_EQ(finalTime.hour(), 0);
    EXPECT_EQ(finalTime.minute(), 40);
    EXPECT_EQ(finalTime.second(), 0)
        << "BUG: Workout should reach 40:00, not stop at 39:59";

    qDebug() << "=== END TEST ===\n";
}

void TrainProgramTestSuite::test_multiRowWorkout_lastRowMissing1Second() {
    // Reproduce the bug: multi-row workout where ONLY the last row stops 1 second early

    qDebug() << "\n=== TEST: Multi-Row Workout (2 rows × 20 minutes) ===";

    // Create a workout with 2 rows of 20 minutes each
    QList<trainrow> rows;

    trainrow row1;
    row1.duration = QTime(0, 20, 0); // 20 minutes (1200 seconds)
    row1.power = 100;
    rows.append(row1);

    trainrow row2;
    row2.duration = QTime(0, 20, 0); // 20 minutes (1200 seconds)
    row2.power = 150;
    rows.append(row2);

    trainprogram program(rows, nullptr);

    qDebug() << "Created workout:";
    qDebug() << "  Row 1:" << row1.duration.toString("hh:mm:ss") << "@ power" << row1.power;
    qDebug() << "  Row 2:" << row2.duration.toString("hh:mm:ss") << "@ power" << row2.power;
    qDebug() << "Expected total duration:" << program.duration().toString("hh:mm:ss");
    qDebug() << "";

    program.restart();

    // Test Row 1 completion (should work correctly)
    qDebug() << "=== ROW 1 ===";

    // Advance to 1199 seconds (19:59) - 1 second before row 1 should end
    program.increaseElapsedTime(1199);
    qDebug() << "At ticks=1199 (19:59):";
    qDebug() << "  totalElapsedTime():" << program.totalElapsedTime().toString("hh:mm:ss");
    qDebug() << "  currentRow power:" << program.currentRow().power;
    qDebug() << "";

    // Advance to 1200 seconds (20:00) - row 1 should complete here
    program.increaseElapsedTime(1); // now at 1200
    qDebug() << "At ticks=1200 (20:00 - Row 1 should complete):";
    qDebug() << "  totalElapsedTime():" << program.totalElapsedTime().toString("hh:mm:ss");
    qDebug() << "  currentRow power:" << program.currentRow().power;

    // Row 1 should have completed and moved to Row 2
    EXPECT_EQ(program.currentRow().power, 150)
        << "Row 1 should complete at 20:00 and move to Row 2";
    qDebug() << "  ✓ Row 1 completed correctly at 20:00";
    qDebug() << "";

    // Test Row 2 completion (BUG: should stop 1 second early)
    qDebug() << "=== ROW 2 ===";

    // Advance to 2399 seconds (39:59) - 1 second before row 2 should end
    program.increaseElapsedTime(1199); // now at 2399 total (19:59 into row 2)
    qDebug() << "At ticks=2399 (39:59 total / 19:59 into Row 2):";
    qDebug() << "  totalElapsedTime():" << program.totalElapsedTime().toString("hh:mm:ss");
    qDebug() << "  isStarted():" << program.isStarted();
    qDebug() << "";

    // CRITICAL: Advance to 2400 seconds (40:00) - row 2 should complete here
    program.increaseElapsedTime(1); // now at 2400
    qDebug() << "At ticks=2400 (40:00 total / 20:00 into Row 2 - SHOULD BE THE END):";
    qDebug() << "  totalElapsedTime():" << program.totalElapsedTime().toString("hh:mm:ss");
    qDebug() << "  isStarted():" << program.isStarted();
    qDebug() << "";

    // THE BUG: Row 2 (the LAST row) should complete at 40:00, not 39:59
    QTime finalTime = program.totalElapsedTime();
    EXPECT_EQ(finalTime.hour(), 0);
    EXPECT_EQ(finalTime.minute(), 40);
    EXPECT_EQ(finalTime.second(), 0)
        << "BUG: Last row should reach 40:00 (20:00 into row 2), not stop at 39:59";

    qDebug() << "=== END TEST ===\n";
}
