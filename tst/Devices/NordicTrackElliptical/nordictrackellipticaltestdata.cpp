#include "nordictrackellipticaltestdata.h" 
#include "nordictrackelliptical.h"

NordicTrackEllipticalTestData::NordicTrackEllipticalTestData() : EllipticalTestData("Nordictrack Elliptical") {
    this->addDeviceName("I_EL", comparison::StartsWithIgnoreCase);
}

deviceType NordicTrackEllipticalTestData::get_expectedDeviceType() const { return deviceType::NordicTrackElliptical; }

bool NordicTrackEllipticalTestData::get_isExpectedDevice(bluetoothdevice *detectedDevice) const {
    return dynamic_cast<nordictrackelliptical*>(detectedDevice)!=nullptr;
}
