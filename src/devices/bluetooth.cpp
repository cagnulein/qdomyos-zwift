#include "bluetooth.h"
#include "homeform.h"
#include <QBluetoothLocalDevice>
#include <QDateTime>
#include <QFile>
#include <QMetaEnum>

#include <QtXml>
#ifdef Q_OS_ANDROID
#include "androidactivityresultreceiver.h"
#include "keepawakehelper.h"
#include <QAndroidJniObject>
#endif

bluetooth::bluetooth(const discoveryoptions &options)
    : bluetooth(options.logs, options.deviceName, options.noWriteResistance, options.noHeartService,
                options.pollDeviceTime, options.noConsole, options.testResistance, options.bikeResistanceOffset,
                options.bikeResistanceGain, options.startDiscovery) {}

bluetooth::bluetooth(bool logs, const QString &deviceName, bool noWriteResistance, bool noHeartService,
                     uint32_t pollDeviceTime, bool noConsole, bool testResistance, int8_t bikeResistanceOffset,
                     double bikeResistanceGain, bool startDiscovery) {
    QSettings settings;
    QLoggingCategory::setFilterRules(QStringLiteral("qt.bluetooth* = true"));
    filterDevice = deviceName;
    this->testResistance = testResistance;
    this->noWriteResistance = noWriteResistance;
    this->noHeartService = noHeartService;
    this->pollDeviceTime = pollDeviceTime;
    this->noConsole = noConsole;
    this->logs = logs;
    this->bikeResistanceGain = bikeResistanceGain;
    this->bikeResistanceOffset = bikeResistanceOffset;

    this->useDiscovery = startDiscovery;

    QString nordictrack_2950_ip =
        settings.value(QZSettings::nordictrack_2950_ip, QZSettings::default_nordictrack_2950_ip).toString();

    if (settings.value(QZSettings::peloton_bike_ocr, QZSettings::default_peloton_bike_ocr).toBool() && !pelotonBike) {
        pelotonBike = new pelotonbike(noWriteResistance, noHeartService);
        emit deviceConnected(QBluetoothDeviceInfo());
        connect(pelotonBike, &bluetoothdevice::connectedAndDiscovered, this, &bluetooth::connectedAndDiscovered);
        connect(pelotonBike, &pelotonbike::debug, this, &bluetooth::debug);
        if (this->discoveryAgent && !this->discoveryAgent->isActive()) {
            emit searchingStop();
        }
        // this signal is not associated to anything in this moment, since the homeform is not loaded yet
        this->signalBluetoothDeviceConnected(pelotonBike);
    }

#ifdef TEST
    schwinnIC4Bike = (schwinnic4bike *)new bike();
    // this signal is not associated to anything in this moment, since the homeform is not loaded yet
    this->signalBluetoothDeviceConnected(schwinnIC4Bike);
    connectedAndDiscovered();
    return;
#endif

    if (!startDiscovery) {
        this->discoveryAgent = nullptr;
        return;
    }

#if !defined(WIN32) && !defined(Q_OS_IOS)
    if (QBluetoothLocalDevice::allDevices().isEmpty()) {
        debug(QStringLiteral("no bluetooth dongle found!"));
    } else

#endif
    {
        // Create a discovery agent and connect to its signals
        discoveryAgent = new QBluetoothDeviceDiscoveryAgent(this);
        connect(discoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered, this, &bluetooth::deviceDiscovered);

#if (QT_VERSION >= QT_VERSION_CHECK(5, 12, 0))
        connect(discoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceUpdated, this, &bluetooth::deviceUpdated);

#endif
        connect(discoveryAgent, &QBluetoothDeviceDiscoveryAgent::canceled, this, &bluetooth::canceled);
#ifndef Q_OS_WIN
        connect(discoveryAgent, &QBluetoothDeviceDiscoveryAgent::finished, this, &bluetooth::finished);
#else
        connect(&discoveryTimeout, &QTimer::timeout, this, &bluetooth::finished);
        discoveryTimeout.start(10000);
#endif

        // Start a discovery
#ifndef Q_OS_WIN
        discoveryAgent->setLowEnergyDiscoveryTimeout(10000);
#endif
        this->startDiscovery();
    }
}

bluetooth::~bluetooth() {

    /*if(device())
    {
        device()->disconnectBluetooth();
    }*/
}

void bluetooth::signalBluetoothDeviceConnected(bluetoothdevice *b) { emit this->bluetoothDeviceConnected(b); }

void bluetooth::finished() {
    debug(QStringLiteral("BTLE scanning finished"));

    QSettings settings;
    bool antbike =
        settings.value(QZSettings::antbike, QZSettings::default_antbike).toBool();
    QString nordictrack_2950_ip =
        settings.value(QZSettings::nordictrack_2950_ip, QZSettings::default_nordictrack_2950_ip).toString();
    QString tdf_10_ip = settings.value(QZSettings::tdf_10_ip, QZSettings::default_tdf_10_ip).toString();
    QString proform_elliptical_ip = settings.value(QZSettings::proform_elliptical_ip, QZSettings::default_proform_elliptical_ip).toString();
    bool fake_bike =
        settings.value(QZSettings::applewatch_fakedevice, QZSettings::default_applewatch_fakedevice).toBool();
    bool fakedevice_elliptical =
        settings.value(QZSettings::fakedevice_elliptical, QZSettings::default_fakedevice_elliptical).toBool();
    bool fakedevice_rower = settings.value(QZSettings::fakedevice_rower, QZSettings::default_fakedevice_rower).toBool();
    bool fakedevice_treadmill =
        settings.value(QZSettings::fakedevice_treadmill, QZSettings::default_fakedevice_treadmill).toBool();
    // wifi devices on windows
    if (!nordictrack_2950_ip.isEmpty() || !tdf_10_ip.isEmpty() || fake_bike || fakedevice_elliptical || fakedevice_rower || fakedevice_treadmill || !proform_elliptical_ip.isEmpty() || antbike) {
        // faking a bluetooth device
        qDebug() << "faking a bluetooth device for nordictrack_2950_ip";
        deviceDiscovered(QBluetoothDeviceInfo());
    }

    if (device()) {
        qDebug() << QStringLiteral("bluetooth::finished but discoveryAgent is not active");
        return;
    }

    QString heartRateBeltName =
        settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name).toString();
    QString ftmsAccessoryName =
        settings.value(QZSettings::ftms_accessory_name, QZSettings::default_ftms_accessory_name).toString();
    bool csc_as_bike =
        settings.value(QZSettings::cadence_sensor_as_bike, QZSettings::default_cadence_sensor_as_bike).toBool();
    bool power_as_bike =
        settings.value(QZSettings::power_sensor_as_bike, QZSettings::default_power_sensor_as_bike).toBool();
    bool power_as_treadmill =
        settings.value(QZSettings::power_sensor_as_treadmill, QZSettings::default_power_sensor_as_treadmill).toBool();
    QString cscName =
        settings.value(QZSettings::cadence_sensor_name, QZSettings::default_cadence_sensor_name).toString();
    QString powerSensorName =
        settings.value(QZSettings::power_sensor_name, QZSettings::default_power_sensor_name).toString();
    QString eliteRizerName =
        settings.value(QZSettings::elite_rizer_name, QZSettings::default_elite_rizer_name).toString();
    QString eliteSterzoSmartName =
        settings.value(QZSettings::elite_sterzo_smart_name, QZSettings::default_elite_sterzo_smart_name).toString();
    bool cscFound = cscName.startsWith(QStringLiteral("Disabled")) && !csc_as_bike;
    bool powerSensorFound =
        powerSensorName.startsWith(QStringLiteral("Disabled")) && !power_as_bike && !power_as_treadmill;
    bool eliteRizerFound = eliteRizerName.startsWith(QStringLiteral("Disabled"));
    bool eliteSterzoSmartFound = eliteSterzoSmartName.startsWith(QStringLiteral("Disabled"));
    bool heartRateBeltFound = heartRateBeltName.startsWith(QStringLiteral("Disabled"));
    bool ftmsAccessoryFound = ftmsAccessoryName.startsWith(QStringLiteral("Disabled"));
    bool ss2k_peloton = settings.value(QZSettings::ss2k_peloton, QZSettings::default_ss2k_peloton).toBool();

    if (ss2k_peloton)
        ftmsAccessoryFound = true;

    // since i can have multiple fanfit i can't wait more because i don't have the full list of the fanfit
    // devices connected to QZ. edit: let's wait at the last one item
    bool fitmetriaFanfitFound =
        !settings.value(QZSettings::fitmetria_fanfit_enable, QZSettings::default_fitmetria_fanfit_enable).toBool();

    bool zwiftDeviceFound =
        !settings.value(QZSettings::zwift_click, QZSettings::default_zwift_click).toBool() && !settings.value(QZSettings::zwift_play, QZSettings::default_zwift_play).toBool();

    bool sramDeviceFound = !settings.value(QZSettings::sram_axs_controller, QZSettings::default_sram_axs_controller).toBool();

    if ((!heartRateBeltFound && !heartRateBeltAvaiable()) || (!ftmsAccessoryFound && !ftmsAccessoryAvaiable()) ||
        (!cscFound && !cscSensorAvaiable()) || (!powerSensorFound && !powerSensorAvaiable()) ||
        (!eliteRizerFound && !eliteRizerAvaiable()) || (!eliteSterzoSmartFound && !eliteSterzoSmartAvaiable()) ||
        (!fitmetriaFanfitFound && !fitmetriaFanfitAvaiable()) ||
        (!zwiftDeviceFound && !zwiftDeviceAvaiable()) ||
        (!sramDeviceFound && !sramDeviceAvaiable())) {

        // force heartRateBelt off
        forceHeartBeltOffForTimeout = true;
    }

    this->startDiscovery();
}

void bluetooth::startDiscovery() {

    if (!this->useDiscovery)
        return;

#ifndef Q_OS_IOS
    QSettings settings;
    bool technogym_myrun_treadmill_experimental = settings
                                                      .value(QZSettings::technogym_myrun_treadmill_experimental,
                                                             QZSettings::default_technogym_myrun_treadmill_experimental)
                                                      .toBool();
    bool trx_route_key = settings.value(QZSettings::trx_route_key, QZSettings::default_trx_route_key).toBool();
    bool bh_spada_2 = settings.value(QZSettings::bh_spada_2, QZSettings::default_bh_spada_2).toBool();
    bool iconcept_elliptical =
        settings.value(QZSettings::iconcept_elliptical, QZSettings::default_iconcept_elliptical).toBool();

    if (!trx_route_key && !bh_spada_2 && !technogym_myrun_treadmill_experimental && !iconcept_elliptical) {
#endif
        discoveryAgent->start(QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);
#ifndef Q_OS_IOS
    } else {
        discoveryAgent->start(QBluetoothDeviceDiscoveryAgent::ClassicMethod |
                              QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);
    }
#endif
}

void bluetooth::stopDiscovery() {
    if (this->discoveryAgent)
        this->discoveryAgent->stop();
    else
        qDebug() << "bluetooth::stopDiscovery() called when discoveryAgent is not defined. ";
}

void bluetooth::canceled() {
    debug(QStringLiteral("BTLE scanning stops"));

    emit searchingStop();
}

void bluetooth::debug(const QString &text) {
    if (logs) {

        qDebug() << text;
    }
}

bool bluetooth::cscSensorAvaiable() {

    QSettings settings;
    bool csc_as_bike =
        settings.value(QZSettings::cadence_sensor_as_bike, QZSettings::default_cadence_sensor_as_bike).toBool();
    QString cscName =
        settings.value(QZSettings::cadence_sensor_name, QZSettings::default_cadence_sensor_name).toString();

    if (csc_as_bike) {
        return false;
    }

    for (const QBluetoothDeviceInfo &b : qAsConst(devices)) {
        if (!cscName.compare(b.name())) {

            return true;
        }
    }
    return false;
}

bool bluetooth::ftmsAccessoryAvaiable() {

    QSettings settings;
    QString ftmsAccessoryName =
        settings.value(QZSettings::ftms_accessory_name, QZSettings::default_ftms_accessory_name).toString();
    bool ss2k_peloton = settings.value(QZSettings::ss2k_peloton, QZSettings::default_ss2k_peloton).toBool();
    if (ss2k_peloton)
        return false;

    Q_FOREACH (QBluetoothDeviceInfo b, devices) {
        if (!ftmsAccessoryName.compare(b.name())) {

            return true;
        }
    }
    return false;
}

bool bluetooth::fitmetriaFanfitAvaiable() {

    Q_FOREACH (QBluetoothDeviceInfo b, devices) {
        if (!b.name().compare("FITFAN-", Qt::CaseInsensitive)) {
            return true;
        } else if (b.name().toUpper().startsWith("HEADWIND ")) {
            return true;
        }
    }
    return false;
}

bool bluetooth::zwiftDeviceAvaiable() {

    uint8_t count = 0;
    Q_FOREACH (QBluetoothDeviceInfo b, devices) {
        if (b.name().toUpper().startsWith("ZWIFT ")) {
            count++;
            if(count >= 2)
                return true;
        }
    }
    return false;
}

bool bluetooth::sramDeviceAvaiable() {

    Q_FOREACH (QBluetoothDeviceInfo b, devices) {
        if (b.name().toUpper().startsWith("SRAM ")) {
           return true;
        }
    }
    return false;
}


bool bluetooth::powerSensorAvaiable() {

    QSettings settings;
    bool power_as_bike =
        settings.value(QZSettings::power_sensor_as_bike, QZSettings::default_power_sensor_as_bike).toBool();
    bool power_as_treadmill =
        settings.value(QZSettings::power_sensor_as_treadmill, QZSettings::default_power_sensor_as_treadmill).toBool();
    QString powerSensorName =
        settings.value(QZSettings::power_sensor_name, QZSettings::default_power_sensor_name).toString();

    if (power_as_bike || power_as_treadmill) {
        return false;
    }

    Q_FOREACH (QBluetoothDeviceInfo b, devices) {
        if (!powerSensorName.compare(b.name())) {

            return true;
        }
    }
    return false;
}

bool bluetooth::eliteRizerAvaiable() {

    QSettings settings;
    QString eliteRizerName =
        settings.value(QZSettings::elite_rizer_name, QZSettings::default_elite_rizer_name).toString();

    Q_FOREACH (QBluetoothDeviceInfo b, devices) {
        if (!eliteRizerName.compare(b.name())) {

            return true;
        }
    }
    return false;
}

bool bluetooth::eliteSterzoSmartAvaiable() {

    QSettings settings;
    QString eliteSterzoSmartName =
        settings.value(QZSettings::elite_sterzo_smart_name, QZSettings::default_elite_sterzo_smart_name).toString();

    Q_FOREACH (QBluetoothDeviceInfo b, devices) {
        if (!eliteSterzoSmartName.compare(b.name())) {

            return true;
        }
    }
    return false;
}

bool bluetooth::heartRateBeltAvaiable() {

    QSettings settings;
    QString heartRateBeltName =
        settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name).toString();

    Q_FOREACH (QBluetoothDeviceInfo b, devices) {
        if (!heartRateBeltName.compare(b.name())) {

            return true;
        }
    }
    return false;
}

void bluetooth::setLastBluetoothDevice(const QBluetoothDeviceInfo &b) {
    QSettings settings;
    settings.setValue(QZSettings::bluetooth_lastdevice_name, b.name());
#ifndef Q_OS_IOS
    settings.setValue(QZSettings::bluetooth_lastdevice_address, b.address().toString());
#else
    settings.setValue(QZSettings::bluetooth_lastdevice_address, b.deviceUuid().toString());
#endif
}

// this doesn't work on Windows. So be careful!
bool bluetooth::deviceHasService(const QBluetoothDeviceInfo &device, QBluetoothUuid service) {
    foreach(QBluetoothUuid s, device.serviceUuids()) {
        if(s == service) {
            return true;
        }
    }
    return false;
}

void bluetooth::deviceDiscovered(const QBluetoothDeviceInfo &device) {

    QSettings settings;
    QString heartRateBeltName =
        settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name).toString();
    QString ftmsAccessoryName =
        settings.value(QZSettings::ftms_accessory_name, QZSettings::default_ftms_accessory_name).toString();
    bool heartRateBeltFound = heartRateBeltName.startsWith(QStringLiteral("Disabled"));
    bool ftmsAccessoryFound = ftmsAccessoryName.startsWith(QStringLiteral("Disabled"));
    bool sramDeviceFound = !settings.value(QZSettings::sram_axs_controller, QZSettings::default_sram_axs_controller).toBool();
    bool zwiftDeviceFound =
        !settings.value(QZSettings::zwift_click, QZSettings::default_zwift_click).toBool() && !settings.value(QZSettings::zwift_play, QZSettings::default_zwift_play).toBool();
    bool fitmetriaFanfitFound =
        !settings.value(QZSettings::fitmetria_fanfit_enable, QZSettings::default_fitmetria_fanfit_enable).toBool();
    bool toorx_ftms = settings.value(QZSettings::toorx_ftms, QZSettings::default_toorx_ftms).toBool();
    bool toorx_ftms_treadmill =
        settings.value(QZSettings::toorx_ftms_treadmill, QZSettings::default_toorx_ftms_treadmill).toBool();
    bool toorx_bike = (settings.value(QZSettings::toorx_bike, QZSettings::default_toorx_bike).toBool() ||
                       settings.value(QZSettings::jll_IC400_bike, QZSettings::default_jll_IC400_bike).toBool() ||
                       settings.value(QZSettings::fytter_ri08_bike, QZSettings::default_fytter_ri08_bike).toBool() ||
                       settings.value(QZSettings::asviva_bike, QZSettings::default_asviva_bike).toBool() ||
                       settings.value(QZSettings::enerfit_SPX_9500, QZSettings::default_enerfit_SPX_9500).toBool() ||
                       settings.value(QZSettings::toorx_srx_3500, QZSettings::default_toorx_srx_3500).toBool() ||
                       settings.value(QZSettings::hop_sport_hs_090h_bike, QZSettings::default_hop_sport_hs_090h_bike).toBool() ||
                       settings.value(QZSettings::toorx_bike_srx_500, QZSettings::default_toorx_bike_srx_500).toBool() ||
                       settings.value(QZSettings::hertz_xr_770, QZSettings::default_hertz_xr_770).toBool()) &&
                      !toorx_ftms;
    bool snode_bike = settings.value(QZSettings::snode_bike, QZSettings::default_snode_bike).toBool();
    bool fitplus_bike = settings.value(QZSettings::fitplus_bike, QZSettings::default_fitplus_bike).toBool() ||
                        settings.value(QZSettings::virtufit_etappe, QZSettings::default_virtufit_etappe).toBool();
    bool csc_as_bike =
        settings.value(QZSettings::cadence_sensor_as_bike, QZSettings::default_cadence_sensor_as_bike).toBool();
    bool power_as_bike =
        settings.value(QZSettings::power_sensor_as_bike, QZSettings::default_power_sensor_as_bike).toBool();
    bool power_as_treadmill =
        settings.value(QZSettings::power_sensor_as_treadmill, QZSettings::default_power_sensor_as_treadmill).toBool();
    QString cscName =
        settings.value(QZSettings::cadence_sensor_name, QZSettings::default_cadence_sensor_name).toString();
    bool cscFound = cscName.startsWith(QStringLiteral("Disabled")) || csc_as_bike;
    bool hammerRacerS = settings.value(QZSettings::hammer_racer_s, QZSettings::default_hammer_racer_s).toBool();
    bool flywheel_life_fitness_ic8 =
        settings.value(QZSettings::flywheel_life_fitness_ic8, QZSettings::default_flywheel_life_fitness_ic8).toBool();
    QString powerSensorName =
        settings.value(QZSettings::power_sensor_name, QZSettings::default_power_sensor_name).toString();
    QString eliteRizerName =
        settings.value(QZSettings::elite_rizer_name, QZSettings::default_elite_rizer_name).toString();
    QString eliteSterzoSmartName =
        settings.value(QZSettings::elite_sterzo_smart_name, QZSettings::default_elite_sterzo_smart_name).toString();
    bool powerSensorFound =
        powerSensorName.startsWith(QStringLiteral("Disabled")) || power_as_bike || power_as_treadmill;
    bool eliteRizerFound = eliteRizerName.startsWith(QStringLiteral("Disabled"));
    bool eliteSterzoSmartFound = eliteSterzoSmartName.startsWith(QStringLiteral("Disabled"));
    bool fake_bike =
        settings.value(QZSettings::applewatch_fakedevice, QZSettings::default_applewatch_fakedevice).toBool();
    bool fakedevice_elliptical =
        settings.value(QZSettings::fakedevice_elliptical, QZSettings::default_fakedevice_elliptical).toBool();
    bool fakedevice_rower = settings.value(QZSettings::fakedevice_rower, QZSettings::default_fakedevice_rower).toBool();
    bool fakedevice_treadmill =
        settings.value(QZSettings::fakedevice_treadmill, QZSettings::default_fakedevice_treadmill).toBool();
    bool pafers_treadmill = settings.value(QZSettings::pafers_treadmill, QZSettings::default_pafers_treadmill).toBool();
    QString proformtdf4ip = settings.value(QZSettings::proformtdf4ip, QZSettings::default_proformtdf4ip).toString();
    QString proformtdf1ip = settings.value(QZSettings::proformtdf1ip, QZSettings::default_proformtdf1ip).toString();
    QString proformtreadmillip =
        settings.value(QZSettings::proformtreadmillip, QZSettings::default_proformtreadmillip).toString();
    bool antbike_setting =
        settings.value(QZSettings::antbike, QZSettings::default_antbike).toBool();
    QString nordictrack_2950_ip =
        settings.value(QZSettings::nordictrack_2950_ip, QZSettings::default_nordictrack_2950_ip).toString();
    QString tdf_10_ip = settings.value(QZSettings::tdf_10_ip, QZSettings::default_tdf_10_ip).toString();
    QString proform_elliptical_ip = settings.value(QZSettings::proform_elliptical_ip, QZSettings::default_proform_elliptical_ip).toString();
    QString computrainerSerialPort =
        settings.value(QZSettings::computrainer_serialport, QZSettings::default_computrainer_serialport).toString();
    QString csaferowerSerialPort = settings.value(QZSettings::csafe_rower, QZSettings::default_csafe_rower).toString();
    bool manufacturerDeviceFound = false;
    bool ss2k_peloton = settings.value(QZSettings::ss2k_peloton, QZSettings::default_ss2k_peloton).toBool();
    bool pafers_treadmill_bh_iboxster_plus =
        settings
            .value(QZSettings::pafers_treadmill_bh_iboxster_plus, QZSettings::default_pafers_treadmill_bh_iboxster_plus)
            .toBool();
    bool gem_module_inclination =
        settings.value(QZSettings::gem_module_inclination, QZSettings::default_gem_module_inclination).toBool();
    bool iconcept_elliptical =
        settings.value(QZSettings::iconcept_elliptical, QZSettings::default_iconcept_elliptical).toBool();
    bool horizon_treadmill_force_ftms =
        settings.value(QZSettings::horizon_treadmill_force_ftms, QZSettings::default_horizon_treadmill_force_ftms)
            .toBool();
    bool sole_inclination =
        settings.value(QZSettings::sole_treadmill_inclination, QZSettings::default_sole_treadmill_inclination).toBool();
    QString ftms_rower = settings.value(QZSettings::ftms_rower, QZSettings::default_ftms_rower).toString();
    QString ftms_bike = settings.value(QZSettings::ftms_bike, QZSettings::default_ftms_bike).toString();
    QString ftms_treadmill = settings.value(QZSettings::ftms_treadmill, QZSettings::default_ftms_treadmill).toString();
    bool saris_trainer = settings.value(QZSettings::saris_trainer, QZSettings::default_saris_trainer).toBool();    
    bool iconsole_elliptical = settings.value(QZSettings::iconsole_elliptical, QZSettings::default_iconsole_elliptical).toBool();

    if (!heartRateBeltFound) {

        heartRateBeltFound = heartRateBeltAvaiable();
    }
    if (!fitmetriaFanfitFound) {

        fitmetriaFanfitFound = fitmetriaFanfitAvaiable();
    }
    if (!zwiftDeviceFound) {

        zwiftDeviceFound = zwiftDeviceAvaiable();
    }
    if(!sramDeviceFound) {

        sramDeviceFound = sramDeviceAvaiable();
    }
    if (!ftmsAccessoryFound) {

        ftmsAccessoryFound = ftmsAccessoryAvaiable();
    }
    if (ss2k_peloton) {
        ftmsAccessoryFound = true;
    }
    if (!cscFound) {

        cscFound = cscSensorAvaiable();
    }
    if (!powerSensorFound) {

        powerSensorFound = powerSensorAvaiable();
    }
    if (!eliteRizerFound) {

        eliteRizerFound = eliteRizerAvaiable();
    }
    if (!eliteSterzoSmartFound) {

        eliteSterzoSmartFound = eliteSterzoSmartAvaiable();
    }

#ifdef Q_OS_IOS
    // Schwinn bikes on iOS allows to be connected to several instances, so in this way
    // QZ will remember the address and will try to connect to it
    QString b =
        settings.value(QZSettings::bluetooth_lastdevice_name, QZSettings::default_bluetooth_lastdevice_name).toString();
    qDebug() << "last device name (IC BIKE workaround)" << b;
    if (!schwinnIC4Bike &&
        !b.compare(settings.value(QZSettings::filter_device, QZSettings::default_filter_device).toString()) &&
        (b.toUpper().startsWith("IC BIKE") || b.toUpper().startsWith("C7-"))) {

        this->stopDiscovery();
        schwinnIC4Bike = new schwinnic4bike(noWriteResistance, noHeartService);
        // stateFileRead();
        QBluetoothDeviceInfo bt;
        bt.setDeviceUuid(QBluetoothUuid(
            settings.value(QZSettings::bluetooth_lastdevice_address, QZSettings::default_bluetooth_lastdevice_address)
                .toString()));
        // set name method doesn't exist
        emit(deviceConnected(bt));
        connect(schwinnIC4Bike, SIGNAL(connectedAndDiscovered()), this, SLOT(connectedAndDiscovered()));
        // connect(echelonConnectSport, SIGNAL(disconnected()), this, SLOT(restart()));
        connect(schwinnIC4Bike, SIGNAL(debug(QString)), this, SLOT(debug(QString)));
        // connect(echelonConnectSport, SIGNAL(speedChanged(double)), this, SLOT(speedChanged(double)));
        // connect(echelonConnectSport, SIGNAL(inclinationChanged(double)), this, SLOT(inclinationChanged(double)));
        qDebug() << "UUID" << bt.deviceUuid();
        schwinnIC4Bike->deviceDiscovered(bt);
        this->signalBluetoothDeviceConnected(schwinnIC4Bike);
        qDebug() << "connecting directly";
    }
#endif

    QVector<quint16> ids = device.manufacturerIds();
    qDebug() << "manufacturerData" << ids;
    foreach (quint16 id, ids) {
        qDebug() << id << device.manufacturerData(id).toHex(' ');

#ifdef Q_OS_ANDROID
        // yesoul bike on android 13 doesn't send anymore the name
        if (device.name().count() == 0 && id == yesoulbike::manufacturerDataId &&
            device.manufacturerData(id).startsWith(
                QByteArray((const char *)yesoulbike::manufacturerData, yesoulbike::manufacturerDataSize))) {
            qDebug() << "yesoulBikeFromManufacturerData forcing!";
            QBluetoothDeviceInfo manufacturerDevice(device.address(), yesoulbike::bluetoothName,
                                                    device.majorDeviceClass());

            bool found = false;
            QMutableListIterator<QBluetoothDeviceInfo> i(devices);
            while (i.hasNext()) {
                QBluetoothDeviceInfo b = i.next();
                if (SAME_BLUETOOTH_DEVICE(b, manufacturerDevice) && !b.name().isEmpty()) {
                    i.setValue(manufacturerDevice); // in order to keep the freshest copy of this struct
                    found = true;
                    break;
                }
            }
            if (!found) {
                devices.append(manufacturerDevice);
            }
            manufacturerDeviceFound = true;
        }
#endif
    }

    if (manufacturerDeviceFound == false) {
        bool found = false;
        QMutableListIterator<QBluetoothDeviceInfo> i(devices);
        while (i.hasNext()) {
            QBluetoothDeviceInfo b = i.next();
            if (SAME_BLUETOOTH_DEVICE(b, device) && !b.name().isEmpty()) {

                i.setValue(device); // in order to keep the freshest copy of this struct
                found = true;
                break;
            }
        }
        if (!found) {
            devices.append(device);
        }
    }

    emit deviceFound(device.name());
    qDebug() << QStringLiteral("Found new device: ") << device.name() << QStringLiteral(" (") << device.address().toString() <<
          ')' << " " << device.majorDeviceClass() << QStringLiteral(":") << device.minorDeviceClass() << device.serviceUuids()
#if defined(Q_OS_DARWIN) || defined(Q_OS_IOS)
            << device.deviceUuid();
#endif
    ;

    // not required for mobile I guess
#if !defined(Q_OS_ANDROID) && !defined(Q_OS_IOS)
    if(!homeformLoaded) {
        qDebug() << "homeform not yet loaded";
        return;
    }
#endif    

    if (onlyDiscover)
        return;

#ifdef Q_OS_WIN
    if (this->device()) {
        qDebug() << QStringLiteral("bluetooth::finished but discoveryAgent is not active");
        return;
    }
#endif

    bool searchDevices = (heartRateBeltFound && ftmsAccessoryFound && cscFound && powerSensorFound && eliteRizerFound &&
                          eliteSterzoSmartFound && fitmetriaFanfitFound && zwiftDeviceFound) ||
                         forceHeartBeltOffForTimeout;

    if (searchDevices) {
        for (const QBluetoothDeviceInfo &b : qAsConst(devices)) {

            bool filter = true;
            if (!filterDevice.isEmpty() && !filterDevice.startsWith(QStringLiteral("Disabled"))) {

                filter = (b.name().compare(filterDevice, Qt::CaseInsensitive) == 0);
            }
            if (b.name().startsWith(QStringLiteral("M3")) && !m3iBike && filter) {

                if (m3ibike::isCorrectUnit(b)) {
                    this->setLastBluetoothDevice(b);
                    this->stopDiscovery();
                    m3iBike = new m3ibike(noWriteResistance, noHeartService);
                    emit deviceConnected(b);
                    connect(m3iBike, &bluetoothdevice::connectedAndDiscovered, this,
                            &bluetooth::connectedAndDiscovered);
                    // connect(domyosBike, SIGNAL(disconnected()), this, SLOT(restart()));
                    connect(m3iBike, &m3ibike::debug, this, &bluetooth::debug);
                    m3iBike->deviceDiscovered(b);
                    connect(this, &bluetooth::searchingStop, m3iBike, &m3ibike::searchingStop);
                    if (this->discoveryAgent && !this->discoveryAgent->isActive())
                        emit searchingStop();
                    this->signalBluetoothDeviceConnected(m3iBike);
                }
            } else if (fake_bike && !fakeBike) {
                this->stopDiscovery();
                fakeBike = new fakebike(noWriteResistance, noHeartService, false);
                emit deviceConnected(b);
                connect(fakeBike, &bluetoothdevice::connectedAndDiscovered, this, &bluetooth::connectedAndDiscovered);
                connect(fakeBike, &fakebike::inclinationChanged, this, &bluetooth::inclinationChanged);
                connect(fakeBike, &fakebike::debug, this, &bluetooth::debug);
                // connect(cscBike, SIGNAL(disconnected()), this, SLOT(restart()));
                // connect(this, SIGNAL(searchingStop()), fakeBike, SLOT(searchingStop())); //NOTE: Commented due to
                // #358
                if (this->discoveryAgent && !this->discoveryAgent->isActive()) {
                    emit searchingStop();
                }
                this->signalBluetoothDeviceConnected(fakeBike);
            } else if (fakedevice_elliptical && !fakeElliptical) {
                this->stopDiscovery();
                fakeElliptical = new fakeelliptical(noWriteResistance, noHeartService, false);
                emit deviceConnected(b);
                connect(fakeElliptical, &bluetoothdevice::connectedAndDiscovered, this,
                        &bluetooth::connectedAndDiscovered);
                connect(fakeElliptical, &fakeelliptical::inclinationChanged, this, &bluetooth::inclinationChanged);
                connect(fakeElliptical, &fakeelliptical::debug, this, &bluetooth::debug);
                // connect(cscBike, SIGNAL(disconnected()), this, SLOT(restart()));
                // connect(this, SIGNAL(searchingStop()), fakeBike, SLOT(searchingStop())); //NOTE: Commented due to
                // #358
                if (this->discoveryAgent && !this->discoveryAgent->isActive()) {
                    emit searchingStop();
                }
                this->signalBluetoothDeviceConnected(fakeElliptical);
            } else if (fakedevice_rower && !fakeRower) {
                this->stopDiscovery();
                fakeRower = new fakerower(noWriteResistance, noHeartService, false);
                emit deviceConnected(b);
                connect(fakeRower, &bluetoothdevice::connectedAndDiscovered, this, &bluetooth::connectedAndDiscovered);
                connect(fakeRower, &fakerower::inclinationChanged, this, &bluetooth::inclinationChanged);
                connect(fakeRower, &fakerower::debug, this, &bluetooth::debug);
                // connect(cscBike, SIGNAL(disconnected()), this, SLOT(restart()));
                // connect(this, SIGNAL(searchingStop()), fakeBike, SLOT(searchingStop())); //NOTE: Commented due to
                // #358
                if (this->discoveryAgent && !this->discoveryAgent->isActive()) {
                    emit searchingStop();
                }
                this->signalBluetoothDeviceConnected(fakeRower);
            } else if (fakedevice_treadmill && !fakeTreadmill) {
                this->stopDiscovery();
                fakeTreadmill = new faketreadmill(noWriteResistance, noHeartService, false);
                emit deviceConnected(b);
                connect(fakeTreadmill, &bluetoothdevice::connectedAndDiscovered, this,
                        &bluetooth::connectedAndDiscovered);
                connect(fakeTreadmill, &faketreadmill::inclinationChanged, this, &bluetooth::inclinationChanged);
                connect(fakeTreadmill, &faketreadmill::debug, this, &bluetooth::debug);
                // connect(cscBike, SIGNAL(disconnected()), this, SLOT(restart()));
                // connect(this, SIGNAL(searchingStop()), fakeBike, SLOT(searchingStop())); //NOTE: Commented due to
                // #358
                if (this->discoveryAgent && !this->discoveryAgent->isActive()) {
                    emit searchingStop();
                }
                this->signalBluetoothDeviceConnected(fakeTreadmill);

            } else if (!proformtdf4ip.isEmpty() && !proformWifiBike) {
                this->stopDiscovery();
                proformWifiBike =
                    new proformwifibike(noWriteResistance, noHeartService, bikeResistanceOffset, bikeResistanceGain);
                emit deviceConnected(b);
                connect(proformWifiBike, &bluetoothdevice::connectedAndDiscovered, this,
                        &bluetooth::connectedAndDiscovered);
                // connect(cscBike, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(proformWifiBike, &proformwifibike::debug, this, &bluetooth::debug);
                proformWifiBike->deviceDiscovered(b);
                // connect(this, SIGNAL(searchingStop()), cscBike, SLOT(searchingStop())); //NOTE: Commented due to #358
                if (this->discoveryAgent && !this->discoveryAgent->isActive()) {
                    emit searchingStop();
                }
                this->signalBluetoothDeviceConnected(proformWifiBike);
            } else if (!proformtdf1ip.isEmpty() && !proformTelnetBike) {
                this->stopDiscovery();
                proformTelnetBike =
                    new proformtelnetbike(noWriteResistance, noHeartService, bikeResistanceOffset, bikeResistanceGain);
                emit deviceConnected(b);
                connect(proformTelnetBike, &bluetoothdevice::connectedAndDiscovered, this,
                        &bluetooth::connectedAndDiscovered);
                // connect(cscBike, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(proformTelnetBike, &proformtelnetbike::debug, this, &bluetooth::debug);
                proformTelnetBike->deviceDiscovered(b);
                // connect(this, SIGNAL(searchingStop()), cscBike, SLOT(searchingStop())); //NOTE: Commented due to #358
                if (this->discoveryAgent && !this->discoveryAgent->isActive()) {
                    emit searchingStop();
                }
                this->signalBluetoothDeviceConnected(proformTelnetBike);                
#ifndef Q_OS_IOS
            } else if (!computrainerSerialPort.isEmpty() && !computrainerBike) {
                this->stopDiscovery();
                computrainerBike =
                    new computrainerbike(noWriteResistance, noHeartService, bikeResistanceOffset, bikeResistanceGain);
                emit deviceConnected(b);
                connect(computrainerBike, &bluetoothdevice::connectedAndDiscovered, this,
                        &bluetooth::connectedAndDiscovered);
                // connect(cscBike, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(computrainerBike, &computrainerbike::debug, this, &bluetooth::debug);
                computrainerBike->deviceDiscovered(b);
                // connect(this, SIGNAL(searchingStop()), cscBike, SLOT(searchingStop())); //NOTE: Commented due to #358
                if (this->discoveryAgent && !this->discoveryAgent->isActive()) {
                    emit searchingStop();
                }
                this->signalBluetoothDeviceConnected(computrainerBike);
            } else if (!csaferowerSerialPort.isEmpty() && !csafeRower) {
                this->stopDiscovery();
                csafeRower = new csaferower(noWriteResistance, noHeartService, false);
                emit deviceConnected(b);
                connect(csafeRower, &bluetoothdevice::connectedAndDiscovered, this, &bluetooth::connectedAndDiscovered);
                // connect(cscBike, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(csafeRower, &csaferower::debug, this, &bluetooth::debug);
                csafeRower->deviceDiscovered(b);
                // connect(this, SIGNAL(searchingStop()), cscBike, SLOT(searchingStop())); //NOTE: Commented due to #358
                if (this->discoveryAgent && !this->discoveryAgent->isActive()) {
                    emit searchingStop();
                }
                this->signalBluetoothDeviceConnected(csafeRower);
#endif
            } else if (antbike_setting && !antBike) {
                this->stopDiscovery();
                antBike = new antbike(noWriteResistance, noHeartService, false);
                emit deviceConnected(b);
                connect(antBike, &bluetoothdevice::connectedAndDiscovered, this,
                        &bluetooth::connectedAndDiscovered);
                // connect(cscBike, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(antBike, &antbike::debug, this, &bluetooth::debug);
                // connect(this, SIGNAL(searchingStop()), cscBike, SLOT(searchingStop())); //NOTE: Commented due to #358
                if (this->discoveryAgent && !this->discoveryAgent->isActive()) {
                    emit searchingStop();
                }
                this->signalBluetoothDeviceConnected(antBike);
            } else if (!proformtreadmillip.isEmpty() && !proformWifiTreadmill) {
                this->stopDiscovery();
                proformWifiTreadmill = new proformwifitreadmill(noWriteResistance, noHeartService, bikeResistanceOffset,
                                                                bikeResistanceGain);
                emit deviceConnected(b);
                connect(proformWifiTreadmill, &bluetoothdevice::connectedAndDiscovered, this,
                        &bluetooth::connectedAndDiscovered);
                // connect(cscBike, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(proformWifiTreadmill, &proformwifitreadmill::debug, this, &bluetooth::debug);
                proformWifiTreadmill->deviceDiscovered(b);
                // connect(this, SIGNAL(searchingStop()), cscBike, SLOT(searchingStop())); //NOTE: Commented due to #358
                if (this->discoveryAgent && !this->discoveryAgent->isActive()) {
                    emit searchingStop();
                }
                this->signalBluetoothDeviceConnected(proformWifiTreadmill);
            } else if (!nordictrack_2950_ip.isEmpty() && !nordictrackifitadbTreadmill) {
                this->stopDiscovery();
                nordictrackifitadbTreadmill = new nordictrackifitadbtreadmill(noWriteResistance, noHeartService);
                emit deviceConnected(b);
                connect(nordictrackifitadbTreadmill, &bluetoothdevice::connectedAndDiscovered, this,
                        &bluetooth::connectedAndDiscovered);
                connect(nordictrackifitadbTreadmill, &nordictrackifitadbtreadmill::debug, this, &bluetooth::debug);
                // nordictrackifitadbTreadmill->deviceDiscovered(b);
                // connect(this, SIGNAL(searchingStop()), cscBike, SLOT(searchingStop())); //NOTE: Commented due to #358
                if (this->discoveryAgent && !this->discoveryAgent->isActive()) {
                    emit searchingStop();
                }
                this->signalBluetoothDeviceConnected(nordictrackifitadbTreadmill);
            } else if (!tdf_10_ip.isEmpty() && !nordictrackifitadbBike) {
                this->stopDiscovery();
                nordictrackifitadbBike = new nordictrackifitadbbike(noWriteResistance, noHeartService,
                                                                    bikeResistanceOffset, bikeResistanceGain);
                emit deviceConnected(b);
                connect(nordictrackifitadbBike, &bluetoothdevice::connectedAndDiscovered, this,
                        &bluetooth::connectedAndDiscovered);
                connect(nordictrackifitadbBike, &nordictrackifitadbbike::debug, this, &bluetooth::debug);
                // nordictrackifitadbTreadmill->deviceDiscovered(b);
                // connect(this, SIGNAL(searchingStop()), cscBike, SLOT(searchingStop())); //NOTE: Commented due to #358
                if (this->discoveryAgent && !this->discoveryAgent->isActive()) {
                    emit searchingStop();
                }
                this->signalBluetoothDeviceConnected(nordictrackifitadbBike);
            } else if (!proform_elliptical_ip.isEmpty() && !nordictrackifitadbElliptical) {
                this->stopDiscovery();
                nordictrackifitadbElliptical = new nordictrackifitadbelliptical(noWriteResistance, noHeartService,
                                                                    bikeResistanceOffset, bikeResistanceGain);
                emit deviceConnected(b);
                connect(nordictrackifitadbElliptical, &bluetoothdevice::connectedAndDiscovered, this,
                        &bluetooth::connectedAndDiscovered);
                connect(nordictrackifitadbElliptical, &nordictrackifitadbelliptical::debug, this, &bluetooth::debug);
                // nordictrackifitadbTreadmill->deviceDiscovered(b);
                // connect(this, SIGNAL(searchingStop()), cscBike, SLOT(searchingStop())); //NOTE: Commented due to #358
                if (this->discoveryAgent && !this->discoveryAgent->isActive()) {
                    emit searchingStop();
                }
                this->signalBluetoothDeviceConnected(nordictrackifitadbElliptical);
            } else if (((csc_as_bike && b.name().startsWith(cscName)) ||
                        b.name().toUpper().startsWith(QStringLiteral("JOROTO-BK-"))) &&
                       !cscBike && filter) {
                this->setLastBluetoothDevice(b);
                this->stopDiscovery();
                cscBike = new cscbike(noWriteResistance, noHeartService, false);
                emit deviceConnected(b);
                connect(cscBike, &bluetoothdevice::connectedAndDiscovered, this, &bluetooth::connectedAndDiscovered);
                // connect(cscBike, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(cscBike, &cscbike::debug, this, &bluetooth::debug);
                cscBike->deviceDiscovered(b);
                // connect(this, SIGNAL(searchingStop()), cscBike, SLOT(searchingStop())); //NOTE: Commented due to #358
                if (this->discoveryAgent && !this->discoveryAgent->isActive()) {
                    emit searchingStop();
                }
                this->signalBluetoothDeviceConnected(cscBike);
            } else if (power_as_bike && b.name().startsWith(powerSensorName) && !powerBike && filter) {
                this->setLastBluetoothDevice(b);
                this->stopDiscovery();
                powerBike = new stagesbike(noWriteResistance, noHeartService, false);
                emit deviceConnected(b);
                connect(powerBike, &bluetoothdevice::connectedAndDiscovered, this, &bluetooth::connectedAndDiscovered);
                // connect(cscBike, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(powerBike, &stagesbike::debug, this, &bluetooth::debug);
                powerBike->deviceDiscovered(b);
                // connect(this, SIGNAL(searchingStop()), cscBike, SLOT(searchingStop())); //NOTE: Commented due to #358
                if (this->discoveryAgent && !this->discoveryAgent->isActive()) {
                    emit searchingStop();
                }
                this->signalBluetoothDeviceConnected(powerBike);
            } else if ((((power_as_treadmill && b.name().startsWith(powerSensorName))) ||
                        (b.name().toUpper().startsWith(QStringLiteral("TREADMILL")) && (deviceHasService(b, QBluetoothUuid((quint16)0x1814)))) ||
                        (b.name().toUpper().startsWith(QStringLiteral("S10")) && deviceHasService(b, QBluetoothUuid((quint16)0x1814))) ||
                        b.name().toUpper().startsWith(QStringLiteral("ZWIFT RUNPOD"))) &&
                       !powerTreadmill && filter) {
                this->setLastBluetoothDevice(b);
                this->stopDiscovery();
                powerTreadmill = new strydrunpowersensor(noWriteResistance, noHeartService, false);
                emit deviceConnected(b);
                connect(powerTreadmill, &bluetoothdevice::connectedAndDiscovered, this,
                        &bluetooth::connectedAndDiscovered);
                // connect(cscBike, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(powerTreadmill, &strydrunpowersensor::debug, this, &bluetooth::debug);
                powerTreadmill->deviceDiscovered(b);
                // connect(this, SIGNAL(searchingStop()), cscBike, SLOT(searchingStop())); //NOTE: Commented due to #358
                if (this->discoveryAgent && !this->discoveryAgent->isActive()) {
                    emit searchingStop();
                }
                this->signalBluetoothDeviceConnected(powerTreadmill);
            } else if (b.name().toUpper().startsWith(QStringLiteral("DOMYOS-ROW")) &&
                       !b.name().startsWith(QStringLiteral("DomyosBridge")) && !domyosRower && filter) {
                this->setLastBluetoothDevice(b);
                this->stopDiscovery();
                domyosRower = new domyosrower(noWriteResistance, noHeartService, testResistance, bikeResistanceOffset,
                                              bikeResistanceGain);
                emit deviceConnected(b);
                connect(domyosRower, &bluetoothdevice::connectedAndDiscovered, this,
                        &bluetooth::connectedAndDiscovered);
                // connect(domyosBike, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(domyosRower, &domyosrower::debug, this, &bluetooth::debug);
                domyosRower->deviceDiscovered(b);
                connect(this, &bluetooth::searchingStop, domyosRower, &domyosrower::searchingStop);
                if (this->discoveryAgent && !this->discoveryAgent->isActive()) {
                    emit searchingStop();
                }
                this->signalBluetoothDeviceConnected(domyosRower);
            } else if ((b.name().startsWith(QStringLiteral("Domyos-Bike")) && (!deviceHasService(b, QBluetoothUuid((quint16)0x1826)) || settings.value(QZSettings::domyosbike_notfmts, QZSettings::default_domyosbike_notfmts).toBool())) &&
                       !b.name().startsWith(QStringLiteral("DomyosBridge")) && !domyosBike && filter) {
                this->setLastBluetoothDevice(b);
                this->stopDiscovery();
                domyosBike = new domyosbike(noWriteResistance, noHeartService, testResistance, bikeResistanceOffset,
                                            bikeResistanceGain);
                emit deviceConnected(b);
                connect(domyosBike, &bluetoothdevice::connectedAndDiscovered, this, &bluetooth::connectedAndDiscovered);
                // connect(domyosBike, SIGNAL(disconnected()), this, SLOT(restart()));
                // connect(domyosBike, SIGNAL(debug(QString)), this, SLOT(debug(QString)));//NOTE: Commented due to #358
                domyosBike->deviceDiscovered(b);
                connect(this, &bluetooth::searchingStop, domyosBike, &domyosbike::searchingStop);
                if (this->discoveryAgent && !this->discoveryAgent->isActive()) {
                    emit searchingStop();
                }
                this->signalBluetoothDeviceConnected(domyosBike);
            } else if ((b.name().toUpper().startsWith(QStringLiteral("FAL-SPORTS")) ||
                       (b.name().toUpper().startsWith(QStringLiteral("I-CONSOLE+")) && iconsole_elliptical)) &&
                       !trxappgateusbElliptical && ftms_bike.contains(QZSettings::default_ftms_bike) && filter) {
                this->setLastBluetoothDevice(b);
                this->stopDiscovery();
                trxappgateusbElliptical = new trxappgateusbelliptical(noWriteResistance, noHeartService,
                                                        bikeResistanceOffset, bikeResistanceGain);
                emit deviceConnected(b);
                connect(trxappgateusbElliptical, &bluetoothdevice::connectedAndDiscovered, this,
                        &bluetooth::connectedAndDiscovered);
                // connect(domyosElliptical, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(trxappgateusbElliptical, &trxappgateusbelliptical::debug, this, &bluetooth::debug);
                trxappgateusbElliptical->deviceDiscovered(b);
                connect(this, &bluetooth::searchingStop, trxappgateusbElliptical, &trxappgateusbelliptical::searchingStop);
                if (this->discoveryAgent && !this->discoveryAgent->isActive()) {
                    emit searchingStop();
                }
                this->signalBluetoothDeviceConnected(trxappgateusbElliptical);
            } else if (b.name().startsWith(QStringLiteral("Domyos-EL")) &&
                       !b.name().startsWith(QStringLiteral("DomyosBridge")) && !domyosElliptical && filter) {
                this->setLastBluetoothDevice(b);
                this->stopDiscovery();
                domyosElliptical = new domyoselliptical(noWriteResistance, noHeartService, testResistance,
                                                        bikeResistanceOffset, bikeResistanceGain);
                emit deviceConnected(b);
                connect(domyosElliptical, &bluetoothdevice::connectedAndDiscovered, this,
                        &bluetooth::connectedAndDiscovered);
                // connect(domyosElliptical, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(domyosElliptical, &domyoselliptical::debug, this, &bluetooth::debug);
                domyosElliptical->deviceDiscovered(b);
                connect(this, &bluetooth::searchingStop, domyosElliptical, &domyoselliptical::searchingStop);
                if (this->discoveryAgent && !this->discoveryAgent->isActive()) {
                    emit searchingStop();
                }
                this->signalBluetoothDeviceConnected(domyosElliptical);
            } else if ((b.name().toUpper().startsWith(QStringLiteral("YPOO-U3-")) ||
                        b.name().toUpper().startsWith(QStringLiteral("SCH_590E")) ||
                        b.name().toUpper().startsWith(QStringLiteral("KETTLER ")) ||
                        (b.name().toUpper().startsWith(QStringLiteral("E35")) && deviceHasService(b, QBluetoothUuid((quint16)0x1826))) ||
                        (b.name().startsWith(QStringLiteral("FS-")) && iconsole_elliptical)) && !ypooElliptical && filter) {
                this->setLastBluetoothDevice(b);
                this->stopDiscovery();
                ypooElliptical =
                    new ypooelliptical(noWriteResistance, noHeartService, bikeResistanceOffset, bikeResistanceGain);
                emit deviceConnected(b);
                connect(ypooElliptical, &bluetoothdevice::connectedAndDiscovered, this,
                        &bluetooth::connectedAndDiscovered);
                // connect(domyosElliptical, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(ypooElliptical, &ypooelliptical::debug, this, &bluetooth::debug);
                ypooElliptical->deviceDiscovered(b);
                // connect(this, &bluetooth::searchingStop, ypooElliptical, &ypooelliptical::searchingStop);
                if (this->discoveryAgent && !this->discoveryAgent->isActive()) {
                    emit searchingStop();
                }
                this->signalBluetoothDeviceConnected(ypooElliptical);
            } else if ((b.name().toUpper().startsWith(QStringLiteral("NAUTILUS E")) || 
                        b.name().toUpper().startsWith(QStringLiteral("NAUTILUS M"))) &&
                       !nautilusElliptical && // NAUTILUS E616
                       filter) {
                this->setLastBluetoothDevice(b);
                this->stopDiscovery();
                nautilusElliptical = new nautiluselliptical(noWriteResistance, noHeartService, testResistance,
                                                            bikeResistanceOffset, bikeResistanceGain);
                emit deviceConnected(b);
                connect(nautilusElliptical, &bluetoothdevice::connectedAndDiscovered, this,
                        &bluetooth::connectedAndDiscovered);
                // connect(nautilusElliptical, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(nautilusElliptical, &nautiluselliptical::debug, this, &bluetooth::debug);
                nautilusElliptical->deviceDiscovered(b);
                connect(this, &bluetooth::searchingStop, nautilusElliptical, &nautiluselliptical::searchingStop);
                if (this->discoveryAgent && !this->discoveryAgent->isActive())
                    emit searchingStop();
                this->signalBluetoothDeviceConnected(nautilusElliptical);
            } else if ((b.name().toUpper().startsWith(QStringLiteral("NAUTILUS B"))) && !nautilusBike &&
                       filter) { // NAUTILUS B628
                this->setLastBluetoothDevice(b);
                this->stopDiscovery();
                nautilusBike = new nautilusbike(noWriteResistance, noHeartService, testResistance, bikeResistanceOffset,
                                                bikeResistanceGain);
                emit deviceConnected(b);
                connect(nautilusBike, &bluetoothdevice::connectedAndDiscovered, this,
                        &bluetooth::connectedAndDiscovered);
                // connect(nautilusBike, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(nautilusBike, &nautilusbike::debug, this, &bluetooth::debug);
                nautilusBike->deviceDiscovered(b);
                connect(this, &bluetooth::searchingStop, nautilusBike, &nautilusbike::searchingStop);
                if (this->discoveryAgent && !this->discoveryAgent->isActive())
                    emit searchingStop();
                this->signalBluetoothDeviceConnected(nautilusBike);
            } else if ((b.name().toUpper().startsWith(QStringLiteral("I_FS"))) && !proformElliptical && filter) {
                this->setLastBluetoothDevice(b);
                this->stopDiscovery();
                proformElliptical = new proformelliptical(noWriteResistance, noHeartService);
                emit deviceConnected(b);
                connect(proformElliptical, &bluetoothdevice::connectedAndDiscovered, this,
                        &bluetooth::connectedAndDiscovered);
                // connect(proformElliptical, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(proformElliptical, &proformelliptical::debug, this, &bluetooth::debug);
                proformElliptical->deviceDiscovered(b);
                // connect(this, &bluetooth::searchingStop, proformElliptical, &proformelliptical::searchingStop);
                if (this->discoveryAgent && !this->discoveryAgent->isActive())
                    emit searchingStop();
                this->signalBluetoothDeviceConnected(proformElliptical);
            } else if ((b.name().toUpper().startsWith(QStringLiteral("I_EL"))) && !nordictrackElliptical && filter) {
                this->setLastBluetoothDevice(b);
                this->stopDiscovery();
                nordictrackElliptical = new nordictrackelliptical(noWriteResistance, noHeartService,
                                                                  bikeResistanceOffset, bikeResistanceGain);
                emit deviceConnected(b);
                connect(nordictrackElliptical, &bluetoothdevice::connectedAndDiscovered, this,
                        &bluetooth::connectedAndDiscovered);
                // connect(nordictrackElliptical, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(nordictrackElliptical, &nordictrackelliptical::debug, this, &bluetooth::debug);
                nordictrackElliptical->deviceDiscovered(b);
                // connect(this, &bluetooth::searchingStop, proformElliptical, &proformelliptical::searchingStop);
                if (this->discoveryAgent && !this->discoveryAgent->isActive())
                    emit searchingStop();
                this->signalBluetoothDeviceConnected(nordictrackElliptical);

            } else if ((b.name().toUpper().startsWith(QStringLiteral("I_VE"))) && !proformEllipticalTrainer && filter) {
                this->setLastBluetoothDevice(b);
                this->stopDiscovery();
                proformEllipticalTrainer = new proformellipticaltrainer(noWriteResistance, noHeartService,
                                                                        bikeResistanceOffset, bikeResistanceGain);
                emit deviceConnected(b);
                connect(proformEllipticalTrainer, &bluetoothdevice::connectedAndDiscovered, this,
                        &bluetooth::connectedAndDiscovered);
                // connect(proformEllipticalTrainer, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(proformEllipticalTrainer, &proformellipticaltrainer::debug, this, &bluetooth::debug);
                proformEllipticalTrainer->deviceDiscovered(b);
                // connect(this, &bluetooth::searchingStop, proformEllipticalTrainer,
                // &proformellipticaltrainer::searchingStop);
                if (this->discoveryAgent && !this->discoveryAgent->isActive())
                    emit searchingStop();
                this->signalBluetoothDeviceConnected(proformEllipticalTrainer);
            } else if ((b.name().toUpper().startsWith(QStringLiteral("I_RW"))) && !proformRower && filter) {
                this->setLastBluetoothDevice(b);
                this->stopDiscovery();
                proformRower = new proformrower(noWriteResistance, noHeartService);
                emit deviceConnected(b);
                connect(proformRower, &bluetoothdevice::connectedAndDiscovered, this,
                        &bluetooth::connectedAndDiscovered);
                // connect(proformRower, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(proformRower, &proformrower::debug, this, &bluetooth::debug);
                proformRower->deviceDiscovered(b);
                // connect(this, &bluetooth::searchingStop, proformElliptical, &proformelliptical::searchingStop);
                if (this->discoveryAgent && !this->discoveryAgent->isActive())
                    emit searchingStop();
                this->signalBluetoothDeviceConnected(proformRower);
            } else if ((b.name().toUpper().startsWith(QStringLiteral("B01_"))) && ftms_bike.contains(QZSettings::default_ftms_bike) && !bhFitnessElliptical && filter) {
                this->setLastBluetoothDevice(b);
                this->stopDiscovery();
                bhFitnessElliptical = new bhfitnesselliptical(noWriteResistance, noHeartService, bikeResistanceOffset,
                                                              bikeResistanceGain);
                emit deviceConnected(b);
                connect(bhFitnessElliptical, &bluetoothdevice::connectedAndDiscovered, this,
                        &bluetooth::connectedAndDiscovered);
                // connect(bhFitnessElliptical, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(bhFitnessElliptical, &bhfitnesselliptical::debug, this, &bluetooth::debug);
                bhFitnessElliptical->deviceDiscovered(b);
                // connect(this, &bluetooth::searchingStop, bhFitnessElliptical, &bhfitnesselliptical::searchingStop);
                if (this->discoveryAgent && !this->discoveryAgent->isActive())
                    emit searchingStop();
                this->signalBluetoothDeviceConnected(bhFitnessElliptical);
            } else if ((b.name().toUpper().startsWith(QStringLiteral("E95S")) ||
                        b.name().toUpper().startsWith(QStringLiteral("E25")) ||
                        (b.name().toUpper().startsWith(QStringLiteral("E35")) && !deviceHasService(b, QBluetoothUuid((quint16)0x1826))) ||
                        b.name().toUpper().startsWith(QStringLiteral("E55")) ||
                        b.name().toUpper().startsWith(QStringLiteral("E95")) ||
                        b.name().toUpper().startsWith(QStringLiteral("E98")) ||
                        b.name().toUpper().startsWith(QStringLiteral("XG400")) ||
                        b.name().toUpper().startsWith(QStringLiteral("E98S"))) &&
                       !soleElliptical && filter) {
                this->setLastBluetoothDevice(b);
                this->stopDiscovery();
                soleElliptical = new soleelliptical(noWriteResistance, noHeartService, testResistance,
                                                    bikeResistanceOffset, bikeResistanceGain);
                emit deviceConnected(b);
                connect(soleElliptical, &bluetoothdevice::connectedAndDiscovered, this,
                        &bluetooth::connectedAndDiscovered);
                // connect(soleElliptical, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(soleElliptical, &soleelliptical::debug, this, &bluetooth::debug);
                soleElliptical->deviceDiscovered(b);
                connect(this, &bluetooth::searchingStop, soleElliptical, &soleelliptical::searchingStop);
                if (this->discoveryAgent && !this->discoveryAgent->isActive())
                    emit searchingStop();
                this->signalBluetoothDeviceConnected(soleElliptical);
            } else if (b.name().startsWith(QStringLiteral("Domyos")) &&
                       !b.name().startsWith(QStringLiteral("DomyosBr")) &&
                       !b.name().toUpper().startsWith(QStringLiteral("DOMYOS-BIKING-")) && !domyos && !domyosElliptical && b.name().compare(ftms_treadmill, Qt::CaseInsensitive) &&
                       !domyosBike && !domyosRower && !ftmsBike && !horizonTreadmill &&
                       (!deviceHasService(b, QBluetoothUuid((quint16)0x1826)) || settings.value(QZSettings::domyostreadmill_notfmts, QZSettings::default_domyostreadmill_notfmts).toBool()) &&
                       filter) {
                this->setLastBluetoothDevice(b);
                this->stopDiscovery();
                domyos = new domyostreadmill(this->pollDeviceTime, noConsole, noHeartService);
#if !defined(Q_OS_ANDROID) && !defined(Q_OS_IOS)
                stateFileRead();
#endif
                emit deviceConnected(b);
                connect(domyos, &bluetoothdevice::connectedAndDiscovered, this, &bluetooth::connectedAndDiscovered);
                // connect(domyos, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(domyos, &domyostreadmill::debug, this, &bluetooth::debug);
                connect(domyos, &domyostreadmill::speedChanged, this, &bluetooth::speedChanged);
                connect(domyos, &domyostreadmill::inclinationChanged, this, &bluetooth::inclinationChanged);
                domyos->deviceDiscovered(b);
                connect(this, &bluetooth::searchingStop, domyos, &domyostreadmill::searchingStop);
                if (this->discoveryAgent && !this->discoveryAgent->isActive())
                    emit searchingStop();
                this->signalBluetoothDeviceConnected(domyos);
            } else if ((
                           // Xiaomi k12 pro treadmill KS-ST-K12PRO
                           b.name().toUpper().startsWith(QStringLiteral("KS-ST-K12PRO")) ||
                           // KingSmith Walking Pad R2
                           b.name().toUpper().startsWith(QStringLiteral("KS-R1AC")) ||
                           b.name().toUpper().startsWith(QStringLiteral("KS-HC-R1AA")) ||
                           b.name().toUpper().startsWith(QStringLiteral("KS-HC-R1AC")) ||
                           // KingSmith Walking Pad X21
                           b.name().toUpper().startsWith(QStringLiteral("KS-X21")) ||
                           b.name().toUpper().startsWith(QStringLiteral("KS-HDSC-X21C")) ||
                           b.name().toUpper().startsWith(QStringLiteral("KS-HDSY-X21C")) ||
                           b.name().toUpper().startsWith(QStringLiteral("KS-NACH-X21C")) ||
                           b.name().toUpper().startsWith(QStringLiteral("KS-NGCH-X21C")) ||

                           // X23 King Smith
                           b.name().toUpper().startsWith(QStringLiteral("KS-NACH-MXG")) ||

                           // KingSmith Walking Pad G1
                           b.name().toUpper().startsWith(QStringLiteral("KS-NGCH-G1C"))) &&
                       !kingsmithR2Treadmill && filter) {
                this->setLastBluetoothDevice(b);
                this->stopDiscovery();
                kingsmithR2Treadmill = new kingsmithr2treadmill(this->pollDeviceTime, noConsole, noHeartService);
#if !defined(Q_OS_ANDROID) && !defined(Q_OS_IOS)
                stateFileRead();
#endif
                emit deviceConnected(b);
                connect(kingsmithR2Treadmill, &bluetoothdevice::connectedAndDiscovered, this,
                        &bluetooth::connectedAndDiscovered);
                // connect(kingsmithR2Treadmill, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(kingsmithR2Treadmill, &kingsmithr2treadmill::debug, this, &bluetooth::debug);
                connect(kingsmithR2Treadmill, &kingsmithr2treadmill::speedChanged, this, &bluetooth::speedChanged);
                connect(kingsmithR2Treadmill, &kingsmithr2treadmill::inclinationChanged, this,
                        &bluetooth::inclinationChanged);
                kingsmithR2Treadmill->deviceDiscovered(b);
                connect(this, &bluetooth::searchingStop, kingsmithR2Treadmill, &kingsmithr2treadmill::searchingStop);
                if (this->discoveryAgent && !this->discoveryAgent->isActive())
                    emit searchingStop();
                this->signalBluetoothDeviceConnected(kingsmithR2Treadmill);
            } else if ((b.name().toUpper().startsWith(QStringLiteral("R1 PRO")) ||
                        b.name().toUpper().startsWith(QStringLiteral("KINGSMITH")) ||
                        b.name().toUpper().startsWith(QStringLiteral("DYNAMAX")) ||
                        b.name().toUpper().startsWith(QStringLiteral("WALKINGPAD")) ||
                        b.name().toUpper().startsWith(QStringLiteral("KS-ST-A1P")) ||  // KingSmith Walkingpad A1 Pro #2041
                        // Poland-distributed WalkingPad R2 TRR2FB
                        b.name().toUpper().startsWith(QStringLiteral("KS-SC-BLR2C")) ||                        
                        !b.name().toUpper().compare(QStringLiteral("RE")) || // just "RE"
                        b.name().toUpper().startsWith(QStringLiteral("KS-H")) ||
                        b.name().toUpper().startsWith(QStringLiteral("KS-F0")) ||
                        b.name().toUpper().startsWith(QStringLiteral("KS-BLC")) || // Walkingpad C2 #1672
                        b.name().toUpper().startsWith(
                            QStringLiteral("KS-BLR"))) && // Treadmill KingSmith WalkingPad R2 Pro KS-HCR1AA
                       !(b.name().toUpper().startsWith(QStringLiteral("KS-HD-Z1D"))) && // it's an FTMS one
                       !kingsmithR1ProTreadmill &&
                       !kingsmithR2Treadmill && filter) {
                this->setLastBluetoothDevice(b);
                this->stopDiscovery();
                kingsmithR1ProTreadmill = new kingsmithr1protreadmill(this->pollDeviceTime, noConsole, noHeartService);
#if !defined(Q_OS_ANDROID) && !defined(Q_OS_IOS)
                stateFileRead();
#endif
                emit deviceConnected(b);
                connect(kingsmithR1ProTreadmill, &bluetoothdevice::connectedAndDiscovered, this,
                        &bluetooth::connectedAndDiscovered);
                // connect(kingsmithR1ProTreadmill, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(kingsmithR1ProTreadmill, &kingsmithr1protreadmill::debug, this, &bluetooth::debug);
                connect(kingsmithR1ProTreadmill, &kingsmithr1protreadmill::speedChanged, this,
                        &bluetooth::speedChanged);
                connect(kingsmithR1ProTreadmill, &kingsmithr1protreadmill::inclinationChanged, this,
                        &bluetooth::inclinationChanged);
                kingsmithR1ProTreadmill->deviceDiscovered(b);
                connect(this, &bluetooth::searchingStop, kingsmithR1ProTreadmill,
                        &kingsmithr1protreadmill::searchingStop);
                if (this->discoveryAgent && !this->discoveryAgent->isActive())
                    emit searchingStop();
                this->signalBluetoothDeviceConnected(kingsmithR1ProTreadmill);
            } else if ((b.name().toUpper().startsWith(QStringLiteral("ZW-"))) && !shuaA5Treadmill && ftms_bike.contains(QZSettings::default_ftms_bike) && filter) {
                this->setLastBluetoothDevice(b);
                this->stopDiscovery();
                shuaA5Treadmill = new shuaa5treadmill(noWriteResistance, noHeartService);
#if !defined(Q_OS_ANDROID) && !defined(Q_OS_IOS)
                stateFileRead();
#endif
                emit deviceConnected(b);
                connect(shuaA5Treadmill, &bluetoothdevice::connectedAndDiscovered, this,
                        &bluetooth::connectedAndDiscovered);
                // connect(shuaA5Treadmill, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(shuaA5Treadmill, &shuaa5treadmill::debug, this, &bluetooth::debug);
                connect(shuaA5Treadmill, &shuaa5treadmill::speedChanged, this, &bluetooth::speedChanged);
                connect(shuaA5Treadmill, &shuaa5treadmill::inclinationChanged, this, &bluetooth::inclinationChanged);
                shuaA5Treadmill->deviceDiscovered(b);
                if (this->discoveryAgent && !this->discoveryAgent->isActive())
                    emit searchingStop();
                this->signalBluetoothDeviceConnected(shuaA5Treadmill);
            } else if ((b.name().toUpper().startsWith(QStringLiteral("TRUE")) ||
                        b.name().toUpper().startsWith(QStringLiteral("ASSAULT TREADMILL ")) ||
                        (b.name().toUpper().startsWith(QStringLiteral("WDWAY")) && b.name().length() == 8) || // WdWay179
                        (b.name().toUpper().startsWith(QStringLiteral("TREADMILL")) && !gem_module_inclination && !deviceHasService(b, QBluetoothUuid((quint16)0x1814)) && !deviceHasService(b, QBluetoothUuid((quint16)0x1826)))) &&
                       !trueTreadmill && filter) {
                this->setLastBluetoothDevice(b);
                this->stopDiscovery();
                trueTreadmill = new truetreadmill(noWriteResistance, noHeartService);
#if !defined(Q_OS_ANDROID) && !defined(Q_OS_IOS)
                stateFileRead();
#endif
                emit deviceConnected(b);
                connect(trueTreadmill, &bluetoothdevice::connectedAndDiscovered, this,
                        &bluetooth::connectedAndDiscovered);
                // connect(shuaA5Treadmill, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(trueTreadmill, &truetreadmill::debug, this, &bluetooth::debug);
                connect(trueTreadmill, &truetreadmill::speedChanged, this, &bluetooth::speedChanged);
                connect(trueTreadmill, &truetreadmill::inclinationChanged, this, &bluetooth::inclinationChanged);
                trueTreadmill->deviceDiscovered(b);
                if (this->discoveryAgent && !this->discoveryAgent->isActive())
                    emit searchingStop();
                this->signalBluetoothDeviceConnected(trueTreadmill);
            } else if (((b.name().toUpper().startsWith(QStringLiteral("F80")) && sole_inclination) ||
                        (b.name().toUpper().startsWith(QStringLiteral("F89")) && sole_inclination) ||
                        b.name().toUpper().startsWith(QStringLiteral("F65")) ||
                        (b.name().toUpper().startsWith(QStringLiteral("TT8")) && !deviceHasService(b, QBluetoothUuid((quint16)0x1826))) ||
                        b.name().toUpper().startsWith(QStringLiteral("F63")) ||
                        (b.name().toUpper().startsWith(QStringLiteral("ST90")) && !deviceHasService(b, QBluetoothUuid((quint16)0x1826))) ||
                        b.name().toUpper().startsWith(QStringLiteral("TRX7.5")) ||
                        b.name().toUpper().startsWith(QStringLiteral("S77")) ||
                        (b.name().toUpper().startsWith(QStringLiteral("F85")) && sole_inclination)) &&
                       !soleF80 && filter) {
                this->setLastBluetoothDevice(b);
                this->stopDiscovery();
                soleF80 = new solef80treadmill(noWriteResistance, noHeartService);
#if !defined(Q_OS_ANDROID) && !defined(Q_OS_IOS)
                stateFileRead();
#endif
                emit deviceConnected(b);
                connect(soleF80, &bluetoothdevice::connectedAndDiscovered, this, &bluetooth::connectedAndDiscovered);
                // connect(soleF80, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(soleF80, &solef80treadmill::debug, this, &bluetooth::debug);
                // NOTE: Commented due to #358
                // connect(soleF80, SIGNAL(speedChanged(double)), this, SLOT(speedChanged(double)));
                // NOTE: Commented due to #358
                // connect(soleF80, SIGNAL(inclinationChanged(double)), this,
                // SLOT(inclinationChanged(double)));
                soleF80->deviceDiscovered(b);
                // NOTE: Commented due to #358
                // connect(this, SIGNAL(searchingStop()), horizonTreadmill, SLOT(searchingStop()));
                if (this->discoveryAgent && !this->discoveryAgent->isActive()) {
                    emit searchingStop();
                }
                this->signalBluetoothDeviceConnected(soleF80);
            } else if ((b.name().toUpper().startsWith(QStringLiteral("LF")) && b.name().length() == 18) &&
                       !lifefitnessTreadmill && filter) {
                this->setLastBluetoothDevice(b);
                this->stopDiscovery();
                lifefitnessTreadmill = new lifefitnesstreadmill(noWriteResistance, noHeartService);
#if !defined(Q_OS_ANDROID) && !defined(Q_OS_IOS)
                stateFileRead();
#endif
                emit deviceConnected(b);
                connect(lifefitnessTreadmill, &bluetoothdevice::connectedAndDiscovered, this,
                        &bluetooth::connectedAndDiscovered);
                // connect(lifefitnessTreadmill, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(lifefitnessTreadmill, &lifefitnesstreadmill::debug, this, &bluetooth::debug);
                // NOTE: Commented due to #358
                // connect(lifefitnessTreadmill, SIGNAL(speedChanged(double)), this, SLOT(speedChanged(double)));
                // NOTE: Commented due to #358
                // connect(lifefitnessTreadmill, SIGNAL(inclinationChanged(double)), this,
                // SLOT(inclinationChanged(double)));
                lifefitnessTreadmill->deviceDiscovered(b);
                // NOTE: Commented due to #358
                // connect(this, SIGNAL(searchingStop()), lifefitnessTreadmill, SLOT(searchingStop()));
                if (this->discoveryAgent && !this->discoveryAgent->isActive()) {
                    emit searchingStop();
                }
                this->signalBluetoothDeviceConnected(lifefitnessTreadmill);
            } else if ((b.name().toUpper().startsWith(QStringLiteral("HORIZON")) ||
                        b.name().toUpper().startsWith(QStringLiteral("AFG SPORT")) ||
                        b.name().toUpper().startsWith(QStringLiteral("WLT2541")) ||
                        (b.name().toUpper().startsWith(QStringLiteral("TREADMILL")) && (gem_module_inclination || deviceHasService(b, QBluetoothUuid((quint16)0x1826)))) ||
                        b.name().toUpper().startsWith(QStringLiteral("T318_")) || // FTMS
                        (b.name().toUpper().startsWith(QStringLiteral("DK")) && b.name().length() >= 11 &&
                         !toorx_bike) ||                                            // FTMS
                        b.name().toUpper().startsWith(QStringLiteral("T218_")) ||   // FTMS
                        b.name().toUpper().startsWith(QStringLiteral("TRX3500")) || // FTMS
                        b.name().toUpper().startsWith(QStringLiteral("JFTMPARAGON")) ||
                        b.name().toUpper().startsWith(QStringLiteral("PARAGON X")) ||
                        b.name().toUpper().startsWith(QStringLiteral("MX-TM ")) ||     // FTMS
                        b.name().toUpper().startsWith(QStringLiteral("JFTM")) ||       // FTMS
                        b.name().toUpper().startsWith(QStringLiteral("CT800")) ||      // FTMS
                        b.name().toUpper().startsWith(QStringLiteral("TRX4500")) ||    // FTMS
                        b.name().toUpper().startsWith(QStringLiteral("MATRIXTF50")) || // FTMS
                        b.name().toUpper().startsWith(QStringLiteral("T01_")) ||       // FTMS
                        (b.name().startsWith(QStringLiteral("SW")) && b.name().length() == 14 &&
                         !b.name().contains('(') && !b.name().contains(')') && deviceHasService(b, QBluetoothUuid((quint16)0x1826))) ||
                        (b.name().toUpper().startsWith(QStringLiteral("TF-")) &&
                         horizon_treadmill_force_ftms) || // FTMS, TF-769DF2
                        ((b.name().toUpper().startsWith(QStringLiteral("TOORX")) ||
                          (b.name().toUpper().startsWith(QStringLiteral("I-CONSOLE+")))) &&
                         !toorx_ftms && toorx_ftms_treadmill) ||
                        !b.name().compare(ftms_treadmill, Qt::CaseInsensitive) ||
                        (b.name().toUpper().startsWith(QStringLiteral("DOMYOS-TC")) && deviceHasService(b, QBluetoothUuid((quint16)0x1826)) && !settings.value(QZSettings::domyostreadmill_notfmts, QZSettings::default_domyostreadmill_notfmts).toBool()) ||
                        b.name().toUpper().startsWith(QStringLiteral("XT685")) ||
                        b.name().toUpper().startsWith(QStringLiteral("XT285")) ||
                        b.name().toUpper().startsWith(QStringLiteral("WELLFIT TM")) ||
                        b.name().toUpper().startsWith(QStringLiteral("XTERRA TR")) ||
                        b.name().toUpper().startsWith(QStringLiteral("T118_")) ||
                        b.name().toUpper().startsWith(QStringLiteral("RUNN ")) ||                        
                        b.name().toUpper().startsWith(QStringLiteral("TF04-")) ||                           // Sport Synology Z5 Treadmill #2415
                        b.name().toUpper().startsWith(QStringLiteral("FIT-")) ||                            // FIT-1596
                        b.name().toUpper().startsWith(QStringLiteral("LJJ-")) ||                            // LJJ-02351A
                        b.name().toUpper().startsWith(QStringLiteral("WLT-EP-")) ||                             // Flow elliptical
                        (b.name().toUpper().startsWith("SCHWINN 810")) ||
                        b.name().toUpper().startsWith(QStringLiteral("KS-MC")) ||    
                        (b.name().toUpper().startsWith(QStringLiteral("KS-HD-Z1D"))) ||                     // Kingsmith WalkingPad Z1
                        (b.name().toUpper().startsWith(QStringLiteral("FIT-")) && deviceHasService(b, QBluetoothUuid((quint16)0x1826))) || // sports tech f37s treadmill #2412
                        (b.name().toUpper().startsWith(QStringLiteral("NOBLEPRO CONNECT")) && deviceHasService(b, QBluetoothUuid((quint16)0x1826))) || // FTMS
                        (b.name().toUpper().startsWith(QStringLiteral("TT8")) && deviceHasService(b, QBluetoothUuid((quint16)0x1826))) ||
                        (b.name().toUpper().startsWith(QStringLiteral("ST90")) && deviceHasService(b, QBluetoothUuid((quint16)0x1826))) ||
                        (b.name().toUpper().startsWith(QStringLiteral("XT485"))  && deviceHasService(b, QBluetoothUuid((quint16)0x1826))) ||
                        b.name().toUpper().startsWith(QStringLiteral("MOBVOI TM")) ||                        // FTMS
                            b.name().toUpper().startsWith(QStringLiteral("LB600")) ||                        // FTMS
                        b.name().toUpper().startsWith(QStringLiteral("TUNTURI T60-")) ||                     // FTMS
                        b.name().toUpper().startsWith(QStringLiteral("KETTLER TREADMILL")) ||                // FTMS
                        b.name().toUpper().startsWith(QStringLiteral("ASSAULTRUNNER")) ||                    // FTMS
                        b.name().toUpper().startsWith(QStringLiteral("CITYSPORTS-LINKER")) ||
                        (b.name().toUpper().startsWith(QStringLiteral("CTM")) && b.name().length() >= 15) || // FTMS
                        (b.name().toUpper().startsWith(QStringLiteral("F85")) && !sole_inclination) ||       // FMTS
                        (b.name().toUpper().startsWith(QStringLiteral("F89")) && !sole_inclination) ||       // FMTS
                        (b.name().toUpper().startsWith(QStringLiteral("F80")) && !sole_inclination) ||       // FMTS
                        (b.name().toUpper().startsWith(QStringLiteral("ANPLUS-")))                           // FTMS
                        ) &&
                       !horizonTreadmill && filter) {
                this->setLastBluetoothDevice(b);
                this->stopDiscovery();
                horizonTreadmill = new horizontreadmill(noWriteResistance, noHeartService);
#if !defined(Q_OS_ANDROID) && !defined(Q_OS_IOS)
                stateFileRead();
#endif
                emit deviceConnected(b);
                connect(horizonTreadmill, &bluetoothdevice::connectedAndDiscovered, this,
                        &bluetooth::connectedAndDiscovered);
                // connect(horizonTreadmill, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(horizonTreadmill, &horizontreadmill::debug, this, &bluetooth::debug);
                // NOTE: Commented due to #358
                // connect(horizonTreadmill, SIGNAL(speedChanged(double)), this, SLOT(speedChanged(double)));
                // NOTE: Commented due to #358
                // connect(horizonTreadmill, SIGNAL(inclinationChanged(double)), this,
                // SLOT(inclinationChanged(double)));
                horizonTreadmill->deviceDiscovered(b);
                // NOTE: Commented due to #358
                // connect(this, SIGNAL(searchingStop()), horizonTreadmill, SLOT(searchingStop()));
                if (this->discoveryAgent && !this->discoveryAgent->isActive()) {
                    emit searchingStop();
                }
                this->signalBluetoothDeviceConnected(horizonTreadmill);
            } else if ((b.name().toUpper().startsWith(QStringLiteral("MYRUN ")) ||
                        b.name().toUpper().startsWith(QStringLiteral("MERACH-U3")) // FTMS
                        ) &&
                       !technogymmyrunTreadmill 
#ifndef Q_OS_IOS                
                        && !technogymmyrunrfcommTreadmill 
#endif                
                        && filter) {
                this->setLastBluetoothDevice(b);
                this->stopDiscovery();
                bool technogym_myrun_treadmill_experimental =
                    settings
                        .value(QZSettings::technogym_myrun_treadmill_experimental,
                               QZSettings::default_technogym_myrun_treadmill_experimental)
                        .toBool();
#ifndef Q_OS_IOS
                if (!technogym_myrun_treadmill_experimental)
#endif
                {
                    technogymmyrunTreadmill = new technogymmyruntreadmill(noWriteResistance, noHeartService);
                    this->setLastBluetoothDevice(b);
#if !defined(Q_OS_ANDROID) && !defined(Q_OS_IOS)
                    stateFileRead();
#endif
                    emit deviceConnected(b);
                    connect(technogymmyrunTreadmill, &bluetoothdevice::connectedAndDiscovered, this,
                            &bluetooth::connectedAndDiscovered);
                    // connect(technogymmyrunTreadmill, SIGNAL(disconnected()), this, SLOT(restart()));
                    connect(technogymmyrunTreadmill, &technogymmyruntreadmill::debug, this, &bluetooth::debug);
                    // NOTE: Commented due to #358
                    // connect(horizonTreadmill, SIGNAL(speedChanged(double)), this, SLOT(speedChanged(double)));
                    // NOTE: Commented due to #358
                    // connect(horizonTreadmill, SIGNAL(inclinationChanged(double)), this,
                    // SLOT(inclinationChanged(double)));
                    technogymmyrunTreadmill->deviceDiscovered(b);
                    // NOTE: Commented due to #358
                    // connect(this, SIGNAL(searchingStop()), horizonTreadmill, SLOT(searchingStop()));
                    if (this->discoveryAgent && !this->discoveryAgent->isActive()) {
                        emit searchingStop();
                    }
                    this->signalBluetoothDeviceConnected(technogymmyrunTreadmill);
                }
#ifndef Q_OS_IOS
                else {
                    technogymmyrunrfcommTreadmill = new technogymmyruntreadmillrfcomm();
#if !defined(Q_OS_ANDROID) && !defined(Q_OS_IOS)
                    stateFileRead();
#endif
                    emit deviceConnected(b);
                    connect(technogymmyrunrfcommTreadmill, &bluetoothdevice::connectedAndDiscovered, this,
                            &bluetooth::connectedAndDiscovered);
                    // connect(technogymmyrunrfcommTreadmill, SIGNAL(disconnected()), this, SLOT(restart()));
                    connect(technogymmyrunrfcommTreadmill, &technogymmyruntreadmillrfcomm::debug, this,
                            &bluetooth::debug);
                    // NOTE: Commented due to #358
                    // connect(technogymmyrunrfcommTreadmill, SIGNAL(speedChanged(double)), this,
                    // SLOT(speedChanged(double))); NOTE: Commented due to #358 connect(technogymmyrunrfcommTreadmill,
                    // SIGNAL(inclinationChanged(double)), this, SLOT(inclinationChanged(double)));
                    technogymmyrunrfcommTreadmill->deviceDiscovered(b);
                    // NOTE: Commented due to #358
                    // connect(this, SIGNAL(searchingStop()), horizonTreadmill, SLOT(searchingStop()));
                    if (this->discoveryAgent && !this->discoveryAgent->isActive()) {
                        emit searchingStop();
                    }
                    this->signalBluetoothDeviceConnected(technogymmyrunrfcommTreadmill);
                }
#endif
            } else if ((b.name().toUpper().startsWith("TACX ") ||
                        b.name().toUpper().startsWith(QStringLiteral("THINK X")) ||
                        b.address() == QBluetoothAddress("C1:14:D9:9C:FB:01") || // specific TACX NEO 2 #1707
                        (b.name().toUpper().startsWith("TACX SMART BIKE"))) &&
                        !b.name().toUpper().startsWith("TACX SATORI") &&
                       !tacxneo2Bike && filter) {
                this->setLastBluetoothDevice(b);
                this->stopDiscovery();
                tacxneo2Bike = new tacxneo2(noWriteResistance, noHeartService);
                // stateFileRead();
                emit(deviceConnected(b));
                connect(tacxneo2Bike, SIGNAL(connectedAndDiscovered()), this, SLOT(connectedAndDiscovered()));
                // connect(tacxneo2Bike, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(tacxneo2Bike, SIGNAL(debug(QString)), this, SLOT(debug(QString)));
                // connect(tacxneo2Bike, SIGNAL(speedChanged(double)), this, SLOT(speedChanged(double)));
                // connect(tacxneo2Bike, SIGNAL(inclinationChanged(double)), this, SLOT(inclinationChanged(double)));
                tacxneo2Bike->deviceDiscovered(b);
                this->signalBluetoothDeviceConnected(tacxneo2Bike);
            } else if ((b.name().toUpper().startsWith(QStringLiteral(">CABLE")) ||
                        (b.name().toUpper().startsWith(QStringLiteral("MD")) && b.name().length() == 7) ||
                        // BIKE 1, BIKE 2, BIKE 3...
                        (b.name().toUpper().startsWith(QStringLiteral("BIKE")) && flywheel_life_fitness_ic8 == false &&
                         b.name().length() == 6)) &&
                       !npeCableBike && filter) {
                this->setLastBluetoothDevice(b);
                this->stopDiscovery();
                npeCableBike = new npecablebike(noWriteResistance, noHeartService);
                // stateFileRead();
                emit deviceConnected(b);
                connect(npeCableBike, &bluetoothdevice::connectedAndDiscovered, this,
                        &bluetooth::connectedAndDiscovered);
                // connect(echelonConnectSport, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(npeCableBike, &npecablebike::debug, this, &bluetooth::debug);
                // connect(echelonConnectSport, SIGNAL(speedChanged(double)), this, SLOT(speedChanged(double)));
                // connect(echelonConnectSport, SIGNAL(inclinationChanged(double)), this,
                // SLOT(inclinationChanged(double)));
                npeCableBike->deviceDiscovered(b);
                this->signalBluetoothDeviceConnected(npeCableBike);
            } else if (((b.name().startsWith("FS-") && hammerRacerS) ||
                        (b.name().toUpper().startsWith(QStringLiteral("ICONSOLE+")) && toorx_ftms ) ||
                        (b.name().toUpper().startsWith("DI") && b.name().length() == 2) || // Elite smart trainer #1682
                        (b.name().toUpper().startsWith("DHZ-")) ||                         // JK fitness 577
                        (b.name().toUpper().startsWith("MKSM")) ||                         // MKSM3600036
                        (b.name().toUpper().startsWith("YS_C1_")) ||                       // Yesoul C1H
                        (b.name().toUpper().startsWith("YS_G1_")) ||                       // Yesoul S3
                        (b.name().toUpper().startsWith("YS_G1MPLUS")) ||                   // Yesoul G1M Plus
                        (b.name().toUpper().startsWith("DS25-")) ||                        // Bodytone DS25
                        (b.name().toUpper().startsWith("SCHWINN 510T")) ||
                        (b.name().toUpper().startsWith("3G CARDIO ")) ||
                        (b.name().toUpper().startsWith("ZWIFT HUB")) || (b.name().toUpper().startsWith("MAGNUS ")) ||
                        (b.name().toUpper().startsWith("HAMMER ") && !power_as_bike && !saris_trainer) ||      // HAMMER 64123
                        (b.name().toUpper().startsWith("FLXCY-")) ||                         // Pro FlexBike
                        (b.name().toUpper().startsWith("QB-WC01")) ||                        // Nexgim QB-C01 smart bike
                        (b.name().toUpper().startsWith("XBR55")) ||                          // Sprint XBR555
                        (b.name().toUpper().startsWith("ECHO_BIKE_")) ||                     // Rogue echo bike V3.0
                        (b.name().toUpper().startsWith("EW-JS-")) ||                         // EW-JS-4990
                        (b.name().toUpper().startsWith("DT-") && b.name().length() >= 14) || // SOLE SB700
                        (b.name().toUpper().startsWith("YSV") && b.name().length() == 9) ||  // YSV100783
                        (b.name().toUpper().startsWith("URSB") && b.name().length() == 7) || // URSB005
                        (b.name().toUpper().startsWith("DBF") && b.name().length() == 6) ||  // DBF135
                        (b.name().toUpper().startsWith("KSU") && b.name().length() == 7) ||  // KSU1102
                        (b.name().toUpper().startsWith(ftmsAccessoryName.toUpper()) &&
                         settings.value(QZSettings::ss2k_peloton, QZSettings::default_ss2k_peloton)
                             .toBool()) || // ss2k on a peloton bike
                        (b.name().toUpper().startsWith("MERACH-MR667-")) ||
                        (b.name().toUpper().startsWith("DS60-")) ||
                        (b.name().toUpper().startsWith("BIKE-")) ||
                        (b.name().toUpper().startsWith("M9-")) ||
                        (b.name().toUpper().startsWith("SPAX-BK-")) ||
                        (b.name().toUpper().startsWith("YSV1")) ||
                        (b.name().toUpper().startsWith("VOLT") && b.name().length() == 4) ||
                        (b.name().toUpper().startsWith("VICTORY")) ||
                        (b.name().toUpper().startsWith("CECOTEC")) ||       // Cecotec DrumFit Indoor 10000 MagnoMotor Connected #2420
                        (b.name().toUpper().startsWith("WATTBIKE")) ||
                        (b.name().toUpper().startsWith("ZYCLEZBIKE")) ||
                        (b.name().toUpper().startsWith("WAVEFIT-")) ||
                        (b.name().toUpper().startsWith("KETTLERBLE")) ||
                        (b.name().toUpper().startsWith("JAS_C3")) ||
                        (b.name().toUpper().startsWith("SCH_190U")) ||
                        (b.name().toUpper().startsWith("RAVE WHITE")) ||
                        (b.name().toUpper().startsWith("DOMYOS-BIKING-")) ||
                        (b.name().startsWith(QStringLiteral("Domyos-Bike")) && deviceHasService(b, QBluetoothUuid((quint16)0x1826)) && !settings.value(QZSettings::domyosbike_notfmts, QZSettings::default_domyosbike_notfmts).toBool()) ||
                        (b.name().toUpper().startsWith("F") && b.name().toUpper().endsWith("ARROW")) || // FI9110 Arrow, https://www.fitnessdigital.it/bicicletta-smart-bike-ion-fitness-arrow-connect/p/10022863/ IO Fitness Arrow
                        (b.name().toUpper().startsWith("ICSE") && b.name().length() == 4) ||
                        (b.name().toUpper().startsWith("TUO") && b.name().length() == 3) ||
                        (b.name().toUpper().startsWith("FLX") && b.name().length() == 10) ||
                        (b.name().toUpper().startsWith("CSRB") && b.name().length() == 11) ||
                        (b.name().toUpper().startsWith("DU30-")) ||                          // BodyTone du30
                        (b.name().toUpper().startsWith("BIKZU_")) ||
                        (b.name().toUpper().startsWith("WLT8828")) ||
                        (b.name().toUpper().startsWith("VANRYSEL-HT")) ||
                        (b.name().toUpper().startsWith("HARISON-X15")) ||
                        (b.name().toUpper().startsWith("FEIVON V2")) ||
                        (b.name().toUpper().startsWith("FELVON V2")) ||
                        (b.name().toUpper().startsWith("JUSTO")) ||
                        (b.name().toUpper().startsWith("T2 ")) ||
                        (b.name().toUpper().startsWith("VFSPINBIKE")) ||
                        (b.name().toUpper().startsWith("GLT") && deviceHasService(b, QBluetoothUuid((quint16)0x1826))) ||
                        (b.name().toUpper().startsWith("SPORT01-") && deviceHasService(b, QBluetoothUuid((quint16)0x1826))) || // Labgrey Magnetic Exercise Bike https://www.amazon.co.uk/dp/B0CXMF1NPY?_encoding=UTF8&psc=1&ref=cm_sw_r_cp_ud_dp_PE420HA7RD7WJBZPN075&ref_=cm_sw_r_cp_ud_dp_PE420HA7RD7WJBZPN075&social_share=cm_sw_r_cp_ud_dp_PE420HA7RD7WJBZPN075&skipTwisterOG=1
                        (b.name().toUpper().startsWith("ZUMO")) || (b.name().toUpper().startsWith("XS08-")) ||
                        (b.name().toUpper().startsWith("B94")) || (b.name().toUpper().startsWith("STAGES BIKE")) ||
                        (b.name().toUpper().startsWith("SUITO")) || (b.name().toUpper().startsWith("D2RIDE")) ||
                        (b.name().toUpper().startsWith("DIRETO X")) || (b.name().toUpper().startsWith("MERACH-667-")) ||
                        !b.name().compare(ftms_bike, Qt::CaseInsensitive) || (b.name().toUpper().startsWith("SMB1")) ||
                        (b.name().toUpper().startsWith("UBIKE FTMS")) || (b.name().toUpper().startsWith("INRIDE"))) &&
                       !ftmsBike && !snodeBike && !fitPlusBike && !stagesBike && filter) {
                this->setLastBluetoothDevice(b);
                this->stopDiscovery();
                ftmsBike = new ftmsbike(noWriteResistance, noHeartService, bikeResistanceOffset, bikeResistanceGain);
                emit deviceConnected(b);
                connect(ftmsBike, &bluetoothdevice::connectedAndDiscovered, this, &bluetooth::connectedAndDiscovered);
                // connect(trxappgateusb, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(ftmsBike, &ftmsbike::debug, this, &bluetooth::debug);
                ftmsBike->deviceDiscovered(b);
                this->signalBluetoothDeviceConnected(ftmsBike);
            } else if ((b.name().toUpper().startsWith("KICKR SNAP") || b.name().toUpper().startsWith("KICKR BIKE") ||
                        b.name().toUpper().startsWith("KICKR ROLLR") ||
                        b.name().toUpper().startsWith("KICKR CORE") ||
                        (b.name().toUpper().startsWith("HAMMER ") && saris_trainer) ||
                        (b.name().toUpper().startsWith("WAHOO KICKR"))) &&
                       !wahooKickrSnapBike && !ftmsBike && filter) {
                this->setLastBluetoothDevice(b);
                this->stopDiscovery();
                wahooKickrSnapBike =
                    new wahookickrsnapbike(noWriteResistance, noHeartService, bikeResistanceOffset, bikeResistanceGain);
                emit deviceConnected(b);
                connect(wahooKickrSnapBike, &bluetoothdevice::connectedAndDiscovered, this,
                        &bluetooth::connectedAndDiscovered);
                // connect(wahooKickrSnapBike, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(wahooKickrSnapBike, &wahookickrsnapbike::debug, this, &bluetooth::debug);
                wahooKickrSnapBike->deviceDiscovered(b);
                this->signalBluetoothDeviceConnected(wahooKickrSnapBike);
            } else if (((b.name().toUpper().startsWith("JFIC")) // HORIZON GR7
                        ) &&
                       !horizonGr7Bike && filter) {
                this->setLastBluetoothDevice(b);
                this->stopDiscovery();
                horizonGr7Bike =
                    new horizongr7bike(noWriteResistance, noHeartService, bikeResistanceOffset, bikeResistanceGain);
                emit deviceConnected(b);
                connect(horizonGr7Bike, &bluetoothdevice::connectedAndDiscovered, this,
                        &bluetooth::connectedAndDiscovered);
                // connect(trxappgateusb, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(horizonGr7Bike, &horizongr7bike::debug, this, &bluetooth::debug);
                horizonGr7Bike->deviceDiscovered(b);
                this->signalBluetoothDeviceConnected(horizonGr7Bike);
            } else if ((b.name().toUpper().startsWith(QStringLiteral("STAGES ")) ||
                        (b.name().toUpper().startsWith("TACX SATORI")) ||
                        (b.name().toUpper().startsWith("RACER S")) ||
                        ((b.name().toUpper().startsWith("KU")) && b.name().length() == 2) ||
                        (b.name().toUpper().startsWith("ELITETRAINER")) ||
                        (b.name().toUpper().startsWith(QStringLiteral("QD")) && b.name().length() == 2) ||
                        (b.name().toUpper().startsWith(QStringLiteral("DFC")) && b.name().length() == 3) ||
                        (b.name().toUpper().startsWith(QStringLiteral("ASSIOMA")) &&
                         powerSensorName.startsWith(QStringLiteral("Disabled")))) &&
                       !stagesBike && !ftmsBike && filter) {
                this->setLastBluetoothDevice(b);
                this->stopDiscovery();
                stagesBike = new stagesbike(noWriteResistance, noHeartService, false);
                // stateFileRead();
                emit deviceConnected(b);
                connect(stagesBike, &bluetoothdevice::connectedAndDiscovered, this, &bluetooth::connectedAndDiscovered);
                // connect(stagesBike, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(stagesBike, &stagesbike::debug, this, &bluetooth::debug);
                // connect(stagesBike, SIGNAL(speedChanged(double)), this, SLOT(speedChanged(double)));
                // connect(stagesBike, SIGNAL(inclinationChanged(double)), this, SLOT(inclinationChanged(double)));
                stagesBike->deviceDiscovered(b);
                this->signalBluetoothDeviceConnected(stagesBike);
            } else if (b.name().toUpper().startsWith(QStringLiteral("SMARTROW")) && !smartrowRower && filter) {
                this->setLastBluetoothDevice(b);
                this->stopDiscovery();
                smartrowRower =
                    new smartrowrower(noWriteResistance, noHeartService, bikeResistanceOffset, bikeResistanceGain);
                // stateFileRead();
                emit deviceConnected(b);
                connect(smartrowRower, &bluetoothdevice::connectedAndDiscovered, this,
                        &bluetooth::connectedAndDiscovered);
                // connect(smartrowRower, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(smartrowRower, SIGNAL(debug(QString)), this, SLOT(debug(QString)));
                // connect(v, SIGNAL(speedChanged(double)), this, SLOT(speedChanged(double)));
                // connect(smartrowRower, SIGNAL(inclinationChanged(double)), this, SLOT(inclinationChanged(double)));
                smartrowRower->deviceDiscovered(b);
                this->signalBluetoothDeviceConnected(smartrowRower);
            } else if ((b.name().toUpper().startsWith(QStringLiteral("PM5")) &&
                        !b.name().toUpper().endsWith(QStringLiteral("ROW"))) &&
                       !concept2Skierg && filter) {
                this->setLastBluetoothDevice(b);
                this->stopDiscovery();
                concept2Skierg = new concept2skierg(noWriteResistance, noHeartService);
                // stateFileRead();
                emit deviceConnected(b);
                connect(concept2Skierg, &bluetoothdevice::connectedAndDiscovered, this,
                        &bluetooth::connectedAndDiscovered);
                // connect(concept2Skierg, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(concept2Skierg, SIGNAL(debug(QString)), this, SLOT(debug(QString)));
                // connect(v, SIGNAL(speedChanged(double)), this, SLOT(speedChanged(double)));
                // connect(concept2Skierg, SIGNAL(inclinationChanged(double)), this, SLOT(inclinationChanged(double)));
                concept2Skierg->deviceDiscovered(b);
                this->signalBluetoothDeviceConnected(concept2Skierg);
            } else if ((b.name().toUpper().startsWith(QStringLiteral("CR 00")) ||
                        b.name().toUpper().startsWith(QStringLiteral("KAYAKPRO")) ||
                        b.name().toUpper().startsWith(QStringLiteral("WHIPR")) ||
                        b.name().toUpper().startsWith(QStringLiteral("H-181-")) ||
                        b.name().toUpper().startsWith(QStringLiteral("S4 COMMS")) ||
                        b.name().toUpper().startsWith(QStringLiteral("KS-WLT")) || // KS-WLT-W1
                        b.name().toUpper().startsWith(QStringLiteral("I-ROWER")) ||
                        b.name().toUpper().startsWith(QStringLiteral("SF-RW")) ||
                        b.name().toUpper().startsWith(QStringLiteral("DFIT-L-R")) ||
                        !b.name().compare(ftms_rower, Qt::CaseInsensitive) ||
                        (b.name().toUpper().startsWith(QStringLiteral("PM5")) &&
                         b.name().toUpper().endsWith(QStringLiteral("ROW")))) &&
                       !ftmsRower && filter) {
                this->setLastBluetoothDevice(b);
                this->stopDiscovery();
                ftmsRower = new ftmsrower(noWriteResistance, noHeartService);
                // stateFileRead();
                emit deviceConnected(b);
                connect(ftmsRower, &bluetoothdevice::connectedAndDiscovered, this, &bluetooth::connectedAndDiscovered);
                // connect(ftmsRower, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(ftmsRower, SIGNAL(debug(QString)), this, SLOT(debug(QString)));
                // connect(v, SIGNAL(speedChanged(double)), this, SLOT(speedChanged(double)));
                // connect(ftmsRower, SIGNAL(inclinationChanged(double)), this, SLOT(inclinationChanged(double)));
                ftmsRower->deviceDiscovered(b);
                this->signalBluetoothDeviceConnected(ftmsRower);
            } else if ((b.name().toUpper().startsWith(QLatin1String("ECH-STRIDE")) ||
                        b.name().toUpper().startsWith(QLatin1String("ECH-UK-")) ||
                        b.name().toUpper().startsWith(QLatin1String("ECH-FR-")) ||
                        b.name().toUpper().startsWith(QLatin1String("STRIDE")) ||
                        b.name().toUpper().startsWith(QLatin1String("STRIDE6S-")) ||
                        b.name().toUpper().startsWith(QLatin1String("ECH-SD-SPT"))) &&
                       !echelonStride && filter) {
                this->setLastBluetoothDevice(b);
                this->stopDiscovery();
                echelonStride = new echelonstride(this->pollDeviceTime, noConsole, noHeartService);
                // stateFileRead();
                emit deviceConnected(b);
                connect(echelonStride, &bluetoothdevice::connectedAndDiscovered, this,
                        &bluetooth::connectedAndDiscovered);
                // connect(echelonRower, SIGNAL(disconnected()), this, SLOT(restart())); connect(echelonStride,
                connect(echelonStride, &echelonstride::debug, this, &bluetooth::debug);
                connect(echelonStride, &echelonstride::speedChanged, this, &bluetooth::speedChanged);
                connect(echelonStride, &echelonstride::inclinationChanged, this, &bluetooth::inclinationChanged);
                echelonStride->deviceDiscovered(b);
                this->signalBluetoothDeviceConnected(echelonStride);
            } else if ((b.name().toUpper().startsWith(QLatin1String("Q37"))) && !octaneElliptical && filter) {
                this->setLastBluetoothDevice(b);
                this->stopDiscovery();
                octaneElliptical = new octaneelliptical(this->pollDeviceTime, noConsole, noHeartService);
                // stateFileRead();
                emit deviceConnected(b);
                connect(octaneElliptical, &bluetoothdevice::connectedAndDiscovered, this,
                        &bluetooth::connectedAndDiscovered);
                // connect(octaneElliptical, SIGNAL(disconnected()), this, SLOT(restart())); connect(echelonStride,
                connect(octaneElliptical, &octaneelliptical::debug, this, &bluetooth::debug);
                connect(octaneElliptical, &octaneelliptical::speedChanged, this, &bluetooth::speedChanged);
                connect(octaneElliptical, &octaneelliptical::inclinationChanged, this, &bluetooth::inclinationChanged);
                octaneElliptical->deviceDiscovered(b);
                this->signalBluetoothDeviceConnected(octaneElliptical);
            } else if ((b.name().toUpper().startsWith(QLatin1String("ZR7")) ||
                        b.name().toUpper().startsWith(QLatin1String("ZR8"))) &&
                       !octaneTreadmill && filter) {
                this->setLastBluetoothDevice(b);
                this->stopDiscovery();
                octaneTreadmill = new octanetreadmill(this->pollDeviceTime, noConsole, noHeartService);
                // stateFileRead();
                emit deviceConnected(b);
                connect(octaneTreadmill, &bluetoothdevice::connectedAndDiscovered, this,
                        &bluetooth::connectedAndDiscovered);
                // connect(octaneTreadmill, SIGNAL(disconnected()), this, SLOT(restart())); connect(echelonStride,
                connect(octaneTreadmill, &octanetreadmill::debug, this, &bluetooth::debug);
                connect(octaneTreadmill, &octanetreadmill::speedChanged, this, &bluetooth::speedChanged);
                connect(octaneTreadmill, &octanetreadmill::inclinationChanged, this, &bluetooth::inclinationChanged);
                octaneTreadmill->deviceDiscovered(b);
                this->signalBluetoothDeviceConnected(octaneTreadmill);
            } else if ((b.name().toUpper().startsWith(QLatin1String("RZ_TREADMIL"))) && !ziproTreadmill && filter) {
                this->setLastBluetoothDevice(b);
                this->stopDiscovery();
                ziproTreadmill = new ziprotreadmill(this->pollDeviceTime, noConsole, noHeartService);
                // stateFileRead();
                emit deviceConnected(b);
                connect(ziproTreadmill, &bluetoothdevice::connectedAndDiscovered, this,
                        &bluetooth::connectedAndDiscovered);
                // connect(ziproTreadmill, SIGNAL(disconnected()), this, SLOT(restart())); connect(echelonStride,
                connect(ziproTreadmill, &ziprotreadmill::debug, this, &bluetooth::debug);
                connect(ziproTreadmill, &ziprotreadmill::speedChanged, this, &bluetooth::speedChanged);
                connect(ziproTreadmill, &ziprotreadmill::inclinationChanged, this, &bluetooth::inclinationChanged);
                ziproTreadmill->deviceDiscovered(b);
                this->signalBluetoothDeviceConnected(ziproTreadmill);
            } else if ((b.name().startsWith(QStringLiteral("ECH-ROW")) ||
                        b.name().toUpper().startsWith(QStringLiteral("ROWSPORT")) ||
                        b.name().startsWith(QStringLiteral("ROW-S"))) &&
                       !echelonRower && filter) {
                this->setLastBluetoothDevice(b);
                this->stopDiscovery();
                echelonRower =
                    new echelonrower(noWriteResistance, noHeartService, bikeResistanceOffset, bikeResistanceGain);
                // stateFileRead();
                emit deviceConnected(b);
                connect(echelonRower, &bluetoothdevice::connectedAndDiscovered, this,
                        &bluetooth::connectedAndDiscovered);
                // connect(echelonRower, SIGNAL(disconnected()), this, SLOT(restart()));
                // connect(echelonRower, SIGNAL(debug(QString)), this, SLOT(debug(QString)));
                // connect(echelonRower, SIGNAL(speedChanged(double)), this, SLOT(speedChanged(double)));
                // connect(echelonRower, SIGNAL(inclinationChanged(double)), this, SLOT(inclinationChanged(double)));
                echelonRower->deviceDiscovered(b);
                this->signalBluetoothDeviceConnected(echelonRower);
            } else if (b.name().startsWith(QStringLiteral("ECH")) && !echelonRower && !echelonStride &&
                       !echelonConnectSport && filter) {
                this->setLastBluetoothDevice(b);
                this->stopDiscovery();
                echelonConnectSport = new echelonconnectsport(noWriteResistance, noHeartService, bikeResistanceOffset,
                                                              bikeResistanceGain);
                // stateFileRead();
                emit deviceConnected(b);
                connect(echelonConnectSport, &bluetoothdevice::connectedAndDiscovered, this,
                        &bluetooth::connectedAndDiscovered);
                // connect(echelonConnectSport, SIGNAL(disconnected()), this, SLOT(restart()));
                // connect(echelonConnectSport, SIGNAL(debug(QString)), this, SLOT(debug(QString)));
                // connect(echelonConnectSport, SIGNAL(speedChanged(double)), this, SLOT(speedChanged(double)));
                // connect(echelonConnectSport, SIGNAL(inclinationChanged(double)), this,
                // SLOT(inclinationChanged(double)));
                echelonConnectSport->deviceDiscovered(b);
                this->signalBluetoothDeviceConnected(echelonConnectSport);
            } else if (b.name().toUpper().startsWith(QStringLiteral("WLT8266BM")) && !apexBike && filter) {
                this->setLastBluetoothDevice(b);
                this->stopDiscovery();
                apexBike = new apexbike(noWriteResistance, noHeartService, bikeResistanceOffset, bikeResistanceGain);
                // stateFileRead();
                emit deviceConnected(b);
                connect(apexBike, &bluetoothdevice::connectedAndDiscovered, this, &bluetooth::connectedAndDiscovered);
                apexBike->deviceDiscovered(b);
                this->signalBluetoothDeviceConnected(apexBike);
            } else if (b.name().toUpper().startsWith(QStringLiteral("BKOOLSMARTPRO")) && !bkoolBike && filter) {
                this->setLastBluetoothDevice(b);
                this->stopDiscovery();
                bkoolBike = new bkoolbike(noWriteResistance, noHeartService);
                // stateFileRead();
                emit deviceConnected(b);
                connect(bkoolBike, &bluetoothdevice::connectedAndDiscovered, this, &bluetooth::connectedAndDiscovered);
                connect(bkoolBike, &bkoolbike::debug, this, &bluetooth::debug);
                bkoolBike->deviceDiscovered(b);
                this->signalBluetoothDeviceConnected(bkoolBike);
            } else if (b.name().toUpper().startsWith(QStringLiteral("MEPANEL")) && !mepanelBike && filter) {
                this->setLastBluetoothDevice(b);
                this->stopDiscovery();
                mepanelBike =
                    new mepanelbike(noWriteResistance, noHeartService, bikeResistanceOffset, bikeResistanceGain);
                // stateFileRead();
                emit deviceConnected(b);
                connect(mepanelBike, &bluetoothdevice::connectedAndDiscovered, this,
                        &bluetooth::connectedAndDiscovered);
                mepanelBike->deviceDiscovered(b);
                this->signalBluetoothDeviceConnected(mepanelBike);
            } else if ((b.name().toUpper().startsWith(QStringLiteral("SCHWINN 170/270"))) && !schwinn170Bike &&
                       filter) {
                this->setLastBluetoothDevice(b);
                this->stopDiscovery();
                schwinn170Bike =
                    new schwinn170bike(noWriteResistance, noHeartService, bikeResistanceOffset, bikeResistanceGain);
                // stateFileRead();
                emit deviceConnected(b);
                connect(schwinn170Bike, &bluetoothdevice::connectedAndDiscovered, this,
                        &bluetooth::connectedAndDiscovered);
                // connect(echelonConnectSport, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(schwinn170Bike, &schwinn170bike::debug, this, &bluetooth::debug);
                // connect(echelonConnectSport, SIGNAL(speedChanged(double)), this, SLOT(speedChanged(double)));
                // connect(echelonConnectSport, SIGNAL(inclinationChanged(double)), this,
                // SLOT(inclinationChanged(double)));
                schwinn170Bike->deviceDiscovered(b);
                this->signalBluetoothDeviceConnected(schwinn170Bike);
            } else if ((b.name().toUpper().startsWith(QStringLiteral("IC BIKE")) ||
                        (b.name().toUpper().startsWith(QStringLiteral("C7-")) && b.name().length() != 17) ||
                        b.name().toUpper().startsWith(QStringLiteral("C9/C10"))) &&
                       !schwinnIC4Bike && filter) {
                this->setLastBluetoothDevice(b);
                this->stopDiscovery();
                schwinnIC4Bike = new schwinnic4bike(noWriteResistance, noHeartService);
                // stateFileRead();
                emit deviceConnected(b);
                connect(schwinnIC4Bike, &bluetoothdevice::connectedAndDiscovered, this,
                        &bluetooth::connectedAndDiscovered);
                // connect(echelonConnectSport, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(schwinnIC4Bike, &schwinnic4bike::debug, this, &bluetooth::debug);
                // connect(echelonConnectSport, SIGNAL(speedChanged(double)), this, SLOT(speedChanged(double)));
                // connect(echelonConnectSport, SIGNAL(inclinationChanged(double)), this,
                // SLOT(inclinationChanged(double)));
                schwinnIC4Bike->deviceDiscovered(b);
                this->signalBluetoothDeviceConnected(schwinnIC4Bike);
            } else if (b.name().toUpper().startsWith(QStringLiteral("EW-BK")) && !sportsTechBike && filter) {
                this->setLastBluetoothDevice(b);
                this->stopDiscovery();
                sportsTechBike = new sportstechbike(noWriteResistance, noHeartService, bikeResistanceOffset,
                                                    bikeResistanceGain);
                // stateFileRead();
                emit deviceConnected(b);
                connect(sportsTechBike, &bluetoothdevice::connectedAndDiscovered, this,
                        &bluetooth::connectedAndDiscovered);
                // connect(echelonConnectSport, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(sportsTechBike, &sportstechbike::debug, this, &bluetooth::debug);
                // connect(echelonConnectSport, SIGNAL(speedChanged(double)), this, SLOT(speedChanged(double)));
                // connect(echelonConnectSport, SIGNAL(inclinationChanged(double)), this,
                // SLOT(inclinationChanged(double)));
                sportsTechBike->deviceDiscovered(b);
                this->signalBluetoothDeviceConnected(sportsTechBike);
            } else if (b.name().toUpper().startsWith(QStringLiteral("EW-EP-")) && !sportsTechElliptical && !horizonTreadmill && filter) {
                this->setLastBluetoothDevice(b);
                this->stopDiscovery();
                sportsTechElliptical = new sportstechelliptical(noWriteResistance, noHeartService, bikeResistanceOffset,
                                                    bikeResistanceGain);
                // stateFileRead();
                emit deviceConnected(b);
                connect(sportsTechElliptical, &bluetoothdevice::connectedAndDiscovered, this,
                        &bluetooth::connectedAndDiscovered);
                // connect(echelonConnectSport, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(sportsTechElliptical, &sportstechelliptical::debug, this, &bluetooth::debug);
                // connect(echelonConnectSport, SIGNAL(speedChanged(double)), this, SLOT(speedChanged(double)));
                // connect(echelonConnectSport, SIGNAL(inclinationChanged(double)), this,
                // SLOT(inclinationChanged(double)));
                sportsTechElliptical->deviceDiscovered(b);
                this->signalBluetoothDeviceConnected(sportsTechElliptical);
            } else if ((b.name().toUpper().startsWith(QStringLiteral("CARDIOFIT")) ||
                        (b.name().toUpper().contains(QStringLiteral("CARE")) &&
                         b.name().length() == 11)) // CARE9040177 - Carefitness CV-351
                       && !sportsPlusBike && filter) {
                this->setLastBluetoothDevice(b);
                this->stopDiscovery();
                sportsPlusBike = new sportsplusbike(noWriteResistance, noHeartService);
                // stateFileRead();
                emit deviceConnected(b);
                connect(sportsPlusBike, &bluetoothdevice::connectedAndDiscovered, this,
                        &bluetooth::connectedAndDiscovered);
                // connect(sportsPlusBike, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(sportsPlusBike, &sportsplusbike::debug, this, &bluetooth::debug);
                // connect(sportsPlusBike, SIGNAL(speedChanged(double)), this, SLOT(speedChanged(double)));
                // connect(sportsPlusBike, SIGNAL(inclinationChanged(double)), this,
                // SLOT(inclinationChanged(double)));
                sportsPlusBike->deviceDiscovered(b);
                this->signalBluetoothDeviceConnected(sportsPlusBike);
            } else if ((b.name().startsWith(yesoulbike::bluetoothName) || 
                        b.name().toUpper().startsWith("YS_G1M_")) && !yesoulBike && filter) {
                this->setLastBluetoothDevice(b);
                this->stopDiscovery();
                yesoulBike =
                    new yesoulbike(noWriteResistance, noHeartService, bikeResistanceOffset, bikeResistanceGain);
                // stateFileRead();
                emit deviceConnected(b);
                connect(yesoulBike, &bluetoothdevice::connectedAndDiscovered, this, &bluetooth::connectedAndDiscovered);
                // connect(yesoulBike, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(yesoulBike, &yesoulbike::debug, this, &bluetooth::debug);
                // connect(echelonConnectSport, SIGNAL(speedChanged(double)), this, SLOT(speedChanged(double)));
                // connect(echelonConnectSport, SIGNAL(inclinationChanged(double)), this,
                // SLOT(inclinationChanged(double)));
                yesoulBike->deviceDiscovered(b);
                this->signalBluetoothDeviceConnected(yesoulBike);
            } else if ((b.name().startsWith(QStringLiteral("I_EB")) || b.name().startsWith(QStringLiteral("I_SB")) ||
                        b.name().toUpper().contains(QStringLiteral("_IFIT_BIKE"))) &&
                       !proformBike && filter) {
                this->setLastBluetoothDevice(b);
                this->stopDiscovery();
                proformBike =
                    new proformbike(noWriteResistance, noHeartService, bikeResistanceOffset, bikeResistanceGain);
                // stateFileRead();
                emit deviceConnected(b);
                connect(proformBike, &bluetoothdevice::connectedAndDiscovered, this,
                        &bluetooth::connectedAndDiscovered);
                // connect(proformBike, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(proformBike, &proformbike::debug, this, &bluetooth::debug);
                // connect(proformBike, SIGNAL(speedChanged(double)), this, SLOT(speedChanged(double)));
                // connect(proformBike, SIGNAL(inclinationChanged(double)), this, SLOT(inclinationChanged(double)));
                proformBike->deviceDiscovered(b);
                this->signalBluetoothDeviceConnected(proformBike);
            } else if ((b.name().startsWith(QStringLiteral("I_TL")) || b.name().startsWith(QStringLiteral("I_IT"))) &&
                       !proformTreadmill && filter) {
                this->setLastBluetoothDevice(b);
                this->stopDiscovery();
                proformTreadmill = new proformtreadmill(noWriteResistance, noHeartService);
                // stateFileRead();
                emit deviceConnected(b);
                connect(proformTreadmill, &bluetoothdevice::connectedAndDiscovered, this,
                        &bluetooth::connectedAndDiscovered);
                // connect(proformtreadmill, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(proformTreadmill, &proformtreadmill::debug, this, &bluetooth::debug);
                // connect(proformtreadmill, SIGNAL(speedChanged(double)), this, SLOT(speedChanged(double)));
                // connect(proformtreadmill, SIGNAL(inclinationChanged(double)), this,
                // SLOT(inclinationChanged(double)));
                proformTreadmill->deviceDiscovered(b);
                this->signalBluetoothDeviceConnected(proformTreadmill);
            } else if ((b.name().toUpper().startsWith(QStringLiteral("ESANGLINKER")) ||
                        b.name().toUpper().startsWith(QStringLiteral("ESLINKER"))) && !eslinkerTreadmill && filter) {
                this->setLastBluetoothDevice(b);
                this->stopDiscovery();
                eslinkerTreadmill = new eslinkertreadmill(this->pollDeviceTime, noConsole, noHeartService);
                // stateFileRead();
                emit deviceConnected(b);
                connect(eslinkerTreadmill, &bluetoothdevice::connectedAndDiscovered, this,
                        &bluetooth::connectedAndDiscovered);
                // connect(proformtreadmill, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(eslinkerTreadmill, &eslinkertreadmill::debug, this, &bluetooth::debug);
                // connect(proformtreadmill, SIGNAL(speedChanged(double)), this, SLOT(speedChanged(double)));
                // connect(proformtreadmill, SIGNAL(inclinationChanged(double)), this,
                // SLOT(inclinationChanged(double)));
                eslinkerTreadmill->deviceDiscovered(b);
                this->signalBluetoothDeviceConnected(eslinkerTreadmill);
            } else if (b.name().toUpper().startsWith(QStringLiteral("PITPAT")) && !deerrunTreadmill && filter) {
                this->setLastBluetoothDevice(b);
                this->stopDiscovery();
                deerrunTreadmill = new deerruntreadmill(this->pollDeviceTime, noConsole, noHeartService);
                // stateFileRead();
                emit deviceConnected(b);
                connect(deerrunTreadmill, &bluetoothdevice::connectedAndDiscovered, this,
                        &bluetooth::connectedAndDiscovered);
                // connect(proformtreadmill, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(deerrunTreadmill, &deerruntreadmill::debug, this, &bluetooth::debug);
                // connect(proformtreadmill, SIGNAL(speedChanged(double)), this, SLOT(speedChanged(double)));
                // connect(proformtreadmill, SIGNAL(inclinationChanged(double)), this,
                // SLOT(inclinationChanged(double)));
                deerrunTreadmill->deviceDiscovered(b);
                this->signalBluetoothDeviceConnected(deerrunTreadmill);
            } else if (b.name().toUpper().startsWith(QStringLiteral("PAFERS_")) && !pafersTreadmill &&
                       (pafers_treadmill || pafers_treadmill_bh_iboxster_plus) && filter) {
                this->setLastBluetoothDevice(b);
                this->stopDiscovery();
                pafersTreadmill = new paferstreadmill(this->pollDeviceTime, noConsole, noHeartService);
                // stateFileRead();
                emit deviceConnected(b);
                connect(pafersTreadmill, &bluetoothdevice::connectedAndDiscovered, this,
                        &bluetooth::connectedAndDiscovered);
                // connect(pafersTreadmill, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(pafersTreadmill, &paferstreadmill::debug, this, &bluetooth::debug);
                // connect(pafersTreadmill, SIGNAL(speedChanged(double)), this, SLOT(speedChanged(double)));
                // connect(pafersTreadmill, SIGNAL(inclinationChanged(double)), this,
                // SLOT(inclinationChanged(double)));
                pafersTreadmill->deviceDiscovered(b);
                this->signalBluetoothDeviceConnected(pafersTreadmill);
            } else if (b.name().toUpper().startsWith(QStringLiteral("BOWFLEX T")) && !bowflexT216Treadmill && filter) {
                this->setLastBluetoothDevice(b);
                this->stopDiscovery();
                bowflexT216Treadmill = new bowflext216treadmill(this->pollDeviceTime, noConsole, noHeartService);
                // stateFileRead();
                emit deviceConnected(b);
                connect(bowflexT216Treadmill, &bluetoothdevice::connectedAndDiscovered, this,
                        &bluetooth::connectedAndDiscovered);
                // connect(bowflexTreadmill, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(bowflexT216Treadmill, &bowflext216treadmill::debug, this, &bluetooth::debug);
                // connect(bowflexTreadmill, SIGNAL(speedChanged(double)), this, SLOT(speedChanged(double)));
                // connect(bowflexTreadmill, SIGNAL(inclinationChanged(double)), this,
                // SLOT(inclinationChanged(double)));
                bowflexT216Treadmill->deviceDiscovered(b);
                this->signalBluetoothDeviceConnected(bowflexT216Treadmill);
            } else if (b.name().toUpper().startsWith(QStringLiteral("CROSSROPE")) && !crossRope && filter) {
                this->setLastBluetoothDevice(b);
                this->stopDiscovery();
                crossRope = new crossrope(this->pollDeviceTime, noConsole, noHeartService);
                // stateFileRead();
                emit deviceConnected(b);
                connect(crossRope, &bluetoothdevice::connectedAndDiscovered, this,
                        &bluetooth::connectedAndDiscovered);
                // connect(crossRope, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(crossRope, &crossrope::debug, this, &bluetooth::debug);
                // connect(crossRope, SIGNAL(speedChanged(double)), this, SLOT(speedChanged(double)));
                // connect(crossRope, SIGNAL(inclinationChanged(double)), this,
                // SLOT(inclinationChanged(double)));
                crossRope->deviceDiscovered(b);
                this->signalBluetoothDeviceConnected(crossRope);
            } else if (b.name().toUpper().startsWith(QStringLiteral("NAUTILUS T")) && !nautilusTreadmill && filter) {
                this->setLastBluetoothDevice(b);
                this->stopDiscovery();
                nautilusTreadmill = new nautilustreadmill(this->pollDeviceTime, noConsole, noHeartService);
                // stateFileRead();
                emit deviceConnected(b);
                connect(nautilusTreadmill, &bluetoothdevice::connectedAndDiscovered, this,
                        &bluetooth::connectedAndDiscovered);
                // connect(nautilusTreadmill, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(nautilusTreadmill, &nautilustreadmill::debug, this, &bluetooth::debug);
                // connect(nautilusTreadmill, SIGNAL(speedChanged(double)), this, SLOT(speedChanged(double)));
                // connect(nautilusTreadmill, SIGNAL(inclinationChanged(double)), this,
                // SLOT(inclinationChanged(double)));
                nautilusTreadmill->deviceDiscovered(b);
                this->signalBluetoothDeviceConnected(nautilusTreadmill);
            } else if ((b.name().startsWith(QStringLiteral("Flywheel")) ||
                        // BIKE 1, BIKE 2, BIKE 3...
                        (b.name().toUpper().startsWith(QStringLiteral("BIKE")) && flywheel_life_fitness_ic8 == true &&
                         b.name().length() == 6)) &&
                       !flywheelBike && filter) {
                this->setLastBluetoothDevice(b);
                this->stopDiscovery();
                flywheelBike = new flywheelbike(noWriteResistance, noHeartService);
                // stateFileRead();
                emit deviceConnected(b);
                connect(flywheelBike, &bluetoothdevice::connectedAndDiscovered, this,
                        &bluetooth::connectedAndDiscovered);
                // connect(flywheelBike, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(flywheelBike, &flywheelbike::debug, this, &bluetooth::debug);
                // connect(echelonConnectSport, SIGNAL(speedChanged(double)), this, SLOT(speedChanged(double)));
                // connect(echelonConnectSport, SIGNAL(inclinationChanged(double)), this,
                // SLOT(inclinationChanged(double)));
                flywheelBike->deviceDiscovered(b);
                this->signalBluetoothDeviceConnected(flywheelBike);
            } else if ((b.name().toUpper().startsWith(QStringLiteral("MCF-"))) && !mcfBike && filter) {
                this->setLastBluetoothDevice(b);
                this->stopDiscovery();
                mcfBike = new mcfbike(noWriteResistance, noHeartService, bikeResistanceOffset, bikeResistanceGain);
                // stateFileRead();
                emit deviceConnected(b);
                connect(mcfBike, &bluetoothdevice::connectedAndDiscovered, this, &bluetooth::connectedAndDiscovered);
                // connect(mcfBike, SIGNAL(disconnected()), this, SLOT(restart()));
                // connect(mcfBike, &mcfbike::debug, this, &bluetooth::debug);
                // connect(mcfBike, SIGNAL(speedChanged(double)), this, SLOT(speedChanged(double)));
                // connect(mcfBike, SIGNAL(inclinationChanged(double)), this,
                // SLOT(inclinationChanged(double)));
                mcfBike->deviceDiscovered(b);
                this->signalBluetoothDeviceConnected(mcfBike);
            } else if ((b.name().startsWith(QStringLiteral("TRX ROUTE KEY")) ||
                        b.name().toUpper().startsWith(QStringLiteral("MASTERT40-")) ||
                        b.name().toUpper().startsWith(QStringLiteral("BH DUALKIT TREAD")) ||
                        b.name().toUpper().startsWith(QStringLiteral("BH-TR-"))) && !toorx && filter) {
                this->setLastBluetoothDevice(b);
                this->stopDiscovery();
                toorx = new toorxtreadmill();
                emit deviceConnected(b);
                connect(toorx, &bluetoothdevice::connectedAndDiscovered, this, &bluetooth::connectedAndDiscovered);
                // connect(toorx, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(toorx, &toorxtreadmill::debug, this, &bluetooth::debug);
                toorx->deviceDiscovered(b);
                this->signalBluetoothDeviceConnected(toorx);
            } else if (((b.name().toUpper().startsWith(QStringLiteral("BH DUALKIT")) && !b.name().toUpper().startsWith(QStringLiteral("BH DUALKIT TREAD"))) ||
                        b.name().toUpper().startsWith(QStringLiteral("BH-"))) && !iConceptBike &&
                       !iconcept_elliptical && filter) {
                this->setLastBluetoothDevice(b);
                this->stopDiscovery();
                iConceptBike = new iconceptbike();
                emit deviceConnected(b);
                connect(iConceptBike, &bluetoothdevice::connectedAndDiscovered, this,
                        &bluetooth::connectedAndDiscovered);
                // connect(toorx, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(iConceptBike, &iconceptbike::debug, this, &bluetooth::debug);
                iConceptBike->deviceDiscovered(b);
                this->signalBluetoothDeviceConnected(iConceptBike);
            } else if ((b.name().toUpper().startsWith(QStringLiteral("BH DUALKIT"))) && !iConceptElliptical &&
                       iconcept_elliptical && filter) {
                this->setLastBluetoothDevice(b);
                this->stopDiscovery();
                iConceptElliptical =
                    new iconceptelliptical(noWriteResistance, noHeartService, bikeResistanceOffset, bikeResistanceGain);
                emit deviceConnected(b);
                connect(iConceptElliptical, &bluetoothdevice::connectedAndDiscovered, this,
                        &bluetooth::connectedAndDiscovered);
                // connect(toorx, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(iConceptElliptical, &iconceptelliptical::debug, this, &bluetooth::debug);
                iConceptElliptical->deviceDiscovered(b);
                this->signalBluetoothDeviceConnected(iConceptElliptical);
            } else if ((b.name().toUpper().startsWith(QStringLiteral("XT385")) ||
                        (b.name().toUpper().startsWith(QStringLiteral("XT485"))  && !deviceHasService(b, QBluetoothUuid((quint16)0x1826))) ||
                        b.name().toUpper().startsWith(QStringLiteral("XT800")) ||
                        b.name().toUpper().startsWith(QStringLiteral("XT900"))) &&
                       !spiritTreadmill && filter) {
                this->setLastBluetoothDevice(b);
                this->stopDiscovery();
                spiritTreadmill = new spirittreadmill();
                emit deviceConnected(b);
                connect(spiritTreadmill, &bluetoothdevice::connectedAndDiscovered, this,
                        &bluetooth::connectedAndDiscovered);
                // connect(spiritTreadmill, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(spiritTreadmill, &spirittreadmill::debug, this, &bluetooth::debug);
                connect(spiritTreadmill, &spirittreadmill::inclinationChanged, this, &bluetooth::inclinationChanged);
                spiritTreadmill->deviceDiscovered(b);
                this->signalBluetoothDeviceConnected(spiritTreadmill);
            } else if (b.name().toUpper().startsWith(QStringLiteral("RUNNERT")) && !activioTreadmill && filter) {
                this->setLastBluetoothDevice(b);
                this->stopDiscovery();
                activioTreadmill = new activiotreadmill();
                emit deviceConnected(b);
                connect(activioTreadmill, &bluetoothdevice::connectedAndDiscovered, this,
                        &bluetooth::connectedAndDiscovered);
                // connect(activioTreadmill, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(activioTreadmill, &activiotreadmill::debug, this, &bluetooth::debug);
                activioTreadmill->deviceDiscovered(b);
                this->signalBluetoothDeviceConnected(activioTreadmill);
            } else if (((b.name().startsWith(QStringLiteral("TOORX"))) ||
                        (b.name().startsWith(QStringLiteral("V-RUN"))) ||
                        (b.name().toUpper().startsWith(QStringLiteral("K80_"))) ||
                        (b.name().toUpper().startsWith(QStringLiteral("I-CONSOLE+"))) ||
                        (b.name().toUpper().startsWith(QStringLiteral("ICONSOLE+"))) ||
                        (b.name().toUpper().startsWith(QStringLiteral("I-RUNNING"))) ||
                        (b.name().toUpper().startsWith(QStringLiteral("DKN RUN"))) ||
                        (b.name().toUpper().startsWith(QStringLiteral("ADIDAS "))) ||
                        (b.name().toUpper().startsWith(QStringLiteral("REEBOK")))) &&
                       !trxappgateusb && !trxappgateusbBike && !toorx_bike && !toorx_ftms && !toorx_ftms_treadmill && !iconsole_elliptical &&
                       filter) {
                this->setLastBluetoothDevice(b);
                this->stopDiscovery();
                trxappgateusb = new trxappgateusbtreadmill();
                emit deviceConnected(b);
                connect(trxappgateusb, &bluetoothdevice::connectedAndDiscovered, this,
                        &bluetooth::connectedAndDiscovered);
                // connect(trxappgateusb, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(trxappgateusb, &trxappgateusbtreadmill::debug, this, &bluetooth::debug);
                trxappgateusb->deviceDiscovered(b);
                this->signalBluetoothDeviceConnected(trxappgateusb);
            } else if ((b.name().toUpper().startsWith(QStringLiteral("TUN ")) ||
                        b.name().toUpper().startsWith(QStringLiteral("FITHIWAY")) ||
                        b.name().toUpper().startsWith(QStringLiteral("FIT HI WAY")) ||
                        b.name().toUpper().startsWith(QStringLiteral("BIKZU_")) ||
                        b.name().toUpper().startsWith(QStringLiteral("PASYOU-")) ||
                        b.name().toUpper().startsWith(QStringLiteral("VIRTUFIT")) ||
                        ((b.name().startsWith(QStringLiteral("TOORX")) ||
                          b.name().toUpper().startsWith(QStringLiteral("I-CONSOIE+")) ||
                          b.name().toUpper().startsWith(QStringLiteral("I-CONSOLE+")) ||
                          b.name().toUpper().startsWith(QStringLiteral("IBIKING+")) ||
                          b.name().toUpper().startsWith(QStringLiteral("ICONSOLE+")) ||
                          b.name().toUpper().startsWith(QStringLiteral("VIFHTR2.1")) ||
                          (b.name().toUpper().startsWith(QStringLiteral("REEBOK"))) ||
                          b.name().toUpper().contains(QStringLiteral("CR011R")) ||
                          b.name().toUpper().startsWith(QStringLiteral("DKN MOTION"))) &&
                         (toorx_bike))) &&
                       !trxappgateusb && !toorx_ftms && !toorx_ftms_treadmill && !trxappgateusbBike && filter && !iconsole_elliptical) {
                this->setLastBluetoothDevice(b);
                this->stopDiscovery();
                trxappgateusbBike =
                    new trxappgateusbbike(noWriteResistance, noHeartService, bikeResistanceOffset, bikeResistanceGain);
                emit deviceConnected(b);
                connect(trxappgateusbBike, &bluetoothdevice::connectedAndDiscovered, this,
                        &bluetooth::connectedAndDiscovered);
                // connect(trxappgateusb, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(trxappgateusbBike, &trxappgateusbbike::debug, this, &bluetooth::debug);
                trxappgateusbBike->deviceDiscovered(b);
                this->signalBluetoothDeviceConnected(trxappgateusbBike);
            } else if ((b.name().toUpper().startsWith(QStringLiteral("X-BIKE"))) && !ultraSportBike && filter) {
                this->setLastBluetoothDevice(b);
                this->stopDiscovery();
                ultraSportBike =
                    new ultrasportbike(noWriteResistance, noHeartService, bikeResistanceOffset, bikeResistanceGain);
                emit deviceConnected(b);
                connect(ultraSportBike, &bluetoothdevice::connectedAndDiscovered, this,
                        &bluetooth::connectedAndDiscovered);
                // connect(ultraSportBike, SIGNAL(disconnected()), this, SLOT(restart()));
                // connect(ultraSportBike, &solebike::debug, this, &bluetooth::debug);
                ultraSportBike->deviceDiscovered(b);
                this->signalBluetoothDeviceConnected(ultraSportBike);
            } else if ((b.name().toUpper().startsWith(QStringLiteral("KEEP_BIKE_")) ||
                        b.name().toUpper().startsWith(QStringLiteral("KEEP_CC_"))) && !keepBike && filter) {
                this->setLastBluetoothDevice(b);
                this->stopDiscovery();
                keepBike = new keepbike(noWriteResistance, noHeartService, bikeResistanceOffset, bikeResistanceGain);
                emit deviceConnected(b);
                connect(keepBike, &bluetoothdevice::connectedAndDiscovered, this, &bluetooth::connectedAndDiscovered);
                // connect(keepBike, SIGNAL(disconnected()), this, SLOT(restart()));
                // connect(keepBike, &solebike::debug, this, &bluetooth::debug);
                keepBike->deviceDiscovered(b);
                this->signalBluetoothDeviceConnected(keepBike);
            } else if ((b.name().toUpper().startsWith(QStringLiteral("LCB")) ||
                        b.name().toUpper().startsWith(QStringLiteral("R92"))) &&
                       !soleBike && filter) {
                this->setLastBluetoothDevice(b);
                this->stopDiscovery();
                soleBike = new solebike(noWriteResistance, noHeartService, bikeResistanceOffset, bikeResistanceGain);
                emit deviceConnected(b);
                connect(soleBike, &bluetoothdevice::connectedAndDiscovered, this, &bluetooth::connectedAndDiscovered);
                // connect(soleBike, SIGNAL(disconnected()), this, SLOT(restart()));
                // connect(soleBike, &solebike::debug, this, &bluetooth::debug);
                soleBike->deviceDiscovered(b);
                this->signalBluetoothDeviceConnected(soleBike);
            } else if ((b.name().toUpper().startsWith(QStringLiteral("BFCP")) ||
                        (b.name().toUpper().startsWith(QStringLiteral("HT")) && (b.name().length() == 11 || b.name().length() == 12))) &&
                       !skandikaWiriBike && filter) {
                this->setLastBluetoothDevice(b);
                this->stopDiscovery();
                skandikaWiriBike =
                    new skandikawiribike(noWriteResistance, noHeartService, bikeResistanceOffset, bikeResistanceGain);
                emit deviceConnected(b);
                connect(skandikaWiriBike, &bluetoothdevice::connectedAndDiscovered, this,
                        &bluetooth::connectedAndDiscovered);
                // connect(skandikaWiriBike, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(skandikaWiriBike, &skandikawiribike::debug, this, &bluetooth::debug);
                skandikaWiriBike->deviceDiscovered(b);
                this->signalBluetoothDeviceConnected(skandikaWiriBike);
            } else if (((b.name().toUpper().startsWith("RQ") && b.name().length() == 5) ||
                        (b.name().toUpper().startsWith("R-Q") && b.name().length() > 6 && !power_as_bike) ||
                        (b.name().toUpper().startsWith("SCH130")) || // not a renpho bike an FTMS one
                        ((b.name().startsWith(QStringLiteral("TOORX"))) && toorx_ftms && !toorx_ftms_treadmill)) &&
                       !renphoBike && !snodeBike && !fitPlusBike && filter) {
                this->setLastBluetoothDevice(b);
                this->stopDiscovery();
                renphoBike = new renphobike(noWriteResistance, noHeartService);
                emit(deviceConnected(b));
                connect(renphoBike, SIGNAL(connectedAndDiscovered()), this, SLOT(connectedAndDiscovered()));
                // connect(trxappgateusb, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(renphoBike, SIGNAL(debug(QString)), this, SLOT(debug(QString)));
                renphoBike->deviceDiscovered(b);
                this->signalBluetoothDeviceConnected(renphoBike);
            } else if ((b.name().toUpper().startsWith("PAFERS_")) && !pafersBike && !pafers_treadmill && filter) {
                this->setLastBluetoothDevice(b);
                this->stopDiscovery();
                pafersBike =
                    new pafersbike(noWriteResistance, noHeartService, bikeResistanceOffset, bikeResistanceGain);
                emit(deviceConnected(b));
                connect(pafersBike, SIGNAL(connectedAndDiscovered()), this, SLOT(connectedAndDiscovered()));
                // connect(pafersBike, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(pafersBike, SIGNAL(debug(QString)), this, SLOT(debug(QString)));
                pafersBike->deviceDiscovered(b);
                this->signalBluetoothDeviceConnected(pafersBike);
            } else if (((b.name().startsWith(QStringLiteral("FS-")) && snode_bike) ||
                        (b.name().toUpper().startsWith(QStringLiteral("TF-")) &&
                         !horizon_treadmill_force_ftms)) && // TF-769DF2
                       !snodeBike &&
                       !ftmsBike && !fitPlusBike && filter) {
                this->setLastBluetoothDevice(b);
                this->stopDiscovery();
                snodeBike = new snodebike(noWriteResistance, noHeartService);
                emit deviceConnected(b);
                connect(snodeBike, &bluetoothdevice::connectedAndDiscovered, this, &bluetooth::connectedAndDiscovered);
                // connect(trxappgateusb, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(snodeBike, &snodebike::debug, this, &bluetooth::debug);
                snodeBike->deviceDiscovered(b);
                this->signalBluetoothDeviceConnected(snodeBike);
            } else if (((b.name().startsWith(QStringLiteral("FS-")) && fitplus_bike) ||
                        (b.name().toUpper().startsWith("H9110 OSAKA")) ||
                        b.name().startsWith(QStringLiteral("MRK-"))) &&
                       !fitPlusBike && !ftmsBike && !ftmsRower && !snodeBike && filter) {
                this->setLastBluetoothDevice(b);
                this->stopDiscovery();
                fitPlusBike =
                    new fitplusbike(noWriteResistance, noHeartService, bikeResistanceOffset, bikeResistanceGain);
                emit deviceConnected(b);
                connect(fitPlusBike, &bluetoothdevice::connectedAndDiscovered, this,
                        &bluetooth::connectedAndDiscovered);
                // connect(fitPlusBike, SIGNAL(disconnected()), this, SLOT(restart()));
                // NOTE: Commented due to #358
                // connect(fitPlusBike, SIGNAL(debug(QString)), this, SLOT(debug(QString)));
                fitPlusBike->deviceDiscovered(b);
                this->signalBluetoothDeviceConnected(fitPlusBike);
            } else if (b.name().toUpper().startsWith(QStringLiteral("EW-TM-")) &&
                       !focusTreadmill && filter) {
                this->setLastBluetoothDevice(b);
                this->stopDiscovery();
                focusTreadmill = new focustreadmill(this->pollDeviceTime, noConsole, noHeartService);
                emit deviceConnected(b);
                connect(focusTreadmill, &bluetoothdevice::connectedAndDiscovered, this,
                        &bluetooth::connectedAndDiscovered);
                connect(focusTreadmill, &focustreadmill::debug, this, &bluetooth::debug);
                focusTreadmill->deviceDiscovered(b);
                connect(this, &bluetooth::searchingStop, focusTreadmill, &focustreadmill::searchingStop);
                if (this->discoveryAgent && !this->discoveryAgent->isActive())
                    emit searchingStop();
                this->signalBluetoothDeviceConnected(focusTreadmill);
            } else if (((b.name().startsWith(QStringLiteral("FS-")) && !horizonTreadmill && !snode_bike && !fitplus_bike && !ftmsBike && !iconsole_elliptical) ||
                        (b.name().toUpper().startsWith(QStringLiteral("NOBLEPRO CONNECT")) && !deviceHasService(b, QBluetoothUuid((quint16)0x1826))) || // FTMS
                        (b.name().startsWith(QStringLiteral("SW")) && b.name().length() == 14 &&
                         !b.name().contains('(') && !b.name().contains(')') && !deviceHasService(b, QBluetoothUuid((quint16)0x1826))) ||
                        (b.name().toUpper().startsWith(QStringLiteral("WINFITA"))) || //  also FTMS
                        (b.name().toUpper().startsWith(QStringLiteral("SW-BLE"))) ||       // FTMS
                        (b.name().startsWith(QStringLiteral("BF70")))) &&
                       !fitshowTreadmill && !iconsole_elliptical && !horizonTreadmill && filter) {
                this->setLastBluetoothDevice(b);
                this->stopDiscovery();
                fitshowTreadmill = new fitshowtreadmill(this->pollDeviceTime, noConsole, noHeartService);
                emit deviceConnected(b);
                connect(fitshowTreadmill, &bluetoothdevice::connectedAndDiscovered, this,
                        &bluetooth::connectedAndDiscovered);
                connect(fitshowTreadmill, &fitshowtreadmill::debug, this, &bluetooth::debug);
                fitshowTreadmill->deviceDiscovered(b);
                connect(this, &bluetooth::searchingStop, fitshowTreadmill, &fitshowtreadmill::searchingStop);
                if (this->discoveryAgent && !this->discoveryAgent->isActive())
                    emit searchingStop();
                this->signalBluetoothDeviceConnected(fitshowTreadmill);
            } else if (b.name().toUpper().startsWith(QStringLiteral("IC")) && b.name().length() == 8 && !inspireBike &&
                       filter) {
                this->setLastBluetoothDevice(b);
                this->stopDiscovery();
                inspireBike = new inspirebike(noWriteResistance, noHeartService);
#if !defined(Q_OS_ANDROID) && !defined(Q_OS_IOS)
                stateFileRead();
#endif
                emit deviceConnected(b);
                connect(inspireBike, &bluetoothdevice::connectedAndDiscovered, this,
                        &bluetooth::connectedAndDiscovered);
                connect(inspireBike, &inspirebike::debug, this, &bluetooth::debug);
                // NOTE: Commented due to #358
                // connect(inspireBike, SIGNAL(speedChanged(double)), this, SLOT(speedChanged(double)));
                // NOTE: Commented due to #358
                // connect(inspireBike, SIGNAL(inclinationChanged(double)), this, SLOT(inclinationChanged(double)));
                inspireBike->deviceDiscovered(b);
                // NOTE: Commented due to #358
                // connect(this, SIGNAL(searchingStop()), inspireBike, SLOT(searchingStop()));
                if (this->discoveryAgent && !this->discoveryAgent->isActive()) {
                    emit searchingStop();
                }
                this->signalBluetoothDeviceConnected(inspireBike);
            } else if (b.name().toUpper().startsWith(QStringLiteral("CHRONO ")) && !chronoBike && filter) {
                this->setLastBluetoothDevice(b);
                this->stopDiscovery();
                chronoBike = new chronobike(noWriteResistance, noHeartService);
#if !defined(Q_OS_ANDROID) && !defined(Q_OS_IOS)
                stateFileRead();
#endif
                emit deviceConnected(b);
                connect(chronoBike, &bluetoothdevice::connectedAndDiscovered, this, &bluetooth::connectedAndDiscovered);
                connect(chronoBike, &chronobike::debug, this, &bluetooth::debug);
                // NOTE: Commented due to #358
                // connect(chronoBike, SIGNAL(speedChanged(double)), this, SLOT(speedChanged(double)));
                // NOTE: Commented due to #358
                // connect(chronoBike, SIGNAL(inclinationChanged(double)), this, SLOT(inclinationChanged(double)));
                chronoBike->deviceDiscovered(b);
                // NOTE: Commented due to #358
                // connect(this, SIGNAL(searchingStop()), chronoBike, SLOT(searchingStop()));
                if (this->discoveryAgent && !this->discoveryAgent->isActive()) {
                    emit searchingStop();
                }
                this->signalBluetoothDeviceConnected(chronoBike);
            }
        }
    }
}

void bluetooth::connectedAndDiscovered() {

    qDebug() << "bluetooth::connectedAndDiscovered()";

    static bool firstConnected = true;
    QSettings settings;
    QString heartRateBeltName =
        settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name).toString();
    QString ftmsAccessoryName =
        settings.value(QZSettings::ftms_accessory_name, QZSettings::default_ftms_accessory_name).toString();
    bool csc_as_bike =
        settings.value(QZSettings::cadence_sensor_as_bike, QZSettings::default_cadence_sensor_as_bike).toBool();
    QString cscName =
        settings.value(QZSettings::cadence_sensor_name, QZSettings::default_cadence_sensor_name).toString();
    bool power_as_bike =
        settings.value(QZSettings::power_sensor_as_bike, QZSettings::default_power_sensor_as_bike).toBool();
    bool power_as_treadmill =
        settings.value(QZSettings::power_sensor_as_treadmill, QZSettings::default_power_sensor_as_treadmill).toBool();
    QString powerSensorName =
        settings.value(QZSettings::power_sensor_name, QZSettings::default_power_sensor_name).toString();
    QString eliteRizerName =
        settings.value(QZSettings::elite_rizer_name, QZSettings::default_elite_rizer_name).toString();
    QString eliteSterzoSmartName =
        settings.value(QZSettings::elite_sterzo_smart_name, QZSettings::default_elite_sterzo_smart_name).toString();
    bool fitmetriaFanfitEnabled =
        settings.value(QZSettings::fitmetria_fanfit_enable, QZSettings::default_fitmetria_fanfit_enable).toBool();

    // only at the first very connection, setting the user default resistance
    if (device() && firstConnected && device()->deviceType() == bluetoothdevice::BIKE &&
        settings.value(QZSettings::bike_resistance_start, QZSettings::default_bike_resistance_start).toUInt() != 1) {
        qobject_cast<bike *>(device())->changeResistance(
            settings.value(QZSettings::bike_resistance_start, QZSettings::default_bike_resistance_start).toUInt());
    } else if (device() && firstConnected && device()->deviceType() == bluetoothdevice::ELLIPTICAL &&
               settings.value(QZSettings::bike_resistance_start, QZSettings::default_bike_resistance_start).toUInt() !=
                   1) {
        qobject_cast<elliptical *>(device())->changeResistance(
            settings.value(QZSettings::bike_resistance_start, QZSettings::default_bike_resistance_start).toUInt());
    }

    if (heartRateBeltName.startsWith(QStringLiteral("Disabled"))) {
        if (!settings.value(QZSettings::hrm_lastdevice_name, QZSettings::default_hrm_lastdevice_name)
                 .toString()
                 .isEmpty()) {
            settings.setValue(QZSettings::hrm_lastdevice_name, "");
        }
        if (!settings.value(QZSettings::hrm_lastdevice_address, QZSettings::default_hrm_lastdevice_address)
                 .toString()
                 .isEmpty()) {
            settings.setValue(QZSettings::hrm_lastdevice_address, "");
        }
    }

    if (this->device() != nullptr) {

#ifdef Q_OS_IOS
        if (settings.value(QZSettings::ios_cache_heart_device, QZSettings::default_ios_cache_heart_device).toBool()) {
            QString heartRateBeltName =
                settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name).toString();
            QString b =
                settings.value(QZSettings::hrm_lastdevice_name, QZSettings::default_hrm_lastdevice_name).toString();
            qDebug() << "last hrm name" << b;
            if (!b.compare(heartRateBeltName) && b.length()) {

                heartRateBelt = new heartratebelt();
                // connect(heartRateBelt, SIGNAL(disconnected()), this, SLOT(restart()));

                connect(heartRateBelt, SIGNAL(debug(QString)), this, SLOT(debug(QString)));
                connect(heartRateBelt, SIGNAL(heartRate(uint8_t)), this->device(), SLOT(heartRate(uint8_t)));
                QBluetoothDeviceInfo bt;
                bt.setDeviceUuid(QBluetoothUuid(
                    settings.value(QZSettings::hrm_lastdevice_address, QZSettings::default_hrm_lastdevice_address)
                        .toString()));
                qDebug() << "UUID" << bt.deviceUuid();
                heartRateBelt->deviceDiscovered(bt);
            }
        }
#endif
        for (const QBluetoothDeviceInfo &b : qAsConst(devices)) {
            if (((b.name().startsWith(heartRateBeltName))) && !heartRateBelt &&
                !heartRateBeltName.startsWith(QStringLiteral("Disabled"))) {
                settings.setValue(QZSettings::hrm_lastdevice_name, b.name());

#ifndef Q_OS_IOS
                settings.setValue(QZSettings::hrm_lastdevice_address, b.address().toString());
#else
                settings.setValue(QZSettings::hrm_lastdevice_address, b.deviceUuid().toString());
#endif
                heartRateBelt = new heartratebelt();
                // connect(heartRateBelt, SIGNAL(disconnected()), this, SLOT(restart()));

                connect(heartRateBelt, &heartratebelt::debug, this, &bluetooth::debug);
                connect(heartRateBelt, &heartratebelt::heartRate, this->device(), &bluetoothdevice::heartRate);
                heartRateBelt->deviceDiscovered(b);
                if(homeform::singleton())
                    homeform::singleton()->setToastRequested(b.name() + " (HR sensor) connected!");
                break;
            }
        }

        for (const QBluetoothDeviceInfo &b : qAsConst(devices)) {
            if (((b.name().startsWith(ftmsAccessoryName))) && !ftmsAccessory &&
                !ftmsAccessoryName.startsWith(QStringLiteral("Disabled")) &&
                !settings.value(QZSettings::ss2k_peloton, QZSettings::default_ss2k_peloton).toBool()) {
                settings.setValue(QZSettings::ftms_accessory_lastdevice_name, b.name());

#ifndef Q_OS_IOS
                settings.setValue(QZSettings::ftms_accessory_address, b.address().toString());
#else
                settings.setValue(QZSettings::ftms_accessory_address, b.deviceUuid().toString());
#endif
                ftmsAccessory = new smartspin2k(false, false, this->device()->maxResistance(), (bike *)this->device());
                // connect(heartRateBelt, SIGNAL(disconnected()), this, SLOT(restart()));

                connect(ftmsAccessory, &smartspin2k::debug, this, &bluetooth::debug);

                connect(this->device(), SIGNAL(resistanceChanged(resistance_t)), ftmsAccessory,
                        SLOT(changeResistance(resistance_t)));
                connect(this->device(), SIGNAL(resistanceRead(resistance_t)), ftmsAccessory,
                        SLOT(resistanceReadFromTheBike(resistance_t)));
                connect(ftmsAccessory, SIGNAL(resistanceRead(resistance_t)), this->device(),
                        SLOT(resistanceFromFTMSAccessory(resistance_t)));
                emit ftmsAccessoryConnected(ftmsAccessory);
                ftmsAccessory->deviceDiscovered(b);
                break;
            }
        }

        if (fitmetriaFanfitEnabled) {
            for (const QBluetoothDeviceInfo &b : qAsConst(devices)) {
                if (((b.name().startsWith("FITFAN-"))) && !fitmetria_fanfit_isconnected(b.name())) {
                    fitmetria_fanfit *f = new fitmetria_fanfit(this->device());

                    connect(f, &fitmetria_fanfit::debug, this, &bluetooth::debug);

                    connect(this->device(), SIGNAL(fanSpeedChanged(uint8_t)), f, SLOT(fanSpeedRequest(uint8_t)));

                    f->deviceDiscovered(b);
                    fitmetriaFanfit.append(f);
                    break;
                } else if (((b.name().toUpper().startsWith("HEADWIND "))) && !fitmetria_fanfit_isconnected(b.name())) {
                    wahookickrheadwind *f = new wahookickrheadwind(this->device());

                    connect(f, &wahookickrheadwind::debug, this, &bluetooth::debug);

                    connect(this->device(), SIGNAL(fanSpeedChanged(uint8_t)), f, SLOT(fanSpeedRequest(uint8_t)));

                    f->deviceDiscovered(b);
                    wahookickrHeadWind.append(f);
                    break;
                } else if (((b.name().toUpper().startsWith("ARIA")) && b.name().length() == 4) && !fitmetria_fanfit_isconnected(b.name())) {
                    eliteariafan *f = new eliteariafan(this->device());

                    connect(f, &eliteariafan::debug, this, &bluetooth::debug);

                    connect(this->device(), SIGNAL(fanSpeedChanged(uint8_t)), f, SLOT(fanSpeedRequest(uint8_t)));

                    f->deviceDiscovered(b);
                    eliteAriaFan.append(f);
                    break;
                }
            }
        }

        if (!csc_as_bike) {
            for (const QBluetoothDeviceInfo &b : qAsConst(devices)) {
                if (((b.name().startsWith(cscName))) && !cadenceSensor &&
                    !cscName.startsWith(QStringLiteral("Disabled"))) {
                    settings.setValue(QZSettings::csc_sensor_lastdevice_name, b.name());

#ifndef Q_OS_IOS
                    settings.setValue(QZSettings::csc_sensor_address, b.address().toString());
#else
                    settings.setValue(QZSettings::csc_sensor_address, b.deviceUuid().toString());
#endif
                    cadenceSensor = new cscbike(false, false, true);
                    // connect(heartRateBelt, SIGNAL(disconnected()), this, SLOT(restart()));

                    connect(cadenceSensor, &cscbike::debug, this, &bluetooth::debug);
                    connect(cadenceSensor, &bluetoothdevice::cadenceChanged, this->device(),
                            &bluetoothdevice::cadenceSensor);
                    cadenceSensor->deviceDiscovered(b);
                    if(homeform::singleton())
                        homeform::singleton()->setToastRequested(b.name() + " (cadence sensor) connected!");
                    break;
                }
            }
        }
    }

    if (!power_as_bike && !power_as_treadmill) {
        for (const QBluetoothDeviceInfo &b : qAsConst(devices)) {
            if (((b.name().startsWith(powerSensorName))) && !powerSensor && !powerSensorRun &&
                !powerSensorName.startsWith(QStringLiteral("Disabled"))) {
                settings.setValue(QZSettings::power_sensor_lastdevice_name, b.name());

#ifndef Q_OS_IOS
                settings.setValue(QZSettings::power_sensor_address, b.address().toString());
#else
                settings.setValue(QZSettings::power_sensor_address, b.deviceUuid().toString());
#endif
                if (device() && device()->deviceType() == bluetoothdevice::BIKE) {
                    powerSensor = new stagesbike(false, false, true);
                    // connect(heartRateBelt, SIGNAL(disconnected()), this, SLOT(restart()));

                    connect(powerSensor, &stagesbike::debug, this, &bluetooth::debug);
                    connect(powerSensor, &bluetoothdevice::powerChanged, this->device(), &bluetoothdevice::powerSensor);
                    powerSensor->deviceDiscovered(b);
                } else if (device() && device()->deviceType() == bluetoothdevice::TREADMILL) {
                    powerSensorRun = new strydrunpowersensor(false, false, true);
                    // connect(heartRateBelt, SIGNAL(disconnected()), this, SLOT(restart()));

                    connect(powerSensorRun, &strydrunpowersensor::onHeartRate, this->device(),
                            &bluetoothdevice::heartRate);
                    connect(powerSensorRun, &strydrunpowersensor::debug, this, &bluetooth::debug);
                    connect(powerSensorRun, &bluetoothdevice::powerChanged, this->device(),
                            &bluetoothdevice::powerSensor);
                    connect(powerSensorRun, &bluetoothdevice::cadenceChanged, this->device(),
                            &bluetoothdevice::cadenceSensor);
                    connect(powerSensorRun, &bluetoothdevice::speedChanged, this->device(),
                            &bluetoothdevice::speedSensor);
                    connect(powerSensorRun, &bluetoothdevice::instantaneousStrideLengthChanged, this->device(),
                            &bluetoothdevice::instantaneousStrideLengthSensor);
                    connect(powerSensorRun, &bluetoothdevice::groundContactChanged, this->device(),
                            &bluetoothdevice::groundContactSensor);
                    connect(powerSensorRun, &bluetoothdevice::verticalOscillationChanged, this->device(),
                            &bluetoothdevice::verticalOscillationSensor);
                    powerSensorRun->deviceDiscovered(b);
                }

                if(homeform::singleton())
                    homeform::singleton()->setToastRequested(b.name() + " (power sensor) connected!");

                break;
            }
        }
    }

    for (const QBluetoothDeviceInfo &b : qAsConst(devices)) {
        if (((b.name().startsWith(eliteRizerName))) && !eliteRizer &&
            !eliteRizerName.startsWith(QStringLiteral("Disabled"))) {
            settings.setValue(QZSettings::elite_rizer_lastdevice_name, b.name());

#ifndef Q_OS_IOS
            settings.setValue(QZSettings::elite_rizer_address, b.address().toString());
#else
            settings.setValue(QZSettings::elite_rizer_address, b.deviceUuid().toString());
#endif
            eliteRizer = new eliterizer(false, false);
            // connect(heartRateBelt, SIGNAL(disconnected()), this, SLOT(restart()));

            connect(eliteRizer, &eliterizer::debug, this, &bluetooth::debug);
            connect(eliteRizer, &eliterizer::steeringAngleChanged, (bike *)this->device(), &bike::changeSteeringAngle);
            connect(this->device(), &bluetoothdevice::inclinationChanged, eliteRizer,
                    &eliterizer::changeInclinationRequested);
            eliteRizer->deviceDiscovered(b);
            break;
        }
    }

    for (const QBluetoothDeviceInfo &b : qAsConst(devices)) {
        if (((b.name().startsWith(eliteSterzoSmartName))) && !eliteSterzoSmart &&
            !eliteSterzoSmartName.startsWith(QStringLiteral("Disabled")) && this->device() &&
            this->device()->deviceType() == bluetoothdevice::BIKE) {
            settings.setValue(QZSettings::elite_sterzo_smart_lastdevice_name, b.name());

#ifndef Q_OS_IOS
            settings.setValue(QZSettings::elite_sterzo_smart_address, b.address().toString());
#else
            settings.setValue(QZSettings::elite_sterzo_smart_address, b.deviceUuid().toString());
#endif
            eliteSterzoSmart = new elitesterzosmart(false, false);
            // connect(heartRateBelt, SIGNAL(disconnected()), this, SLOT(restart()));

            connect(eliteSterzoSmart, &elitesterzosmart::debug, this, &bluetooth::debug);
            connect(eliteSterzoSmart, &eliterizer::steeringAngleChanged, (bike *)this->device(),
                    &bike::changeSteeringAngle);
            eliteSterzoSmart->deviceDiscovered(b);
            break;
        }
    }

    if(settings.value(QZSettings::sram_axs_controller, QZSettings::default_sram_axs_controller).toBool()) {
        for (const QBluetoothDeviceInfo &b : qAsConst(devices)) {
            if (((b.name().toUpper().startsWith("SRAM "))) && !sramAXSController && this->device() &&
                    this->device()->deviceType() == bluetoothdevice::BIKE) {

                sramAXSController = new sramaxscontroller();
                // connect(heartRateBelt, SIGNAL(disconnected()), this, SLOT(restart()));

                connect(sramAXSController, &sramaxscontroller::debug, this, &bluetooth::debug);
                connect(sramAXSController, &sramaxscontroller::plus, (bike*)this->device(), &bike::gearUp);
                connect(sramAXSController, &sramaxscontroller::minus, (bike*)this->device(), &bike::gearDown);
                sramAXSController->deviceDiscovered(b);
                if(homeform::singleton())
                    homeform::singleton()->setToastRequested("SRAM Connected!");
                break;
            }
        }
    }

    if(settings.value(QZSettings::zwift_click, QZSettings::default_zwift_click).toBool()) {
        for (const QBluetoothDeviceInfo &b : qAsConst(devices)) {
            if (((b.name().toUpper().startsWith("ZWIFT CLICK"))) && !zwiftClickRemote && this->device() &&
                    this->device()->deviceType() == bluetoothdevice::BIKE) {

                if(b.manufacturerData(2378).size() > 0) {
                    qDebug() << "this should be 9. is it? " << int(b.manufacturerData(2378).at(0));
                } else {
                    qDebug() << "manufacturer not found for ZWIFT CLICK";
                }

                zwiftClickRemote = new zwiftclickremote(this->device(), AbstractZapDevice::ZWIFT_PLAY_TYPE::NONE);
                // connect(heartRateBelt, SIGNAL(disconnected()), this, SLOT(restart()));

                connect(zwiftClickRemote, &zwiftclickremote::debug, this, &bluetooth::debug);
                connect(zwiftClickRemote->playDevice, &ZwiftPlayDevice::plus, (bike*)this->device(), &bike::gearUp);
                connect(zwiftClickRemote->playDevice, &ZwiftPlayDevice::minus, (bike*)this->device(), &bike::gearDown);
                zwiftClickRemote->deviceDiscovered(b);
                if(homeform::singleton())
                    homeform::singleton()->setToastRequested("Zwift Click Connected!");
                break;
            }
        }
    }

    if(settings.value(QZSettings::zwift_play, QZSettings::default_zwift_play).toBool()) {
        for (const QBluetoothDeviceInfo &b : qAsConst(devices)) {
            if ((((b.name().toUpper().startsWith("ZWIFT PLAY"))) || b.name().toUpper().startsWith("ZWIFT RIDE") || b.name().toUpper().startsWith("ZWIFT SF2")) && zwiftPlayDevice.size() < 2 && this->device() &&
                    this->device()->deviceType() == bluetoothdevice::BIKE) {

                if(b.manufacturerData(2378).size() > 0) {
                    qDebug() << "this should be 3 or 2. is it? " << int(b.manufacturerData(2378).at(0));
                    zwiftPlayDevice.append(new zwiftclickremote(this->device(),
                                     int(b.manufacturerData(2378).at(0)) == 3 ? AbstractZapDevice::ZWIFT_PLAY_TYPE::LEFT : AbstractZapDevice::ZWIFT_PLAY_TYPE::RIGHT));
                } else {
                    qDebug() << "manufacturer not found for ZWIFT CLICK";
                    zwiftPlayDevice.append(new zwiftclickremote(this->device(),
                                     zwiftPlayDevice.length() == 0 ? AbstractZapDevice::ZWIFT_PLAY_TYPE::LEFT : AbstractZapDevice::ZWIFT_PLAY_TYPE::RIGHT));

                }
                // connect(heartRateBelt, SIGNAL(disconnected()), this, SLOT(restart()));

                connect(zwiftPlayDevice.last(), &zwiftclickremote::debug, this, &bluetooth::debug);
                connect(zwiftPlayDevice.last()->playDevice, &ZwiftPlayDevice::plus, (bike*)this->device(), &bike::gearUp);
                connect(zwiftPlayDevice.last()->playDevice, &ZwiftPlayDevice::minus, (bike*)this->device(), &bike::gearDown);
                zwiftPlayDevice.last()->deviceDiscovered(b);
                if(homeform::singleton())
                    homeform::singleton()->setToastRequested("Zwift Play/Ride Connected!");
            }
        }
    }
#ifdef Q_OS_ANDROID
    if (settings.value(QZSettings::ant_cadence, QZSettings::default_ant_cadence).toBool() ||
        settings.value(QZSettings::ant_heart, QZSettings::default_ant_heart).toBool()) {
        QAndroidJniObject activity = QAndroidJniObject::callStaticObjectMethod("org/qtproject/qt5/android/QtNative",
                                                                               "activity", "()Landroid/app/Activity;");
        KeepAwakeHelper::antObject(true)->callMethod<void>(
            "antStart", "(Landroid/app/Activity;ZZZZ)V", activity.object<jobject>(),
            settings.value(QZSettings::ant_cadence, QZSettings::default_ant_cadence).toBool(),
            settings.value(QZSettings::ant_heart, QZSettings::default_ant_heart).toBool(),
            settings.value(QZSettings::ant_garmin, QZSettings::default_ant_garmin).toBool(),
            device()->deviceType() == bluetoothdevice::TREADMILL ||
                device()->deviceType() == bluetoothdevice::ELLIPTICAL);
    }

    if (settings.value(QZSettings::android_notification, QZSettings::default_android_notification).toBool()) {
        QAndroidJniObject javaNotification = QAndroidJniObject::fromString("QZ is running!");
        QAndroidJniObject::callStaticMethod<void>(
            "org/cagnulen/qdomyoszwift/NotificationClient", "notify", "(Landroid/content/Context;Ljava/lang/String;)V",
            QtAndroid::androidContext().object(), javaNotification.object<jstring>());
    }
#endif

#ifdef Q_OS_ANDROID
    if (settings.value(QZSettings::peloton_workout_ocr, QZSettings::default_peloton_workout_ocr).toBool() ||
        settings.value(QZSettings::peloton_bike_ocr, QZSettings::default_peloton_bike_ocr).toBool() ||
        settings.value(QZSettings::zwift_ocr, QZSettings::default_zwift_ocr).toBool()) {
        AndroidActivityResultReceiver *a = new AndroidActivityResultReceiver();
        QAndroidJniObject MediaProjectionManager = QtAndroid::androidActivity().callObjectMethod(
            "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;",
            QAndroidJniObject::fromString("media_projection").object<jstring>());
        QAndroidJniObject intent =
            MediaProjectionManager.callObjectMethod("createScreenCaptureIntent", "()Landroid/content/Intent;");
        QtAndroid::startActivity(intent, 100, a);
    }
#endif

#if defined(Q_OS_ANDROID) || defined(Q_OS_IOS)
    if (settings.value(QZSettings::garmin_companion, QZSettings::default_garmin_companion).toBool()) {
#ifdef Q_OS_ANDROID
        QAndroidJniObject::callStaticMethod<void>("org/cagnulen/qdomyoszwift/Garmin", "init",
                                                  "(Landroid/content/Context;)V", QtAndroid::androidContext().object());
#else
#ifndef IO_UNDER_QT
        if (!h) {
            h = new lockscreen();
            h->garminconnect_init();
        }
#endif
#endif
    }
#endif

#ifdef Q_OS_IOS
    // in order to allow to populate the tiles with the IC BIKE auto connect feature
    if (firstConnected) {
        QBluetoothDeviceInfo bt;
        QString b = settings.value(QZSettings::bluetooth_lastdevice_name, QZSettings::default_bluetooth_lastdevice_name)
                        .toString();
        bt.setDeviceUuid(QBluetoothUuid(
            settings.value(QZSettings::bluetooth_lastdevice_address, QZSettings::default_bluetooth_lastdevice_address)
                .toString()));
        // set name method doesn't exist
        emit(deviceConnected(bt));
    }
#endif

    firstConnected = false;
}

void bluetooth::heartRate(uint8_t heart) { Q_UNUSED(heart) }

void bluetooth::restart() {

    QSettings settings;

    if (onlyDiscover) {

        onlyDiscover = false;
        this->startDiscovery();
        return;
    }

    if (settings.value(QZSettings::bluetooth_no_reconnection, QZSettings::default_bluetooth_no_reconnection).toBool()) {
        exit(EXIT_SUCCESS);
    }

    devices.clear();

    emit this->bluetoothDeviceDisconnected();

    if (domyos) {

        delete domyos;
        domyos = nullptr;
    }
    if (m3iBike) {

        delete m3iBike;
        m3iBike = nullptr;
    }
    if (fitshowTreadmill) {

        delete fitshowTreadmill;
        fitshowTreadmill = nullptr;
    }
    if (focusTreadmill) {

        delete focusTreadmill;
        focusTreadmill = nullptr;
    }
    if (horizonTreadmill) {

        delete horizonTreadmill;
        horizonTreadmill = nullptr;
    }
    if (lifefitnessTreadmill) {

        delete lifefitnessTreadmill;
        lifefitnessTreadmill = nullptr;
    }
    if (technogymmyrunTreadmill) {

        delete technogymmyrunTreadmill;
        technogymmyrunTreadmill = nullptr;
    }
#ifndef Q_OS_IOS
    if (technogymmyrunrfcommTreadmill) {

        delete technogymmyrunrfcommTreadmill;
        technogymmyrunrfcommTreadmill = nullptr;
    }
#endif
    if (soleF80) {

        delete soleF80;
        soleF80 = nullptr;
    }
    if (kingsmithR2Treadmill) {

        delete kingsmithR2Treadmill;
        kingsmithR2Treadmill = nullptr;
    }
    if (kingsmithR1ProTreadmill) {

        delete kingsmithR1ProTreadmill;
        kingsmithR1ProTreadmill = nullptr;
    }
    if (shuaA5Treadmill) {

        delete shuaA5Treadmill;
        shuaA5Treadmill = nullptr;
    }
    if (trueTreadmill) {

        delete trueTreadmill;
        trueTreadmill = nullptr;
    }
    if (domyosBike) {

        delete domyosBike;
        domyosBike = nullptr;
    }
    if (domyosRower) {

        delete domyosRower;
        domyosRower = nullptr;
    }
    if (apexBike) {
        delete apexBike;
        apexBike = nullptr;
    }
    if (bkoolBike) {
        delete bkoolBike;
        bkoolBike = nullptr;
    }
    if (domyosElliptical) {

        delete domyosElliptical;
        domyosElliptical = nullptr;
    }
    if (ypooElliptical) {

        delete ypooElliptical;
        ypooElliptical = nullptr;
    }
    if (soleElliptical) {

        delete soleElliptical;
        soleElliptical = nullptr;
    }
    if (nautilusElliptical) {

        delete nautilusElliptical;
        nautilusElliptical = nullptr;
    }
    if (nautilusBike) {

        delete nautilusBike;
        nautilusBike = nullptr;
    }
    if (bhFitnessElliptical) {

        delete bhFitnessElliptical;
        bhFitnessElliptical = nullptr;
    }
    if (cscBike) {

        delete cscBike;
        cscBike = nullptr;
    }
    if (proformWifiBike) {

        delete proformWifiBike;
        proformWifiBike = nullptr;
    }
    if (antBike) {

        delete antBike;
        antBike = nullptr;
    }
    if (proformTelnetBike) {

        delete proformTelnetBike;
        proformTelnetBike = nullptr;
    }    
    if (proformWifiTreadmill) {

        delete proformWifiTreadmill;
        proformWifiTreadmill = nullptr;
    }
    if (nordictrackifitadbTreadmill) {

        delete nordictrackifitadbTreadmill;
        nordictrackifitadbTreadmill = nullptr;
    }
    if (nordictrackifitadbBike) {

        delete nordictrackifitadbBike;
        nordictrackifitadbBike = nullptr;
    }
    if (nordictrackifitadbElliptical) {

        delete nordictrackifitadbElliptical;
        nordictrackifitadbElliptical = nullptr;
    }
    if (powerBike) {

        delete powerBike;
        powerBike = nullptr;
    }
    if (powerTreadmill) {

        delete powerTreadmill;
        powerTreadmill = nullptr;
    }
    if (fakeBike) {

        delete fakeBike;
        fakeBike = nullptr;
    }
    if (fakeElliptical) {

        delete fakeElliptical;
        fakeElliptical = nullptr;
    }
    if (fakeRower) {

        delete fakeRower;
        fakeRower = nullptr;
    }
    if (fakeTreadmill) {

        delete fakeTreadmill;
        fakeTreadmill = nullptr;
    }
    if (npeCableBike) {

        delete npeCableBike;
        npeCableBike = nullptr;
    }
    if (tacxneo2Bike) {

        delete tacxneo2Bike;
        tacxneo2Bike = nullptr;
    }
    if (stagesBike) {

        delete stagesBike;

        stagesBike = nullptr;
    }
    if (toorx) {

        delete toorx;
        toorx = nullptr;
    }
    if (iConceptBike) {

        delete iConceptBike;
        iConceptBike = nullptr;
    }
    if (iConceptElliptical) {

        delete iConceptElliptical;
        iConceptElliptical = nullptr;
    }
    if (trxappgateusb) {

        delete trxappgateusb;
        trxappgateusb = nullptr;
    }
    if (spiritTreadmill) {

        delete spiritTreadmill;
        spiritTreadmill = nullptr;
    }
    if (activioTreadmill) {

        delete activioTreadmill;
        activioTreadmill = nullptr;
    }
    if (trxappgateusbBike) {

        delete trxappgateusbBike;
        trxappgateusbBike = nullptr;
    }
    if (trxappgateusbElliptical) {

        delete trxappgateusbElliptical;
        trxappgateusbElliptical = nullptr;
    }
    if (soleBike) {

        delete soleBike;
        soleBike = nullptr;
    }
    if (keepBike) {

        delete keepBike;
        keepBike = nullptr;
    }
    if (ultraSportBike) {

        delete ultraSportBike;
        ultraSportBike = nullptr;
    }
    if (mepanelBike) {

        delete mepanelBike;
        mepanelBike = nullptr;
    }
    if (echelonConnectSport) {

        delete echelonConnectSport;
        echelonConnectSport = nullptr;
    }
    if (echelonRower) {

        delete echelonRower;
        echelonRower = nullptr;
    }
    if (echelonStride) {

        delete echelonStride;
        echelonStride = nullptr;
    }
    if (octaneTreadmill) {

        delete octaneTreadmill;
        octaneTreadmill = nullptr;
    }
    if (ziproTreadmill) {

        delete ziproTreadmill;
        ziproTreadmill = nullptr;
    }
    if (octaneElliptical) {

        delete octaneElliptical;
        octaneElliptical = nullptr;
    }
    if (ftmsRower) {

        delete ftmsRower;
        ftmsRower = nullptr;
    }
    if (concept2Skierg) {

        delete concept2Skierg;
        concept2Skierg = nullptr;
    }
    if (smartrowRower) {

        delete smartrowRower;
        smartrowRower = nullptr;
    }
    if (yesoulBike) {

        delete yesoulBike;
        yesoulBike = nullptr;
    }
    if (proformBike) {

        delete proformBike;
        proformBike = nullptr;
    }
    if (proformTreadmill) {

        delete proformTreadmill;
        proformTreadmill = nullptr;
    }
    if (proformElliptical) {

        delete proformElliptical;
        proformElliptical = nullptr;
    }
    if (nordictrackElliptical) {

        delete nordictrackElliptical;
        nordictrackElliptical = nullptr;
    }
    if (proformEllipticalTrainer) {

        delete proformEllipticalTrainer;
        proformEllipticalTrainer = nullptr;
    }
    if (proformRower) {

        delete proformRower;
        proformRower = nullptr;
    }
    if (eslinkerTreadmill) {

        delete eslinkerTreadmill;
        eslinkerTreadmill = nullptr;
    }
    if (deerrunTreadmill) {

        delete deerrunTreadmill;
        deerrunTreadmill = nullptr;
    }
    if (crossRope) {

        delete crossRope;
        crossRope = nullptr;
    }
    if (bowflexTreadmill) {

        delete bowflexTreadmill;
        bowflexTreadmill = nullptr;
    }
    if (bowflexT216Treadmill) {

        delete bowflexT216Treadmill;
        bowflexT216Treadmill = nullptr;
    }
    if (pafersTreadmill) {

        delete pafersTreadmill;
        pafersTreadmill = nullptr;
    }
    if (nautilusTreadmill) {

        delete nautilusTreadmill;
        nautilusTreadmill = nullptr;
    }
    if (flywheelBike) {

        delete flywheelBike;
        flywheelBike = nullptr;
    }
    if (mcfBike) {

        delete mcfBike;
        mcfBike = nullptr;
    }
    if (schwinnIC4Bike) {

        delete schwinnIC4Bike;
        schwinnIC4Bike = nullptr;
    }
    if (schwinn170Bike) {

        delete schwinn170Bike;
        schwinn170Bike = nullptr;
    }
    if (sportsTechBike) {

        delete sportsTechBike;
        sportsTechBike = nullptr;
    }
    if (sportsTechElliptical) {

        delete sportsTechElliptical;
        sportsTechElliptical = nullptr;
    }
    if (sportsPlusBike) {

        delete sportsPlusBike;
        sportsPlusBike = nullptr;
    }
    if (pelotonBike) {

        delete pelotonBike;
        pelotonBike = nullptr;
    }
    if (inspireBike) {

        delete inspireBike;
        inspireBike = nullptr;
    }
#ifndef Q_OS_IOS
    if (computrainerBike) {

        delete computrainerBike;
        computrainerBike = nullptr;
    }
    if (csafeRower) {

        delete csafeRower;
        csafeRower = nullptr;
    }
#endif
    if (chronoBike) {

        delete chronoBike;
        chronoBike = nullptr;
    }
    if (snodeBike) {

        delete snodeBike;
        snodeBike = nullptr;
    }
    if (ftmsBike) {

        delete ftmsBike;
        ftmsBike = nullptr;
    }
    if (wahooKickrSnapBike) {

        delete wahooKickrSnapBike;
        wahooKickrSnapBike = nullptr;
    }
    if (horizonGr7Bike) {

        delete horizonGr7Bike;
        horizonGr7Bike = nullptr;
    }
    if (renphoBike) {

        delete renphoBike;
        renphoBike = nullptr;
    }
    if (pafersBike) {

        delete pafersBike;
        pafersBike = nullptr;
    }
    if (fitPlusBike) {

        delete fitPlusBike;
        fitPlusBike = nullptr;
    }
    if (skandikaWiriBike) {

        delete skandikaWiriBike;
        skandikaWiriBike = nullptr;
    }
    if (heartRateBelt) {

        // heartRateBelt->disconnectBluetooth(); // to test
        delete heartRateBelt;
        heartRateBelt = nullptr;
    }
    if (ftmsAccessory) {

        // heartRateBelt->disconnectBluetooth(); // to test
        delete ftmsAccessory;
        ftmsAccessory = nullptr;
    }
    if (fitmetriaFanfit.length()) {

        foreach (fitmetria_fanfit *f, fitmetriaFanfit) {
            delete f;
            f = nullptr;
        }
        fitmetriaFanfit.clear();
    }
    if (wahookickrHeadWind.length()) {

        foreach (wahookickrheadwind *f, wahookickrHeadWind) {
            delete f;
            f = nullptr;
        }
        wahookickrHeadWind.clear();
    }
    if (eliteAriaFan.length()) {

        foreach (eliteariafan *f, eliteAriaFan) {
            delete f;
            f = nullptr;
        }
        eliteAriaFan.clear();
    }
    if (cadenceSensor) {

        // heartRateBelt->disconnectBluetooth(); // to test
        delete cadenceSensor;
        cadenceSensor = nullptr;
    }
    if (powerSensor) {

        // heartRateBelt->disconnectBluetooth(); // to test
        delete powerSensor;
        powerSensor = nullptr;
    }
    if (powerSensorRun) {

        // heartRateBelt->disconnectBluetooth(); // to test
        delete powerSensorRun;
        powerSensorRun = nullptr;
    }
    if (eliteRizer) {

        // heartRateBelt->disconnectBluetooth(); // to test
        delete eliteRizer;
        eliteRizer = nullptr;
    }
    if (eliteSterzoSmart) {

        // heartRateBelt->disconnectBluetooth(); // to test
        delete eliteSterzoSmart;
        eliteSterzoSmart = nullptr;
    }
    this->startDiscovery();
}

bluetoothdevice *bluetooth::device() {
    if (domyos) {

        return domyos;
    } else if (domyosBike) {
        return domyosBike;
    } else if (domyosRower) {
        return domyosRower;
    } else if (fitshowTreadmill) {
        return fitshowTreadmill;
    } else if (focusTreadmill) {
        return focusTreadmill;
    } else if (domyosElliptical) {
        return domyosElliptical;
    } else if (ypooElliptical) {
        return ypooElliptical;
    } else if (soleElliptical) {
        return soleElliptical;
    } else if (nautilusElliptical) {
        return nautilusElliptical;
    } else if (nautilusBike) {
        return nautilusBike;
    } else if (bhFitnessElliptical) {
        return bhFitnessElliptical;
    } else if (cscBike) {
        return cscBike;
    } else if (proformWifiBike) {
        return proformWifiBike;
    } else if (proformTelnetBike) {
        return proformTelnetBike;        
    } else if (proformWifiTreadmill) {
        return proformWifiTreadmill;
    } else if (antBike) {
        return antBike;
    } else if (nordictrackifitadbTreadmill) {
        return nordictrackifitadbTreadmill;
    } else if (nordictrackifitadbBike) {
        return nordictrackifitadbBike;
    } else if (nordictrackifitadbElliptical) {
        return nordictrackifitadbElliptical;
    } else if (powerBike) {
        return powerBike;
    } else if (powerTreadmill) {
        return powerTreadmill;
    } else if (fakeBike) {
        return fakeBike;
    } else if (fakeElliptical) {
        return fakeElliptical;
    } else if (fakeRower) {
        return fakeRower;
    } else if (fakeTreadmill) {
        return fakeTreadmill;
    } else if (npeCableBike) {
        return npeCableBike;
    } else if (tacxneo2Bike) {
        return tacxneo2Bike;
    } else if (stagesBike) {
        return stagesBike;
    } else if (toorx) {
        return toorx;
    } else if (iConceptBike) {
        return iConceptBike;
    } else if (iConceptElliptical) {
        return iConceptElliptical;
    } else if (spiritTreadmill) {
        return spiritTreadmill;
    } else if (activioTreadmill) {
        return activioTreadmill;
    } else if (trxappgateusb) {
        return trxappgateusb;
    } else if (trxappgateusbBike) {
        return trxappgateusbBike;
    } else if (trxappgateusbElliptical) {
        return trxappgateusbElliptical;
    } else if (soleBike) {
        return soleBike;
    } else if (keepBike) {
        return keepBike;
    } else if (apexBike) {
        return apexBike;
    } else if (bkoolBike) {
        return bkoolBike;
    } else if (ultraSportBike) {
        return ultraSportBike;
    } else if (horizonTreadmill) {
        return horizonTreadmill;
    } else if (lifefitnessTreadmill) {
        return lifefitnessTreadmill;
    } else if (technogymmyrunTreadmill) {
        return technogymmyrunTreadmill;
#ifndef Q_OS_IOS
    } else if (technogymmyrunrfcommTreadmill) {
        return technogymmyrunrfcommTreadmill;
#endif
    } else if (soleF80) {
        return soleF80;
    } else if (kingsmithR2Treadmill) {
        return kingsmithR2Treadmill;
    } else if (kingsmithR1ProTreadmill) {
        return kingsmithR1ProTreadmill;
    } else if (shuaA5Treadmill) {
        return shuaA5Treadmill;
    } else if (trueTreadmill) {
        return trueTreadmill;
    } else if (mepanelBike) {
        return mepanelBike;
    } else if (echelonConnectSport) {
        return echelonConnectSport;
    } else if (echelonRower) {
        return echelonRower;
    } else if (echelonStride) {
        return echelonStride;
    } else if (octaneTreadmill) {
        return octaneTreadmill;
    } else if (ziproTreadmill) {
        return ziproTreadmill;
    } else if (octaneElliptical) {
        return octaneElliptical;
    } else if (ftmsRower) {
        return ftmsRower;
    } else if (concept2Skierg) {
        return concept2Skierg;
    } else if (smartrowRower) {
        return smartrowRower;
    } else if (yesoulBike) {
        return yesoulBike;
    } else if (proformBike) {
        return proformBike;
    } else if (proformTreadmill) {
        return proformTreadmill;
    } else if (proformElliptical) {
        return proformElliptical;
    } else if (nordictrackElliptical) {
        return nordictrackElliptical;
    } else if (proformEllipticalTrainer) {
        return proformEllipticalTrainer;
    } else if (proformRower) {
        return proformRower;
    } else if (eslinkerTreadmill) {
        return eslinkerTreadmill;
    } else if (deerrunTreadmill) {
        return deerrunTreadmill;
    } else if (crossRope) {
        return crossRope;
    } else if (bowflexTreadmill) {
        return bowflexTreadmill;
    } else if (bowflexT216Treadmill) {
        return bowflexT216Treadmill;
    } else if (pafersTreadmill) {
        return pafersTreadmill;
    } else if (nautilusTreadmill) {
        return nautilusTreadmill;
    } else if (flywheelBike) {
        return flywheelBike;
    } else if (mcfBike) {
        return mcfBike;
    } else if (schwinnIC4Bike) {
        return schwinnIC4Bike;
    } else if (schwinn170Bike) {
        return schwinn170Bike;
    } else if (sportsTechBike) {
        return sportsTechBike;
    } else if (sportsTechElliptical) {
        return sportsTechElliptical;
    } else if (sportsPlusBike) {
        return sportsPlusBike;
    } else if (inspireBike) {
        return inspireBike;
    } else if (chronoBike) {
        return chronoBike;
    } else if (m3iBike) {
        return m3iBike;
    } else if (snodeBike) {
        return snodeBike;
    } else if (ftmsBike) {
        return ftmsBike;
    } else if (wahooKickrSnapBike) {
        return wahooKickrSnapBike;
    } else if (horizonGr7Bike) {
        return horizonGr7Bike;
    } else if (renphoBike) {
        return renphoBike;
    } else if (pafersBike) {
        return pafersBike;
    } else if (fitPlusBike) {
        return fitPlusBike;
    } else if (pelotonBike) {
        return pelotonBike;
    } else if (skandikaWiriBike) {
        return skandikaWiriBike;
#ifndef Q_OS_IOS
    } else if (computrainerBike) {
        return computrainerBike;
    } else if (csafeRower) {
        return csafeRower;
#endif
    }
    return nullptr;
}

bool bluetooth::handleSignal(int signal) {
    if (signal == SIGNALS::SIG_INT) {
        qDebug() << QStringLiteral("SIGINT");
        QFile::remove(QStringLiteral("status.xml"));
        exit(EXIT_SUCCESS);
    }
    // Let the signal propagate as though we had not been there
    return false;
}

void bluetooth::stateFileRead() {
    if (!device()) {
        return;
    }

    QFile *log;
    QDomDocument xmlBOM;
    log = new QFile(QStringLiteral("status.xml"));
    if (!log->open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << QStringLiteral("Open status.xml for writing failed");

        return;
    }
    xmlBOM.setContent(log);
    QDomElement root = xmlBOM.documentElement();

    // Get root names and attributes
    // QString Type = root.tagName(); //unused
    // QString lastUpdated = root.attribute(QStringLiteral("Updated"), QDateTime::currentDateTime().toString());

    QDomElement machine = root.firstChild().toElement();
    // Loop while there is a child
    while (!machine.isNull()) {

        // Check if the child tag name is COMPONENT
        if (machine.tagName() == QStringLiteral("Treadmill")) {

            // Read and display the component ID
            double speed = machine.attribute(QStringLiteral("Speed"), QStringLiteral("0.0")).toDouble();
            double inclination = machine.attribute(QStringLiteral("Incline"), QStringLiteral("0.0")).toDouble();

            qobject_cast<treadmill *>(device())->setLastSpeed(speed);
            qobject_cast<treadmill *>(device())->setLastInclination(inclination);
        }

        // Next component
        machine = machine.nextSibling().toElement();
    }

    log->close();
}

void bluetooth::stateFileUpdate() {
    if (!device()) {
        return;
    }
    if (device()->deviceType() != bluetoothdevice::TREADMILL) {
        return;
    }

    QFile *log;
    QDomDocument docStatus;
    QDomElement docRoot;
    QDomElement docTreadmill;
    QDomElement docHeart;
    log = new QFile(QStringLiteral("status.xml"));
    if (!log->open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << QStringLiteral("Open status.xml for writing failed");

        return;
    }
    docRoot = docStatus.createElement(QStringLiteral("Gym"));
    docStatus.appendChild(docRoot);
    docTreadmill = docStatus.createElement(QStringLiteral("Treadmill"));
    docTreadmill.setAttribute(QStringLiteral("Speed"), QString::number(device()->currentSpeed().value(), 'f', 1));
    docTreadmill.setAttribute(
        QStringLiteral("Incline"),
        QString::number(qobject_cast<treadmill *>(device())->currentInclination().value(), 'f', 1));
    docRoot.appendChild(docTreadmill);
    // docHeart = docStatus.createElement("Heart");
    // docHeart.setAttribute("Rate", QString::number(currentHeart));
    // docRoot.appendChild(docHeart);
    docRoot.setAttribute(QStringLiteral("Updated"), QDateTime::currentDateTime().toString());
    QTextStream stream(log);
    stream << docStatus.toString();
    log->flush();
    log->close();
}

void bluetooth::speedChanged(double speed) {

    Q_UNUSED(speed);
    stateFileUpdate();
}

void bluetooth::inclinationChanged(double grade, double inclination) {

    Q_UNUSED(grade);
    Q_UNUSED(inclination);
    stateFileUpdate();
}

bool bluetooth::fitmetria_fanfit_isconnected(QString name) {
    foreach (fitmetria_fanfit *f, fitmetriaFanfit) {
        if (!name.compare(f->bluetoothDevice.name()))
            return true;
    }
    foreach (wahookickrheadwind *f, wahookickrHeadWind) {
        if (!name.compare(f->bluetoothDevice.name()))
            return true;
    }
    foreach (eliteariafan *f, eliteAriaFan) {
        if (!name.compare(f->bluetoothDevice.name()))
            return true;
    }
    return false;
}

#if (QT_VERSION >= QT_VERSION_CHECK(5, 12, 0))
void bluetooth::deviceUpdated(const QBluetoothDeviceInfo &device, QBluetoothDeviceInfo::Fields updateFields) {

    debug("deviceUpdated " + device.name() + " " + updateFields);
}
#endif
