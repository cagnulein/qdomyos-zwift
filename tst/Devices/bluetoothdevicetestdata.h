#pragma once

#include <vector>
#include <unordered_set>

#include <QString>
#include <QBluetoothDeviceInfo>

#include "bluetoothdevice.h"
#include "devicenamepatterngroup.h"
#include "devicediscoveryinfo.h"
#include "Tools/devicetypeid.h"

typedef std::function<void(const DeviceDiscoveryInfo &info, bool enable, std::vector<DeviceDiscoveryInfo> &configurations)> ConfigurationApplicatorMultiple;
typedef std::function<void(DeviceDiscoveryInfo &info, bool enable)> ConfigurationApplicatorSingle;
typedef std::function<void()> DeviceTestInitializer;

class BluetoothDeviceTestData
{
protected:
    bool enabled = true;
    bool skipped = false;
    QString disabledReason = nullptr;
    QString skippedReason = nullptr;
    QString name = nullptr;

    std::unordered_set<DeviceTypeId> exclusions;
    DeviceNamePatternGroup * deviceNamePatternGroup=nullptr;
    ConfigurationApplicatorMultiple applicatorMultiple=nullptr;
    ConfigurationApplicatorSingle applicatorSingle=nullptr;
    DeviceTestInitializer initializer=nullptr;
    bool usingNonBluetoothDiscovery = false;
    std::function<bool(bluetoothdevice*)> isExpectedDevice=nullptr;
    DeviceTypeId expectedDeviceType=-1;
    BluetoothDeviceTestData();
public:
    /**
     * @brief Gets the name of the device, or device variant.
     * @return
     */
    QString Name() const;

    /**
     * @brief Indicates if non-bluetooth discovery should be used. Default: false
     * @return
     */
    bool UseNonBluetoothDiscovery() const;

    /**
     * @brief Gets a unique identifier
     * @return
     */
    DeviceTypeId ExpectedDeviceType() const;

    /**
     * @brief Indicates if the test data is enabled to be included in the test runs.
     * @return
     */
    bool IsEnabled() const;

    /**
     * @brief Indicates if the test data should be included in test runs, but skipped.
     * @return
     */
    bool IsSkipped() const { return skipped; }

    /**
     * @brief Indicates the reason the test data is disabled.
     * @return
     */
    const QString DisabledReason() const;

    /**
     * @brief Indicates the reason the test data is skipped.
     * @return
     */
    const QString SkippedReason() const;

    /**
     * @brief Indicates if the object is of the expected type.
     * @param device The device to check.
     * @return
     */
    bool IsExpectedDevice(bluetoothdevice * device) const;

    /**
     * @brief Gets a list of test data object names for devices that should prevent this test data object's expected device from being detected.
     * @return
     */
    const QStringList Exclusions() const;

    /**
     * @brief Gets the manager for the valid and invalid device names for this test data object.
     * @return
     */
    const DeviceNamePatternGroup * NamePatternGroup() const;

    /**
     * @brief Gets combinations of configurations beginning with the specified object.
     * @param info
     */
    std::vector<DeviceDiscoveryInfo> ApplyConfigurations(const DeviceDiscoveryInfo& info, bool enable) const;

    /**
     * @brief Calls the initializer for the device if it is defined.
     */
    void InitializeDevice() const;

    virtual ~BluetoothDeviceTestData();
};
