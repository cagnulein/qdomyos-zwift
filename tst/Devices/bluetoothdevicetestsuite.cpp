#include "bluetoothdevicetestsuite.h"
#include "discoveryoptions.h"
#include "bluetooth.h"
#include "bluetoothsignalreceiver.h"
#include "devicetestdataindex.h"

const QString testUUID = QStringLiteral("b8f79bac-32e5-11ed-a261-0242ac120002");
QBluetoothUuid uuid{testUUID};


void BluetoothDeviceTestSuite::tryDetectDevice(bluetooth &bt,
                                                  const QBluetoothDeviceInfo &deviceInfo) const {
    try {
        // It is possible to use an EXPECT_NO_THROW here, but this
        // way is easier to place a breakpoint on the call to bt.deviceDiscovered.
        bt.homeformLoaded = true;

        if(this->testParam->UseNonBluetoothDiscovery())
            bt.nonBluetoothDeviceDiscovery();
        else
            bt.deviceDiscovered(deviceInfo);
    } catch (...) {
        FAIL() << "Failed to perform device detection.";
    }
}

std::string BluetoothDeviceTestSuite::getTypeName(bluetoothdevice *b) const {
    if(!b) return "nullptr";
    QString name = typeid(*b).name();

    int length = name.length();
    int sum = 0, i=0;
    while(i<length && name[i].isDigit() && sum!=length-i) {
        sum = sum * 10 + name[i].toLatin1()-(char)'0';
        i++;
    }

    if(sum==length-i)
        return name.right(length-i).toStdString();

    return name.toStdString();
}


std::string BluetoothDeviceTestSuite::formatString(std::string format, bluetoothdevice *b) const {

    QString qs = QString(format.c_str());
    QString typeName = this->getTypeName(b).c_str();
    return qs.replace("{typeName}", typeName).toStdString();
}

void BluetoothDeviceTestSuite::testDeviceDetection(const BluetoothDeviceTestData * testData, bluetooth &bt,
                                                      const QBluetoothDeviceInfo &deviceInfo,
                                                  bool expectMatch,
                                                  bool restart,
                                                  const QString& failMessage) const {
    this->testDeviceDetection(testData, bt, deviceInfo, expectMatch, restart, failMessage.toStdString());
}



void BluetoothDeviceTestSuite::testDeviceDetection(const BluetoothDeviceTestData * testData, bluetooth &bt,
                                                  const QBluetoothDeviceInfo &deviceInfo,
                                                  bool expectMatch,
                                                  bool restart,
                                                  const std::string& failMessage) const {

    BluetoothSignalReceiver signalReceiver(bt);

    this->tryDetectDevice(bt, deviceInfo);

    bluetoothdevice * device = bt.device();

    if(expectMatch) {
        std::string formattedFailMessage = this->formatString(failMessage, device);
        EXPECT_TRUE(testData->IsExpectedDevice(device)) << formattedFailMessage;

        EXPECT_EQ(device, signalReceiver.get_device()) << "Connection signal not received";
    } else {
        EXPECT_FALSE(testData->IsExpectedDevice(device)) << this->formatString(failMessage, device);
    }

    if(restart) {
        bt.restart();
        EXPECT_EQ(nullptr, signalReceiver.get_device()) << "Disconnection signal not received";
    }
}

void BluetoothDeviceTestSuite::SetUp() {
    auto testParam = this->GetParam();

    this->testParam = DeviceTestDataIndex::GetTestData(testParam);

    if(!this->testParam)
        GTEST_FAIL() << "Failed to get test data for: " << testParam.toStdString();

    QString skipMessage = nullptr;

    qDebug() << "Got test data";

    if(!this->testParam->IsEnabled())
    {
        QString reason = this->testParam->DisabledReason();
        skipMessage = QString("Device %1 is disabled for testing.").arg(testParam);

        if(reason!=nullptr && reason!="")
            skipMessage = QString("%1 Reason: %2").arg(skipMessage).arg(reason);
    }
    else if(this->testParam->IsSkipped())
    {
        QString reason = this->testParam->SkippedReason();
        skipMessage = QString("Device %1 is skipped for testing.").arg(testParam);

        if(reason!=nullptr && reason!="")
            skipMessage = QString("%1 Reason: %2").arg(skipMessage).arg(reason);
    }

    if(skipMessage!=nullptr)
        GTEST_SKIP() << skipMessage.toStdString();

    qDebug() << "Not disabled or skipped";

    this->testParam->InitializeDevice();

    qDebug() << "Test Data Device Initialization complete";

    this->defaultDiscoveryOptions = discoveryoptions{};
    this->defaultDiscoveryOptions.startDiscovery = false;
    this->defaultDiscoveryOptions.logs = true;

    this->names = this->testParam->NamePatternGroup()->DeviceNames();

    qDebug() << "Got device names";

    EXPECT_GT(this->names.size(), 0) << "No bluetooth names configured for test";

    this->testSettings.activate();

    qDebug() << "Test settings activated";
}

void BluetoothDeviceTestSuite::test_deviceDetection(const bool validNames, const bool enablingConfigs) {
    auto testData = this->testParam;

    bluetooth bt(this->defaultDiscoveryOptions);

    auto names = validNames ? testData->NamePatternGroup()->DeviceNames() : testData->NamePatternGroup()->InvalidDeviceNames();

    for(QString deviceName : names)
    {
        QBluetoothDeviceInfo btdi(uuid, deviceName,0);
        DeviceDiscoveryInfo info(btdi);

        auto configurations = testData->ApplyConfigurations(info, enablingConfigs);

        for(size_t i=0; i<configurations.size(); i++){

            DeviceDiscoveryInfo discoveryInfo = configurations[i];
            this->testSettings.loadFrom(discoveryInfo);

            QString failMessage = nullptr;

            if(enablingConfigs && validNames)
                failMessage = QString("Failed to detect device for %1 using valid name: \"%2\" and enabling configuration: %3, got a {typeName} instead")
                                      .arg(testData->Name()).arg(deviceName).arg(i);
            else
                failMessage = QString("Created device for %1 using %2valid name: \"%3\" and %4abling configuration: %5, got a {typeName} instead of nullptr")
                                  .arg(testData->Name()).arg(validNames ? "":"in").arg(deviceName).arg(enablingConfigs ? "en":"dis").arg(i);

            this->testDeviceDetection(testData, bt, *discoveryInfo.DeviceInfo(), enablingConfigs && validNames, true, failMessage);

        }
    }
}

std::vector<DeviceDiscoveryInfo> BluetoothDeviceTestSuite::getConfigurations(const BluetoothDeviceTestData *testData, const QString& deviceName, bool enabled) const {
    QBluetoothDeviceInfo btdi(uuid, deviceName,0);
    DeviceDiscoveryInfo info(btdi);
    auto result = testData->ApplyConfigurations(info, enabled);

    if(enabled && result.empty()) {
        result.push_back(DeviceDiscoveryInfo(info));
    }

    return result;
}

void BluetoothDeviceTestSuite::test_deviceDetection_validNames_enabled() {
    this->test_deviceDetection(true, true);
}

void BluetoothDeviceTestSuite::test_deviceDetection_validNames_disabled() {
    this->test_deviceDetection(true, false);
}

void BluetoothDeviceTestSuite::test_deviceDetection_invalidNames_enabled()
{
    this->test_deviceDetection(false, true);
}

void BluetoothDeviceTestSuite::test_deviceDetection_exclusions() {
    auto testData = this->testParam;

    auto exclusionNames = testData->Exclusions();

    if(exclusionNames.size()==0)
        GTEST_SKIP() << "No exclusions defined for this device: " << testData->Name().toStdString();

    // Only take the first for each type of exclusion
    std::vector<const BluetoothDeviceTestData*> exclusions;
    std::unordered_set<int> exclusionTypeIds;
    for(auto exclusionName : exclusionNames)
    {
        auto deviceTestData = DeviceTestDataIndex::GetTestData(exclusionName);
        if(exclusionTypeIds.count(deviceTestData->ExpectedDeviceType()))
            continue;
        exclusions.push_back(deviceTestData);
        exclusionTypeIds.insert(deviceTestData->ExpectedDeviceType());
    }

    bluetooth bt(this->defaultDiscoveryOptions);

    // Test that it doesn't detect this device if its higher priority "namesakes" are already detected.
    for(auto exclusion : exclusions) {

        // For each name that would otherwise result in the device being detected
        for(QString deviceName : this->names)
        {
            // Get the enabling configurations
            auto enablingConfigurations = this->getConfigurations(testData, deviceName, true);

            for(size_t i=0; i<enablingConfigurations.size(); i++) {
                DeviceDiscoveryInfo enablingDiscoveryInfo = enablingConfigurations[i];

                // get an enabling configuration for the exclusion
                DeviceDiscoveryInfo exclusionDiscoveryInfo(true);

                QString exclusionDeviceName = exclusion->NamePatternGroup()->DeviceNames()[0];

                auto configurationsEnablingTheExclusion = this->getConfigurations(exclusion, exclusionDeviceName, true);

                if(configurationsEnablingTheExclusion.size()>0) {
                    // get a configuration that should enable the detection of the excluding device
                    exclusionDiscoveryInfo = configurationsEnablingTheExclusion[0];
                }
                else
                    GTEST_FAIL() << "There are no enabling configurations for exclusion device \"" << exclusion->Name().toStdString() << "\" using BT name \"" << exclusionDeviceName.toStdString() << "\"";

                this->testSettings.loadFrom(exclusionDiscoveryInfo);

                QString failMessage = QString("Failed to create exclusion device: %1, got a {typeName} instead")
                                          .arg(exclusion->Name());
                // try to create the excluding device
                this->testDeviceDetection(exclusion, bt, *exclusionDiscoveryInfo.DeviceInfo(), true, false, failMessage);

                // now configure to have the bluetooth object try, but fail to detect the target device
                this->testSettings.loadFrom(enablingDiscoveryInfo);

                failMessage = QString("Detected the %1 from %2 with valid config %3 in spite of exclusion by %4")
                                  .arg(testData->Name()).arg(deviceName).arg(i).arg(exclusion->Name());
                this->testDeviceDetection(testData, bt, *enablingDiscoveryInfo.DeviceInfo(), false, true, failMessage);
            }

        }
    }
}
