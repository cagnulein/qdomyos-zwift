#include "bluetoothdevicetestdata.h"


void BluetoothDeviceTestData::exclude(BluetoothDeviceTestData *testData) {
    this->exclusions.push_back(std::shared_ptr<BluetoothDeviceTestData>(testData));
}

void BluetoothDeviceTestData::addDeviceName(const QString& deviceName, bool multiCase, bool isStartsWith) {
    this->deviceNames.append(deviceName);

    if(isStartsWith) {
        this->deviceNames.append(deviceName+"X");
    }

    if(multiCase) {
        QString upper = deviceName.toUpper();
        QString lower = deviceName.toLower();

        if(deviceName!=upper) this->deviceNames.append(upper);
        if(deviceName!=lower) this->deviceNames.append(lower);
    }
}

void BluetoothDeviceTestData::addDeviceName(const QString& deviceNameStartsWith, const QString& deviceNameEndsWith, bool multiCase) {

    this->addDeviceName(deviceNameStartsWith+deviceNameEndsWith, multiCase, false);
    this->addDeviceName(deviceNameStartsWith+"XXX"+deviceNameEndsWith, multiCase, false);

}

void BluetoothDeviceTestData::addInvalidDeviceName(const QString& deviceName) { this->invalidDeviceNames.append(deviceName); }

QStringList BluetoothDeviceTestData::get_deviceNames() const { return this->deviceNames;}

QStringList BluetoothDeviceTestData::get_failingDeviceNames() const { return this->invalidDeviceNames; }

std::vector<std::shared_ptr<BluetoothDeviceTestData> > BluetoothDeviceTestData::get_exclusions() const {
    return this->exclusions;
}
