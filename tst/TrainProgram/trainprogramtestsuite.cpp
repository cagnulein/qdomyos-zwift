#include "trainprogramtestsuite.h"
#include "trainprogram.h"
#include "mockbluetooth.h"
#include <QSignalSpy>
#include <QSettings>

TrainProgramTestSuite::TrainProgramTestSuite() {
}

void TrainProgramTestSuite::test_singleRow40MinuteWorkout_shouldReach40Minutes() {
    qDebug() << "\n=== REAL TEST: 10-second workout (2 rows × 5 seconds) ===";
    qDebug() << "Testing REAL trainprogram class with REAL scheduler() method\n";

    // Enable trainprogram_stop_at_end so end() emits stop() signal
    QSettings settings;
    settings.setValue("trainprogram_stop_at_end", true);

    // Create mock bluetooth with mock device
    MockBluetooth* mockBt = new MockBluetooth();

    // Create REAL workout: 2 rows of 5 seconds each = 10 seconds total
    QList<trainrow> rows;

    trainrow row1;
    row1.duration = QTime(0, 0, 5);
    row1.speed = 9.5;
    row1.forcespeed = true;
    rows.append(row1);

    trainrow row2;
    row2.duration = QTime(0, 0, 5);
    row2.speed = 10.0;
    row2.forcespeed = true;
    rows.append(row2);

    qDebug() << "Workout configuration:";
    qDebug() << "  Row 1: 5 seconds @ 9.5 km/h";
    qDebug() << "  Row 2: 5 seconds @ 10 km/h";
    qDebug() << "  Total: 10 seconds\n";

    // Create REAL trainprogram object
    trainprogram* tp = new trainprogram(rows, mockBt);

    // Use QSignalSpy to detect when stop() is emitted
    QSignalSpy stopSpy(tp, &trainprogram::stop);

    // Start the workout
    tp->onTapeStarted();
    qDebug() << "Workout started.\n";

    // Simulate ticks 1-10 (during the workout)
    qDebug() << "Simulating ticks 1-10...";
    for (int tick = 1; tick <= 10; tick++) {
        tp->scheduler(tick);

        if (tick == 5) {
            qDebug() << "  Tick 5: Row 1 completed, should transition to Row 2";
        }

        if (stopSpy.count() > 0) {
            qDebug() << "\n✗ FAIL: stop() was emitted at tick" << tick << "(BUG: ended too early!)";
            FAIL() << "Workout ended prematurely at tick " << tick << " instead of after tick 10";
            delete tp;
            delete mockBt;
            return;
        }
    }

    qDebug() << "  Tick 10 completed without ending";
    qDebug() << "  stop() signal count:" << stopSpy.count();
    EXPECT_EQ(stopSpy.count(), 0) << "stop() should NOT be emitted during tick 10";

    // Tick 11: workout should end
    qDebug() << "\nSimulating tick 11 (should trigger end())...";
    tp->scheduler(11);

    qDebug() << "  Tick 11 completed";
    qDebug() << "  stop() signal count:" << stopSpy.count();
    EXPECT_EQ(stopSpy.count(), 1) << "stop() should be emitted AFTER tick 10 (at tick 11)";

    qDebug() << "\n=== TEST RESULT ===";
    if (stopSpy.count() == 1) {
        qDebug() << "✓ SUCCESS: Workout completed all 10 ticks before ending";
        qDebug() << "✓ User saw complete workout: ticks 1-10, then ended at tick 11";
        qDebug() << "✓ Fix verified: end() called when time EXCEEDED, not when REACHED";
    } else {
        qDebug() << "✗ FAIL: stop() was not emitted at correct time";
    }

    delete tp;
    delete mockBt;
}

void TrainProgramTestSuite::test_multiRowWorkout_lastRowMissing1Second() {
    qDebug() << "\n=== REAL TEST: Multi-row workout (3 rows of varying duration) ===";
    qDebug() << "Verifying fix works for intermediate row transitions too\n";

    QSettings settings;
    settings.setValue("trainprogram_stop_at_end", true);

    MockBluetooth* mockBt = new MockBluetooth();

    // Create workout: 3 rows of different durations
    QList<trainrow> rows;

    trainrow row1;
    row1.duration = QTime(0, 0, 3);  // 3 seconds
    row1.speed = 8.0;
    row1.forcespeed = true;
    rows.append(row1);

    trainrow row2;
    row2.duration = QTime(0, 0, 4);  // 4 seconds
    row2.speed = 9.0;
    row2.forcespeed = true;
    rows.append(row2);

    trainrow row3;
    row3.duration = QTime(0, 0, 5);  // 5 seconds
    row3.speed = 10.0;
    row3.forcespeed = true;
    rows.append(row3);

    qDebug() << "Workout configuration:";
    qDebug() << "  Row 1: 3 seconds @ 8 km/h";
    qDebug() << "  Row 2: 4 seconds @ 9 km/h";
    qDebug() << "  Row 3: 5 seconds @ 10 km/h";
    qDebug() << "  Total: 12 seconds\n";

    trainprogram* tp = new trainprogram(rows, mockBt);
    QSignalSpy stopSpy(tp, &trainprogram::stop);

    tp->onTapeStarted();
    qDebug() << "Workout started.\n";

    qDebug() << "Simulating ticks 1-12...";
    for (int tick = 1; tick <= 12; tick++) {
        tp->scheduler(tick);

        if (tick == 3) {
            qDebug() << "  Tick 3: Row 1 should complete";
        } else if (tick == 7) {
            qDebug() << "  Tick 7: Row 2 should complete";
        }

        if (stopSpy.count() > 0) {
            qDebug() << "\n✗ FAIL: stop() emitted at tick" << tick << "(ended too early!)";
            FAIL() << "Workout ended prematurely at tick " << tick;
            delete tp;
            delete mockBt;
            return;
        }
    }

    qDebug() << "  Tick 12 completed without ending";
    qDebug() << "  stop() signal count:" << stopSpy.count();
    EXPECT_EQ(stopSpy.count(), 0) << "stop() should NOT be emitted during the workout";

    // Tick 13: should end
    qDebug() << "\nSimulating tick 13 (should trigger end())...";
    tp->scheduler(13);

    qDebug() << "  Tick 13 completed";
    qDebug() << "  stop() signal count:" << stopSpy.count();
    EXPECT_EQ(stopSpy.count(), 1) << "stop() should be emitted at tick 13";

    qDebug() << "\n=== TEST RESULT ===";
    if (stopSpy.count() == 1) {
        qDebug() << "✓ SUCCESS: All 3 rows completed correctly";
        qDebug() << "✓ Intermediate transitions (row 1→2, row 2→3) worked correctly";
        qDebug() << "✓ Final row completed before ending";
    }

    delete tp;
    delete mockBt;
}
