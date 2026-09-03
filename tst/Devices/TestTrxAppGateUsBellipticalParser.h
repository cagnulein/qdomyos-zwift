#pragma once

#include <gtest/gtest.h>
#include <QByteArray>

#include "devices/trxappgateusbelliptical/trxappgateusbelliptical.h"

TEST(TrxAppGateUsBellipticalParserTest, ToorxUsesCadenceFieldFromPacket) {
    const QByteArray packet = QByteArray::fromHex("f0b223010121011401130101010101010145020261");

    EXPECT_DOUBLE_EQ(trxappgateusbelliptical::toorxCadenceFromPacket(packet), 18.0);
}

TEST(TrxAppGateUsBellipticalParserTest, ToorxUsesSpeedFieldFromPacket) {
    const QByteArray packet = QByteArray::fromHex("f0b223010121011401130101010101010145020261");

    EXPECT_DOUBLE_EQ(trxappgateusbelliptical::toorxSpeedFromPacket(packet), 1.9);
}
