#ifndef DISCOVEREDDEVICE_H
#define DISCOVEREDDEVICE_H

#include <QBluetoothDeviceInfo>

/**
 * @brief Device type supported by the application.
 */
enum deviceType {
    None,
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
    MepanelBike
};

/**
 * @brief The discovereddevice class encapsulates a bluetooth device identifier and
 * the device that it has been identified as.
 */
class discovereddevice {
  public:
    deviceType const type;
    const QBluetoothDeviceInfo deviceInfo;

    discovereddevice(deviceType type, const QBluetoothDeviceInfo& b): type(type), deviceInfo(b) {
    }

};

#endif // DISCOVEREDDEVICE_H
