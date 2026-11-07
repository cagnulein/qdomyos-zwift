#include <gtest/gtest.h>

#include "devices/rower.h"
#include "TestProformRowerParser.h"

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

#include "virtualdevices/virtualrowerftms.h"

TEST(VirtualRowerFTMSResistanceTest, EncodesResistanceBetweenPowerAndEnergyWithinDefaultMtu) {
    const QByteArray frame = virtualrowerftms::buildRowerData(40, 200, 70000, 150, 220, 12, 75, 140);
    ASSERT_EQ(frame.size(), 20);
    EXPECT_EQ(static_cast<uint8_t>(frame.at(0)), 0xac);
    EXPECT_EQ(static_cast<uint8_t>(frame.at(1)), 0x03);
    EXPECT_EQ(static_cast<uint8_t>(frame.at(7)), 0x01); // 24-bit distance, not truncated to 16 bits
    EXPECT_EQ(static_cast<uint8_t>(frame.at(10)), 220); // instantaneous power
    EXPECT_EQ(static_cast<uint8_t>(frame.at(12)), 12);  // resistance level, SINT16 resolution 1
    EXPECT_EQ(static_cast<uint8_t>(frame.at(13)), 0);
    EXPECT_EQ(static_cast<uint8_t>(frame.at(14)), 75);  // energy follows resistance
    EXPECT_EQ(static_cast<uint8_t>(frame.at(19)), 140); // heart rate is the final byte
}

TEST(VirtualRowerFTMSResistanceTest, AcceptsLegacyUint8AndCurrentSint16Tenths) {
    int level = 0;
    EXPECT_TRUE(virtualrowerftms::decodeTargetResistance(QByteArray::fromHex("0478"), 24, &level));
    EXPECT_EQ(level, 12);
    EXPECT_TRUE(virtualrowerftms::decodeTargetResistance(QByteArray::fromHex("047800"), 24, &level));
    EXPECT_EQ(level, 12);
    EXPECT_TRUE(virtualrowerftms::decodeTargetResistance(QByteArray::fromHex("04f000"), 24, &level));
    EXPECT_EQ(level, 24);
    EXPECT_EQ(virtualrowerftms::targetResistanceChangedStatus(12), QByteArray::fromHex("077800"));
}

TEST(VirtualRowerFTMSResistanceTest, RejectsMalformedOutOfRangeAndFractionalLevels) {
    int level = 0;
    EXPECT_FALSE(virtualrowerftms::decodeTargetResistance(QByteArray::fromHex("04"), 24, &level));
    EXPECT_FALSE(virtualrowerftms::decodeTargetResistance(QByteArray::fromHex("04780000"), 24, &level));
    EXPECT_FALSE(virtualrowerftms::decodeTargetResistance(QByteArray::fromHex("04fa00"), 24, &level));
    EXPECT_FALSE(virtualrowerftms::decodeTargetResistance(QByteArray::fromHex("040500"), 24, &level));
    EXPECT_FALSE(virtualrowerftms::decodeTargetResistance(QByteArray::fromHex("047b00"), 24, &level));
    EXPECT_FALSE(virtualrowerftms::decodeTargetResistance(QByteArray::fromHex("04f000"), 15, &level));
    EXPECT_FALSE(virtualrowerftms::decodeTargetResistance(QByteArray::fromHex("05f000"), 24, &level));
}
