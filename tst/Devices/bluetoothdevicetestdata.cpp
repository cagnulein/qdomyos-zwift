#include "bluetoothdevicetestdata.h"


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
