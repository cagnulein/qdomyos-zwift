#include <memory>
#include "bluetooth.h"
#include "bluetoothdevicetestsuite.h"
#include "ActivioTreadmill/activiotreadmilltestdata.h"
#include "BHFitnessElliptical/bhfitnessellipticaltestdata.h"
#include "Bike/biketestdata.h"
#include "BowflexT216Treadmill/bowflext216treadmilltestdata.h"
#include "BowflexTreadmill/bowflextreadmilltestdata.h"
#include "Chronobike/chronobiketestdata.h"
#include "Concept2SkiErg/concept2skiergtestdata.h"
#include "CSCBike/cscbiketestdata.h"
#include "DomyosBike/domyosbiketestdata.h"
#include "DomyosElliptical/domyosellipticaltestdata.h"
#include "DomyosRower/domyosrowertestdata.h"
#include "DomyosTreadmill/domyostreadmilltestdata.h"
#include "EchelonConnectSportBike/echelonconnectsportbiketestdata.h"
#include "EchelonRower/echelonrowertestdata.h"
#include "EchelonStrideTreadmill/echelonstridetreadmilltestdata.h"
#include "EliteRizer/eliterizertestdata.h"
#include "EliteSterzoSmart/elitesterzosmarttestdata.h"
#include "Elliptical/ellipticaltestdata.h"
#include "ESLinkerTreadmill/eslinkertreadmilltestdata.h"
#include "FakeBike/fakebiketestdata.h"
#include "FakeElliptical/fakeellipticaltestdata.h"
#include "FitmetriaFanFit/fitmetriafanfittestdata.h"
#include "FitPlusBike/fitplusbiketestdata.h"
#include "FitshowTreadmill/fitshowtreadmilltestdata.h"
#include "FlywheelBike/flywheelbiketestdata.h"
#include "FTMSBike/ftmsbiketestdata.h"
#include "FTMSRower/ftmsrowertestdata.h"
#include "HeartRateBelt/heartratebelttestdata.h"
#include "HorizonGR7Bike/horizongr7biketestdata.h"
#include "HorizonTreadmill/horizontreadmilltestdata.h"
#include "iConceptBike/iconceptbiketestdata.h"
#include "InspireBike/inspirebiketestdata.h"
#include "KeepBike/keepbiketestdata.h"
#include "KingsmithR1ProTreadmill/kingsmithr1protreadmilltestdata.h"
#include "KingsmithR2Treadmill/kingsmithr2treadmilltestdata.h"
#include "M3IBike/m3ibiketestdata.h"
#include "MCFBike/mcfbiketestdata.h"
#include "NautilusBike/nautilusbiketestdata.h"
#include "NautilusElliptical/nautilusellipticaltestdata.h"
#include "NautilusTreadmill/nautilustreadmilltestdata.h"
#include "NordicTrackElliptical/nordictrackellipticaltestdata.h"
#include "NordicTrackIFitADBTreadmill/nordictrackifitadbtreadmilltestdata.h"
#include "NPECableBike/npecablebiketestdata.h"
#include "OctaneTreadmill/octanetreadmilltestdata.h"
#include "PafersBike/pafersbiketestdata.h"
#include "PafersTreadmill/paferstreadmilltestdata.h"
#include "ProFormBike/proformbiketestdata.h"
#include "ProFormElliptical/proformellipticaltestdata.h"
#include "ProFormEllipticalTrainer/proformellipticaltrainertestdata.h"
#include "ProFormRower/proformrowertestdata.h"
#include "ProFormTreadmill/proformtreadmilltestdata.h"
#include "ProFormWiFiBike/proformwifibiketestdata.h"
#include "ProFormWiFiTreadmill/proformwifitreadmilltestdata.h"
#include "RenphoBike/renphobiketestdata.h"
#include "Rower/rowertestdata.h"
#include "SchwinnIC4Bike/schwinnic4biketestdata.h"
#include "Shuaa5Treadmill/shuaa5treadmilltestdata.h"
#include "SkandikaWiryBike/skandikawirybiketestdata.h"
#include "SmartRowRower/smartrowrowertestdata.h"
#include "SmartSpin2k/smartspin2ktestdata.h"
#include "SnodeBike/snodebiketestdata.h"
#include "SoleBike/solebiketestdata.h"
#include "SoleElliptical/soleellipticaltestdata.h"
#include "SoleF80Treadmill/solef80treadmilltestdata.h"
#include "SpiritTreadmill/spirittreadmilltestdata.h"
#include "SportsPlusBike/sportsplusbiketestdata.h"
#include "SportsTechBike/sportstechbiketestdata.h"
#include "StagesBike/stagesbiketestdata.h"
#include "StrydeRunPowerSensor/stryderunpowersensortestdata.h"
#include "TacxNeo2/tacxneo2testdata.h"
#include "TechnoGymMyRunTreadmill/technogymmyruntreadmilltestdata.h"
#include "TechnogymMyRunTreadmillRFComm/technogymmyruntreadmillrfcommtestdata.h"
#include "ToorxTreadmill/toorxtreadmilltestdata.h"
#include "Treadmill/treadmilltestdata.h"
#include "TrueTreadmill/truetreadmilltestdata.h"
#include "TrxAppGateUSBBike/trxappgateusbbiketestdata.h"
#include "TrxAppGateUSBTreadmill/trxappgateusbtreadmilltestdata.h"
#include "UltrasportBike/ultrasportbiketestdata.h"
#include "WahooKickrSnapBike/wahookickrsnapbiketestdata.h"
#include "YesoulBike/yesoulbiketestdata.h"

void BluetoothDeviceTestSuite::test_deviceDetection(const BluetoothDeviceTestData& testData)
{
    const QString testUUID = QStringLiteral("b8f79bac-32e5-11ed-a261-0242ac120002");
    QBluetoothUuid uuid{testUUID};

    QStringList names = testData.get_deviceNames();

    EXPECT_GT(names.length(), 0);

    devicediscoveryinfo discoveryInfo = bluetooth::getDiscoveryInfo();
    for(QString deviceName : names)
    {

        QBluetoothDeviceInfo deviceInfo{uuid, deviceName, 0};

        auto discovered = bluetooth::discoverDevice(discoveryInfo, deviceInfo);

        EXPECT_EQ(discovered.type, testData.get_expectedDeviceType());
        //EXPECT_TRUE(testData.get_isExpectedDevice(detectedDevice));
    }


}

TEST_F(BluetoothDeviceTestSuite, ActivioTreadmillDetected) {
    ActivioTreadmillTestData testData;
    this->test_deviceDetection(testData);
}


TEST_F(BluetoothDeviceTestSuite, BHFitnessEllipticalDetected) {
    BHFitnessEllipticalTestData testData;
    this->test_deviceDetection(testData);
}


TEST_F(BluetoothDeviceTestSuite, BikeDetected) {
    BikeTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, BowflexT216TreadmillDetected) {
    BowflexT216TreadmillTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, BowflexTreadmillDetected) {
    BowflexTreadmillTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, ChronobikeDetected) {
    ChronobikeTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, Concept2SkiErgDetected) {
    Concept2SkiErgTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, CSCBikeDetected) {
    CSCBikeTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, DomyosBikeDetected) {
    DomyosBikeTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, DomyosEllipticalDetected) {
    DomyosEllipticalTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, DomyosRowerDetected) {
    DomyosRowerTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, DomyosTreadmillDetected) {
    DomyosTreadmillTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, EchelonConnectSportBikeDetected) {
    EchelonConnectSportBikeTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, EchelonRowerDetected) {
    EchelonRowerTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, EchelonStrideTreadmillDetected) {
    EchelonStrideTreadmillTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, EliteRizerDetected) {
    EliteRizerTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, EliteSterzoSmartDetected) {
    EliteSterzoSmartTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, EllipticalDetected) {
    EllipticalTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, ESLinkerTreadmillDetected) {
    ESLinkerTreadmillTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, FakeBikeDetected) {
    FakeBikeTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, FakeEllipticalDetected) {
    FakeEllipticalTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, FitmetriaFanFitDetected) {
    FitmetriaFanFitTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, FitPlusBikeDetected) {
    FitPlusBikeTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, FitshowTreadmillDetected) {
    FitshowTreadmillTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, FlywheelBikeDetected) {
    FlywheelBikeTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, FTMSBikeDetected) {
    FTMSBikeTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, FTMSRowerDetected) {
    FTMSRowerTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, HeartRateBeltDetected) {
    HeartRateBeltTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, HorizonGR7BikeDetected) {
    HorizonGR7BikeTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, HorizonTreadmillDetected) {
    HorizonTreadmillTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, iConceptBikeDetected) {
    iConceptBikeTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, InspireBikeDetected) {
    InspireBikeTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, KeepBikeDetected) {
    KeepBikeTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, KingsmithR1ProTreadmillDetected) {
    KingsmithR1ProTreadmillTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, KingsmithR2TreadmillDetected) {
    KingsmithR2TreadmillTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, M3IBikeDetected) {
    M3IBikeTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, MCFBikeDetected) {
    MCFBikeTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, NautilusBikeDetected) {
    NautilusBikeTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, NautilusEllipticalDetected) {
    NautilusEllipticalTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, NautilusTreadmillDetected) {
    NautilusTreadmillTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, NordicTrackEllipticalDetected) {
    NordicTrackEllipticalTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, NordicTrackIFitADBTreadmillDetected) {
    NordicTrackIFitADBTreadmillTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, NPECableBikeDetected) {
    NPECableBikeTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, OctaneTreadmillDetected) {
    OctaneTreadmillTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, PafersBikeDetected) {
    PafersBikeTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, PafersTreadmillDetected) {
    PafersTreadmillTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, ProFormBikeDetected) {
    ProFormBikeTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, ProFormEllipticalDetected) {
    ProFormEllipticalTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, ProFormEllipticalTrainerDetected) {
    ProFormEllipticalTrainerTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, ProFormRowerDetected) {
    ProFormRowerTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, ProFormTreadmillDetected) {
    ProFormTreadmillTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, ProFormWiFiBikeDetected) {
    ProFormWiFiBikeTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, ProFormWiFiTreadmillDetected) {
    ProFormWiFiTreadmillTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, RenphoBikeDetected) {
    RenphoBikeTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, RowerDetected) {
    RowerTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, SchwinnIC4BikeDetected) {
    SchwinnIC4BikeTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, Shuaa5TreadmillDetected) {
    Shuaa5TreadmillTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, SkandikaWiryBikeDetected) {
    SkandikaWiryBikeTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, SmartRowRowerDetected) {
    SmartRowRowerTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, SmartSpin2kDetected) {
    SmartSpin2kTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, SnodeBikeDetected) {
    SnodeBikeTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, SoleBikeDetected) {
    SoleBikeTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, SoleEllipticalDetected) {
    SoleEllipticalTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, SoleF80TreadmillDetected) {
    SoleF80TreadmillTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, SpiritTreadmillDetected) {
    SpiritTreadmillTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, SportsPlusBikeDetected) {
    SportsPlusBikeTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, SportsTechBikeDetected) {
    SportsTechBikeTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, StagesBikeDetected) {
    StagesBikeTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, StrydeRunPowerSensorDetected) {
    StrydeRunPowerSensorTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, TacxNeo2Detected) {
    TacxNeo2TestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, TechnoGymMyRunTreadmillDetected) {
    TechnoGymMyRunTreadmillTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, TechnogymMyRunTreadmillRFCommDetected) {
    TechnogymMyRunTreadmillRFCommTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, ToorxTreadmillDetected) {
    ToorxTreadmillTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, TreadmillDetected) {
    TreadmillTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, TrueTreadmillDetected) {
    TrueTreadmillTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, TrxAppGateUSBBikeDetected) {
    TrxAppGateUSBBikeTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, TrxAppGateUSBTreadmillDetected) {
    TrxAppGateUSBTreadmillTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, UltrasportBikeDetected) {
    UltrasportBikeTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, WahooKickrSnapBikeDetected) {
    WahooKickrSnapBikeTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, YesoulBikeDetected) {
    YesoulBikeTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, BikeDetected) {
    BikeTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, BowflexT216TreadmillDetected) {
    BowflexT216TreadmillTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, BowflexTreadmillDetected) {
    BowflexTreadmillTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, ChronobikeDetected) {
    ChronobikeTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, Concept2SkiErgDetected) {
    Concept2SkiErgTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, CSCBikeDetected) {
    CSCBikeTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, DomyosBikeDetected) {
    DomyosBikeTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, DomyosEllipticalDetected) {
    DomyosEllipticalTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, DomyosRowerDetected) {
    DomyosRowerTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, DomyosTreadmillDetected) {
    DomyosTreadmillTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, EchelonConnectSportBikeDetected) {
    EchelonConnectSportBikeTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, EchelonRowerDetected) {
    EchelonRowerTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, EchelonStrideTreadmillDetected) {
    EchelonStrideTreadmillTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, EliteRizerDetected) {
    EliteRizerTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, EliteSterzoSmartDetected) {
    EliteSterzoSmartTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, EllipticalDetected) {
    EllipticalTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, ESLinkerTreadmillDetected) {
    ESLinkerTreadmillTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, FakeBikeDetected) {
    FakeBikeTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, FakeEllipticalDetected) {
    FakeEllipticalTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, FitmetriaFanFitDetected) {
    FitmetriaFanFitTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, FitPlusBikeDetected) {
    FitPlusBikeTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, FitshowTreadmillDetected) {
    FitshowTreadmillTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, FlywheelBikeDetected) {
    FlywheelBikeTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, FTMSBikeDetected) {
    FTMSBikeTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, FTMSRowerDetected) {
    FTMSRowerTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, HeartRateBeltDetected) {
    HeartRateBeltTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, HorizonGR7BikeDetected) {
    HorizonGR7BikeTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, HorizonTreadmillDetected) {
    HorizonTreadmillTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, iConceptBikeDetected) {
    iConceptBikeTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, InspireBikeDetected) {
    InspireBikeTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, KeepBikeDetected) {
    KeepBikeTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, KingsmithR1ProTreadmillDetected) {
    KingsmithR1ProTreadmillTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, KingsmithR2TreadmillDetected) {
    KingsmithR2TreadmillTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, M3IBikeDetected) {
    M3IBikeTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, MCFBikeDetected) {
    MCFBikeTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, NautilusBikeDetected) {
    NautilusBikeTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, NautilusEllipticalDetected) {
    NautilusEllipticalTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, NautilusTreadmillDetected) {
    NautilusTreadmillTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, NordicTrackEllipticalDetected) {
    NordicTrackEllipticalTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, NordicTrackIFitADBTreadmillDetected) {
    NordicTrackIFitADBTreadmillTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, NPECableBikeDetected) {
    NPECableBikeTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, OctaneTreadmillDetected) {
    OctaneTreadmillTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, PafersBikeDetected) {
    PafersBikeTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, PafersTreadmillDetected) {
    PafersTreadmillTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, ProFormBikeDetected) {
    ProFormBikeTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, ProFormEllipticalDetected) {
    ProFormEllipticalTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, ProFormEllipticalTrainerDetected) {
    ProFormEllipticalTrainerTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, ProFormRowerDetected) {
    ProFormRowerTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, ProFormTreadmillDetected) {
    ProFormTreadmillTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, ProFormWiFiBikeDetected) {
    ProFormWiFiBikeTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, ProFormWiFiTreadmillDetected) {
    ProFormWiFiTreadmillTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, RenphoBikeDetected) {
    RenphoBikeTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, RowerDetected) {
    RowerTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, SchwinnIC4BikeDetected) {
    SchwinnIC4BikeTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, Shuaa5TreadmillDetected) {
    Shuaa5TreadmillTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, SkandikaWiryBikeDetected) {
    SkandikaWiryBikeTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, SmartRowRowerDetected) {
    SmartRowRowerTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, SmartSpin2kDetected) {
    SmartSpin2kTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, SnodeBikeDetected) {
    SnodeBikeTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, SoleBikeDetected) {
    SoleBikeTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, SoleEllipticalDetected) {
    SoleEllipticalTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, SoleF80TreadmillDetected) {
    SoleF80TreadmillTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, SpiritTreadmillDetected) {
    SpiritTreadmillTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, SportsPlusBikeDetected) {
    SportsPlusBikeTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, SportsTechBikeDetected) {
    SportsTechBikeTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, StagesBikeDetected) {
    StagesBikeTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, StrydeRunPowerSensorDetected) {
    StrydeRunPowerSensorTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, TacxNeo2Detected) {
    TacxNeo2TestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, TechnoGymMyRunTreadmillDetected) {
    TechnoGymMyRunTreadmillTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, TechnogymMyRunTreadmillRFCommDetected) {
    TechnogymMyRunTreadmillRFCommTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, ToorxTreadmillDetected) {
    ToorxTreadmillTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, TreadmillDetected) {
    TreadmillTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, TrueTreadmillDetected) {
    TrueTreadmillTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, TrxAppGateUSBBikeDetected) {
    TrxAppGateUSBBikeTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, TrxAppGateUSBTreadmillDetected) {
    TrxAppGateUSBTreadmillTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, UltrasportBikeDetected) {
    UltrasportBikeTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, WahooKickrSnapBikeDetected) {
    WahooKickrSnapBikeTestData testData;
    this->test_deviceDetection(testData);
}

TEST_F(BluetoothDeviceTestSuite, YesoulBikeDetected) {
    YesoulBikeTestData testData;
    this->test_deviceDetection(testData);
}








