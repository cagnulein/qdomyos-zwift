#include "TestTreadmillInclinationOverrides.h"

#include "qzsettings.h"

#include <QSettings>

void TreadmillInclinationOverrideTest::SetUp() {
    testSettings.qsettings.clear();
    testSettings.activate();
}

void TreadmillInclinationOverrideTest::TearDown() {
    testSettings.qsettings.clear();
    testSettings.deactivate();
}

TEST_F(TreadmillInclinationOverrideTest, LegacyMappingIsUnchangedWhenDeviceCommandEqualsRawRow) {
    QSettings settings;
    settings.setValue(QZSettings::treadmill_inclination_override_45, 6.5);
    settings.setValue(QZSettings::treadmill_inclination_override_50, 7.0);
    settings.setValue(QZSettings::treadmill_inclination_override_55, 7.5);
    // A serialized default must not activate mapped-state tracking merely because the key exists.
    settings.setValue(QZSettings::treadmill_inclination_override_device_command_50, 5.0);

    EXPECT_DOUBLE_EQ(treadmill.feedRawInclination(5.0), 7.0);
    EXPECT_DOUBLE_EQ(treadmill.treadmillInclinationOverrideReverse(7.0), 5.0);
    EXPECT_DOUBLE_EQ(treadmill.feedRawInclination(4.5), 6.5);
}

TEST_F(TreadmillInclinationOverrideTest, LegacyBoundaryValuesRemainUnchanged) {
    QSettings settings;
    for (int row = 0; row <= 30; ++row) {
        const int index = row * 5;
        const QString suffix = index == 0 ? QStringLiteral("0")
                                          : (index == 5 ? QStringLiteral("05") : QString::number(index));
        settings.setValue(QStringLiteral("treadmill_inclination_override_") + suffix,
                          2.0 + (16.0 * row / 30.0));
    }

    EXPECT_DOUBLE_EQ(treadmill.treadmillInclinationOverrideReverse(1.0), 2.0);
    EXPECT_DOUBLE_EQ(treadmill.treadmillInclinationOverrideReverse(20.0), 18.0);
}

TEST_F(TreadmillInclinationOverrideTest, AlexandraThreePercentUsesLogicalValueOfPhysicalCommand) {
    QSettings settings;
    settings.setValue(QZSettings::treadmill_inclination_override_10, 1.67);
    settings.setValue(QZSettings::treadmill_inclination_override_15, 2.50);
    settings.setValue(QZSettings::treadmill_inclination_override_20, 3.33);
    settings.setValue(QZSettings::treadmill_inclination_override_device_command_15, 2.0);
    settings.setValue(QZSettings::treadmill_inclination_override_device_command_20, 2.0);

    EXPECT_DOUBLE_EQ(treadmill.feedRawInclination(0.0), 0.0);
    EXPECT_DOUBLE_EQ(treadmill.treadmillInclinationOverrideReverse(3.0), 2.0);
    EXPECT_DOUBLE_EQ(treadmill.treadmillInclinationLogicalForDeviceCommand(2.0), 3.33);

    EXPECT_DOUBLE_EQ(treadmill.feedRawInclination(1.0), 3.33);
    EXPECT_DOUBLE_EQ(treadmill.rawInclinationValue(), 1.0);
    EXPECT_DOUBLE_EQ(treadmill.feedRawInclination(1.0), 3.33);
    EXPECT_DOUBLE_EQ(treadmill.rawInclinationValue(), 1.0);
}

TEST_F(TreadmillInclinationOverrideTest, AlexandraEightPercentUsesLogicalValueOfPhysicalCommand) {
    QSettings settings;
    settings.setValue(QZSettings::treadmill_inclination_override_45, 7.50);
    settings.setValue(QZSettings::treadmill_inclination_override_50, 8.33);
    settings.setValue(QZSettings::treadmill_inclination_override_device_command_45, 5.0);
    settings.setValue(QZSettings::treadmill_inclination_override_device_command_50, 5.0);

    EXPECT_DOUBLE_EQ(treadmill.feedRawInclination(3.0), 3.0);
    EXPECT_DOUBLE_EQ(treadmill.treadmillInclinationOverrideReverse(8.0), 5.0);
    EXPECT_DOUBLE_EQ(treadmill.treadmillInclinationLogicalForDeviceCommand(5.0), 8.33);

    // P30 physical level 5 reports raw 3, so feedback can remain unchanged after the command.
    EXPECT_DOUBLE_EQ(treadmill.feedRawInclination(3.0), 8.33);
    EXPECT_DOUBLE_EQ(treadmill.rawInclinationValue(), 3.0);
}

TEST_F(TreadmillInclinationOverrideTest, NonRowDeviceCommandUsesNearestSupportedRowWithoutInterpolation) {
    QSettings settings;
    settings.setValue(QZSettings::treadmill_inclination_override_20, 3.33);

    EXPECT_DOUBLE_EQ(treadmill.treadmillInclinationLogicalForDeviceCommand(2.2), 3.33);
}

TEST_F(TreadmillInclinationOverrideTest, UnchangedLossyFeedbackSettlesAfterBoundedSamples) {
    QSettings settings;
    settings.setValue(QZSettings::treadmill_inclination_override_10, 1.67);
    settings.setValue(QZSettings::treadmill_inclination_override_15, 2.50);
    settings.setValue(QZSettings::treadmill_inclination_override_20, 3.33);
    settings.setValue(QZSettings::treadmill_inclination_override_device_command_15, 2.0);

    EXPECT_DOUBLE_EQ(treadmill.feedRawInclination(1.0), 1.67);
    EXPECT_DOUBLE_EQ(treadmill.treadmillInclinationOverrideReverse(3.0), 2.0);
    EXPECT_DOUBLE_EQ(treadmill.feedRawInclination(1.0), 3.33);
    EXPECT_DOUBLE_EQ(treadmill.feedRawInclination(1.0), 3.33);
    EXPECT_DOUBLE_EQ(treadmill.feedRawInclination(1.0), 3.33);
}

TEST_F(TreadmillInclinationOverrideTest, ManualRawChangeAfterSettlingRestoresLegacyMapping) {
    QSettings settings;
    settings.setValue(QZSettings::treadmill_inclination_override_40, 6.67);
    settings.setValue(QZSettings::treadmill_inclination_override_45, 7.50);
    settings.setValue(QZSettings::treadmill_inclination_override_50, 8.33);
    settings.setValue(QZSettings::treadmill_inclination_override_device_command_45, 5.0);

    EXPECT_DOUBLE_EQ(treadmill.feedRawInclination(3.0), 3.0);
    EXPECT_DOUBLE_EQ(treadmill.treadmillInclinationOverrideReverse(8.0), 5.0);
    EXPECT_DOUBLE_EQ(treadmill.feedRawInclination(3.0), 8.33);
    EXPECT_DOUBLE_EQ(treadmill.feedRawInclination(3.0), 8.33);
    EXPECT_DOUBLE_EQ(treadmill.feedRawInclination(3.0), 8.33);

    EXPECT_DOUBLE_EQ(treadmill.feedRawInclination(4.0), 6.67);
    EXPECT_DOUBLE_EQ(treadmill.rawInclinationValue(), 4.0);
}
