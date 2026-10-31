#include <gtest/gtest.h>

#include "devices/proformtreadmill/sport30protocol.h"

namespace {
std::string toHex(const ProFormSport30Protocol::AuthorizationPackets &packets) {
    constexpr char digits[] = "0123456789abcdef";
    std::string result;
    for (size_t packetIndex = 0; packetIndex < packets.size(); ++packetIndex) {
        if (packetIndex != 0) {
            result.push_back('\n');
        }
        for (size_t byteIndex = 0; byteIndex < packets[packetIndex].size(); ++byteIndex) {
            const auto value = packets[packetIndex][byteIndex];
            result.push_back(digits[value >> 4]);
            result.push_back(digits[value & 0x0f]);
            if (byteIndex + 1 < packets[packetIndex].size()) {
                result.push_back(':');
            }
        }
    }
    return result;
}
} // namespace

TEST(ProFormSport30ProtocolTest, EncodesNativeModeCommandAndChecksum) {
    const auto frame = ProFormSport30Protocol::makeStateCommand(0x03);
    const std::array<uint8_t, 20> expected = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x04, 0x09, 0x02, 0x02,
                                               0x00, 0x10, 0x03, 0x00, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00};
    EXPECT_EQ(frame, expected);

    for (const uint8_t state : {0x01, 0x02, 0x03, 0x04, 0x0d}) {
        const auto command = ProFormSport30Protocol::makeStateCommand(state);
        uint8_t checksum = 0;
        for (size_t i = 6; i <= 12; ++i) {
            checksum += command[i];
        }
        EXPECT_EQ(command[12], state);
        EXPECT_EQ(command[14], checksum) << "state=" << static_cast<unsigned>(state);
    }
}

TEST(ProFormSport30ProtocolTest, StartCommandPreservesCurrentInclineAndRecomputesChecksum) {
    const auto flat = ProFormSport30Protocol::makeStartCommand(0.0);
    const std::array<uint8_t, 20> expectedFlat = {0xff, 0x11, 0x02, 0x04, 0x02, 0x0d, 0x04, 0x0d, 0x02, 0x02,
                                                   0x03, 0x10, 0xa0, 0x00, 0x00, 0x00, 0x0a, 0x00, 0xd2, 0x00};
    EXPECT_EQ(flat, expectedFlat);

    const auto inclined = ProFormSport30Protocol::makeStartCommand(12.34);
    EXPECT_EQ(inclined[14], 0xd2);
    EXPECT_EQ(inclined[15], 0x04);
    uint8_t checksum = 0;
    for (size_t i = 6; i <= 17; ++i) {
        checksum += inclined[i];
    }
    EXPECT_EQ(inclined[18], checksum);
}

TEST(ProFormSport30ProtocolTest, ParsesThreeFramedAuthorizationPackets) {
    using ProFormSport30Protocol::AuthorizationPackets;
    const AuthorizationPackets expected = {{
        {0x00, 0x12, 0x02, 0x04, 0x02, 0x28, 0x04, 0x28, 0x90, 0x07, 0x01,
         0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa},
        {0x01, 0x12, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb,
         0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb},
        {0xff, 0x08, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
         0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc},
    }};

    AuthorizationPackets parsed{};
    ASSERT_TRUE(ProFormSport30Protocol::parseAuthorizationPackets(toHex(expected), parsed));
    EXPECT_EQ(parsed, expected);
}

TEST(ProFormSport30ProtocolTest, RejectsMalformedAuthorizationPackets) {
    using ProFormSport30Protocol::AuthorizationPackets;
    const AuthorizationPackets valid = {{
        {0x00, 0x12, 0x02, 0x04, 0x02, 0x28, 0x04, 0x28, 0x90, 0x07, 0x01,
         0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa},
        {0x01, 0x12, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb,
         0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb},
        {0xff, 0x08, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
         0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc},
    }};
    const std::string input = toHex(valid);
    AuthorizationPackets parsed{};

    EXPECT_FALSE(ProFormSport30Protocol::parseAuthorizationPackets(input.substr(0, input.size() - 2), parsed));
    std::string invalidMarker = input;
    invalidMarker.replace(0, 2, "02");
    EXPECT_FALSE(ProFormSport30Protocol::parseAuthorizationPackets(invalidMarker, parsed));
    EXPECT_FALSE(ProFormSport30Protocol::parseAuthorizationPackets(input + "zz", parsed));
}
