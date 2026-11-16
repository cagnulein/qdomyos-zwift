#include "virtualdevices/virtualrowercadence.h"

#include <gtest/gtest.h>

TEST(VirtualRowerCadenceTest, AppliesGainAndOffset) {
    EXPECT_DOUBLE_EQ(virtualrowercadence::transform(80.0, 0.25, 0.0), 20.0);
    EXPECT_DOUBLE_EQ(virtualrowercadence::transform(80.0, 0.5, 2.0), 42.0);
}

TEST(VirtualRowerCadenceTest, PreservesCadenceWithDefaultCalibration) {
    EXPECT_DOUBLE_EQ(virtualrowercadence::transform(37.5, 1.0, 0.0), 37.5);
}
