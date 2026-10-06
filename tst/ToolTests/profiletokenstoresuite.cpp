#include "profiletokenstoresuite.h"

#include "profiletokenstore.h"

#include <QTemporaryDir>

namespace {
QTemporaryDir *settingsDirectory = nullptr;
QSettings *settings = nullptr;
}

void ProfileTokenStoreTestSuite::SetUp()
{
    settingsDirectory = new QTemporaryDir;
    ASSERT_TRUE(settingsDirectory->isValid());
    settings = new QSettings(settingsDirectory->filePath(QStringLiteral("settings.ini")),
                              QSettings::IniFormat);
}

void ProfileTokenStoreTestSuite::TearDown()
{
    delete settings;
    settings = nullptr;
    delete settingsDirectory;
    settingsDirectory = nullptr;
}

TEST_F(ProfileTokenStoreTestSuite, ScopedAccountsAreIsolated)
{
    ProfileTokenStore::save(*settings, QStringLiteral("strava_accesstoken"),
                            QStringLiteral("access-a"), QStringLiteral("athlete-a"), true);
    ProfileTokenStore::save(*settings, QStringLiteral("strava_accesstoken"),
                            QStringLiteral("access-b"), QStringLiteral("athlete-b"), true);

    EXPECT_EQ(ProfileTokenStore::value(*settings, QStringLiteral("strava_accesstoken"),
                                       QStringLiteral("athlete-a")).toString().toStdString(),
              "access-a");
    EXPECT_EQ(ProfileTokenStore::value(*settings, QStringLiteral("strava_accesstoken"),
                                       QStringLiteral("athlete-b")).toString().toStdString(),
              "access-b");
    EXPECT_EQ(settings->value(QStringLiteral("strava_accesstoken")).toString().toStdString(),
              "access-b");
}

TEST_F(ProfileTokenStoreTestSuite, ClearingOneAccountKeepsTheOther)
{
    ProfileTokenStore::save(*settings, QStringLiteral("garmin_access_token"),
                            QStringLiteral("access-a"), QStringLiteral("garmin-a"), true);
    ProfileTokenStore::save(*settings, QStringLiteral("garmin_access_token"),
                            QStringLiteral("access-b"), QStringLiteral("garmin-b"), true);

    ProfileTokenStore::remove(*settings, QStringLiteral("garmin_access_token"),
                              QStringLiteral("garmin-a"), true);

    EXPECT_TRUE(ProfileTokenStore::value(*settings, QStringLiteral("garmin_access_token"),
                                         QStringLiteral("garmin-a")).toString().isEmpty());
    EXPECT_EQ(ProfileTokenStore::value(*settings, QStringLiteral("garmin_access_token"),
                                       QStringLiteral("garmin-b")).toString().toStdString(),
              "access-b");
    EXPECT_TRUE(settings->value(QStringLiteral("garmin_access_token")).toString().isEmpty());
}

TEST_F(ProfileTokenStoreTestSuite, LegacyMirrorCanBeUsedWithoutAUserId)
{
    ProfileTokenStore::save(*settings, QStringLiteral("strava_refreshtoken"),
                            QStringLiteral("legacy-refresh"), QString(), true);

    EXPECT_EQ(ProfileTokenStore::value(*settings, QStringLiteral("strava_refreshtoken"),
                                       QString()).toString().toStdString(),
              "legacy-refresh");
    EXPECT_FALSE(ProfileTokenStore::hasAnyValue(*settings,
                                                QStringList() << QStringLiteral("strava_refreshtoken"),
                                                QStringLiteral("missing-user")));
}
