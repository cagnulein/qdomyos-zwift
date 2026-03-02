#ifndef DISCOVERYOPTIONS_H
#define DISCOVERYOPTIONS_H
#include <QString>

class discoveryoptions {
public:
    /**
     * @brief Specifies if logs should be written.
     */
    bool logs = false;

    /**
     * @brief Specify a specific device by name.
     */
    QString deviceName = QString();

    /**
     * @brief Used to suppress resistance.
     */
    bool noWriteResistance = false;

    /**
     * @brief Used to suppress the device's heart service.
     */
    bool noHeartService = false;

    /**
     * @brief The device polling interval in milliseconds.
     */
    uint32_t pollDeviceTime = 200;


    bool noConsole = false;


    bool testResistance = false;

    /**
     * @brief Specifies a value that will be added to the resistance requests going to the bike, after the gain has been applied.
     */
    int8_t bikeResistanceOffset = 4;

    /**
     * @brief The resistance requests going to the bike should be multiplied by this, before adding the resistance offset.
     */
    double bikeResistanceGain = 1.0;

    /**
     * @brief Use to suppress starting discovery in the constructor, e.g. for testing.
     */
    bool startDiscovery = true;
};


#endif // DISCOVERYOPTIONS_H
