#include <gtest/gtest.h>

#include "Tools/testsettings.h"
#include "devices/treadmill.h"

namespace {
class KsEspInclinationTest : public ::testing::Test {
  protected:
    TestSettings settings{QStringLiteral("QZTests"), QStringLiteral("KsEspInclination")};

    void SetUp() override {
        settings.activate();
        settings.qsettings.clear();
    }

    void TearDown() override { settings.qsettings.clear(); }
};
} // namespace

TEST_F(KsEspInclinationTest, OtherTreadmillsKeepTheFifteenPercentReverseLimit) {
    EXPECT_DOUBLE_EQ(treadmill::treadmillInclinationOverrideReverse(14.5), 14.5);
    EXPECT_DOUBLE_EQ(treadmill::treadmillInclinationOverrideReverse(15.0), 15.0);
    EXPECT_DOUBLE_EQ(treadmill::treadmillInclinationOverrideReverse(16.0), 15.0);
    EXPECT_DOUBLE_EQ(treadmill::treadmillInclinationOverrideReverse(24.0), 15.0);
}

TEST_F(KsEspInclinationTest, ExtendedReverseRangeKeepsTargetsFromFifteenThroughTwentyFive) {
    for (double target : {15.0, 15.5, 16.0, 18.0, 24.0, 24.5, 25.0}) {
        EXPECT_DOUBLE_EQ(treadmill::treadmillInclinationOverrideReverse(target, 25.0), target)
            << "Incorrect KS-ESP incline target " << target;
    }

    // A down button press after increasing on the treadmill console to 25%
    // must request 24%, not snap the physical treadmill back to 15%.
    EXPECT_DOUBLE_EQ(treadmill::treadmillInclinationOverrideReverse(25.0 - 1.0, 25.0), 24.0);
    EXPECT_DOUBLE_EQ(treadmill::treadmillInclinationOverrideReverse(26.0, 25.0), 25.0);
}

TEST_F(KsEspInclinationTest, ExtendedReverseRangeKeepsLowAndNegativeTargets) {
    for (double target : {-2.0, -1.0, 0.0, 1.0, 10.0}) {
        EXPECT_DOUBLE_EQ(treadmill::treadmillInclinationOverrideReverse(target, 25.0), target);
    }
}

TEST_F(KsEspInclinationTest, UserConfiguredMaxInclineStillApplies) {
    treadmill device;

    settings.qsettings.setValue(QZSettings::treadmill_incline_max, 25.0);
    device.changeInclination(25.0, 25.0);
    EXPECT_DOUBLE_EQ(device.lastRequestedInclination().value(), 25.0);

    settings.qsettings.setValue(QZSettings::treadmill_incline_max, 18.0);
    device.changeInclination(25.0, 25.0);
    EXPECT_DOUBLE_EQ(device.lastRequestedInclination().value(), 18.0);
}
