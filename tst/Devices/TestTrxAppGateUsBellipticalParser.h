#pragma once

#include <gtest/gtest.h>
#include <QByteArray>

#include "devices/trxappgateusbelliptical/trxappgateusbelliptical.h"

TEST(TrxAppGateUsBellipticalParserTest, ToorxSupportsResistanceLevelsThrough32) {
    EXPECT_FALSE(trxappgateusbelliptical::toorxResistanceInRange(0));
    EXPECT_TRUE(trxappgateusbelliptical::toorxResistanceInRange(1));
    EXPECT_TRUE(trxappgateusbelliptical::toorxResistanceInRange(32));
    EXPECT_FALSE(trxappgateusbelliptical::toorxResistanceInRange(33));
}

TEST(TrxAppGateUsBellipticalParserTest, ToorxUsesCadenceFieldFromPacket) {
    const QByteArray packet = QByteArray::fromHex("f0b223010121011401130101010101010145020261");

    EXPECT_DOUBLE_EQ(trxappgateusbelliptical::toorxCadenceFromPacket(packet), 18.0);
}

TEST(TrxAppGateUsBellipticalParserTest, ToorxUsesSpeedFieldFromPacket) {
    const QByteArray packet = QByteArray::fromHex("f0b223010121011401130101010101010145020261");

    EXPECT_DOUBLE_EQ(trxappgateusbelliptical::toorxSpeedFromPacket(packet), 1.9);
}

// Captured from Stefan's MAXXUS CX 6.1 / FAL-SPORTS0187 debug logs on 21-22 Oct 2026.
// Time bytes (4-5) change while telemetry bytes (6-17) remain zero.
TEST(TrxAppGateUsBellipticalParserTest, FalSports0187DoesNotMistakeTimerForCadence) {
    const QByteArray first = QByteArray::fromHex("f0b2350111010101010101010101010101010202fa");
    const QByteArray second = QByteArray::fromHex("f0b235010b3a01010101010101010101010102022d");
    ASSERT_EQ(first.size(), 21);
    ASSERT_EQ(second.size(), 21);

    EXPECT_DOUBLE_EQ(trxappgateusbelliptical::toorxCadenceFromPacket(first), 0.0);
    EXPECT_DOUBLE_EQ(trxappgateusbelliptical::toorxCadenceFromPacket(second), 0.0);
    EXPECT_DOUBLE_EQ(trxappgateusbelliptical::toorxSpeedFromPacket(first), 0.0);
    EXPECT_DOUBLE_EQ(trxappgateusbelliptical::toorxSpeedFromPacket(second), 0.0);
    EXPECT_DOUBLE_EQ(trxappgateusbelliptical::iConsoleWattsFromPacket(first), 0.0);
    EXPECT_DOUBLE_EQ(trxappgateusbelliptical::iConsoleWattsFromPacket(second), 0.0);
    EXPECT_EQ(static_cast<uint8_t>(first.at(18)) - 1, 1);
}

TEST(TrxAppGateUsBellipticalParserTest, IConsoleWattsUsesUnsignedBytes) {
    QByteArray packet(21, char(0x01));
    packet[16] = char(0x81);  // (129 - 1) * 100 = 12800 tenths of a watt
    packet[17] = char(0x01);
    EXPECT_DOUBLE_EQ(trxappgateusbelliptical::iConsoleWattsFromPacket(packet), 1280.0);
}
