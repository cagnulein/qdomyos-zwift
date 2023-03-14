#include "virtualdevicetestsuite.h"
#include "objectfactory.h"
#include "Devices/lockscreenfunctionstestdata.h"


template<typename T>
VirtualDeviceTestSuite<T>::VirtualDeviceTestSuite() : testSettings("Roberto Viola", "QDomyos-Zwift Testing")  {}

template<typename T>
void VirtualDeviceTestSuite<T>::SetUp() {

    this->testSettings.activate();

    // Override the lockscreen
    this->testLockscreen = new TestLockscreen();
    auto thisObject = this;
    ObjectFactory::lockscreenFactory = [thisObject]()->QZLockscreen*{ return thisObject->testLockscreen; };
}

template<typename T>
void VirtualDeviceTestSuite<T>::TearDown() {
    ObjectFactory::lockscreenFactory = nullptr;
}

template<typename T>
void VirtualDeviceTestSuite<T>::test_lockscreenConfiguration() {
    VirtualDeviceTestData& testData = this->typeParam;

    this->testLockscreen->reset();

    // Don't let the virtual device try to start dircon
    this->testSettings.qsettings.setValue(QZSettings::dircon_yes, false);

    std::shared_ptr<virtualdevice> virtualDevice = std::shared_ptr<virtualdevice>(testData.createDevice(nullptr));
    EXPECT_NE(nullptr, virtualDevice.get()) << "No virtual device created";
    if(!virtualDevice.get()) return;
    const auto lockscreenFunctions = virtualDevice.get()->getLockscreenFunctions();

    EXPECT_NE(nullptr, lockscreenFunctions)
            << testData.get_testName()
            << " : No lockscreen functions object";
    if(!lockscreenFunctions) return;

    const auto lockscreen = lockscreenFunctions->getLockscreen();
    EXPECT_NE(nullptr, lockscreen)
            << testData.get_testName()
            << " : No lockscreen object";
    EXPECT_EQ(this->testLockscreen, lockscreen)
            << testData.get_testName()
            << " : Lockscreen object should have been the test lockscreen";

    auto expectedVirtualDeviceConfig =     testData.get_expectedLockscreenConfigurationType();
    EXPECT_EQ(expectedVirtualDeviceConfig, this->testLockscreen->get_virtualDeviceType())
            << testData.get_testName()
                       << " : Unexpected Peloton workaround configuration type in lockscreen object. Expected "
                       << LockscreenFunctionsTestData::getConfigurationTypeName(expectedVirtualDeviceConfig)
                       << " got "
                       << LockscreenFunctionsTestData::getConfigurationTypeName(this->testLockscreen->get_virtualDeviceType());


    EXPECT_TRUE(this->testLockscreen->get_zwiftMode())
            << testData.get_testName()
            << " : Peloton workaround in lockscreen object unexpectedly not in Zwift mode";

}
