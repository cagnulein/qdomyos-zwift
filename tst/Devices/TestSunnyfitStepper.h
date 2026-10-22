#pragma once

#include "gtest/gtest.h"
#include <QByteArray>
#include <vector>

/**
 * @brief Sunnyfit Mini Stepper (SF-S) BLE Packet Test Data
 *
 * Extracted from btsnoop_hci.log capture of actual device communication.
 * These are the 20-byte data frames (0x5a 0x05) from the capture file.
 */
class SunnyfitStepperTestData {
public:
    /**
     * @brief Raw 20-byte data frames captured from actual device
     * Format: 0x5a (start) + 0x05 (command) + 18 bytes of data
     *
     * Byte positions:
     *   [0]:     0x5a (start marker)
     *   [1]:     0x05 (command type - data frame)
     *   [6]:     Cadence (SPM) - single byte
     *   [16]:    Step Counter (increments 0, 1, 2, 3...)
     */
    static const std::vector<QByteArray> getTestFrames() {
        return {
            // Frame 0: cadence=0, step=0
            QByteArray::fromHex("5a05001a032200000524000000000003260000052900"),

            // Frame 1: cadence=0, step=1
            QByteArray::fromHex("5a05001a032200000524010000000003260100052900"),

            // Frame 2: cadence=0, step=2
            QByteArray::fromHex("5a05001a032200000524020000000003260200052900"),

            // Frame 3: cadence=32, step=3
            QByteArray::fromHex("5a05001a032220000524020000000003260300052900"),

            // Frame 4: cadence=67, step=4
            QByteArray::fromHex("5a05001a032243000524040000000003260400052900"),

            // Frame 5: cadence=67, step=5
            QByteArray::fromHex("5a05001a032243000524040000000003260500052900"),

            // Frame 6: cadence=67, step=6
            QByteArray::fromHex("5a05001a032243000524040000000003260600052900"),

            // Frame 7: cadence=20, step=7
            QByteArray::fromHex("5a05001a032214000524050000000003260700052900"),

            // Frame 8: cadence=53, step=8
            QByteArray::fromHex("5a05001a032235000524070000000003260800052900"),

            // Frame 9: cadence=63, step=9
            QByteArray::fromHex("5a05001a03223f000524080000000003260900052900"),

            // Frame 10: cadence=63, step=10
            QByteArray::fromHex("5a05001a03223f000524080000000003260a00052900"),
        };
    }

    /**
     * @brief Expected extracted values from each test frame
     */
    struct ExpectedMetrics {
        int frameIndex;
        double expectedCadence;
        int expectedStepCount;
        double expectedSpeed;  // cadence / 3.2
    };

    static const std::vector<ExpectedMetrics> getExpectedValues() {
        return {
            {0, 0.0, 0, 0.0},           // cadence=0, step=0
            {1, 0.0, 1, 0.0},           // cadence=0, step=1
            {2, 0.0, 2, 0.0},           // cadence=0, step=2
            {3, 32.0, 3, 10.0},         // cadence=32, step=3, speed=32/3.2=10
            {4, 67.0, 4, 20.9375},      // cadence=67, step=4, speed=67/3.2≈20.94
            {5, 67.0, 5, 20.9375},      // cadence=67, step=5
            {6, 67.0, 6, 20.9375},      // cadence=67, step=6
            {7, 20.0, 7, 6.25},         // cadence=20, step=7, speed=20/3.2=6.25
            {8, 53.0, 8, 16.5625},      // cadence=53, step=8, speed=53/3.2≈16.56
            {9, 63.0, 9, 19.6875},      // cadence=63, step=9, speed=63/3.2≈19.69
            {10, 63.0, 10, 19.6875},    // cadence=63, step=10
        };
    }

    /**
     * @brief Parse a single 20-byte frame and extract metrics
     * @return pair<cadence, stepCount> or returns {-1, -1} on error
     */
    static std::pair<double, int> parseFrame(const QByteArray& frame) {
        if (frame.length() != 20) {
            return {-1, -1};
        }

        if ((uint8_t)frame[0] != 0x5a) {
            return {-1, -1};
        }

        // Extract cadence from byte 6 (single byte)
        double cadence = (double)(uint8_t)frame[6];

        // Extract step counter from byte 16 (single byte, little-endian)
        int stepCount = (uint8_t)frame[16];

        return {cadence, stepCount};
    }
};

/**
 * @brief Test suite for Sunnyfit Stepper frame parsing
 */
class SunnyfitStepperParsingTest : public testing::Test {
protected:
    SunnyfitStepperTestData testData;
};

/**
 * @brief Test parsing of individual frames
 */
TEST_F(SunnyfitStepperParsingTest, ParseFrames) {
    auto frames = SunnyfitStepperTestData::getTestFrames();
    auto expectedValues = SunnyfitStepperTestData::getExpectedValues();

    ASSERT_EQ(frames.size(), expectedValues.size())
        << "Test data mismatch: frames and expected values should have same size";

    for (size_t i = 0; i < frames.size(); ++i) {
        auto [cadence, stepCount] = SunnyfitStepperTestData::parseFrame(frames[i]);

        EXPECT_EQ(cadence, expectedValues[i].expectedCadence)
            << "Frame " << i << ": Cadence mismatch";

        EXPECT_EQ(stepCount, expectedValues[i].expectedStepCount)
            << "Frame " << i << ": Step count mismatch";
    }
}

/**
 * @brief Test speed calculation from cadence
 */
TEST_F(SunnyfitStepperParsingTest, CalculateSpeed) {
    auto frames = SunnyfitStepperTestData::getTestFrames();
    auto expectedValues = SunnyfitStepperTestData::getExpectedValues();

    for (size_t i = 0; i < frames.size(); ++i) {
        auto [cadence, stepCount] = SunnyfitStepperTestData::parseFrame(frames[i]);

        double calculatedSpeed = cadence / 3.2;

        EXPECT_DOUBLE_EQ(calculatedSpeed, expectedValues[i].expectedSpeed)
            << "Frame " << i << ": Speed calculation mismatch (cadence=" << cadence << ")";
    }
}

/**
 * @brief Test step counter increments
 */
TEST_F(SunnyfitStepperParsingTest, StepCounterIncrement) {
    auto frames = SunnyfitStepperTestData::getTestFrames();
    auto expectedValues = SunnyfitStepperTestData::getExpectedValues();

    int previousSteps = -1;
    for (size_t i = 0; i < frames.size(); ++i) {
        auto [cadence, stepCount] = SunnyfitStepperTestData::parseFrame(frames[i]);

        if (previousSteps >= 0) {
            int increment = stepCount - previousSteps;
            EXPECT_EQ(increment, 1)
                << "Frame " << i << ": Step counter should increment by 1 (was "
                << previousSteps << ", now " << stepCount << ")";
        }
        previousSteps = stepCount;
    }
}

/**
 * @brief Test cadence variation detection
 */
TEST_F(SunnyfitStepperParsingTest, CadenceVariation) {
    auto frames = SunnyfitStepperTestData::getTestFrames();

    std::vector<double> cadences;
    for (const auto& frame : frames) {
        auto [cadence, stepCount] = SunnyfitStepperTestData::parseFrame(frame);
        cadences.push_back(cadence);
    }

    // Verify we have cadence variation in the test data
    double minCadence = *std::min_element(cadences.begin(), cadences.end());
    double maxCadence = *std::max_element(cadences.begin(), cadences.end());

    EXPECT_LT(minCadence, maxCadence)
        << "Test data should have cadence variation";

    EXPECT_EQ(minCadence, 0.0)
        << "Minimum cadence should be 0";

    EXPECT_EQ(maxCadence, 67.0)
        << "Maximum cadence should be 67";
}

/**
 * @brief Test frame validation
 */
TEST_F(SunnyfitStepperParsingTest, FrameValidation) {
    // Invalid length
    QByteArray shortFrame = QByteArray::fromHex("5a05001a0322");
    auto [cadence, stepCount] = SunnyfitStepperTestData::parseFrame(shortFrame);
    EXPECT_EQ(cadence, -1) << "Should reject short frames";
    EXPECT_EQ(stepCount, -1) << "Should reject short frames";

    // Invalid start marker
    QByteArray invalidStart = QByteArray::fromHex("0105001a032200000524000000000003260000052900");
    std::tie(cadence, stepCount) = SunnyfitStepperTestData::parseFrame(invalidStart);
    EXPECT_EQ(cadence, -1) << "Should reject frames with invalid start marker";
    EXPECT_EQ(stepCount, -1) << "Should reject frames with invalid start marker";

    // Valid frame
    QByteArray validFrame = SunnyfitStepperTestData::getTestFrames()[3];
    std::tie(cadence, stepCount) = SunnyfitStepperTestData::parseFrame(validFrame);
    EXPECT_EQ(cadence, 32.0) << "Should parse valid frame";
    EXPECT_EQ(stepCount, 3) << "Should parse valid frame";
}
