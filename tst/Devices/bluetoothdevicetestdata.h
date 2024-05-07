#pragma once

#include "devices/bluetoothdevice.h"
#include <QStringList>
#include <vector>
#include <memory>
#include "devicediscoveryinfo.h"

/**
 * @brief Device type supported by the application.
 * NOTE: Not ultimately used for anything currently. This was originally made as part of a plan to split device discovery
 * into two sections, detection, which returns one of these enum values, and creation, which creates the bluetoothdevice object.
 * The test data classes still have a function that returns one of these. This has been left here for reuse in a possible future refactoring
 * of device detection.
 */
enum deviceType {
    None,
    ApexBike,
    BkoolBike,
    M3IBike,
    FakeBike,
    FakeElliptical,
    FakeTreadmill,
    ProformWifiBike,
    ProformWifiTreadmill,
    NordicTrackIFitADBTreadmill,
    NordicTrackIFitADBBike,
    CSCBike,
    PowerBike_Stages,
    PowerTreadmill_StrydrunPowerSensor,
    DomyosRower,
    DomyosBike,
    DomyosElliptical,
    NautilusElliptical,
    NautilusBike,
    ProformElliptical,
    NordicTrackElliptical,
    ProformEllipticalTrainer,
    ProformRower,
    BHFitnessElliptical,
    SoleElliptical,
    DomyosTreadmill,
    KingsmithR2Treadmill,
    KingsmithR1ProTreadmill,
    Shuaa5Treadmill,
    TrueTreadmill,
    SoleF80Treadmill,
    HorizonTreadmill,
    TechnoGymMyRunTreadmill,
    TechnoGymMyRunTreadmillRfComm,
    TacxNeo2,
    NPECableBike,
    FTMSBike,
    WahooKickrSnapBike,
    HorizonGr7Bike,
    StagesBike,
    SmartRowRower,
    Concept2SkiErg,
    FTMSRower,
    EchelonStride,
    OctaneElliptical,
    OctaneTreadmill,
    EchelonRower,
    EchelonConnectSport,
    SchwinnIC4Bike,
    Schwinn170Bike,
    SportsTechBike,
    SportsPlusBike,
    YesoulBike,
    ProformBike,
    ProformTreadmill,
    ESLinkerTreadmill,
    PafersTreadmill,
    BowflexT216Treadmill,
    NautilusTreadmill,
    FlywheelBike,
    MCFBike,
    ToorxTreadmill,
    IConceptBike,
    IConceptElliptical,
    SpiritTreadmill,
    ActivioTreadmill,
    TrxAppGateUSBTreadmill,
    TrxAppGateUSBBike,
    UltraSportBike,
    KeepBike,
    SoleBike,
    SkandikawiriBike,
    RenphoBike,
    PafersBike,
    SnodeBike,
    FitPlusBike,
    FitshowTreadmill,
    InspireBike,
    ChronoBike,
    MepanelBike,
    LifeFitnessTreadmill,
    YpooElliptical,
    ZiproTreadmill,
    CompuTrainerBike,
    FocusTreadmill,
    TrxAppGateUSBElliptical,
};

class BluetoothDeviceTestData;
typedef std::shared_ptr<BluetoothDeviceTestData> BluetoothDeviceTestData_ptr;
typedef std::vector<BluetoothDeviceTestData_ptr> BluetoothDeviceTestDataVector;

class BluetoothDeviceTestData  {
    std::vector<std::shared_ptr<BluetoothDeviceTestData>> exclusions;
    QStringList deviceNames;
    QStringList invalidDeviceNames;
    bool exclusionsConfigured = false;
    bool configuringExclusions = false;
    std::string testName;
protected:

    /**
     * @brief Indicates how bluetooth device names should be compared.
     */
    enum comparison : int {
        Exact = 0,
        IgnoreCase = 1,
        StartsWith = 2,
        StartsWithIgnoreCase = IgnoreCase+StartsWith
    };

    /**
     * @brief Call exclude(...) to populate the exclusions vector. This vector is populated on demand
     * to avoid circularities in the constructors.
     */
    virtual void configureExclusions();

    void exclude(BluetoothDeviceTestData* testData);

    /**
     * @brief Add a device name that should be identified as this device.
     * If the comparison specified contains "starts with", then samples that start with the
     * specified device name will be added. If a case sensitive comparison is specified,
     * samples with invalid casing will be added to the invalid names list.
     * @param deviceName
     * @param length The expected length. Use 0 for unrestricted.
     */
    void addDeviceName(const QString& deviceName, comparison cmp, uint8_t length=0);

    /**
     * @brief Add a device name as a prefix and suffix that should be identified as this device.
     * If a case sensitive comparison is specified,
     * samples with invalid casing will be added to the invalid names list.
     * @param deviceName
     */
    void addDeviceName(const QString& deviceNameStartsWith, const QString& deviceNameEndsWith, comparison cmp);

    /**
     * @brief Add a device name that should NOT be identified as this device.
     * @param deviceName
     */
    void addInvalidDeviceName(const QString& deviceName, comparison cmp);

    /**
     * @brief Add the specified device names with different casings.
     * Used to provide device names to test case sensitivity or insensitivity.
     * @param names
     * @param target
     */
    static void addDifferentCasings(const QStringList &names, QStringList &target);


    /**
     * @brief Configure multiple devicediscoveryinfo objects to either enable or disable the device in multiple ways.
     * @param info
     * @param enable
     * @param configurations The variations of the provided object to test.
     */
    virtual void configureSettings(const DeviceDiscoveryInfo& info, bool enable, std::vector<DeviceDiscoveryInfo>& configurations) const;


    /**
     * @brief Configure multiple QBluetoothDeviceInfo objects to either enable or disable the device in multiple ways.
     * @info An initial object configured aith a name and UUID for copying. This is expceted to have no service UUIDs.
     * @param enable Indicates if the request is for enabling (true) or disabling (false) configurations on the bluetooth device info.
     * @param bluetoothDeviceInfos The objects to test.
     */
    virtual void configureBluetoothDeviceInfos(const QBluetoothDeviceInfo& info,  bool enable, std::vector<QBluetoothDeviceInfo>& bluetoothDeviceInfos) const {}

    /**
     * @brief Configure the devicediscoveryinfo object to either enable or disable the device.
     * Used for where there is only 1 scenario.
     * @param info
     * @param enable
     */
    virtual bool configureSettings(DeviceDiscoveryInfo& info, bool enable) const;

    /**
     * @brief Constructor
     * @param testName A user-friendly name for the tested device, e.g. "SupaDupa Treadmill"
     */
    BluetoothDeviceTestData(std::string testName);
public:

    virtual ~BluetoothDeviceTestData();

    /**
     * @brief Gets the test name.
     * @return
     */
    virtual std::string get_testName() const;

    /**
     * @brief Indicates if the test data is for an abstract class.
     * @return
     */
    virtual bool get_isAbstract() const;

    /**
     * @brief A list of bluetooth device names that should be recognised as this device.
     */
    virtual QStringList get_deviceNames() const;

    /**
     * @brief A list of bluetooth device names that should NOT be recognised as this device.
     * By default this is generated by modifying the result of get_deviceNames().
     */
    virtual QStringList get_failingDeviceNames() const;

    /**
     * @brief Gets a list of BluetoothDeviceTestData objects for devices whose
     * prior detection should prevent the detection of this device when an expected device name is found.
     * @return
     */
    virtual std::vector<std::shared_ptr<BluetoothDeviceTestData>> get_exclusions();

    /**
     * @brief Gets combinations of configurations beginning with the specified object.
     * @param info
     */
    virtual std::vector<DeviceDiscoveryInfo> get_configurations(const DeviceDiscoveryInfo& info, bool enable);

    /**
     * @brief Gets the expected device type enumeration value to be detected for this device.
     */
    virtual deviceType get_expectedDeviceType() const =0;

    /**
     * @brief Determines if the specified bluetoothdevice* object is of the expected type for this device.
     * @param detectedDevice
     * @return
     */
    virtual bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const =0;

    /**
     * @brief Specifies a test IP address for wifi devices.
     */
    virtual QString get_testIP() const;

    /**
     * @brief Gets a vector of QBluetoothDeviceInfo objects for the specified name and UUID. Can be used to
     * generate valid and invalid objects where device identification relies on more than just the name.
     * @param uuid
     * @param name
     * @param valid
     */
    std::vector<QBluetoothDeviceInfo> get_bluetoothDeviceInfo(const QBluetoothUuid& uuid, const QString& name, bool valid=true);


};
