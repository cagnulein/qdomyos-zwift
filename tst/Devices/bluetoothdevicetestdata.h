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
    ConfigurationApplicatorMultiple configuratorMultiple=nullptr;
    ConfigurationApplicatorSingle configuratorSingle=nullptr;
    std::function<bool(bluetoothdevice*)> isExpectedDevice=nullptr;
    DeviceTypeId expectedDeviceType=-1;
    BluetoothDeviceTestData();
public:
    QString Name() const;
    DeviceTypeId ExpectedDeviceType() const;
    bool IsEnabled() const;
    bool IsSkipped() const { return skipped; }
    const QString DisabledReason() const;
    const QString SkippedReason() const;
    bool IsExpectedDevice(bluetoothdevice * device) const;
    const QStringList Exclusions() const;
    const DeviceNamePatternGroup * NamePatternGroup() const;

    /**
     * @brief Gets combinations of configurations beginning with the specified object.
     * @param info
     */
    std::vector<DeviceDiscoveryInfo> ApplyConfigurations(const DeviceDiscoveryInfo& info, bool enable) const;

    virtual ~BluetoothDeviceTestData();
};
