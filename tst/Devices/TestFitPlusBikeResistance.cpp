#include <gtest/gtest.h>

#include "devices/fitplusbike/fitplusbike.h"

TEST(FitPlusBikeResistance, ConvertsNativeLevelToPelotonScale) {
    EXPECT_NEAR(fitplusbike::pelotonResistanceFromBikeLevel(5, 16, 1.0, 0.0), 31.25, 0.0001);
    EXPECT_NEAR(fitplusbike::pelotonResistanceFromBikeLevel(8, 16, 1.0, 0.0), 50.0, 0.0001);
    EXPECT_NEAR(fitplusbike::pelotonResistanceFromBikeLevel(16, 16, 1.0, 0.0), 100.0, 0.0001);
}
