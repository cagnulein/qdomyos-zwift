#include <QApplication>

#include <memory>
#include "bluetooth.h"
#include "bluetoothdevicetestsuite.h"
#include "bluetoothdevicetestdatafactory.h"


template <class T>
void BluetoothDeviceTestSuite::test_deviceDetection() {
    T testData;
    test_deviceDetection(testData);
}

void BluetoothDeviceTestSuite::test_deviceDetection(BluetoothDeviceTestData& testData)
{
    const QString testUUID = QStringLiteral("b8f79bac-32e5-11ed-a261-0242ac120002");
    QBluetoothUuid uuid{testUUID};
    QStringList names = testData.get_deviceNames();

    EXPECT_GT(names.length(), 0) << "No bluetooth names configured for test";

    devicediscoveryinfo defaultDiscoveryInfo;

    // Test that the device is identified when enabled in the settings
    auto enablingConfigurations = testData.get_configurations(defaultDiscoveryInfo, true);
    auto disablingConfigurations = testData.get_configurations(defaultDiscoveryInfo, false);

    // Assuming these settings don't matter for this device.
    if(enablingConfigurations.size()==0)
        enablingConfigurations.push_back(defaultDiscoveryInfo);

    // set up options suitable for testing device creation
    discoveryoptions options;
    options.startDiscovery = false;
    options.createTemplateManagers = false;

    bluetooth bt(options);

    EXPECT_GT(enablingConfigurations.size(), 0) << "Test broken - there should be at least 1 enabling configuration";

    for(devicediscoveryinfo discoveryInfo : enablingConfigurations) {
        for(QString deviceName : names)
        {
            QBluetoothDeviceInfo deviceInfo = testData.get_bluetoothDeviceInfo(uuid, deviceName);
            auto discovered = bluetooth::discoverDevice(discoveryInfo, deviceInfo);
            EXPECT_EQ(discovered.type, testData.get_expectedDeviceType()) << "Expected device type not detected for name: " << deviceName.toStdString();

            // try to create the device
            std::shared_ptr<bluetoothdevice> device;

            EXPECT_NO_THROW(device = std::shared_ptr<bluetoothdevice>(bt.createDevice(discovered))) << "Failed to create device object";

            EXPECT_TRUE(testData.get_isExpectedDevice(device.get()));
        }
    }

    // Test that the device is not identified when disabled in the settings
    for(devicediscoveryinfo discoveryInfo : disablingConfigurations) {
        for(QString deviceName : names)
        {
            QBluetoothDeviceInfo deviceInfo = testData.get_bluetoothDeviceInfo(uuid, deviceName);
            auto discovered = bluetooth::discoverDevice(discoveryInfo, deviceInfo);
            EXPECT_NE(discovered.type, testData.get_expectedDeviceType()) << "Expected device type detected when disabled";
        }
    }

    if(testData.get_testInvalidBluetoothDeviceInfo()) {
        // Test that the device is not identified when disabled in the settings        
        for(devicediscoveryinfo discoveryInfo : disablingConfigurations) {
            for(QString deviceName : names)
            {
                QBluetoothDeviceInfo deviceInfo = testData.get_bluetoothDeviceInfo(uuid, deviceName, false);
                auto discovered = bluetooth::discoverDevice(discoveryInfo, deviceInfo);
                EXPECT_NE(discovered.type, testData.get_expectedDeviceType()) << "Expected device type detected from invalid bluetooth device info";
            }
        }
    }

    // Test that it doesn't detect this device if its higher priority "namesakes" are already detected.
    auto exclusions = testData.get_exclusions();
    for(auto exclusion : exclusions) {
        for(devicediscoveryinfo enablingDiscoveryInfo : enablingConfigurations) {
            devicediscoveryinfo discoveryInfo(enablingDiscoveryInfo);

            // indicate that the excludes are "already discovered"
            discoveryInfo.exclude(exclusion->get_expectedDeviceType());

            for(QString deviceName : names)
            {
                QBluetoothDeviceInfo deviceInfo = testData.get_bluetoothDeviceInfo(uuid, deviceName);
                auto discovered = bluetooth::discoverDevice(discoveryInfo, deviceInfo);
                EXPECT_NE(discovered.type, testData.get_expectedDeviceType()) << "Detected device in spite of exclusion";
            }
        }
    }

    // Test that it doesn't detect this device for the "wrong" names
    for(devicediscoveryinfo enablingDiscoveryInfo : enablingConfigurations) {

        devicediscoveryinfo discoveryInfo(enablingDiscoveryInfo);
        names = testData.get_failingDeviceNames();

        for(QString deviceName : names)
        {
            QBluetoothDeviceInfo deviceInfo = testData.get_bluetoothDeviceInfo(uuid, deviceName);
            auto discovered = bluetooth::discoverDevice(discoveryInfo, deviceInfo);
            EXPECT_NE(discovered.type, testData.get_expectedDeviceType()) << "Detected device from invalid name: " << deviceName.toStdString();
        }
    }
}

TEST_F(BluetoothDeviceTestSuite, ActivioTreadmillDetected) {
    this->test_deviceDetection<ActivioTreadmillTestData>();
}

TEST_F(BluetoothDeviceTestSuite, BHFitnessEllipticalDetected) {
    this->test_deviceDetection<BHFitnessEllipticalTestData>();
}

TEST_F(BluetoothDeviceTestSuite, BikeDetected) {
    GTEST_SKIP() << "Not a real device.";
    this->test_deviceDetection<BikeTestData>();
}

TEST_F(BluetoothDeviceTestSuite, BowflexT216TreadmillDetected) {
    this->test_deviceDetection<BowflexT216TreadmillTestData>();
}

TEST_F(BluetoothDeviceTestSuite, BowflexTreadmillDetected) {
    GTEST_SKIP() << "Device not supported";

    this->test_deviceDetection<BowflexTreadmillTestData>();
}

TEST_F(BluetoothDeviceTestSuite, ChronobikeDetected) {
    this->test_deviceDetection<ChronobikeTestData>();
}

TEST_F(BluetoothDeviceTestSuite, Concept2SkiErgDetected) {
    this->test_deviceDetection<Concept2SkiErgTestData>();
}

TEST_F(BluetoothDeviceTestSuite, CSCBikeDetected) {
    this->test_deviceDetection<CSCBikeTestData>();
}

TEST_F(BluetoothDeviceTestSuite, DomyosBikeDetected) {
    this->test_deviceDetection<DomyosBikeTestData>();
}

TEST_F(BluetoothDeviceTestSuite, DomyosEllipticalDetected) {
    this->test_deviceDetection<DomyosEllipticalTestData>();
}

TEST_F(BluetoothDeviceTestSuite, DomyosRowerDetected) {
    this->test_deviceDetection<DomyosRowerTestData>();
}

TEST_F(BluetoothDeviceTestSuite, DomyosTreadmillDetected) {
    this->test_deviceDetection<DomyosTreadmillTestData>();
}

TEST_F(BluetoothDeviceTestSuite, EchelonConnectSportBikeDetected) {
    this->test_deviceDetection<EchelonConnectSportBikeTestData>();
}

TEST_F(BluetoothDeviceTestSuite, EchelonRowerDetected) {
    this->test_deviceDetection<EchelonRowerTestData>();
}

TEST_F(BluetoothDeviceTestSuite, EchelonStrideTreadmillDetected) {
    this->test_deviceDetection<EchelonStrideTreadmillTestData>();
}

TEST_F(BluetoothDeviceTestSuite, EllipticalDetected) {
    GTEST_SKIP() << "Not a real device.";
    this->test_deviceDetection<EllipticalTestData>();
}

TEST_F(BluetoothDeviceTestSuite, ESLinkerTreadmillDetected) {
    this->test_deviceDetection<ESLinkerTreadmillTestData>();
}

TEST_F(BluetoothDeviceTestSuite, FakeBikeDetected) {
    this->test_deviceDetection<FakeBikeTestData>();
}

TEST_F(BluetoothDeviceTestSuite, FakeEllipticalDetected) {
    this->test_deviceDetection<FakeEllipticalTestData>();
}

TEST_F(BluetoothDeviceTestSuite, FakeTreadmillDetected) {
    this->test_deviceDetection<FakeTreadmillTestData>();
}

TEST_F(BluetoothDeviceTestSuite, FitPlusBikeFSDetected) {
    this->test_deviceDetection<FitPlusBikeFSTestData>();
}

TEST_F(BluetoothDeviceTestSuite, FitPlusBikeMRKDetected) {
    this->test_deviceDetection<FitPlusBikeMRKTestData>();
}

TEST_F(BluetoothDeviceTestSuite, FitshowTreadmillFSDetected) {
    this->test_deviceDetection<FitshowTreadmillFSTestData>();
}

TEST_F(BluetoothDeviceTestSuite, FitshowTreadmillSWDetected) {
    this->test_deviceDetection<FitshowTreadmillSWTestData>();
}

TEST_F(BluetoothDeviceTestSuite, FitshowTreadmillBFDetected) {
    this->test_deviceDetection<FitshowTreadmillBFTestData>();
}

TEST_F(BluetoothDeviceTestSuite, FlywheelBike1Detected) {
    this->test_deviceDetection<FlywheelBike1TestData>();
}

TEST_F(BluetoothDeviceTestSuite, FlywheelBike2Detected) {
    this->test_deviceDetection<FlywheelBike2TestData>();
}

TEST_F(BluetoothDeviceTestSuite, FTMSBike1Detected) {
    this->test_deviceDetection<FTMSBike1TestData>();
}

TEST_F(BluetoothDeviceTestSuite, FTMSBike2Detected) {
    this->test_deviceDetection<FTMSBike2TestData>();
}

TEST_F(BluetoothDeviceTestSuite, FTMSRowerDetected) {
    this->test_deviceDetection<FTMSRowerTestData>();
}

TEST_F(BluetoothDeviceTestSuite, HorizonGR7BikeDetected) {
    this->test_deviceDetection<HorizonGR7BikeTestData>();
}

TEST_F(BluetoothDeviceTestSuite, HorizonTreadmillDetected) {
    this->test_deviceDetection<HorizonTreadmillTestData>();
}

TEST_F(BluetoothDeviceTestSuite, iConceptBikeDetected) {
    this->test_deviceDetection<iConceptBikeTestData>();
}

TEST_F(BluetoothDeviceTestSuite, InspireBikeDetected) {
    this->test_deviceDetection<InspireBikeTestData>();
}

TEST_F(BluetoothDeviceTestSuite, KeepBikeDetected) {
    this->test_deviceDetection<KeepBikeTestData>();
}

TEST_F(BluetoothDeviceTestSuite, KingsmithR1ProTreadmillDetected) {
    this->test_deviceDetection<KingsmithR1ProTreadmillTestData>();
}

TEST_F(BluetoothDeviceTestSuite, KingsmithR2TreadmillDetected) {
    this->test_deviceDetection<KingsmithR2TreadmillTestData>();
}

TEST_F(BluetoothDeviceTestSuite, M3IBikeDetected) {
    this->test_deviceDetection<M3IBikeTestData>();
}

TEST_F(BluetoothDeviceTestSuite, MCFBikeDetected) {
    this->test_deviceDetection<MCFBikeTestData>();
}

TEST_F(BluetoothDeviceTestSuite, NautilusBikeDetected) {
    this->test_deviceDetection<NautilusBikeTestData>();
}

TEST_F(BluetoothDeviceTestSuite, NautilusEllipticalDetected) {
    this->test_deviceDetection<NautilusEllipticalTestData>();
}

TEST_F(BluetoothDeviceTestSuite, NautilusTreadmillDetected) {
    this->test_deviceDetection<NautilusTreadmillTestData>();
}

TEST_F(BluetoothDeviceTestSuite, NordicTrackEllipticalDetected) {
    this->test_deviceDetection<NordicTrackEllipticalTestData>();
}

TEST_F(BluetoothDeviceTestSuite, NordicTrackIFitADBTreadmillDetected) {
    this->test_deviceDetection<NordicTrackIFitADBTreadmillTestData>();
}

TEST_F(BluetoothDeviceTestSuite, NPECableBike1Detected) {
    this->test_deviceDetection<NPECableBike1TestData>();
}

TEST_F(BluetoothDeviceTestSuite, NPECableBike2Detected) {

    this->test_deviceDetection<NPECableBike2TestData>();
}

TEST_F(BluetoothDeviceTestSuite, OctaneTreadmillDetected) {
    this->test_deviceDetection<OctaneTreadmillTestData>();
}

TEST_F(BluetoothDeviceTestSuite, PafersBikeDetected) {
    this->test_deviceDetection<PafersBikeTestData>();
}

TEST_F(BluetoothDeviceTestSuite, PafersTreadmillDetected) {
    this->test_deviceDetection<PafersTreadmillTestData>();
}

TEST_F(BluetoothDeviceTestSuite, ProFormBikeDetected) {
    this->test_deviceDetection<ProFormBikeTestData>();
}

TEST_F(BluetoothDeviceTestSuite, ProFormEllipticalDetected) {
    this->test_deviceDetection<ProFormEllipticalTestData>();
}

TEST_F(BluetoothDeviceTestSuite, ProFormEllipticalTrainerDetected) {
    this->test_deviceDetection<ProFormEllipticalTrainerTestData>();
}

TEST_F(BluetoothDeviceTestSuite, ProFormRowerDetected) {
    this->test_deviceDetection<ProFormRowerTestData>();
}

TEST_F(BluetoothDeviceTestSuite, ProFormTreadmillDetected) {
    this->test_deviceDetection<ProFormTreadmillTestData>();
}

TEST_F(BluetoothDeviceTestSuite, ProFormWiFiBikeDetected) {
    this->test_deviceDetection<ProFormWiFiBikeTestData>();
}

TEST_F(BluetoothDeviceTestSuite, ProFormWiFiTreadmillDetected) {
    this->test_deviceDetection<ProFormWiFiTreadmillTestData>();
}

TEST_F(BluetoothDeviceTestSuite, RenphoBike1Detected) {
    this->test_deviceDetection<RenphoBike1TestData>();
}

TEST_F(BluetoothDeviceTestSuite, RenphoBike2Detected) {
    this->test_deviceDetection<RenphoBike2TestData>();
}

TEST_F(BluetoothDeviceTestSuite, RowerDetected) {
    GTEST_SKIP() << "Not a real device.";
    this->test_deviceDetection<RowerTestData>();
}

TEST_F(BluetoothDeviceTestSuite, SchwinnIC4BikeDetected) {
    this->test_deviceDetection<SchwinnIC4BikeTestData>();
}

TEST_F(BluetoothDeviceTestSuite, Shuaa5TreadmillDetected) {
    this->test_deviceDetection<Shuaa5TreadmillTestData>();
}

TEST_F(BluetoothDeviceTestSuite, SkandikaWiryBikeDetected) {
    this->test_deviceDetection<SkandikaWiryBikeTestData>();
}

TEST_F(BluetoothDeviceTestSuite, SmartRowRowerDetected) {
    this->test_deviceDetection<SmartRowRowerTestData>();
}

TEST_F(BluetoothDeviceTestSuite, SnodeBike1Detected) {
    this->test_deviceDetection<SnodeBike1TestData>();
}

TEST_F(BluetoothDeviceTestSuite, SnodeBike2Detected) {
    this->test_deviceDetection<SnodeBike2TestData>();
}

TEST_F(BluetoothDeviceTestSuite, SoleBikeDetected) {
    this->test_deviceDetection<SoleBikeTestData>();
}

TEST_F(BluetoothDeviceTestSuite, SoleEllipticalDetected) {
    this->test_deviceDetection<SoleEllipticalTestData>();
}

TEST_F(BluetoothDeviceTestSuite, SoleF80TreadmillDetected) {
    this->test_deviceDetection<SoleF80TreadmillTestData>();
}

TEST_F(BluetoothDeviceTestSuite, SpiritTreadmillDetected) {
    this->test_deviceDetection<SpiritTreadmillTestData>();
}

TEST_F(BluetoothDeviceTestSuite, SportsPlusBikeDetected) {
    this->test_deviceDetection<SportsPlusBikeTestData>();
}

TEST_F(BluetoothDeviceTestSuite, SportsTechBikeDetected) {
    this->test_deviceDetection<SportsTechBikeTestData>();
}

TEST_F(BluetoothDeviceTestSuite, StagesBike1Detected) {
    this->test_deviceDetection<StagesBike1TestData>();
}

TEST_F(BluetoothDeviceTestSuite, StagesBike2Detected) {
    this->test_deviceDetection<StagesBike2TestData>();
}

TEST_F(BluetoothDeviceTestSuite, StrydeRunPowerSensorDetected) {
    this->test_deviceDetection<StrydeRunPowerSensorTestData>();
}

TEST_F(BluetoothDeviceTestSuite, TacxNeo2Detected) {
    this->test_deviceDetection<TacxNeo2TestData>();
}

TEST_F(BluetoothDeviceTestSuite, TechnoGymMyRunTreadmillDetected) {
    this->test_deviceDetection<TechnoGymMyRunTreadmillTestData>();
}

TEST_F(BluetoothDeviceTestSuite, TechnogymMyRunTreadmillRFCommDetected) {
    #ifndef Q_OS_IOS
        this->test_deviceDetection<TechnogymMyRunTreadmillRFCommTestData>();
    #else
        GTEST_SKIP() << "Not relevant to iOS";
    #endif

}

TEST_F(BluetoothDeviceTestSuite, ToorxTreadmillDetected) {
    this->test_deviceDetection<ToorxTreadmillTestData>();
}

TEST_F(BluetoothDeviceTestSuite, TreadmillDetected) {
    GTEST_SKIP() << "Not a real device.";
    this->test_deviceDetection<TreadmillTestData>();
}

TEST_F(BluetoothDeviceTestSuite, TrueTreadmillDetected) {
    this->test_deviceDetection<TrueTreadmillTestData>();
}

TEST_F(BluetoothDeviceTestSuite, TrxAppGateUSBBike1Detected) {
    this->test_deviceDetection<TrxAppGateUSBBike1TestData>();
}

TEST_F(BluetoothDeviceTestSuite, TrxAppGateUSBBike2Detected) {
    this->test_deviceDetection<TrxAppGateUSBBike2TestData>();
}

TEST_F(BluetoothDeviceTestSuite, TrxAppGateUSBTreadmillDetected) {
    this->test_deviceDetection<TrxAppGateUSBTreadmillTestData>();
}

TEST_F(BluetoothDeviceTestSuite, UltrasportBikeDetected) {
    this->test_deviceDetection<UltrasportBikeTestData>();
}

TEST_F(BluetoothDeviceTestSuite, WahooKickrSnapBikeDetected) {
    this->test_deviceDetection<WahooKickrSnapBikeTestData>();
}

TEST_F(BluetoothDeviceTestSuite, YesoulBikeDetected) {
    this->test_deviceDetection<YesoulBikeTestData>();
}







