# QDomyos-Zwift Guide to Writing Unit Tests

## About

The testing project tst/qdomyos-zwift-tests.pro contains test code that uses the Google Test library.

## Adding a new device

New devices are added to the main QZ application by creating or modifying a subclass of the bluetoothdevice class.

At minimum, each device has a corresponding BluetoothDeviceTestData subclass in the test project, which is coded to provide information to the test framework to generate tests for device detection and potentially other things.

In the test project
* create a new folder for the device under tst/Devices. This is for anything you define for testing this device.
* add a new class with header file and optionally .cpp file to the project in that folder. Name the class DeviceNameTestData, substituting an appropriate name in place of "DeviceName".
* edit the header file to inherit the class from the BluetoothDeviceTestData abstract subclass appropriate to the device type, i.e. BikeTestData, RowerTestData, EllipticalTestData, TreadmillTestData.
* have this new subclass' constructor pass a unique test name to its superclass.

The tests are not organised around real devices that are handled, but the bluetoothdevice subclass that handles them - the "driver" of sorts.

You need to provide the following:
- patterns for valid names (e.g. equals a value, starts with a value, case sensitivity, specific length)
- invalid names to ensure the device is not identified when the name is invalid
- configuration settings that are required for the device to be detected
- invalid configurations to test that the device is not detected, e.g. when it's disabled in the settings, but the name is correct
- exclusion devices: if a device with the same name but of a higher priority type is detected, this device should not be detected
- valid and invalid QBluetoothDeviceInfo configurations, e.g. to check the device is only detected when the manufacturer data is set correctly, or certain services are available or not.

## Tools in the Test Framework

### TestSettings

The detection of many devices depends on settings that are accessed programmatically using the QSettings class and the constants in the QZSettings namespace. The TestSettings class stores a QSettings object with what is intended to be a unique application and organisation name, to keep the configuration it represents seperate from others in the system. It also makes the stored QSettings object the default by setting the QCoreApplication's organisation and application names to those of the QSettings object. The original values are restored by calling the deactivate() function or on object destruction.

i.e. a test will 
* apply a configuration from a TestSettings object
* perform device detection
* use the TestSettings object to restore the previous settings either directly or by letting its destructor be called.

### DeviceDiscoveryInfo

This class contains a set of fields that store strongly typed QSettings values. 
It also provides methods to read and write the values it knows about from and to a QSettings object.

It is used in conjunction with a TestSettings object to write a configuration during a test.


## Writing a device detection test

Because of the way the TestData classes currently work, it may be necessary to define multiple test data classes to cover the various cases.
For example, if any of a list of names is enough to identify a device, or another group of names but with a certain service in the bluetooth device info, that will require multiple classes.

### Recognition by Name

Consider the detection code for the Domyos Bike:

```
 } else if (b.name().startsWith(QStringLiteral("Domyos-Bike")) &&
                       !b.name().startsWith(QStringLiteral("DomyosBridge")) && !domyosBike && filter) {

```

Reading this, to identify this device:
- bluetooth name should start with "Domyos-Bike" using a case sensitive comparison
- bluetooth name should NOT start with "DomyosBridge", also using a case sensitive comparison
- there should not have been a device using the corresponding device class detected already (i.e. domyos)
- filter has not been activated (this isn't tested)

In this case, we are not testing the last two, but can test the first two.


```
#pragma once

#include "Devices/Bike/biketestdata.h"
#include "devices/domyosbike/domyosbike.h"

class DomyosBikeTestData : public BikeTestData {

public:
    DomyosBikeTestData() : BikeTestData("Domyos Bike") {

        this->addDeviceName("Domyos-Bike", comparison::StartsWith);
        this->addInvalidDeviceName("DomyosBridge", comparison::StartsWith);
    }

	// not used yet
    deviceType get_expectedDeviceType() const override { return deviceType::DomyosBike; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<domyosbike*>(detectedDevice)!=nullptr;
    }
};
```

The constructor adds a valid device name, and an invalid one. Various overloads of these methods and other members of the comparison enumeration provide other capabilities for specifying test data. If you add a valid device name that says the name should start with a value, additional names will be added automatically to the valid list with additional characters to test that it is in fact a "starts with" relationship. Also, valid and invalid names will be generated base on whether the comparison is case sensitive or not.

The get_expectedDeviceType() function is not actually used and is part of an unfinished refactoring of the device detection code, whereby the bluetoothdevice object doesn't actually get created intially. You could add a new value to the deviceType enum and return that, but it's not used yet. There's always deviceType::None.

The get_isExpectedDevice(bluetoothdevice *) function must be overridden to indicate if the specified object is of the type expected for this test data.

### Configuration Settings

Consider the CompuTrainerTestData. This device is not detected by name, but only by whether or not it is enabled in the settings.
To specify this in the test data, we override one of the configureSettings methods, the one for the simple case where there is a single valid and a single invalid configuration. 

Settings from QSettings that contribute to tests should be put into the DeviceDiscoveryInfo class.

For example, for the Computrainer Bike, the "computrainer_serial_port" value from the QSettings determines if the bike should be detected or not.

```
class DeviceDiscoveryInfo {
public :
	...
	QString computrainer_serial_port = nullptr;	
	...
}
```

The getValues and setValues methods should be updated to include the addition(s):

```

void DeviceDiscoveryInfo::setValues(QSettings &settings, bool clear) const {
	if(clear) settings.clear();	
	...
	settings.setValue(QZSettings::computrainer_serialport, this->computrainer_serial_port);
	...
}

void DeviceDiscoveryInfo::getValues(QSettings &settings){
	...
	this->computrainer_serial_port = settings.value(QZSettings::computrainer_serialport, QZSettings::default_computrainer_serialport).toString();
	...
}

```

In the following example, the DeviceDiscoveryInfo class has been updated to contain the device's configuration setting (computrainer_serial_port). 
- if an enabling configuration is requested (enable==true) a string that is known to be accepted is supplied
- if a disabling configuration is requested (enable==false) an empty string is supplied.

This example uses the simpler of 2 configureSettings methods returns true/false to indicate if the configuration should be used for the test.

```
#pragma once

#include "Devices/Bike/biketestdata.h"
#include "devices/computrainerbike/computrainerbike.h"

class CompuTrainerTestData : public BikeTestData {
protected:
    bool configureSettings(DeviceDiscoveryInfo& info, bool enable) const override {
        info.computrainer_serial_port = enable ? "X":QString();
        return true;
    }
public:
    CompuTrainerTestData() : BikeTestData("CompuTrainer Bike") {
        // any name
        this->addDeviceName("", comparison::StartsWithIgnoreCase);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::CompuTrainerBike; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<computrainerbike*>(detectedDevice)!=nullptr;
    }
};
```

Similarly, the Pafers Bike has a simple configuration setting:

```
#include "Devices/Bike/biketestdata.h"
#include "devices/pafersbike/pafersbike.h"


class PafersBikeTestData : public BikeTestData {
protected:
    bool configureSettings(DeviceDiscoveryInfo& info, bool enable) const override {
        // the treadmill is given priority
        info.pafers_treadmill = !enable;
        return true;
    }
public:
    PafersBikeTestData() : BikeTestData("Pafers Bike") {
        this->addDeviceName("PAFERS_", comparison::StartsWithIgnoreCase);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::PafersBike; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<pafersbike*>(detectedDevice)!=nullptr;
    }
};
```

A more complicated example is the Pafers Treadmill. It involves a name match, but also some configuration settings obtained earlier...

```
bool pafers_treadmill = settings.value(QZSettings::pafers_treadmill, QZSettings::default_pafers_treadmill).toBool();
...

bool pafers_treadmill_bh_iboxster_plus =
        settings
            .value(QZSettings::pafers_treadmill_bh_iboxster_plus, QZSettings::default_pafers_treadmill_bh_iboxster_plus)
            .toBool();
...

} else if (b.name().toUpper().startsWith(QStringLiteral("PAFERS_")) && !pafersTreadmill &&
                       (pafers_treadmill || pafers_treadmill_bh_iboxster_plus) && filter) {
```

Here the device could be activated due to a name match and various combinations of settings.
For this, the configureSettings function that takes a vector of DeviceDiscoveryInfo objects which is populated with configurations that lead to the specified result (enable = detected, !enable=not detected). Instead of returning a boolean to indicate if a configuration has been supplied, it populates a vector of DeviceDiscoveryInfo objects.

```
#pragma once

#include "Devices/Treadmill/treadmilltestdata.h"
#include "devices/paferstreadmill/paferstreadmill.h"

class PafersTreadmillTestData : public TreadmillTestData {
protected:
    void configureSettings(const DeviceDiscoveryInfo& info, bool enable, std::vector<DeviceDiscoveryInfo>& configurations) const override {
        DeviceDiscoveryInfo config(info);

        if (enable) {
            for(int x = 1; x<=3; x++) {
                config.pafers_treadmill = x & 1;
                config.pafers_treadmill_bh_iboxster_plus = x & 2;
                configurations.push_back(config);
            }
        } else {
            config.pafers_treadmill = false;
            config.pafers_treadmill_bh_iboxster_plus = false;
            configurations.push_back(config);
        }
    }

public:
    PafersTreadmillTestData() : TreadmillTestData("Pafers Treadmill") {
        this->addDeviceName("PAFERS_", comparison::StartsWithIgnoreCase);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::PafersTreadmill; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<paferstreadmill*>(detectedDevice)!=nullptr;
    }
};
```

### Considering Extra QBluetoothDeviceInfo Content

Detection of some devices requires some specific bluetooth device information. 

Supplying enabling and disabling QBluetoothDeviceInfo objects is done using a similar pattern to the multiple configurations scenario.
For example, the M3iBike requires specific manufacturer information.


```
void M3IBikeTestData::configureBluetoothDeviceInfos(const QBluetoothDeviceInfo& info,  bool enable, std::vector<QBluetoothDeviceInfo>& bluetoothDeviceInfos) const {
    // The M3I bike detector looks into the manufacturer data.

    QBluetoothDeviceInfo result = info;

    if(!enable) {
        result.setManufacturerData(1, QByteArray("Invalid manufacturer data."));
        bluetoothDeviceInfos.push_back(result);

        return;
    }

    int key=0;
    result.setManufacturerData(key++, hex2bytes("02010639009F00000000000000000014008001"));

    bluetoothDeviceInfos.push_back(result);
}

```

The test framework populates the incoming QBluetoothDeviceInfo object with a name and a UUID. This is expected to have nothing else defined. 
Another example is one of the test data classes for detecting a device that uses the statesbike class:

Detection code from bluetooth.cpp:

```
((b.name().toUpper().startsWith("KICKR CORE")) && !deviceHasService(b, QBluetoothUuid((quint16)0x1826)) && deviceHasService(b, QBluetoothUuid((quint16)0x1818)))
```

This condition is actually extracted from a more complicated example where the current test data classes can't cover all the detection criteria in one implementation. This is why this class inherits from StagesBikeTestData rather than BikeTestData directly.

```
class StagesBike3TestData : public StagesBikeTestData {
protected:
    void configureBluetoothDeviceInfos(const QBluetoothDeviceInfo& info,  bool enable, std::vector<QBluetoothDeviceInfo>& bluetoothDeviceInfos) const override {
        // The condition, if the name is acceptable, is:
        // !deviceHasService(b, QBluetoothUuid((quint16)0x1826)) && deviceHasService(b, QBluetoothUuid((quint16)0x1818)))

        if(enable) {
            QBluetoothDeviceInfo result = info;
            result.setServiceUuids(QVector<QBluetoothUuid>({QBluetoothUuid((quint16)0x1818)}));
            bluetoothDeviceInfos.push_back(result);
        } else {
            QBluetoothDeviceInfo hasInvalid = info;
            hasInvalid.setServiceUuids(QVector<QBluetoothUuid>({QBluetoothUuid((quint16)0x1826)}));
            QBluetoothDeviceInfo hasBoth = hasInvalid;
            hasBoth.setServiceUuids(QVector<QBluetoothUuid>({QBluetoothUuid((quint16)0x1818),QBluetoothUuid((quint16)0x1826)}));

            bluetoothDeviceInfos.push_back(info); // has neither
            bluetoothDeviceInfos.push_back(hasInvalid);
            bluetoothDeviceInfos.push_back(hasBoth);
        }
    }

public:
    StagesBike3TestData() : StagesBikeTestData("Stages Bike (KICKR CORE)") {

        this->addDeviceName("KICKR CORE", comparison::StartsWithIgnoreCase);
    }
};
```

In this case, it populates the vector with the single enabling configuration if that's what's been requested, otherwise 3 disabling ones.

### Exclusions

Sometimes there might be ambiguity when multiple devices are available, and the detection code may specify that if the other conditions match, but certain specific kinds of devices (the exclusion devices) have already been detected, the newly matched device should be ignored.

The TestData class can be made to cover this by overriding the configureExclusions() method to add instances of the TestData classes for the exclusion devices to the object's internal list of exclusions.

Detection code:

```
} else if (b.name().startsWith(QStringLiteral("ECH")) && !echelonRower && !echelonStride &&
                       !echelonConnectSport && filter) {
```
The configureExclusions code is overridden to specify the exclusion test data objects. Note that the test for a previously detected device of the same type is not included.

```
#pragma once

#include "Devices/Bike/biketestdata.h"
#include "Devices/EchelonRower/echelonrowertestdata.h"
#include "Devices/EchelonStrideTreadmill/echelonstridetreadmilltestdata.h"
#include "devices/echelonconnectsport/echelonconnectsport.h"

class EchelonConnectSportBikeTestData : public BikeTestData {

public:
    EchelonConnectSportBikeTestData() : BikeTestData("Echelon Connect Sport Bike") {
        this->addDeviceName("ECH", comparison::StartsWith);
    }

    void configureExclusions() override {
        this->exclude(new EchelonRowerTestData());
        this->exclude(new EchelonStrideTreadmillTestData());
    }

    deviceType get_expectedDeviceType() const override { return deviceType::EchelonConnectSport; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<echelonconnectsport*>(detectedDevice)!=nullptr;
    }
};


```

### When a single TestData Class Can't Cover all the Conditions

Detection code:

```
QString powerSensorName =
        settings.value(QZSettings::power_sensor_name, QZSettings::default_power_sensor_name).toString();
...
} else if ((b.name().toUpper().startsWith(QStringLiteral("STAGES ")) ||
                        (b.name().toUpper().startsWith("TACX SATORI")) ||
                        ((b.name().toUpper().startsWith("KICKR CORE")) && !deviceHasService(b, QBluetoothUuid((quint16)0x1826)) && deviceHasService(b, QBluetoothUuid((quint16)0x1818))) ||
                        (b.name().toUpper()==QStringLiteral("QD")) ||
                        (b.name().toUpper().startsWith(QStringLiteral("ASSIOMA")) &&
                         powerSensorName.startsWith(QStringLiteral("Disabled")))) &&
                       !stagesBike && !ftmsBike && filter) {
```

This presents 3 scenarios for the current test framework. 
1. Match names only (starts with:"STAGES ", starts with: "TACX SATORI", equals: "QD")
2. Match the name "KICKR CORE", presence and absence of specific service ids
3. Match the name "ASSIOMA" and the power sensor name setting starts with "Disabled"
					   
The framework is not currently capable of specifying all these scenarios in a single class. 
The generated test data is approximately the combinations of these lists: names * settings * bluetoothdeviceInfo * exclusions.
If a combination should not exist, a separate class should be used.

In the example of the StagesBikeTestData classes, the exclusions, which apply to all situations, are implemented in the superclass StagesBikeTestData,


```
#pragma once

#include "Devices/Bike/biketestdata.h"
#include "devices/stagesbike/stagesbike.h"
#include "Devices/FTMSBike/ftmsbiketestdata.h"

class StagesBikeTestData : public BikeTestData {
protected:
    StagesBikeTestData(std::string testName): BikeTestData(testName) {
    }
   
    void configureExclusions() override {
        this->exclude(new FTMSBike1TestData());
        this->exclude(new FTMSBike2TestData());
    }
    
public:

    deviceType get_expectedDeviceType() const override { return deviceType::StagesBike; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<stagesbike*>(detectedDevice)!=nullptr;
    }
};
```

The name-match only in one subclass:

```
class StagesBike1TestData : public StagesBikeTestData {

public:
    StagesBike1TestData() : StagesBikeTestData("Stages Bike") {
        this->addDeviceName("STAGES ", comparison::StartsWithIgnoreCase);
        this->addDeviceName("TACX SATORI", comparison::StartsWithIgnoreCase);
    }

};

```

The name and setting match in another subclass:

```

class StagesBike2TestData : public StagesBikeTestData {
protected:
    bool configureSettings(DeviceDiscoveryInfo& info, bool enable) const override {
        info.powerSensorName = enable ? "Disabled":"Roberto";
        return true;
    }
public:
    StagesBike2TestData() : StagesBikeTestData("Stages Bike (Assioma / Power Sensor disabled") {

        this->addDeviceName("ASSIOMA", comparison::StartsWithIgnoreCase);
    }
};

```
The name and bluetooth device info configurations in another:

```

class StagesBike3TestData : public StagesBikeTestData {
protected:
    void configureBluetoothDeviceInfos(const QBluetoothDeviceInfo& info,  bool enable, std::vector<QBluetoothDeviceInfo>& bluetoothDeviceInfos) const override {
        // The condition, if the name is acceptable, is:
        // !deviceHasService(b, QBluetoothUuid((quint16)0x1826)) && deviceHasService(b, QBluetoothUuid((quint16)0x1818)))

        if(enable) {
            QBluetoothDeviceInfo result = info;
            result.setServiceUuids(QVector<QBluetoothUuid>({QBluetoothUuid((quint16)0x1818)}));
            bluetoothDeviceInfos.push_back(result);
        } else {
            QBluetoothDeviceInfo hasInvalid = info;
            hasInvalid.setServiceUuids(QVector<QBluetoothUuid>({QBluetoothUuid((quint16)0x1826)}));
            QBluetoothDeviceInfo hasBoth = hasInvalid;
            hasBoth.setServiceUuids(QVector<QBluetoothUuid>({QBluetoothUuid((quint16)0x1818),QBluetoothUuid((quint16)0x1826)}));

            bluetoothDeviceInfos.push_back(info); // has neither
            bluetoothDeviceInfos.push_back(hasInvalid);
            bluetoothDeviceInfos.push_back(hasBoth);
        }
    }

public:
    StagesBike3TestData() : StagesBikeTestData("Stages Bike (KICKR CORE)") {

        this->addDeviceName("KICKR CORE", comparison::StartsWithIgnoreCase);
    }
};

```

## Telling Google Test Where to Look

To register your test data class(es) with Google Test:

- open tst/Devices/devices.h
- add a #include for your new header file(s)
- add your new classes to the BluetoothDeviceTestDataTypes list.

This will add tests for your new device class to test runs of the tests in the BluetoothDeviceTestSuite class, which are about detecting, and not detecting devices in circumstances generated from the TestData classes.


