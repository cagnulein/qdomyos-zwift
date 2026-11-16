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


TEST(FtmsBikeParserTest, BhBi9200PhysicalShiftsMatchRecordedResistanceSequence) {
    // 0x2ACE resistance values from the 2026-11-16 BI9200 QZ-only test:
    // 81 00 00 01 00 -> 02 -> 03 -> 04 -> 05 -> 06 -> 05 -> 04 -> 03 -> 02 -> 01.
    const int levels[] = {1, 2, 3, 4, 5, 6, 5, 4, 3, 2, 1};
    resistance_t delta = 99;
    for (int i = 1; i < 11; ++i) {
        ASSERT_TRUE(ftmsbike::bhBi9200PhysicalGearDelta(
            QStringLiteral("H9110 BI9200"), true, levels[i - 1], levels[i], -1, &delta));
        EXPECT_EQ(i <= 5 ? 1 : -1, delta);
    }
}

TEST(FtmsBikeParserTest, BhBi9200DoesNotChangeGearsOnEchoOrWithoutOptIn) {
    resistance_t delta = 99;
    EXPECT_FALSE(ftmsbike::bhBi9200PhysicalGearDelta(
        QStringLiteral("H9110 BI9200"), true, 2, 3, 0, &delta));
    EXPECT_EQ(0, delta);
    EXPECT_FALSE(ftmsbike::bhBi9200PhysicalGearDelta(
        QStringLiteral("H9110 BI9200"), true, 2, 3, 2999, &delta));
    EXPECT_EQ(0, delta);
    EXPECT_TRUE(ftmsbike::bhBi9200PhysicalGearDelta(
        QStringLiteral("H9110 BI9200"), true, 2, 3, 3000, &delta));
    EXPECT_EQ(1, delta);
    EXPECT_FALSE(ftmsbike::bhBi9200PhysicalGearDelta(
        QStringLiteral("H9110 BI9200"), false, 2, 3, -1, &delta));
    EXPECT_EQ(0, delta);
}

TEST(FtmsBikeParserTest, BhBi9200PhysicalShiftClassifierIsIsolatedFromOtherBikes) {
    resistance_t delta = 99;
    for (const QString &name : {QStringLiteral("H9110 OSAKA"), QStringLiteral("H9110 BI9200 OTHER"),
                                QStringLiteral("Wahoo KICKR"), QStringLiteral("FS-IB50")}) {
        EXPECT_FALSE(ftmsbike::bhBi9200PhysicalGearDelta(name, true, 2, 3, -1, &delta));
        EXPECT_EQ(0, delta);
    }
    EXPECT_TRUE(ftmsbike::bhBi9200PhysicalGearDelta(
        QStringLiteral("h9110 bi9200"), true, 2, 3, -1, &delta));
    EXPECT_EQ(1, delta);
}

TEST(FtmsBikeParserTest, BhBi9200RejectsUninitializedAndNonButtonResistanceChanges) {
    resistance_t delta = 99;
    const auto expectIgnored = [&delta](resistance_t before, resistance_t after) {
        EXPECT_FALSE(ftmsbike::bhBi9200PhysicalGearDelta(
            QStringLiteral("H9110 BI9200"), true, before, after, -1, &delta));
        EXPECT_EQ(0, delta);
    };
    expectIgnored(-1, 1);
    expectIgnored(0, 1);
    expectIgnored(1, 1);
    expectIgnored(6, 3);
    expectIgnored(1, 6);
    expectIgnored(31, 33);
    expectIgnored(32, 33);
    EXPECT_FALSE(ftmsbike::bhBi9200PhysicalGearDelta(
        QStringLiteral("H9110 BI9200"), true, 2, 3, -1, nullptr));
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

