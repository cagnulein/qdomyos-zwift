#include <gtest/gtest.h>
#include <QByteArray>

#include "devices/trxappgateusbrower/trxappgateusbrower.h"

// Actual BLE notifications from the Odin R700 / i-Console+0132 support log,
// October 25, 2026. Only packet bytes are included; no user/device identifiers.
TEST(TrxAppGateUsbRowerParserTest, DecodesCapturedOdinR700Session) {
    struct Sample {
        const char *hex;
        int elapsed, strokes, cadence, distanceMeters, watts, paceSeconds, calories;
        double speed;
    };
    const Sample samples[] = {
        // Initial idle frame.
        {"f0b201e70101010101010101010101010101010102029e", 0, 0, 0, 0, 0, 0, 0, 0},
        // The machine reports distance and a cached pace before reporting cadence.
        {"f0b201e70101010101010102010101010101042d0202ce", 0, 0, 0, 1, 0, 224, 0, 0},
        {"f0b201e7010b0106011a011201020101014f071302023e", 10, 5, 25, 17, 78, 378, 1, 1800.0 / 378},
        {"f0b201e7011f010f011b012b010301010152083c0202a3", 30, 14, 26, 42, 81, 479, 2, 1800.0 / 479},
        // Highest recorded power: 111 W, not the ~700 W from the old parser.
        {"f0b201e7020c01130124013801040101020d0627020251", 71, 18, 35, 55, 111, 338, 3, 1800.0 / 338},
        // Paused, then still paused 25 seconds later.
        {"f0b201e70124010f0101012e01040101010101010202ff", 35, 14, 0, 45, 0, 0, 3, 0},
        {"f0b201e70201010f0101012e01040101010101010202dd", 60, 14, 0, 45, 0, 0, 3, 0},
        // Above the first two-digit distance field: 250 m.
        {"f0b201e7061a013c01180335010e01010147052f0202c9", 325, 59, 23, 250, 70, 286, 13, 1800.0 / 286},
        {"f0b201e70626013c01010335010e010101010101020246", 337, 59, 0, 250, 0, 0, 13, 0},
    };

    for (const auto &sample : samples) {
        SCOPED_TRACE(sample.hex);
        const auto metrics = trxappgateusbrower::iConsoleRowerMetricsFromPacket(QByteArray::fromHex(sample.hex));
        ASSERT_TRUE(metrics.valid);
        EXPECT_EQ(metrics.elapsed, sample.elapsed);
        EXPECT_EQ(metrics.strokes, sample.strokes);
        EXPECT_EQ(metrics.cadence, sample.cadence);
        EXPECT_EQ(metrics.distanceMeters, sample.distanceMeters);
        EXPECT_EQ(metrics.watts, sample.watts);
        EXPECT_EQ(metrics.paceSeconds, sample.paceSeconds);
        EXPECT_EQ(metrics.calories, sample.calories);
        EXPECT_EQ(metrics.heart, 0);
        EXPECT_EQ(metrics.resistance, 1);
        EXPECT_NEAR(metrics.speed, sample.speed, 0.000001);
    }
}

TEST(TrxAppGateUsbRowerParserTest, RejectsNonTelemetryAndShortPackets) {
    EXPECT_FALSE(trxappgateusbrower::iConsoleRowerMetricsFromPacket(
        QByteArray::fromHex("f0b701e70493")).valid);
    EXPECT_FALSE(trxappgateusbrower::iConsoleRowerMetricsFromPacket(
        QByteArray::fromHex("f0b201e7010101010101010101010101010101010202")).valid);
    EXPECT_FALSE(trxappgateusbrower::iConsoleRowerMetricsFromPacket(
        QByteArray::fromHex("f0b201e80101010101010101010101010101010102029e")).valid);
}
