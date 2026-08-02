#pragma once

#include <gtest/gtest.h>

#include <QByteArray>

#include "devices/ftmsrower/ftmsrower.h"

#include <cmath>
#include <vector>

class FtmsRowerPm5RegressionTest : public testing::Test {
protected:
    struct PacketSample {
        qint64 timestampMs;
        QByteArray payload;
    };

    static QByteArray fromHex(const char *hex) {
        return QByteArray::fromHex(hex);
    }

    static std::vector<PacketSample> concept2NoFtmsPacketsFromUserLog() {
        // Extracted from debug-Mon_May_4_14_48_43_2026.log
        return {
            {1777931339337LL, fromHex("1904002f0100010101010200000000000080ac")},
            {1777931339410LL, fromHex("280400320916ff25533641000000000000")},
            {1777931340498LL, fromHex("9404004d0100010101010400000000000080ad")},
            {1777931340517LL, fromHex("940400230917ffab539b44000000000000")},
        };
    }

    static std::vector<PacketSample> concept2NoFtmsIdlePacketsFromUserLog() {
        // Extracted from debug-Mon_May_4_14_48_43_2026.log after rowing stops
        return {
            {1777931511952LL, fromHex("0b2d002d0c00010101000100000000000080af")},
            {1777931511973LL, fromHex("0b2d007e0a17ffe4484548000000000000")},
        };
    }

    static PacketSample ftmsDistancePacketFromIssue3872Log() {
        // Extracted from debug-Fri_Nov_14_06_40_47_2025.log (#3872)
        return {1763131313296LL, fromHex("ff0a00030000c40000002e0000000000000100")};
    }

    static PacketSample concept2ZeroDistancePacket31FromIssue3872Log() {
        return {1763131313964LL, fromHex("00000000000000010000000000000000008000")};
    }

    static PacketSample concept2ZeroDistancePacket32FromIssue3872Log() {
        return {1763131313972LL, fromHex("000000000000ff00000000000000000000")};
    }

    // Extracted from debug-Sun_Sep_14_19_26_19_2025.log (#3686, "Metrics at 0 with
    // Concept 2 PM5 Rowerg"). This PM5 has no FTMS service and reported the exact same
    // CE060031/CE060032 payload (RowState idle, byte9 == 0) for roughly 48 seconds after
    // connecting, which is what the reporter saw as "metrics stuck at 0". Distance must stay
    // pinned at the value derived from that first packet the whole time, and speed/cadence must
    // stay at 0 while idle.
    static std::vector<PacketSample> concept2NoFtmsIdleBurstFromIssue3686Log() {
        return {
            {1757870782324LL, fromHex("2e0d00df0100010101000100000000000080e3")}, // 0031, RowState=0
            {1757870782391LL, fromHex("2e0d008c034eff21d7177e0000000000")},       // 0032
            {1757870796894LL, fromHex("2e0d00df0100010101000100000000000080e3")}, // 0031, RowState=0
            {1757870796897LL, fromHex("2e0d008c034eff21d7177e0000000000")},       // 0032
            {1757870803083LL, fromHex("2e0d00df0100010101000100000000000080e3")}, // 0031, RowState=0
            {1757870803086LL, fromHex("2e0d008c034eff21d7177e0000000000")},       // 0032
        };
    }

    // Extracted from the same #3686 log a few packets after the idle burst above, once the
    // rower actually starts moving: RowState (byte9 of CE060031) flips from 0 to 1 and the
    // reported distance advances from 0x0001df (479 dm) to 0x0001e1 (481 dm).
    static std::vector<PacketSample> concept2NoFtmsActiveTransitionFromIssue3686Log() {
        return {
            {1757870830394LL, fromHex("4e0d00e10100010101010200000000000080e3")}, // 0031, RowState=1
            {1757870830402LL, fromHex("4e0d004603a5ff2ae9a9890000000000")},      // 0032
        };
    }

    // Real bytes from debug-Fri_Nov_14_06_40_47_2025.log (#3872), a PM5 that exposes BOTH the
    // Concept2-native characteristics and FTMS. None of the collected logs happen to contain a
    // continuous capture of a transient single-packet cadence dip mid-row (spm briefly reporting 0
    // while RowState stays "rowing"), so this composes three genuinely real payloads from the same
    // session at chosen offsets to reproduce that topology: a real CE060032 packet with spm=22
    // (from t=1763131316068 in the log), a real CE060032 packet with spm=0 (the same all-zero
    // payload the log emits before/after active rowing), and a real FTMS 0x2AD1 payload with
    // moreData=0 (from t=1763131261903 in the log, reused here since it is a genuine "no more
    // data, cadence byte present" frame).
    static PacketSample concept2CadenceNonZeroPacketFromIssue3872Log() {
        return {1763131316068LL, fromHex("9e0100580a16fff349774a000000000000")}; // 0032, spm=22
    }

    static PacketSample concept2CadenceTransientZeroPacketFromIssue3872Log() {
        return {0LL, fromHex("000000000000ff00000000000000000000")}; // 0032, spm=0 (real all-zero payload)
    }

    static PacketSample ftmsMoreDataZeroPacketFromIssue3872Log() {
        // FTMS 0x2AD1, moreData=0, cadence byte (index 2) == 44 (nonzero), so this packet only
        // reports cadence == 0 if the staleness check fires - it does not encode zero cadence itself.
        return {0LL, fromHex("00012c02000000df01ff")};
    }
};

TEST_F(FtmsRowerPm5RegressionTest, Pm5WithoutFtmsRealLogMustProducePositiveDistance) {
    ftmsrower::ParserRegressionState state;
    state.hasFtmsService = false;

    const auto packets = concept2NoFtmsPacketsFromUserLog();
    ASSERT_EQ(packets.size(), 4u);

    // Intentionally exercise the shared production parser helper instead of re-implementing parsing here.
    ftmsrower::processPm5ParserState(state, QStringLiteral("{ce060031-43e5-11e4-916c-0800200c9a66}"),
                                     packets[0].payload, packets[0].timestampMs);
    const double firstDistanceKm = state.distanceKm;
    EXPECT_GT(firstDistanceKm, 0.0);

    ftmsrower::processPm5ParserState(state, QStringLiteral("{ce060032-43e5-11e4-916c-0800200c9a66}"),
                                     packets[1].payload, packets[1].timestampMs);
    const double secondDistanceKm = state.distanceKm;
    EXPECT_GE(secondDistanceKm, firstDistanceKm);

    ftmsrower::processPm5ParserState(state, QStringLiteral("{ce060031-43e5-11e4-916c-0800200c9a66}"),
                                     packets[2].payload, packets[2].timestampMs);
    const double thirdDistanceKm = state.distanceKm;
    EXPECT_GE(thirdDistanceKm, secondDistanceKm);

    ftmsrower::processPm5ParserState(state, QStringLiteral("{ce060032-43e5-11e4-916c-0800200c9a66}"),
                                     packets[3].payload, packets[3].timestampMs);
    EXPECT_GE(state.distanceKm, thirdDistanceKm);
    EXPECT_TRUE(std::isfinite(state.distanceKm));
}

TEST_F(FtmsRowerPm5RegressionTest, Pm5WithoutFtmsIdlePacketsMustZeroSpeedAndCadenceWithoutBreakingDistance) {
    ftmsrower::ParserRegressionState state;
    state.hasFtmsService = false;

    const auto rowingPackets = concept2NoFtmsPacketsFromUserLog();
    const auto idlePackets = concept2NoFtmsIdlePacketsFromUserLog();
    ASSERT_EQ(rowingPackets.size(), 4u);
    ASSERT_EQ(idlePackets.size(), 2u);

    ftmsrower::processPm5ParserState(state, QStringLiteral("{ce060031-43e5-11e4-916c-0800200c9a66}"),
                                     rowingPackets[0].payload, rowingPackets[0].timestampMs);
    ftmsrower::processPm5ParserState(state, QStringLiteral("{ce060032-43e5-11e4-916c-0800200c9a66}"),
                                     rowingPackets[1].payload, rowingPackets[1].timestampMs);
    const double activeDistanceKm = state.distanceKm;
    ASSERT_GT(activeDistanceKm, 0.0);
    ASSERT_GT(state.speedKmh, 0.0);
    ASSERT_GT(state.cadence, 0.0);

    // Intentionally exercise the shared production parser helper instead of re-implementing parsing here.
    ftmsrower::processPm5ParserState(state, QStringLiteral("{ce060031-43e5-11e4-916c-0800200c9a66}"),
                                     idlePackets[0].payload, idlePackets[0].timestampMs);
    EXPECT_TRUE(state.rowStateReceived);
    EXPECT_EQ(state.rowState, 0);

    ftmsrower::processPm5ParserState(state, QStringLiteral("{ce060032-43e5-11e4-916c-0800200c9a66}"),
                                     idlePackets[1].payload, idlePackets[1].timestampMs);
    EXPECT_DOUBLE_EQ(state.speedKmh, 0.0);
    EXPECT_DOUBLE_EQ(state.cadence, 0.0);
    EXPECT_GE(state.distanceKm, activeDistanceKm);
    EXPECT_TRUE(std::isfinite(state.distanceKm));
}

TEST_F(FtmsRowerPm5RegressionTest, Pm5WithFtmsMustIgnoreConcept2DistancePackets) {
    ftmsrower::ParserRegressionState state;
    state.hasFtmsService = true;

    // Intentionally exercise the shared production parser helper instead of re-implementing parsing here.
    ftmsrower::processFtmsParserState(state, ftmsDistancePacketFromIssue3872Log().payload,
                                      ftmsDistancePacketFromIssue3872Log().timestampMs, false, false, false, false,
                                      false, false);
    const double ftmsDistanceKm = state.distanceKm;

    EXPECT_TRUE(std::isfinite(ftmsDistanceKm));

    ftmsrower::processPm5ParserState(state, QStringLiteral("{ce060031-43e5-11e4-916c-0800200c9a66}"),
                                     concept2ZeroDistancePacket31FromIssue3872Log().payload,
                                     concept2ZeroDistancePacket31FromIssue3872Log().timestampMs);
    EXPECT_DOUBLE_EQ(state.distanceKm, ftmsDistanceKm);

    ftmsrower::processPm5ParserState(state, QStringLiteral("{ce060032-43e5-11e4-916c-0800200c9a66}"),
                                     concept2ZeroDistancePacket32FromIssue3872Log().payload,
                                     concept2ZeroDistancePacket32FromIssue3872Log().timestampMs);
    EXPECT_DOUBLE_EQ(state.distanceKm, ftmsDistanceKm);
    EXPECT_TRUE(std::isfinite(state.distanceKm));
}

TEST_F(FtmsRowerPm5RegressionTest, Pm5WithoutFtmsSustainedIdleBurstFromIssue3686MustNotDriftDistance) {
    // Regression for #3686 "Metrics at 0 with Concept 2 PM5 Rowerg": this real device sent the
    // exact same idle CE060031/CE060032 payload repeatedly for ~21 seconds after connecting
    // (RowState byte == 0 the whole time). Distance must stay pinned to the value derived from
    // the very first packet, and speed/cadence must stay at 0, for as long as the device reports
    // RowState == 0 - it must not drift up or down just because time passes.
    ftmsrower::ParserRegressionState state;
    state.hasFtmsService = false;

    const auto idleBurst = concept2NoFtmsIdleBurstFromIssue3686Log();
    ASSERT_EQ(idleBurst.size(), 6u);

    // Intentionally exercise the shared production parser helper instead of re-implementing parsing here.
    ftmsrower::processPm5ParserState(state, QStringLiteral("{ce060031-43e5-11e4-916c-0800200c9a66}"),
                                     idleBurst[0].payload, idleBurst[0].timestampMs);
    ftmsrower::processPm5ParserState(state, QStringLiteral("{ce060032-43e5-11e4-916c-0800200c9a66}"),
                                     idleBurst[1].payload, idleBurst[1].timestampMs);
    const double firstDistanceKm = state.distanceKm;
    ASSERT_TRUE(state.rowStateReceived);
    ASSERT_EQ(state.rowState, 0);
    ASSERT_GT(firstDistanceKm, 0.0);
    EXPECT_DOUBLE_EQ(state.speedKmh, 0.0);
    EXPECT_DOUBLE_EQ(state.cadence, 0.0);

    for (size_t i = 2; i < idleBurst.size(); i += 2) {
        ftmsrower::processPm5ParserState(state, QStringLiteral("{ce060031-43e5-11e4-916c-0800200c9a66}"),
                                         idleBurst[i].payload, idleBurst[i].timestampMs);
        ftmsrower::processPm5ParserState(state, QStringLiteral("{ce060032-43e5-11e4-916c-0800200c9a66}"),
                                         idleBurst[i + 1].payload, idleBurst[i + 1].timestampMs);
        EXPECT_EQ(state.rowState, 0);
        EXPECT_DOUBLE_EQ(state.speedKmh, 0.0);
        EXPECT_DOUBLE_EQ(state.cadence, 0.0);
        EXPECT_DOUBLE_EQ(state.distanceKm, firstDistanceKm)
            << "distance must not drift while the PM5 keeps reporting RowState == 0";
    }

    EXPECT_TRUE(std::isfinite(state.distanceKm));
}

TEST_F(FtmsRowerPm5RegressionTest, Pm5WithoutFtmsActiveTransitionFromIssue3686MustAdvanceMetrics) {
    // Regression for #3686: once the same real PM5 device actually starts rowing (RowState flips
    // from 0 to 1), distance must advance past the idle-pinned value and speed/cadence must
    // become non-zero. This is the exact recovery the reporter expected but initially did not see.
    ftmsrower::ParserRegressionState state;
    state.hasFtmsService = false;

    const auto idleBurst = concept2NoFtmsIdleBurstFromIssue3686Log();
    ASSERT_GE(idleBurst.size(), 2u);
    ftmsrower::processPm5ParserState(state, QStringLiteral("{ce060031-43e5-11e4-916c-0800200c9a66}"),
                                     idleBurst[0].payload, idleBurst[0].timestampMs);
    ftmsrower::processPm5ParserState(state, QStringLiteral("{ce060032-43e5-11e4-916c-0800200c9a66}"),
                                     idleBurst[1].payload, idleBurst[1].timestampMs);
    const double idleDistanceKm = state.distanceKm;
    ASSERT_EQ(state.rowState, 0);

    const auto activeTransition = concept2NoFtmsActiveTransitionFromIssue3686Log();
    ASSERT_EQ(activeTransition.size(), 2u);

    // Intentionally exercise the shared production parser helper instead of re-implementing parsing here.
    ftmsrower::processPm5ParserState(state, QStringLiteral("{ce060031-43e5-11e4-916c-0800200c9a66}"),
                                     activeTransition[0].payload, activeTransition[0].timestampMs);
    EXPECT_TRUE(state.rowStateReceived);
    EXPECT_EQ(state.rowState, 1);
    EXPECT_GE(state.distanceKm, idleDistanceKm);

    ftmsrower::processPm5ParserState(state, QStringLiteral("{ce060032-43e5-11e4-916c-0800200c9a66}"),
                                     activeTransition[1].payload, activeTransition[1].timestampMs);
    EXPECT_GT(state.distanceKm, idleDistanceKm)
        << "distance must advance once RowState leaves idle instead of staying stuck like the "
           "reporter observed";
    EXPECT_GT(state.speedKmh, 0.0);
    EXPECT_GT(state.cadence, 0.0);
    EXPECT_TRUE(std::isfinite(state.distanceKm));
}

TEST_F(FtmsRowerPm5RegressionTest,
       Pm5WithFtmsCadenceCarryOverFromConcept2PathSuppressesFtmsStaleCadenceResetFromIssue3872) {
    // Regression/gap test for a PM5 that exposes BOTH FTMS and the Concept2-native characteristics
    // (the exact dual-path scenario already covered by the "MustIgnoreConcept2Distance*" tests
    // above, using the same #3872 session). lastStrokeMs is shared between the two parser paths:
    // processPm5ParserState (fed by CE060032 notifications) bumps it whenever the carried-over
    // cadence is still positive, even on a tick whose own spm byte is 0; processFtmsParserState
    // (fed by FTMS 0x2AD1 notifications) uses it to decide whether cadence is stale and should be
    // reset to 0 after 3s of silence. This composes three real payloads from the #3872 log (see
    // the PacketSample helpers above) to show the interaction: after a genuine nonzero-cadence
    // CE060032 packet, a later CE060032 packet with spm=0 still refreshes lastStrokeMs merely
    // because cadence carried over above 0 - even though this specific packet carried no fresh
    // stroke data. That refresh then hides a real FTMS staleness window that pre-existed this
    // refactor: replaying the same sequence against the pre-refactor logic (lastStroke only bumped
    // when this tick's own spm byte was > 0) would have left lastStroke pinned at the first
    // packet's timestamp, so the FTMS packet below - arriving 4.5s later - would have been treated
    // as stale and reset cadence to 0.
    ftmsrower::ParserRegressionState state;
    state.hasFtmsService = true;
    state.rowStateReceived = true;
    state.rowState = 1; // matches the real RowState (CE060031 byte 9) during this window of #3872

    const auto nonZeroSpmPacket = concept2CadenceNonZeroPacketFromIssue3872Log();
    ftmsrower::processPm5ParserState(state, QStringLiteral("{ce060032-43e5-11e4-916c-0800200c9a66}"),
                                     nonZeroSpmPacket.payload, nonZeroSpmPacket.timestampMs);
    ASSERT_GT(state.cadence, 0.0);
    ASSERT_EQ(state.lastStrokeMs, nonZeroSpmPacket.timestampMs);

    // A real CE060032 payload the device also sent (spm byte == 0), replayed 3.5s after the
    // genuine nonzero reading above - long enough that the *original* nonzero reading would
    // already be considered stale by the time the FTMS packet below arrives.
    const qint64 dipTimestampMs = nonZeroSpmPacket.timestampMs + 3500;
    ftmsrower::processPm5ParserState(state, QStringLiteral("{ce060032-43e5-11e4-916c-0800200c9a66}"),
                                     concept2CadenceTransientZeroPacketFromIssue3872Log().payload, dipTimestampMs);
    // Cadence carries over unchanged (matches old and new behavior alike)...
    EXPECT_GT(state.cadence, 0.0);
    // ...but lastStrokeMs was refreshed anyway, purely because cadence stayed positive - this is
    // the behavior change introduced by extracting the PM5 parser: the pre-refactor code only did
    // this when spm itself was > 0 on this exact tick.
    EXPECT_EQ(state.lastStrokeMs, dipTimestampMs)
        << "known behavior change: lastStroke now advances on a zero-spm tick whenever cadence "
           "carried over from a previous packet, not only when this tick's own spm byte is > 0";

    // A real FTMS moreData=0 packet, 1s after the dip above (so not stale relative to the
    // refreshed lastStrokeMs) but 4.5s after the last genuinely fresh nonzero spm reading (so it
    // *would* have been stale under the pre-refactor lastStroke semantics).
    const qint64 ftmsTimestampMs = dipTimestampMs + 1000;
    ftmsrower::processFtmsParserState(state, ftmsMoreDataZeroPacketFromIssue3872Log().payload, ftmsTimestampMs,
                                      false, false, false, false, false, false);

    EXPECT_GT(state.cadence, 0.0) << "known gap: the FTMS stale-cadence-reset (normally fired after "
                                     "3s of silence) is suppressed here because the Concept2-native "
                                     "path kept refreshing lastStrokeMs on ticks with no fresh stroke "
                                     "data; the pre-refactor code would have reset cadence to 0 at "
                                     "this point instead";
}

TEST_F(FtmsRowerPm5RegressionTest, Pm5WithoutFtmsReplayingAnEarlierRealPacketMakesDistanceGoBackwards) {
    // None of the collected real logs (#4609, #3686, #3872) happen to contain an actual mid-session
    // PM5 distance-counter reset (e.g. a BLE reconnect, or the user pressing the erg's reset button),
    // so this test does not come from one single continuous real session like the others in this file.
    // Instead it replays two genuinely real CE060031 payloads from the SAME #4609 log
    // (debug-Mon_May_4_14_48_43_2026.log) out of their original chronological order: the low-distance
    // packet from the start of the session (303 dm) after the high-distance packet from near the end
    // (3117 dm, the same bytes as concept2NoFtmsIdlePacketsFromUserLog()[0]). This reproduces exactly
    // what would happen on a real reconnect/counter-reset mid-workout, using only bytes the device
    // actually sent - it exposes a real gap: processPm5ParserState assigns
    // `state.distanceKm = distance_dm / 10000.0` directly, with no guard against the new value being
    // lower than the already-accumulated distance, so distance visibly goes backwards.
    ftmsrower::ParserRegressionState state;
    state.hasFtmsService = false;

    const auto highDistancePacket = concept2NoFtmsIdlePacketsFromUserLog().front(); // real bytes, 3117 dm
    const auto lowDistancePacket = concept2NoFtmsPacketsFromUserLog().front();      // real bytes, 303 dm

    ftmsrower::processPm5ParserState(state, QStringLiteral("{ce060031-43e5-11e4-916c-0800200c9a66}"),
                                     highDistancePacket.payload, highDistancePacket.timestampMs);
    const double highDistanceKm = state.distanceKm;
    ASSERT_GT(highDistanceKm, 0.03);

    ftmsrower::processPm5ParserState(state, QStringLiteral("{ce060031-43e5-11e4-916c-0800200c9a66}"),
                                     lowDistancePacket.payload, lowDistancePacket.timestampMs + 1);

    // This EXPECT documents the current (buggy) behavior rather than the desired one: distance drops
    // instead of staying pinned at the previous maximum. If a monotonicity guard is added to
    // processPm5ParserState, this assertion should be updated to EXPECT_GE(state.distanceKm, highDistanceKm).
    EXPECT_LT(state.distanceKm, highDistanceKm)
        << "known gap: PM5-without-FTMS distance has no monotonicity guard, so a replayed/older "
           "real packet (e.g. after a reconnect) makes distance jump backwards instead of staying "
           "pinned at the previous maximum";
    EXPECT_TRUE(std::isfinite(state.distanceKm));
}

TEST_F(FtmsRowerPm5RegressionTest, Pm5WithFtmsMustIgnoreConcept2DistanceEvenWhenConcept2DistanceIsPositive) {
    ftmsrower::ParserRegressionState state;
    state.hasFtmsService = true;

    // Intentionally exercise the shared production parser helper instead of re-implementing parsing here.
    ftmsrower::processFtmsParserState(state, ftmsDistancePacketFromIssue3872Log().payload,
                                      ftmsDistancePacketFromIssue3872Log().timestampMs, false, false, false, false,
                                      false, false);
    const double ftmsDistanceKm = state.distanceKm;
    ASSERT_GT(ftmsDistanceKm, 0.0);

    const auto concept2Packets = concept2NoFtmsPacketsFromUserLog();
    ASSERT_FALSE(concept2Packets.empty());

    ftmsrower::processPm5ParserState(state, QStringLiteral("{ce060031-43e5-11e4-916c-0800200c9a66}"),
                                     concept2Packets.front().payload, concept2Packets.front().timestampMs);
    EXPECT_DOUBLE_EQ(state.distanceKm, ftmsDistanceKm);
    EXPECT_TRUE(state.rowStateReceived);
    EXPECT_EQ(state.rowState, 1);
    EXPECT_TRUE(std::isfinite(state.distanceKm));
}
