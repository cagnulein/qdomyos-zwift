#include "tacxneo2testdata.h" 
#include "tacxneo2.h"

TacxNeo2TestData::TacxNeo2TestData() : BikeTestData("Tacx Neo 2 Bike") {
    this->addDeviceName("TACX NEO", comparison::StartsWithIgnoreCase);
    this->addDeviceName("TACX FLOW", comparison::StartsWithIgnoreCase);
    this->addDeviceName("TACX SMART BIKE", comparison::StartsWithIgnoreCase);
    this->addDeviceName("THINK X", comparison::StartsWithIgnoreCase);
}

deviceType TacxNeo2TestData::get_expectedDeviceType() const { return deviceType::TacxNeo2; }

bool TacxNeo2TestData::get_isExpectedDevice(bluetoothdevice *detectedDevice) const {
    return dynamic_cast<tacxneo2*>(detectedDevice)!=nullptr;
}
