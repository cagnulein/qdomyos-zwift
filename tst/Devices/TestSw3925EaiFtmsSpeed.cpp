#include <QByteArray>
#include <gtest/gtest.h>

#include "devices/horizontreadmill/horizontreadmill.h"

TEST(Sw3925EaiFtmsSpeedRegressionTest, UsesTheObservedHighByteTenthsQuirkOnlyForThisModel) {
    EXPECT_EQ(horizontreadmill::encodeFtmsTargetSpeed(11.0, true), QByteArray::fromHex("02006e"));
    EXPECT_EQ(horizontreadmill::encodeFtmsTargetSpeed(6.0, true), QByteArray::fromHex("02003c"));
    EXPECT_EQ(horizontreadmill::encodeFtmsTargetSpeed(16.0, true), QByteArray::fromHex("0200a0"));
    EXPECT_EQ(horizontreadmill::encodeFtmsTargetSpeed(8.0, true), QByteArray::fromHex("020050"));
}

TEST(Sw3925EaiFtmsSpeedRegressionTest, KeepsStandardFtmsLittleEndianHundredthsForOtherModels) {
    EXPECT_EQ(horizontreadmill::encodeFtmsTargetSpeed(11.0, false), QByteArray::fromHex("024c04"));
    EXPECT_EQ(horizontreadmill::encodeFtmsTargetSpeed(6.0, false), QByteArray::fromHex("025802"));
}

TEST(Sw3925EaiFtmsSpeedRegressionTest, MatchesOnlyTheObservedSw3925EaiAdvertisedNamePrefix) {
    EXPECT_TRUE(horizontreadmill::isSw3925EaiModel(QStringLiteral("SW3925EAI-0304")));
    EXPECT_TRUE(horizontreadmill::isSw3925EaiModel(QStringLiteral("sw3925eai-9999")));
    EXPECT_FALSE(horizontreadmill::isSw3925EaiModel(QStringLiteral("SW3925EAIX-0304")));
    EXPECT_FALSE(horizontreadmill::isSw3925EaiModel(QStringLiteral("SW3925-0304")));
}
