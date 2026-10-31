#include "devices/proformbike/proformbike.h"

#include <gtest/gtest.h>

#include <string>

namespace {

std::string toHex(const std::vector<std::uint8_t> &bytes) {
    static const char digits[] = "0123456789abcdef";
    std::string result;
    result.reserve(bytes.size() * 2);
    for (std::uint8_t byte : bytes) {
        result.push_back(digits[byte >> 4U]);
        result.push_back(digits[byte & 0x0fU]);
    }
    return result;
}

TEST(NordictrackVx450ResistanceCommand, EncodesAllCapturedLevelsWithChecksum) {
    const std::array<std::uint8_t, 11> fixedPrefix = {{0xff, 0x0d, 0x02, 0x04, 0x02, 0x09,
                                                       0x07, 0x09, 0x02, 0x01, 0x04}};

    for (int level = 1; level <= 20; ++level) {
        proformbikeprotocol::Vx450ResistancePacket packet = {};
        ASSERT_TRUE(proformbikeprotocol::buildVx450ResistancePacket(level, &packet));

        for (std::size_t i = 0; i < fixedPrefix.size(); ++i) {
            EXPECT_EQ(fixedPrefix[i], packet[i]) << "level=" << level;
        }

        const std::uint16_t expectedRaw = static_cast<std::uint16_t>(level * 500 - 1);
        const std::uint16_t actualRaw = static_cast<std::uint16_t>(packet[11]) |
                                        (static_cast<std::uint16_t>(packet[12]) << 8);
        EXPECT_EQ(expectedRaw, actualRaw) << "level=" << level;
        EXPECT_EQ(0, packet[13]) << "level=" << level;

        std::uint8_t checksum = 0;
        for (std::size_t i = 6; i <= 13; ++i) {
            checksum = static_cast<std::uint8_t>(checksum + packet[i]);
        }
        EXPECT_EQ(checksum, packet[14]) << "level=" << level;

        for (std::size_t i = 15; i < packet.size(); ++i) {
            EXPECT_EQ(0, packet[i]) << "level=" << level << " byte=" << i;
        }
    }
}

TEST(NordictrackVx450ResistanceCommand, MatchesCapturedPayloadVectors) {
    struct CapturedVector {
        int level;
        std::uint16_t rawResistance;
        std::uint8_t checksum;
    };
    const CapturedVector vectors[] = {{1, 499, 0x0b}, {7, 3499, 0xcf}, {20, 9999, 0x4d}};

    for (const CapturedVector &vector : vectors) {
        proformbikeprotocol::Vx450ResistancePacket packet = {};
        ASSERT_TRUE(proformbikeprotocol::buildVx450ResistancePacket(vector.level, &packet));
        const std::uint16_t rawResistance = static_cast<std::uint16_t>(packet[11]) |
                                            (static_cast<std::uint16_t>(packet[12]) << 8);
        EXPECT_EQ(vector.rawResistance, rawResistance) << "level=" << vector.level;
        EXPECT_EQ(vector.checksum, packet[14]) << "level=" << vector.level;
    }
}

TEST(NordictrackVx450ResistanceCommand, RejectsLevelsOutsideHardwareRange) {
    proformbikeprotocol::Vx450ResistancePacket packet = {};
    EXPECT_FALSE(proformbikeprotocol::buildVx450ResistancePacket(0, &packet));
    EXPECT_FALSE(proformbikeprotocol::buildVx450ResistancePacket(21, &packet));
    EXPECT_FALSE(proformbikeprotocol::buildVx450ResistancePacket(1, nullptr));
}

TEST(NordictrackVx450ResistanceCommand, MatchesCapturedInitializationSequence) {
    const char *expected[] = {
        "fe020802",
        "ff08020402040204818700000000000000000000",
        "fe020802",
        "ff08020402040704808b00000000000000000000",
        "fe020802",
        "ff08020402040704889300000000000000000000",
        "fe020b02",
        "ff0b020402070207820000008b00000000000000",
        "fe020a02",
        "ff0a0204020602068400008c0000000000000000",
        "fe020802",
        "ff08020402040204959b00000000000000000000",
        "fe022c04",
        "0012020402280728900400693815e0b9907d5839",
        "011218f5d0c9a08d986978754059505d58595855",
        "ff08504940600200005000000000000000000000",
        "fe021303",
        "00120204020f070f020009001000c01c4c0000e0",
        "ff01390000000000000000000000000000000000",
        "fe021303",
        "00120204020f070f0200093c9671001040400080",
        "ff01740000000000000000000000000000000000",
        "fe021102",
        "ff110204020d070d020500000000085802007d00",
        "fe020b02",
        "ff0b020402070707020001809100000000000000",
        "fe021102",
        "ff110204020d070d020500000000085802007d00",
        "fe021002",
        "ff100204020c070c020400000002e41f001e0000",
    };
    const std::uint16_t expectedDelays[] = {0, 205, 0, 225, 0, 115, 0, 295, 0, 225, 0, 200, 0, 0, 0,
                                             265, 0, 0, 230, 25, 15, 320, 0, 135, 0, 270, 45, 180, 0, 0};
    const std::vector<proformbikeprotocol::Vx450InitFrame> &frames =
        proformbikeprotocol::vx450InitializationFrames();
    ASSERT_EQ(sizeof(expected) / sizeof(expected[0]), frames.size());
    ASSERT_EQ(sizeof(expectedDelays) / sizeof(expectedDelays[0]), frames.size());
    for (std::size_t i = 0; i < frames.size(); ++i) {
        EXPECT_EQ(expected[i], toHex(frames[i].bytes)) << "frame=" << i;
        EXPECT_EQ(expectedDelays[i], frames[i].delayAfterMs) << "frame=" << i;
    }
}

TEST(NordictrackVx450ResistanceCommand, MatchesCapturedPollingCycleAndResistancePreamble) {
    const char *expected[] = {
        "fe021303",
        "00120204020f070f0200093c9671001040400080",
        "ff01740000000000000000000000000000000000",
        "fe020b02",
        "ff0b020402070707020001809100000000000000",
    };
    const std::vector<proformbikeprotocol::Vx450Frame> &frames = proformbikeprotocol::vx450PollingFrames();
    ASSERT_EQ(sizeof(expected) / sizeof(expected[0]), frames.size());
    for (std::size_t i = 0; i < frames.size(); ++i) {
        EXPECT_EQ(expected[i], toHex(frames[i])) << "frame=" << i;
    }
    EXPECT_EQ("fe020d02", toHex(proformbikeprotocol::vx450ResistancePreamble()));
}

TEST(NordictrackVx450ResistanceCommand, ParsesCapturedTwoCResistanceTelemetry) {
    const proformbikeprotocol::Vx450Frame level6 = {
        0x00, 0x12, 0x01, 0x04, 0x02, 0x2c, 0x07, 0x2c, 0x02, 0x02,
        0xb7, 0x0b, 0x00, 0x00, 0x3b, 0x00, 0x00, 0x00, 0x25, 0x00};
    const proformbikeprotocol::Vx450Frame level20 = {
        0x00, 0x12, 0x01, 0x04, 0x02, 0x2c, 0x07, 0x2c, 0x02, 0x02,
        0x0f, 0x27, 0xa5, 0x00, 0x31, 0x01, 0x00, 0x00, 0x31, 0x00};
    std::uint16_t level = 0;
    ASSERT_TRUE(proformbikeprotocol::parseVx450ResistanceTelemetry(level6.data(), level6.size(), &level));
    EXPECT_EQ(6, level);
    ASSERT_TRUE(proformbikeprotocol::parseVx450ResistanceTelemetry(level20.data(), level20.size(), &level));
    EXPECT_EQ(20, level);

    proformbikeprotocol::Vx450Frame legacy32 = level6;
    legacy32[5] = 0x32;
    legacy32[7] = 0x32;
    EXPECT_FALSE(proformbikeprotocol::parseVx450ResistanceTelemetry(legacy32.data(), legacy32.size(), &level));

    proformbikeprotocol::Vx450Frame unrelated = level6;
    unrelated[5] = 0x13;
    unrelated[7] = 0x13;
    EXPECT_FALSE(proformbikeprotocol::parseVx450ResistanceTelemetry(unrelated.data(), unrelated.size(), &level));

    proformbikeprotocol::Vx450Frame wrongResistanceValue = level6;
    wrongResistanceValue[10] = 0xb8; // 3000 is not the captured level-6 encoding (2999).
    EXPECT_FALSE(proformbikeprotocol::parseVx450ResistanceTelemetry(wrongResistanceValue.data(),
                                                                     wrongResistanceValue.size(), &level));
    EXPECT_FALSE(proformbikeprotocol::parseVx450ResistanceTelemetry(nullptr, level6.size(), &level));
    EXPECT_FALSE(proformbikeprotocol::parseVx450ResistanceTelemetry(level6.data(), level6.size(), nullptr));
}

} // namespace
