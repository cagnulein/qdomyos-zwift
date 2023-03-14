#include "virtualtreadmilltestdata.h"
#include "virtualtreadmill.h"

VirtualTreadmillTestData::VirtualTreadmillTestData() : VirtualDeviceTestData("Virtual Treadmill")
{

}

virtualdevice *VirtualTreadmillTestData::createDevice(bluetoothdevice *device) {
    return new virtualtreadmill(device, false);
}
