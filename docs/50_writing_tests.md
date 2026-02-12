# QDomyos-Zwift Guide to Writing Unit Tests

## About

The testing project tst/qdomyos-zwift-tests.pro contains test code that uses the Google Test library.

## Adding a new device

New devices are added to the main QZ application by creating or modifying a subclass of the bluetoothdevice class.

At minimum, each device has a corresponding BluetoothDeviceTestData object constructed in the DeviceTestDataIndex class in the test project, which is coded to provide information to the test framework to generate tests for device detection and potentially other things.

In the test project
* add a new device name constant to the DeviceIndex class.
* locate the implementation of DeviceTestDataindex::Initialize and build the test data from a call to DeviceTestDataIndex::RegisterNewDeviceTestData(...)
* pass the device name constant defined in the DeviceIndex class  to the call to DeviceTestDataIndex::RegisterNewDeviceTestData(...).

The tests are not organised around real devices that are handled, but the bluetoothdevice subclass that handles them - the "driver" of sorts.

You need to provide the following:
- patterns for valid names (e.g. equals a value, starts with a value, case sensitivity, specific length)
- invalid names to ensure the device is not identified when the name is invalid
- configuration settings that are required for the device to be detected, including bluetooth device information configuration
- invalid configurations to test that the device is not detected, e.g. when it's disabled in the settings, but the name is correct
- exclusion devices: for example if a device with the same name but of a higher priority type is detected, this device should not be detected

## Tools in the Test Framework

### TestSettings

The detection of many devices depends on settings that are accessed programmatically using the QSettings class and the constants in the QZSettings namespace. The TestSettings class stores a QSettings object with what is intended to be a unique application and organisation name, to keep the configuration it represents seperate from others in the system. It also makes the stored QSettings object the default by setting the QCoreApplication's organisation and application names to those of the QSettings object. The original values are restored by calling the deactivate() function or on object destruction.

i.e. a test will 
* apply a configuration from a TestSettings object
* perform device detection
* use the TestSettings object to restore the previous settings either directly or by letting its destructor be called.

### DeviceDiscoveryInfo

This class:
* stores values for a specific subset of the QZSettings keys.
* provides methods to read and write the values it knows about from and to a QSettings object.
* provides a QBluetoothDeviceInfo object configured with the device name currently being tested.

It is used in conjunction with a TestSettings object to write a configuration during a test.


## Writing a device detection test

Because of the way the BluetoothDeviceTestDataBuilder currently works, it may be necessary to define multiple test data objects to cover the various cases.
For example, if any of a list of names is enough to identify a device, or another group of names but with a certain service in the bluetooth device info, that will require multiple test data objects.

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

In deviceindex.h:

```
static const QString DomyosBike;
```

In deviceindex.cpp:

```
DEFINE_DEVICE(DomyosBike, "Domyos Bike");
```

This pair adds the "friendly name" for the device as a constant, and also adds the key/value pair to an index.

In DeviceTestDataIndex::Initialize():

```
// Domyos bike
RegisterNewDeviceTestData(DeviceIndex::DomyosBike)
	->expectDevice<domyosbike>()        
	->acceptDeviceName("Domyos-Bike", DeviceNameComparison::StartsWith)
	->rejectDeviceName("DomyosBridge", DeviceNameComparison::StartsWith);
```

This set of instructions adds a valid device name, and an invalid one. Various overloads of these methods, other methods, and other members of the comparison enumeration provide other capabilities for specifying test data. If you add a valid device name that says the name should start with a value, additional names will be added automatically to the valid list with additional characters to test that it is in fact a "starts with" relationship. Also, valid and invalid names will be generated based on whether the comparison is case sensitive or not.

### Configuration Settings

Consider the CompuTrainer bike. This device is not detected by name, but only by whether or not it is enabled in the settings.
To specify this in the test data, we use one of the BluetoothDeviceTestData::configureSettingsWith(...) methods, the one for the simple case where there is a single QZSetting with a specific enabling and disabling value.

Settings from QSettings that contribute to tests should be put into the DeviceDiscoveryInfo class.

For example, for the Computrainer Bike, the "computrainer_serialport" value from the QSettings determines if the bike should be detected or not.

The computrainer_serialport QZSettings key should be registered in devicediscoveryinfo.cpp

In devicediscoveryinfo.cpp:
```
void InitializeTrackedSettings() {

	...

	trackedSettings.insert(QZSettings::computrainer_serialport, QZSettings::default_computrainer_serialport);

	...
}

```

For this test data,
* if enabling configurations are requested, the computrainer_serialport setting will be populated with "COMX"
* if disabling configurations are requested, the computrainer_serialport setting will be populated with ""

DeviceTestDataIndex::Initialize():

```
// Computrainer Bike
RegisterNewDeviceTestData(DeviceIndex::ComputrainerBike)
	->expectDevice<computrainerbike>()
	->acceptDeviceName("", DeviceNameComparison::StartsWithIgnoreCase)
	->configureSettingsWith(QZSettings::computrainer_serialport, "COMX", "");
```


Similarly, the Pafers Bike has a simple configuration setting:

```
    // Pafers Bike
    RegisterNewDeviceTestData(DeviceIndex::PafersBike)
        ->expectDevice<pafersbike>()
        ->acceptDeviceName("PAFERS_", DeviceNameComparison::StartsWithIgnoreCase)
        ->configureSettingsWith(QZSettings::pafers_treadmill,false);
```

In that case, ```configureSettingsWith(QZSettings::pafers_treadmill,false)``` indicates that the pafers_treadmill setting will be false for enabling configurations and true for disabling ones.

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
For this, the configureSettingsWith(...) function that takes a lambda function which consumes a vector of DeviceDiscoveryInfo objects which is populated with configurations that lead to the specified result (enable = detected, !enable=not detected).

```
// Pafers Treadmill
RegisterNewDeviceTestData(DeviceIndex::PafersTreadmill)
	->expectDevice<paferstreadmill>()
	->acceptDeviceName("PAFERS_", DeviceNameComparison::StartsWithIgnoreCase)
	->configureSettingsWith( [](const DeviceDiscoveryInfo& info, bool enable, std::vector<DeviceDiscoveryInfo>& configurations)->void {
		DeviceDiscoveryInfo config(info);

		if (enable) {
			for(int x = 1; x<=3; x++) {
				config.setValue(QZSettings::pafers_treadmill, x & 1);
				config.setValue(QZSettings::pafers_treadmill_bh_iboxster_plus, x & 2);
				configurations.push_back(config);
			}
		} else {
			config.setValue(QZSettings::pafers_treadmill, false);
			config.setValue(QZSettings::pafers_treadmill_bh_iboxster_plus, false);
			configurations.push_back(config);
		}
	});
```

### Considering Extra QBluetoothDeviceInfo Content

Detection of some devices requires some specific bluetooth device information. 

Supplying enabling and disabling QBluetoothDeviceInfo objects is done by accessing the QBluetoothDeviceInfo member of the DeviceDiscoveryInfo object.
For example, the M3iBike requires specific manufacturer information, using the simpler of the lambda functions accepted by the configureSettingsWith function.

```
	// M3I Bike
    RegisterNewDeviceTestData(DeviceIndex::M3IBike)
        ->expectDevice<m3ibike>()
        ->acceptDeviceName("M3", DeviceNameComparison::StartsWith)
        ->configureSettingsWith(
            [](DeviceDiscoveryInfo& info,  bool enable)->void
            {
                // The M3I bike detector looks into the manufacturer data.
                if(!enable) {
                    info.DeviceInfo()->setManufacturerData(1, QByteArray("Invalid manufacturer data."));
                    return;
                }

                int key=0;
                info.DeviceInfo()->setManufacturerData(key++, hex2bytes("02010639009F00000000000000000014008001"));
            });

```

The test framework populates the incoming QBluetoothDeviceInfo object with a UUID and the name (generated from the acceptDeviceName and rejectDeviceName calls) currently being tested.
This is expected to have nothing else defined. 
Another example is one of the test data definitions for detecting a device that uses the stagesbike class:

Detection code from bluetooth.cpp:

```
((b.name().toUpper().startsWith("KICKR CORE")) && !deviceHasService(b, QBluetoothUuid((quint16)0x1826)) && deviceHasService(b, QBluetoothUuid((quint16)0x1818)))
```

This condition is actually extracted from a more complicated example where the BluetoothDeviceTestData class can't cover all the detection criteria with one instance. 

```
// Stages Bike General
auto stagesBikeExclusions = { GetTypeId<ftmsbike>() };

//
// ... other stages bike variants
//

// Stages Bike (KICKR CORE)
RegisterNewDeviceTestData(DeviceIndex::StagesBike_KICKRCORE)
	->expectDevice<stagesbike>()        
	->acceptDeviceName("KICKR CORE", DeviceNameComparison::StartsWithIgnoreCase)
	->excluding(stagesBikeExclusions)
	->configureSettingsWith(
		[](const DeviceDiscoveryInfo& info,  bool enable, std::vector<DeviceDiscoveryInfo>& configurations)->void
		{
			// The condition, if the name is acceptable, is:
			// !deviceHasService(b, QBluetoothUuid((quint16)0x1826)) && deviceHasService(b, QBluetoothUuid((quint16)0x1818)))

			if(enable) {
				DeviceDiscoveryInfo result = info;
				result.addBluetoothService(QBluetoothUuid((quint16)0x1818));
				result.removeBluetoothService(QBluetoothUuid((quint16)0x1826));
				configurations.push_back(result);
			} else {
				DeviceDiscoveryInfo hasNeither = info;
				hasNeither.removeBluetoothService(QBluetoothUuid((quint16)0x1818));
				hasNeither.removeBluetoothService(QBluetoothUuid((quint16)0x1826));

				DeviceDiscoveryInfo hasInvalid = info;
				hasInvalid.addBluetoothService(QBluetoothUuid((quint16)0x1826));
				DeviceDiscoveryInfo hasBoth = hasInvalid;
				hasBoth.addBluetoothService(QBluetoothUuid((quint16)0x1818));
				hasBoth.addBluetoothService(QBluetoothUuid((quint16)0x1826));

				configurations.push_back(info); // has neither
				configurations.push_back(hasInvalid);
				configurations.push_back(hasBoth);
			}
		});

```

In this case, it populates the vector with the single enabling configuration if that's what's been requested, otherwise 3 disabling ones.

### Exclusions

Sometimes there might be ambiguity when multiple devices are available, and the detection code may specify that if the other conditions match, but certain specific kinds of devices (the exclusion devices) have already been detected, the newly matched device should be ignored.

The test data object can be made to cover this by calling the excluding(...) functions to add type identifiers for the bluetoothdevice classes for the exclusion devices to the object's internal list of exclusions.

Detection code:

```
} else if (b.name().startsWith(QStringLiteral("ECH")) && !echelonRower && !echelonStride &&
                       !echelonConnectSport && filter) {
```
The excluding<T>() template function is called to specify the exclusion device type. Note that the test for a previously detected device of the same type is not included.

```
// Echelon Connect Sport Bike
RegisterNewDeviceTestData(DeviceIndex::EchelonConnectSportBike)
	->expectDevice<echelonconnectsport>()        
	->acceptDeviceName("ECH", DeviceNameComparison::StartsWith)
	->excluding<echelonrower>()
	->excluding<echelonstride>();

```

### When a single test data object can't cover all the conditions

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
					   
The framework is not currently capable of specifying all these scenarios in a single test data object, without checking the name of the supplied QBluetoothDeviceInfo object against name conditions specified and constructing extra configurations based on that.
The generated test data is approximately the combinations of these lists: names * settings * exclusions.
If a combination should not exist, separate test data objects should be used.

In the example of the Stages Bike test data, the exclusions, which apply to all situations, are implemented in an array of type ids:


```
// Stages Bike General
auto stagesBikeExclusions = { GetTypeId<ftmsbike>() };
```

The name-match only in one test data instance:

```
// Stages Bike
RegisterNewDeviceTestData(DeviceIndex::StagesBike)
	->expectDevice<stagesbike>()        
	->acceptDeviceNames({"STAGES ", "TACX SATORI"}, DeviceNameComparison::StartsWithIgnoreCase)
	->acceptDeviceName("QD", DeviceNameComparison::IgnoreCase)
	->excluding(stagesBikeExclusions);
```

The name and setting match in another instance:

```
// Stages Bike Stages Bike (Assioma / Power Sensor disabled
RegisterNewDeviceTestData(DeviceIndex::StagesBike_Assioma_PowerSensorDisabled)
	->expectDevice<stagesbike>()        
	->acceptDeviceName("ASSIOMA", DeviceNameComparison::StartsWithIgnoreCase)
	->configureSettingsWith(QZSettings::power_sensor_name, "DisabledX", "XDisabled")
	->excluding( stagesBikeExclusions);

```
The name and bluetooth device info configurations in another:

```
// Stages Bike (KICKR CORE)
RegisterNewDeviceTestData(DeviceIndex::StagesBike_KICKRCORE)
	->expectDevice<stagesbike>()        
	->acceptDeviceName("KICKR CORE", DeviceNameComparison::StartsWithIgnoreCase)
	->excluding(stagesBikeExclusions)
	->configureSettingsWith(
		[](const DeviceDiscoveryInfo& info,  bool enable, std::vector<DeviceDiscoveryInfo>& configurations)->void
		{
			// The condition, if the name is acceptable, is:
			// !deviceHasService(b, QBluetoothUuid((quint16)0x1826)) && deviceHasService(b, QBluetoothUuid((quint16)0x1818)))

			if(enable) {
				DeviceDiscoveryInfo result = info;
				result.addBluetoothService(QBluetoothUuid((quint16)0x1818));
				result.removeBluetoothService(QBluetoothUuid((quint16)0x1826));
				configurations.push_back(result);
			} else {
				DeviceDiscoveryInfo hasNeither = info;
				hasNeither.removeBluetoothService(QBluetoothUuid((quint16)0x1818));
				hasNeither.removeBluetoothService(QBluetoothUuid((quint16)0x1826));

				DeviceDiscoveryInfo hasInvalid = info;
				hasInvalid.addBluetoothService(QBluetoothUuid((quint16)0x1826));
				DeviceDiscoveryInfo hasBoth = hasInvalid;
				hasBoth.addBluetoothService(QBluetoothUuid((quint16)0x1818));
				hasBoth.addBluetoothService(QBluetoothUuid((quint16)0x1826));

				configurations.push_back(info); // has neither
				configurations.push_back(hasInvalid);
				configurations.push_back(hasBoth);
			}
		});

```

## Telling Google Test Where to Look

The BluetoothDeviceTestSuite configuration specifies that the test data will be obtained from the DeviceTestDataIndex class, so there's nothing more to do.



