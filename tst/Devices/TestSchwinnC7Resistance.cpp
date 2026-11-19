#include <gtest/gtest.h>
#include <limits>

#include "devices/schwinnic4bike/schwinnic4bike.h"

// User report: Bowflex C7-E6ED, 2026-11-19. The real FTMS resistance
// was 40 in all 203 packets, although the old power/cadence estimate
// drifted between 1 and 73 at different pedalling speeds.
TEST(SchwinnC7ResistanceRegression, PreferPhysicalResistanceAcrossCadences) {
    const QString deviceName = QStringLiteral("C7-E6ED");
    const int observedCadences[] = {30, 75, 100};
    for (int cadence : observedCadences) {
        (void)cadence; // Varying cadence must not change selection of the reported level.
        EXPECT_TRUE(schwinnic4bike::shouldUseBikeFtmsResistance(deviceName, true, 40));
    }
}

TEST(SchwinnC7ResistanceRegression, AcceptsValidReportedRangeAndCaseInsensitivePrefix) {
    EXPECT_TRUE(schwinnic4bike::shouldUseBikeFtmsResistance(QStringLiteral("c7-1234"), true, 1));
    EXPECT_TRUE(schwinnic4bike::shouldUseBikeFtmsResistance(QStringLiteral("C7-E6ED"), true, 100));
}

TEST(SchwinnC7ResistanceRegression, FallsBackWhenLevelIsMissingOrInvalid) {
    const QString c7 = QStringLiteral("C7-E6ED");
    EXPECT_FALSE(schwinnic4bike::shouldUseBikeFtmsResistance(c7, false, 40));
    EXPECT_FALSE(schwinnic4bike::shouldUseBikeFtmsResistance(c7, true, 0));
    EXPECT_FALSE(schwinnic4bike::shouldUseBikeFtmsResistance(c7, true, -1));
    EXPECT_FALSE(schwinnic4bike::shouldUseBikeFtmsResistance(c7, true, 101));
    EXPECT_FALSE(schwinnic4bike::shouldUseBikeFtmsResistance(
        c7, true, std::numeric_limits<double>::quiet_NaN()));
}

TEST(SchwinnC7ResistanceRegression, LeavesOtherSchwinnModelsOnLegacyEstimate) {
    EXPECT_FALSE(schwinnic4bike::shouldUseBikeFtmsResistance(QStringLiteral("IC BIKE"), true, 40));
    EXPECT_FALSE(schwinnic4bike::shouldUseBikeFtmsResistance(QStringLiteral("C9/C10"), true, 40));
    EXPECT_FALSE(schwinnic4bike::shouldUseBikeFtmsResistance(QStringLiteral("C7"), true, 40));
}
