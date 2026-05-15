#pragma once

#include <gtest/gtest.h>
#include <QByteArray>
#include <vector>

class Schwinn411510EParserRegressionTest : public testing::Test {
protected:
    struct ParsedFrame {
        uint16_t strideCount = 0;
        uint16_t resistance = 0;
        uint16_t instantPower = 0;
    };

    static uint16_t readU16(const QByteArray &frame, int index) {
        return (uint16_t)((uint8_t)frame.at(index)) |
               ((uint16_t)((uint8_t)frame.at(index + 1)) << 8);
    }

    static uint32_t readFlags(const QByteArray &frame) {
        return (uint32_t)((uint8_t)frame.at(0)) |
               ((uint32_t)((uint8_t)frame.at(1)) << 8) |
               ((uint32_t)((uint8_t)frame.at(2)) << 16);
    }

    static ParsedFrame parseFrame(const QByteArray &frame, bool consumeAvgSpeedField) {
        ParsedFrame parsed;
        const uint32_t flags = readFlags(frame);
        const bool moreData = (flags & (1u << 0)) != 0;
        const bool avgSpeed = (flags & (1u << 1)) != 0;
        const bool totDistance = (flags & (1u << 2)) != 0;
        const bool strideCount = (flags & (1u << 4)) != 0;
        const bool resistanceLvl = (flags & (1u << 7)) != 0;
        const bool instantPower = (flags & (1u << 8)) != 0;

        int index = 3;

        if (!moreData) {
            index += 2;
        }

        if (avgSpeed && consumeAvgSpeedField) {
            index += 2;
        }

        if (totDistance) {
            index += 3;
        }

        if (strideCount) {
            parsed.strideCount = readU16(frame, index);
            index += 2;
        }

        if (resistanceLvl) {
            parsed.resistance = readU16(frame, index);
            index += 2;
        }

        if (instantPower) {
            parsed.instantPower = readU16(frame, index);
        }

        return parsed;
    }

    static std::vector<QByteArray> getLogFrames() {
        return {
            QByteArray::fromHex("966f00ae0291010300002200a00061003d0001008c0106581c005317"),
            QByteArray::fromHex("966f00ae0291010300002200a00061003d0002008c0106581c005317"),
            QByteArray::fromHex("966f00ae0292010300002200a00061003d0002008c0106581c005317"),
            QByteArray::fromHex("966f00af0292010300002400a00061003d0002008c0106581c005317"),
        };
    }
};

TEST_F(Schwinn411510EParserRegressionTest, AvgSpeedMustBeConsumedToKeepFieldAlignment) {
    const QByteArray frame = getLogFrames().front();

    const ParsedFrame buggy = parseFrame(frame, false);
    const ParsedFrame fixed = parseFrame(frame, true);

    EXPECT_EQ(buggy.strideCount, 0);
    EXPECT_GT(fixed.strideCount, 0);

    // Missing the avgSpeed field shifts every following metric left by 2 bytes.
    EXPECT_EQ(buggy.resistance, fixed.strideCount);
    EXPECT_EQ(buggy.instantPower, fixed.resistance);
}

TEST_F(Schwinn411510EParserRegressionTest, RealLogFramesRecoverStrideCountOnlyWhenAvgSpeedIsConsumed) {
    const auto frames = getLogFrames();

    std::vector<uint16_t> buggyStrideCounts;
    std::vector<uint16_t> fixedStrideCounts;

    for (const QByteArray &frame : frames) {
        buggyStrideCounts.push_back(parseFrame(frame, false).strideCount);
        fixedStrideCounts.push_back(parseFrame(frame, true).strideCount);
    }

    EXPECT_EQ(buggyStrideCounts, (std::vector<uint16_t>{0, 0, 0, 0}));
    EXPECT_EQ(fixedStrideCounts, (std::vector<uint16_t>{34, 34, 34, 36}));
}
