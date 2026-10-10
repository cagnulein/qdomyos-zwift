#include "devices/sportsplusbike/carefitnessparser.h"

#include <gtest/gtest.h>

TEST(CareFitnessParser, DetectsAndParsesCv351Telemetry) {
    const QByteArray packet = QByteArray::fromHex("20300066008900226a0000cb");

    const carefitness::Protocol protocol = carefitness::detectProtocol(carefitness::Protocol::Unknown, packet);
    ASSERT_EQ(protocol, carefitness::Protocol::Cv351);

    const carefitness::Telemetry telemetry = carefitness::parsePacket(protocol, packet);
    EXPECT_FALSE(telemetry.hasSpeed);
    ASSERT_TRUE(telemetry.hasCadence);
    EXPECT_DOUBLE_EQ(telemetry.cadence, 66.0);
}

TEST(CareFitnessParser, DetectsAndParsesCv385SpeedAndCadenceFrames) {
    const QByteArray speedPacket = QByteArray::fromHex("20000131000100000000095c");
    const QByteArray cadencePacket = QByteArray::fromHex("20100035000100000000096f");

    const carefitness::Protocol protocol = carefitness::detectProtocol(carefitness::Protocol::Unknown, speedPacket);
    ASSERT_EQ(protocol, carefitness::Protocol::Cv385);

    const carefitness::Telemetry speed = carefitness::parsePacket(protocol, speedPacket);
    ASSERT_TRUE(speed.hasSpeed);
    EXPECT_NEAR(speed.speed, 3.05, 0.001);
    EXPECT_FALSE(speed.hasCadence);

    const carefitness::Telemetry cadence = carefitness::parsePacket(protocol, cadencePacket);
    EXPECT_FALSE(cadence.hasSpeed);
    ASSERT_TRUE(cadence.hasCadence);
    EXPECT_DOUBLE_EQ(cadence.cadence, 53.0);
}

TEST(CareFitnessParser, IgnoresUnknownFrameWithoutChangingProtocol) {
    const QByteArray unknownPacket = QByteArray::fromHex("200400000000000000000000");

    EXPECT_EQ(carefitness::detectProtocol(carefitness::Protocol::Unknown, unknownPacket),
              carefitness::Protocol::Unknown);
    EXPECT_EQ(carefitness::detectProtocol(carefitness::Protocol::Cv351, unknownPacket),
              carefitness::Protocol::Cv351);
    EXPECT_EQ(carefitness::detectProtocol(carefitness::Protocol::Cv385, unknownPacket),
              carefitness::Protocol::Cv385);
}
