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
    // "<< 01 12 26 00 46 00 00 01 00 0a 00 00 21 03 00 00 24 01 00 00" from debug-Wed_Jul_15_12_14_08_2026b.log
    // After SE7i init completes, the machine switches to byte[4]=0x46 (remote-control mode marker).
    EXPECT_TRUE(nordictrackelliptical::isS700SpeedPacket(
        QByteArray::fromHex("011226004600000100000000210300002401" "0000")));
}

TEST(NordictrackEllipticalS700ParserTest, DoesNotMatchNonSpeedPackets) {
    // Packet with byte[3]!=0x00 (0x74 here): must not match regardless of byte[4].
    EXPECT_FALSE(nordictrackelliptical::isS700SpeedPacket(
        QByteArray::fromHex("0112007400330000000000000000000000000000")));

    // Default (non-SE7i) protocol packet: 0x5a sits at byte[2] not byte[4], byte[4]=0x66: no match.
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

    // Post-init 0x46 packet from debug-Wed_Jul_15_12_14_08_2026b.log: speed 8.01 km/h
    const QByteArray packet3 = QByteArray::fromHex("011226004600000100000000210300002401" "0000");
    ASSERT_TRUE(nordictrackelliptical::isS700SpeedPacket(packet3));
    EXPECT_NEAR(nordictrackelliptical::s700SpeedFromPacket(packet3), 8.01, 0.001);
}

TEST(NordictrackEllipticalS700ParserTest, CadenceIsDecodedFromByte2) {
    // "<< 01 12 26 00 5a ..." — byte[2]=0x26=38 RPM
    const QByteArray packet1 = QByteArray::fromHex("011226005a003208010a00000b03900108010000");
    ASSERT_TRUE(nordictrackelliptical::isS700SpeedPacket(packet1));
    EXPECT_EQ(nordictrackelliptical::s700CadenceFromPacket(packet1), 38);

    // "<< 01 12 28 00 5a ..." — byte[2]=0x28=40 RPM
    const QByteArray packet2 = QByteArray::fromHex("011228005a005005000a000030035e0117010000");
    ASSERT_TRUE(nordictrackelliptical::isS700SpeedPacket(packet2));
    EXPECT_EQ(nordictrackelliptical::s700CadenceFromPacket(packet2), 40);

    // "<< 01 12 22 00 5a ..." from debug-Mon_Jul_13 (new log after patch) — byte[2]=0x22=34 RPM
    const QByteArray packet3 = QByteArray::fromHex("011222005a000e0d000a0000cc0232000b010000");
    ASSERT_TRUE(nordictrackelliptical::isS700SpeedPacket(packet3));
    EXPECT_EQ(nordictrackelliptical::s700CadenceFromPacket(packet3), 34);
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

TEST(NordictrackEllipticalS700ParserTest, MachineWattsDecodedFromBytes14And15LittleEndian) {
    // "<< 00 12 01 04 02 30 06 30 02 02 00 00 00 00 17 00 01 00 00 00" from debug-Tue_Jul_14 log
    // Machine console showed 23W (0x17) when this packet arrived.
    const QByteArray packet1 = QByteArray::fromHex("0012010402300630020200000000170001000000");
    ASSERT_TRUE(nordictrackelliptical::isSe7iResistanceInclinationPacket(packet1));
    EXPECT_EQ(nordictrackelliptical::se7iWattsFromPacket(packet1), 23);

    // watts=0x1e=30 from a later packet in the same log
    const QByteArray packet2 = QByteArray::fromHex("00120104023006300202000000001e000a000000");
    ASSERT_TRUE(nordictrackelliptical::isSe7iResistanceInclinationPacket(packet2));
    EXPECT_EQ(nordictrackelliptical::se7iWattsFromPacket(packet2), 30);

    // watts=0 before exercise starts
    const QByteArray packet3 = QByteArray::fromHex("0012010402300630020200000000000000000000");
    ASSERT_TRUE(nordictrackelliptical::isSe7iResistanceInclinationPacket(packet3));
    EXPECT_EQ(nordictrackelliptical::se7iWattsFromPacket(packet3), 0);
}
