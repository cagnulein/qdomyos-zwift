#pragma once

#include <gtest/gtest.h>
#include "devices/kettlerusbbike/KettlerUSB.h"

// Recorded on Kettler E7 USB (57600 baud), QZ 2.20.15, 2025-11-28.
TEST(KettlerUSBProtocolTest, E7IdleFramePreservesLegacyOffsets) {
    KettlerUSB::StatusFrame frame;
    ASSERT_TRUE(KettlerUSB::decodeStatusFrame(QStringLiteral("0\t0\t0\t0\t025\t0\t00:00\t0"), frame));
    EXPECT_EQ(KettlerUSB::Protocol::Legacy8, frame.protocol);
    EXPECT_DOUBLE_EQ(0.0, frame.heartRate);
    EXPECT_DOUBLE_EQ(0.0, frame.cadence);
    EXPECT_DOUBLE_EQ(0.0, frame.speed);
    EXPECT_DOUBLE_EQ(0.0, frame.distance);
    EXPECT_DOUBLE_EQ(0.0, frame.power);
}

TEST(KettlerUSBProtocolTest, E7MovingFramePreservesLegacyOffsets) {
    KettlerUSB::StatusFrame frame;
    ASSERT_TRUE(KettlerUSB::decodeStatusFrame(QStringLiteral("0\t89\t316\t43\t065\t115\t09:02\t65"), frame));
    EXPECT_EQ(KettlerUSB::Protocol::Legacy8, frame.protocol);
    EXPECT_DOUBLE_EQ(0.0, frame.heartRate);
    EXPECT_DOUBLE_EQ(89.0, frame.cadence);
    EXPECT_DOUBLE_EQ(31.6, frame.speed);
    EXPECT_DOUBLE_EQ(4300.0, frame.distance);
    EXPECT_DOUBLE_EQ(65.0, frame.power);
}

// Kettler AX1 Revision 4904, issue #5379, 2026-10-18.
TEST(KettlerUSBProtocolTest, AX1FrameHasLeadingProtocolField) {
    KettlerUSB::StatusFrame frame;
    ASSERT_TRUE(KettlerUSB::decodeStatusFrame(QStringLiteral("190\t000\t053\t188\t007\t043\t0019\t02:50\t043"), frame));
    EXPECT_EQ(KettlerUSB::Protocol::Extended9, frame.protocol);
    EXPECT_DOUBLE_EQ(0.0, frame.heartRate);
    EXPECT_DOUBLE_EQ(53.0, frame.cadence);
    EXPECT_DOUBLE_EQ(18.8, frame.speed);
    EXPECT_DOUBLE_EQ(700.0, frame.distance);
    EXPECT_DOUBLE_EQ(43.0, frame.power);
}

// Preserve the original >=8-field parser: some E7 consoles append vendor fields.
TEST(KettlerUSBProtocolTest, E7WithVendorFieldIsStillLegacy) {
    KettlerUSB::StatusFrame frame;
    ASSERT_TRUE(KettlerUSB::decodeStatusFrame(QStringLiteral("0\t89\t316\t43\t065\t115\t09:02\t65\tEXTRA"), frame));
    EXPECT_EQ(KettlerUSB::Protocol::Legacy8, frame.protocol);
    EXPECT_DOUBLE_EQ(89.0, frame.cadence);
    EXPECT_DOUBLE_EQ(31.6, frame.speed);
    EXPECT_DOUBLE_EQ(65.0, frame.power);
}

// Preserve the original E7 tolerance for intermittently unavailable
// distance/energy fields; only actual required telemetry must be numeric.
TEST(KettlerUSBProtocolTest, E7AllowsMissingDistanceAndEnergy) {
    KettlerUSB::StatusFrame frame;
    ASSERT_TRUE(KettlerUSB::decodeStatusFrame(QStringLiteral("0\t89\t316\t--\t065\t--\t09:02\t65"), frame));
    EXPECT_EQ(KettlerUSB::Protocol::Legacy8, frame.protocol);
    EXPECT_DOUBLE_EQ(0.0, frame.distance);
    EXPECT_DOUBLE_EQ(65.0, frame.power);
}

TEST(KettlerUSBProtocolTest, AX1CanHaveOptionalNonnumericFields) {
    KettlerUSB::StatusFrame frame;
    ASSERT_TRUE(KettlerUSB::decodeStatusFrame(QStringLiteral("AX1\t000\t053\t188\t007\t--\t0019\t02:50\t043"), frame));
    EXPECT_EQ(KettlerUSB::Protocol::Extended9, frame.protocol);
    EXPECT_DOUBLE_EQ(53.0, frame.cadence);
    EXPECT_DOUBLE_EQ(43.0, frame.power);
}

TEST(KettlerUSBProtocolTest, RejectsAmbiguousTimeColumns) {
    EXPECT_FALSE(KettlerUSB::isStatusResponse(QStringLiteral("0\t89\t316\t43\t065\t115\t09:02\t10:02\t65")));
    EXPECT_FALSE(KettlerUSB::isStatusResponse(QStringLiteral("0\t89\t316\t43\t065\t115\tbad\tbad\t65")));
}

TEST(KettlerUSBProtocolTest, RejectsIdentificationAcknowledgementAndMalformedFrames) {
    for (const QString &reply : {QStringLiteral("ACK"),
                                 QStringLiteral("ERROR"),
                                 QStringLiteral("SK10B3579"),
                                 QStringLiteral("SK10B E7"),
                                 QStringLiteral("07682-860-0014"),
                                 QStringLiteral("3579"),
                                 QStringLiteral("190\t000\t053\t188\t007\t043\t0019\t02:50"),
                                 QStringLiteral("190\tRPM\t053\t188\t007\t043\t0019\t02:50\t043"),
                                 QStringLiteral("190\t000\t053\t188\t007\t043\t0019\tbad\t043"),
                                 QStringLiteral("0\t89\t316\t43\t065\t115\tbad\t65")}) {
        EXPECT_FALSE(KettlerUSB::isStatusResponse(reply)) << reply.toStdString();
    }
}

TEST(KettlerUSBProtocolTest, PreserveFragmentedAndQueuedLegacyStartupReplies) {
    QByteArray buffer("SK10");
    QString response;
    EXPECT_FALSE(KettlerUSB::takeResponse(buffer, response));

    buffer.append("B3579\r\n3579\r\nSK10B E7\r\nACK\r\n");
    ASSERT_TRUE(KettlerUSB::takeResponse(buffer, response));
    EXPECT_EQ(QStringLiteral("SK10B3579"), response);
    ASSERT_TRUE(KettlerUSB::takeResponse(buffer, response));
    EXPECT_EQ(QStringLiteral("3579"), response);
    ASSERT_TRUE(KettlerUSB::takeResponse(buffer, response));
    EXPECT_EQ(QStringLiteral("SK10B E7"), response);
    ASSERT_TRUE(KettlerUSB::takeResponse(buffer, response));
    EXPECT_EQ(QStringLiteral("ACK"), response);
    EXPECT_TRUE(buffer.isEmpty());
}

TEST(KettlerUSBProtocolTest, PreservePartialAndQueuedAX1Frame) {
    QByteArray buffer("ACK\r\n190\t000\t053");
    QString response;
    ASSERT_TRUE(KettlerUSB::takeResponse(buffer, response));
    EXPECT_EQ(QStringLiteral("ACK"), response);
    EXPECT_FALSE(KettlerUSB::takeResponse(buffer, response));

    buffer.append("\t188\t007\t043\t0019\t02:50\t043\r\n");
    ASSERT_TRUE(KettlerUSB::takeResponse(buffer, response));
    EXPECT_TRUE(KettlerUSB::isStatusResponse(response));
    EXPECT_TRUE(buffer.isEmpty());
}