#include <gtest/gtest.h>
#include "garminconnect.h"
#include "homeform.h"
#include "qzsettings.h"
#include "simplecrypt.h"
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QTemporaryDir>

namespace {
const QString jay = QStringLiteral("garmin.com_jay@example.com");
const QString jan = QStringLiteral("garmin.com_jan@example.com");
const quint64 cryptKey = 123456789;

class ProfileAccountIsolationTestSuite : public testing::Test {
protected:
    QTemporaryDir directory;
    QString oldOrganization, oldApplication, oldIniRoot;
    QSettings::Format oldFormat;

    void SetUp() override {
        ASSERT_TRUE(directory.isValid());
        oldOrganization = QCoreApplication::organizationName();
        oldApplication = QCoreApplication::applicationName();
        oldFormat = QSettings::defaultFormat();
        QSettings probe(QSettings::IniFormat, QSettings::UserScope,
                        QStringLiteral("QZPathProbe"), QStringLiteral("Probe"));
        QDir root(QFileInfo(probe.fileName()).absolutePath());
        root.cdUp();
        oldIniRoot = root.absolutePath();
        QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, directory.path());
        QSettings::setDefaultFormat(QSettings::IniFormat);
        QCoreApplication::setOrganizationName(QStringLiteral("QZAccountTests"));
        QCoreApplication::setApplicationName(QStringLiteral("Isolation"));
        QSettings settings;
        settings.setValue(QZSettings::cryptoKeySettingsProfiles, cryptKey);
        settings.setValue(QZSettings::garmin_domain, QStringLiteral("garmin.com"));
    }

    void TearDown() override {
        QCoreApplication::setOrganizationName(oldOrganization);
        QCoreApplication::setApplicationName(oldApplication);
        QSettings::setDefaultFormat(oldFormat);
        QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, oldIniRoot);
    }

    void select(const QString &email) {
        QSettings().setValue(QZSettings::garmin_email, email);
    }

    void session(const QString &id, const QString &token) {
        QSettings settings;
        ProfileTokenStore::save(settings, QZSettings::garmin_access_token, token, id, false);
        ProfileTokenStore::save(settings, QZSettings::garmin_expires_at,
                                QDateTime::currentSecsSinceEpoch() + 3600, id, false);
    }

    void encrypted(QSettings &profile, const QString &key, const QString &value) {
        SimpleCrypt crypt(cryptKey);
        profile.setValue(key, crypt.encryptToString(value));
    }
};

TEST_F(ProfileAccountIsolationTestSuite, JanWithoutTokensNeverAdoptsJaysLegacySession) {
    session(QString(), QStringLiteral("jay-token"));
    select(QStringLiteral("jan@example.com"));
    GarminConnect client;
    EXPECT_TRUE(client.matchesCurrentUser());
    ASSERT_FALSE(client.isAuthenticated());
    EXPECT_FALSE(client.tryRefreshToken());
    EXPECT_FALSE(client.uploadFitFile(directory.filePath(QStringLiteral("missing.fit"))));
    EXPECT_FALSE(client.uploadActivity(QByteArray(), QStringLiteral("test.fit")));
    EXPECT_TRUE(ProfileTokenStore::value(QSettings(), QZSettings::garmin_access_token, jan).toString().isEmpty());
}

TEST_F(ProfileAccountIsolationTestSuite, ScopedSessionWinsOverForeignLegacyTokens) {
    session(QString(), QStringLiteral("jay-token"));
    session(jan, QStringLiteral("jan-token"));
    select(QStringLiteral("jan@example.com"));
    GarminConnect client;
    EXPECT_TRUE(client.isAuthenticated());
    EXPECT_EQ(QSettings().value(QZSettings::garmin_access_token).toString(), QStringLiteral("jan-token"));
}

TEST_F(ProfileAccountIsolationTestSuite, CachedClientCannotAuthenticateOrUploadForAnotherAccount) {
    session(jay, QStringLiteral("jay-token"));
    select(QStringLiteral("jay@example.com"));
    GarminConnect client;
    ASSERT_TRUE(client.isAuthenticated());
    select(QStringLiteral("jan@example.com"));
    ASSERT_FALSE(client.matchesCurrentUser());
    EXPECT_FALSE(client.isAuthenticated());
    EXPECT_FALSE(client.tryRefreshToken());
    EXPECT_FALSE(client.login(QStringLiteral("jan@example.com"), QStringLiteral("unused")));
    EXPECT_FALSE(client.uploadFitFile(directory.filePath(QStringLiteral("missing.fit"))));
    EXPECT_FALSE(client.uploadActivity(QByteArray(), QStringLiteral("test.fit")));
}

TEST_F(ProfileAccountIsolationTestSuite, OldClientLogoutCannotDeleteTheNewAccountsSession) {
    session(jay, QStringLiteral("jay-token"));
    session(jan, QStringLiteral("jan-token"));
    select(QStringLiteral("jay@example.com"));
    GarminConnect oldClient;
    select(QStringLiteral("jan@example.com"));
    GarminConnect currentClient;
    oldClient.logout();
    EXPECT_TRUE(currentClient.isAuthenticated());
    EXPECT_EQ(QSettings().value(QZSettings::garmin_access_token).toString(), QStringLiteral("jan-token"));
    EXPECT_EQ(ProfileTokenStore::value(QSettings(), QZSettings::garmin_access_token, jan).toString(), QStringLiteral("jan-token"));
    EXPECT_TRUE(ProfileTokenStore::value(QSettings(), QZSettings::garmin_access_token, jay).toString().isEmpty());
}

TEST_F(ProfileAccountIsolationTestSuite, GarminDomainAndEmailArePartOfTheSessionIdentity) {
    session(jay, QStringLiteral("jay-token"));
    select(QStringLiteral(" JAY@EXAMPLE.COM "));
    GarminConnect client;
    ASSERT_TRUE(client.isAuthenticated());
    QSettings().setValue(QZSettings::garmin_domain, QStringLiteral("garmin.cn"));
    EXPECT_FALSE(client.matchesCurrentUser());
    GarminConnect chinaClient;
    EXPECT_FALSE(chinaClient.isAuthenticated());
}

TEST_F(ProfileAccountIsolationTestSuite, DuplicatedProfileCannotRelabelGenericGarminTokens) {
    const QString path = directory.filePath(QStringLiteral("jan.qzs"));
    {
        QSettings profile(path, QSettings::IniFormat);
        profile.setValue(QZSettings::garmin_email, QStringLiteral("jan@example.com"));
        encrypted(profile, QZSettings::garmin_access_token, QStringLiteral("jay-token"));
        encrypted(profile, QZSettings::garmin_oauth1_token, QStringLiteral("jay-oauth1"));
        profile.setValue(QZSettings::garmin_expires_at, QDateTime::currentSecsSinceEpoch() + 3600);
        encrypted(profile, QZSettings::garmin_access_token + QStringLiteral("_") + jay, QStringLiteral("foreign-token"));
    }
    homeform::loadSettings(QUrl::fromLocalFile(path));
    GarminConnect client;
    EXPECT_FALSE(client.isAuthenticated());
    EXPECT_TRUE(ProfileTokenStore::value(QSettings(), QZSettings::garmin_access_token, jan).toString().isEmpty());
    EXPECT_TRUE(ProfileTokenStore::value(QSettings(), QZSettings::garmin_oauth1_token, jan).toString().isEmpty());
}

TEST_F(ProfileAccountIsolationTestSuite, SavedProfilesRoundTripTheirOwnAccountsAndPersonalValues) {
    const QStringList emails = {QStringLiteral("jay@example.com"), QStringLiteral("jan@example.com")};
    const QStringList ids = {jay, jan};
    for (int i = 0; i < 2; ++i) {
        QSettings profile(directory.filePath(QString::number(i) + QStringLiteral(".qzs")), QSettings::IniFormat);
        profile.setValue(QZSettings::garmin_email, emails[i]);
        profile.setValue(QZSettings::weight, 60 + i * 20);
        profile.setValue(QZSettings::android_documents_folder, false);
        encrypted(profile, QZSettings::garmin_access_token + QStringLiteral("_") + ids[i], ids[i]);
        profile.setValue(QZSettings::garmin_expires_at + QStringLiteral("_") + ids[i], QDateTime::currentSecsSinceEpoch() + 3600);
    }
    for (int i : {0, 1, 0}) {
        homeform::loadSettings(QUrl::fromLocalFile(directory.filePath(QString::number(i) + QStringLiteral(".qzs"))));
        GarminConnect client;
        EXPECT_TRUE(client.isAuthenticated());
        EXPECT_EQ(QSettings().value(QZSettings::garmin_access_token).toString(), ids[i]);
        EXPECT_EQ(QSettings().value(QZSettings::weight).toInt(), 60 + i * 20);
        EXPECT_FALSE(QSettings().value(QZSettings::android_documents_folder).toBool());
    }
}

TEST_F(ProfileAccountIsolationTestSuite, LegacyStravaProfileCannotInheritThePreviousAccount) {
    QSettings settings;
    settings.setValue(QZSettings::strava_current_user_id, QStringLiteral("jay"));
    settings.setValue(QZSettings::strava_accesstoken, QStringLiteral("jay-token"));
    settings.setValue(QZSettings::strava_refreshtoken, QStringLiteral("jay-refresh"));
    const QString path = directory.filePath(QStringLiteral("legacy.qzs"));
    {
        QSettings profile(path, QSettings::IniFormat);
        encrypted(profile, QZSettings::strava_accesstoken, QStringLiteral("ambiguous-token"));
        encrypted(profile, QZSettings::strava_refreshtoken, QStringLiteral("ambiguous-refresh"));
    }
    homeform::loadSettings(QUrl::fromLocalFile(path));
    EXPECT_TRUE(settings.value(QZSettings::strava_current_user_id).toString().isEmpty());
    EXPECT_TRUE(settings.value(QZSettings::strava_accesstoken).toString().isEmpty());
    EXPECT_TRUE(settings.value(QZSettings::strava_refreshtoken).toString().isEmpty());
}

TEST_F(ProfileAccountIsolationTestSuite, StravaProfileImportsOnlyItsOwnScopedAccessAndRefreshTokens) {
    const QString path = directory.filePath(QStringLiteral("strava.qzs"));
    {
        QSettings profile(path, QSettings::IniFormat);
        profile.setValue(QZSettings::strava_current_user_id, QStringLiteral("jan"));
        encrypted(profile, QZSettings::strava_accesstoken + QStringLiteral("_jan"), QStringLiteral("jan-token"));
        encrypted(profile, QZSettings::strava_refreshtoken + QStringLiteral("_jan"), QStringLiteral("jan-refresh"));
        encrypted(profile, QZSettings::strava_accesstoken + QStringLiteral("_jay"), QStringLiteral("foreign-token"));
        encrypted(profile, QZSettings::strava_accesstoken, QStringLiteral("legacy-foreign-token"));
    }
    homeform::loadSettings(QUrl::fromLocalFile(path));
    QSettings settings;
    EXPECT_EQ(settings.value(QZSettings::strava_current_user_id).toString(), QStringLiteral("jan"));
    EXPECT_EQ(ProfileTokenStore::value(settings, QZSettings::strava_accesstoken, QStringLiteral("jan")).toString(), QStringLiteral("jan-token"));
    EXPECT_EQ(ProfileTokenStore::value(settings, QZSettings::strava_refreshtoken, QStringLiteral("jan")).toString(), QStringLiteral("jan-refresh"));
    EXPECT_TRUE(ProfileTokenStore::value(settings, QZSettings::strava_accesstoken, QStringLiteral("jay")).toString().isEmpty());
    EXPECT_TRUE(settings.value(QZSettings::strava_accesstoken).toString().isEmpty());
}
}
