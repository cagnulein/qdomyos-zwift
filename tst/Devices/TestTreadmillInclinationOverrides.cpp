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

TEST_F(TreadmillInclinationOverrideTest, LegacyReverseMappingStillReturnsRawRow) {
    QSettings settings;
    settings.setValue(QZSettings::treadmill_inclination_override_15, 2.5);
    settings.setValue(QZSettings::treadmill_inclination_override_20, 3.0);

    EXPECT_DOUBLE_EQ(treadmill.treadmillInclinationOverrideReverse(2.5), 1.5);
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

TEST_F(TreadmillInclinationOverrideTest, CustomizedDeviceCommandsReplaceSelectedRawRows) {
    QSettings settings;
    settings.setValue(QZSettings::treadmill_inclination_override_15, 2.5);
    settings.setValue(QZSettings::treadmill_inclination_override_20, 3.0);
    settings.setValue(QZSettings::treadmill_inclination_override_device_command_15, 2.0);
    settings.setValue(QZSettings::treadmill_inclination_override_45, 7.5);
    settings.setValue(QZSettings::treadmill_inclination_override_50, 8.0);
    settings.setValue(QZSettings::treadmill_inclination_override_device_command_45, 5.0);

    EXPECT_DOUBLE_EQ(treadmill.treadmillInclinationOverrideReverse(2.5), 2.0);
    EXPECT_DOUBLE_EQ(treadmill.treadmillInclinationOverrideReverse(7.5), 5.0);
}

TEST_F(TreadmillInclinationOverrideTest, RemembersLogicalValueForCompatibleLossyFeedback) {
    QSettings settings;
    settings.setValue(QZSettings::treadmill_inclination_override_10, 1.67);
    settings.setValue(QZSettings::treadmill_inclination_override_15, 3.33);
    settings.setValue(QZSettings::treadmill_inclination_override_20, 4.0);
    settings.setValue(QZSettings::treadmill_inclination_override_device_command_15, 2.0);

    EXPECT_DOUBLE_EQ(treadmill.feedRawInclination(0.0), 0.0);
    EXPECT_DOUBLE_EQ(treadmill.treadmillInclinationOverrideReverse(3.33), 2.0);

    EXPECT_DOUBLE_EQ(treadmill.feedRawInclination(1.0), 3.33);
    EXPECT_DOUBLE_EQ(treadmill.rawInclinationValue(), 1.0);

    EXPECT_DOUBLE_EQ(treadmill.feedRawInclination(2.0), 4.0);
    EXPECT_DOUBLE_EQ(treadmill.rawInclinationValue(), 2.0);
}
