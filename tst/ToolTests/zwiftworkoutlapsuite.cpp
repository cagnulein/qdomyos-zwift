#include "zwiftworkoutlapsuite.h"

#include <QFile>
#include <QTemporaryDir>

#include "zwiftworkout.h"

namespace {

static int countAutoLapBoundaries(const QList<trainrow> &rows) {
    int laps = 0;
    for (int i = 0; i < rows.size(); ++i) {
        const bool hasNextRow = (i + 1 < rows.size());
        bool emitLapForBoundary = false;

        if (!hasNextRow) {
            emitLapForBoundary = true;
        } else {
            const trainrow &currentRow = rows.at(i);
            const trainrow &nextRow = rows.at(i + 1);
            const bool inRampNow = QTime(0, 0, 0).secsTo(currentRow.rampDuration) > 0;
            const bool nextIsRampContinuation = QTime(0, 0, 0).secsTo(nextRow.rampElapsed) > 0;
            const bool sameRampContinuation = inRampNow && nextIsRampContinuation;
            emitLapForBoundary = !sameRampContinuation;
        }

        if (emitLapForBoundary) {
            ++laps;
        }
    }
    return laps;
}

} // namespace

TEST_F(ZwiftWorkoutLapSuite, WarmupIntervalsCooldownBoundaries) {
    QTemporaryDir tempDir;
    ASSERT_TRUE(tempDir.isValid());

    const QString zwoPath = tempDir.filePath("lap_boundaries_test.zwo");
    QFile zwoFile(zwoPath);
    ASSERT_TRUE(zwoFile.open(QIODevice::WriteOnly | QIODevice::Text));

    const QByteArray zwo = R"(<workout_file>
    <author>test</author>
    <name>lap boundaries</name>
    <description>test</description>
    <sportType>bike</sportType>
    <workout>
        <Warmup Duration="60" PowerLow="0.5" PowerHigh="0.75"/>
        <IntervalsT Repeat="2" OnDuration="30" OffDuration="15" OnPower="1" OffPower="0.45"/>
        <Cooldown Duration="40" PowerLow="0.45" PowerHigh="0.3"/>
    </workout>
</workout_file>)";
    zwoFile.write(zwo);
    zwoFile.close();

    QString description;
    QString tags;
    const QList<trainrow> rows = zwiftworkout::load(zwoPath, &description, &tags);
    ASSERT_FALSE(rows.isEmpty());

    // Expected boundaries:
    // warmup end, interval on->off (rep1), off->on (rep2), on->off (rep2), cooldown start, cooldown end.
    EXPECT_EQ(countAutoLapBoundaries(rows), 6);
}
