#include <gtest/gtest.h>

#include <QByteArray>
#include <QHostAddress>

#include "devices/bkoolbike/bkoolcadenceparser.h"

using namespace BkoolCadence;

TEST(BkoolCadence, CombinedWheelAndEmptyCrankDoesNotCreateStartupSpike) {
    // Real Smart Bike v1 trace (Nov 18, 2026): previously interpreted as
    // 525.128 rpm because wheel revolutions were divided by a nonzero timer.
    const QByteArray first = QByteArray::fromHex("3000100023000000fe1f00000000");
    const QByteArray second = QByteArray::fromHex("3000100025000000fe2f00000000");
    const PowerSample a = parsePowerSample(first, 0x30, 4);
    const PowerSample b = parsePowerSample(second, 0x30, 4);
    ASSERT_TRUE(a.hasPublicationCounter);
    ASSERT_TRUE(a.emptyCrankFields);
    EXPECT_FALSE(a.hasCadenceCounter);
    EXPECT_EQ(a.publicationCounter.revolutions, 35u);
    EXPECT_EQ(a.publicationCounter.eventTime, 0x1ffe);
    EXPECT_TRUE(b.emptyCrankFields);
    EXPECT_FALSE(b.hasCadenceCounter);
    EXPECT_EQ(b.publicationCounter.revolutions, 37u);
}

TEST(BkoolCadence, CrankFieldsOffsetAfterWheelIsCorrect) {
    // Both flags with real crank counts: use the bytes after the 6-byte
    // wheel block, not the wheel timestamp at offset 8.
    const PowerSample a =
        parsePowerSample(QByteArray::fromHex("3000100023000000fe1f64000010"), 0x30, 4);
    const PowerSample b =
        parsePowerSample(QByteArray::fromHex("3000100025000000fe2f65000014"), 0x30, 4);
    ASSERT_TRUE(a.hasCadenceCounter);
    ASSERT_TRUE(b.hasCadenceCounter);
    EXPECT_FALSE(a.cadenceCounter.isWheel);
    EXPECT_EQ(a.cadenceCounter.revolutions, 100u);
    EXPECT_EQ(a.cadenceCounter.eventTime, 0x1000);
    EXPECT_DOUBLE_EQ(cadenceFromCounters(a.cadenceCounter, b.cadenceCounter), 60.0);
}

TEST(BkoolCadence, WheelOnlyFallbackAndRolloverRemainSupported) {
    const PowerSample a =
        parsePowerSample(QByteArray::fromHex("1000100023000000fe1f"), 0x10, 4);
    const PowerSample b =
        parsePowerSample(QByteArray::fromHex("1000100025000000fe2f"), 0x10, 4);
    ASSERT_TRUE(a.hasCadenceCounter);
    ASSERT_TRUE(b.hasCadenceCounter);
    EXPECT_TRUE(a.cadenceCounter.isWheel);
    EXPECT_DOUBLE_EQ(cadenceFromCounters(a.cadenceCounter, b.cadenceCounter), 60.0);

    Counter before;
    before.revolutions = 99;
    before.eventTime = 65500;
    Counter after;
    after.revolutions = 100;
    after.eventTime = 988; // 16-bit timestamp wrapped: delta is 1024
    EXPECT_DOUBLE_EQ(cadenceFromCounters(before, after), 60.0);
    after.isWheel = true;
    EXPECT_LT(cadenceFromCounters(before, after), 0.0);
}

TEST(BkoolCadence, RejectsMissingOrShortFieldsWithoutOutOfBoundsReads) {
    EXPECT_FALSE(parsePowerSample(QByteArray::fromHex("3000100023000000fe1f"), 0x30, 4)
                     .hasCadenceCounter);
    EXPECT_FALSE(parsePowerSample(QByteArray::fromHex("2000100064"), 0x20, 4)
                     .hasPublicationCounter);
    EXPECT_FALSE(parsePowerSample(QByteArray(), 0x30, 4).hasCadenceCounter);
}

TEST(BkoolCadence, RejectsImplausibleProprietarySpikeWithoutChangingScale) {
    EXPECT_NEAR(parseCustomCadence(QByteArray::fromHex("0400390b10")),
                2873.0 / 36.0, 0.0001);
    // Real incoming packet: 0x1d70 / 36 = 209.33 rpm, implausible in this ride.
    EXPECT_LT(parseCustomCadence(QByteArray::fromHex("0400701d13")), 0.0);
    EXPECT_DOUBLE_EQ(parseCustomCadence(QByteArray::fromHex("0400000013")), 0.0);
    EXPECT_LT(parseCustomCadence(QByteArray::fromHex("04000000")), 0.0);
}
