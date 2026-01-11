#include "bluetoothdevicetestdata.h"
#include "devicetestdataindex.h"

QString BluetoothDeviceTestData::Name() const { return this->name; }

bool BluetoothDeviceTestData::UseNonBluetoothDiscovery() const { return this->usingNonBluetoothDiscovery; }

DeviceTypeId BluetoothDeviceTestData::ExpectedDeviceType() const {
    if(this->expectedDeviceType<0)
        throw std::domain_error("Expected device not set");
    return this->expectedDeviceType;
}

bool BluetoothDeviceTestData::IsEnabled() const { return this->enabled; }

const QString BluetoothDeviceTestData::DisabledReason() const  { return this->disabledReason; }

const QString BluetoothDeviceTestData::SkippedReason() const  { return this->skippedReason; }

bool BluetoothDeviceTestData::IsExpectedDevice(bluetoothdevice *device) const { return this->isExpectedDevice(device); }

const QStringList BluetoothDeviceTestData::Exclusions() const {
    auto testData = DeviceTestDataIndex::WhereExpects(this->exclusions);

    QString missing = "";

    // first check that all the test data is there
    for(const auto key : this->exclusions) {
        if(!testData.contains(key))
            missing += QString("%1 ").arg(key);
    }

    if(missing.length()>0) {
        QString message = QString("Failed to find test data for excluded ids: ")+missing;
        throw std::domain_error(message.toStdString());
    }

    QStringList result;
    for(const auto deviceTestData : testData.values())
        result.append(deviceTestData->Name());

    return result;
}

const DeviceNamePatternGroup *BluetoothDeviceTestData::NamePatternGroup() const { return this->deviceNamePatternGroup; }

BluetoothDeviceTestData::~BluetoothDeviceTestData(){
    delete this->deviceNamePatternGroup;
}

BluetoothDeviceTestData::BluetoothDeviceTestData() {}


std::vector<DeviceDiscoveryInfo> BluetoothDeviceTestData::ApplyConfigurations(const DeviceDiscoveryInfo &info, bool enable) const {
    std::vector<DeviceDiscoveryInfo> result;

    auto name = info.DeviceName();

    if(this->applicatorSingle)
    {
        DeviceDiscoveryInfo newInfo(info);
        this->applicatorSingle(newInfo, enable);
        result.push_back(newInfo);
    }

    if(this->applicatorMultiple) {
        auto count = result.size();
        this->applicatorMultiple(info, enable, result);

        //if(result.size()<=count)
        //    throw std::domain_error("No configurations added. Please check the lambda is accepting the vector by address.");
    }

    for(auto config : result) {
        if(config.DeviceName()!=name)
            throw std::domain_error("Settings applicator changed the BT name.");
    }

    return result;
}

void BluetoothDeviceTestData::InitializeDevice() const
{
    if(this->initializer!=nullptr)
        this->initializer();
}


