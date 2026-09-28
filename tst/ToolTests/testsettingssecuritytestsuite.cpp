#include <gtest/gtest.h>
#include <QStringList>

#include "qzsettings.h"

TEST(QZSettingsSecurityTest, FiltersSensitiveKeysUsedByStartupSettingsLog) {
    const QStringList sensitiveKeys = {
        QStringLiteral("peloton_password"),
        QStringLiteral("strava_refresh_token"),
        QStringLiteral("pzp_username"),
        QStringLiteral("user_email"),
        QStringLiteral("garmin_email"),
        QStringLiteral("garmin_device_serial"),
        QStringLiteral("ZWIFT_PASSWORD"),
        QStringLiteral("/MQTT_TOKEN"),
    };

    for (const QString &key : sensitiveKeys) {
        EXPECT_TRUE(QZSettings::isSensitiveSettingKey(key)) << key.toStdString();
    }
}

TEST(QZSettingsSecurityTest, AllowsNonSensitiveTemplateSettings) {
    const QStringList allowedKeys = {
        QStringLiteral("tile_speed_enabled"),
        QStringLiteral("miles_unit"),
        QStringLiteral("heart_rate_zone1"),
        QStringLiteral("ftp"),
    };

    for (const QString &key : allowedKeys) {
        EXPECT_FALSE(QZSettings::isSensitiveSettingKey(key)) << key.toStdString();
    }
}
