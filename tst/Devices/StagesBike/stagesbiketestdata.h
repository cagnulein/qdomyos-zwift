#pragma once

#include "Devices/Bike/biketestdata.h"
#include "devices/stagesbike/stagesbike.h"

class StagesBikeTestData : public BikeTestData {
protected:
    bike* doCreateInstance(const BikeOptions& options) override {
        return new stagesbike(options.noResistance, options.noHeartService, options.noVirtualDevice);
    }
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
        this->addDeviceName("TACX SATORI", comparison::StartsWithIgnoreCase);
        this->addDeviceName("QD", comparison::IgnoreCase);
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
