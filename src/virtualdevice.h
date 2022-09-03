#ifndef VIRTUALDEVICE_H
#define VIRTUALDEVICE_H

#include <QString>

class virtualdevice {
private:
    virtualdevice() {}
public:
    /**
     * @brief The base default name advertised via Bluetooth. This
     * can be used to check the start of a device name.
     */
    const static QString VirtualDeviceBaseName;

    /**
     * @brief The QSettings key the bluetooth name suffix is stored under.
     */
    const static QString VirtualDeviceNameSuffixSettingsKey;

    /**
     * @brief he QSettings key to get the setting indicating if the bluetooth name suffix is enabled.
     */
    const static QString VirtualDeviceNameSuffixEnabledSettingsKey;

    /**
     * @brief Gets the full bluetooth name to be advertised, including (if enabled) the suffix from the settings.
     */
    static QString get_VirtualDeviceName();

};



#endif // VIRTUALDEVICE_H
