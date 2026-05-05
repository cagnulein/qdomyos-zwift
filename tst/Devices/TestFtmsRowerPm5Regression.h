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
