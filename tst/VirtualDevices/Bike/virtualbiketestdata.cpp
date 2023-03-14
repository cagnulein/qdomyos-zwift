#include "virtualbiketestdata.h"
#include "virtualbike.h"

VirtualBikeTestData::VirtualBikeTestData() : VirtualDeviceTestData("Virtual Bike")
{

}

QZLockscreenFunctions::configurationType VirtualBikeTestData::get_expectedLockscreenConfigurationType() const  {
    return QZLockscreenFunctions::configurationType::BIKE;
}

virtualdevice *VirtualBikeTestData::createDevice(bluetoothdevice * device) {
    return new virtualbike(device);
}
