#include <gtest/gtest.h>

#include "homeform.h"

TEST(TargetZoneTile, WritableStateNotifiesOnlyOnChanges) {
    DataObject tile(QString(), QString(), QStringLiteral("Z2"), true,
                    QStringLiteral("target_zone"), 48, 16);
    int notifications = 0;
    QObject::connect(&tile, &DataObject::writableChanged,
                     [&](bool) { ++notifications; });

    // Bike target zones remain adjustable.
    EXPECT_TRUE(tile.property("writable").toBool());
    tile.setWritable(true);
    EXPECT_EQ(0, notifications);

    // The rowing effort tile hides the +/- controls in both QML layouts.
    tile.setWritable(false);
    EXPECT_FALSE(tile.property("writable").toBool());
    EXPECT_EQ(1, notifications);
    tile.setWritable(false);
    EXPECT_EQ(1, notifications);

    // Switching back to a bike restores the controls.
    tile.setWritable(true);
    EXPECT_TRUE(tile.property("writable").toBool());
    EXPECT_EQ(2, notifications);
}

TEST(TargetZoneTile, RemovesStaleWattRangeWithoutChangingEffortColor) {
    DataObject tile(QString(), QString(), QStringLiteral("Easy"), false,
                    QStringLiteral("target_zone"), 48, 16);
    tile.setSecondLine(QStringLiteral("0-77W 0%"));
    tile.setValueFontColor(QStringLiteral("limegreen"));

    tile.setSecondLine(QString());

    EXPECT_TRUE(tile.secondLine().isEmpty());
    EXPECT_EQ(QStringLiteral("Easy"), tile.value());
    EXPECT_EQ(QStringLiteral("limegreen"), tile.valueFontColor());
}
