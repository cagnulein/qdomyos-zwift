#pragma once

#include <gtest/gtest.h>
#include <QByteArray>
#include <QString>

#include "devices/inspireic15dserialbike/inspireic15dserialbike.h"
#include "devices/inspireic15dserialbike/inspireic15dserialreader.h"
#include "qzsettings.h"

namespace {

// Builds one hardware-format ACK reply frame:
// [0xf1][type][payloadLength][digits, least-significant first][checksum][0xf6]
QByteArray buildAckFrame(char type, int value) {
    QByteArray digits;
    if (value <= 0) {
        digits.append('0');
    } else {
        int v = value;
        while (v > 0) {
            digits.append(static_cast<char>('0' + (v % 10)));
            v /= 10;
        }
    }

    QByteArray frame;
    frame.append(static_cast<char>(0xf1));
    frame.append(type);
    frame.append(static_cast<char>(digits.size()));
    frame.append(digits);

    quint8 checksum = 0;
    for (int i = 0; i < frame.size(); ++i)
        checksum = static_cast<quint8>(checksum + static_cast<quint8>(frame.at(i)));
    frame.append(static_cast<char>(checksum));
    frame.append(static_cast<char>(0xf6));
    return frame;
}

QByteArray buildNakFrame(char type) {
    QByteArray frame;
    frame.append(static_cast<char>(0xf3));
    frame.append(type);
    frame.append(static_cast<char>(0x00));
    frame.append(static_cast<char>(0x00));
    frame.append(static_cast<char>(0xf6));
    return frame;
}

} // namespace

TEST(InspireIC15DFindFrameMarkerTest, FindsAckAndNakMarkers) {
    EXPECT_EQ(inspireic15dserialreader::findFrameMarker(QByteArray::fromHex("00f1410275f6")), 1);
    EXPECT_EQ(inspireic15dserialreader::findFrameMarker(QByteArray::fromHex("00f3410000f6")), 1);
}

TEST(InspireIC15DFindFrameMarkerTest, ReturnsNegativeOneWhenNoMarkerPresent) {
    EXPECT_EQ(inspireic15dserialreader::findFrameMarker(QByteArray::fromHex("0011223344")), -1);
    EXPECT_EQ(inspireic15dserialreader::findFrameMarker(QByteArray()), -1);
}

TEST(InspireIC15DFrameSizeAtTest, ReturnsZeroWhenNotEnoughBytesYet) {
    EXPECT_EQ(inspireic15dserialreader::frameSizeAt(QByteArray::fromHex("f141"), 0), 0);
    EXPECT_EQ(inspireic15dserialreader::frameSizeAt(QByteArray(), 0), 0);
}

TEST(InspireIC15DFrameSizeAtTest, ComputesAckFrameSizeFromPayloadLengthByte) {
    // marker, type, payloadLength=2 -> frame size 2 + 5 = 7
    EXPECT_EQ(inspireic15dserialreader::frameSizeAt(QByteArray::fromHex("f1410275f6"), 0), 7);
}

TEST(InspireIC15DFrameSizeAtTest, NakFrameIsAlwaysFiveBytes) {
    EXPECT_EQ(inspireic15dserialreader::frameSizeAt(QByteArray::fromHex("f341ffff"), 0), 5);
}

TEST(InspireIC15DFrameSizeAtTest, RejectsImplausiblePayloadLength) {
    // payload length byte = 0xff (255) is > 32 and should signal resync.
    EXPECT_EQ(inspireic15dserialreader::frameSizeAt(QByteArray::fromHex("f141ff"), 0), -1);
}

TEST(InspireIC15DParseFrameTest, DecodesCadenceFrame) {
    const QByteArray frame = buildAckFrame('A', 75);
    const auto parsed = inspireic15dserialreader::parseFrame(frame);

    EXPECT_TRUE(parsed.valid);
    EXPECT_FALSE(parsed.negativeAcknowledgement);
    EXPECT_EQ(parsed.type, 'A');
    EXPECT_EQ(parsed.value, 75);
}

TEST(InspireIC15DParseFrameTest, DecodesResistanceFrame) {
    const QByteArray frame = buildAckFrame('I', 40);
    const auto parsed = inspireic15dserialreader::parseFrame(frame);

    EXPECT_TRUE(parsed.valid);
    EXPECT_EQ(parsed.type, 'I');
    EXPECT_EQ(parsed.value, 40);
}

TEST(InspireIC15DParseFrameTest, DecodesPowerFrame) {
    const QByteArray frame = buildAckFrame('D', 123);
    const auto parsed = inspireic15dserialreader::parseFrame(frame);

    EXPECT_TRUE(parsed.valid);
    EXPECT_EQ(parsed.type, 'D');
    EXPECT_EQ(parsed.value, 123);
}

TEST(InspireIC15DParseFrameTest, DigitsAreLeastSignificantFirst) {
    // value 5 (ones digit only) and value 0 both single-digit; check a multi-digit boundary.
    EXPECT_EQ(inspireic15dserialreader::parseFrame(buildAckFrame('A', 9)).value, 9);
    EXPECT_EQ(inspireic15dserialreader::parseFrame(buildAckFrame('A', 10)).value, 10);
    EXPECT_EQ(inspireic15dserialreader::parseFrame(buildAckFrame('A', 199)).value, 199);
}

TEST(InspireIC15DParseFrameTest, NegativeAcknowledgementFrameIsReportedWithoutValidation) {
    const QByteArray frame = buildNakFrame('A');
    const auto parsed = inspireic15dserialreader::parseFrame(frame);

    EXPECT_TRUE(parsed.negativeAcknowledgement);
    EXPECT_FALSE(parsed.valid);
}

TEST(InspireIC15DParseFrameTest, RejectsMissingTerminator) {
    QByteArray frame = buildAckFrame('A', 75);
    frame[frame.size() - 1] = static_cast<char>(0x00);
    EXPECT_FALSE(inspireic15dserialreader::parseFrame(frame).valid);
}

TEST(InspireIC15DParseFrameTest, RejectsCorruptedChecksum) {
    QByteArray frame = buildAckFrame('A', 75);
    frame[frame.size() - 2] = static_cast<char>(static_cast<quint8>(frame[frame.size() - 2]) ^ 0xFF);
    EXPECT_FALSE(inspireic15dserialreader::parseFrame(frame).valid);
}

TEST(InspireIC15DParseFrameTest, RejectsNonDigitPayload) {
    QByteArray frame = buildAckFrame('A', 75);
    // Corrupt the first payload digit (index 3) to a non-digit byte.
    frame[3] = static_cast<char>('x');
    EXPECT_FALSE(inspireic15dserialreader::parseFrame(frame).valid);
}

TEST(InspireIC15DParseFrameTest, RejectsEmptyFrame) { EXPECT_FALSE(inspireic15dserialreader::parseFrame(QByteArray()).valid); }

TEST(InspireIC15DStreamScanTest, ExtractsFrameFollowingGarbagePrefix) {
    const QByteArray frame = buildAckFrame('I', 40);
    const QByteArray stream = QByteArray::fromHex("0102") + frame;

    const int start = inspireic15dserialreader::findFrameMarker(stream);
    ASSERT_GE(start, 0);
    const QByteArray trimmed = stream.mid(start);
    const int size = inspireic15dserialreader::frameSizeAt(trimmed, 0);
    ASSERT_EQ(size, frame.size());
    EXPECT_EQ(trimmed.left(size), frame);
}

// --- Stock console conversion formulas (validated against real hardware, issue #4888) --------

TEST(InspireIC15DStockConversionTest, ResistanceDividesControllerRawByTwoPointFive) {
    EXPECT_EQ(inspireic15dserialbike::stockResistance(100), 40);
    EXPECT_EQ(inspireic15dserialbike::stockResistance(0), 0);
    EXPECT_EQ(inspireic15dserialbike::stockResistance(25), 10);
}

TEST(InspireIC15DStockConversionTest, PowerIsZeroWithoutCadenceOrNegativeResistance) {
    EXPECT_EQ(inspireic15dserialbike::stockPower(0, 40), 0);
    EXPECT_EQ(inspireic15dserialbike::stockPower(75, -1), 0);
}

TEST(InspireIC15DStockConversionTest, PowerIsPositiveWithPlausibleCadenceAndResistance) {
    const int watts = inspireic15dserialbike::stockPower(75, 40);
    EXPECT_GT(watts, 0);
}

// --- Settings-gated, opt-in detection ----------------------------------------------------

TEST(InspireIC15DSettingsTest, SerialPortDefaultsToEmptyMeaningDisabled) {
    EXPECT_EQ(QZSettings::default_inspire_ic15d_serialport, QStringLiteral(""));
}

TEST(InspireIC15DSettingsTest, MetricPollingDefaultsToDisabled) {
    EXPECT_FALSE(QZSettings::default_inspire_ic15d_metric_polling);
}
