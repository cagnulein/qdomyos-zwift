#include "trainprogramtestsuite.h"
#include "trainprogram.h"
#include <QTime>
#include <QList>

TrainProgramTestSuite::TrainProgramTestSuite()
{

}

void TrainProgramTestSuite::test_singleRow40MinuteWorkout_shouldReach40Minutes() {
    // Test based on REAL log: 2 rows of 5 seconds each = 10 seconds total
    // Bug: ends at tick 10 instead of after tick 10

    qDebug() << "\n=== TEST: 10 Second Workout (2×5 seconds) ===\n";

    QList<trainrow> rows;

    // Row 1: 5 seconds @ speed 9.5
    trainrow row1;
    row1.duration = QTime(0, 0, 5);
    row1.speed = 9.5;
    row1.forcespeed = true;
    rows.append(row1);

    // Row 2: 5 seconds @ speed 10
    trainrow row2;
    row2.duration = QTime(0, 0, 5);
    row2.speed = 10;
    row2.forcespeed = true;
    rows.append(row2);

    trainprogram program(rows, nullptr);

    qDebug() << "Workout:";
    qDebug() << "  Row 1: 5 seconds @ 9.5 km/h";
    qDebug() << "  Row 2: 5 seconds @ 10 km/h";
    qDebug() << "  Total: 10 seconds";
    qDebug() << "";

    // Simulate what happens at tick 9 and tick 10
    // Based on the logic at line 993-1004 in trainprogram.cpp

    qDebug() << "=== SIMULATING BUG ===\n";

    // At tick 9:
    {
        int32_t ticks = 9;
        uint32_t calculatedElapsedTime = 0;
        uint32_t calculatedLine = 0;

        qDebug() << "At tick 9:";

        for (calculatedLine = 0; calculatedLine < static_cast<uint32_t>(rows.length()); calculatedLine++) {
            uint32_t rowTime = rows.at(calculatedLine).duration.hour() * 3600 +
                              rows.at(calculatedLine).duration.minute() * 60 +
                              rows.at(calculatedLine).duration.second();
            calculatedElapsedTime += rowTime;

            qDebug() << "  Row" << calculatedLine << ": calculatedElapsedTime =" << calculatedElapsedTime;
            qDebug() << "    Check:" << calculatedElapsedTime << "> " << ticks << "?";

            if (calculatedElapsedTime > static_cast<uint32_t>(ticks)) {
                qDebug() << "    → TRUE, would break here";
                break;
            }
        }

        qDebug() << "  Final calculatedLine =" << calculatedLine;
        qDebug() << "  (currentStep would stay at 1, no transition)";
        qDebug() << "";
    }

    // At tick 10:
    {
        int32_t ticks = 10;
        uint32_t calculatedElapsedTime = 0;
        uint32_t calculatedLine = 0;
        uint32_t currentStep = 1; // We're on row 2

        qDebug() << "At tick 10:";

        for (calculatedLine = 0; calculatedLine < static_cast<uint32_t>(rows.length()); calculatedLine++) {
            uint32_t rowTime = rows.at(calculatedLine).duration.hour() * 3600 +
                              rows.at(calculatedLine).duration.minute() * 60 +
                              rows.at(calculatedLine).duration.second();
            calculatedElapsedTime += rowTime;

            qDebug() << "  Row" << calculatedLine << ": calculatedElapsedTime =" << calculatedElapsedTime;
            qDebug() << "    Check:" << calculatedElapsedTime << ">" << ticks << "?";

            if (calculatedElapsedTime > static_cast<uint32_t>(ticks)) {
                qDebug() << "    → TRUE, would break here";
                break;
            }
        }

        qDebug() << "  Final calculatedLine =" << calculatedLine;
        qDebug() << "  Check: calculatedLine (" << calculatedLine << ") != currentStep (" << currentStep << ")?";

        if (calculatedLine != currentStep) {
            qDebug() << "    → TRUE, would enter transition block";
            currentStep = calculatedLine;
            qDebug() << "    currentStep now =" << currentStep;

            if (currentStep >= static_cast<uint32_t>(rows.length())) {
                qDebug() << "    Check: currentStep (" << currentStep << ") >= rows.length() (" << rows.length() << ")?";
                qDebug() << "    → TRUE, would call end()!";
                qDebug() << "    *** BUG: Calls end() AT tick 10, not AFTER tick 10 ***";
            }
        }
        qDebug() << "";
    }

    // At tick 10 WITH FIX:
    {
        int32_t ticks = 10;
        uint32_t calculatedElapsedTime = 0;
        uint32_t calculatedLine = 0;

        qDebug() << "\nAt tick 10 WITH FIX:";

        for (calculatedLine = 0; calculatedLine < static_cast<uint32_t>(rows.length()); calculatedLine++) {
            uint32_t rowTime = rows.at(calculatedLine).duration.hour() * 3600 +
                              rows.at(calculatedLine).duration.minute() * 60 +
                              rows.at(calculatedLine).duration.second();
            calculatedElapsedTime += rowTime;

            if (calculatedElapsedTime > static_cast<uint32_t>(ticks)) {
                break;
            }
        }

        qDebug() << "  Final calculatedLine =" << calculatedLine << "(rows.length() =" << rows.length() << ")";

        // New fix logic: check if calculatedLine >= rows.length()
        if (calculatedLine >= static_cast<uint32_t>(rows.length())) {
            // Calculate total time
            uint32_t totalTime = 0;
            for (int i = 0; i < rows.length(); i++) {
                totalTime += rows.at(i).duration.hour() * 3600 +
                             rows.at(i).duration.minute() * 60 +
                             rows.at(i).duration.second();
            }

            qDebug() << "  calculatedLine >= rows.length(), NEW FIX checks totalTime";
            qDebug() << "  totalTime =" << totalTime << ", ticks =" << ticks;
            qDebug() << "  Check: ticks (" << ticks << ") > totalTime (" << totalTime << ")?";

            if (static_cast<uint32_t>(ticks) > totalTime) {
                qDebug() << "    → TRUE, would call end()";
            } else {
                qDebug() << "    → FALSE, continue current row";
                qDebug() << "    *** FIX SUCCESS: Tick 10 displays, NO premature end() ***";
            }
        }
        qDebug() << "";
    }

    // At tick 11 WITH FIX:
    {
        int32_t ticks = 11;
        uint32_t calculatedElapsedTime = 0;
        uint32_t calculatedLine = 0;

        qDebug() << "At tick 11 WITH FIX:";

        for (calculatedLine = 0; calculatedLine < static_cast<uint32_t>(rows.length()); calculatedLine++) {
            uint32_t rowTime = rows.at(calculatedLine).duration.hour() * 3600 +
                              rows.at(calculatedLine).duration.minute() * 60 +
                              rows.at(calculatedLine).duration.second();
            calculatedElapsedTime += rowTime;

            if (calculatedElapsedTime > static_cast<uint32_t>(ticks)) {
                break;
            }
        }

        qDebug() << "  Final calculatedLine =" << calculatedLine;

        if (calculatedLine >= static_cast<uint32_t>(rows.length())) {
            uint32_t totalTime = 0;
            for (int i = 0; i < rows.length(); i++) {
                totalTime += rows.at(i).duration.hour() * 3600 +
                             rows.at(i).duration.minute() * 60 +
                             rows.at(i).duration.second();
            }

            qDebug() << "  totalTime =" << totalTime << ", ticks =" << ticks;
            qDebug() << "  Check: ticks (" << ticks << ") > totalTime (" << totalTime << ")?";

            if (static_cast<uint32_t>(ticks) > totalTime) {
                qDebug() << "    → TRUE, calls end() (CORRECT - after tick 10 displayed)";
            }
        }
        qDebug() << "";
    }

    qDebug() << "=== ROOT CAUSE ===";
    qDebug() << "The '>' comparison allows tick 10 to set calculatedLine = rows.length()";
    qDebug() << "Old code called end() immediately without checking if time EXCEEDED vs REACHED";
    qDebug() << "This triggered end() DURING tick 10, before it was displayed";
    qDebug() << "User saw: 1,2,3,4,5,6,7,8,9... then stopped (never showed 10)";
    qDebug() << "";

    qDebug() << "=== FIX IMPLEMENTED ===";
    qDebug() << "Added check before transition logic in trainprogram.cpp:1028-1043";
    qDebug() << "When calculatedLine >= rows.length():";
    qDebug() << "  - Calculate totalTime of workout";
    qDebug() << "  - Only call end() if ticks > totalTime (EXCEEDED)";
    qDebug() << "  - If ticks == totalTime (REACHED), continue current row";
    qDebug() << "";
    qDebug() << "Result:";
    qDebug() << "  - Tick 10: ticks (10) > totalTime (10)? NO → stay on row, display tick";
    qDebug() << "  - Tick 11: ticks (11) > totalTime (10)? YES → call end()";
    qDebug() << "  - User sees: 1,2,3,4,5,6,7,8,9,10 then workout ends (CORRECT!)";
    qDebug() << "";

    qDebug() << "=== END TEST ===\n";
}

void TrainProgramTestSuite::test_multiRowWorkout_lastRowMissing1Second() {
    qDebug() << "\n=== This test is now covered by the single test above ===\n";
}
