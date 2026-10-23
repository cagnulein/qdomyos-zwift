#include <gtest/gtest.h>

#include "Tools/testsettings.h"
#include "characteristics/characteristicwriteprocessor2ad9.h"
#include "devices/treadmill.h"

namespace {
class ControlPointTestTreadmill : public treadmill {
  public:
    void changeSpeed(double value) override { receivedSpeed = value; }
    void changeInclination(double grade, double) override { receivedIncline = grade; }
    double receivedSpeed = -1;
    double receivedIncline = -100;
};

class FtmsTreadmillControlTest : public ::testing::Test {
  protected:
    TestSettings settings{QStringLiteral("QZTests"), QStringLiteral("FtmsTreadmillControl")};
    ControlPointTestTreadmill treadmill;
    CharacteristicWriteProcessor2AD9 processor{1, 0, &treadmill, nullptr};
    void SetUp() override { settings.activate(); }
};
} // namespace

TEST_F(FtmsTreadmillControlTest, DecodesSpeedTargetsAcrossHighBitByteBoundaries) {
    struct Example {
        const char *packet;
        double speed;
    };
    for (const auto &example : {Example{"027800", 1.2}, Example{"028200", 1.3}, Example{"02c800", 2.0},
                                Example{"020401", 2.6}, Example{"025807", 18.8}}) {
        QByteArray reply;
        ASSERT_EQ(processor.writeProcess(0x2AD9, QByteArray::fromHex(example.packet), reply), CP_OK);
        EXPECT_DOUBLE_EQ(treadmill.receivedSpeed, example.speed) << example.packet;
    }
}

TEST_F(FtmsTreadmillControlTest, DecodesSignedInclineAndHighBitPositiveTargets) {
    struct Example {
        const char *packet;
        double incline;
    };
    for (const auto &example : {Example{"03c4ff", -6}, Example{"03f6ff", -1}, Example{"030000", 0},
                                Example{"038200", 13}, Example{"039600", 15}}) {
        QByteArray reply;
        ASSERT_EQ(processor.writeProcess(0x2AD9, QByteArray::fromHex(example.packet), reply), CP_OK);
        EXPECT_DOUBLE_EQ(treadmill.receivedIncline, example.incline) << example.packet;
    }
}
