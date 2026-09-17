#pragma once

#include <gtest/gtest.h>
#include <QByteArray>

#include "devices/nautilustreadmill/nautilustreadmill.h"

TEST(NautilusTreadmillParserTest, DecodesSchwinnOmniSpeedInKilometersPerHour) {
    const QByteArray packet = QByteArray::fromHex("00 00 61 b0 04 62 00 63 7d 00 64 00");

    EXPECT_NEAR(nautilustreadmill::speedFromPacket(packet), 19.31208, 0.00001);
}

TEST(NautilusTreadmillParserTest, DecodesSchwinnOmniInclination) {
    const QByteArray packet = QByteArray::fromHex("00 00 61 64 00 62 0f 63 7d 00 64 00");

    EXPECT_EQ(nautilustreadmill::inclinationFromPacket(packet), 15.0);
}

TEST(NautilusTreadmillParserTest, RejectsShortSchwinnOmniPackets) {
    const QByteArray packet = QByteArray::fromHex("00 00 61 64 00");

    EXPECT_EQ(nautilustreadmill::speedFromPacket(packet), 0.0);
    EXPECT_EQ(nautilustreadmill::inclinationFromPacket(packet), 0.0);
}
