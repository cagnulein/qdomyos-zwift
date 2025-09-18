#include <gtest/gtest.h>

#include "devices/kettlerracersbike/kettlerhandshake.h"

namespace {
struct TestVector {
    const char *seedHex;
    const char *cipherHex;
};
}

TEST(KettlerHandshakeTest, KnownVectors)
{
    const TestVector vectors[] = {
        {"000152D2D787", "E2223C64325BAD88CA1390E7A0FED75A"},
        {"000160AC609A", "213276DEB7413E7D5D1A2C514362E56B"},
        {"000117E0D1F9", "29A52B6D31AEF7E172D89647CC7934B5"},
    };

    for (const auto &vec : vectors) {
        const auto seed = QByteArray::fromHex(vec.seedHex);
        const auto expected = QByteArray::fromHex(vec.cipherHex);
        const auto actual = kettler::computeHandshake(seed);
        EXPECT_EQ(actual, expected) << "Failed vector for seed " << vec.seedHex;
    }
}

TEST(KettlerHandshakeTest, RejectsUnexpectedSeedSize)
{
    EXPECT_TRUE(kettler::computeHandshake(QByteArray()).isEmpty());
    EXPECT_TRUE(kettler::computeHandshake(QByteArray::fromHex("0011")) .isEmpty());
    EXPECT_TRUE(kettler::computeHandshake(QByteArray(7, '\x00')).isEmpty());
}
