#include <gtest/gtest.h>

#include "devices/bkoolbike/bkoolbike.h"

TEST(BkoolStandaloneResistanceRegression, RetainsLastLocalTileValueWhenTelemetryIsZero) {
    // The effective resistance sent to the hardware can include gear modifiers,
    // but the displayed resistance must remain the unmodified tile value.
    EXPECT_DOUBLE_EQ(bkoolbike::displayedResistance(0, 2, false), 2);
    EXPECT_DOUBLE_EQ(bkoolbike::displayedResistance(0, 10, false), 10);
    EXPECT_DOUBLE_EQ(bkoolbike::displayedResistance(0, 124, false), 124);
}

TEST(BkoolStandaloneResistanceRegression, PreservesTelemetryWithoutLocalCommand) {
    EXPECT_DOUBLE_EQ(bkoolbike::displayedResistance(7, -1, false), 7);
}

TEST(BkoolStandaloneResistanceRegression, NeverOverridesExternalFtmsOrErgControl) {
    EXPECT_DOUBLE_EQ(bkoolbike::displayedResistance(7, 124, true), 7);
    EXPECT_DOUBLE_EQ(bkoolbike::displayedResistance(0, 124, true), 0);
}
