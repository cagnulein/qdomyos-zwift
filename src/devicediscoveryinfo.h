#ifndef DEVICEDISCOVERYINFO_H
#define DEVICEDISCOVERYINFO_H

#include <QString>
#include <vector>
#include "discovereddevice.h"

/**
 * @brief Settings used during the device discovery process.
 */
class devicediscoveryinfo {
private:
    /**
     * @brief Devices already discovered. May be used to exclude certain devices of
     * ambiguous identity.
     */
    std::vector<deviceType> exclusions;
public :
    QString filterDevice = QLatin1String("");

    bool fake_bike = false;
    bool fakedevice_elliptical = false;
    bool fakedevice_treadmill = false;

    bool proformWifiBike = false;
    QString proformtdf4ip = nullptr;

    QString proformtreadmillip = nullptr;
    QString nordictrack_2950_ip = nullptr;
    QString tdf_10_ip = nullptr;

    bool csc_as_bike = false;
    QString cscName = nullptr;

    bool power_as_bike = false;
    QString powerSensorName = nullptr;
    bool power_as_treadmill = false;

    bool hammerRacerS = false;
    bool pafers_treadmill= false;

    bool flywheel_life_fitness_ic8 = false;
    bool toorx_bike = false;

    bool toorx_ftms = false;
    bool snode_bike = false;
    bool fitplus_bike = false;

    /**
     * @brief Exclude a device type. Can be used to indicate a device of this type is already discovered.
     * @param type the device type to exclude.
     */
    void exclude(deviceType type);

    /**
     * @brief Determine if the device type is excluded.
     * @param type The type to test for exclusion.
     */
    bool excludes(deviceType type) const;

};

#endif // DEVICEDISCOVERYINFO_H
