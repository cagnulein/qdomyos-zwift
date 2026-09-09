#pragma once

#include <gtest/gtest.h>

#include "devices/trxappgateusbbike/trxappgateusbbike.h"

TEST(ToorxSrx500ParserTest, ReadsInstantPowerFromIndoorBikeData) {
    double power = 0;

    const bool parsed = trxappgateusbbike::parseSrx500FtmsPower(
        QByteArray::fromHex("6402ae0b0a011300ca005600"), power);

    EXPECT_TRUE(parsed);
    EXPECT_DOUBLE_EQ(power, 202.0);
}

TEST(ToorxSrx500ParserTest, UsesAveragePowerWhenInstantPowerIsAbsent) {
    double power = 0;

    const bool parsed = trxappgateusbbike::parseSrx500FtmsPower(
        QByteArray::fromHex("8100c800"), power);

    EXPECT_TRUE(parsed);
    EXPECT_DOUBLE_EQ(power, 200.0);
}

TEST(ToorxSrx500ParserTest, RejectsIndoorBikeDataWithoutPower) {
    double power = 0;

    const bool parsed = trxappgateusbbike::parseSrx500FtmsPower(
        QByteArray::fromHex("0402ae0b0a01"), power);

    EXPECT_FALSE(parsed);
}