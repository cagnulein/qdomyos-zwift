#ifndef VIRTUALDEVICE_H
#define VIRTUALDEVICE_H

#include <QString>

class virtualdevice {
private:
    virtualdevice() {}
public:
    /**
      * @brief Legacy Bluetooth device name. This is the default when the alternative name is not in use,
      * and is still used for a consistent customer experience.
      */
     const static QString DomyosBridge;

    /**
     * @brief The base default name advertised via Bluetooth.
     */
    const static QString VirtualDeviceBaseName;

    /**
     * @brief The QSettings key the alternative bluetooth name suffix is stored under.
     */
    const static QString VirtualDeviceAlternativeNameSuffixSettingsKey;

    /**
     * @brief The QSettings key to get the setting indicating if the alternative bluetooth name is enabled.
     */
    const static QString VirtualDeviceAlternativeNameEnabledSettingsKey;

    /**
     * @brief Gets the full bluetooth name to be advertised.
     */
    static QString get_VirtualDeviceName();

};



#endif // VIRTUALDEVICE_H
