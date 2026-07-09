#pragma once

#include <gtest/gtest.h>
#include <QByteArray>

#include "devices/nordictrackelliptical/nordictrackelliptical.h"

/**
 * @file TestNordictrackEllipticalS700Parser.h
 * @brief Regression test for issue #4794 (NordicTrack Elliptical Spacesaver S700).
 *
 * On this unit, in SE7i compatibility mode, incline/resistance work but speed/pace/calories/watts
 * never update. Root cause: the live telemetry packet reuses the SE7i type 0x01 frame shifted by
 * 2 bytes, so its 0x5a marker sits at byte[4] (preceded by two 0x00 bytes) instead of the SE7i
 * byte[4]==0x46 the code was checking for, so the speed branch never matched and Speed stayed 0
 * forever (watts()/KCal for elliptical only depend on currentSpeed()).
 *
 * These packets are taken verbatim from the debug logs attached to the GitHub issue.
 */

TEST(NordictrackEllipticalS700ParserTest, RecognizesS700SpeedPacketsFromRealLog) {
    // "<< 01 12 26 00 5a 00 32 08 01 0a 00 00 0b 03 90 01 08 01 00 00" from debug-Thu_Jul_9_09_51_31_2026.log
    EXPECT_TRUE(nordictrackelliptical::isS700SpeedPacket(
        QByteArray::fromHex("011226005a003208010a00000b03900108010000")));
    // "<< 01 12 28 00 5a 00 50 05 00 0a 00 00 30 03 5e 01 17 01 00 00" from the same log
    EXPECT_TRUE(nordictrackelliptical::isS700SpeedPacket(
        QByteArray::fromHex("011228005a005005000a000030035e0117010000")));
}

TEST(NordictrackEllipticalS700ParserTest, DoesNotMatchLegacySe7iOrDefaultSpeedPackets) {
    // Legacy SE7i marker (byte[4] == 0x46) used by other NordicTrack SE7i units: must stay untouched.
    EXPECT_FALSE(nordictrackelliptical::isS700SpeedPacket(
        QByteArray::fromHex("0112000046001711010300000000c20100000000")));

    // Default (non-SE7i) protocol packet: marker 0x5a sits at byte[2] instead of byte[4], must not collide.
    EXPECT_FALSE(nordictrackelliptical::isS700SpeedPacket(
        QByteArray::fromHex("01125a00662100000000000a000069030a010000")));

    EXPECT_FALSE(nordictrackelliptical::isS700SpeedPacket(QByteArray::fromHex("0102")));
}

TEST(NordictrackEllipticalS700ParserTest, SpeedIsDecodedFromBytes12And13LittleEndianDividedBy100) {
    const QByteArray packet1 = QByteArray::fromHex("011226005a003208010a00000b03900108010000");
    ASSERT_TRUE(nordictrackelliptical::isS700SpeedPacket(packet1));
    EXPECT_NEAR(nordictrackelliptical::s700SpeedFromPacket(packet1), 7.79, 0.001);

    const QByteArray packet2 = QByteArray::fromHex("011228005a005005000a000030035e0117010000");
    ASSERT_TRUE(nordictrackelliptical::isS700SpeedPacket(packet2));
    EXPECT_NEAR(nordictrackelliptical::s700SpeedFromPacket(packet2), 8.16, 0.001);
}

TEST(NordictrackEllipticalS700ParserTest, ResistanceAndInclinationDecodedFromSharedSe7iPacket) {
    // "<< 00 12 01 04 02 30 06 30 02 02 c2 01 f6 0f 00 00 00 00 00 00" from debug-Thu_Jul_9_09_54_49_2026.log
    // (the same shared SE7i-style type 0x00 packet the S700 also uses for incline/resistance telemetry)
    const QByteArray packet = QByteArray::fromHex("00120104023006300202c201f60f000000000000");

    ASSERT_TRUE(nordictrackelliptical::isSe7iResistanceInclinationPacket(packet));
    EXPECT_NEAR(nordictrackelliptical::se7iInclinationFromPacket(packet), 4.50, 0.001);
    EXPECT_NEAR(nordictrackelliptical::se7iResistanceFromPacket(packet), 9.0022, 0.001);
}

TEST(NordictrackEllipticalS700ParserTest, RejectsUnrelatedPacketsForResistanceInclinationDecode) {
    EXPECT_FALSE(nordictrackelliptical::isSe7iResistanceInclinationPacket(
        QByteArray::fromHex("011226005a003208010a00000b03900108010000")));
    EXPECT_FALSE(nordictrackelliptical::isSe7iResistanceInclinationPacket(QByteArray::fromHex("00")));
}
