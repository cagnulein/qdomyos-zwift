#ifndef DEVICEDISCOVERYINFO_H
#define DEVICEDISCOVERYINFO_H

#include <QSettings>
#include <QString>
#include <vector>

/**
 * @brief Settings used during the device discovery process.
 */
class DeviceDiscoveryInfo {
public :
    QString filterDevice = QLatin1String("");

    bool fake_bike = false;
    bool fakedevice_elliptical = false;
    bool fakedevice_treadmill = false;
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
    bool toorx_ftms_treadmill = false;
    bool horizon_treadmill_force_ftms = false;
    bool snode_bike = false;
    bool fitplus_bike = false;

    bool technogym_myrun_treadmill_experimental = false;

    QString computrainer_serial_port = nullptr;
    bool ss2k_peloton = false;

    QString ftmsAccessoryName = nullptr;
    QString ftms_bike = nullptr;

    bool pafers_treadmill_bh_iboxster_plus = false;

    bool iconcept_elliptical = false;

    bool sole_treadmill_inclination = false;



    /**
     * @brief Constructor.
     * @param loadDefaults Indicates if the default values should be loaded.
     */
    explicit DeviceDiscoveryInfo(bool loadDefaults=true);

    /**
     * @brief Configures the QSettings object.
     * @param settings
     * @param clear Specifies whether to clear the settings object.
     */
    void setValues(QSettings& settings, bool clear=true) const;

    /**
     * @brief Loads this object with values from the QSettings object.
     */
    void getValues(QSettings& settings);

    /**
     * @brief Temporarily creates an empty QSettings object and loads this object's members from it.
     */
    void loadDefaultValues();

};

#endif // DEVICEDISCOVERYINFO_H
