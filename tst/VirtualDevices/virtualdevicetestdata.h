#pragma once

#include "bluetoothdevice.h"
#include "virtualdevice.h"

class VirtualDeviceTestData
{
    std::string testName = nullptr;
public:
    VirtualDeviceTestData(std::string testName) : testName(testName) {}
    ~VirtualDeviceTestData() = default;

    /**
     * @brief Gets the test name.
     * @return
     */
    virtual std::string get_testName() const;
    /**
     * @brief Gets the expected lockscreen configuration type enumeration value to be used by this device..
     */
    virtual QZLockscreenFunctions::configurationType get_expectedLockscreenConfigurationType() const =0;

    virtual virtualdevice * createDevice(bluetoothdevice * device) = 0;
};

