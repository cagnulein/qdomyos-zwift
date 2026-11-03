#pragma once

#include <QByteArray>
#include <gtest/gtest.h>

#include "devices/wahookickruntreadmill/wahookickruntreadmill.h"

TEST(WahooKickrRunFtmsRegressionTest, ParsesActualInclinationFromStephanLoggedTreadmillData) {
    const auto onePercent = wahookickruntreadmill::parseFtmsTreadmillData(
        QByteArray::fromHex("0c002c018d00000a00ff7f"));
    const auto twoPercent = wahookickruntreadmill::parseFtmsTreadmillData(
        QByteArray::fromHex("0c002c018f00001400ff7f"));

    ASSERT_TRUE(onePercent.hasInclination);
    EXPECT_NEAR(onePercent.inclination, 1.0, 0.001);
    ASSERT_TRUE(twoPercent.hasInclination);
    EXPECT_NEAR(twoPercent.inclination, 2.0, 0.001);
}

TEST(WahooKickrRunFtmsRegressionTest, HandlesTreadmillDataWithoutAnInclinationField) {
    const auto data = wahookickruntreadmill::parseFtmsTreadmillData(
        QByteArray::fromHex("00002c01"));

    EXPECT_FALSE(data.hasInclination);
}

TEST(WahooKickrRunFtmsRegressionTest, EncodesSignedInclinationWithinKickrRunRange) {
    EXPECT_EQ(wahookickruntreadmill::encodeFtmsInclination(-3.0), QByteArray::fromHex("03e2ff"));
    EXPECT_EQ(wahookickruntreadmill::encodeFtmsInclination(-2.0), QByteArray::fromHex("03ecff"));
    EXPECT_EQ(wahookickruntreadmill::encodeFtmsInclination(15.0), QByteArray::fromHex("039600"));
}
