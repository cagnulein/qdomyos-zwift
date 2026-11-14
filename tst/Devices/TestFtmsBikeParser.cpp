#include "TestFtmsBikeParser.h"

#include "devices/ftmsbike/ftmsbike.h"
#include "devices/daumbike/daumbike.h"

TEST(FtmsBikeParserTest, DecodesFsIb50Resistance) {
    resistance_t resistance = -1;
    const QByteArray packet = QByteArray::fromHex("02 42 02 47 07 1e 34 00 00 00 00 00 00 00 00");

    ASSERT_TRUE(ftmsbike::parseFsIb50Resistance(packet, &resistance));
    EXPECT_EQ(30, resistance);
}

TEST(FtmsBikeParserTest, DecodesMinimumAndMaximumObservedLevels) {
    resistance_t resistance = -1;

    EXPECT_TRUE(ftmsbike::parseFsIb50Resistance(
        QByteArray::fromHex("02 42 02 d6 07 01 36 00 00 00 00 00 00 00 00"), &resistance));
    EXPECT_EQ(1, resistance);

    EXPECT_TRUE(ftmsbike::parseFsIb50Resistance(
        QByteArray::fromHex("02 42 02 41 08 20 3b 00 00 00 00 00 00 00 00"), &resistance));
    EXPECT_EQ(32, resistance);
}

TEST(FtmsBikeParserTest, RejectsNonFsIb50Packets) {
    resistance_t resistance = -1;

    EXPECT_FALSE(ftmsbike::parseFsIb50Resistance(
        QByteArray::fromHex("02 42 02 47 07 1e 34 00 00 00 00 00 00 00"), &resistance));
    EXPECT_FALSE(ftmsbike::parseFsIb50Resistance(
        QByteArray::fromHex("02 44 05 41 03 00 00 00 00 00 00 00 00 00 00"), &resistance));
    EXPECT_FALSE(ftmsbike::parseFsIb50Resistance(
        QByteArray::fromHex("02 42 02 47 07 21 34 00 00 00 00 00 00 00 00"), &resistance));
    EXPECT_FALSE(ftmsbike::parseFsIb50Resistance(
        QByteArray::fromHex("02 42 02 47 07 1e 34 00 00 00 00 00 00 00 00"), nullptr));
}

TEST(FtmsBikeParserTest, IgnoresResistanceChangesDuringInclinationHysteresis) {
    resistance_t delta = 0;

    EXPECT_FALSE(ftmsbike::fsIb50ResistanceGearDelta(14, 24, 1999, -1, &delta));
    EXPECT_EQ(0, delta);
}

TEST(FtmsBikeParserTest, ConvertsResistanceChangesOutsideInclinationHysteresisToGearDelta) {
    resistance_t delta = 0;

    EXPECT_TRUE(ftmsbike::fsIb50ResistanceGearDelta(14, 24, 2000, -1, &delta));
    EXPECT_EQ(10, delta);
    EXPECT_TRUE(ftmsbike::fsIb50ResistanceGearDelta(24, 14, -1, -1, &delta));
    EXPECT_EQ(-10, delta);
}

TEST(FtmsBikeParserTest, DoesNotCreateGearDeltaFromAnUninitializedOrUnchangedBaseline) {
    resistance_t delta = 0;

    EXPECT_FALSE(ftmsbike::fsIb50ResistanceGearDelta(-1, 14, -1, -1, &delta));
    EXPECT_FALSE(ftmsbike::fsIb50ResistanceGearDelta(14, 14, -1, -1, &delta));
    EXPECT_FALSE(ftmsbike::fsIb50ResistanceGearDelta(14, 24, 2000, -1, nullptr));
}

TEST(FtmsBikeParserTest, IgnoresResistanceChangesWhileInclinationMetricIsActive) {
    resistance_t delta = 0;

    EXPECT_FALSE(ftmsbike::fsIb50ResistanceGearDelta(1, 3, 3553, 500, &delta));
    EXPECT_EQ(0, delta);
}

TEST(FtmsBikeParserTest, AppliesGearGainToFsIb50PhysicalGearDelta) {
    EXPECT_DOUBLE_EQ(0.05, ftmsbike::applyFsIb50GearGain(1, 0.05));
    EXPECT_DOUBLE_EQ(-0.2, ftmsbike::applyFsIb50GearGain(-4, 0.05));
}

TEST(FtmsBikeParserTest, KeepsFullFsIb50GearDeltaWithDefaultGain) {
    EXPECT_DOUBLE_EQ(5.0, ftmsbike::applyFsIb50GearGain(5, 1.0));
}

TEST(DaumBikeControlTest, AllowsZeroPowerFallbackWithConnectedVirtualFtmsBike) {
    EXPECT_TRUE(daumbike::shouldWriteResistanceToDaum(true, true, 0));
}

TEST(DaumBikeControlTest, KeepsOrdinaryPendingResistanceBlockedWithConnectedVirtualFtmsBike) {
    EXPECT_FALSE(daumbike::shouldWriteResistanceToDaum(true, true, -1));
}

TEST(DaumBikeControlTest, AllowsDaumResistanceWithoutVirtualFtmsOwnership) {
    EXPECT_TRUE(daumbike::shouldWriteResistanceToDaum(false, false, -1));
    EXPECT_TRUE(daumbike::shouldWriteResistanceToDaum(true, false, -1));
}


TEST(FtmsBikeParserTest, DecodesCycplusR200NativeGearFromRecordedNotification) {
    // R200 31260, CYCPLUS BC2 23402, 2026-11-14 10:40:18 (62 bytes).
    const QByteArray packet = QByteArray::fromHex(
        "fe ef ff ee 20 36 43 59 43 50 4c 55 53 20 42 43 32 20 32 33 34 30 32 00 "
        "be 01 01 09 00 00 43 59 43 50 4c 55 53 20 42 43 32 20 32 33 34 30 32 "
        "00 be 01 01 09 00 00 f2 ff 07 04 00 4e 0c 3c");

    qint16 gear = 0;
    ASSERT_EQ(62, packet.size());
    ASSERT_TRUE(ftmsbike::parseCycplusNativeGear(packet, &gear));
    EXPECT_EQ(-14, gear);
}

TEST(FtmsBikeParserTest, DecodesCycplusR200NativeGearAndReset) {
    QByteArray packet(62, '\0');
    qint16 gear = 42;

    packet[54] = static_cast<char>(0xe7);
    packet[55] = static_cast<char>(0xff);
    ASSERT_TRUE(ftmsbike::parseCycplusNativeGear(packet, &gear));
    EXPECT_EQ(-25, gear);

    packet[54] = 0;
    packet[55] = 0;
    ASSERT_TRUE(ftmsbike::parseCycplusNativeGear(packet, &gear));
    EXPECT_EQ(0, gear);

    packet[54] = 1;
    ASSERT_TRUE(ftmsbike::parseCycplusNativeGear(packet, &gear));
    EXPECT_EQ(1, gear);
}

TEST(FtmsBikeParserTest, RejectsMalformedCycplusGearPackets) {
    qint16 gear = 42;
    EXPECT_FALSE(ftmsbike::parseCycplusNativeGear(QByteArray(61, '\0'), &gear));
    EXPECT_FALSE(ftmsbike::parseCycplusNativeGear(QByteArray(63, '\0'), &gear));
    EXPECT_FALSE(ftmsbike::parseCycplusNativeGear(QByteArray(62, '\0'), nullptr));
    EXPECT_EQ(42, gear);
}
