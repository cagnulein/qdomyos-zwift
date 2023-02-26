#include "wahookickrsnapbiketestdata.h" 
#include "wahookickrsnapbike.h"

WahooKickrSnapBikeTestData::WahooKickrSnapBikeTestData() : BikeTestData("Wahoo Kickr Snap Bike") {
    this->addDeviceName("KICKR SNAP", comparison::StartsWithIgnoreCase);
    this->addDeviceName("KICKR BIKE", comparison::StartsWithIgnoreCase);
    this->addDeviceName("KICKR ROLLR", comparison::StartsWithIgnoreCase);
    this->addDeviceName("WAHOO KICKR", comparison::StartsWithIgnoreCase);
}

deviceType WahooKickrSnapBikeTestData::get_expectedDeviceType() const { return deviceType::WahooKickrSnapBike; }

bool WahooKickrSnapBikeTestData::get_isExpectedDevice(bluetoothdevice *detectedDevice) const {
    return dynamic_cast<wahookickrsnapbike*>(detectedDevice)!=nullptr;
}
