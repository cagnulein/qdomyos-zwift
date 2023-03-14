#include "virtualrowertestdata.h"
#include "virtualrower.h"

VirtualRowerTestData::VirtualRowerTestData(): VirtualDeviceTestData("Virtual Rower")
{

}

virtualdevice *VirtualRowerTestData::createDevice(bluetoothdevice *device) {
    return new virtualrower(device);
}
