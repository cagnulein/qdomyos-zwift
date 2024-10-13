#include "bluetoothdevicetestdatabuilder.h"

BluetoothDeviceTestDataBuilder::BluetoothDeviceTestDataBuilder(QString name) : BluetoothDeviceTestData() {
    this->name = name;
    this->deviceNamePatternGroup = new DeviceNamePatternGroup();
}

BluetoothDeviceTestDataBuilder *BluetoothDeviceTestDataBuilder::acceptDeviceNames(const QStringList &deviceNames, DeviceNameComparison cmp)
{
    for(QString name : deviceNames)
        this->acceptDeviceName(name, cmp);
    return this;
}

BluetoothDeviceTestDataBuilder *BluetoothDeviceTestDataBuilder::acceptDeviceName(const QString &deviceName, DeviceNameComparison cmp, uint8_t length) {
    this->deviceNamePatternGroup->addDeviceName(deviceName, cmp, length);
    return this;
}

BluetoothDeviceTestDataBuilder *BluetoothDeviceTestDataBuilder::acceptDeviceName(const QString &deviceNameStartsWith, const QString &deviceNameEndsWith, DeviceNameComparison cmp) {
    this->deviceNamePatternGroup->addDeviceName(deviceNameStartsWith, deviceNameEndsWith, cmp);
    return this;
}

BluetoothDeviceTestDataBuilder *BluetoothDeviceTestDataBuilder::rejectDeviceName(const QString &deviceName, const DeviceNameComparison cmp) {
    this->deviceNamePatternGroup->addInvalidDeviceName(deviceName, cmp);
    return this;
}

BluetoothDeviceTestDataBuilder *BluetoothDeviceTestDataBuilder::configureSettingsWith(ConfigurationApplicatorMultiple applicator) {
    if(this->applicatorMultiple || this->applicatorSingle)
        throw std::invalid_argument("Only 1 configurator is supported.");
    this->applicatorMultiple = applicator;
    return this;
}

BluetoothDeviceTestDataBuilder *BluetoothDeviceTestDataBuilder::configureSettingsWith(ConfigurationApplicatorSingle applicator) {
    if(this->applicatorMultiple || this->applicatorSingle)
        throw std::invalid_argument("Only 1 configurator is supported.");
    this->applicatorSingle = applicator;
    return this;
}

BluetoothDeviceTestDataBuilder *BluetoothDeviceTestDataBuilder::configureSettingsWith(const QString &qzSettingsKey, bool enablingValue) {
    return this->configureSettingsWith(qzSettingsKey, enablingValue, !enablingValue);
}

BluetoothDeviceTestDataBuilder *BluetoothDeviceTestDataBuilder::configureSettingsWith(const QString &qzSettingsKey, QVariant enablingValue, QVariant disablingValue) {
    if(this->applicatorMultiple || this->applicatorSingle)
        throw std::invalid_argument("Only 1 configurator is supported.");

    if(enablingValue==disablingValue)
        throw std::invalid_argument("Enabling and disabling values must be different.");

    this->applicatorSingle = [qzSettingsKey, enablingValue, disablingValue](DeviceDiscoveryInfo& info, bool enable) -> void {
        info.setValue(qzSettingsKey, enable ? enablingValue:disablingValue);
    };
    return this;
}

BluetoothDeviceTestDataBuilder *BluetoothDeviceTestDataBuilder::configureSettingsWith(const QBluetoothUuid &uuid, bool addedIsEnabled) {
    this->applicatorSingle = [uuid,addedIsEnabled](DeviceDiscoveryInfo& info, bool enable) -> void {
        if(enable==addedIsEnabled)
            info.addBluetoothService(uuid);
        else
            info.removeBluetoothService(uuid);
    };
    return this;
}

BluetoothDeviceTestDataBuilder *BluetoothDeviceTestDataBuilder::excluding(std::initializer_list<DeviceTypeId> exclusions) {
    this->exclusions.insert(exclusions);
    return this;
}

BluetoothDeviceTestDataBuilder *BluetoothDeviceTestDataBuilder::disable(const QString& reason) {
    this->enabled = false;
    this->disabledReason = reason;
    return this;
}

BluetoothDeviceTestDataBuilder *BluetoothDeviceTestDataBuilder::skip(const QString &reason) {
    this->skipped = true;
    this->skippedReason = reason;
    return this;
}


