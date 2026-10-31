#include "devices/proformtreadmill/proformtreadmill.h"

#include <gtest/gtest.h>
#include <QByteArray>

namespace {
QList<QByteArray> expectedAuthorizationPackets() {
    return {QByteArray::fromHex("0012020402280428900701") + QByteArray(9, static_cast<char>(0xaa)),
            QByteArray::fromHex("0112") + QByteArray(18, static_cast<char>(0xbb)),
            QByteArray::fromHex("ff08") + QByteArray(18, static_cast<char>(0xcc))};
}

QByteArray validAuthorizationPacketInput() {
    const auto packets = expectedAuthorizationPackets();
    return packets.at(0).toHex(':') + '\n' + packets.at(1).toHex(':') + '\n' + packets.at(2).toHex(':');
}
} // namespace

TEST(ProFormSport30ProtocolTest, EncodesNativeModeCommandAndChecksum) {
    const QByteArray expected = QByteArray::fromHex("ff0d020402090409020200100300240000000000");
    EXPECT_EQ(proformtreadmill::sport30StateCommand(0x03), expected);

    for (const uint8_t state : {0x01, 0x02, 0x03, 0x04, 0x0d}) {
        const QByteArray command = proformtreadmill::sport30StateCommand(state);
        uint8_t checksum = 0;
        for (int i = 6; i <= 12; ++i) {
            checksum += static_cast<uint8_t>(command.at(i));
        }
        EXPECT_EQ(static_cast<uint8_t>(command.at(12)), state);
        EXPECT_EQ(static_cast<uint8_t>(command.at(14)), checksum) << "state=" << static_cast<unsigned>(state);
    }
}

TEST(ProFormSport30ProtocolTest, StartCommandPreservesCurrentInclineAndRecomputesChecksum) {
    const QByteArray flat = proformtreadmill::sport30StartCommand(0.0);
    EXPECT_EQ(flat, QByteArray::fromHex("ff110204020d040d02020310a00000000a00d200"));

    const QByteArray inclined = proformtreadmill::sport30StartCommand(12.34);
    EXPECT_EQ(static_cast<uint8_t>(inclined.at(14)), 0xd2);
    EXPECT_EQ(static_cast<uint8_t>(inclined.at(15)), 0x04);
    uint8_t checksum = 0;
    for (int i = 6; i <= 17; ++i) {
        checksum += static_cast<uint8_t>(inclined.at(i));
    }
    EXPECT_EQ(static_cast<uint8_t>(inclined.at(18)), checksum);
}

TEST(ProFormSport30ProtocolTest, ParsesThreeFramedAuthorizationPackets) {
    const auto expected = expectedAuthorizationPackets();
    QList<QByteArray> parsed;
    ASSERT_TRUE(proformtreadmill::parseSport30AuthorizationPackets(validAuthorizationPacketInput(), &parsed));
    EXPECT_EQ(parsed, expected);
}

TEST(ProFormSport30ProtocolTest, RejectsMalformedAuthorizationPackets) {
    const QByteArray input = validAuthorizationPacketInput();
    QList<QByteArray> parsed;

    EXPECT_FALSE(proformtreadmill::parseSport30AuthorizationPackets(input.left(input.size() - 2), &parsed));

    QByteArray invalidMarker = input;
    invalidMarker.replace(0, 2, "02");
    EXPECT_FALSE(proformtreadmill::parseSport30AuthorizationPackets(invalidMarker, &parsed));
    EXPECT_FALSE(proformtreadmill::parseSport30AuthorizationPackets(input + "zz", &parsed));
    EXPECT_FALSE(proformtreadmill::parseSport30AuthorizationPackets(input, nullptr));
}