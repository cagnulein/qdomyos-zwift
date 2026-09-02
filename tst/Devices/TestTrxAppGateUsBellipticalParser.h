#pragma once

#include <gtest/gtest.h>
#include <QByteArray>

#include "devices/trxappgateusbelliptical/trxappgateusbelliptical.h"

TEST(TrxAppGateUsBellipticalParserTest, Toorx0095UsesByteFiveAsCadence) {
    const QByteArray packet = QByteArray::fromHex("f0b22301073a010101010101010101010101020217");

    EXPECT_DOUBLE_EQ(trxappgateusbelliptical::toorx0095CadenceFromPacket(packet), 58.0);
}

TEST(TrxAppGateUsBellipticalParserTest, Toorx0095SpeedIsCalculatedFromCadenceAndRatio) {
    EXPECT_NEAR(trxappgateusbelliptical::toorx0095SpeedFromCadence(58.0, 0.33), 19.14, 0.000001);
}
