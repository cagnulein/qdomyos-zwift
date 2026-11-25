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
    const auto expectIgnored = [&delta, &limits](resistance_t before, resistance_t after, qint64 since) {
        EXPECT_FALSE(ftmsbike::physicalResistanceGearDelta(before, after, since, limits, &delta));
        EXPECT_EQ(0, delta);
    };
    expectIgnored(-1, 1, -1);
    expectIgnored(0, 1, -1);
    expectIgnored(1, 1, -1);
    expectIgnored(1, 7, -1); // Greater than the allowlisted device's maximum 5-step jump.
    expectIgnored(27, 20, -1);
    expectIgnored(31, 33, -1);
    expectIgnored(32, 33, -1);
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

TEST(FtmsBikeParserTest, PhysicalResistanceTargetPacketUsesNativeFtmsTenths) {
    EXPECT_EQ(QByteArray::fromHex("040a00"),
              ftmsbike::ftmsResistanceTargetPacket(1, 1, 32));
    EXPECT_EQ(QByteArray::fromHex("043c00"),
              ftmsbike::ftmsResistanceTargetPacket(6, 1, 32));
    EXPECT_EQ(QByteArray::fromHex("046e00"),
              ftmsbike::ftmsResistanceTargetPacket(11, 1, 32));
    EXPECT_EQ(QByteArray::fromHex("044001"),
              ftmsbike::ftmsResistanceTargetPacket(32, 1, 32));
    EXPECT_EQ(QByteArray::fromHex("044001"),
              ftmsbike::ftmsResistanceTargetPacket(55, 1, 32));
    EXPECT_TRUE(ftmsbike::ftmsResistanceTargetPacket(8, 20, 1).isEmpty());
}

TEST(FtmsBikeParserTest, PhysicalResistanceCommandEchoTracksTargetInsteadOfBlockingAllShifts) {
    // While a 1 -> 6 FTMS command is being applied, intermediate values
    // must not become spurious virtual gears.
    EXPECT_TRUE(ftmsbike::isExpectedResistanceCommandEcho(3, 1, 6, 300, 3000));
    EXPECT_TRUE(ftmsbike::isExpectedResistanceCommandEcho(6, 1, 6, 600, 3000));
    EXPECT_FALSE(ftmsbike::isExpectedResistanceCommandEcho(7, 1, 6, 600, 3000));
    EXPECT_FALSE(ftmsbike::isExpectedResistanceCommandEcho(5, 1, 6, 3000, 3000));
    EXPECT_TRUE(ftmsbike::isExpectedResistanceCommandEcho(7, 10, 6, 600, 3000));
    EXPECT_FALSE(ftmsbike::isExpectedResistanceCommandEcho(11, 10, 6, 600, 3000));
    EXPECT_FALSE(ftmsbike::isExpectedResistanceCommandEcho(11, 10, -1, 600, 3000));
    // A 6 -> 7 hardware press while command 6 -> 6 is pending is NOT an echo.
    EXPECT_FALSE(ftmsbike::isExpectedResistanceCommandEcho(7, 6, 6, 200, 3000));
}

TEST(FtmsBikeParserTest, PhysicalResistanceTargetAvoidsZwiftEchoLoop) {
    EXPECT_TRUE(ftmsbike::shouldWriteResistanceTarget(6, 5, 5));
    // Zwift repeats the same grade every second.
    EXPECT_FALSE(ftmsbike::shouldWriteResistanceTarget(6, 6, 5));
    // Hardware physical shift already reached the requested target.
    EXPECT_FALSE(ftmsbike::shouldWriteResistanceTarget(7, 6, 7));
    EXPECT_TRUE(ftmsbike::shouldWriteResistanceTarget(8, 7, 7));
}

TEST(FtmsBikeParserTest, PhysicalResistanceTargetDoesNotApplyModifierTwice) {
    // Before this fix, bike::changeResistance() produced 6 but update()
    // added gearsModifier()*5 again, yielding 11 on the BI9200.
    EXPECT_EQ(6, ftmsbike::effectiveRequestedResistance(6, 5, true));
    // With the feature disabled, preserve existing FTMS behavior.
    EXPECT_EQ(11, ftmsbike::effectiveRequestedResistance(6, 5, false));
    // Legacy QZ virtual-gear-only request remains unchanged.
    EXPECT_EQ(9, ftmsbike::effectiveRequestedResistance(-1, 10, false));
}

TEST(FtmsBikeParserTest, PhysicalResistanceCommandHistorySuppressesDelayedEchoes) {
    // Real BI9200 build 1707, Nov 25 11:43:13-18: older FTMS responses
    // arrived after newer targets and created false +5/-2 virtual shifts.
    using Command = ftmsbike::PhysicalResistanceCommand;
    QList<Command> history;
    const qint64 start = 1795603393002LL;
    history.append(Command{15, 16, start});
    history.append(Command{15, 21, start + 2000});
    // Even if the latest command targeted 19, the 20 reported by the bike
    // can be a delayed echo of the earlier 15->21 write.
    history.append(Command{20, 19, start + 3000});
    EXPECT_TRUE(ftmsbike::matchesRecentResistanceCommand(20, history, start + 3500, 4000));
    EXPECT_TRUE(ftmsbike::matchesRecentResistanceCommand(18, history, start + 3500, 4000));
    history.append(Command{18, 24, start + 4000});
    EXPECT_TRUE(ftmsbike::matchesRecentResistanceCommand(23, history, start + 4600, 4000));
    EXPECT_TRUE(ftmsbike::matchesRecentResistanceCommand(21, history, start + 4700, 4000));
    // An event outside all commanded trajectories is a possible handlebar shift.
    EXPECT_FALSE(ftmsbike::matchesRecentResistanceCommand(27, history, start + 4700, 4000));
    // Old responses must eventually expire, allowing physical changes again.
    EXPECT_FALSE(ftmsbike::matchesRecentResistanceCommand(20, history, start + 9000, 4000));
}

TEST(FtmsBikeParserTest, PhysicalResistanceCommandHistoryDoesNotHideStableHandlebarPresses) {
    using Command = ftmsbike::PhysicalResistanceCommand;
    const QList<Command> history{Command{6, 6, 1000}};
    EXPECT_TRUE(ftmsbike::matchesRecentResistanceCommand(6, history, 1600, 4000));
    EXPECT_FALSE(ftmsbike::matchesRecentResistanceCommand(7, history, 1600, 4000));
    EXPECT_FALSE(ftmsbike::matchesRecentResistanceCommand(5, history, 1600, 4000));
    // A native 6->7 step still passes the existing pure gear classifier.
    const ftmsbike::PhysicalResistanceGearLimits limits{1, 32, 5, 3000};
    resistance_t delta = 0;
    EXPECT_TRUE(ftmsbike::physicalResistanceGearDelta(6, 7, -1, limits, &delta));
    EXPECT_EQ(1, delta);
}

TEST(FtmsBikeParserTest, PhysicalResistanceSettlingGuardsMotorOvershoot) {
    // Build 1707: a native 4->6 reply after a command 4->5 was not within
    // the nominal trajectory, but was still a motor response at 1.5 s.
    EXPECT_TRUE(ftmsbike::isPhysicalResistanceCommandSettling(1500, 2000));
    EXPECT_TRUE(ftmsbike::isPhysicalResistanceCommandSettling(1999, 2000));
    EXPECT_FALSE(ftmsbike::isPhysicalResistanceCommandSettling(2000, 2000));
    EXPECT_FALSE(ftmsbike::isPhysicalResistanceCommandSettling(-1, 2000));
    // No write recently: native +/- shifts remain available.
    EXPECT_FALSE(ftmsbike::isPhysicalResistanceCommandSettling(5000, 2000));
}

TEST(FtmsBikeParserTest, PhysicalResistanceGradeChangesAreRelativeToManualPosition) {
    // Alejandro's 2% -> 5% example: start at resistance 1, not absolute 24.
    const resistance_t grade2Base = 2;
    const resistance_t grade5Base = 5;
    const resistance_t relative = ftmsbike::relativeResistanceTarget(1, grade2Base, grade5Base);
    EXPECT_EQ(4, relative);
    EXPECT_EQ(4, ftmsbike::relativeResistanceTarget(relative, grade5Base, grade5Base));
    // The rider shifts up twice while Zwift's grade is fixed.
    const resistance_t shifted = relative + 2;
    EXPECT_EQ(7, ftmsbike::relativeResistanceTarget(shifted, 5, 6));
    EXPECT_EQ(1, ftmsbike::relativeResistanceTarget(1, 6, 0));
    EXPECT_EQ(32, ftmsbike::relativeResistanceTarget(31, 1, 20));
}

TEST(FtmsBikeParserTest, PhysicalResistanceShiftReanchorsToNativeWithoutReplayingIt) {
    // A target of 5 may settle at native 6; the next real 6->7 shift must
    // remain level 7, not be overwritten with desired=6.
    EXPECT_EQ(7, ftmsbike::reanchorResistanceTarget(7, 5, 5));
    // Keep the remaining 12-level gradient ramp while accepting a native shift.
    EXPECT_EQ(26, ftmsbike::reanchorResistanceTarget(14, 24, 12));
    EXPECT_EQ(14, ftmsbike::reanchorResistanceTarget(14, -1, -1));
    EXPECT_EQ(32, ftmsbike::reanchorResistanceTarget(32, 31, 11));
}

TEST(FtmsBikeParserTest, PhysicalResistanceGradientRampIsBoundedInBothDirections) {
    EXPECT_EQ(3, ftmsbike::limitedResistanceTarget(1, 24, 2));
    EXPECT_EQ(5, ftmsbike::limitedResistanceTarget(3, 24, 2));
    EXPECT_EQ(22, ftmsbike::limitedResistanceTarget(24, 1, 2));
    EXPECT_EQ(11, ftmsbike::limitedResistanceTarget(10, 11, 2));
    EXPECT_EQ(10, ftmsbike::limitedResistanceTarget(10, 10, 2));
    EXPECT_EQ(10, ftmsbike::limitedResistanceTarget(10, 32, 0));
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

