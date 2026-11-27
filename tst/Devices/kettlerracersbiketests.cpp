#include <gtest/gtest.h>

#include "devices/kettlerracersbike/kettlerhandshake.h"
#include "devices/kettlerracersbike/kettlercsctelemetry.h"

namespace {
struct TestVector {
    const char *seedHex;
    const char *cipherHex;
};
}

TEST(KettlerHandshakeTest, KnownVectors)
{
    const TestVector vectors[] = {
        {"000152D2D787", "E2223C64325BAD88CA1390E7A0FED75A"},
        {"000160AC609A", "213276DEB7413E7D5D1A2C514362E56B"},
        {"000117E0D1F9", "29A52B6D31AEF7E172D89647CC7934B5"},
    };

    for (const auto &vec : vectors) {
        const auto seed = QByteArray::fromHex(vec.seedHex);
        const auto expected = QByteArray::fromHex(vec.cipherHex);
        const auto actual = kettler::computeHandshake(seed);
        EXPECT_EQ(actual, expected) << "Failed vector for seed " << vec.seedHex;
    }
}

TEST(KettlerHandshakeTest, RejectsUnexpectedSeedSize)
{
    EXPECT_TRUE(kettler::computeHandshake(QByteArray()).isEmpty());
    EXPECT_TRUE(kettler::computeHandshake(QByteArray::fromHex("0011")) .isEmpty());
    EXPECT_TRUE(kettler::computeHandshake(QByteArray(7, '\x00')).isEmpty());
}

TEST(KettlerCscTest, DecodesUnsignedLittleEndianCounters)
{
    const auto packet = QByteArray::fromHex("03fedcba987aebf1a2f8d3");
    EXPECT_EQ(kettler::readLe32(packet.constData() + 1), 0x98badcfeu);
    EXPECT_EQ(kettler::readLe16(packet.constData() + 5), 0xeb7au);
    EXPECT_EQ(kettler::readLe16(packet.constData() + 7), 0xa2f1u);
    EXPECT_EQ(kettler::readLe16(packet.constData() + 9), 0xd3f8u);
}

TEST(KettlerCscTest, ConvertsMetresPerSecondToKilometresPerHour)
{
    EXPECT_NEAR(kettler::wheelSpeedKmh(10, 9, 2048, 1024), 7.56, 0.0001);
    // Recorded Racer S packets, 27 November 2026, including event timer rollover.
    const auto previous = QByteArray::fromHex("039900000019d775004526");
    const auto current = QByteArray::fromHex("039b000000a2357600e82f");
    EXPECT_NEAR(kettler::wheelSpeedKmh(
                    kettler::readLe32(current.constData() + 1),
                    kettler::readLe32(previous.constData() + 1),
                    kettler::readLe16(current.constData() + 5),
                    kettler::readLe16(previous.constData() + 5)),
                0.636, 0.01);
    EXPECT_NEAR(kettler::wheelSpeedKmh(1, 0xffffffffu, 512, 65024), 15.12, 0.0001);
    EXPECT_DOUBLE_EQ(kettler::wheelSpeedKmh(10, 10, 1024, 0), 0.0);
    EXPECT_DOUBLE_EQ(kettler::wheelSpeedKmh(10, 9, 1024, 1024), 0.0);
}
