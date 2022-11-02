#include "bluetoothdevicetestdata.h"


void BluetoothDeviceTestData::configureExclusions() {}

void BluetoothDeviceTestData::exclude(BluetoothDeviceTestData *testData) {
    if(!this->configuringExclusions)
        throw "Exclusions can only be specified from the configureExclusions method.";

    this->exclusions.push_back(std::shared_ptr<BluetoothDeviceTestData>(testData));
}

void BluetoothDeviceTestData::addDeviceName(const QString& deviceName, comparison cmp, uint8_t length) {

    int expectedLength = length < 0 ? 0:length;

    QStringList newNames;

    if(expectedLength==0 || expectedLength==deviceName.length())
        newNames.append(deviceName);

    if(cmp & comparison::StartsWith && (expectedLength==0 || expectedLength>deviceName.length())) {
        QString pad = expectedLength==0 ? "X" : QString(expectedLength-deviceName.length(), 'X');
        newNames.append(deviceName+pad);
    }

    this->deviceNames.append(newNames);


    if(cmp & comparison::IgnoreCase) {
        addDifferentCasings(newNames, this->deviceNames);
    } else {
        addDifferentCasings(newNames, this->invalidDeviceNames);
    }
}

void BluetoothDeviceTestData::addDeviceName(const QString& deviceNameStartsWith, const QString& deviceNameEndsWith, comparison cmp) {

    comparison modifiedComparison = (comparison)(cmp & ~comparison::StartsWith);
    this->addDeviceName(deviceNameStartsWith+deviceNameEndsWith, modifiedComparison);
    this->addDeviceName(deviceNameStartsWith+"XXX"+deviceNameEndsWith, modifiedComparison);

}

void BluetoothDeviceTestData::addDifferentCasings(const QStringList& names, QStringList& target) {
    for(auto name : names) {
        QString newName = name.toUpper();
        if(newName!=name)
            target.append(newName);

        newName = name.toLower();
        if(newName!=name)
            target.append(newName);
    }
}

void BluetoothDeviceTestData::configureSettings(const devicediscoveryinfo &info, bool enable, std::vector<devicediscoveryinfo> configurations) const { }

bool BluetoothDeviceTestData::configureSettings(devicediscoveryinfo &info, bool enable) const { return false;}

BluetoothDeviceTestData::BluetoothDeviceTestData() {
    // You can do set-up work for each test here.
}

BluetoothDeviceTestData::~BluetoothDeviceTestData() {}

void BluetoothDeviceTestData::addInvalidDeviceName(const QString& deviceName, comparison cmp){
    QStringList newNames;

    newNames.append(deviceName);

    if(cmp & comparison::StartsWith)
        newNames.append(deviceName+"X");

    this->invalidDeviceNames.append(newNames);

    if(cmp & comparison::IgnoreCase) {
        addDifferentCasings(newNames, this->invalidDeviceNames);
    }
}

QStringList BluetoothDeviceTestData::get_deviceNames() const { return this->deviceNames;}

QStringList BluetoothDeviceTestData::get_failingDeviceNames() const { return this->invalidDeviceNames; }

std::vector<std::shared_ptr<BluetoothDeviceTestData> > BluetoothDeviceTestData::get_exclusions() {
    if(!this->exclusionsConfigured) {
        this->configuringExclusions = true;
        this->configureExclusions();
        this->exclusionsConfigured = true;
        this->configuringExclusions = false;

    }
    return this->exclusions;
}

std::vector<devicediscoveryinfo> BluetoothDeviceTestData::get_configurations(const devicediscoveryinfo &info, bool enable) {
    std::vector<devicediscoveryinfo> result;

    devicediscoveryinfo newInfo(info);
    if(this->configureSettings(newInfo, enable))
        result.push_back(newInfo);

    this->configureSettings(info, enable, result);

    return result;
}

bool BluetoothDeviceTestData::get_testInvalidBluetoothDeviceInfo() const { return this->testInvalidBluetoothDeviceInfo; }

QString BluetoothDeviceTestData::get_testIP() const { return "1.2.3.4"; }

QBluetoothDeviceInfo BluetoothDeviceTestData::get_bluetoothDeviceInfo(const QBluetoothUuid &uuid, const QString &name, bool valid) {
    if(!valid)
        throw "Invalid bluetooth device info is not implemented in this class.";
    return QBluetoothDeviceInfo(uuid, name, 0);
}
