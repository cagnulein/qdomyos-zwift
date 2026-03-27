#pragma once

#include "gtest/gtest.h"
#include <QByteArray>
#include <vector>

class Stairmaster8GTestData {
public:
    struct ExpectedMetrics {
        int frameIndex;
        int expectedStepCount;
        int expectedFootfalls;
    };

    static const std::vector<QByteArray> getPrimaryFrames() {
        return {
            QByteArray::fromHex("53fb02080061010000f000030000000006000000"),
            QByteArray::fromHex("53fb0217006a010000f000090010000012000100"),
            QByteArray::fromHex("53fb023f006d0200003601190030000030000500"),
            QByteArray::fromHex("53fb027900720200000e0137007000006d000a00"),
        };
    }

    static const std::vector<ExpectedMetrics> getExpectedValues() {
        return {
            {0, 3, 6},
            {1, 9, 18},
            {2, 25, 48},
            {3, 55, 109},
        };
    }

    static std::pair<int, int> parseFrame(const QByteArray &frame) {
        if (frame.length() != 20) {
            return {-1, -1};
        }

        if ((uint8_t)frame[0] != 0x53 || (uint8_t)frame[1] != 0xfb || (uint8_t)frame[2] != 0x02) {
            return {-1, -1};
        }

        const int steps = (uint8_t)frame[11] | ((uint8_t)frame[12] << 8);
        const int footfalls = (uint8_t)frame[16] | ((uint8_t)frame[17] << 8);
        return {steps, footfalls};
    }
};

class Stairmaster8GParsingTest : public testing::Test {};

TEST_F(Stairmaster8GParsingTest, ParsePrimaryFrames) {
    auto frames = Stairmaster8GTestData::getPrimaryFrames();
    auto expectedValues = Stairmaster8GTestData::getExpectedValues();

    ASSERT_EQ(frames.size(), expectedValues.size());

    for (size_t i = 0; i < frames.size(); ++i) {
        auto [steps, footfalls] = Stairmaster8GTestData::parseFrame(frames[i]);

        EXPECT_EQ(steps, expectedValues[i].expectedStepCount) << "Frame " << i << ": steps mismatch";
        EXPECT_EQ(footfalls, expectedValues[i].expectedFootfalls) << "Frame " << i << ": footfalls mismatch";
        EXPECT_GE(footfalls, steps) << "Frame " << i << ": footfalls should not be less than steps";
    }
}

TEST_F(Stairmaster8GParsingTest, FrameValidation) {
    QByteArray shortFrame = QByteArray::fromHex("53fb02080061");
    auto [steps, footfalls] = Stairmaster8GTestData::parseFrame(shortFrame);
    EXPECT_EQ(steps, -1);
    EXPECT_EQ(footfalls, -1);

    QByteArray invalidStart = QByteArray::fromHex("00fb02080061010000f000030000000006000000");
    std::tie(steps, footfalls) = Stairmaster8GTestData::parseFrame(invalidStart);
    EXPECT_EQ(steps, -1);
    EXPECT_EQ(footfalls, -1);
}
