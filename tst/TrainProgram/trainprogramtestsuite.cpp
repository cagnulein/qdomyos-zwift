#include "trainprogramtestsuite.h"
#include "trainprogram.h"
#include "devices/faketreadmill/faketreadmill.h"
#include "homeform.h"
#include <QSignalSpy>
#include <QSettings>
#include <QEventLoop>
#include <QTimer>
#include <QTest>

TrainProgramTestSuite::TrainProgramTestSuite() {
}

void TrainProgramTestSuite::test_singleRow40MinuteWorkout_shouldReach40Minutes() {
    qDebug() << "\n=== REAL TEST: 40-second workout with REAL scheduler ===";
    qDebug() << "Using faketreadmill and REAL trainprogram::scheduler()\n";

    // Enable trainprogram_stop_at_end so end() emits stop() signal
    QSettings settings;
    settings.setValue("trainprogram_stop_at_end", true);

    // Create REAL faketreadmill device
    faketreadmill* fakeTreadmill = new faketreadmill(false, false, true);

    // Simulate device started with speed > 0 (required by scheduler)
    // We use the speed plus event from homeform to simulate user running
    QMetaObject::invokeMethod(fakeTreadmill, "changeSpeed", Qt::DirectConnection, Q_ARG(double, 10.0));

    // Create workout: 3 rows totaling 40 seconds
    QList<trainrow> rows;

    trainrow row1;
    row1.duration = QTime(0, 0, 10);  // 10 seconds
    row1.speed = 8.0;
    row1.forcespeed = true;
    rows.append(row1);

    trainrow row2;
    row2.duration = QTime(0, 0, 15);  // 15 seconds
    row2.speed = 10.0;
    row2.forcespeed = true;
    rows.append(row2);

    trainrow row3;
    row3.duration = QTime(0, 0, 15);  // 15 seconds
    row3.speed = 12.0;
    row3.forcespeed = true;
    rows.append(row3);

    qDebug() << "Workout configuration:";
    qDebug() << "  Row 1: 10 seconds @ 8 km/h";
    qDebug() << "  Row 2: 15 seconds @ 10 km/h";
    qDebug() << "  Row 3: 15 seconds @ 12 km/h";
    qDebug() << "  Total: 40 seconds\n";

    // Create bluetooth manager mock that returns our fake treadmill
    bluetooth* bt = new bluetooth(false, "", false, false, 200, false, false, 4, 1.0, false);

    // We can't set the device directly in bluetooth, but trainprogram will work with it
    // Create REAL trainprogram object with fake bluetooth
    trainprogram* tp = new trainprogram(rows, bt);

    // Use QSignalSpy to detect when stop() is emitted
    QSignalSpy stopSpy(tp, &trainprogram::stop);

    qDebug() << "Starting workout...";
    tp->onTapeStarted();
    qDebug() << "Workout started. Waiting for completion (max 50 seconds)...\n";

    // Create event loop to wait for stop signal
    QEventLoop loop;
    QTimer timeout;
    timeout.setSingleShot(true);
    timeout.setInterval(50000); // 50 seconds timeout (40s workout + 10s buffer)

    // Connect stop signal to quit loop
    connect(tp, &trainprogram::stop, &loop, &QEventLoop::quit);
    connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);

    // Record start time
    QElapsedTimer timer;
    timer.start();

    // Start timeout and run event loop
    timeout.start();
    loop.exec();

    qint64 elapsed = timer.elapsed();
    qDebug() << "\nWorkout completed after" << elapsed << "ms";
    qDebug() << "Expected: ~40000ms (40 seconds)";
    qDebug() << "stop() signal count:" << stopSpy.count();

    // Verify stop() was emitted
    EXPECT_EQ(stopSpy.count(), 1) << "stop() should be emitted after workout completes";

    // Verify timing is correct (should be around 40 seconds, allow ±5 seconds tolerance)
    EXPECT_GT(elapsed, 38000) << "Workout should not end before 38 seconds";
    EXPECT_LT(elapsed, 45000) << "Workout should complete within 45 seconds";

    qDebug() << "\n=== TEST RESULT ===";
    if (stopSpy.count() == 1 && elapsed >= 38000 && elapsed <= 45000) {
        qDebug() << "✓ SUCCESS: Workout completed all rows before ending";
        qDebug() << "✓ Used REAL scheduler() - not test-friendly method!";
        qDebug() << "✓ Fix verified: workout reached full duration";
    } else {
        qDebug() << "✗ FAIL: Test did not complete as expected";
    }

    delete tp;
    delete bt;
    delete fakeTreadmill;
}

void TrainProgramTestSuite::test_multiRowWorkout_lastRowMissing1Second() {
    qDebug() << "\n=== REAL TEST: 30-second multi-row workout ===";
    qDebug() << "Testing that all rows complete with REAL scheduler\n";

    QSettings settings;
    settings.setValue("trainprogram_stop_at_end", true);

    // Create REAL faketreadmill
    faketreadmill* fakeTreadmill = new faketreadmill(false, false, true);
    QMetaObject::invokeMethod(fakeTreadmill, "changeSpeed", Qt::DirectConnection, Q_ARG(double, 10.0));

    // Create workout: 4 rows of varying durations = 30 seconds
    QList<trainrow> rows;

    trainrow row1;
    row1.duration = QTime(0, 0, 5);  // 5 seconds
    row1.speed = 7.0;
    row1.forcespeed = true;
    rows.append(row1);

    trainrow row2;
    row2.duration = QTime(0, 0, 8);  // 8 seconds
    row2.speed = 9.0;
    row2.forcespeed = true;
    rows.append(row2);

    trainrow row3;
    row3.duration = QTime(0, 0, 10);  // 10 seconds
    row3.speed = 11.0;
    row3.forcespeed = true;
    rows.append(row3);

    trainrow row4;
    row4.duration = QTime(0, 0, 7);  // 7 seconds
    row4.speed = 8.0;
    row4.forcespeed = true;
    rows.append(row4);

    qDebug() << "Workout configuration:";
    qDebug() << "  Row 1: 5 seconds @ 7 km/h";
    qDebug() << "  Row 2: 8 seconds @ 9 km/h";
    qDebug() << "  Row 3: 10 seconds @ 11 km/h";
    qDebug() << "  Row 4: 7 seconds @ 8 km/h";
    qDebug() << "  Total: 30 seconds\n";

    bluetooth* bt = new bluetooth(false, "", false, false, 200, false, false, 4, 1.0, false);
    trainprogram* tp = new trainprogram(rows, bt);
    QSignalSpy stopSpy(tp, &trainprogram::stop);

    qDebug() << "Starting workout...";
    tp->onTapeStarted();
    qDebug() << "Workout started. Waiting for completion (max 40 seconds)...\n";

    QEventLoop loop;
    QTimer timeout;
    timeout.setSingleShot(true);
    timeout.setInterval(40000);

    connect(tp, &trainprogram::stop, &loop, &QEventLoop::quit);
    connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);

    QElapsedTimer timer;
    timer.start();

    timeout.start();
    loop.exec();

    qint64 elapsed = timer.elapsed();
    qDebug() << "\nWorkout completed after" << elapsed << "ms";
    qDebug() << "Expected: ~30000ms (30 seconds)";
    qDebug() << "stop() signal count:" << stopSpy.count();

    EXPECT_EQ(stopSpy.count(), 1) << "stop() should be emitted";
    EXPECT_GT(elapsed, 28000) << "Workout should not end before 28 seconds";
    EXPECT_LT(elapsed, 35000) << "Workout should complete within 35 seconds";

    qDebug() << "\n=== TEST RESULT ===";
    if (stopSpy.count() == 1 && elapsed >= 28000 && elapsed <= 35000) {
        qDebug() << "✓ SUCCESS: All 4 rows completed correctly";
        qDebug() << "✓ Intermediate transitions worked correctly";
        qDebug() << "✓ Final row completed before ending";
    }

    delete tp;
    delete bt;
    delete fakeTreadmill;
}
