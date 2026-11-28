#pragma once

#include <gtest/gtest.h>
#include <QString>

#include "devices/ftmsbike/ftmsbike.h"

TEST(FtmsBikeResistanceTest, MatrixR30XrUses16BitResistanceForAnyMxRbSuffix) {
    EXPECT_TRUE(ftmsbike::uses16BitResistanceForDevice(QStringLiteral("MX-RB 4D41")));
    EXPECT_TRUE(ftmsbike::uses16BitResistanceForDevice(QStringLiteral("mx-rb 4d41")));
    EXPECT_TRUE(ftmsbike::uses16BitResistanceForDevice(QStringLiteral("MX-RB 4D42")));
    EXPECT_TRUE(ftmsbike::uses16BitResistanceForDevice(QStringLiteral("MX-RB 4D4")));
    EXPECT_TRUE(ftmsbike::uses16BitResistanceForDevice(QStringLiteral("MX-RB 4D41 extra")));
    EXPECT_FALSE(ftmsbike::uses16BitResistanceForDevice(QStringLiteral("MX-TM 4D41")));
    EXPECT_FALSE(ftmsbike::uses16BitResistanceForDevice(QStringLiteral("Other 4D41")));
    EXPECT_FALSE(ftmsbike::uses16BitResistanceForDevice(QStringLiteral("MX-RBXR 4D41")));
}

TEST(FtmsBikeResistanceTest, MatrixR30XrOnlyAdvancesQueueForMatchingFtmsControlAck) {
    const QBluetoothUuid controlPoint((quint16)0x2AD9);
    const QBluetoothUuid indoorBikeData((quint16)0x2AD2);

    // The request-control reply may arrive while other FTMS notifications are
    // streaming. Only the matching control-point indication may unblock writes.
    EXPECT_TRUE(ftmsbike::isMatchingFtmsControlResponse(
        controlPoint, QByteArray::fromHex("800001"), FTMS_REQUEST_CONTROL));
    EXPECT_TRUE(ftmsbike::isMatchingFtmsControlResponse(
        controlPoint, QByteArray::fromHex("800401"), FTMS_SET_TARGET_RESISTANCE_LEVEL));
    EXPECT_TRUE(ftmsbike::isMatchingFtmsControlResponse(
        controlPoint, QByteArray::fromHex("800405"), FTMS_SET_TARGET_RESISTANCE_LEVEL));
    EXPECT_FALSE(ftmsbike::isMatchingFtmsControlResponse(
        controlPoint, QByteArray::fromHex("800701"), FTMS_REQUEST_CONTROL));
    EXPECT_FALSE(ftmsbike::isMatchingFtmsControlResponse(
        controlPoint, QByteArray::fromHex("800401"), FTMS_REQUEST_CONTROL));
    EXPECT_FALSE(ftmsbike::isMatchingFtmsControlResponse(
        indoorBikeData, QByteArray::fromHex("800001"), FTMS_REQUEST_CONTROL));
    EXPECT_FALSE(ftmsbike::isMatchingFtmsControlResponse(
        controlPoint, QByteArray::fromHex("8000"), FTMS_REQUEST_CONTROL));
}

TEST(FtmsBikeResistanceTest, NormalizesOnlyExactZBike20Name) {
    EXPECT_EQ(ftmsbike::normalizedResistanceForDevice(QStringLiteral("ZBike2.0"), 50), 0);
    EXPECT_EQ(ftmsbike::normalizedResistanceForDevice(QStringLiteral("ZBike2.0"), 60), 1);
    EXPECT_EQ(ftmsbike::normalizedResistanceForDevice(QStringLiteral("ZBike2.0"), 70), 2);
    EXPECT_EQ(ftmsbike::normalizedResistanceForDevice(QStringLiteral("ZBike2.0"), 100), 5);
    EXPECT_EQ(ftmsbike::normalizedResistanceForDevice(QStringLiteral("ZBike2.0"), 110), 6);
    EXPECT_EQ(ftmsbike::normalizedResistanceForDevice(QStringLiteral("ZBike2.0"), 120), 7);
    EXPECT_EQ(ftmsbike::normalizedResistanceForDevice(QStringLiteral("ZBike2.0"), 200), 15);
}

TEST(FtmsBikeResistanceTest, RoundsAndClampsZBike20Resistance) {
    EXPECT_EQ(ftmsbike::normalizedResistanceForDevice(QStringLiteral("ZBike2.0"), 98), 5);
    EXPECT_EQ(ftmsbike::normalizedResistanceForDevice(QStringLiteral("ZBike2.0"), 99), 5);
    EXPECT_EQ(ftmsbike::normalizedResistanceForDevice(QStringLiteral("ZBike2.0"), 101), 5);
    EXPECT_EQ(ftmsbike::normalizedResistanceForDevice(QStringLiteral("ZBike2.0"), 102), 5);
    EXPECT_EQ(ftmsbike::normalizedResistanceForDevice(QStringLiteral("ZBike2.0"), 0), 0);
    EXPECT_EQ(ftmsbike::normalizedResistanceForDevice(QStringLiteral("ZBike2.0"), 999), 15);
}

TEST(FtmsBikeResistanceTest, KeepsRawResistanceForOtherFtmsDeviceNames) {
    EXPECT_EQ(ftmsbike::normalizedResistanceForDevice(QStringLiteral("ZBike2.0 Pro"), 100), 100);
    EXPECT_EQ(ftmsbike::normalizedResistanceForDevice(QStringLiteral("Other FTMS Bike"), 100), 100);
}

TEST(FtmsBikeResistanceTest, ConvertsDisplayedZBike20ResistanceBackToRawOnlyWhenNeeded) {
    EXPECT_EQ(ftmsbike::rawResistanceForDevice(QStringLiteral("ZBike2.0"), 0), 50);
    EXPECT_EQ(ftmsbike::rawResistanceForDevice(QStringLiteral("ZBike2.0"), 5), 100);
    EXPECT_EQ(ftmsbike::rawResistanceForDevice(QStringLiteral("ZBike2.0"), 15), 200);
    EXPECT_EQ(ftmsbike::rawResistanceForDevice(QStringLiteral("ZBike2.0"), -1), -1);
    EXPECT_EQ(ftmsbike::rawResistanceForDevice(QStringLiteral("ZBike2.0 Pro"), 5), 5);
}