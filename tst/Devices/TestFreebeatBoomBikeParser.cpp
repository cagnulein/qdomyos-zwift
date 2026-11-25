#include "devices/freebeatboombike/freebeatboombike.h"

#include <gtest/gtest.h>
#include <QByteArray>

TEST(FreebeatBoomBikeParser, DecodesBoomBikeTelemetry) {
    const QByteArray packet = QByteArray::fromHex("550300000000000a500000003f");
    freebeatboombike::Telemetry telemetry;

    ASSERT_TRUE(freebeatboombike::parseTelemetry(packet, &telemetry));
    EXPECT_EQ(telemetry.rpm, 80);
    EXPECT_EQ(telemetry.resistance, 10);
    EXPECT_NEAR(telemetry.speed, 17.865, 0.01);
}

TEST(FreebeatBoomBikeParser, RejectsInvalidBoomBikeTelemetry) {
    freebeatboombike::Telemetry telemetry;

    EXPECT_FALSE(freebeatboombike::parseTelemetry(QByteArray::fromHex("5503"), &telemetry));
    EXPECT_FALSE(freebeatboombike::parseTelemetry(QByteArray::fromHex("550400000000000a500000003f"), &telemetry));
    EXPECT_FALSE(freebeatboombike::parseTelemetry(QByteArray::fromHex("550300000000000a500000003e"), &telemetry));
    EXPECT_FALSE(freebeatboombike::parseTelemetry(QByteArray::fromHex("550300000000000a500000003f"), nullptr));
}

TEST(FreebeatBoomBikeParser, BuildsProtocolCommandsFromDocumentedLayout) {
    EXPECT_EQ(freebeatboombike::queryCommand(), QByteArray::fromHex("25080008a0"));
    EXPECT_EQ(freebeatboombike::streamCommand(true), QByteArray::fromHex("25220123a0"));
    EXPECT_EQ(freebeatboombike::streamCommand(false), QByteArray::fromHex("25220022a0"));
    EXPECT_EQ(freebeatboombike::resistanceCommand(1), QByteArray::fromHex("25030104a0"));
    EXPECT_EQ(freebeatboombike::resistanceCommand(100), QByteArray::fromHex("25036467a0"));
    EXPECT_EQ(freebeatboombike::resistanceCommand(0), QByteArray::fromHex("25030104a0"));
}

TEST(FreebeatBoomBikeParser, CalculatesDistanceInKilometresFromSpeed) {
    EXPECT_NEAR(freebeatboombike::distanceIncrement(17.865, 3600000), 17.865, 0.001);
    EXPECT_NEAR(freebeatboombike::distanceIncrement(17.865, 1000), 17.865 / 3600.0, 0.000001);
}
