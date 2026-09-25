#include "TestFtmsBikeParser.h"

#include "devices/ftmsbike/ftmsbike.h"

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

TEST(FtmsBikeParserTest, DoesNotWriteResistanceForAnFsIb50PhysicalGearChange) {
    EXPECT_FALSE(ftmsbike::resistanceWriteRequired(true, true, -1, true));
}

TEST(FtmsBikeParserTest, KeepsExplicitResistanceRequestsWhenAnFsIb50GearChanges) {
    EXPECT_TRUE(ftmsbike::resistanceWriteRequired(true, true, 8, true));
}

TEST(FtmsBikeParserTest, WritesResistanceForNonFsIb50GearChanges) {
    EXPECT_TRUE(ftmsbike::resistanceWriteRequired(false, false, -1, true));
}
