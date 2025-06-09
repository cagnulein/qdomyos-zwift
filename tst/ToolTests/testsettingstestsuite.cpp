#include "testsettingstestsuite.h"

#include <QCoreApplication>
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

    EXPECT_EQ(QCoreApplication::organizationName(), originalOrgName);
    EXPECT_EQ(QCoreApplication::applicationName(), originalAppName);
}


