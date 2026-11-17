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


TEST(FtmsBikeParserTest, PhysicalResistanceGearDetectionIsExplicitlyOptedInAndModelGated) {
    EXPECT_TRUE(ftmsbike::physicalResistanceGearsEnabled(QStringLiteral("H9110 BI9200"), true));
    EXPECT_TRUE(ftmsbike::physicalResistanceGearsEnabled(QStringLiteral("h9110 bi9200"), true));
    EXPECT_FALSE(ftmsbike::physicalResistanceGearsEnabled(QStringLiteral("H9110 BI9200"), false));
    for (const QString &name : {QStringLiteral("H9110 OSAKA"), QStringLiteral("H9110 BI9200 OTHER"),
                                QStringLiteral("Wahoo KICKR"), QStringLiteral("FS-IB50")}) {
        EXPECT_FALSE(ftmsbike::physicalResistanceGearsEnabled(name, true));
    }
}

TEST(FtmsBikeParserTest, PhysicalResistanceGearDeltaCoversRecordedNormalShifts) {
    const ftmsbike::PhysicalResistanceGearLimits limits{1, 32, 5, 3000};
    // Full BI9200 QZ-only recorded sequence 1->6->1.
    const int levels[] = {1, 2, 3, 4, 5, 6, 5, 4, 3, 2, 1};
    resistance_t delta = 99;
    for (int i = 1; i < 11; ++i) {
        ASSERT_TRUE(ftmsbike::physicalResistanceGearDelta(
            levels[i - 1], levels[i], -1, limits, &delta));
        EXPECT_EQ(i <= 5 ? 1 : -1, delta);
    }
}

TEST(FtmsBikeParserTest, PhysicalResistanceGearDeltaCoversRecordedFastShifts) {
    const ftmsbike::PhysicalResistanceGearLimits limits{1, 32, 5, 3000};
    // FTMS 0x2ACE from the BI9200 build 1684 log on 2026-11-17.
    const int transitions[][3] = {{23, 25, 2}, {32, 27, -5}, {26, 24, -2},
                                  {24, 21, -3}, {20, 18, -2}, {18, 16, -2}};
    resistance_t delta = 99;
    for (const auto &transition : transitions) {
        ASSERT_TRUE(ftmsbike::physicalResistanceGearDelta(
            transition[0], transition[1], -1, limits, &delta));
        EXPECT_EQ(transition[2], delta);
    }
}

TEST(FtmsBikeParserTest, PhysicalResistanceGearDeltaRejectsUnreliableChanges) {
    const ftmsbike::PhysicalResistanceGearLimits limits{1, 32, 5, 3000};
    resistance_t delta = 99;
    const auto expectIgnored = [&delta, &limits](resistance_t before, resistance_t after, qint64 since = -1) {
        EXPECT_FALSE(ftmsbike::physicalResistanceGearDelta(before, after, since, limits, &delta));
        EXPECT_EQ(0, delta);
    };
    expectIgnored(-1, 1);
    expectIgnored(0, 1);
    expectIgnored(1, 1);
    expectIgnored(1, 7); // Greater than the allowlisted device's maximum 5-step jump.
    expectIgnored(27, 20);
    expectIgnored(31, 33);
    expectIgnored(32, 33);
    expectIgnored(2, 3, 0);
    expectIgnored(2, 3, 2999);
    expectIgnored(23, 25, 100);
    expectIgnored(32, 27, 2999);
    EXPECT_TRUE(ftmsbike::physicalResistanceGearDelta(32, 27, 3000, limits, &delta));
    EXPECT_EQ(-5, delta);
    EXPECT_FALSE(ftmsbike::physicalResistanceGearDelta(2, 3, -1, limits, nullptr));
}

TEST(FtmsBikeParserTest, PhysicalResistanceGearDeltaHasReusablePerModelLimits) {
    const ftmsbike::PhysicalResistanceGearLimits futureProfile{1, 40, 2, 1500};
    resistance_t delta = 99;
    EXPECT_TRUE(ftmsbike::physicalResistanceGearDelta(32, 34, -1, futureProfile, &delta));
    EXPECT_EQ(2, delta);
    EXPECT_FALSE(ftmsbike::physicalResistanceGearDelta(32, 35, -1, futureProfile, &delta));
    EXPECT_EQ(0, delta);
    EXPECT_TRUE(ftmsbike::physicalResistanceGearDelta(32, 34, 1500, futureProfile, &delta));
    EXPECT_EQ(2, delta);
    EXPECT_FALSE(ftmsbike::physicalResistanceGearDelta(32, 34, 1499, futureProfile, &delta));
}

TEST(FtmsBikeParserTest, PhysicalResistanceGearReplayIsUnchangedWhenFeatureDisabled) {
    EXPECT_TRUE(ftmsbike::shouldReplayPhysicalGearChange(false, false));
    EXPECT_TRUE(ftmsbike::shouldReplayPhysicalGearChange(false, true));
    EXPECT_TRUE(ftmsbike::shouldReplayPhysicalGearChange(true, false));
    // Prevent the regression in 1683: native shift must not trigger a second FTMS write.
    EXPECT_FALSE(ftmsbike::shouldReplayPhysicalGearChange(true, true));
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

