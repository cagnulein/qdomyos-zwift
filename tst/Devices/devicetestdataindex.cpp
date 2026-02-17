#include "devicetestdataindex.h"
#include "deviceindex.h"

#include "bluetooth.h"
#include "devicenamepatterngroup.h"
#include "bluetoothdevicetestdata.h"
#include "bluetoothdevicetestdatabuilder.h"
#include "devicediscoveryinfo.h"
#include "qzsettings.h"




bool DeviceTestDataIndex::isInitialized = false;

/**
 * @brief hex2bytes Converts a hexadecimal string to bytes, 2 characters at a time.
 * @param s An hexadecimal string e.g. "023F4A" to  { 0x02, 0x3F, 0x4A }
 */
static QByteArray hex2bytes(const std::string& s)
{
    QByteArray v;

    for (size_t i = 0; i < s.length(); i +=2)
    {
        std::string slice(s, i, 2);
        uint8_t value = std::stoul(slice, 0, 16);
        v.append(value);
    }
    return v;
}


QMap<QString,const BluetoothDeviceTestData*> DeviceTestDataIndex::testData;


const std::vector<QString> DeviceTestDataIndex::Names() {
    std::vector<QString> result;

    for(auto key : testData.keys())
        result.push_back(key);

    return result;
}

const std::vector<const BluetoothDeviceTestData *> DeviceTestDataIndex::TestData() {
    std::vector<const BluetoothDeviceTestData*> result;

    for(auto item : testData)
        result.push_back(item);

    return result;
}

BluetoothDeviceTestDataBuilder *  DeviceTestDataIndex::RegisterNewDeviceTestData(const QString& name)
{
    auto existing = testData.value(name, nullptr);
    if(existing)
        delete existing;
    BluetoothDeviceTestDataBuilder * result = new BluetoothDeviceTestDataBuilder(name);
    testData.insert(name, result);
    return result;
}

const BluetoothDeviceTestData *DeviceTestDataIndex::GetTestData(const QString &name) {
    if(!isInitialized)
        throw std::invalid_argument("Device test data is not initialized.");

    return testData.value(name, nullptr);
}


QMultiMap<DeviceTypeId, const BluetoothDeviceTestData*> DeviceTestDataIndex::WhereExpects(const std::unordered_set<DeviceTypeId> &typeIds) {
    QMultiMap<DeviceTypeId, const BluetoothDeviceTestData*> result;

    if(typeIds.empty())
        return result;

    for(auto item : qAsConst(testData)) {
        if(typeIds.count(item->ExpectedDeviceType()))
            result.insert(item->ExpectedDeviceType(), item);
    }

    return result;
}

void DeviceTestDataIndex::Initialize() {

    if(isInitialized)
        return;

    const QString testIP = "1.2.3.4";    

    // Activio Treadmill
    RegisterNewDeviceTestData(DeviceIndex::ActivioTreadmill)
        ->expectDevice<activiotreadmill>()
        ->acceptDeviceName("RUNNERT", DeviceNameComparison::StartsWithIgnoreCase);

    // Apex Bike
    RegisterNewDeviceTestData(DeviceIndex::ApexBike)
        ->expectDevice<apexbike>()
        ->acceptDeviceName("WLT8266BM", DeviceNameComparison::StartsWithIgnoreCase);

    // Ant Bike
    RegisterNewDeviceTestData(DeviceIndex::AntBike)
        ->expectDevice<antbike>()
        ->acceptDeviceName("", DeviceNameComparison::Exact)
        ->configureSettingsWith(QZSettings::antbike);

    // BH Fitness Elliptical
    RegisterNewDeviceTestData(DeviceIndex::BHFitnessElliptical)
        ->expectDevice<bhfitnesselliptical>()
        ->acceptDeviceName("B01_", DeviceNameComparison::StartsWithIgnoreCase);


    // BKool Bike
    RegisterNewDeviceTestData(DeviceIndex::BKoolBike)
        ->expectDevice<bkoolbike>()
        ->acceptDeviceName("BKOOLSMARTPRO", DeviceNameComparison::StartsWithIgnoreCase);

    // Bowflex T216 Treadmill
    RegisterNewDeviceTestData(DeviceIndex::BowflexT216Treadmill)
        ->expectDevice<bowflext216treadmill>()
        ->acceptDeviceName("BOWFLEX T", DeviceNameComparison::StartsWithIgnoreCase);


    //// Bowflex Treadmill
    //RegisterNewDeviceTestData(DeviceIndex::BowflexTreadmill)
    //->expectDevice<bowflexttreadmill>();



    // ChronoBike
    RegisterNewDeviceTestData(DeviceIndex::Chronobike)
        ->expectDevice<chronobike>()
        ->acceptDeviceName("CHRONO ", DeviceNameComparison::StartsWithIgnoreCase);


    // Computrainer Bike
    RegisterNewDeviceTestData(DeviceIndex::ComputrainerBike)
        ->expectDevice<computrainerbike>()
        ->acceptDeviceName("", DeviceNameComparison::StartsWithIgnoreCase)
        ->configureSettingsWith(QZSettings::computrainer_serialport, "COMX", "");


    // Concept2 Ski Erg
    RegisterNewDeviceTestData(DeviceIndex::Concept2SkiErg)
        ->expectDevice<concept2skierg>()
        ->acceptDeviceName("PM5", "SKI", DeviceNameComparison::IgnoreCase)
        ->acceptDeviceName("PM5", DeviceNameComparison::IgnoreCase);

    // Crossrope
    RegisterNewDeviceTestData(DeviceIndex::Crossrope)
        ->expectDevice<crossrope>()
        ->acceptDeviceName("CROSSROPE", DeviceNameComparison::StartsWithIgnoreCase);

    // CSC Bike (Named)
    QString cscBikeName = "CyclingSpeedCadenceBike-";
    RegisterNewDeviceTestData(DeviceIndex::CSCBike_Named)
        ->expectDevice<cscbike>()        
        ->acceptDeviceName(cscBikeName, DeviceNameComparison::StartsWith)
        ->rejectDeviceName("X" + cscBikeName, DeviceNameComparison::Exact)
        ->configureSettingsWith([cscBikeName](DeviceDiscoveryInfo &info, bool enable) -> void {
            info.setValue(QZSettings::cadence_sensor_name, enable ? cscBikeName : "Disabled");
            info.setValue(QZSettings::cadence_sensor_as_bike, enable);
        });

    // CSafe Rower
    RegisterNewDeviceTestData(DeviceIndex::CSafeRower)
        ->expectDevice<csaferower>()
        ->acceptDeviceName("", DeviceNameComparison::StartsWithIgnoreCase)
        ->configureSettingsWith(QZSettings::csafe_rower, "COMX", "");

    // CSafe Elliptical
    RegisterNewDeviceTestData(DeviceIndex::CSafeElliptical)
        ->expectDevice<csafeelliptical>()
        ->acceptDeviceName("", DeviceNameComparison::StartsWithIgnoreCase)
        ->configureSettingsWith(QZSettings::csafe_elliptical_port, "COMX", "");

    cscBikeName = "CyclingSpeedCadenceBike-";
    RegisterNewDeviceTestData(DeviceIndex::CSCBike)
        ->expectDevice<cscbike>()
        ->acceptDeviceName(QStringLiteral("JOROTO-BK-"), DeviceNameComparison::StartsWithIgnoreCase)
        ->configureSettingsWith(
            [cscBikeName](const DeviceDiscoveryInfo &info, bool enable, std::vector<DeviceDiscoveryInfo> &configurations)->void
            {
                DeviceDiscoveryInfo config(info);

                if(enable) {
                    // If the Bluetooth name doesn't match the one being tested, but if csc_as_bike is enabled in the settings,
                    // and the bluetooth name does match the cscName in the settings, the device will be detected anyway,
                    // so prevent this by not including that specific configuration
                    //
                    // In order for the search to actually happen, the cscName has to be "Disabled" or csc_as_bike must be true.
                    /*
                                                    config.setValue(QZSettings::csc_as_bike, true);
                                                    config.setValue(QZSettings::cscName, cscBikeName);
                                                    configurations.push_back(config);
                                                    */

                    config.setValue(QZSettings::cadence_sensor_name, "Disabled");
                    config.setValue(QZSettings::cadence_sensor_as_bike, true);
                    configurations.push_back(config);

                    config.setValue(QZSettings::cadence_sensor_name, "Disabled");
                    config.setValue(QZSettings::cadence_sensor_as_bike, false);
                    configurations.push_back(config);

                    config.setValue(QZSettings::cadence_sensor_as_bike, true);
                    config.setValue(QZSettings::cadence_sensor_name,"NOT "+cscBikeName);
                    configurations.push_back(config);
                }
                else  {
                    // prevent the search
                    config.setValue(QZSettings::cadence_sensor_as_bike, false);
                    config.setValue(QZSettings::cadence_sensor_name, "NOT "+cscBikeName);
                    configurations.push_back(config);
                }
            });

    // Cyclops Phantom Bike
    RegisterNewDeviceTestData(DeviceIndex::CyclopsPhantomBike)
        ->expectDevice<cycleopsphantombike>()
        ->acceptDeviceName("INDOORCYCLE", DeviceNameComparison::StartsWithIgnoreCase);

    // DeerRun Treadmill
    RegisterNewDeviceTestData(DeviceIndex::DeerRunTreadmill)
        ->expectDevice<deerruntreadmill>()
        ->acceptDeviceName("PITPAT-T", DeviceNameComparison::StartsWithIgnoreCase);

    // Domyos bike
    RegisterNewDeviceTestData(DeviceIndex::DomyosBike)
        ->expectDevice<domyosbike>()        
        ->acceptDeviceName("Domyos-Bike", DeviceNameComparison::StartsWith)
        ->rejectDeviceName("DomyosBridge", DeviceNameComparison::StartsWith)
        ->configureSettingsWith([](const DeviceDiscoveryInfo& info, bool enable, std::vector<DeviceDiscoveryInfo>& configurations)->void{
            DeviceDiscoveryInfo config(info);

            if(enable) {
                // has service and both values of domyosbike_notftms
                config.includeBluetoothService(QBluetoothUuid((quint16)0x1826), false);
                config.setValue(QZSettings::domyosbike_notfmts, true);
                configurations.push_back(config);

                config.setValue(QZSettings::domyosbike_notfmts, false);
                configurations.push_back(config);

                config.includeBluetoothService(QBluetoothUuid((quint16)0x1826), true);
                config.setValue(QZSettings::domyosbike_notfmts, true);
                configurations.push_back(config);
            } else {
                config.includeBluetoothService(QBluetoothUuid((quint16)0x1826), true);
                config.setValue(QZSettings::domyosbike_notfmts, false);
                configurations.push_back(config);
            }
        });


    // DomyosElliptical
    RegisterNewDeviceTestData(DeviceIndex::DomyosElliptical)
        ->expectDevice<domyoselliptical>()        
        ->acceptDeviceName("Domyos-EL", DeviceNameComparison::StartsWith)
        ->rejectDeviceName("DomyosBridge",DeviceNameComparison::StartsWith);


    // Domyos rower
    RegisterNewDeviceTestData(DeviceIndex::DomyosRower)
        ->expectDevice<domyosrower>()        
        ->acceptDeviceName("DOMYOS-ROW", DeviceNameComparison::StartsWithIgnoreCase)
        ->rejectDeviceName("DomyosBridge", DeviceNameComparison::StartsWith);


    // Domyos Treadmill
    RegisterNewDeviceTestData(DeviceIndex::DomyosTreadmill)
        ->expectDevice<domyostreadmill>()        
        ->acceptDeviceName("Domyos", DeviceNameComparison::StartsWith)
        ->rejectDeviceName("DomyosBr", DeviceNameComparison::StartsWith)
        ->rejectDeviceName("DOMYOS-BIKING-", DeviceNameComparison::StartsWith)
        ->excluding<domyoselliptical>()
        ->excluding<domyosbike>()
        ->excluding<domyosrower>()
        ->excluding<horizontreadmill>()
        ->excluding<ftmsbike>()
        ->configureSettingsWith([](const DeviceDiscoveryInfo& info, bool enable, std::vector<DeviceDiscoveryInfo>& configurations) {
            DeviceDiscoveryInfo result(info);
            auto service = QBluetoothUuid((quint16)0x1826);

            // Enabled means the device name isn't the value of the ftms_treadmill setting
            result.setValue(QZSettings::ftms_treadmill, "NOT " + result.DeviceInfo()->name());

            // Doesn't have 0x1826 but domyostreadmill_notfmts is default
            result.removeBluetoothService(service);
            result.setValue(QZSettings::domyostreadmill_notfmts, true);
            configurations.push_back(result);
            result.setValue(QZSettings::domyostreadmill_notfmts, false);
            configurations.push_back(result);

            // Does have 0x1826 and domyostreadmill_notfmts is default
            result.addBluetoothService(service);
            result.setValue(QZSettings::domyostreadmill_notfmts, true);
            result.setValue(QZSettings::ftms_treadmill, "NOT " + result.DeviceInfo()->name());
            configurations.push_back(result);

            if(!enable) {
                // Disable the "enabling" configurations using the name
                for(auto& config : configurations)
                    config.setValue(QZSettings::ftms_treadmill, config.DeviceInfo()->name()); // TODO: do something to check case insensitivity

                // Has 0x1826 and domyostreadmill_notfmts is not default
                result.addBluetoothService(service);
                result.setValue(QZSettings::domyostreadmill_notfmts, false);
                result.setValue(QZSettings::ftms_treadmill, result.DeviceInfo()->name());
                configurations.push_back(result);

                result.addBluetoothService(service);
                result.setValue(QZSettings::domyostreadmill_notfmts, false);
                result.setValue(QZSettings::ftms_treadmill, "NOT " + result.DeviceInfo()->name());
                configurations.push_back(result);
            }
        });

    // Echelon Connect Sport Bike
    RegisterNewDeviceTestData(DeviceIndex::EchelonConnectSportBike)
        ->expectDevice<echelonconnectsport>()        
        ->acceptDeviceName("ECH", DeviceNameComparison::StartsWith)
        ->excluding<echelonrower>()
        ->excluding<echelonstride>();

    // Echelon Rower
    RegisterNewDeviceTestData(DeviceIndex::EchelonRower)
        ->expectDevice<echelonrower>()        
        ->acceptDeviceName("ECH-ROW", DeviceNameComparison::StartsWith)
        ->acceptDeviceName("ROWSPORT-", DeviceNameComparison::StartsWithIgnoreCase)
        ->acceptDeviceName("ROW-S", DeviceNameComparison::StartsWith);


    // Echelon Stride Treadmill
    RegisterNewDeviceTestData(DeviceIndex::EchelonStrideTreadmill)
        ->expectDevice<echelonstride>()        
        ->acceptDeviceName("ECH-STRIDE", DeviceNameComparison::StartsWithIgnoreCase)
        ->acceptDeviceName("ECH-UK-", DeviceNameComparison::StartsWithIgnoreCase)
        ->acceptDeviceName("ECH-SD-SPT", DeviceNameComparison::StartsWithIgnoreCase);


    // TODO: check if this is actually used
    // Elite Sterzo Smart
    RegisterNewDeviceTestData(DeviceIndex::EliteSterzoSmart)
        ->expectDevice<elitesterzosmart>()
        ->disable("Unable to detect with current logic");


    // ES Linker Treadmill
    RegisterNewDeviceTestData(DeviceIndex::ESLinkerTreadmill)
        ->expectDevice<eslinkertreadmill>()
        ->acceptDeviceName("ESLINKER", DeviceNameComparison::StartsWithIgnoreCase);


    // Fake Bike
    RegisterNewDeviceTestData(DeviceIndex::FakeBike)
        ->expectDevice<fakebike>()        
        ->acceptDeviceName("", DeviceNameComparison::StartsWithIgnoreCase)
        ->configureSettingsWith(QZSettings::applewatch_fakedevice);


    // Fake Elliptical
    RegisterNewDeviceTestData(DeviceIndex::FakeElliptical)
        ->expectDevice<fakeelliptical>()
        ->acceptDeviceName("", DeviceNameComparison::StartsWithIgnoreCase)
        ->configureSettingsWith(QZSettings::fakedevice_elliptical);

    // Fake Rower
    RegisterNewDeviceTestData(DeviceIndex::FakeRower)
        ->expectDevice<fakerower>()
        ->acceptDeviceName("", DeviceNameComparison::StartsWithIgnoreCase)
        ->configureSettingsWith(QZSettings::fakedevice_rower);

    // Fake Treadmill
    RegisterNewDeviceTestData(DeviceIndex::FakeTreadmill)
        ->expectDevice<faketreadmill>()        
        ->acceptDeviceName("", DeviceNameComparison::StartsWithIgnoreCase)
        ->configureSettingsWith(QZSettings::fakedevice_treadmill);


    // FitPlus F5
    RegisterNewDeviceTestData(DeviceIndex::FitPlusF5)
        ->expectDevice<fitplusbike>()        
        ->acceptDeviceName("FS-", DeviceNameComparison::StartsWith)
        ->configureSettingsWith( QZSettings::fitplus_bike);


    // FitPlus MRK
    RegisterNewDeviceTestData(DeviceIndex::FitPlusBike_MRK_NoSettings)
        ->expectDevice<fitplusbike>()
        ->acceptDeviceName("MRK-", DeviceNameComparison::StartsWith)
        ->rejectDeviceName("MRK-T25-", DeviceNameComparison::StartsWith)
        ->excluding<ftmsbike>()
        ->excluding<snodebike>();

    // FitShow FS
    RegisterNewDeviceTestData(DeviceIndex::FitShowFS)
        ->expectDevice<fitshowtreadmill>()        
        ->acceptDeviceName("FS-", DeviceNameComparison::StartsWith)
        ->configureSettingsWith(
            [](const DeviceDiscoveryInfo& info, bool enable, std::vector<DeviceDiscoveryInfo>& configurations) -> void
            {
                DeviceDiscoveryInfo config(info);

                if(enable){
                    config.setValue(QZSettings::snode_bike, false);
                    config.setValue(QZSettings::fitplus_bike, false);
                    configurations.push_back(config);
                } else {
                    for(int i=1; i<4; i++) {
                        config.setValue(QZSettings::snode_bike, i&1);
                        config.setValue(QZSettings::fitplus_bike, i&2);
                        configurations.push_back(config);
                    }
                }
            })
        ->excluding<ftmsbike>();


    // FitShow SW
    RegisterNewDeviceTestData(DeviceIndex::FitShowSW)
        ->expectDevice<fitshowtreadmill>()

        // SW, 14 characters total
        ->acceptDeviceName("SW345678901234", DeviceNameComparison::Exact)
        ->acceptDeviceName("SWFOURTEENCHAR", DeviceNameComparison::Exact)
        ->acceptDeviceName("WINFITA", DeviceNameComparison::StartsWithIgnoreCase)
        ->acceptDeviceName("NOBLEPRO CONNECT", DeviceNameComparison::StartsWithIgnoreCase)

        // too long and too short
        ->rejectDeviceName("SW3456789012345", DeviceNameComparison::Exact)
        ->rejectDeviceName("SW34567890123", DeviceNameComparison::Exact);


    // FitShow BF
    RegisterNewDeviceTestData(DeviceIndex::FitShowBF)
        ->expectDevice<fitshowtreadmill>()
        ->acceptDeviceName("BF70", DeviceNameComparison::StartsWith);


    // Flywheel Bike
    RegisterNewDeviceTestData(DeviceIndex::FlywheelBike)
        ->expectDevice<flywheelbike>()
        ->acceptDeviceName("Flywheel", DeviceNameComparison::StartsWith);


    // Flywheel Life Fitness IC8
    RegisterNewDeviceTestData(DeviceIndex::FlywheelLifeFitnessIC8)
        ->expectDevice<flywheelbike>()        
        ->acceptDeviceName("BIKE", DeviceNameComparison::StartsWithIgnoreCase, 6)
        ->configureSettingsWith(QZSettings::flywheel_life_fitness_ic8);

    // Focus Treadmill
    RegisterNewDeviceTestData(DeviceIndex::FocusTreadmill)
        ->expectDevice<focustreadmill>()
        ->acceptDeviceName("EW-TM-", DeviceNameComparison::StartsWithIgnoreCase);

    // FTMS Bike general
    auto ftmsBikeConfigureExclusions = {
        DeviceTestDataIndex::GetTypeId<snodebike>(),
        DeviceTestDataIndex::GetTypeId<fitplusbike>(),
        DeviceTestDataIndex::GetTypeId<stagesbike>()
    };

    // FTMS Bike Hammer Racer S
    RegisterNewDeviceTestData(DeviceIndex::FTMSBikeHammerRacerS)
        ->expectDevice<ftmsbike>()        
        ->acceptDeviceName("FS-", DeviceNameComparison::StartsWith)
        ->configureSettingsWith(QZSettings::hammer_racer_s)
        ->excluding(ftmsBikeConfigureExclusions);

    // FTMS Bike Hammer 64123
    RegisterNewDeviceTestData(DeviceIndex::FTMSBikeHammer)
        ->expectDevice<ftmsbike>()
        ->acceptDeviceName("HAMMER ", DeviceNameComparison::StartsWithIgnoreCase)
        ->configureSettingsWith(
            [](const DeviceDiscoveryInfo &info, bool enable, std::vector<DeviceDiscoveryInfo> &configurations) -> void
            {
                DeviceDiscoveryInfo config(info);

                if (enable) {
                    config.setValue(QZSettings::power_sensor_as_bike, false);
                    config.setValue(QZSettings::saris_trainer, false);
                    configurations.push_back(config);
                } else {
                for(int x = 1; x<=3; x++) {
                    config.setValue(QZSettings::power_sensor_as_bike, x & 1);
                    config.setValue(QZSettings::saris_trainer, x & 2);
                    configurations.push_back(config);
                }

            }})
        ->excluding(ftmsBikeConfigureExclusions);

    // FTMS Bike IConsole
    RegisterNewDeviceTestData(DeviceIndex::FTMSBikeIConsole)
        ->expectDevice<ftmsbike>()
        ->acceptDeviceName("ICONSOLE+", DeviceNameComparison::StartsWithIgnoreCase)
        ->configureSettingsWith(QZSettings::toorx_ftms)
        ->excluding(ftmsBikeConfigureExclusions);


    // FTMS Bike
    QStringList acceptableFTMSNames {
        "DHZ-", // JK fitness 577
        "MKSM", // MKSM3600036
        "YS_C1_", // Yesoul C1H
        "YS_G1_", // Yesoul S3
        "YS_G1MPLUS", // Yesoul G1M Plus
        "DS25-", // Bodytone DS25
        "SCHWINN 510T",
        "3G CARDIO ",
        "ZWIFT HUB",
        "FLXCY-", // Pro FlexBike
        "QB-WC01", // Nexgim QB-C01 smart bike
        "XBR55",
        "ECHO_BIKE_",
        "EW-JS-",
        "MERACH-MR667-",
        "DS60-",
        "SPAX-BK-",
        "YSV1",
        "VICTORY",
        "CECOTEC", // Cecotec DrumFit Indoor 10000 MagnoMotor Connected #2420
        "WATTBIKE",
        "ZYCLEZBIKE",
        "WAVEFIT-",
        "KETTLERBLE",
        "JAS_C3",
        "SCH_190U",
        "RAVE WHITE",
        "DOMYOS-BIKING-",
        "DU30-",
        "BIKZU_",
        "WLT8828",
        "VANRYSEL-HT",
        "HARISON-X15",
        "FEIVON V2",
        "FELVON V2",
        "ZUMO",
        "JUSTO",
        "T2 ",
        "VFSPINBIKE",
        "XS08-",
        "B94",
        "STAGES BIKE",
        "SUITO",
        "D2RIDE",
        "DIRETO X",
        "MERACH-667-",
        "SMB1",
        "UBIKE FTMS",
        "INRIDE"
    };
    RegisterNewDeviceTestData(DeviceIndex::FTMSBike)
        ->expectDevice<ftmsbike>()
        ->acceptDeviceNames(acceptableFTMSNames, DeviceNameComparison::StartsWithIgnoreCase)
        ->acceptDeviceName("DI", DeviceNameComparison::StartsWithIgnoreCase, 2) // Elite smart trainer #1682)
        ->acceptDeviceName("YSV", DeviceNameComparison::StartsWithIgnoreCase, 9) // YSV100783
        ->acceptDeviceName("URSB", DeviceNameComparison::StartsWithIgnoreCase, 7) // URSB005
        ->acceptDeviceName("DBF", DeviceNameComparison::StartsWithIgnoreCase, 6) // DBF135
        ->acceptDeviceName("KSU", DeviceNameComparison::StartsWithIgnoreCase, 7) // KSU1102
        ->acceptDeviceName("VOLT", DeviceNameComparison::StartsWithIgnoreCase, 4)
        ->acceptDeviceName("F","ARROW",DeviceNameComparison::IgnoreCase) // FI9110 Arrow, https://www.fitnessdigital.it/bicicletta-smart-bike-ion-fitness-arrow-connect/p/10022863/ IO Fitness Arrow
        ->acceptDeviceName("ICSE", DeviceNameComparison::StartsWithIgnoreCase, 4)
        ->acceptDeviceName("FLX", DeviceNameComparison::StartsWithIgnoreCase, 10)
        ->acceptDeviceName("CSRB", DeviceNameComparison::StartsWithIgnoreCase, 11)


        // Starts with DT- and is 14+ characters long.
        // TODO: update once addDeviceName can generate valid and invalid names for variable length patterns

        ->acceptDeviceName("DT-0123456789A", DeviceNameComparison::IgnoreCase) // Sole SB700
        ->acceptDeviceName("DT-0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ", DeviceNameComparison::IgnoreCase) // Sole SB700
        ->rejectDeviceName("DT-0123456789", DeviceNameComparison::IgnoreCase) // too short for Sole SB700
        ->rejectDeviceName("DBF13", DeviceNameComparison::IgnoreCase) // too short for DBF135
        ->rejectDeviceName("DBF1355", DeviceNameComparison::IgnoreCase) // too long for DBF135

        ->excluding(ftmsBikeConfigureExclusions);

    // FTMS Accessory
    QString ftmsAccessoryName = "accessory";
    RegisterNewDeviceTestData(DeviceIndex::FTMSAccessory)
        ->expectDevice<ftmsbike>()        
        ->acceptDeviceName(ftmsAccessoryName, DeviceNameComparison::StartsWithIgnoreCase)
        ->configureSettingsWith(
            [ftmsAccessoryName](DeviceDiscoveryInfo& info, bool enable)->void
            {
                info.setValue(QZSettings::ss2k_peloton, enable);
                info.setValue(QZSettings::ftms_accessory_name, enable ? ftmsAccessoryName : "NOT " + ftmsAccessoryName );
            })
        ->excluding(ftmsBikeConfigureExclusions);


    // FTMS "BIKE-"
    RegisterNewDeviceTestData(DeviceIndex::FTMSBike3)
        ->expectDevice<ftmsbike>()
        ->acceptDeviceName("BIKE-", DeviceNameComparison::StartsWithIgnoreCase)
        ->excluding(ftmsBikeConfigureExclusions)
        ->configureSettingsWith([](const DeviceDiscoveryInfo& info, bool enable, std::vector<DeviceDiscoveryInfo>& configurations) -> void {
            if(!enable)
                return;

            DeviceDiscoveryInfo config(info);

            // distinguish from npecablebike
            config.setValue(QZSettings::flywheel_life_fitness_ic8, true);
            configurations.push_back(config);
        });

    // FTMS Bike 2
    RegisterNewDeviceTestData(DeviceIndex::FTMSBike2)
        ->expectDevice<ftmsbike>()
        ->acceptDeviceNames({"GLT",
                             "SPORT01-"}, // Labgrey Magnetic Exercise Bike https://www.amazon.co.uk/dp/B0CXMF1NPY?_encoding=UTF8&psc=1&ref=cm_sw_r_cp_ud_dp_PE420HA7RD7WJBZPN075&ref_=cm_sw_r_cp_ud_dp_PE420HA7RD7WJBZPN075&social_share=cm_sw_r_cp_ud_dp_PE420HA7RD7WJBZPN075&skipTwisterOG=1,
                            DeviceNameComparison::StartsWithIgnoreCase)
        ->excluding(ftmsBikeConfigureExclusions)
        ->configureSettingsWith(QBluetoothUuid((quint16)0x1826));

    // FTMS Rower
    RegisterNewDeviceTestData(DeviceIndex::FTMSRower)
        ->expectDevice<ftmsrower>()        
        ->acceptDeviceNames({"CR 00","KAYAKPRO","WHIPR", "KS-WLT", "I-ROWER", "S4 COMMS"}, DeviceNameComparison::StartsWithIgnoreCase)
        ->acceptDeviceNames({"PM5ROW","PM5XROW","SF-RW"}, DeviceNameComparison::IgnoreCase);

    // Horizon GR7 Bike
    RegisterNewDeviceTestData(DeviceIndex::HorizonGR7Bike)
        ->expectDevice<horizongr7bike>()
        ->acceptDeviceName("JFIC", DeviceNameComparison::StartsWithIgnoreCase);


    // TODO: revisit

    // Horizon Treadmill
    RegisterNewDeviceTestData(DeviceIndex::HorizonTreadmill)
        ->expectDevice<horizontreadmill>()        
        ->acceptDeviceNames({"HORIZON","AFG SPORT","WLT2541"}, DeviceNameComparison::StartsWithIgnoreCase)
        ->acceptDeviceNames(
            // FTMS
            {"T318_", "T218_", "TRX3500", "JFTMPARAGON", "PARAGON X", "JFTM", "CT800",
             "MOBVOI TM", "DK202000725", "CTM780102C6BB32D62", "MX-TM ", "MATRIXTF50", "MOBVOI TM",
             "KETTLER TREADMILL", "ASSAULTRUNNER"}, DeviceNameComparison::StartsWithIgnoreCase);


    // Horizon Treadmill (Toorx)
    RegisterNewDeviceTestData(DeviceIndex::HorizonTreadmill_Toorx)
        ->expectDevice<horizontreadmill>()        
        ->acceptDeviceName("TOORX", DeviceNameComparison::StartsWithIgnoreCase)
        ->acceptDeviceName("I-CONSOLE+", DeviceNameComparison::StartsWithIgnoreCase)
        ->configureSettingsWith(
            [](const DeviceDiscoveryInfo &info, bool enable, std::vector<DeviceDiscoveryInfo> &configurations) -> void
            {
                DeviceDiscoveryInfo config(info);

                if (enable) {
                    config.setValue(QZSettings::toorx_ftms_treadmill, true);
                    config.setValue(QZSettings::toorx_ftms, false);
                    configurations.push_back(config);
                } else {
                    // Basic case where the device is disabled in the settings
                    config.setValue(QZSettings::toorx_ftms_treadmill, false);
                    config.setValue(QZSettings::toorx_ftms, false);
                    configurations.push_back(config);

                    // Basic case where the device is disabled in the settings and has an excluding settings
                    config.setValue(QZSettings::toorx_ftms_treadmill, false);
                    config.setValue(QZSettings::toorx_ftms, true);
                    configurations.push_back(config);

                    // Enabled in settings, but with excluding setting
                    config.setValue(QZSettings::toorx_ftms_treadmill, true);
                    config.setValue(QZSettings::toorx_ftms, true);
                    configurations.push_back(config);
                }
            });


    // Horizon Treadmill (Bodytone)
    RegisterNewDeviceTestData(DeviceIndex::HorizonTreadmill_Bodytone)
        ->expectDevice<horizontreadmill>()        
        ->acceptDeviceName("TF-", DeviceNameComparison::StartsWithIgnoreCase)
        ->configureSettingsWith(QZSettings::horizon_treadmill_force_ftms);

    // Horizon Treadmill (Domyos TC)
    RegisterNewDeviceTestData(DeviceIndex::HorizonTreadmill_DomyosTC)
        ->expectDevice<horizontreadmill>()        
        ->acceptDeviceName("DOMYOS-TC", DeviceNameComparison::StartsWithIgnoreCase)
        ->configureSettingsWith([](DeviceDiscoveryInfo& info, bool enable) -> void {
            info.setValue(QZSettings::domyostreadmill_notfmts, !enable);
            info.includeBluetoothService(QBluetoothUuid((quint16)0x1826), enable);
        });


    // iConcept Bike
    RegisterNewDeviceTestData(DeviceIndex::iConceptBike)
        ->expectDevice<iconceptbike>()
        ->acceptDeviceName("BH DUALKIT", DeviceNameComparison::StartsWithIgnoreCase)
        ->configureSettingsWith(QZSettings::iconcept_elliptical, false);

    // iConcept Elliptical
    RegisterNewDeviceTestData(DeviceIndex::iConceptElliptical)
        ->expectDevice<iconceptelliptical>()        
        ->acceptDeviceName("BH DUALKIT", DeviceNameComparison::StartsWithIgnoreCase)
        ->configureSettingsWith(QZSettings::iconcept_elliptical);


    // Inspire Bike
    RegisterNewDeviceTestData(DeviceIndex::InspireBike)
        ->expectDevice<inspirebike>()
        ->acceptDeviceName("IC", DeviceNameComparison::StartsWithIgnoreCase, 8);

    // Keep Bike
    RegisterNewDeviceTestData(DeviceIndex::KeepBike)
        ->expectDevice<keepbike>()
        ->acceptDeviceName("KEEP_BIKE_", DeviceNameComparison::StartsWithIgnoreCase);


    // Kingsmith R1 Pro Treadmill
    RegisterNewDeviceTestData(DeviceIndex::KingsmithR1ProTreadmill)
        ->expectDevice<kingsmithr1protreadmill>()
        ->acceptDeviceName("RE", DeviceNameComparison::IgnoreCase)
        ->acceptDeviceNames({"R1 PRO",
                             "KINGSMITH",
                             "KS-H",
                             "KS-BLC", // Walkingpad C2 #1672
                             "DYNAMAX",
                             "WALKINGPAD",
                             "KS-BLR", // Treadmill KingSmith WalkingPad R2 Pro KS-HCR1AA
                             "KS-ST-A1P", // KingSmith Walkingpad A1 Pro #2041

                            // Poland-distributed WalkingPad R2 TRR2FB
                            "KS-SC-BLR2C",

                            }, DeviceNameComparison::StartsWithIgnoreCase)
        ->rejectDeviceName("KS-HD-Z1D", DeviceNameComparison::StartsWithIgnoreCase) // it's an FTMS one
        ->excluding<kingsmithr2treadmill>();

    // Kingsmith R2 Treadmill
    RegisterNewDeviceTestData(DeviceIndex::KingsmithR2Treadmill)
        ->expectDevice<kingsmithr2treadmill>()        
        ->acceptDeviceName("KS-ST-K12PRO", DeviceNameComparison::StartsWithIgnoreCase)

        // KingSmith Walking Pad R2
        ->acceptDeviceNames({"KS-R1AC","KS-HC-R1AA","KS-HC-R1AC"}, DeviceNameComparison::StartsWithIgnoreCase)

        // KingSmith Walking Pad X21
        ->acceptDeviceNames({"KS-X21", "KS-HDSC-X21C","KS-HDSY-X21C","KS-NGCH-X21C","KS-NACH-X21C"}, DeviceNameComparison::StartsWithIgnoreCase)

        // KingSmith Walking Pad G1
        ->acceptDeviceName("KS-NGCH-G1C", DeviceNameComparison::StartsWithIgnoreCase);

    // Life Fitness Treadmill
    RegisterNewDeviceTestData(DeviceIndex::LifeFitnessTreadmill)
        ->expectDevice<lifefitnesstreadmill>()
        ->acceptDeviceName("LF", DeviceNameComparison::StartsWithIgnoreCase, 18);

    // M3I Bike
    RegisterNewDeviceTestData(DeviceIndex::M3IBike)
        ->expectDevice<m3ibike>()
        ->acceptDeviceName("M3", DeviceNameComparison::StartsWith)
        ->configureSettingsWith(
            [](DeviceDiscoveryInfo& info,  bool enable)->void
            {
                // The M3I bike detector looks into the manufacturer data.
                if(!enable) {
                    info.DeviceInfo()->setManufacturerData(1, QByteArray("Invalid manufacturer data."));
                    return;
                }

                int key=0;
                info.DeviceInfo()->setManufacturerData(key++, hex2bytes("02010639009F00000000000000000014008001"));

                /*
    // more data that has been supplied
    result.setManufacturerData(key++, hex2bytes("02010639009F00000000000000000014008001"));
    result.setManufacturerData(key++, hex2bytes("02010639009FBA000000020000000014008001"));
    result.setManufacturerData(key++, hex2bytes("02010639009FBA000000020000000013008001"));
    result.setManufacturerData(key++, hex2bytes("02010639009FBA000000020000000013008001"));
    result.setManufacturerData(key++, hex2bytes("02010639009FBA000000020000000012008001"));
    result.setManufacturerData(key++, hex2bytes("02010639009FBA000000020000000012008001"));
    result.setManufacturerData(key++, hex2bytes("02010639009FBA000000000000000011008001"));
    result.setManufacturerData(key++, hex2bytes("02010639009FBC000000020000000010008001"));
    result.setManufacturerData(key++, hex2bytes("02010639009FBC000000020000000010008001"));
    result.setManufacturerData(key++, hex2bytes("02010639009FBC00000002000000000F008001"));
    result.setManufacturerData(key++, hex2bytes("02010639009FBC00000002000000000F008001"));
    result.setManufacturerData(key++, hex2bytes("02010639009FBC00000002000000000E008001"));
    result.setManufacturerData(key++, hex2bytes("02010639009FBC00000002000000000E008001"));
    result.setManufacturerData(key++, hex2bytes("02010639009F0000000003000000000C000001"));
    result.setManufacturerData(key++, hex2bytes("02010639009FDD00000003000000000C000001"));
    result.setManufacturerData(key++, hex2bytes("02010639009FDD00000003000000000C000001"));
    result.setManufacturerData(key++, hex2bytes("02010639009FDD00000003000000000B000001"));
    result.setManufacturerData(key++, hex2bytes("02010639009FDD00000003000000000B000001"));
    result.setManufacturerData(key++, hex2bytes("02010639009FDD00000003000000000B000001"));
    result.setManufacturerData(key++, hex2bytes("02010639009FDD00000003000000000A000001"));
    result.setManufacturerData(key++, hex2bytes("02010639009FDD00000003000000000A000001"));
    result.setManufacturerData(key++, hex2bytes("02010639009F0000000000000000000A000001"));
    result.setManufacturerData(key++, hex2bytes("02010639009FE8000000030000000009000001"));
    result.setManufacturerData(key++, hex2bytes("02010639009FE8000000030000000009000001"));
    result.setManufacturerData(key++, hex2bytes("02010639009FE8000000030000000009000001"));
    result.setManufacturerData(key++, hex2bytes("02010639009FE8000000030000000008000001"));
    result.setManufacturerData(key++, hex2bytes("02010639009FE8000000030000000008000001"));
    result.setManufacturerData(key++, hex2bytes("02010639009FE8000000030000000008000001"));
    result.setManufacturerData(key++, hex2bytes("02010639009FE8000000030000000007000001"));
    result.setManufacturerData(key++, hex2bytes("02010639009FE8000000030000000000000001"));
    result.setManufacturerData(key++, hex2bytes("02010639009FD3000000030000000000000001"));
    result.setManufacturerData(key++, hex2bytes("02010639009FD3000000030000000000000001"));
    result.setManufacturerData(key++, hex2bytes("02010639009FD3000000030000000000000001"));
    result.setManufacturerData(key++, hex2bytes("02010639009FD3000000030000000000000001"));
    */

            });


    // MCF Bike
    RegisterNewDeviceTestData(DeviceIndex::MCFBike)
        ->expectDevice<mcfbike>()
        ->acceptDeviceName("MCF-", DeviceNameComparison::StartsWithIgnoreCase);


    // Mepanel Bike
    RegisterNewDeviceTestData(DeviceIndex::MepanelBike)
        ->expectDevice<mepanelbike>()
        ->acceptDeviceName("MEPANEL", DeviceNameComparison::StartsWithIgnoreCase);


    // Nautilus Bike
    RegisterNewDeviceTestData(DeviceIndex::NautilusBike)
        ->expectDevice<nautilusbike>()
        ->acceptDeviceName("NAUTILUS B", DeviceNameComparison::StartsWithIgnoreCase);


    // Nautilus Elliptical
    RegisterNewDeviceTestData(DeviceIndex::NautilusElliptical)
        ->expectDevice<nautiluselliptical>()
        ->acceptDeviceNames({"NAUTILUS E", "NAUTILUS M"}, DeviceNameComparison::StartsWithIgnoreCase);

    // Nautilus Treadmill
    RegisterNewDeviceTestData(DeviceIndex::NautilusTreadmill)
        ->expectDevice<nautilustreadmill>()
        ->acceptDeviceName("NAUTILUS T", DeviceNameComparison::StartsWithIgnoreCase);


    // Norditrack Elliptical
    RegisterNewDeviceTestData(DeviceIndex::NorditrackElliptical)
        ->expectDevice<nordictrackelliptical>()
        ->acceptDeviceName("I_EL", DeviceNameComparison::StartsWithIgnoreCase);


    // Nordictrack IFit ADB Bike
    RegisterNewDeviceTestData(DeviceIndex::NordictrackIFitADBBike)
        ->expectDevice<nordictrackifitadbbike>()
        ->acceptDeviceName("", DeviceNameComparison::StartsWithIgnoreCase)
        ->configureSettingsWith(QZSettings::tdf_10_ip, testIP, "");

    // Nordictrack IFit ADB Elliptical
    RegisterNewDeviceTestData(DeviceIndex::NordictrackIFitADBElliptical)
        ->expectDevice<nordictrackifitadbelliptical>()
        ->acceptDeviceName("", DeviceNameComparison::StartsWithIgnoreCase)
        ->configureSettingsWith(QZSettings::proform_elliptical_ip, testIP, "");

    // Nordictrack IFit ADB Treadmill
    RegisterNewDeviceTestData(DeviceIndex::NordictrackIFitADBTreadmill)
        ->expectDevice<nordictrackifitadbtreadmill>()        
        ->acceptDeviceName("", DeviceNameComparison::StartsWithIgnoreCase)
        ->configureSettingsWith(QZSettings::nordictrack_2950_ip, testIP, "");

    // NPE Cable Bike
    RegisterNewDeviceTestData(DeviceIndex::NPECableBike)
        ->expectDevice<npecablebike>()
        ->acceptDeviceName(">CABLE", DeviceNameComparison::StartsWithIgnoreCase)
        ->acceptDeviceName("MD", DeviceNameComparison::StartsWithIgnoreCase, 7)
        ->configureSettingsWith(QZSettings::flywheel_life_fitness_ic8, false);

    // NPE Cable Bike
    RegisterNewDeviceTestData(DeviceIndex::NPECableBike)
        ->expectDevice<npecablebike>()
        ->acceptDeviceName(">CABLE", DeviceNameComparison::StartsWithIgnoreCase)
        ->acceptDeviceName("MD", DeviceNameComparison::StartsWithIgnoreCase, 7);


    // NPE Cable Bike (excluding Flywheel Life Fitness IC8)
    RegisterNewDeviceTestData(DeviceIndex::NPECableBike_Excluding_FlywheelLifeFitnessIC8)
        ->expectDevice<npecablebike>()

        // BIKE 1, BIKE 2, BIKE 3...
        ->acceptDeviceName("BIKE", DeviceNameComparison::StartsWithIgnoreCase, 6)

        // put in a name that's too long.
        ->rejectDeviceName("BIKE567", DeviceNameComparison::IgnoreCase)

        ->configureSettingsWith(QZSettings::flywheel_life_fitness_ic8, false /* don't allow this device if the Flywheel bike is enabled. */);

    // Octane Elliptical
    RegisterNewDeviceTestData(DeviceIndex::OctaneElliptical)
        ->expectDevice<octaneelliptical>()
        ->acceptDeviceName("Q37", DeviceNameComparison::StartsWithIgnoreCase);

    // Octane Elliptical
    RegisterNewDeviceTestData(DeviceIndex::OctaneTreadmill)
        ->expectDevice<octanetreadmill>()
        ->acceptDeviceNames({"ZR7", "ZR8"}, DeviceNameComparison::StartsWithIgnoreCase);

    // Pafers Bike
    RegisterNewDeviceTestData(DeviceIndex::PafersBike)
        ->expectDevice<pafersbike>()
        ->acceptDeviceName("PAFERS_", DeviceNameComparison::StartsWithIgnoreCase)
        ->configureSettingsWith(QZSettings::pafers_treadmill,false);

    // Pafers Treadmill
    RegisterNewDeviceTestData(DeviceIndex::PafersTreadmill)
        ->expectDevice<paferstreadmill>()
        ->acceptDeviceName("PAFERS_", DeviceNameComparison::StartsWithIgnoreCase)
        ->configureSettingsWith( [](const DeviceDiscoveryInfo& info, bool enable, std::vector<DeviceDiscoveryInfo>& configurations)->void {
            DeviceDiscoveryInfo config(info);

            if (enable) {
                for(int x = 1; x<=3; x++) {
                    config.setValue(QZSettings::pafers_treadmill, x & 1);
                    config.setValue(QZSettings::pafers_treadmill_bh_iboxster_plus, x & 2);
                    configurations.push_back(config);
                }
            } else {
                config.setValue(QZSettings::pafers_treadmill, false);
                config.setValue(QZSettings::pafers_treadmill_bh_iboxster_plus, false);
                configurations.push_back(config);
            }
        });

    // Pitpat Bike
    RegisterNewDeviceTestData(DeviceIndex::PitpatBike)
        ->expectDevice<pitpatbike>()
        ->acceptDeviceName("PITPAT-S", DeviceNameComparison::StartsWithIgnoreCase);

    // Proform Bike
    RegisterNewDeviceTestData(DeviceIndex::ProformBike)
        ->expectDevice<proformbike>()        
        ->acceptDeviceNames({"I_EB", "I_SB"}, DeviceNameComparison::StartsWith);

    // Proform Elliptical
    RegisterNewDeviceTestData(DeviceIndex::ProformElliptical)
        ->expectDevice<proformelliptical>()
        ->acceptDeviceName("I_FS", DeviceNameComparison::StartsWithIgnoreCase);

    // Proform Elliptical Trainer
    RegisterNewDeviceTestData(DeviceIndex::ProformEllipticalTrainer)
        ->expectDevice<proformellipticaltrainer>()
        ->acceptDeviceName("I_VE", DeviceNameComparison::StartsWithIgnoreCase);

    // Proform Rower
    RegisterNewDeviceTestData(DeviceIndex::ProFormRower)
        ->expectDevice<proformrower>()
        ->acceptDeviceName("I_RW", DeviceNameComparison::StartsWithIgnoreCase);

    // Proform Treadmill
    RegisterNewDeviceTestData(DeviceIndex::ProFormTreadmill)
        ->expectDevice<proformtreadmill>()
        ->acceptDeviceName("I_TL", DeviceNameComparison::StartsWith);

    // ProForm Wifi Bike
    RegisterNewDeviceTestData(DeviceIndex::ProFormWifiBike)
        ->expectDevice<proformwifibike>()        
        ->acceptDeviceName("", DeviceNameComparison::StartsWithIgnoreCase)
        ->configureSettingsWith(QZSettings::proformtdf4ip, testIP, "");

    // ProForm Wifi Bike
    RegisterNewDeviceTestData(DeviceIndex::ProFormTelnetBike)
        ->expectDevice<proformtelnetbike>()
        ->acceptDeviceName("", DeviceNameComparison::StartsWithIgnoreCase)
        ->configureSettingsWith(QZSettings::proformtdf1ip, testIP, "")
        ->disable("Locks up");

    // ProForm Wifi Treadmill
    RegisterNewDeviceTestData(DeviceIndex::ProFormWifiTreadmill)
        ->expectDevice<proformwifitreadmill>()        
        ->acceptDeviceName("", DeviceNameComparison::StartsWithIgnoreCase)
        ->configureSettingsWith(QZSettings::proformtreadmillip, testIP, "");

    // Renpho Bike General
    auto renphoBikeExclusions = {
        GetTypeId<fitplusbike>(),
        GetTypeId<snodebike>()
    };

    // Renpho Bike
    RegisterNewDeviceTestData(DeviceIndex::RenphoBike)
        ->expectDevice<renphobike>()
        ->acceptDeviceName("RQ", DeviceNameComparison::StartsWithIgnoreCase, 5)
        ->acceptDeviceName("SCH130", DeviceNameComparison::StartsWithIgnoreCase)
        ->excluding(renphoBikeExclusions);

    // Renpho Bike
    RegisterNewDeviceTestData(DeviceIndex::RenphoBike2)
        ->expectDevice<renphobike>()
        // TODO: generate valid/invalid names for a minimum length
        ->acceptDeviceName("R-Q", DeviceNameComparison::StartsWithIgnoreCase,7)
        ->acceptDeviceName("R-Q12345678910", DeviceNameComparison::StartsWithIgnoreCase)
        ->rejectDeviceName("R-Q123", DeviceNameComparison::IgnoreCase) // too short
        ->configureSettingsWith(QZSettings::power_sensor_as_bike, false)
        ->excluding(renphoBikeExclusions);

    // Renpho Bike (Toorx FTMS)
    RegisterNewDeviceTestData(DeviceIndex::RenphoBike_ToorxFTMS)
        ->expectDevice<renphobike>()
        ->acceptDeviceName("TOORX", DeviceNameComparison::StartsWith)
        ->configureSettingsWith(
            [](const DeviceDiscoveryInfo &info, bool enable, std::vector<DeviceDiscoveryInfo> &configurations) -> void
            {
                DeviceDiscoveryInfo config(info);

                config.setValue(QZSettings::toorx_ftms, enable);
                config.setValue(QZSettings::toorx_bike, true);
                configurations.push_back(config);
                config.setValue(QZSettings::toorx_ftms, enable);
                config.setValue(QZSettings::toorx_bike, false);
                configurations.push_back(config);
            })
        ->excluding(renphoBikeExclusions);


    // Schwinn 170 Bike
    RegisterNewDeviceTestData(DeviceIndex::Schwinn170Bike)
        ->expectDevice<schwinn170bike>()        
        ->acceptDeviceName("SCHWINN 170/270", DeviceNameComparison::StartsWithIgnoreCase);


    // Schwinn IC4 Bike
    RegisterNewDeviceTestData(DeviceIndex::SchwinnIC4Bike)
        ->expectDevice<schwinnic4bike>()        
        ->acceptDeviceNames({"IC BIKE", "C7-", "C9/C10"}, DeviceNameComparison::StartsWithIgnoreCase)

        // 17 characters, beginning with C7-
        ->rejectDeviceName("C7-45678901234567", DeviceNameComparison::IgnoreCase);;


    // Shuaa5 Treadmill
    RegisterNewDeviceTestData(DeviceIndex::Shuaa5Treadmill)
        ->expectDevice<shuaa5treadmill>()
        ->acceptDeviceName("ZW-", DeviceNameComparison::StartsWithIgnoreCase)
        ->configureSettingsWith(QZSettings::ftms_bike, "X"+QZSettings::default_ftms_bike+"X", "XX");


    // Skandika Wiry Bike
    RegisterNewDeviceTestData(DeviceIndex::SkandikaWiryBike)
        ->expectDevice<skandikawiribike>()        
        ->acceptDeviceName("BFCP", DeviceNameComparison::StartsWithIgnoreCase);


    // Smart Row Rower
    RegisterNewDeviceTestData(DeviceIndex::SmartRowRower)
        ->expectDevice<smartrowrower>()        
        ->acceptDeviceName("SMARTROW", DeviceNameComparison::StartsWithIgnoreCase);

    // Snode Bike General
    auto snodeBikeExclusions = {
        GetTypeId<fitplusbike>(),
        GetTypeId<ftmsbike>()
    };

    // Snode Bike FS
    RegisterNewDeviceTestData(DeviceIndex::SnodeBikeFS)
        ->expectDevice<snodebike>()
        ->acceptDeviceName("FS-", DeviceNameComparison::StartsWith)
        ->configureSettingsWith(QZSettings::snode_bike)
        ->excluding(snodeBikeExclusions);

    // Snode Bike TF
    RegisterNewDeviceTestData(DeviceIndex::SnodeBikeTF)
        ->expectDevice<snodebike>()        
        ->acceptDeviceName("TF-", DeviceNameComparison::StartsWithIgnoreCase)
        ->configureSettingsWith(QZSettings::horizon_treadmill_force_ftms, false)
        ->excluding(snodeBikeExclusions);


    // Sole Bike
    RegisterNewDeviceTestData(DeviceIndex::SoleBike)
        ->expectDevice<solebike>()
        ->acceptDeviceNames({"LCB", "R92"}, DeviceNameComparison::StartsWithIgnoreCase);

    // Sole Elliptical
    RegisterNewDeviceTestData(DeviceIndex::SoleElliptical)
        ->expectDevice<soleelliptical>()        
        ->acceptDeviceNames({"E95S","E25","E55","E95","E98","XG400","E98S"}, DeviceNameComparison::StartsWithIgnoreCase);

    // Sole Elliptical 2
    RegisterNewDeviceTestData(DeviceIndex::SoleElliptical2)
        ->expectDevice<soleelliptical>()
        ->acceptDeviceName("E35", DeviceNameComparison::StartsWithIgnoreCase)
        ->configureSettingsWith(QBluetoothUuid((quint16)0x1826), false);

    // Sole F80 Treadmill
    RegisterNewDeviceTestData(DeviceIndex::SoleF80Treadmill)
        ->expectDevice<solef80treadmill>()        
        ->acceptDeviceNames({"F65","S77","TT8","F63","ST90"}, DeviceNameComparison::StartsWithIgnoreCase);

    // Sole F80 Treadmill 2
    RegisterNewDeviceTestData(DeviceIndex::SoleF80Treadmill2)
        ->expectDevice<solef80treadmill>()
        ->acceptDeviceNames({"TT8","ST90"}, DeviceNameComparison::StartsWithIgnoreCase)
        ->configureSettingsWith(QBluetoothUuid((quint16)0x1826), false);

    // Sole F85 Treadmill
    RegisterNewDeviceTestData(DeviceIndex::SoleF85Treadmill)
        ->expectDevice<solef80treadmill>()        
        ->acceptDeviceNames({"F85","F89","F80"}, DeviceNameComparison::StartsWithIgnoreCase)
        ->configureSettingsWith(QZSettings::sole_treadmill_inclination);

    // Spirit Treadmill
    RegisterNewDeviceTestData(DeviceIndex::SpiritTreadmill)
        ->expectDevice<spirittreadmill>()
        ->acceptDeviceNames({"XT385","XT485","XT800","XT900"}, DeviceNameComparison::StartsWithIgnoreCase);

    // Sports Plus Bike
    RegisterNewDeviceTestData(DeviceIndex::SportsPlusBike)
        ->expectDevice<sportsplusbike>()        
        ->acceptDeviceName("CARDIOFIT", DeviceNameComparison::StartsWithIgnoreCase);

    // Sports Tech Bike
    RegisterNewDeviceTestData(DeviceIndex::SportsTechBike)
        ->expectDevice<sportstechbike>()        
        ->acceptDeviceName("EW-BK", DeviceNameComparison::StartsWithIgnoreCase);


    // Stages Bike General
    auto stagesBikeExclusions = { GetTypeId<ftmsbike>() };

    // Stages Bike
    RegisterNewDeviceTestData(DeviceIndex::StagesBike)
        ->expectDevice<stagesbike>()        
        ->acceptDeviceNames({"STAGES ", "TACX SATORI", "RACER S", "ELITETRAINER"}, DeviceNameComparison::StartsWithIgnoreCase)
        ->acceptDeviceNames({"QD","DFC", "KU"}, DeviceNameComparison::IgnoreCase)
        ->excluding(stagesBikeExclusions);

    // Stages Bike Stages Bike (Assioma / Power Sensor disabled
    RegisterNewDeviceTestData(DeviceIndex::StagesBike_Assioma_PowerSensorDisabled)
        ->expectDevice<stagesbike>()        
        ->acceptDeviceName("ASSIOMA", DeviceNameComparison::StartsWithIgnoreCase)
        ->configureSettingsWith(QZSettings::power_sensor_name, "DisabledX", "XDisabled")
        ->excluding(stagesBikeExclusions);

    // Power (Stages) Bike
    QString powerSensorName = "WattsItCalled";
    RegisterNewDeviceTestData(DeviceIndex::StagesPowerBike)
        ->expectDevice<stagesbike>()
        ->acceptDeviceName(powerSensorName+"Suffix", DeviceNameComparison::Exact) // needs a non-trivial name, but could be anything
        ->configureSettingsWith([powerSensorName](const DeviceDiscoveryInfo& info, bool enable, std::vector<DeviceDiscoveryInfo>& configurations) -> void {
            DeviceDiscoveryInfo config(info);

            if(enable) {
                config.setValue(QZSettings::power_sensor_as_bike, true);
                config.setValue(QZSettings::power_sensor_name, powerSensorName);
                configurations.push_back(config);
            } else {
                // enabled but wrong name
                config.setValue(QZSettings::power_sensor_as_bike, true);
                config.setValue(QZSettings::power_sensor_name, "NOT "+ powerSensorName);
                configurations.push_back(config);

                // disabled but acceptable name
                config.setValue(QZSettings::power_sensor_as_bike, false);
                config.setValue(QZSettings::power_sensor_name, powerSensorName);
                configurations.push_back(config);

                // disabled and wrong name
                config.setValue(QZSettings::power_sensor_as_bike, false);
                config.setValue(QZSettings::power_sensor_name, "NOT "+powerSensorName);
                configurations.push_back(config);
            }
        });

    // StrydeRun Power Sensor
    RegisterNewDeviceTestData(DeviceIndex::StrydeRunTreadmill_PowerSensor)
        ->expectDevice<strydrunpowersensor>()        
        ->acceptDeviceName("", DeviceNameComparison::StartsWith,1) // accept any name
        ->configureSettingsWith(
            [](const DeviceDiscoveryInfo &info, bool enable, std::vector<DeviceDiscoveryInfo> &configurations) -> void
            {
                DeviceDiscoveryInfo config(info);
                QString name = config.DeviceInfo()->name();
                if(enable) {
                    // power_as_treadmill enabled and powerSensorName in settings matches device name
                    config.setValue(QZSettings::power_sensor_as_treadmill, true);
                    config.setValue(QZSettings::power_sensor_name, name);
                    configurations.push_back(config);
                } else {
                    // enabled but powerSensorName in settings does not match device name
                    config.setValue(QZSettings::power_sensor_as_treadmill, true);
                    config.setValue(QZSettings::power_sensor_name, "NOT " + name);
                    configurations.push_back(config);

                    // disabled with non-matching name
                    config.setValue(QZSettings::power_sensor_as_treadmill, false);
                    config.setValue(QZSettings::power_sensor_name, "NOT " + name);
                    configurations.push_back(config);

                    // disabled with matching name
                    config.setValue(QZSettings::power_sensor_as_treadmill, false);
                    config.setValue(QZSettings::power_sensor_name, name);
                    configurations.push_back(config);
                }
            });

    RegisterNewDeviceTestData(DeviceIndex::StrydeRunTreadmill_PowerSensor2)
        ->expectDevice<strydrunpowersensor>()
        ->acceptDeviceNames({"TREADMILL", "S10"}, DeviceNameComparison::StartsWithIgnoreCase)
        ->configureSettingsWith(QBluetoothUuid((quint16)0x1814));

    // Tacx Neo Bike
    RegisterNewDeviceTestData(DeviceIndex::TacxNeoBike)
        ->expectDevice<tacxneo2>()        
        ->acceptDeviceNames({"TACX ", "TACX SMART BIKE","THINK X"}, DeviceNameComparison::StartsWithIgnoreCase)
        ->rejectDeviceName("TACX SATORI", DeviceNameComparison::StartsWithIgnoreCase);

    // Tacx Neo 2 Bike
    RegisterNewDeviceTestData(DeviceIndex::TacxNeo2Bike)
        ->expectDevice<tacxneo2>()
        ->acceptDeviceName("", DeviceNameComparison::StartsWithIgnoreCase)
        ->rejectDeviceName("TACX SATORI", DeviceNameComparison::StartsWithIgnoreCase)
        ->configureSettingsWith([](const DeviceDiscoveryInfo& info, bool enable, std::vector<DeviceDiscoveryInfo>& configurations)->void {
            auto address = QBluetoothAddress(enable ? "C1:14:D9:9C:FB:01":"00:00:00:00:00:00");
            auto newDevice = QBluetoothDeviceInfo(address, info.DeviceName(), 0);
            auto config = DeviceDiscoveryInfo(info, newDevice);
            configurations.push_back(config);

        });

    // TechnoGym MyRun Treadmill
    RegisterNewDeviceTestData(DeviceIndex::TechnoGymMyRunTreadmill)
        ->expectDevice<technogymmyruntreadmill>()        
        ->acceptDeviceNames({"MYRUN ","MERACH-U3"}, DeviceNameComparison::StartsWithIgnoreCase)
        ->configureSettingsWith(QZSettings::technogym_myrun_treadmill_experimental, false);

    // TechnoGym MyRun Treadmill RF Comm
    RegisterNewDeviceTestData(DeviceIndex::TechnoGymMyRunTreadmillRFComm)
        ->expectDevice<technogymmyruntreadmillrfcomm>()        
        ->acceptDeviceNames({"MYRUN ","MERACH-U3"}, DeviceNameComparison::StartsWithIgnoreCase)
        ->configureSettingsWith(QZSettings::technogym_myrun_treadmill_experimental);

    // Toorx Treadmill
    RegisterNewDeviceTestData(DeviceIndex::ToorxTreadmill)
        ->expectDevice<toorxtreadmill>()        
        ->acceptDeviceName("TRX ROUTE KEY", DeviceNameComparison::StartsWith)
        ->acceptDeviceNames({"BH DUALKIT TREAD", "BH-TR-", "MASTERT40-"}, DeviceNameComparison::StartsWithIgnoreCase);

    // True Treadmill
    RegisterNewDeviceTestData(DeviceIndex::TrueTreadmill)
        ->expectDevice<truetreadmill>()
        ->acceptDeviceNames({"TRUE", "ASSAULT TREADMILL "}, DeviceNameComparison::StartsWithIgnoreCase)
        ->acceptDeviceName("WDWAY", DeviceNameComparison::StartsWithIgnoreCase, 8); // WdWay179

    // True Treadmill 2
    RegisterNewDeviceTestData(DeviceIndex::TrueTreadmill2)
        ->expectDevice<truetreadmill>()
        ->acceptDeviceName("TREADMILL", DeviceNameComparison::StartsWithIgnoreCase)
        ->configureSettingsWith([](const DeviceDiscoveryInfo& info, bool enable, std::vector<DeviceDiscoveryInfo>& configurations)-> void{
            DeviceDiscoveryInfo config(info);
            config.setValue(QZSettings::gem_module_inclination, !enable);

            auto bt = std::vector<QBluetoothUuid>{QBluetoothUuid((quint16)0x1814),QBluetoothUuid((quint16)0x1826)};

            for(size_t i=enable ? 0:1,
                 limit = enable ? 0:3;
                 i<=limit; i++) {
                config.includeBluetoothService(bt[0], i&1);
                config.includeBluetoothService(bt[1], i&2);
                configurations.push_back(config);
            }

});

    // Toorx AppGate USB Bike General
    auto toorxAppGateUSBBikeExclusions ={ GetTypeId<trxappgateusbtreadmill>() };

    // Toorx AppGate USB Bike
    RegisterNewDeviceTestData(DeviceIndex::ToorxAppGateUSBBike)
        ->expectDevice<trxappgateusbbike>()        
        ->acceptDeviceNames({"TUN ","PASYOU-","IBIKING+",}, DeviceNameComparison::StartsWithIgnoreCase)
        ->configureSettingsWith(
            [](const DeviceDiscoveryInfo &info, bool enable, std::vector<DeviceDiscoveryInfo> &configurations) -> void
            {
                // This particular case of TrxAppGateUSBBike is independent of the setting

                DeviceDiscoveryInfo config(info);
                config.setValue(QZSettings::toorx_bike, true);
                config.setValue(QZSettings::toorx_ftms_treadmill, !enable);
                configurations.push_back(config);

                config.setValue(QZSettings::toorx_bike, false);
                config.setValue(QZSettings::toorx_ftms_treadmill, !enable);
                configurations.push_back(config);
            })
        ->excluding(toorxAppGateUSBBikeExclusions);


    // TODO: revisit, this missing a few names

    // Toorx AppGate USB Bike (Enabled in settings)
    RegisterNewDeviceTestData(DeviceIndex::ToorxAppGateUSBBike_EnabledInSettings)
        ->expectDevice<trxappgateusbbike>()        
        ->acceptDeviceName("TOORX", DeviceNameComparison::StartsWith)
        ->acceptDeviceNames({"I-CONSOIE+",
                             "I-CONSOLE+",
                             "ICONSOLE+",
                             "VIFHTR2.1",
                             "REEBOK",
                             "DKN MOTION"}, DeviceNameComparison::StartsWithIgnoreCase)
        ->acceptDeviceName("CR011R", DeviceNameComparison::IgnoreCase)
        ->acceptDeviceNames({"CR011R", "xCR011R"}, DeviceNameComparison::StartsWithIgnoreCase) // a contains relationship
        ->configureSettingsWith(
            [](const DeviceDiscoveryInfo &info, bool enable, std::vector<DeviceDiscoveryInfo> &configurations) -> void
            {
                DeviceDiscoveryInfo config(info);

                if(enable) {
                    config.setValue(QZSettings::toorx_bike, true);
                    config.setValue(QZSettings::toorx_ftms_treadmill, false);
                    configurations.push_back(config);
                } else {
                    config.setValue(QZSettings::toorx_bike, false);
                    config.setValue(QZSettings::toorx_ftms_treadmill, true);
                    configurations.push_back(config);
                    config.setValue(QZSettings::toorx_bike, false);
                    config.setValue(QZSettings::toorx_ftms_treadmill, false);
                    configurations.push_back(config);
                }
            })
        ->excluding(toorxAppGateUSBBikeExclusions);


    auto trxAppGateUSBEllipticalSettingsApplicator =
        [](const DeviceDiscoveryInfo &info, bool enable, std::vector<DeviceDiscoveryInfo> &configurations) -> void
    {
        DeviceDiscoveryInfo config(info);
        if(enable) {
            config.setValue(QZSettings::ftms_bike, QZSettings::default_ftms_bike);
            configurations.push_back(config);
            config.setValue(QZSettings::ftms_bike, "X"+QZSettings::default_ftms_bike+"X");
            configurations.push_back(config);
        } else {
            config.setValue(QZSettings::ftms_bike, "PLACEHOLDER");
            configurations.push_back(config);
        }
    };
    // TrxAppGateUSB Elliptical
    RegisterNewDeviceTestData(DeviceIndex::TrxAppGateUSBElliptical)
        ->expectDevice<trxappgateusbelliptical>()        
        ->acceptDeviceName("FAL-SPORTS", DeviceNameComparison::StartsWithIgnoreCase)
        // TODO: deal with I-CONSOLE+
        ->excluding(toorxAppGateUSBBikeExclusions)
        ->configureSettingsWith(trxAppGateUSBEllipticalSettingsApplicator) ;


    // TrxAppGateUSB Elliptical (I-CONSOLE+)
    RegisterNewDeviceTestData(DeviceIndex::TrxAppGateUSBEllipticalIConsole)
        ->expectDevice<trxappgateusbelliptical>()
        ->acceptDeviceName("I-CONSOLE+", DeviceNameComparison::StartsWithIgnoreCase)
        ->excluding(toorxAppGateUSBBikeExclusions)
        ->configureSettingsWith([trxAppGateUSBEllipticalSettingsApplicator](const DeviceDiscoveryInfo &info, bool enable, std::vector<DeviceDiscoveryInfo> &configurations) -> void
                                {
                                    trxAppGateUSBEllipticalSettingsApplicator(info, true, configurations);

                                    if(!enable)
                                        trxAppGateUSBEllipticalSettingsApplicator(info, false, configurations);

                                    // enable or disable them wrt this applicator
                                    for(auto &config : configurations)
                                        config.setValue(QZSettings::iconsole_elliptical, enable);
                                }) ;

    // Toorx AppGate USB Treadmill
    RegisterNewDeviceTestData(DeviceIndex::ToorxAppGateUSBTreadmill)
        ->expectDevice<trxappgateusbtreadmill>()        
        ->acceptDeviceNames({"TOORX", "V-RUN"}, DeviceNameComparison::StartsWith)
        ->acceptDeviceName("K80_", DeviceNameComparison::StartsWithIgnoreCase)
        ->acceptDeviceNames({"I-CONSOLE+","ICONSOLE+","I-RUNNING","DKN RUN","ADIDAS ","REEBOK"}, DeviceNameComparison::StartsWithIgnoreCase)
        ->configureSettingsWith([](const DeviceDiscoveryInfo &info, bool enable, std::vector<DeviceDiscoveryInfo> &configurations) -> void
                                {
                                    DeviceDiscoveryInfo config(info);

                                    // TODO: revist because of all the settings this one needs to consider

                                    if(enable) {
                                        config.setValue(QZSettings::toorx_bike, false); // 1 of many possible
                                        config.setValue(QZSettings::toorx_ftms_treadmill, false);
                                        config.setValue(QZSettings::toorx_ftms, false);
                                        config.setValue(QZSettings::iconsole_elliptical, false);
                                        configurations.push_back(config);
                                    } else {
                                        for(int i=1; i<16; i++) {
                                            config.setValue(QZSettings::toorx_bike, i&1);
                                            config.setValue(QZSettings::toorx_ftms_treadmill, i&2);
                                            config.setValue(QZSettings::toorx_ftms, i&4);
                                            config.setValue(QZSettings::iconsole_elliptical, i&8);
                                            configurations.push_back(config);
                                        }
                                    }
                                })
        ->excluding<trxappgateusbbike>();

    // Ultrasport Bike
    RegisterNewDeviceTestData(DeviceIndex::UltrasportBike)
        ->expectDevice<ultrasportbike>()
        ->acceptDeviceName("X-BIKE", DeviceNameComparison::StartsWithIgnoreCase);


    // Wahoo KICKR CORE
    RegisterNewDeviceTestData(DeviceIndex::WahooKickrSnapBike_KICKRCORE)
        ->expectDevice<wahookickrsnapbike>()
        ->acceptDeviceNames({"KICKR CORE" }, DeviceNameComparison::StartsWithIgnoreCase)
        ->excluding<ftmsbike>();

    // Wahoo Kickr Snap Bike
    RegisterNewDeviceTestData(DeviceIndex::WahooKickrSnapBike)
        ->expectDevice<wahookickrsnapbike>()        
        ->acceptDeviceNames({"KICKR SNAP","KICKR BIKE", "KICKR ROLLR","WAHOO KICKR"}, DeviceNameComparison::StartsWithIgnoreCase)
        ->excluding<ftmsbike>();

    // Wahoo Kickr Snap Bike (Saris Trainer)
    RegisterNewDeviceTestData(DeviceIndex::WahooKickrSarisTrainer)
        ->expectDevice<wahookickrsnapbike>()
        ->acceptDeviceName("HAMMER ", DeviceNameComparison::StartsWithIgnoreCase)
        ->configureSettingsWith(QZSettings::saris_trainer)
        ->excluding<ftmsbike>();

    // Yesoul Bike
    RegisterNewDeviceTestData(DeviceIndex::YesoulBike)
        ->expectDevice<yesoulbike>()        
        ->acceptDeviceName(yesoulbike::bluetoothName, DeviceNameComparison::StartsWith)
        ->acceptDeviceName("YS_G1M_", DeviceNameComparison::StartsWithIgnoreCase);

    // Ypoo Elliptical
    RegisterNewDeviceTestData(DeviceIndex::YpooElliptical)
        ->expectDevice<ypooelliptical>()        
        ->acceptDeviceNames({"YPOO-U3-", "SCH_590E", "KETTLER"}, DeviceNameComparison::StartsWithIgnoreCase);

    // Ypoo Elliptical 2
    RegisterNewDeviceTestData(DeviceIndex::YpooElliptical2)
        ->expectDevice<ypooelliptical>()
        ->acceptDeviceName("E35", DeviceNameComparison::StartsWithIgnoreCase)
        ->configureSettingsWith(QBluetoothUuid((quint16)0x1826));

    // Ypoo Elliptical 3
    RegisterNewDeviceTestData(DeviceIndex::YpooElliptical3)
        ->expectDevice<ypooelliptical>()
        ->acceptDeviceName("FS-", DeviceNameComparison::StartsWith)
        ->configureSettingsWith(QZSettings::iconsole_elliptical);

    // Zipro Treadmill
    RegisterNewDeviceTestData(DeviceIndex::ZiproTreadmill)
        ->expectDevice<ziprotreadmill>()
        ->acceptDeviceName("RZ_TREADMIL", DeviceNameComparison::StartsWithIgnoreCase);


    // TODO: revisit
    // Zwift Runpod
    QString zwiftRunPodPowerSensorName = "WattsItCalled";
    RegisterNewDeviceTestData(DeviceIndex::ZwiftRunpod)
        ->expectDevice<strydrunpowersensor>()        
        ->acceptDeviceName("ZWIFT RUNPOD", DeviceNameComparison::StartsWithIgnoreCase)
        ->configureSettingsWith(
            [zwiftRunPodPowerSensorName](const DeviceDiscoveryInfo &info, bool enable, std::vector<DeviceDiscoveryInfo> &configurations) -> void
            {
                DeviceDiscoveryInfo config(info);

                if(enable) {
                    /* Avoid the config that enables the StrydeRunPowerSensorTestData device
                    // power_as_treadmill enabled and powerSensorName in settings matches device name
                    config.setValue(QZSettings::power_as_treadmill, true);
                    config.setValue(QZSettings::powerSensorName, powerSensorName);
                    configurations.push_back(config);
                    */

                    /*
                     * In order for the search to occur, the power sensor name must start with "Disabled", or
                     * power_as_bike or power_as_treadmill must be true.
                    */

                    config.setValue(QZSettings::power_sensor_as_treadmill, true);
                    config.setValue(QZSettings::power_sensor_name, "NOT " + zwiftRunPodPowerSensorName);
                    configurations.push_back(config);

                    config.setValue(QZSettings::power_sensor_as_treadmill, false);
                    config.setValue(QZSettings::power_sensor_name, "Disabled");
                    configurations.push_back(config);

                } else {
                    // disable the search
                    config.setValue(QZSettings::power_sensor_as_treadmill, false);
                    config.setValue(QZSettings::power_sensor_name, zwiftRunPodPowerSensorName);
                    config.setValue(QZSettings::power_sensor_as_bike, false);
                    configurations.push_back(config);
                }
            });


    isInitialized = true;

    // Debug log the type ids
    for(auto deviceTestData : testData) {
        qDebug() << "Device: " << deviceTestData->Name() << " expected device type id: " << deviceTestData->ExpectedDeviceType();
    }

    // Validate the test data
    for(auto deviceTestData : testData) {

        try {
            auto exclusions = deviceTestData->Exclusions();
        } catch(std::domain_error) {
            qDebug() << "Device: " << deviceTestData->Name() << " specifies at least 1 exclusion for which no test data was found.";
        }

    }
}


