#pragma once

#include <gtest/gtest.h>
#include <QByteArray>
#include <QString>

#include "devices/keepbike/keepbike.h"

TEST(KeepBikeNewProtocolRegressionTest, LoggedMetricsFrameHasValidEnvelopeAndCrc) {
    const QByteArray packet = QByteArray::fromHex(
        "a5a5a01c2600ef2332165545c2debc04000001b53130362f37ff08e1abf6d006101118032801303a381740033ae1");

    EXPECT_TRUE(keepbike::isNewProtocolFrame(packet));
    EXPECT_EQ(keepbike::newProtocolCrc(packet.left(packet.length() - 2)), 0xe13a);
}

TEST(KeepBikeNewProtocolRegressionTest, MetricsComeFromLoggedProtobufPayload) {
    const QByteArray packet = QByteArray::fromHex(
        "a5a5a01c2600ef2332165545c2debc04000001b53130362f37ff08e1abf6d006101118032801303a381740033ae1");

    const keepbike::NewProtocolMetrics metrics = keepbike::parseNewProtocolMetricsFrame(packet);

    EXPECT_TRUE(metrics.valid);
    EXPECT_EQ(metrics.timestamp, 1780323809u);
    EXPECT_EQ(metrics.elapsed, 17);
    EXPECT_EQ(metrics.resistance, 1);
    EXPECT_EQ(metrics.cadence, 58);
    EXPECT_EQ(metrics.watt, 23);
    EXPECT_EQ(metrics.status, 3);
}

TEST(KeepBikeNewProtocolRegressionTest, ResistanceComesFromLoggedStatusFrame) {
    const QByteArray resistanceFourPacket = QByteArray::fromHex(
        "a5a5a0421600ef2332165545d88f6500000004b53130362f34ff08048272");
    const QByteArray resistanceOnePacket = QByteArray::fromHex(
        "a5a5a0441600ef2332165545d8906500000004b53130362f34ff0801e6be");
    const QByteArray commandStatusPacket = QByteArray::fromHex(
        "a5a5a02e2100ef2332165583fd43b48d000002b53130362f34ff080212094e4f20535441545553767f");

    EXPECT_EQ(keepbike::parseNewProtocolResistanceFrame(resistanceFourPacket), 4);
    EXPECT_EQ(keepbike::parseNewProtocolResistanceFrame(resistanceOnePacket), 1);
    EXPECT_EQ(keepbike::parseNewProtocolResistanceFrame(commandStatusPacket), 2);
}

TEST(KeepBikeNewProtocolRegressionTest, BuildsResistanceWritePayload) {
    EXPECT_EQ(keepbike::buildNewProtocolResistancePayload(4),
              QByteArray::fromHex("01b53130362f37ff20a01f"));
    EXPECT_EQ(keepbike::buildNewProtocolResistancePayload(12),
              QByteArray::fromHex("01b53130362f37ff20e05d"));
}

TEST(KeepBikeNewProtocolRegressionTest, RejectsCorruptMetricsFrame) {
    QByteArray packet = QByteArray::fromHex(
        "a5a5a01c2600ef2332165545c2debc04000001b53130362f37ff08e1abf6d006101118032801303a381740033ae1");
    packet[packet.length() - 1] = static_cast<char>(0x00);

    EXPECT_FALSE(keepbike::isNewProtocolFrame(packet));
    EXPECT_FALSE(keepbike::parseNewProtocolMetricsFrame(packet).valid);
}

TEST(KeepBikeNewProtocolRegressionTest, BuildsFramesWithValidNewProtocolCrc) {
    const QByteArray frame =
        keepbike::buildNewProtocolFrame(0x00a0, 0x04c4c2, QByteArray::fromHex("01b53130362f37"));

    EXPECT_TRUE(keepbike::isNewProtocolFrame(frame));
    EXPECT_TRUE(frame.startsWith(QByteArray::fromHex("a5a5a00013003216ef235501c2c4040000")));
}

TEST(KeepBikeNewProtocolRegressionTest, ResistanceWriteFrameUsesMetricsCommandType) {
    const QByteArray frame = keepbike::buildNewProtocolFrame(
        0x36a0, 0x04d7f9c2, keepbike::buildNewProtocolResistancePayload(4));

    EXPECT_TRUE(keepbike::isNewProtocolFrame(frame));
    EXPECT_TRUE(frame.startsWith(QByteArray::fromHex("a5a5a03617003216ef235501c2f9d7040000")));
    EXPECT_TRUE(frame.contains(QByteArray::fromHex("01b53130362f37ff20a01f")));
}
