#include <gtest/gtest.h>

#include "devices/rower.h"

class RowerTargetResistanceTestDevice : public rower {
  public:
    void setReportedResistance(resistance_t resistance) { Resistance = resistance; }
};

TEST(RowerTargetResistanceRegressionTest, JorotoTelemetryChangesDoNotReplaceRawResistanceBase) {
    RowerTargetResistanceTestDevice device;
    device.setAutoResistance(false);

    // Reproduce the JOROTO MR40 support case: the rower reports resistance 1,
    // then flips to 2 while Target Resistance is increasing the difficulty gain.
    device.setDifficult(1.0);
    device.setReportedResistance(1);
    device.changeResistance(device.currentResistance().value());
    EXPECT_DOUBLE_EQ(device.lastRequestedResistance().value(), 1.0);

    device.setReportedResistance(2);
    device.setDifficult(2.0);
    device.changeResistance(device.currentResistance().value());

    // Keep raw resistance 1 as the base: 1 * 2.0 == 2. The old behavior
    // replaced the base with telemetry 2 and jumped to 2 * 2.0 == 4.
    EXPECT_DOUBLE_EQ(device.lastRequestedResistance().value(), 2.0);

    device.setDifficult(3.0);
    device.changeResistance(device.currentResistance().value());
    EXPECT_DOUBLE_EQ(device.lastRequestedResistance().value(), 3.0);
}

TEST(RowerTargetResistanceRegressionTest, ExplicitResistanceRequestStillReplacesRawBase) {
    RowerTargetResistanceTestDevice device;
    device.setAutoResistance(false);

    device.setDifficult(2.0);
    device.setReportedResistance(1);
    device.changeResistance(1);
    EXPECT_DOUBLE_EQ(device.lastRequestedResistance().value(), 2.0);

    // With unchanged difficulty this is a real new raw request, not a Target
    // Resistance multiplier reapplication.
    device.changeResistance(3);
    EXPECT_DOUBLE_EQ(device.lastRequestedResistance().value(), 6.0);

    device.setReportedResistance(1);
    device.setDifficult(3.0);
    device.changeResistance(device.currentResistance().value());
    EXPECT_DOUBLE_EQ(device.lastRequestedResistance().value(), 9.0);
}
