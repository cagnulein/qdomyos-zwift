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

void BluetoothDeviceTestData::configureSettings(const DeviceDiscoveryInfo &info, bool enable, std::vector<DeviceDiscoveryInfo>& configurations) const { }

bool BluetoothDeviceTestData::configureSettings(DeviceDiscoveryInfo &info, bool enable) const { return false;}

BluetoothDeviceTestData::BluetoothDeviceTestData(std::string testName) {
    this->testName = testName;
}

BluetoothDeviceTestData::~BluetoothDeviceTestData() {}

std::string BluetoothDeviceTestData::get_testName() const {
    return this->testName;
}

bool BluetoothDeviceTestData::get_isAbstract() const { return false; }

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

std::vector<DeviceDiscoveryInfo> BluetoothDeviceTestData::get_configurations(const DeviceDiscoveryInfo &info, bool enable) {
    std::vector<DeviceDiscoveryInfo> result;

    DeviceDiscoveryInfo newInfo(info);
    if(this->configureSettings(newInfo, enable))
        result.push_back(newInfo);

    this->configureSettings(info, enable, result);

    return result;
}

QString BluetoothDeviceTestData::get_testIP() const { return "1.2.3.4"; }

std::vector<QBluetoothDeviceInfo> BluetoothDeviceTestData::get_bluetoothDeviceInfo(const QBluetoothUuid &uuid, const QString &name, bool valid) {
    std::vector<QBluetoothDeviceInfo> result;

    QBluetoothDeviceInfo info(uuid, name, 0);

    this->configureBluetoothDeviceInfos(info, valid, result);

    // make sure there is always a valid item
    if(valid && result.empty())
        result.push_back(info);

    return result;
}

