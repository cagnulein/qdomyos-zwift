#include "profileregressiontestsuite.h"

#include "homeform.h"
#include "garminconnect.h"
#include "profiletokenstore.h"
#include "qzsettings.h"

#include <QDir>
#include <QFileInfo>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QProcess>
#include <QProcessEnvironment>
#include <QUrl>
#include <QDateTime>

namespace {
QTemporaryDir *root = nullptr;
QString originalCwd;
bool usingExternalRoot = false;

const QString janGarminId = QStringLiteral("garmin.com_jan@example.invalid");
const QString jayGarminId = QStringLiteral("garmin.com_jay@example.invalid");
const QString sameGarminId = QStringLiteral("garmin.com_same@example.invalid");

struct ProfileValues {
    QString name;
    int weight;
    int ftp;
    int age;
    QString garminEmail;
    bool garminEnabled;
    QString tokenId;
    QString commonEmail;
};

QString profilePath(const QString &name)
{
    return homeform::getProfileDir() + QLatin1Char('/') + name + QStringLiteral(".qzs");
}

QString userIdFor(const ProfileValues &profile)
{
    if (profile.garminEmail.isEmpty())
        return {};
    QString id = QStringLiteral("garmin.com_") + profile.garminEmail;
    id.replace(QRegularExpression(QStringLiteral("[^A-Za-z0-9_.@-]")), QStringLiteral("_"));
    return id;
}

void clearGlobalSettings()
{
    QSettings settings;
    const QVariant cryptoKey = settings.value(QZSettings::cryptoKeySettingsProfiles);
    settings.clear();
    if (cryptoKey.isValid())
        settings.setValue(QZSettings::cryptoKeySettingsProfiles, cryptoKey);
    settings.sync();
}

void setProfileValues(const ProfileValues &profile)
{
    QSettings settings;
    settings.setValue(QZSettings::profile_name, profile.name);
    settings.setValue(QZSettings::weight, profile.weight);
    settings.setValue(QZSettings::ftp, profile.ftp);
    settings.setValue(QZSettings::age, profile.age);
    settings.setValue(QZSettings::user_email, profile.commonEmail);
    settings.setValue(QZSettings::garmin_email, profile.garminEmail);
    settings.setValue(QZSettings::garmin_domain, QStringLiteral("garmin.com"));
    settings.setValue(QZSettings::garmin_upload_enabled, profile.garminEnabled);

    const QString id = userIdFor(profile);
    if (profile.tokenId.isEmpty()) {
        settings.remove(ProfileTokenStore::scopedKey(QZSettings::garmin_access_token, id));
        settings.remove(ProfileTokenStore::scopedKey(QZSettings::garmin_refresh_token, id));
        settings.remove(ProfileTokenStore::scopedKey(QZSettings::garmin_token_type, id));
        settings.remove(ProfileTokenStore::scopedKey(QZSettings::garmin_expires_at, id));
        settings.remove(ProfileTokenStore::scopedKey(QZSettings::garmin_refresh_token_expires_at, id));
        settings.remove(QZSettings::garmin_access_token);
        settings.remove(QZSettings::garmin_refresh_token);
        settings.remove(QZSettings::garmin_token_type);
        settings.remove(QZSettings::garmin_expires_at);
        settings.remove(QZSettings::garmin_refresh_token_expires_at);
    } else {
        const qint64 expiresAt = QDateTime::currentSecsSinceEpoch() + 3600;
        ProfileTokenStore::save(settings, QZSettings::garmin_access_token,
                                QStringLiteral("access-") + profile.tokenId, id, true);
        ProfileTokenStore::save(settings, QZSettings::garmin_refresh_token,
                                QStringLiteral("refresh-") + profile.tokenId, id, true);
        ProfileTokenStore::save(settings, QZSettings::garmin_token_type, QStringLiteral("Bearer"), id, true);
        ProfileTokenStore::save(settings, QZSettings::garmin_expires_at, expiresAt, id, true);
        ProfileTokenStore::save(settings, QZSettings::garmin_refresh_token_expires_at, expiresAt, id, true);
    }
    settings.sync();
}

void setProfileFieldsWithoutGarminTokenChanges(const ProfileValues &profile)
{
    QSettings settings;
    settings.setValue(QZSettings::profile_name, profile.name);
    settings.setValue(QZSettings::weight, profile.weight);
    settings.setValue(QZSettings::ftp, profile.ftp);
    settings.setValue(QZSettings::age, profile.age);
    settings.setValue(QZSettings::user_email, profile.commonEmail);
    settings.setValue(QZSettings::garmin_email, profile.garminEmail);
    settings.setValue(QZSettings::garmin_domain, QStringLiteral("garmin.com"));
    settings.setValue(QZSettings::garmin_upload_enabled, profile.garminEnabled);
    settings.sync();
}

void saveProfileReal(const QString &name)
{
    // saveProfile() only uses static/path/settings services. Avoid the unrelated
    // Bluetooth/QML startup graph while invoking the production implementation.
    void *storage = ::operator new(sizeof(homeform));
    auto *form = static_cast<homeform *>(storage);
    form->saveProfile(name);
    ::operator delete(storage);

    QDir dir(homeform::getProfileDir());
    const QFileInfoList files = dir.entryInfoList(QStringList() << QStringLiteral("*.qzs"),
                                                   QDir::Files, QDir::Name);
    qInfo().noquote() << "SAVE profile=" << name
                      << "settings.profile_name=" << QSettings().value(QZSettings::profile_name).toString()
                      << "target=" << profilePath(name)
                      << "keys=" << QSettings(profilePath(name), QSettings::IniFormat).allKeys().join(QStringLiteral(","))
                      << "files=" << [&files]() {
                             QStringList result;
                             for (const QFileInfo &file : files)
                                 result << file.fileName() + QStringLiteral("(") + QString::number(file.size()) + QLatin1Char(')');
                             return result.join(QStringLiteral(","));
                         }()
                      << "Jan.exists=" << QFileInfo::exists(profilePath(QStringLiteral("Jan")))
                      << "Jay.exists=" << QFileInfo::exists(profilePath(QStringLiteral("Jay")));
    ASSERT_TRUE(QFileInfo::exists(profilePath(name)));
}

void loadProfileReal(const QString &name)
{
    const QString source = profilePath(name);
    QSettings before;
    const QString beforeName = before.value(QZSettings::profile_name).toString();
    homeform::loadSettings(QUrl::fromLocalFile(source));
    QSettings after;
    qInfo().noquote() << "LOAD requested=" << name
                      << "source=" << source
                      << "profile_name_before=" << beforeName
                      << "profile_name_after=" << after.value(QZSettings::profile_name).toString()
                      << "garmin_email=" << after.value(QZSettings::garmin_email).toString()
                      << "garmin_upload_enabled=" << after.value(QZSettings::garmin_upload_enabled).toBool();
}

void expectProfile(const ProfileValues &profile)
{
    QSettings settings;
    EXPECT_EQ(settings.value(QZSettings::profile_name).toString(), profile.name);
    EXPECT_EQ(settings.value(QZSettings::weight).toInt(), profile.weight);
    EXPECT_EQ(settings.value(QZSettings::ftp).toInt(), profile.ftp);
    EXPECT_EQ(settings.value(QZSettings::age).toInt(), profile.age);
    EXPECT_EQ(settings.value(QZSettings::user_email).toString(), profile.commonEmail);
    EXPECT_EQ(settings.value(QZSettings::garmin_email).toString(), profile.garminEmail);
    EXPECT_EQ(settings.value(QZSettings::garmin_upload_enabled).toBool(), profile.garminEnabled);

    const QString id = userIdFor(profile);
    if (profile.tokenId.isEmpty()) {
        EXPECT_TRUE(ProfileTokenStore::value(settings, QZSettings::garmin_access_token, id).toString().isEmpty());
    } else {
        EXPECT_EQ(ProfileTokenStore::value(settings, QZSettings::garmin_access_token, id).toString(),
                  QStringLiteral("access-") + profile.tokenId);
    }
}

void expectBothFiles()
{
    ASSERT_TRUE(QFileInfo::exists(profilePath(QStringLiteral("Jan"))));
    ASSERT_TRUE(QFileInfo::exists(profilePath(QStringLiteral("Jay"))));
}

void startTwoProfiles(bool sameGarminEmail, bool sameGeneralEmail)
{
    const ProfileValues jan{QStringLiteral("Jan"), 64, 200, 45,
                            sameGarminEmail ? QStringLiteral("same@example.invalid") : QStringLiteral("jan@example.invalid"),
                            true, QStringLiteral("JAN"),
                            sameGeneralEmail ? QStringLiteral("common@example.invalid") : QStringLiteral("jan-general@example.invalid")};
    const ProfileValues jay{QStringLiteral("Jay"), 75, 175, 53,
                            sameGarminEmail ? QStringLiteral("same@example.invalid") : QStringLiteral("jay@example.invalid"),
                            true, QStringLiteral("JAY"),
                            sameGeneralEmail ? QStringLiteral("common@example.invalid") : QStringLiteral("jay-general@example.invalid")};

    setProfileValues(jan); saveProfileReal(jan.name);
    setProfileValues(jay); saveProfileReal(jay.name);
    expectBothFiles();

    clearGlobalSettings();
    loadProfileReal(jan.name); expectProfile(jan);
    clearGlobalSettings();
    loadProfileReal(jay.name); expectProfile(jay);

    clearGlobalSettings();
    loadProfileReal(jay.name); expectProfile(jay);
    QSettings().setValue(QZSettings::weight, 76); QSettings().sync();
    saveProfileReal(jay.name);
    expectBothFiles();
    const ProfileValues modifiedJay{jay.name, 76, jay.ftp, jay.age, jay.garminEmail, jay.garminEnabled, jay.tokenId, jay.commonEmail};
    clearGlobalSettings(); loadProfileReal(jan.name); expectProfile(jan);

    clearGlobalSettings();
    loadProfileReal(jan.name); expectProfile(jan);
    QSettings().setValue(QZSettings::weight, 65); QSettings().sync();
    saveProfileReal(jan.name);
    expectBothFiles();
    const ProfileValues modifiedJan{jan.name, 65, jan.ftp, jan.age, jan.garminEmail, jan.garminEnabled, jan.tokenId, jan.commonEmail};
    clearGlobalSettings(); loadProfileReal(jay.name); expectProfile(modifiedJay);
    clearGlobalSettings(); loadProfileReal(jan.name); expectProfile(modifiedJan);
}

void verifyScopedAndLegacy(const QString &profileName, const QString &email, const QString &tokenId)
{
    QSettings settings;
    const QString id = QStringLiteral("garmin.com_") + email;
    EXPECT_EQ(settings.value(QZSettings::garmin_email).toString(), email);
    EXPECT_EQ(ProfileTokenStore::value(settings, QZSettings::garmin_access_token, id).toString(),
              QStringLiteral("access-") + tokenId);
    qInfo().noquote() << "GARMIN active profile=" << profileName
                      << "scoped_key=" << ProfileTokenStore::scopedKey(QZSettings::garmin_access_token, id)
                      << "legacy_key_present=" << settings.contains(QZSettings::garmin_access_token);
}
}

void ProfileRegressionTestSuite::SetUp()
{
    QStandardPaths::setTestModeEnabled(true);
    QCoreApplication::setOrganizationName(QStringLiteral("Roberto Viola"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("robertoviola.cloud"));
    QCoreApplication::setApplicationName(QStringLiteral("qDomyos-Zwift-profile-regression"));
    originalCwd = QDir::currentPath();
    const QByteArray childMode = qgetenv("QZ_PROFILE_TEST_CHILD");
    if (childMode == "1") {
        usingExternalRoot = true;
        ASSERT_FALSE(qEnvironmentVariable("QZ_PROFILE_TEST_ROOT").isEmpty());
        ASSERT_TRUE(QDir::setCurrent(qEnvironmentVariable("QZ_PROFILE_TEST_ROOT")));
    } else {
        root = new QTemporaryDir;
        ASSERT_TRUE(root->isValid());
        ASSERT_TRUE(QDir::setCurrent(root->path()));
        clearGlobalSettings();
    }
    QDir().mkpath(homeform::getProfileDir());
}

void ProfileRegressionTestSuite::TearDown()
{
    if (!usingExternalRoot)
        clearGlobalSettings();
    QDir::setCurrent(originalCwd);
    if (!usingExternalRoot) {
        delete root;
        root = nullptr;
    }
    usingExternalRoot = false;
}

TEST_F(ProfileRegressionTestSuite, BasicProfileIsolation)
{
    startTwoProfiles(false, false);
}

TEST_F(ProfileRegressionTestSuite, JasonFailureSequence)
{
    startTwoProfiles(false, false);
}

TEST_F(ProfileRegressionTestSuite, ChildProcessLoadsProfilesAfterRestart)
{
    if (qgetenv("QZ_PROFILE_TEST_CHILD") != "1")
        GTEST_SKIP() << "child-only test";

    expectBothFiles();
    clearGlobalSettings();
    loadProfileReal(QStringLiteral("Jan"));
    expectProfile({QStringLiteral("Jan"), 64, 200, 45, QStringLiteral("jan@example.invalid"), true,
                   QStringLiteral("JAN"), QStringLiteral("common@example.invalid")});
    clearGlobalSettings();
    loadProfileReal(QStringLiteral("Jay"));
    expectProfile({QStringLiteral("Jay"), 75, 175, 53, QStringLiteral("jay@example.invalid"), true,
                   QStringLiteral("JAY"), QStringLiteral("common@example.invalid")});
}

TEST_F(ProfileRegressionTestSuite, RealProcessRestartPreservesBothProfiles)
{
    if (qgetenv("QZ_PROFILE_TEST_CHILD") == "1")
        GTEST_SKIP() << "parent-only test";
    if (qgetenv("QZ_PROFILE_RUN_PROCESS_RESTART") != "1")
        GTEST_SKIP() << "set QZ_PROFILE_RUN_PROCESS_RESTART=1 for the cross-process check";

    const ProfileValues jan{QStringLiteral("Jan"), 64, 200, 45, QStringLiteral("jan@example.invalid"), true,
                            QStringLiteral("JAN"), QStringLiteral("common@example.invalid")};
    const ProfileValues jay{QStringLiteral("Jay"), 75, 175, 53, QStringLiteral("jay@example.invalid"), true,
                            QStringLiteral("JAY"), QStringLiteral("common@example.invalid")};
    setProfileValues(jan);
    saveProfileReal(jan.name);
    setProfileValues(jay);
    saveProfileReal(jay.name);
    expectBothFiles();

    QProcess child;
    QProcessEnvironment environment = QProcessEnvironment::systemEnvironment();
    environment.insert(QStringLiteral("QZ_PROFILE_TEST_CHILD"), QStringLiteral("1"));
    environment.insert(QStringLiteral("QZ_PROFILE_TEST_ROOT"), root->path());
    child.setProcessEnvironment(environment);
    child.setWorkingDirectory(root->path());
    child.setProgram(QCoreApplication::applicationFilePath());
    child.setArguments({QStringLiteral("--gtest_filter=ProfileRegressionTestSuite.ChildProcessLoadsProfilesAfterRestart")});
    child.start();
    ASSERT_TRUE(child.waitForFinished(120000));
    qInfo().noquote() << "RESTART child_exit=" << child.exitCode()
                      << "status=" << child.exitStatus()
                      << "stdout_bytes=" << child.readAllStandardOutput().size()
                      << "stderr_bytes=" << child.readAllStandardError().size();
    ASSERT_EQ(child.exitStatus(), QProcess::NormalExit);
    ASSERT_EQ(child.exitCode(), 0);
    expectBothFiles();
}

TEST_F(ProfileRegressionTestSuite, SameGeneralOptionsEmailDoesNotIdentifyProfile)
{
    startTwoProfiles(false, true);
}

TEST_F(ProfileRegressionTestSuite, DifferentGarminAccountsRemainScoped)
{
    startTwoProfiles(false, false);
    clearGlobalSettings(); loadProfileReal(QStringLiteral("Jan")); verifyScopedAndLegacy(QStringLiteral("Jan"), QStringLiteral("jan@example.invalid"), QStringLiteral("JAN"));
    clearGlobalSettings(); loadProfileReal(QStringLiteral("Jay")); verifyScopedAndLegacy(QStringLiteral("Jay"), QStringLiteral("jay@example.invalid"), QStringLiteral("JAY"));
}

TEST_F(ProfileRegressionTestSuite, GarminProfileFilesContainOnlyActiveAccountTokens)
{
    const ProfileValues jan{QStringLiteral("Jan"), 64, 200, 45, QStringLiteral("jan@example.invalid"), true,
                            QStringLiteral("JAN"), QStringLiteral("common@example.invalid")};
    const ProfileValues jay{QStringLiteral("Jay"), 75, 175, 53, QStringLiteral("jay@example.invalid"), true,
                            QStringLiteral("JAY"), QStringLiteral("common@example.invalid")};

    setProfileValues(jan);
    saveProfileReal(jan.name);
    setProfileValues(jay);
    saveProfileReal(jay.name);

    const QSettings janFile(profilePath(jan.name), QSettings::IniFormat);
    const QSettings jayFile(profilePath(jay.name), QSettings::IniFormat);
    EXPECT_TRUE(janFile.contains(ProfileTokenStore::scopedKey(QZSettings::garmin_access_token, janGarminId)));
    EXPECT_TRUE(jayFile.contains(ProfileTokenStore::scopedKey(QZSettings::garmin_access_token, jayGarminId)));
    EXPECT_FALSE(janFile.contains(ProfileTokenStore::scopedKey(QZSettings::garmin_access_token, jayGarminId)));
    EXPECT_FALSE(jayFile.contains(ProfileTokenStore::scopedKey(QZSettings::garmin_access_token, janGarminId)));
}

TEST_F(ProfileRegressionTestSuite, GarminTokensSurviveRealProfileSwitchWithoutGlobalReset)
{
    const ProfileValues jan{QStringLiteral("Jan"), 64, 200, 45, QStringLiteral("jan@example.invalid"), true,
                            QStringLiteral("JAN"), QStringLiteral("common@example.invalid")};
    const ProfileValues jay{QStringLiteral("Jay"), 75, 175, 53, QStringLiteral("jay@example.invalid"), true,
                            QStringLiteral("JAY"), QStringLiteral("common@example.invalid")};

    setProfileValues(jan);
    saveProfileReal(jan.name);
    setProfileValues(jay);
    saveProfileReal(jay.name);

    clearGlobalSettings();
    loadProfileReal(jan.name);
    {
        GarminConnect garmin;
        ASSERT_TRUE(garmin.isAuthenticated());
        EXPECT_EQ(ProfileTokenStore::value(QSettings(), QZSettings::garmin_access_token, janGarminId).toString(),
                  QStringLiteral("access-JAN"));
    }

    // This is the UI path: switch profiles without clearing QSettings globally,
    // then recreate GarminConnect as loadSettings() does in the application.
    loadProfileReal(jay.name);
    {
        GarminConnect garmin;
        ASSERT_TRUE(garmin.isAuthenticated());
        EXPECT_EQ(ProfileTokenStore::value(QSettings(), QZSettings::garmin_access_token, jayGarminId).toString(),
                  QStringLiteral("access-JAY"));
    }
    QSettings().setValue(QZSettings::weight, 76);
    QSettings().sync();
    saveProfileReal(jay.name);

    loadProfileReal(jan.name);
    {
        GarminConnect garmin;
        ASSERT_TRUE(garmin.isAuthenticated());
        EXPECT_EQ(ProfileTokenStore::value(QSettings(), QZSettings::garmin_access_token, janGarminId).toString(),
                  QStringLiteral("access-JAN"));
    }

    loadProfileReal(jay.name);
    {
        GarminConnect garmin;
        ASSERT_TRUE(garmin.isAuthenticated());
        EXPECT_EQ(ProfileTokenStore::value(QSettings(), QZSettings::garmin_access_token, jayGarminId).toString(),
                  QStringLiteral("access-JAY"));
    }
}

TEST_F(ProfileRegressionTestSuite, RealGarminConnectLoadsScopedTokensAndMirrorsLegacyKeysOffline)
{
    const ProfileValues jan{QStringLiteral("Jan"), 64, 200, 45, QStringLiteral("jan@example.invalid"), true,
                            QStringLiteral("JAN"), QStringLiteral("common@example.invalid")};
    const ProfileValues jay{QStringLiteral("Jay"), 75, 175, 53, QStringLiteral("jay@example.invalid"), true,
                            QStringLiteral("JAY"), QStringLiteral("common@example.invalid")};
    setProfileValues(jan);
    saveProfileReal(jan.name);
    setProfileValues(jay);
    saveProfileReal(jay.name);

    clearGlobalSettings();
    loadProfileReal(jan.name);
    {
        GarminConnect garmin;
        EXPECT_TRUE(garmin.isAuthenticated());
        QSettings settings;
        EXPECT_EQ(settings.value(QZSettings::garmin_access_token).toString(), QStringLiteral("access-JAN"));
        EXPECT_EQ(settings.value(QZSettings::garmin_refresh_token).toString(), QStringLiteral("refresh-JAN"));
        EXPECT_EQ(ProfileTokenStore::value(settings, QZSettings::garmin_access_token, janGarminId).toString(),
                  QStringLiteral("access-JAN"));
        EXPECT_TRUE(ProfileTokenStore::value(settings, QZSettings::garmin_access_token, jayGarminId).toString().isEmpty());
    }

    clearGlobalSettings();
    loadProfileReal(jay.name);
    {
        GarminConnect garmin;
        EXPECT_TRUE(garmin.isAuthenticated());
        QSettings settings;
        EXPECT_EQ(settings.value(QZSettings::garmin_access_token).toString(), QStringLiteral("access-JAY"));
        EXPECT_EQ(settings.value(QZSettings::garmin_refresh_token).toString(), QStringLiteral("refresh-JAY"));
        EXPECT_EQ(ProfileTokenStore::value(settings, QZSettings::garmin_access_token, jayGarminId).toString(),
                  QStringLiteral("access-JAY"));
        EXPECT_TRUE(ProfileTokenStore::value(settings, QZSettings::garmin_access_token, janGarminId).toString().isEmpty());
    }
}

TEST_F(ProfileRegressionTestSuite, DisabledProfileLeavesPreexistingGlobalGarminStateObservable)
{
    const ProfileValues jan{QStringLiteral("Jan"), 64, 200, 45, QString(), false, QString(), QStringLiteral("common@example.invalid")};
    const ProfileValues jay{QStringLiteral("Jay"), 75, 175, 53, QStringLiteral("jay@example.invalid"), true,
                            QStringLiteral("JAY"), QStringLiteral("common@example.invalid")};
    setProfileValues(jay);
    saveProfileReal(jay.name);
    setProfileFieldsWithoutGarminTokenChanges(jan);
    saveProfileReal(jan.name);
    expectBothFiles();

    loadProfileReal(jan.name);
    QSettings settings;
    EXPECT_FALSE(settings.value(QZSettings::garmin_upload_enabled).toBool());
    EXPECT_EQ(settings.value(QZSettings::garmin_access_token).toString(), QStringLiteral("access-JAY"));
    EXPECT_EQ(ProfileTokenStore::value(settings, QZSettings::garmin_access_token, jayGarminId).toString(),
              QStringLiteral("access-JAY"));

    GarminConnect garmin;
    EXPECT_TRUE(garmin.isAuthenticated());
}

TEST_F(ProfileRegressionTestSuite, SameGarminAccountHasSharedScopedIdentityButSeparateProfiles)
{
    startTwoProfiles(true, false);
    EXPECT_TRUE(QFileInfo::exists(profilePath(QStringLiteral("Jan"))));
    EXPECT_TRUE(QFileInfo::exists(profilePath(QStringLiteral("Jay"))));
    clearGlobalSettings(); loadProfileReal(QStringLiteral("Jan")); expectProfile({QStringLiteral("Jan"), 65, 200, 45, QStringLiteral("same@example.invalid"), true, QStringLiteral("JAN"), QStringLiteral("jan-general@example.invalid")});
    clearGlobalSettings(); loadProfileReal(QStringLiteral("Jay")); expectProfile({QStringLiteral("Jay"), 76, 175, 53, QStringLiteral("same@example.invalid"), true, QStringLiteral("JAY"), QStringLiteral("jay-general@example.invalid")});
}

TEST_F(ProfileRegressionTestSuite, SameGeneralAndGarminEmailStillKeepsProfileFilesSeparate)
{
    startTwoProfiles(true, true);
    expectBothFiles();
    clearGlobalSettings();
    loadProfileReal(QStringLiteral("Jan"));
    expectProfile({QStringLiteral("Jan"), 65, 200, 45, QStringLiteral("same@example.invalid"), true,
                   QStringLiteral("JAN"), QStringLiteral("common@example.invalid")});
    clearGlobalSettings();
    loadProfileReal(QStringLiteral("Jay"));
    expectProfile({QStringLiteral("Jay"), 76, 175, 53, QStringLiteral("same@example.invalid"), true,
                   QStringLiteral("JAY"), QStringLiteral("common@example.invalid")});
    expectBothFiles();
}

TEST_F(ProfileRegressionTestSuite, ReverseCreationOrderKeepsBothProfiles)
{
    const ProfileValues jan{QStringLiteral("Jan"), 64, 200, 45, QStringLiteral("jan@example.invalid"), true,
                            QStringLiteral("JAN"), QStringLiteral("common@example.invalid")};
    const ProfileValues jay{QStringLiteral("Jay"), 75, 175, 53, QStringLiteral("jay@example.invalid"), true,
                            QStringLiteral("JAY"), QStringLiteral("common@example.invalid")};

    setProfileValues(jay);
    saveProfileReal(jay.name);
    setProfileValues(jan);
    saveProfileReal(jan.name);
    expectBothFiles();

    clearGlobalSettings();
    loadProfileReal(jay.name);
    expectProfile(jay);
    clearGlobalSettings();
    loadProfileReal(jan.name);
    expectProfile(jan);

    clearGlobalSettings();
    loadProfileReal(jan.name);
    QSettings().setValue(QZSettings::weight, 65);
    QSettings().sync();
    saveProfileReal(jan.name);
    expectBothFiles();
    clearGlobalSettings();
    loadProfileReal(jay.name);
    expectProfile(jay);

    clearGlobalSettings();
    loadProfileReal(jay.name);
    QSettings().setValue(QZSettings::weight, 76);
    QSettings().sync();
    saveProfileReal(jay.name);
    expectBothFiles();
    clearGlobalSettings();
    loadProfileReal(jan.name);
    expectProfile({jan.name, 65, jan.ftp, jan.age, jan.garminEmail, jan.garminEnabled, jan.tokenId, jan.commonEmail});
}

TEST_F(ProfileRegressionTestSuite, DisabledGarminKeepsGlobalScopedStateWithoutDeletingProfileFiles)
{
    const ProfileValues enabledJan{QStringLiteral("Jan"), 64, 200, 45, QStringLiteral("jan@example.invalid"), true, QStringLiteral("JAN"), QStringLiteral("jan-general@example.invalid")};
    const ProfileValues enabledJay{QStringLiteral("Jay"), 75, 175, 53, QStringLiteral("jay@example.invalid"), true, QStringLiteral("JAY"), QStringLiteral("jay-general@example.invalid")};
    const ProfileValues disabledJan{QStringLiteral("Jan"), 64, 200, 45, QString(), false, QString(), QStringLiteral("jan-general@example.invalid")};
    setProfileValues(enabledJan); saveProfileReal(enabledJan.name);
    setProfileValues(enabledJay); saveProfileReal(enabledJay.name);
    setProfileValues(disabledJan);
    ASSERT_EQ(ProfileTokenStore::value(QSettings(), QZSettings::garmin_access_token, janGarminId).toString(),
              QStringLiteral("access-JAN"));
    saveProfileReal(disabledJan.name);
    expectBothFiles();
    const QStringList keys = QSettings(profilePath(QStringLiteral("Jan")), QSettings::IniFormat).allKeys();
    EXPECT_TRUE(keys.contains(ProfileTokenStore::scopedKey(QZSettings::garmin_access_token, janGarminId)));
    loadProfileReal(disabledJan.name);
    expectProfile(disabledJan);
    EXPECT_EQ(ProfileTokenStore::value(QSettings(), QZSettings::garmin_access_token, janGarminId).toString(),
              QStringLiteral("access-JAN"));
}

TEST_F(ProfileRegressionTestSuite, StaleKeysInExistingProfileFileAreRemovedWhenNoLongerActive)
{
    const ProfileValues enabledJan{QStringLiteral("Jan"), 64, 200, 45, QStringLiteral("jan@example.invalid"), true, QStringLiteral("JAN"), QStringLiteral("jan-general@example.invalid")};
    const ProfileValues disabledJan{QStringLiteral("Jan"), 64, 200, 45, QString(), false, QString(), QStringLiteral("jan-general@example.invalid")};
    setProfileValues(enabledJan);
    saveProfileReal(enabledJan.name);

    setProfileValues(disabledJan);
    QSettings settings;
    for (const QString &key : settings.allKeys()) {
        if (key == QZSettings::garmin_access_token ||
            key == QZSettings::garmin_refresh_token ||
            key == QZSettings::garmin_token_type ||
            key == QZSettings::garmin_expires_at ||
            key == QZSettings::garmin_refresh_token_expires_at ||
            key == QZSettings::garmin_oauth1_token ||
            key == QZSettings::garmin_oauth1_token_secret ||
            key == QZSettings::garmin_last_refresh ||
            key.startsWith(QZSettings::garmin_access_token + QStringLiteral("_")) ||
            key.startsWith(QZSettings::garmin_refresh_token + QStringLiteral("_")) ||
            key.startsWith(QZSettings::garmin_token_type + QStringLiteral("_"))) {
            settings.remove(key);
        }
    }
    settings.sync();
    saveProfileReal(disabledJan.name);

    const QStringList keys = QSettings(profilePath(QStringLiteral("Jan")), QSettings::IniFormat).allKeys();
    for (const QString &key : keys) {
        EXPECT_FALSE(key == QZSettings::garmin_access_token ||
                     key == QZSettings::garmin_refresh_token ||
                     key == QZSettings::garmin_token_type ||
                     key.startsWith(QZSettings::garmin_access_token + QStringLiteral("_")) ||
                     key.startsWith(QZSettings::garmin_refresh_token + QStringLiteral("_")) ||
                     key.startsWith(QZSettings::garmin_token_type + QStringLiteral("_")));
    }
    EXPECT_TRUE(QFileInfo::exists(profilePath(QStringLiteral("Jan"))));
}
TEST_F(ProfileRegressionTestSuite, DisabledGarminProfileDoesNotRemoveEnabledProfile)
{
    const ProfileValues jan{QStringLiteral("Jan"), 64, 200, 45, QString(), false, QString(), QStringLiteral("jan-general@example.invalid")};
    const ProfileValues jay{QStringLiteral("Jay"), 75, 175, 53, QStringLiteral("jay@example.invalid"), true, QStringLiteral("JAY"), QStringLiteral("jay-general@example.invalid")};
    setProfileValues(jan); saveProfileReal(jan.name);
    setProfileValues(jay); saveProfileReal(jay.name);
    expectBothFiles();
    clearGlobalSettings(); loadProfileReal(jan.name); expectProfile(jan); expectBothFiles();
    clearGlobalSettings(); loadProfileReal(jay.name); expectProfile(jay); expectBothFiles();
    clearGlobalSettings(); loadProfileReal(jan.name); expectProfile(jan); expectBothFiles();
}

void setStravaProfileValues(const QString &profileName, const QString &athleteId, const QString &tokenId)
{
    QSettings settings;
    settings.setValue(QZSettings::profile_name, profileName);
    settings.setValue(QZSettings::strava_current_user_id, athleteId);
    ProfileTokenStore::save(settings, QZSettings::strava_accesstoken,
                            QStringLiteral("strava-access-") + tokenId, athleteId, true);
    ProfileTokenStore::save(settings, QZSettings::strava_refreshtoken,
                            QStringLiteral("strava-refresh-") + tokenId, athleteId, true);
    ProfileTokenStore::save(settings, QZSettings::strava_lastrefresh,
                            QStringLiteral("refresh-time-") + tokenId, athleteId, true);
    ProfileTokenStore::save(settings, QZSettings::strava_expires,
                            QStringLiteral("expires-") + tokenId, athleteId, true);
    settings.sync();
}

TEST_F(ProfileRegressionTestSuite, StravaProfileFilesContainOnlyActiveAccountTokens)
{
    setStravaProfileValues(QStringLiteral("Jan"), QStringLiteral("101"), QStringLiteral("JAN"));
    saveProfileReal(QStringLiteral("Jan"));
    setStravaProfileValues(QStringLiteral("Jay"), QStringLiteral("202"), QStringLiteral("JAY"));
    saveProfileReal(QStringLiteral("Jay"));

    const QSettings janFile(profilePath(QStringLiteral("Jan")), QSettings::IniFormat);
    const QSettings jayFile(profilePath(QStringLiteral("Jay")), QSettings::IniFormat);
    EXPECT_TRUE(janFile.contains(ProfileTokenStore::scopedKey(QZSettings::strava_accesstoken, QStringLiteral("101"))));
    EXPECT_TRUE(jayFile.contains(ProfileTokenStore::scopedKey(QZSettings::strava_accesstoken, QStringLiteral("202"))));
    EXPECT_FALSE(janFile.contains(ProfileTokenStore::scopedKey(QZSettings::strava_accesstoken, QStringLiteral("202"))));
    EXPECT_FALSE(jayFile.contains(ProfileTokenStore::scopedKey(QZSettings::strava_accesstoken, QStringLiteral("101"))));
    EXPECT_FALSE(janFile.contains(ProfileTokenStore::scopedKey(QZSettings::strava_refreshtoken, QStringLiteral("202"))));
    EXPECT_FALSE(jayFile.contains(ProfileTokenStore::scopedKey(QZSettings::strava_refreshtoken, QStringLiteral("101"))));
}

TEST_F(ProfileRegressionTestSuite, StravaScopedRefreshTokenSurvivesProfileLoad)
{
    setStravaProfileValues(QStringLiteral("Jan"), QStringLiteral("101"), QStringLiteral("JAN"));
    saveProfileReal(QStringLiteral("Jan"));

    clearGlobalSettings();
    loadProfileReal(QStringLiteral("Jan"));
    QSettings settings;
    EXPECT_EQ(settings.value(QZSettings::strava_current_user_id).toString(), QStringLiteral("101"));
    EXPECT_EQ(ProfileTokenStore::value(settings, QZSettings::strava_accesstoken, QStringLiteral("101")).toString(),
              QStringLiteral("strava-access-JAN"));
    EXPECT_EQ(ProfileTokenStore::value(settings, QZSettings::strava_refreshtoken, QStringLiteral("101")).toString(),
              QStringLiteral("strava-refresh-JAN"));
}

TEST_F(ProfileRegressionTestSuite, LegacyStravaProfileStillLoadsUnscopedAccessToken)
{
    QSettings settings;
    settings.setValue(QZSettings::profile_name, QStringLiteral("Jan"));
    settings.setValue(QZSettings::strava_accesstoken, QStringLiteral("legacy-access"));
    settings.setValue(QZSettings::strava_refreshtoken, QStringLiteral("legacy-refresh"));
    settings.sync();
    saveProfileReal(QStringLiteral("Jan"));

    clearGlobalSettings();
    loadProfileReal(QStringLiteral("Jan"));
    EXPECT_TRUE(QSettings().value(QZSettings::strava_current_user_id).toString().isEmpty());
    EXPECT_EQ(QSettings().value(QZSettings::strava_accesstoken).toString(), QStringLiteral("legacy-access"));
}
