#include <gtest/gtest.h>

#include "devices/daumbike/daumbluetoothbike.h"

TEST(DaumBluetoothTest, MatchesExactFsIb50BluetoothName) {
    EXPECT_TRUE(daumbluetoothbike::matchesBluetoothName(QStringLiteral("FS-IB50")));
    EXPECT_TRUE(daumbluetoothbike::matchesBluetoothName(QStringLiteral("fs-ib50")));
    EXPECT_FALSE(daumbluetoothbike::matchesBluetoothName(QStringLiteral("FS-IB500")));
    EXPECT_FALSE(daumbluetoothbike::matchesBluetoothName(QStringLiteral("DAUM 8008 TRS 3")));
}
