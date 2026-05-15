#pragma once

#include <gtest/gtest.h>
#include <QByteArray>
#include <vector>

#include "horizontreadmill/horizontreadmill.h"

class HorizonParagonXParserRegressionTest : public testing::Test {
protected:
    struct CapturedFrame {
        const char *label;
        QByteArray frame;
        uint8_t expectedConsoleState;
        uint8_t expectedBeltState;
        bool expectedBeltRunning;
        double expectedReportedSpeed;
        double expectedEffectiveSpeed;
    };

    static std::vector<CapturedFrame> getLogFrames() {
        // Real Horizon Paragon X packets extracted from debug-Thu_May_14_21_40_13_2026.log.
        return {
            {
                "running from QZ",
                QByteArray::fromHex("55aa0000021740005c7300000000012d00000000000000005c015c015c010f00010001008200170006000000000000000000010000000000d3ff00000100000000000c000000000000000d0a"),
                0x01, 0x00, false, 5.6005032, 0.0,
            },
            {
                "belt moving from QZ",
                QByteArray::fromHex("55aa00000217400012a000000000012e00020000000000005c015c015c010f0001000100ae014a0007000000000000003600010000000000000000000100000000000e000000000000000d0a"),
                0x01, 0x02, true, 5.6005032, 5.6005032,
            },
            {
                "stopped from QZ while raw speed remains stale",
                QByteArray::fromHex("55aa000002174000a3c200000000043100000000000000005c015c015c010f0001000100ae014a000a000000000000003c010100000000000000000001000000000014000000000000000d0a"),
                0x04, 0x00, false, 5.6005032, 0.0,
            },
            {
                "paused from treadmill at 0.8 kmh",
                QByteArray::fromHex("55aa000002174000f6b30000000002090000000000000000320032003200000000000000820000000100000000000000ab000100000000000000000001000000000002000000000000000d0a"),
                0x02, 0x00, false, 0.80467, 0.0,
            },
            {
                "starting from treadmill at 0.8 kmh",
                QByteArray::fromHex("55aa000002174000e13b00000000050900010000000000003200320032000000000000008200170001000000000000003b000100000000000000000001000000000002000000000000000d0a"),
                0x05, 0x01, false, 0.80467, 0.0,
            },
            {
                "belt moving from treadmill at 1.2 kmh",
                QByteArray::fromHex("55aa00000217400063b800000000011d00020000000000004b004b004b0000000000000096001a000400000000000000ed000100000000000000000001000000000009000000000000000d0a"),
                0x01, 0x02, true, 1.207005, 1.207005,
            },
            {
                "paused from treadmill at 1.2 kmh",
                QByteArray::fromHex("55aa000002174000e3bf00000000022400000000000000004b004b004b00000000010001960000000500000000000000fd00010000000000000000000100000000000c000000000000000d0a"),
                0x02, 0x00, false, 1.207005, 0.0,
            },
        };
    }
};

TEST_F(HorizonParagonXParserRegressionTest, UsesConsoleAndBeltStateInsteadOfTrustingRawSpeed) {
    for (const CapturedFrame &captured : getLogFrames()) {
        const horizontreadmill::ParagonXStatus status = horizontreadmill::parseParagonXStatusPacket(captured.frame);

        EXPECT_TRUE(status.valid) << captured.label;
        EXPECT_EQ(status.consoleState, captured.expectedConsoleState) << captured.label;
        EXPECT_EQ(status.beltState, captured.expectedBeltState) << captured.label;
        EXPECT_EQ(status.beltRunning, captured.expectedBeltRunning) << captured.label;
        EXPECT_NEAR(status.reportedSpeed, captured.expectedReportedSpeed, 0.0001) << captured.label;
        EXPECT_NEAR(status.effectiveSpeed, captured.expectedEffectiveSpeed, 0.0001) << captured.label;
    }
}

TEST_F(HorizonParagonXParserRegressionTest, RejectsNonStatusPackets) {
    const QByteArray pauseEvent = QByteArray::fromHex("55aa0000030302000f1d00000d0a");
    const horizontreadmill::ParagonXStatus status = horizontreadmill::parseParagonXStatusPacket(pauseEvent);

    EXPECT_FALSE(status.valid);
    EXPECT_FALSE(status.beltRunning);
    EXPECT_DOUBLE_EQ(status.reportedSpeed, 0);
    EXPECT_DOUBLE_EQ(status.effectiveSpeed, 0);
}
