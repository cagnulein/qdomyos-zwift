#pragma once

#include <cmath>

#include <gtest/gtest.h>
#include <QByteArray>

#include "sportsplusbike/sportsplusbike.h"

TEST(CareSportsPlusBikeDetectionTest, UsesLengthAndFirstNumericCharacter) {
    EXPECT_TRUE(sportsplusbike::isCareSportsPlusBike13Name(QStringLiteral("CARE113770737")));

    // Known CARE rowers must not be reclassified by the 13-character bike rule.
    EXPECT_FALSE(sportsplusbike::isCareSportsPlusBike13Name(QStringLiteral("CARE10692135")));
    EXPECT_FALSE(sportsplusbike::isCareSportsPlusBike13Name(QStringLiteral("CARE968300122")));

    // Existing 11-character CARE bikes are handled by the legacy detection branch.
    EXPECT_FALSE(sportsplusbike::isCareSportsPlusBike13Name(QStringLiteral("CARE9040177")));
    EXPECT_FALSE(sportsplusbike::isCareSportsPlusBike13Name(QStringLiteral("CARE11377073")));
    EXPECT_FALSE(sportsplusbike::isCareSportsPlusBike13Name(QStringLiteral("CARE2A3770737")));
}

TEST(CareSportsPlusBikeParserTest, ParsesRealFramesFromCare113770737Log) {
    struct FrameExpectation {
        const char *hex;
        int cadence;
        int watts;
    };

    // Frames copied from debug-Mon_Aug_3_08_40_03_2026.log.
    const FrameExpectation frames[] = {
        {"20 10 00 00 00 00 00 00 00 00 00 30", 0, 0},
        {"20 10 00 41 00 02 00 00 00 00 11 84", 41, 11},
        {"20 10 00 51 00 21 00 03 00 01 22 c8", 51, 22},
        {"20 10 00 60 00 64 00 14 00 00 80 88", 60, 80},
        {"20 00 01 53 00 02 00 00 00 00 11 87", -1, 11},
    };

    for (const auto &expected : frames) {
        const QByteArray packet = QByteArray::fromHex(expected.hex);

        ASSERT_TRUE(sportsplusbike::isCareSportsPlusBike13Packet(packet)) << expected.hex;
        EXPECT_EQ(sportsplusbike::careSportsPlusBike13Cadence(packet), expected.cadence) << expected.hex;
        EXPECT_EQ(sportsplusbike::careSportsPlusBike13Watts(packet), expected.watts) << expected.hex;

        if (expected.cadence >= 0) {
            EXPECT_NEAR(sportsplusbike::careSportsPlusBike13Speed(packet),
                        0.37497622 * expected.cadence, 1e-9)
                << expected.hex;
        } else {
            EXPECT_DOUBLE_EQ(sportsplusbike::careSportsPlusBike13Speed(packet), 0.0) << expected.hex;
        }
    }
}

TEST(CareSportsPlusBikeParserTest, RejectsInvalidFrameChecksum) {
    QByteArray packet = QByteArray::fromHex("20 10 00 41 00 02 00 00 00 00 11 84");
    packet[11] = static_cast<char>(0x85);

    EXPECT_FALSE(sportsplusbike::isCareSportsPlusBike13Packet(packet));
    EXPECT_EQ(sportsplusbike::careSportsPlusBike13Cadence(packet), -1);
    EXPECT_EQ(sportsplusbike::careSportsPlusBike13Watts(packet), -1);
}
