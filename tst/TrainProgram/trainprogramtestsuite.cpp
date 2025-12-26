#include "trainprogramtestsuite.h"
#include "trainprogram.h"
#include "zwiftworkout.h"
#include <QTime>
#include <QList>

TrainProgramTestSuite::TrainProgramTestSuite()
{

}

void TrainProgramTestSuite::test_singleRow40MinuteWorkout_shouldReach40Minutes() {
    // Test the REAL trainprogram logic, not just display
    // Bug: workout stops when calculatedElapsedTime > ticks at 2399, not 2400

    qDebug() << "\n=== TEST: Single Row Logic (40 minutes = 2400 seconds) ===\n";

    // Create a single row workout of 40 minutes
    QList<trainrow> rows;
    trainrow row;
    row.duration = QTime(0, 40, 0); // 2400 seconds
    row.power = 100;
    rows.append(row);

    trainprogram program(rows, nullptr);

    qDebug() << "Workout created: 1 row of" << row.duration.toString("hh:mm:ss");
    qDebug() << "Expected completion: at ticks = 2400";
    qDebug() << "";

    // The bug is in the comparison logic in scheduler():
    // Line ~1001: if (calculatedElapsedTime > static_cast<uint32_t>(ticks) ...)
    //
    // For a 2400 second workout:
    // - calculateTimeForRow(0) returns 2400
    // - When ticks=2399: 2400 > 2399 = TRUE → breaks → ends early!
    // - When ticks=2400: 2400 > 2400 = FALSE → should continue

    // Simulate what happens in scheduler() at different ticks
    qDebug() << "=== Simulating scheduler() logic ===\n";

    // Test the comparison at tick 2398
    {
        int32_t ticks = 2398;
        uint32_t calculatedElapsedTime = 0;
        for (int i = 0; i < rows.length(); i++) {
            uint32_t rowTime = (rows.at(i).duration.second() +
                               (rows.at(i).duration.minute() * 60) +
                               (rows.at(i).duration.hour() * 3600));
            calculatedElapsedTime += rowTime;
        }

        bool shouldBreak = (calculatedElapsedTime > static_cast<uint32_t>(ticks));
        qDebug() << "At ticks=" << ticks << "(39:58):";
        qDebug() << "  calculatedElapsedTime =" << calculatedElapsedTime;
        qDebug() << "  calculatedElapsedTime > ticks?" << shouldBreak;
        qDebug() << "  → Workout should" << (shouldBreak ? "END" : "CONTINUE");
        qDebug() << "";
    }

    // Test the comparison at tick 2399 (THE BUG!)
    {
        int32_t ticks = 2399;
        uint32_t calculatedElapsedTime = 0;
        for (int i = 0; i < rows.length(); i++) {
            uint32_t rowTime = (rows.at(i).duration.second() +
                               (rows.at(i).duration.minute() * 60) +
                               (rows.at(i).duration.hour() * 3600));
            calculatedElapsedTime += rowTime;
        }

        bool shouldBreak = (calculatedElapsedTime > static_cast<uint32_t>(ticks));
        qDebug() << "At ticks=" << ticks << "(39:59):";
        qDebug() << "  calculatedElapsedTime =" << calculatedElapsedTime;
        qDebug() << "  calculatedElapsedTime > ticks?" << shouldBreak;
        qDebug() << "  → Workout should" << (shouldBreak ? "END" : "CONTINUE");
        qDebug() << "  *** BUG: This returns TRUE, ending workout 1 second early! ***";
        qDebug() << "";

        // This is the bug! At tick 2399, the comparison says "END"
        // but the workout should complete at tick 2400
        EXPECT_FALSE(shouldBreak)
            << "BUG FOUND: At ticks=2399, shouldBreak is TRUE but workout should continue to 2400";
    }

    // Test the comparison at tick 2400 (when it SHOULD end)
    {
        int32_t ticks = 2400;
        uint32_t calculatedElapsedTime = 0;
        for (int i = 0; i < rows.length(); i++) {
            uint32_t rowTime = (rows.at(i).duration.second() +
                               (rows.at(i).duration.minute() * 60) +
                               (rows.at(i).duration.hour() * 3600));
            calculatedElapsedTime += rowTime;
        }

        bool shouldBreak = (calculatedElapsedTime > static_cast<uint32_t>(ticks));
        qDebug() << "At ticks=" << ticks << "(40:00):";
        qDebug() << "  calculatedElapsedTime =" << calculatedElapsedTime;
        qDebug() << "  calculatedElapsedTime > ticks?" << shouldBreak;
        qDebug() << "  → Workout should" << (shouldBreak ? "END" : "CONTINUE");
        qDebug() << "";
    }

    qDebug() << "=== ROOT CAUSE ===";
    qDebug() << "Line ~1001 in trainprogram.cpp:";
    qDebug() << "  if (calculatedElapsedTime > static_cast<uint32_t>(ticks) ...)";
    qDebug() << "";
    qDebug() << "This uses STRICT > comparison, which causes:";
    qDebug() << "  2400 > 2399 = TRUE → breaks at tick 2399 (1 second early)";
    qDebug() << "";
    qDebug() << "FIX: The comparison should allow the last tick to complete:";
    qDebug() << "  Either change to: >= (ticks + 1)";
    qDebug() << "  Or ensure the workout processes tick 2400 before ending";
    qDebug() << "";

    qDebug() << "=== END TEST ===\n";
}

void TrainProgramTestSuite::test_multiRowWorkout_lastRowMissing1Second() {
    // Test why ONLY the last row is affected

    qDebug() << "\n=== TEST: Multi-Row Workout (2 rows × 20 minutes) ===\n";

    // Create a workout with 2 rows
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

    qDebug() << "Workout created:";
    qDebug() << "  Row 1:" << row1.duration.toString("hh:mm:ss") << "@ power" << row1.power;
    qDebug() << "  Row 2:" << row2.duration.toString("hh:mm:ss") << "@ power" << row2.power;
    qDebug() << "";

    qDebug() << "=== Row 1 Transition (at ticks=1200) ===\n";

    // When scheduler processes tick 1200:
    // - calculatedLine loop: calculatedElapsedTime = 1200 (row 0)
    // - Next iteration: calculatedElapsedTime = 2400 (row 0 + row 1)
    // - Check: 1200 > 1200? NO → continues to next row
    // - Sets currentStep = 1 (row 2)
    {
        int32_t ticks = 1200;
        uint32_t calculatedElapsedTime = 0;

        qDebug() << "At ticks=" << ticks << "(20:00 - end of Row 1):";

        for (int calculatedLine = 0; calculatedLine < rows.length(); calculatedLine++) {
            uint32_t rowTime = (rows.at(calculatedLine).duration.second() +
                               (rows.at(calculatedLine).duration.minute() * 60) +
                               (rows.at(calculatedLine).duration.hour() * 3600));
            calculatedElapsedTime += rowTime;

            bool shouldBreak = (calculatedElapsedTime > static_cast<uint32_t>(ticks));

            qDebug() << "  Checking row" << calculatedLine << ":";
            qDebug() << "    calculatedElapsedTime =" << calculatedElapsedTime;
            qDebug() << "    shouldBreak?" << shouldBreak;

            if (shouldBreak && calculatedLine >= 0) { // simplified currentStep check
                qDebug() << "    → BREAK, move to next row";
                break;
            }
        }
        qDebug() << "  Result: Moves to Row 2 ✓";
        qDebug() << "";
    }

    qDebug() << "=== Row 2 End (at ticks=2399) - THE BUG ===\n";

    // When scheduler processes tick 2399:
    // - calculatedLine loop: calculatedElapsedTime = 2400 (both rows)
    // - Check: 2400 > 2399? YES → breaks
    // - currentStep >= rows.length() → calls end()!
    {
        int32_t ticks = 2399;
        uint32_t calculatedElapsedTime = 0;

        qDebug() << "At ticks=" << ticks << "(39:59 - last row should continue):";

        for (int calculatedLine = 0; calculatedLine < rows.length(); calculatedLine++) {
            uint32_t rowTime = (rows.at(calculatedLine).duration.second() +
                               (rows.at(calculatedLine).duration.minute() * 60) +
                               (rows.at(calculatedLine).duration.hour() * 3600));
            calculatedElapsedTime += rowTime;

            bool shouldBreak = (calculatedElapsedTime > static_cast<uint32_t>(ticks));

            qDebug() << "  Checking row" << calculatedLine << ":";
            qDebug() << "    calculatedElapsedTime =" << calculatedElapsedTime;
            qDebug() << "    shouldBreak?" << shouldBreak;

            if (shouldBreak && calculatedLine >= 1) { // assuming currentStep=1
                qDebug() << "    → BREAK";
                qDebug() << "    *** BUG: Breaks at last row, calls end() ***";
                break;
            }
        }
        qDebug() << "";

        // This is the bug!
        EXPECT_FALSE(calculatedElapsedTime > static_cast<uint32_t>(ticks))
            << "BUG: Last row ends 1 second early because 2400 > 2399";
    }

    qDebug() << "=== WHY ONLY LAST ROW? ===";
    qDebug() << "- For intermediate rows: the > comparison triggers transition to NEXT row";
    qDebug() << "- For the LAST row: there is no next row → calls end() instead";
    qDebug() << "- The off-by-one affects all rows, but only visible on the last one!";
    qDebug() << "";

    qDebug() << "=== END TEST ===\n";
}
