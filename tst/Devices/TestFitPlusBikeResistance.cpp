#include <gtest/gtest.h>

#include "devices/fitplusbike/fitplusbike.h"

TEST(FitPlusBikeResistance, ConvertsNativeLevelToPelotonScale) {
    EXPECT_NEAR(fitplusbike::pelotonResistanceFromBikeLevel(7, 24, 1.0, 0.0), 29.1667, 0.0001);
    EXPECT_NEAR(fitplusbike::pelotonResistanceFromBikeLevel(8, 24, 1.0, 0.0), 33.3333, 0.0001);
    EXPECT_NEAR(fitplusbike::pelotonResistanceFromBikeLevel(24, 24, 1.0, 0.0), 100.0, 0.0001);
}
