#include "devicenamepatterngroup.h"

DeviceNamePatternGroup::DeviceNamePatternGroup() {}


DeviceNamePatternGroup * DeviceNamePatternGroup::addDeviceName(const QString& deviceName, DeviceNameComparison cmp, uint8_t length) {

    int expectedLength = length < 0 ? 0:length;

    QStringList newNames;

    if(expectedLength==0 || expectedLength==deviceName.length())
        newNames.append(deviceName);

    if(cmp & DeviceNameComparison::StartsWith && (expectedLength==0 || expectedLength>deviceName.length())) {
        QString pad = expectedLength==0 ? "X" : QString(expectedLength-deviceName.length(), 'X');
        newNames.append(deviceName+pad);
    }

    this->deviceNames.append(newNames);

    if(deviceName!="") {
        // Only do this if the supplied device name is not empty
        // because the additional padded values in newNames represent the entire name
        // without any prefix with case to be sensitive to.
        if (cmp & DeviceNameComparison::IgnoreCase) {
            this->addDifferentCasings(newNames, this->deviceNames);
        } else {
            this->addDifferentCasings(newNames, this->invalidDeviceNames);
        }
    }

    return this;
}

DeviceNamePatternGroup * DeviceNamePatternGroup::addDeviceName(const QString& deviceNameStartsWith, const QString& deviceNameEndsWith, DeviceNameComparison cmp) {

    DeviceNameComparison modifiedComparison = (DeviceNameComparison)(cmp & ~DeviceNameComparison::StartsWith);
    this->addDeviceName(deviceNameStartsWith+deviceNameEndsWith, modifiedComparison);
    this->addDeviceName(deviceNameStartsWith+"XXX"+deviceNameEndsWith, modifiedComparison);
    return this;
}

void DeviceNamePatternGroup::addDifferentCasings(const QStringList& names, QStringList& target) {
    for(auto name : names) {
        QString newName = name.toUpper();
        if(newName!=name)
            target.append(newName);

        newName = name.toLower();
        if(newName!=name)
            target.append(newName);
    }
}


DeviceNamePatternGroup * DeviceNamePatternGroup::addInvalidDeviceName(const QString& deviceName, DeviceNameComparison cmp){
    QStringList newNames;

    newNames.append(deviceName);

    if(cmp & DeviceNameComparison::StartsWith)
        newNames.append(deviceName+"X");

    this->invalidDeviceNames.append(newNames);

    if(cmp & DeviceNameComparison::IgnoreCase) {
        addDifferentCasings(newNames, this->invalidDeviceNames);
    }

    return this;
}
