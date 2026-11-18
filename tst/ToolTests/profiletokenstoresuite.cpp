#include "profiletokenstoresuite.h"

#include "profiletokenstore.h"
#include "authutils.h"

#include <QJsonDocument>

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

TEST_F(ProfileTokenStoreTestSuite, StravaPrefersExactStringIdAndKeepsAdjacentAccountsSeparate)
{
    const auto first = QJsonDocument::fromJson(
        R"({"id":9007199254740992,"id_str":"9007199254740992"})").object();
    const auto second = QJsonDocument::fromJson(
        R"({"id":9007199254740993,"id_str":"9007199254740993"})").object();
    const QString firstId = stravaAthleteIdFromJson(first);
    const QString secondId = stravaAthleteIdFromJson(second);
    ASSERT_EQ(firstId.toStdString(), "9007199254740992");
    ASSERT_EQ(secondId.toStdString(), "9007199254740993");
    EXPECT_EQ(stravaAthleteIdFromJson(QJsonDocument::fromJson(
        R"({"id":9223372036854775807,"id_str":"9223372036854775807"})").object()).toStdString(),
        "9223372036854775807");

    const QString key = QStringLiteral("strava_accesstoken");
    ProfileTokenStore::save(*settings, key, QStringLiteral("access-a"), firstId, true);
    ProfileTokenStore::save(*settings, key, QStringLiteral("access-b"), secondId, true);
    EXPECT_EQ(ProfileTokenStore::value(*settings, key, firstId).toString().toStdString(), "access-a");
    EXPECT_EQ(ProfileTokenStore::value(*settings, key, secondId).toString().toStdString(), "access-b");
}

TEST_F(ProfileTokenStoreTestSuite, StravaAcceptsLegacySafeIdsButRejectsAmbiguousOrInvalidIds)
{
    const auto id = [](const QByteArray &json) {
        return stravaAthleteIdFromJson(QJsonDocument::fromJson(json).object());
    };
    EXPECT_EQ(id(R"({"id":123456789})").toStdString(), "123456789");
    EXPECT_EQ(id(R"({"id":"123456789"})").toStdString(), "123456789");
    EXPECT_EQ(id(R"({"id":9007199254740991})").toStdString(), "9007199254740991");
    EXPECT_TRUE(id(R"({"id":9007199254740992})").isEmpty());
    EXPECT_TRUE(id(R"({"id":9007199254740993})").isEmpty());
    EXPECT_TRUE(id(R"({"id":9223372036854775807})").isEmpty());
    EXPECT_TRUE(id(R"({"id":123.5})").isEmpty());
    EXPECT_TRUE(id(R"({"id":-1})").isEmpty());
    EXPECT_TRUE(id(R"({"id":0})").isEmpty());
    EXPECT_TRUE(id(R"({"id":123,"id_str":"invalid"})").isEmpty());
    EXPECT_TRUE(id(R"({"id":123,"id_str":123})").isEmpty());
    EXPECT_TRUE(id(R"({"id_str":"18446744073709551616"})").isEmpty());
    EXPECT_TRUE(id(R"({"id_str":"0"})").isEmpty());
    EXPECT_TRUE(id(R"({})").isEmpty());
}
