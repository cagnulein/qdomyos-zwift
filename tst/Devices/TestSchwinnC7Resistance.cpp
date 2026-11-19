#include <gtest/gtest.h>
#include <limits>

#include "devices/schwinnic4bike/schwinnic4bike.h"

// User report: Bowflex C7-E6ED, 2026-11-19. All 203 received FTMS packets
// contained physical resistance 40 even though the old power/cadence estimate
// varied between 1 and 73. These are real low/high cadence packet samples.
TEST(SchwinnC7ResistanceRegression, PrefersPhysicalResistanceFromRealFtmsSamples) {
    const QByteArray samples[] = {
        QByteArray::fromHex(
            "fe 09 27 08 00 00 40 00 00 00 73 0a 00 28 00 33 00 00 00 2a 00 dc 00 03 29 04"),
        QByteArray::fromHex(
            "fe 09 5d 10 00 00 c2 00 00 00 2e 0c 00 28 00 37 01 00 00 34 00 2f 05 16 6b 04")
    };
    const int expectedCadences[] = {32, 97};
    const int expectedWatts[] = {51, 311};

    for (int i = 0; i < 2; ++i) {
        const QByteArray &packet = samples[i];
        const quint16 flags = quint8(packet.at(0)) | (quint16(quint8(packet.at(1))) << 8);
        ASSERT_TRUE((flags & 0x0020) != 0); // FTMS Indoor Bike Data: resistance level present

        // The two FTMS samples have the same optional-field layout as the QZ driver.
        const int cadence = (quint8(packet.at(6)) | (quint16(quint8(packet.at(7))) << 8)) / 2;
        const int resistance = quint8(packet.at(13)) | (quint16(quint8(packet.at(14))) << 8);
        const int watts = quint8(packet.at(15)) | (quint16(quint8(packet.at(16))) << 8);

        EXPECT_EQ(expectedCadences[i], cadence);
        EXPECT_EQ(expectedWatts[i], watts);
        EXPECT_EQ(40, resistance);
        EXPECT_TRUE(schwinnic4bike::shouldUseBikeFtmsResistance(QStringLiteral("C7-E6ED"), true, resistance));
    }
}

TEST(SchwinnC7ResistanceRegression, AcceptsValidReportedRangeAndCaseInsensitivePrefix) {
    EXPECT_TRUE(schwinnic4bike::shouldUseBikeFtmsResistance(QStringLiteral("c7-1234"), true, 1));
    EXPECT_TRUE(schwinnic4bike::shouldUseBikeFtmsResistance(QStringLiteral("C7-E6ED"), true, 100));
}

TEST(SchwinnC7ResistanceRegression, FallsBackWhenLevelIsMissingOrInvalid) {
    const QString c7 = QStringLiteral("C7-E6ED");
    EXPECT_FALSE(schwinnic4bike::shouldUseBikeFtmsResistance(c7, false, 40));
    EXPECT_FALSE(schwinnic4bike::shouldUseBikeFtmsResistance(c7, true, 0));
    EXPECT_FALSE(schwinnic4bike::shouldUseBikeFtmsResistance(c7, true, -1));
    EXPECT_FALSE(schwinnic4bike::shouldUseBikeFtmsResistance(c7, true, 101));
    EXPECT_FALSE(schwinnic4bike::shouldUseBikeFtmsResistance(
        c7, true, std::numeric_limits<double>::quiet_NaN()));
}

TEST(SchwinnC7ResistanceRegression, LeavesOtherSchwinnModelsOnLegacyEstimate) {
    EXPECT_FALSE(schwinnic4bike::shouldUseBikeFtmsResistance(QStringLiteral("IC BIKE"), true, 40));
    EXPECT_FALSE(schwinnic4bike::shouldUseBikeFtmsResistance(QStringLiteral("C9/C10"), true, 40));
    EXPECT_FALSE(schwinnic4bike::shouldUseBikeFtmsResistance(QStringLiteral("C7"), true, 40));
}
