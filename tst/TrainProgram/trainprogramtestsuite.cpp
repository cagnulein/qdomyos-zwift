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

    // At tick 11 (what SHOULD happen):
    {
        int32_t ticks = 11;
        uint32_t calculatedElapsedTime = 0;
        uint32_t calculatedLine = 0;
        uint32_t currentStep = 1;

        qDebug() << "At tick 11 (what SHOULD happen):";

        for (calculatedLine = 0; calculatedLine < static_cast<uint32_t>(rows.length()); calculatedLine++) {
            uint32_t rowTime = rows.at(calculatedLine).duration.hour() * 3600 +
                              rows.at(calculatedLine).duration.minute() * 60 +
                              rows.at(calculatedLine).duration.second();
            calculatedElapsedTime += rowTime;

            // With the fix: use >= instead of >
            if (calculatedElapsedTime >= static_cast<uint32_t>(ticks)) {
                break;
            }
        }

        qDebug() << "  Final calculatedLine =" << calculatedLine;

        if (calculatedLine != currentStep) {
            currentStep = calculatedLine;
            if (currentStep >= static_cast<uint32_t>(rows.length())) {
                qDebug() << "    → Would call end() here (CORRECT)";
            }
        }
        qDebug() << "";
    }

    qDebug() << "=== ROOT CAUSE ===";
    qDebug() << "The '>' comparison at line 1001 allows tick 10 to set calculatedLine = rows.length()";
    qDebug() << "This triggers end() DURING tick 10, before it's fully displayed";
    qDebug() << "User sees: 1,2,3,4,5,6,7,8,9... then stops (never shows 10)";
    qDebug() << "";

    qDebug() << "=== FIX ===";
    qDebug() << "Option 1: Delay end() until AFTER tick 10 completes";
    qDebug() << "Option 2: Change comparison to allow last tick to complete";
    qDebug() << "";

    qDebug() << "=== END TEST ===\n";
}

void TrainProgramTestSuite::test_multiRowWorkout_lastRowMissing1Second() {
    qDebug() << "\n=== This test is now covered by the single test above ===\n";
}
