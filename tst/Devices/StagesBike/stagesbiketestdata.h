#pragma once

#include "Devices/Bike/biketestdata.h"
#include "Devices/FTMSBike/ftmsbiketestdata.h"

#include "devices/stagesbike/stagesbike.h"

class StagesBikeTestData : public BikeTestData {
protected:
    StagesBikeTestData(std::string testName): BikeTestData(testName) {
    }

    void configureExclusions() override;
public:

    deviceType get_expectedDeviceType() const override { return deviceType::StagesBike; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<stagesbike*>(detectedDevice)!=nullptr;
    }
};

class StagesBike1TestData : public StagesBikeTestData {

public:
    StagesBike1TestData() : StagesBikeTestData("Stages Bike") {
        this->addDeviceName("STAGES ", comparison::StartsWithIgnoreCase);
    }

};


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

class StagesBike3TestData : public StagesBikeTestData {

public:
    StagesBike3TestData() : StagesBikeTestData("Stages Bike (KICKR CORE)") {
        this->testInvalidBluetoothDeviceInfo = true;

        this->addDeviceName("KICKR CORE", comparison::StartsWithIgnoreCase);
    }

    QBluetoothDeviceInfo get_bluetoothDeviceInfo(const QBluetoothUuid& uuid, const QString& name, bool valid=true) override
    {
        QBluetoothDeviceInfo result = BluetoothDeviceTestData::get_bluetoothDeviceInfo(uuid, name, true);

        // The condition, if the name is acceptable, is:
        // !deviceHasService(b, QBluetoothUuid((quint16)0x1826)) && deviceHasService(b, QBluetoothUuid((quint16)0x1818)))

        if(valid) {
            result.setServiceUuids(QVector<QBluetoothUuid>({QBluetoothUuid((quint16)0x1818)}));
            return result;
        }

        result.setServiceUuids(QVector<QBluetoothUuid>({QBluetoothUuid((quint16)0x1826)}));

        // this doesn't check the invalid case where it has both services. Framework does not currently support multiple QBluetoothDeviceInfo scenarios

        return result;
    }

};
