#include "bluetoothdevicetestdatafactory.h"

#include <memory>
#include "bluetooth.h"
#include "bluetoothdevicetestsuite.h"


BluetoothDeviceTestDataVector BluetoothDeviceTestDataFactory::allTestData;

BluetoothDeviceTestDataFactory::BluetoothDeviceTestDataFactory()
{

}

void BluetoothDeviceTestDataFactory::registerTestData() {
    registerTestData(new M3IBikeTestData());
    registerTestData(new FakeBikeTestData());
    registerTestData(new FakeEllipticalTestData());
    registerTestData(new FakeTreadmillTestData());
    registerTestData(new ProFormWiFiBikeTestData());
    registerTestData(new ProFormWiFiTreadmillTestData());
    registerTestData(new NordicTrackIFitADBTreadmillTestData());
    registerTestData(new CSCBikeTestData());
    registerTestData(new StagesBike1TestData());
    registerTestData(new StagesBike2TestData());
    registerTestData(new StrydeRunPowerSensorTestData());
    registerTestData(new DomyosRowerTestData());
    registerTestData(new DomyosBikeTestData());
    registerTestData(new DomyosEllipticalTestData());
    registerTestData(new NautilusEllipticalTestData());
    registerTestData(new NautilusBikeTestData());
    registerTestData(new ProFormEllipticalTestData());
    registerTestData(new NordicTrackEllipticalTestData());
    registerTestData(new ProFormEllipticalTrainerTestData());
    registerTestData(new ProFormRowerTestData());
    registerTestData(new BHFitnessEllipticalTestData());
    registerTestData(new SoleEllipticalTestData());
    registerTestData(new DomyosTreadmillTestData());
    registerTestData(new KingsmithR2TreadmillTestData());
    registerTestData(new KingsmithR1ProTreadmillTestData());
    registerTestData(new Shuaa5TreadmillTestData());
    registerTestData(new TrueTreadmillTestData());
    registerTestData(new SoleF80TreadmillTestData());
    registerTestData(new HorizonTreadmillTestData());
    registerTestData(new TechnoGymMyRunTreadmillTestData());
    registerTestData(new TechnogymMyRunTreadmillRFCommTestData());
    registerTestData(new TacxNeo2TestData());
    registerTestData(new NPECableBike1TestData());
    registerTestData(new NPECableBike2TestData());
    registerTestData(new FTMSBike1TestData());
    registerTestData(new FTMSBike2TestData());
    registerTestData(new WahooKickrSnapBikeTestData());
    registerTestData(new HorizonGR7BikeTestData());
    registerTestData(new SmartRowRowerTestData());
    registerTestData(new Concept2SkiErgTestData());
    registerTestData(new FTMSRowerTestData());
    registerTestData(new EchelonStrideTreadmillTestData());
    registerTestData(new OctaneTreadmillTestData());
    registerTestData(new EchelonRowerTestData());
    registerTestData(new EchelonConnectSportBikeTestData());
    registerTestData(new SchwinnIC4BikeTestData());
    registerTestData(new SportsTechBikeTestData());
    registerTestData(new SportsPlusBikeTestData());
    registerTestData(new YesoulBikeTestData());
    registerTestData(new ProFormBikeTestData());
    registerTestData(new ProFormTreadmillTestData());
    registerTestData(new ESLinkerTreadmillTestData());
    registerTestData(new PafersTreadmillTestData());
    registerTestData(new BowflexT216TreadmillTestData());
    registerTestData(new NautilusTreadmillTestData());
    registerTestData(new FlywheelBike1TestData());
    registerTestData(new FlywheelBike2TestData());
    registerTestData(new MCFBikeTestData());
    registerTestData(new ToorxTreadmillTestData());
    registerTestData(new iConceptBikeTestData());
    registerTestData(new SpiritTreadmillTestData());
    registerTestData(new ActivioTreadmillTestData());
    registerTestData(new TrxAppGateUSBTreadmillTestData());
    registerTestData(new TrxAppGateUSBBike1TestData());
    registerTestData(new TrxAppGateUSBBike2TestData());
    registerTestData(new UltrasportBikeTestData());
    registerTestData(new KeepBikeTestData());
    registerTestData(new SoleBikeTestData());
    registerTestData(new SkandikaWiryBikeTestData());
    registerTestData(new RenphoBike1TestData());
    registerTestData(new RenphoBike2TestData());
    registerTestData(new PafersBikeTestData());
    registerTestData(new SnodeBike1TestData());
    registerTestData(new SnodeBike2TestData());
    registerTestData(new FitPlusBikeFSTestData());
    registerTestData(new FitPlusBikeMRKTestData());
    registerTestData(new FitshowTreadmillBFTestData());
    registerTestData(new FitshowTreadmillFSTestData());
    registerTestData(new FitshowTreadmillSWTestData());
    registerTestData(new InspireBikeTestData());
    registerTestData(new ChronobikeTestData());
    registerTestData(new MepanelBikeTestData());
}
