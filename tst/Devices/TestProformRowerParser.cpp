#include <gtest/gtest.h>

#include <QByteArray>
#include <QtGlobal>

#include "Tools/testsettings.h"
#include "devices/proformrower/proformrower.h"
#include "qzsettings.h"

namespace {
QByteArray frame(const QByteArray &prefix, const QByteArray &body) {
    QByteArray result = prefix;
    result.append(body);
    while (result.size() < 20)
        result.append('\0');
    return result;
}

QList<QByteArray> q750rStrokeFrames() {
    const QByteArray body = QByteArray::fromHex(
        "0104022e142e020200006000130000004600000001022f0000002f0000004bb20000002c010200b400000000000001000a4b");
    return {frame(QByteArray::fromHex("fe023204"), QByteArray(16, '\0')),
            frame(QByteArray::fromHex("0012"), body.mid(0, 18)),
            frame(QByteArray::fromHex("0112"), body.mid(18, 18)),
            frame(QByteArray::fromHex("ff0e"), body.mid(36, 14))};
}

QList<QByteArray> q750rPaceFrames() {
    const QByteArray body = QByteArray::fromHex(
        "0104021d141d02020000ffffffffffffffff0000000000000000009900c304008d");
    return {frame(QByteArray::fromHex("fe022103"), QByteArray(16, '\0')),
            frame(QByteArray::fromHex("0012"), body.mid(0, 18)),
            frame(QByteArray::fromHex("ff0f"), body.mid(18, 15))};
}
} // namespace

TEST(ProformRowerTelemetryParserTest, ReadsCadenceAndStrokeCountFromComplete750r2eResponse) {
    ProformRowerTelemetryParser parser;
    ProformRowerTelemetry metrics;
    for (const QByteArray &part : q750rStrokeFrames()) {
        const bool complete = parser.consumeFragment(part, metrics);
        if (part.startsWith(QByteArray::fromHex("ff0e")))
            EXPECT_TRUE(complete);
        else
            EXPECT_FALSE(complete);
    }
    EXPECT_TRUE(metrics.hasCadence);
    EXPECT_EQ(metrics.cadence, 10);
    EXPECT_TRUE(metrics.hasStrokeCount);
    EXPECT_EQ(metrics.strokeCount, 1);
    EXPECT_FALSE(metrics.hasSpeed);
}

TEST(ProformRowerTelemetryParserTest, ReadsPaceFromShifted750r1dResponse) {
    ProformRowerTelemetryParser parser;
    ProformRowerTelemetry metrics;
    for (const QByteArray &part : q750rPaceFrames())
        parser.consumeFragment(part, metrics);

    EXPECT_TRUE(metrics.hasSpeed);
    EXPECT_NEAR(metrics.speedKmh, 1800.0 / 153.0, 0.0001);
    EXPECT_FALSE(metrics.hasCadence);
}

TEST(ProformRowerTelemetryParserTest, RejectsIncompleteOutOfOrderAndBadChecksumResponses) {
    ProformRowerTelemetryParser parser;
    ProformRowerTelemetry metrics;
    const QList<QByteArray> frames = q750rStrokeFrames();

    parser.consumeFragment(frames.at(0), metrics);
    parser.consumeFragment(frames.at(1), metrics);
    EXPECT_FALSE(parser.consumeFragment(frames.at(3), metrics));
    EXPECT_FALSE(metrics.hasCadence);

    parser.reset();
    QList<QByteArray> corrupt = q750rStrokeFrames();
    corrupt.last()[15] = static_cast<char>(corrupt.last().at(15) ^ 0x01);
    for (const QByteArray &part : corrupt)
        EXPECT_FALSE(parser.consumeFragment(part, metrics));
    EXPECT_FALSE(metrics.hasCadence);
}

TEST(ProformRowerTelemetryParserTest, RejectsInvalidHeadersAndRecoversForNextMessage) {
    ProformRowerTelemetryParser parser;
    ProformRowerTelemetry metrics;
    const QList<QByteArray> frames = q750rStrokeFrames();

    QByteArray wrongLength = frames.first();
    wrongLength[2] = 0x31;
    EXPECT_FALSE(parser.consumeFragment(wrongLength, metrics));

    QByteArray wrongPacketCount = frames.first();
    wrongPacketCount[3] = 0x05;
    EXPECT_FALSE(parser.consumeFragment(wrongPacketCount, metrics));

    for (const QByteArray &part : frames)
        parser.consumeFragment(part, metrics);
    EXPECT_TRUE(metrics.hasCadence);
    EXPECT_EQ(metrics.cadence, 10);
}

TEST(ProformRowerTelemetryParserTest, RejectsWrongSizedFragmentAndRecovers) {
    ProformRowerTelemetryParser parser;
    ProformRowerTelemetry metrics;
    const QList<QByteArray> frames = q750rPaceFrames();
    EXPECT_FALSE(parser.consumeFragment(frames.first(), metrics));

    QByteArray shortFragment = frames.at(1);
    shortFragment.chop(1);
    EXPECT_FALSE(parser.consumeFragment(shortFragment, metrics));
    EXPECT_FALSE(metrics.hasSpeed);

    for (const QByteArray &part : frames)
        parser.consumeFragment(part, metrics);
    EXPECT_TRUE(metrics.hasSpeed);
    EXPECT_NEAR(metrics.speedKmh, 1800.0 / 153.0, 0.0001);
}

TEST(ProformRowerTelemetryParserTest, PreservesLegacyFf11ParsingWhen750rIsEnabled) {
    TestSettings testSettings(QStringLiteral("QZ ProForm Rower Tests"),
                              QStringLiteral("ProForm Rower 750R FF11 Regression"));
    testSettings.activate();
    testSettings.qsettings.clear();
    testSettings.qsettings.setValue(QZSettings::proform_rower_750r, true);
    testSettings.qsettings.sync();

    {
        proformrower device(false, true);
        QByteArray legacyFrame(20, '\0');
        legacyFrame[0] = static_cast<char>(0xff);
        legacyFrame[1] = static_cast<char>(0x11);
        legacyFrame[12] = 10;
        legacyFrame[13] = static_cast<char>(153);
        const QLowEnergyCharacteristic characteristic;
        ASSERT_TRUE(QMetaObject::invokeMethod(
            &device, "characteristicChanged", Qt::DirectConnection,
            Q_ARG(QLowEnergyCharacteristic, characteristic), Q_ARG(QByteArray, legacyFrame)));

        EXPECT_EQ(device.currentCadence().value(), 10);
        EXPECT_NEAR(device.currentSpeed().value(), 1800.0 / 153.0, 0.0001);
    }

    testSettings.deactivate();
}

TEST(ProformRowerTelemetryParserTest, DoesNotDecodeStandard1fAs750rPace) {
    ProformRowerTelemetryParser parser;
    ProformRowerTelemetry metrics;
    const QByteArray standard = QByteArray::fromHex(
        "0104021f141f02020000ffffffffffffffff00000000000000001100148400e80000c0");
    const QList<QByteArray> frames = {
        frame(QByteArray::fromHex("fe022303"), QByteArray(16, '\0')),
        frame(QByteArray::fromHex("0012"), standard.mid(0, 18)),
        frame(QByteArray::fromHex("ff11"), standard.mid(18, 17))
    };
    for (const QByteArray &part : frames)
        EXPECT_FALSE(parser.consumeFragment(part, metrics));
    EXPECT_FALSE(metrics.hasSpeed);
    EXPECT_FALSE(metrics.hasCadence);
}
