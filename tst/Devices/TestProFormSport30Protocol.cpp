#include "devices/proformtreadmill/proformtreadmill.h"

#include <gtest/gtest.h>
#include <QByteArray>


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
