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

    DeviceDiscoveryInfo defaultDiscoveryInfo(true);

    // Don't let the virtual device try to start dircon
    defaultDiscoveryInfo.dircon_yes = false;

    auto configs = testData.get_lockscreenConfigurations(defaultDiscoveryInfo);

    EXPECT_GT(configs.size(), 0) << "No lockscreen configurations configured for test";

    for (auto config : configs) {
        this->testLockscreen->reset();
        this->testSettings.loadFrom(config.get_settings());

        std::shared_ptr<virtualdevice> virtualDevice = std::shared_ptr<virtualdevice>(testData.createDevice(nullptr));
        EXPECT_NE(nullptr, virtualDevice.get()) << "No virtual device created";
        if(!virtualDevice.get()) continue;
        const auto lockscreenFunctions = virtualDevice.get()->getLockscreenFunctions();

        EXPECT_NE(nullptr, lockscreenFunctions)
                << testData.get_testName()
                << " : No lockscreen functions object";
        if(!lockscreenFunctions) continue;

        const auto lockscreen = lockscreenFunctions->getLockscreen();
        EXPECT_NE(nullptr, lockscreen)
                << testData.get_testName()
                << " : No lockscreen object";
        EXPECT_EQ(this->testLockscreen, lockscreen)
                << testData.get_testName()
                << " : Lockscreen object should have been the test lockscreen";

        EXPECT_EQ(config.get_isPelotonActive(), lockscreenFunctions->isPelotonWorkaroundActive())
                << testData.get_testName()
                << " : Peloton workaround in lockscreen object unexpectedly "
                << (config.get_isPelotonActive() ? "not":"")
                << " active";

        auto expectedVirtualDeviceConfig = config.get_lockscreenConfigType();
        EXPECT_EQ(expectedVirtualDeviceConfig, this->testLockscreen->get_virtualDeviceType())
                << testData.get_testName()
                << " : Unexpected Peloton workaround configuration type in lockscreen object. Expected "
                << LockscreenFunctionsTestData::getConfigurationTypeName(expectedVirtualDeviceConfig)
                << " got "
                << LockscreenFunctionsTestData::getConfigurationTypeName(this->testLockscreen->get_virtualDeviceType());


        EXPECT_EQ(config.get_lockscreenZwiftMode(), this->testLockscreen->get_zwiftMode())
                << testData.get_testName()
                << " : Peloton workaround in lockscreen object unexpectedly "
                << (config.get_lockscreenZwiftMode() ? "not":"")
                << " in Zwift mode";
    }
}
