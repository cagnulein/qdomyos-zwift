#pragma once

#include <gtest/gtest.h>

#include <QList>
#include <QScopedPointer>
#include <QString>
#include <QStringList>

#include "Tools/testsettings.h"
#include "ergtable.h"

namespace {

/**
 * Builds a table the way a real session does - from the persisted ergDataPoints string, through
 * the constructor - rather than through collectData(), which needs ten samples of a pair before
 * the point reaches the table at all.
 */
class ergTableFixture {
  public:
    ergTableFixture() : settings(QStringLiteral("QZ Testing"), QStringLiteral("ErgTableSelection")) {
        settings.activate();
    }

    /**
     * @param points cadence, wattage, resistance triples.
     */
    ergTable *build(const QList<ergDataPoint> &points) {
        QStringList encoded;
        for (const ergDataPoint &point : points)
            encoded.append(QStringLiteral("%1|%2|%3").arg(point.cadence).arg(point.wattage).arg(point.resistance));

        settings.qsettings.setValue(QZSettings::ergDataPoints, encoded.join(QStringLiteral(";")));
        table.reset(new ergTable());
        return table.data();
    }

  private:
    TestSettings settings;
    QScopedPointer<ergTable> table;
};

/**
 * The shape that produced the defect on a YPOO console: levels 1-4 learned only while warming up
 * at a high cadence, level 5 never visited at all, and the levels that carry the low cadences
 * learned higher up the range. The old estimator answered every one of these with a wattage
 * measured at 80+ rpm, so a rider at 60 rpm was told level 4 made 88 W when it makes 59.
 */
QList<ergDataPoint> sparseLowEnd() {
    QList<ergDataPoint> points;
    points.append(ergDataPoint(60, 56, 1)); // level 1 is the only one learned at a low cadence
    points.append(ergDataPoint(80, 83, 1));
    points.append(ergDataPoint(81, 85, 2));
    points.append(ergDataPoint(84, 90, 2));
    points.append(ergDataPoint(80, 86, 3));
    points.append(ergDataPoint(84, 92, 3));
    points.append(ergDataPoint(80, 88, 4));
    points.append(ergDataPoint(84, 94, 4));
    // level 5: no samples at all
    points.append(ergDataPoint(92, 113, 6)); // learned once, at a cadence no one rides at
    points.append(ergDataPoint(100, 127, 6));
    points.append(ergDataPoint(60, 84, 14)); // the levels that do carry low cadences
    points.append(ergDataPoint(80, 125, 14));
    points.append(ergDataPoint(60, 89, 16));
    points.append(ergDataPoint(80, 134, 16));
    return points;
}

} // namespace

TEST(ErgTableSelectionTest, ExtrapolatesBelowTheLowestCadenceLearned) {
    ergTableFixture fixture;
    QList<ergDataPoint> points;
    points.append(ergDataPoint(80, 100, 5));
    points.append(ergDataPoint(90, 120, 5));
    ergTable *table = fixture.build(points);

    // Repeating the 80 rpm measurement here is what made ERG undershoot: it claims the level
    // still makes 100 W at 60 rpm. The slope of the two samples says 60.
    EXPECT_NEAR(60.0, table->estimateWattage(60, 5), 0.5);
    EXPECT_NEAR(80.0, table->estimateWattage(70, 5), 0.5);
}

TEST(ErgTableSelectionTest, ExtrapolatesAboveTheHighestCadenceLearned) {
    ergTableFixture fixture;
    QList<ergDataPoint> points;
    points.append(ergDataPoint(80, 100, 5));
    points.append(ergDataPoint(90, 120, 5));
    ergTable *table = fixture.build(points);

    EXPECT_NEAR(140.0, table->estimateWattage(100, 5), 0.5);
}

TEST(ErgTableSelectionTest, InterpolatesBetweenMeasuredCadences) {
    ergTableFixture fixture;
    QList<ergDataPoint> points;
    points.append(ergDataPoint(80, 100, 5));
    points.append(ergDataPoint(90, 120, 5));
    ergTable *table = fixture.build(points);

    EXPECT_NEAR(100.0, table->estimateWattage(80, 5), 0.5);
    EXPECT_NEAR(110.0, table->estimateWattage(85, 5), 0.5);
    EXPECT_NEAR(120.0, table->estimateWattage(90, 5), 0.5);
}

TEST(ErgTableSelectionTest, NeverExtrapolatesBelowZero) {
    ergTableFixture fixture;
    QList<ergDataPoint> points;
    points.append(ergDataPoint(80, 100, 5));
    points.append(ergDataPoint(90, 120, 5));
    ergTable *table = fixture.build(points);

    EXPECT_DOUBLE_EQ(0.0, table->estimateWattage(20, 5));
}

TEST(ErgTableSelectionTest, UnlearnedLevelIsInterpolatedNotBorrowed) {
    ergTableFixture fixture;
    QList<ergDataPoint> points;
    points.append(ergDataPoint(80, 100, 4));
    points.append(ergDataPoint(90, 120, 4));
    points.append(ergDataPoint(80, 200, 8));
    points.append(ergDataPoint(90, 240, 8));
    ergTable *table = fixture.build(points);

    // Level 6 has no samples. Adopting the nearest level's numbers wholesale gave it either
    // 100 W or 200 W depending on which one the scan happened to reach first.
    EXPECT_NEAR(150.0, table->estimateWattage(80, 6), 0.5);
    EXPECT_NEAR(125.0, table->estimateWattage(80, 5), 0.5);
    EXPECT_NEAR(175.0, table->estimateWattage(80, 7), 0.5);
}

TEST(ErgTableSelectionTest, PicksTheNearestLevelNotTheLowerBracket) {
    ergTableFixture fixture;
    QList<ergDataPoint> points;
    for (uint16_t r = 1; r <= 10; r++)
        points.append(ergDataPoint(80, 10 * r, r)); // level r makes 10r watts at 80 rpm
    ergTable *table = fixture.build(points);

    // 68 W sits between level 6 (60 W) and level 7 (70 W). Returning the lower end of the
    // bracket - the old behaviour - was short by 8 W where taking the nearest is short by 2.
    EXPECT_EQ(7, table->resistanceFromPowerRequest(68, 80, 20));
    EXPECT_EQ(6, table->resistanceFromPowerRequest(62, 80, 20));
    EXPECT_EQ(6, table->resistanceFromPowerRequest(60, 80, 20));
}

TEST(ErgTableSelectionTest, ASpikeInTheCurveDoesNotTrapTheSearch) {
    ergTableFixture fixture;
    QList<ergDataPoint> points;
    for (uint16_t r = 1; r <= 10; r++)
        // Level 4 reads high - one bad sample, or a level learned at a cadence no one rides.
        points.append(ergDataPoint(80, r == 4 ? 95 : 10 * r, r));
    ergTable *table = fixture.build(points);

    // The spike must not become the answer for every target above it: 90 W belongs on level 9,
    // which really makes 90 W, not on level 4, which makes 40.
    EXPECT_EQ(9, table->resistanceFromPowerRequest(90, 80, 20));
}

TEST(ErgTableSelectionTest, ADipInTheCurveIsAbsorbed) {
    ergTableFixture fixture;
    QList<ergDataPoint> points;
    for (uint16_t r = 1; r <= 10; r++)
        points.append(ergDataPoint(80, r == 6 ? 5 : 10 * r, r));
    ergTable *table = fixture.build(points);

    // Level 6 reading 5 W must not be offered for a 5 W target, nor drag the search off the
    // levels around it: 70 W still belongs on level 7.
    EXPECT_EQ(7, table->resistanceFromPowerRequest(70, 80, 20));
    EXPECT_EQ(9, table->resistanceFromPowerRequest(90, 80, 20));
}

TEST(ErgTableSelectionTest, LowCadenceRequestDoesNotLandOnTheLowLevels) {
    ergTableFixture fixture;
    ergTable *table = fixture.build(sparseLowEnd());

    // The measured regression. At 60 rpm the old selector answered level 4 for anything from
    // 70 to 113 W, because level 5 borrowed level 6's 113 W - a figure from 92 rpm - and the
    // upward scan stopped at the first bracket it made. Level 4 makes 59 W at 60 rpm.
    const uint16_t chosen = table->resistanceFromPowerRequest(93, 60, 32);
    EXPECT_GT(chosen, 4);
    EXPECT_NEAR(93.0, table->estimateWattage(60, chosen), 12.0);
}

TEST(ErgTableSelectionTest, NoCadenceStillAnswersTheBottomOfTheRange) {
    ergTableFixture fixture;
    ergTable *table = fixture.build(sparseLowEnd());

    // The table cannot answer without a cadence; it is the caller's job to hold position
    // instead of acting on this. ftmsbike::resistanceFromPowerRequest() does.
    EXPECT_EQ(1, table->resistanceFromPowerRequest(150, 0, 32));
}

TEST(ErgTableSelectionTest, EmptyTableIsHarmless) {
    ergTableFixture fixture;
    ergTable *table = fixture.build(QList<ergDataPoint>());

    EXPECT_DOUBLE_EQ(0.0, table->estimateWattage(80, 5));
    EXPECT_EQ(1, table->resistanceFromPowerRequest(150, 80, 32));
}

TEST(ErgTableSelectionTest, SamplesAreKeptImmediatelyWhenTheReportIsTheCommand) {
    ergTableFixture fixture;
    ergTable *table = fixture.build(QList<ergDataPoint>());

    // Ten samples of one pair make a point. Without the flag the settling window swallows them,
    // which is what stops a slew ramp - a level every 250-500 ms - from ever teaching the table
    // the levels it passes through.
    table->setResistanceReportsCommand(true);
    for (int i = 0; i < WattageStats::MIN_SAMPLES_REQUIRED; i++)
        table->collectData(80, 150, 9);

    EXPECT_EQ(1, table->getConsolidatedData().size());
    EXPECT_NEAR(150.0, table->estimateWattage(80, 9), 0.5);
}

TEST(ErgTableSelectionTest, SamplesAreDroppedRightAfterAChangeByDefault) {
    ergTableFixture fixture;
    ergTable *table = fixture.build(QList<ergDataPoint>());

    for (int i = 0; i < WattageStats::MIN_SAMPLES_REQUIRED; i++)
        table->collectData(80, 150, 9);

    EXPECT_TRUE(table->getConsolidatedData().isEmpty());
}
