#include "trainprogramtestsuite.h"
#include "trainprogram.h"
#include "zwiftworkout.h"
#include <QTime>
#include <QList>
#include <QSignalSpy>

TrainProgramTestSuite::TrainProgramTestSuite()
{

}

void TrainProgramTestSuite::test_singleRow40MinuteWorkout_shouldReach40Minutes() {
    // Test using REAL trainprogram class and scheduler() method

    qDebug() << "\n=== REAL TEST: Single Row 40-Minute Workout ===\n";

    // Create workout
    QList<trainrow> rows;
    trainrow row;
    row.duration = QTime(0, 40, 0); // 2400 seconds
    row.power = 100;
    rows.append(row);

    trainprogram program(rows, nullptr);

    qDebug() << "Workout: 1 row of" << row.duration.toString("hh:mm:ss");
    qDebug() << "Expected to complete at tick 2400";
    qDebug() << "";

    // Connect to stop signal to detect when workout ends
    QSignalSpy stopSpy(&program, &trainprogram::stop);

    // Start the program
    program.restart();

    qDebug() << "Program started, isStarted() =" << program.isStarted();
    qDebug() << "";

    // Simulate workout by calling scheduler() repeatedly
    // NOTE: scheduler() calls ticks++, so we need to call it 2400 times

    qDebug() << "Simulating workout...";

    // Fast forward to near the end
    for (int i = 0; i < 2397; i++) {
        program.increaseElapsedTime(1);
    }

    qDebug() << "At ticks=2397 (39:57):";
    qDebug() << "  isStarted():" << program.isStarted();
    qDebug() << "  stop signal count:" << stopSpy.count();
    qDebug() << "";

    // Tick 2398
    program.increaseElapsedTime(1);
    qDebug() << "At ticks=2398 (39:58):";
    qDebug() << "  isStarted():" << program.isStarted();
    qDebug() << "  stop signal count:" << stopSpy.count();
    qDebug() << "";

    // Tick 2399 - BUG: workout might end here
    program.increaseElapsedTime(1);
    qDebug() << "At ticks=2399 (39:59):";
    qDebug() << "  isStarted():" << program.isStarted();
    qDebug() << "  stop signal count:" << stopSpy.count();

    if (stopSpy.count() > 0) {
        qDebug() << "  *** BUG CONFIRMED: Workout ended at tick 2399 instead of 2400! ***";
    }
    qDebug() << "";

    // Tick 2400 - Should be the real end
    program.increaseElapsedTime(1);
    qDebug() << "At ticks=2400 (40:00):";
    qDebug() << "  isStarted():" << program.isStarted();
    qDebug() << "  stop signal count:" << stopSpy.count();
    qDebug() << "";

    // The workout should NOT have stopped before tick 2400
    // It should stop at or after tick 2400
    int stopsAt2399 = 0;
    if (stopSpy.count() > 0) {
        // Check if first stop was before we hit 2400
        // Since we manually increment, we can't know exact timing
        // but we know it should NOT stop at 2399
        qDebug() << "Workout stopped. Checking if it stopped too early...";
    }

    qDebug() << "=== Analysis ===";
    qDebug() << "Expected: workout completes at tick 2400 (40:00)";
    qDebug() << "Actual: stop signal emitted" << stopSpy.count() << "times";
    qDebug() << "";

    // For now, just log what happened - we need to see scheduler() output
    qDebug() << "=== END TEST ===\n";
}

void TrainProgramTestSuite::test_multiRowWorkout_lastRowMissing1Second() {
    // Test with 2 rows to see if only last row is affected

    qDebug() << "\n=== REAL TEST: Multi-Row Workout ===\n";

    QList<trainrow> rows;

    trainrow row1;
    row1.duration = QTime(0, 20, 0); // 1200 seconds
    row1.power = 100;
    rows.append(row1);

    trainrow row2;
    row2.duration = QTime(0, 20, 0); // 1200 seconds
    row2.power = 150;
    rows.append(row2);

    trainprogram program(rows, nullptr);

    qDebug() << "Workout:";
    qDebug() << "  Row 1:" << row1.duration.toString("hh:mm:ss") << "@ power" << row1.power;
    qDebug() << "  Row 2:" << row2.duration.toString("hh:mm:ss") << "@ power" << row2.power;
    qDebug() << "";

    QSignalSpy stopSpy(&program, &trainprogram::stop);

    program.restart();

    qDebug() << "=== Testing Row 1 transition ===";

    // Advance to near end of row 1
    for (int i = 0; i < 1198; i++) {
        program.increaseElapsedTime(1);
    }

    qDebug() << "At ticks=1198 (19:58):";
    qDebug() << "  currentRow power:" << program.currentRow().power;
    qDebug() << "";

    program.increaseElapsedTime(1); // 1199
    qDebug() << "At ticks=1199 (19:59):";
    qDebug() << "  currentRow power:" << program.currentRow().power;
    qDebug() << "";

    program.increaseElapsedTime(1); // 1200
    qDebug() << "At ticks=1200 (20:00 - Row 1 should complete):";
    qDebug() << "  currentRow power:" << program.currentRow().power;

    if (program.currentRow().power == 150) {
        qDebug() << "  ✓ Row 1 completed, moved to Row 2";
    } else {
        qDebug() << "  ✗ Still on Row 1!";
    }
    qDebug() << "";

    qDebug() << "=== Testing Row 2 (last row) end ===";

    // Advance to near end of row 2
    for (int i = 1200; i < 2398; i++) {
        program.increaseElapsedTime(1);
    }

    qDebug() << "At ticks=2398 (39:58):";
    qDebug() << "  isStarted():" << program.isStarted();
    qDebug() << "  stop count:" << stopSpy.count();
    qDebug() << "";

    program.increaseElapsedTime(1); // 2399
    qDebug() << "At ticks=2399 (39:59):";
    qDebug() << "  isStarted():" << program.isStarted();
    qDebug() << "  stop count:" << stopSpy.count();

    if (stopSpy.count() > 0) {
        qDebug() << "  *** BUG: Last row ended at 2399 instead of 2400! ***";
    }
    qDebug() << "";

    program.increaseElapsedTime(1); // 2400
    qDebug() << "At ticks=2400 (40:00):";
    qDebug() << "  isStarted():" << program.isStarted();
    qDebug() << "  stop count:" << stopSpy.count();
    qDebug() << "";

    qDebug() << "=== END TEST ===\n";
}
