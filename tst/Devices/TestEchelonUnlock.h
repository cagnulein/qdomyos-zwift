#pragma once

#include <gtest/gtest.h>

#include "devices/echelonconnectsport/echelonunlock.h"

TEST(EchelonUnlockTest, BuildsMailExampleResponse) {
    const auto result = echelonunlock::responseForChallenge(QByteArray::fromHex("f0e0354412468122"));

    ASSERT_TRUE(result.valid());
    EXPECT_EQ(result.authValue, 63);
    EXPECT_EQ(result.response, QByteArray::fromHex("f0e0000063000033"));
}

TEST(EchelonUnlockTest, PreservesJavascriptDecimalPrefixParsing) {
    const auto result = echelonunlock::responseForChallenge(QByteArray::fromHex("f0e09c01240d11aa"));

    ASSERT_TRUE(result.valid());
    EXPECT_EQ(result.authValue, 4);
    EXPECT_EQ(result.response, QByteArray::fromHex("f0e00000040000d4"));
}

TEST(EchelonUnlockTest, MatchesRowerChallengeVector) {
    const auto result = echelonunlock::responseForChallenge(QByteArray::fromHex("f0e09800013140da"));

    ASSERT_TRUE(result.valid());
    EXPECT_EQ(result.authValue, 18);
    EXPECT_EQ(result.response, QByteArray::fromHex("f0e00000180000e8"));
}

TEST(EchelonUnlockTest, MatchesStrideChallengeVectors) {
    const auto first = echelonunlock::responseForChallenge(QByteArray::fromHex("f0e088293570678d"));
    const auto second = echelonunlock::responseForChallenge(QByteArray::fromHex("f0e0142910648304"));

    ASSERT_TRUE(first.valid());
    ASSERT_TRUE(second.valid());
    EXPECT_EQ(first.authValue, 10);
    EXPECT_EQ(first.response, QByteArray::fromHex("f0e00000100000e0"));
    EXPECT_EQ(second.authValue, 66);
    EXPECT_EQ(second.response, QByteArray::fromHex("f0e0000066000036"));
}

TEST(EchelonUnlockTest, RejectsMalformedChallenges) {
    EXPECT_EQ(echelonunlock::responseForChallenge(QByteArray::fromHex("f0e03544124681" )).error,
              echelonunlock::Error::InvalidLength);
    EXPECT_EQ(echelonunlock::responseForChallenge(QByteArray::fromHex("f1e0354412468122")).error,
              echelonunlock::Error::InvalidPrefix);
    EXPECT_EQ(echelonunlock::responseForChallenge(QByteArray::fromHex("f0e0fd3e6548d58d")).error,
              echelonunlock::Error::InvalidDecimalPrefix);
    EXPECT_EQ(echelonunlock::responseForChallenge(QByteArray::fromHex("f0e03544124600aa")).error,
              echelonunlock::Error::ZeroDivisor);
}

TEST(EchelonUnlockTest, BuildsActivationCommand) {
    EXPECT_EQ(echelonunlock::activationCommand(), QByteArray::fromHex("f0b00101a2"));
}
