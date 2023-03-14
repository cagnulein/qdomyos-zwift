#pragma once

#include "bluetoothdevice.h"
#include "virtualdevice.h"
#include "Devices/devicediscoveryinfo.h"
#include "Devices/lockscreenfunctionstestdata.h"

class VirtualDeviceTestData
{
    std::string testName = nullptr;
    bool expectedZwiftMode = true;
protected:
    /**
     * @brief Configure multiple devicediscoveryinfo objects to either enable or disable the Pelton workaround in multiple ways.
     * @param info
     * @param configurations The variations of the provided object to test.
     */
    virtual void configureLockscreenSettings(const DeviceDiscoveryInfo& info, std::vector<LockscreenFunctionsTestData>& configurations) const;
public:
    VirtualDeviceTestData(std::string testName, bool expectedZwiftMode) : testName(testName), expectedZwiftMode(expectedZwiftMode) {}
    ~VirtualDeviceTestData() = default;

    /**
     * @brief Gets the test name.
     * @return
     */
    virtual std::string get_testName() const;

    /**
     * @brief Creates the virtual device of the type this test data class represents.
     * @param device A bluetoothdevice instance for the virtual device.
     * @return
     */
    virtual virtualdevice * createDevice(bluetoothdevice * device) = 0;


    /**
     * @brief Gets combinations of configurations for enabling/disabilng the
     * the Peloton workaround beginning with the specified object.
     * @param info
     */
    virtual std::vector<LockscreenFunctionsTestData> get_lockscreenConfigurations(const DeviceDiscoveryInfo& info);

};

