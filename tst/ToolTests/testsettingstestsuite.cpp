#include "testsettingstestsuite.h"

#include <QCoreApplication>
#include <QFile>
#include <QProcess>
#include <QProcessEnvironment>
#include <QResource>
#include "Tools/testsettings.h"


TestSettingsTestSuite::TestSettingsTestSuite()
{

}

void TestSettingsTestSuite::test_testSettings(){
    QString originalOrgName = "Original Org Name";
    QString originalAppName = "Original App Name";
    QString testSettingName = "TestSetting";

    // Set the original organisation name
    QCoreApplication::setOrganizationName(originalOrgName);
    QCoreApplication::setApplicationName(originalAppName);

    // Create a test settings object with a different org/app configuration
    TestSettings testSettings("Test Org Name", "Test App Name");

    // Create the default settings object, which should be different from the test settings object
    QSettings default0;
    EXPECT_EQ(QCoreApplication::organizationName(), default0.organizationName());
    EXPECT_EQ(QCoreApplication::applicationName(), default0.applicationName());

    EXPECT_NE(QCoreApplication::organizationName(), testSettings.qsettings.organizationName());
    EXPECT_NE(QCoreApplication::applicationName(), testSettings.qsettings.applicationName());

    // Changes to the default settings should not affect the test settings
    default0.setValue(testSettingName, 100);
    testSettings.qsettings.setValue(testSettingName, 200);

    int defaultValue = default0.value(testSettingName, 0).toInt();
    int testValue = testSettings.qsettings.value(testSettingName, 0).toInt();
    EXPECT_NE(testValue, defaultValue);

    testSettings.activate();
    QSettings default1;

    // Now that the test settings have been activated, the QCoreApplication and the new default settings object should match the test settings.
    EXPECT_EQ(QCoreApplication::organizationName(), default1.organizationName());
    EXPECT_EQ(QCoreApplication::applicationName(), default1.applicationName());
    EXPECT_EQ(QCoreApplication::organizationName(), testSettings.qsettings.organizationName());
    EXPECT_EQ(QCoreApplication::applicationName(), testSettings.qsettings.applicationName());

    // Changes to the default settings should not affect the test settings
    default1.setValue(testSettingName, 500);
    testSettings.qsettings.setValue(testSettingName, 100);

    defaultValue = default1.value(testSettingName, 0).toInt();
    testValue = testSettings.qsettings.value(testSettingName, 0).toInt();
    EXPECT_EQ(testValue, defaultValue);

    testSettings.deactivate();

    EXPECT_EQ(QCoreApplication::applicationName(), originalAppName);
    EXPECT_EQ(QCoreApplication::organizationName(), originalOrgName);

}

void TestSettingsTestSuite::test_destructor(){
    QString originalOrgName = "Original Org Name";
    QString originalAppName = "Original App Name";

    // Set the original organisation name
    QCoreApplication::setOrganizationName(originalOrgName);
    QCoreApplication::setApplicationName(originalAppName);

    // Create a test settings object with a different org/app configuration
    // Inside a code block so that
    {
        TestSettings testSettings("Test Org Name", "Test App Name");
        testSettings.activate();

        EXPECT_EQ(QCoreApplication::organizationName(), testSettings.qsettings.organizationName());
        EXPECT_EQ(QCoreApplication::applicationName(), testSettings.qsettings.applicationName());

        // testSettings should be destroyed here, which should restore the original QCoreApplication details
    }

    EXPECT_EQ(QCoreApplication::applicationName(), originalAppName);
    EXPECT_EQ(QCoreApplication::organizationName(), originalOrgName);
}

void TestSettingsTestSuite::test_longTranslatedSwitchLabelsWrap(){
    // Run the real QML control in an isolated GUI subprocess. The main test process
    // intentionally remains QCoreApplication-based, so this does not change the
    // environment or behavior of the rest of the test suite.
    QProcess probe;
    QProcessEnvironment environment = QProcessEnvironment::systemEnvironment();
    environment.insert("QT_QPA_PLATFORM", "offscreen");
    probe.setProcessEnvironment(environment);
    probe.start(QCoreApplication::applicationFilePath(),
                QStringList() << "--settings-label-layout-probe");

    ASSERT_TRUE(probe.waitForStarted(10000));
    ASSERT_TRUE(probe.waitForFinished(30000));

    const QByteArray probeOutput = probe.readAllStandardError() + probe.readAllStandardOutput();
    EXPECT_EQ(probe.exitStatus(), QProcess::NormalExit) << probeOutput.constData();
    EXPECT_EQ(probe.exitCode(), 0) << probeOutput.constData();

    Q_INIT_RESOURCE(qml);

    QFile indicatorOnlySwitch(":/IndicatorOnlySwitch.qml");
    ASSERT_TRUE(indicatorOnlySwitch.open(QIODevice::ReadOnly | QIODevice::Text));
    const QString indicatorSource = QString::fromUtf8(indicatorOnlySwitch.readAll());

    // Do not replace Material's IconLabel: it owns colors, disabled state,
    // padding, icon handling and RTL layout. Only configure its text child.
    EXPECT_FALSE(indicatorSource.contains("contentItem:"));
    EXPECT_TRUE(indicatorSource.contains("configureTextItem"));
    EXPECT_TRUE(indicatorSource.contains("Text.WrapAtWordBoundaryOrAnywhere"));
    EXPECT_TRUE(indicatorSource.contains("maximumLineCount = 2"));
    EXPECT_TRUE(indicatorSource.contains("elide = Text.ElideRight"));

    QFile settingsQml(":/settings.qml");
    ASSERT_TRUE(settingsQml.open(QIODevice::ReadOnly | QIODevice::Text));
    const QString settingsSource = QString::fromUtf8(settingsQml.readAll());

    // Regression case: the German translation of this setting previously appeared
    // as "Nutze Apple Watch Cadence für gefälschte Laufbandges..." on narrow screens.
    const QString regressionTitle =
        "text: qsTr(\"Use Apple Watch Cadence for Fake Treadmill Speed\")";
    const int titlePosition = settingsSource.indexOf(regressionTitle);
    ASSERT_NE(titlePosition, -1);

    const int delegatePosition = settingsSource.lastIndexOf("IndicatorOnlySwitch {", titlePosition);
    ASSERT_NE(delegatePosition, -1);
    EXPECT_LT(titlePosition - delegatePosition, 600);

    // No translated control may enlarge the vertical settings page beyond the
    // visible viewport. This protects labels, buttons, combo boxes and future
    // controls with a large implicit width.
    EXPECT_TRUE(settingsSource.contains("contentWidth: availableWidth"));

    // Concrete regression case from the German UI: the two log buttons used to
    // make the entire settings page wider than a phone screen. Keep them in two
    // columns when they fit and stack them when the translated labels do not.
    EXPECT_TRUE(settingsSource.contains("id: logsButtonsLayout"));
    EXPECT_TRUE(settingsSource.contains(
        "columns: width >= clearLogs.implicitWidth + showLogs.implicitWidth + columnSpacing ? 2 : 1"));
    EXPECT_TRUE(settingsSource.contains("id: showLogs"));
}


