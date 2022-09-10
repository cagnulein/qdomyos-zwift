#include "bluetooth.h"
#include "homeform.h"
#include <QBluetoothLocalDevice>
#include <QDateTime>
#include <QFile>
#include <QMetaEnum>

#include <QtXml>
#ifdef Q_OS_ANDROID
#include "keepawakehelper.h"
#include <QAndroidJniObject>
#endif

bluetooth::bluetooth(bool logs, const QString &deviceName, bool noWriteResistance, bool noHeartService,
                     uint32_t pollDeviceTime, bool noConsole, bool testResistance, uint8_t bikeResistanceOffset,
                     double bikeResistanceGain) {
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
    QString path = homeform::getWritableAppDir() + QStringLiteral("QZTemplates");
    this->userTemplateManager = TemplateInfoSenderBuilder::getInstance(
        QStringLiteral("user"), QStringList({path, QStringLiteral(":/templates/")}), this);
    QString innerId = QStringLiteral("inner");
    QString sKey = QStringLiteral("template_") + innerId + QStringLiteral("_" TEMPLATE_PRIVATE_WEBSERVER_ID "_");
    settings.setValue(sKey + QStringLiteral("enabled"), true);
    settings.setValue(sKey + QStringLiteral("type"), TEMPLATE_TYPE_WEBSERVER);
    settings.setValue(sKey + QStringLiteral("port"), 0);
    this->innerTemplateManager =
        TemplateInfoSenderBuilder::getInstance(innerId, QStringList({QStringLiteral(":/inner_templates/")}), this);

#ifdef TEST
    schwinnIC4Bike = (schwinnic4bike *)new bike();
    userTemplateManager->start(schwinnIC4Bike);
    innerTemplateManager->start(schwinnIC4Bike);
    connectedAndDiscovered();
    return;
#endif
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

#ifdef Q_OS_IOS
        // Schwinn bikes on iOS allows to be connected to several instances, so in this way
        // QZ will remember the address and will try to connect to it
        QString b = settings.value("bluetooth_lastdevice_name", "").toString();
        qDebug() << "last device name (IC BIKE workaround)" << b;
        if (!b.compare(settings.value("filter_device", "Disabled").toString()) &&
            (b.toUpper().startsWith("IC BIKE") || b.toUpper().startsWith("C7-"))) {

            this->stopDiscovery();
            schwinnIC4Bike = new schwinnic4bike(noWriteResistance, noHeartService);
            // stateFileRead();
            QBluetoothDeviceInfo bt;
            bt.setDeviceUuid(QBluetoothUuid(settings.value("bluetooth_lastdevice_address", "").toString()));
            // set name method doesn't exist
            emit(deviceConnected(bt));
            connect(schwinnIC4Bike, SIGNAL(connectedAndDiscovered()), this, SLOT(connectedAndDiscovered()));
            // connect(echelonConnectSport, SIGNAL(disconnected()), this, SLOT(restart()));
            connect(schwinnIC4Bike, SIGNAL(debug(QString)), this, SLOT(debug(QString)));
            // connect(echelonConnectSport, SIGNAL(speedChanged(double)), this, SLOT(speedChanged(double)));
            // connect(echelonConnectSport, SIGNAL(inclinationChanged(double)), this, SLOT(inclinationChanged(double)));
            qDebug() << "UUID" << bt.deviceUuid();
            schwinnIC4Bike->deviceDiscovered(bt);
            userTemplateManager->start(schwinnIC4Bike);
            innerTemplateManager->start(schwinnIC4Bike);
            qDebug() << "connecting directly";
        }
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

void bluetooth::finished() {
    debug(QStringLiteral("BTLE scanning finished"));

#ifdef Q_OS_WIN
    if (device()) {
        qDebug() << QStringLiteral("bluetooth::finished but discoveryAgent is not active");
        return;
    }
#endif

    QSettings settings;
    QString heartRateBeltName =
        settings.value(QStringLiteral("heart_rate_belt_name"), QStringLiteral("Disabled")).toString();
    QString ftmsAccessoryName =
        settings.value(QStringLiteral("ftms_accessory_name"), QStringLiteral("Disabled")).toString();
    bool csc_as_bike = settings.value(QStringLiteral("cadence_sensor_as_bike"), false).toBool();
    bool power_as_bike = settings.value(QStringLiteral("power_sensor_as_bike"), false).toBool();
    bool power_as_treadmill = settings.value(QStringLiteral("power_sensor_as_treadmill"), false).toBool();
    QString cscName = settings.value(QStringLiteral("cadence_sensor_name"), QStringLiteral("Disabled")).toString();
    QString powerSensorName =
        settings.value(QStringLiteral("power_sensor_name"), QStringLiteral("Disabled")).toString();
    QString eliteRizerName = settings.value(QStringLiteral("elite_rizer_name"), QStringLiteral("Disabled")).toString();
    QString eliteSterzoSmartName =
        settings.value(QStringLiteral("elite_sterzo_smart_name"), QStringLiteral("Disabled")).toString();
    bool cscFound = cscName.startsWith(QStringLiteral("Disabled")) && !csc_as_bike;
    bool powerSensorFound =
        powerSensorName.startsWith(QStringLiteral("Disabled")) && !power_as_bike && !power_as_treadmill;
    bool eliteRizerFound = eliteRizerName.startsWith(QStringLiteral("Disabled"));
    bool eliteSterzoSmartFound = eliteSterzoSmartName.startsWith(QStringLiteral("Disabled"));
    bool heartRateBeltFound = heartRateBeltName.startsWith(QStringLiteral("Disabled"));
    bool ftmsAccessoryFound = ftmsAccessoryName.startsWith(QStringLiteral("Disabled"));

    // since i can have multiple fanfit i can't wait more because i don't have the full list of the fanfit
    // devices connected to QZ
    // bool fitmetriaFanfitEnabled = settings.value(QStringLiteral("fitmetria_fanfit_enable"), false).toBool();

    if ((!heartRateBeltFound && !heartRateBeltAvaiable()) || (!ftmsAccessoryFound && !ftmsAccessoryAvaiable()) ||
        (!cscFound && !cscSensorAvaiable()) || (!powerSensorFound && !powerSensorAvaiable()) ||
        (!eliteRizerFound && !eliteRizerAvaiable()) || (!eliteSterzoSmartFound && !eliteSterzoSmartAvaiable())) {

        // force heartRateBelt off
        forceHeartBeltOffForTimeout = true;
    }

    this->startDiscovery();
}

void bluetooth::startDiscovery() {

    #ifndef Q_OS_IOS
        QSettings settings;
        bool technogym_myrun_treadmill_experimental =
            settings.value(QStringLiteral("technogym_myrun_treadmill_experimental"), false).toBool();
        bool trx_route_key = settings.value(QStringLiteral("trx_route_key"), false).toBool();
        bool bh_spada_2 = settings.value(QStringLiteral("bh_spada_2"), false).toBool();

        if (!trx_route_key && !bh_spada_2 && !technogym_myrun_treadmill_experimental) {
    #endif
            discoveryAgent->start(QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);
    #ifndef Q_OS_IOS
        } else {
            discoveryAgent->start(QBluetoothDeviceDiscoveryAgent::ClassicMethod |
                                  QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);
        }
    #endif
}


void bluetooth::stopDiscovery()
{
    if(this->discoveryAgent)
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
    bool csc_as_bike = settings.value(QStringLiteral("cadence_sensor_as_bike"), false).toBool();
    QString cscName = settings.value(QStringLiteral("cadence_sensor_name"), QStringLiteral("Disabled")).toString();

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
        settings.value(QStringLiteral("ftms_accessory_name"), QStringLiteral("Disabled")).toString();

    Q_FOREACH (QBluetoothDeviceInfo b, devices) {
        if (!ftmsAccessoryName.compare(b.name())) {

            return true;
        }
    }
    return false;
}

bool bluetooth::powerSensorAvaiable() {

    QSettings settings;
    bool power_as_bike = settings.value(QStringLiteral("power_sensor_as_bike"), false).toBool();
    bool power_as_treadmill = settings.value(QStringLiteral("power_sensor_as_treadmill"), false).toBool();
    QString powerSensorName =
        settings.value(QStringLiteral("power_sensor_name"), QStringLiteral("Disabled")).toString();

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
    QString eliteRizerName = settings.value(QStringLiteral("elite_rizer_name"), QStringLiteral("Disabled")).toString();

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
        settings.value(QStringLiteral("elite_sterzo_smart_name"), QStringLiteral("Disabled")).toString();

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
        settings.value(QStringLiteral("heart_rate_belt_name"), QStringLiteral("Disabled")).toString();

    Q_FOREACH (QBluetoothDeviceInfo b, devices) {
        if (!heartRateBeltName.compare(b.name())) {

            return true;
        }
    }
    return false;
}

void bluetooth::deviceDiscovered(const QBluetoothDeviceInfo &device) {

    QSettings settings;
    QString heartRateBeltName =
        settings.value(QStringLiteral("heart_rate_belt_name"), QStringLiteral("Disabled")).toString();
    QString ftmsAccessoryName =
        settings.value(QStringLiteral("ftms_accessory_name"), QStringLiteral("Disabled")).toString();
    bool heartRateBeltFound = heartRateBeltName.startsWith(QStringLiteral("Disabled"));
    bool ftmsAccessoryFound = ftmsAccessoryName.startsWith(QStringLiteral("Disabled"));
    bool toorx_ftms = settings.value(QStringLiteral("toorx_ftms"), false).toBool();
    bool toorx_bike = (settings.value(QStringLiteral("toorx_bike"), false).toBool() ||
                       settings.value(QStringLiteral("jll_IC400_bike"), false).toBool() ||
                       settings.value(QStringLiteral("fytter_ri08_bike"), false).toBool() ||
                       settings.value(QStringLiteral("asviva_bike"), false).toBool() ||
                       settings.value(QStringLiteral("hertz_xr_770"), false).toBool()) &&
                      !toorx_ftms;
    bool snode_bike = settings.value(QStringLiteral("snode_bike"), false).toBool();
    bool fitplus_bike = settings.value(QStringLiteral("fitplus_bike"), false).toBool() ||
                        settings.value(QStringLiteral("virtufit_etappe"), false).toBool();
    bool csc_as_bike = settings.value(QStringLiteral("cadence_sensor_as_bike"), false).toBool();
    bool power_as_bike = settings.value(QStringLiteral("power_sensor_as_bike"), false).toBool();
    bool power_as_treadmill = settings.value(QStringLiteral("power_sensor_as_treadmill"), false).toBool();
    QString cscName = settings.value(QStringLiteral("cadence_sensor_name"), QStringLiteral("Disabled")).toString();
    bool cscFound = cscName.startsWith(QStringLiteral("Disabled")) || csc_as_bike;
    bool hammerRacerS = settings.value(QStringLiteral("hammer_racer_s"), false).toBool();
    bool flywheel_life_fitness_ic8 = settings.value(QStringLiteral("flywheel_life_fitness_ic8"), false).toBool();
    QString powerSensorName =
        settings.value(QStringLiteral("power_sensor_name"), QStringLiteral("Disabled")).toString();
    QString eliteRizerName = settings.value(QStringLiteral("elite_rizer_name"), QStringLiteral("Disabled")).toString();
    QString eliteSterzoSmartName =
        settings.value(QStringLiteral("elite_sterzo_smart_name"), QStringLiteral("Disabled")).toString();
    bool powerSensorFound =
        powerSensorName.startsWith(QStringLiteral("Disabled")) || power_as_bike || power_as_treadmill;
    bool eliteRizerFound = eliteRizerName.startsWith(QStringLiteral("Disabled"));
    bool eliteSterzoSmartFound = eliteSterzoSmartName.startsWith(QStringLiteral("Disabled"));
    bool fake_bike = settings.value(QStringLiteral("applewatch_fakedevice"), false).toBool();
    bool fakedevice_elliptical = settings.value(QStringLiteral("fakedevice_elliptical"), false).toBool();
    bool fakedevice_treadmill = settings.value(QStringLiteral("fakedevice_treadmill"), false).toBool();
    bool pafers_treadmill = settings.value(QStringLiteral("pafers_treadmill"), false).toBool();
    QString proformtdf4ip = settings.value(QStringLiteral("proformtdf4ip"), "").toString();
    QString proformtreadmillip = settings.value(QStringLiteral("proformtreadmillip"), "").toString();
    QString nordictrack_2950_ip = settings.value(QStringLiteral("nordictrack_2950_ip"), "").toString();
    QString tdf_10_ip = settings.value(QStringLiteral("tdf_10_ip"), "").toString();
    bool manufacturerDeviceFound = false;

    if (!heartRateBeltFound) {

        heartRateBeltFound = heartRateBeltAvaiable();
    }
    if (!ftmsAccessoryFound) {

        ftmsAccessoryFound = ftmsAccessoryAvaiable();
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

    QVector<quint16> ids = device.manufacturerIds();
    qDebug() << "manufacturerData";
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
    debug(QStringLiteral("Found new device: ") + device.name() + QStringLiteral(" (") + device.address().toString() +
          ')' + " " + device.majorDeviceClass() + QStringLiteral(":") + device.minorDeviceClass());
#if defined(Q_OS_DARWIN) || defined(Q_OS_IOS)
    qDebug() << device.deviceUuid();
#endif

    if (onlyDiscover)
        return;

#ifdef Q_OS_WIN
    if (this->device()) {
        qDebug() << QStringLiteral("bluetooth::finished but discoveryAgent is not active");
        return;
    }
#endif

    if ((heartRateBeltFound && ftmsAccessoryFound && cscFound && powerSensorFound && eliteRizerFound &&
         eliteSterzoSmartFound) ||
        forceHeartBeltOffForTimeout) {
        for (const QBluetoothDeviceInfo &b : qAsConst(devices)) {

            bool filter = true;
            if (!filterDevice.isEmpty() && !filterDevice.startsWith(QStringLiteral("Disabled"))) {

                filter = (b.name().compare(filterDevice, Qt::CaseInsensitive) == 0);
            }
            if (b.name().startsWith(QStringLiteral("M3")) && !m3iBike && filter) {

                if (m3ibike::isCorrectUnit(b)) {
                    this->stopDiscovery();
                    m3iBike = new m3ibike(noWriteResistance, noHeartService);
                    emit deviceConnected(b);
                    connect(m3iBike, &bluetoothdevice::connectedAndDiscovered, this,
                            &bluetooth::connectedAndDiscovered);
                    // connect(domyosBike, SIGNAL(disconnected()), this, SLOT(restart()));
                    connect(m3iBike, &m3ibike::debug, this, &bluetooth::debug);
                    m3iBike->deviceDiscovered(b);
                    connect(this, &bluetooth::searchingStop, m3iBike, &m3ibike::searchingStop);
                    if (!discoveryAgent->isActive())
                        emit searchingStop();
                    userTemplateManager->start(m3iBike);
                    innerTemplateManager->start(m3iBike);
                }
            } else if (fake_bike && !fakeBike) {
                this->stopDiscovery();
                fakeBike = new fakebike(noWriteResistance, noHeartService, false);
                emit deviceConnected(b);
                connect(fakeBike, &bluetoothdevice::connectedAndDiscovered, this, &bluetooth::connectedAndDiscovered);
                connect(fakeBike, &fakebike::inclinationChanged, this, &bluetooth::inclinationChanged);
                // connect(cscBike, SIGNAL(disconnected()), this, SLOT(restart()));
                // connect(this, SIGNAL(searchingStop()), fakeBike, SLOT(searchingStop())); //NOTE: Commented due to
                // #358
                if (!discoveryAgent->isActive()) {
                    emit searchingStop();
                }
                userTemplateManager->start(fakeBike);
                innerTemplateManager->start(fakeBike);
            } else if (fakedevice_elliptical && !fakeElliptical) {
                this->stopDiscovery();
                fakeElliptical = new fakeelliptical(noWriteResistance, noHeartService, false);
                emit deviceConnected(b);
                connect(fakeElliptical, &bluetoothdevice::connectedAndDiscovered, this,
                        &bluetooth::connectedAndDiscovered);
                connect(fakeElliptical, &fakeelliptical::inclinationChanged, this, &bluetooth::inclinationChanged);
                // connect(cscBike, SIGNAL(disconnected()), this, SLOT(restart()));
                // connect(this, SIGNAL(searchingStop()), fakeBike, SLOT(searchingStop())); //NOTE: Commented due to
                // #358
                if (!discoveryAgent->isActive()) {
                    emit searchingStop();
                }
                userTemplateManager->start(fakeElliptical);
                innerTemplateManager->start(fakeElliptical);
            } else if (fakedevice_treadmill && !fakeTreadmill) {
                this->stopDiscovery();
                fakeTreadmill = new faketreadmill(noWriteResistance, noHeartService, false);
                emit deviceConnected(b);
                connect(fakeTreadmill, &bluetoothdevice::connectedAndDiscovered, this,
                        &bluetooth::connectedAndDiscovered);
                connect(fakeTreadmill, &faketreadmill::inclinationChanged, this, &bluetooth::inclinationChanged);
                // connect(cscBike, SIGNAL(disconnected()), this, SLOT(restart()));
                // connect(this, SIGNAL(searchingStop()), fakeBike, SLOT(searchingStop())); //NOTE: Commented due to
                // #358
                if (!discoveryAgent->isActive()) {
                    emit searchingStop();
                }
                userTemplateManager->start(fakeTreadmill);
                innerTemplateManager->start(fakeTreadmill);

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
                if (!discoveryAgent->isActive()) {
                    emit searchingStop();
                }
                userTemplateManager->start(proformWifiBike);
                innerTemplateManager->start(proformWifiBike);
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
                if (!discoveryAgent->isActive()) {
                    emit searchingStop();
                }
                userTemplateManager->start(proformWifiTreadmill);
                innerTemplateManager->start(proformWifiTreadmill);
            } else if (!nordictrack_2950_ip.isEmpty() && !nordictrackifitadbTreadmill) {
                this->stopDiscovery();
                nordictrackifitadbTreadmill = new nordictrackifitadbtreadmill(noWriteResistance, noHeartService);
                emit deviceConnected(b);
                connect(nordictrackifitadbTreadmill, &bluetoothdevice::connectedAndDiscovered, this,
                        &bluetooth::connectedAndDiscovered);
                connect(nordictrackifitadbTreadmill, &nordictrackifitadbtreadmill::debug, this, &bluetooth::debug);
                // nordictrackifitadbTreadmill->deviceDiscovered(b);
                // connect(this, SIGNAL(searchingStop()), cscBike, SLOT(searchingStop())); //NOTE: Commented due to #358
                if (!discoveryAgent->isActive()) {
                    emit searchingStop();
                }
                userTemplateManager->start(nordictrackifitadbTreadmill);
                innerTemplateManager->start(nordictrackifitadbTreadmill);
            } else if (!tdf_10_ip.isEmpty() && !nordictrackifitadbBike) {
                this->stopDiscovery();
                nordictrackifitadbBike = new nordictrackifitadbbike(noWriteResistance, noHeartService);
                emit deviceConnected(b);
                connect(nordictrackifitadbBike, &bluetoothdevice::connectedAndDiscovered, this,
                        &bluetooth::connectedAndDiscovered);
                connect(nordictrackifitadbBike, &nordictrackifitadbbike::debug, this, &bluetooth::debug);
                // nordictrackifitadbTreadmill->deviceDiscovered(b);
                // connect(this, SIGNAL(searchingStop()), cscBike, SLOT(searchingStop())); //NOTE: Commented due to #358
                if (!discoveryAgent->isActive()) {
                    emit searchingStop();
                }
                userTemplateManager->start(nordictrackifitadbBike);
                innerTemplateManager->start(nordictrackifitadbBike);
            } else if (csc_as_bike && b.name().startsWith(cscName) && !cscBike && filter) {

                this->stopDiscovery();
                cscBike = new cscbike(noWriteResistance, noHeartService, false);
                emit deviceConnected(b);
                connect(cscBike, &bluetoothdevice::connectedAndDiscovered, this, &bluetooth::connectedAndDiscovered);
                // connect(cscBike, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(cscBike, &cscbike::debug, this, &bluetooth::debug);
                cscBike->deviceDiscovered(b);
                // connect(this, SIGNAL(searchingStop()), cscBike, SLOT(searchingStop())); //NOTE: Commented due to #358
                if (!discoveryAgent->isActive()) {
                    emit searchingStop();
                }
                userTemplateManager->start(cscBike);
                innerTemplateManager->start(cscBike);
            } else if (power_as_bike && b.name().startsWith(powerSensorName) && !powerBike && filter) {

                this->stopDiscovery();
                powerBike = new stagesbike(noWriteResistance, noHeartService, false);
                emit deviceConnected(b);
                connect(powerBike, &bluetoothdevice::connectedAndDiscovered, this, &bluetooth::connectedAndDiscovered);
                // connect(cscBike, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(powerBike, &stagesbike::debug, this, &bluetooth::debug);
                powerBike->deviceDiscovered(b);
                // connect(this, SIGNAL(searchingStop()), cscBike, SLOT(searchingStop())); //NOTE: Commented due to #358
                if (!discoveryAgent->isActive()) {
                    emit searchingStop();
                }
                userTemplateManager->start(powerBike);
                innerTemplateManager->start(powerBike);
            } else if (power_as_treadmill && b.name().startsWith(powerSensorName) && !powerTreadmill && filter) {

                this->stopDiscovery();
                powerTreadmill = new strydrunpowersensor(noWriteResistance, noHeartService, false);
                emit deviceConnected(b);
                connect(powerTreadmill, &bluetoothdevice::connectedAndDiscovered, this,
                        &bluetooth::connectedAndDiscovered);
                // connect(cscBike, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(powerTreadmill, &strydrunpowersensor::debug, this, &bluetooth::debug);
                powerTreadmill->deviceDiscovered(b);
                // connect(this, SIGNAL(searchingStop()), cscBike, SLOT(searchingStop())); //NOTE: Commented due to #358
                if (!discoveryAgent->isActive()) {
                    emit searchingStop();
                }
                userTemplateManager->start(powerTreadmill);
                innerTemplateManager->start(powerTreadmill);
            } else if (b.name().toUpper().startsWith(QStringLiteral("DOMYOS-ROW")) &&
                       !b.name().startsWith(QStringLiteral("DomyosBridge")) && !domyosRower && filter) {
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
                if (!discoveryAgent->isActive()) {
                    emit searchingStop();
                }
                userTemplateManager->start(domyosRower);
                innerTemplateManager->start(domyosRower);
            } else if (b.name().startsWith(QStringLiteral("Domyos-Bike")) &&
                       !b.name().startsWith(QStringLiteral("DomyosBridge")) && !domyosBike && filter) {
                this->stopDiscovery();
                domyosBike = new domyosbike(noWriteResistance, noHeartService, testResistance, bikeResistanceOffset,
                                            bikeResistanceGain);
                emit deviceConnected(b);
                connect(domyosBike, &bluetoothdevice::connectedAndDiscovered, this, &bluetooth::connectedAndDiscovered);
                // connect(domyosBike, SIGNAL(disconnected()), this, SLOT(restart()));
                // connect(domyosBike, SIGNAL(debug(QString)), this, SLOT(debug(QString)));//NOTE: Commented due to #358
                domyosBike->deviceDiscovered(b);
                connect(this, &bluetooth::searchingStop, domyosBike, &domyosbike::searchingStop);
                if (!discoveryAgent->isActive()) {
                    emit searchingStop();
                }
                userTemplateManager->start(domyosBike);
                innerTemplateManager->start(domyosBike);
            } else if (b.name().startsWith(QStringLiteral("Domyos-EL")) &&
                       !b.name().startsWith(QStringLiteral("DomyosBridge")) && !domyosElliptical && filter) {
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
                if (!discoveryAgent->isActive()) {
                    emit searchingStop();
                }
                userTemplateManager->start(domyosElliptical);
                innerTemplateManager->start(domyosElliptical);
            } else if ((b.name().toUpper().startsWith(QStringLiteral("NAUTILUS E"))) &&
                       !nautilusElliptical && // NAUTILUS E616
                       filter) {
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
                if (!discoveryAgent->isActive())
                    emit searchingStop();
                userTemplateManager->start(nautilusElliptical);
                innerTemplateManager->start(nautilusElliptical);
            } else if ((b.name().toUpper().startsWith(QStringLiteral("NAUTILUS B"))) && !nautilusBike &&
                       filter) { // NAUTILUS B628
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
                if (!discoveryAgent->isActive())
                    emit searchingStop();
                userTemplateManager->start(nautilusBike);
                innerTemplateManager->start(nautilusBike);
            } else if ((b.name().toUpper().startsWith(QStringLiteral("I_FS"))) && !proformElliptical && filter) {
                this->stopDiscovery();
                proformElliptical = new proformelliptical(noWriteResistance, noHeartService);
                emit deviceConnected(b);
                connect(proformElliptical, &bluetoothdevice::connectedAndDiscovered, this,
                        &bluetooth::connectedAndDiscovered);
                // connect(proformElliptical, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(proformElliptical, &proformelliptical::debug, this, &bluetooth::debug);
                proformElliptical->deviceDiscovered(b);
                // connect(this, &bluetooth::searchingStop, proformElliptical, &proformelliptical::searchingStop);
                if (!discoveryAgent->isActive())
                    emit searchingStop();
                userTemplateManager->start(proformElliptical);
                innerTemplateManager->start(proformElliptical);
            } else if ((b.name().toUpper().startsWith(QStringLiteral("I_EL"))) && !nordictrackElliptical && filter) {
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
                if (!discoveryAgent->isActive())
                    emit searchingStop();
                userTemplateManager->start(nordictrackElliptical);
                innerTemplateManager->start(nordictrackElliptical);

            } else if ((b.name().toUpper().startsWith(QStringLiteral("I_VE"))) && !proformEllipticalTrainer && filter) {
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
                if (!discoveryAgent->isActive())
                    emit searchingStop();
                userTemplateManager->start(proformEllipticalTrainer);
                innerTemplateManager->start(proformEllipticalTrainer);
            } else if ((b.name().toUpper().startsWith(QStringLiteral("I_RW"))) && !proformRower && filter) {
                this->stopDiscovery();
                proformRower = new proformrower(noWriteResistance, noHeartService);
                emit deviceConnected(b);
                connect(proformRower, &bluetoothdevice::connectedAndDiscovered, this,
                        &bluetooth::connectedAndDiscovered);
                // connect(proformRower, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(proformRower, &proformrower::debug, this, &bluetooth::debug);
                proformRower->deviceDiscovered(b);
                // connect(this, &bluetooth::searchingStop, proformElliptical, &proformelliptical::searchingStop);
                if (!discoveryAgent->isActive())
                    emit searchingStop();
                userTemplateManager->start(proformRower);
                innerTemplateManager->start(proformRower);
            } else if ((b.name().toUpper().startsWith(QStringLiteral("B01_"))) && !bhFitnessElliptical && filter) {
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
                if (!discoveryAgent->isActive())
                    emit searchingStop();
                userTemplateManager->start(bhFitnessElliptical);
                innerTemplateManager->start(bhFitnessElliptical);
            } else if ((b.name().toUpper().startsWith(QStringLiteral("E95S")) ||
                        b.name().toUpper().startsWith(QStringLiteral("E25")) ||
                        b.name().toUpper().startsWith(QStringLiteral("E35")) ||
                        b.name().toUpper().startsWith(QStringLiteral("E55")) ||
                        b.name().toUpper().startsWith(QStringLiteral("E95")) ||
                        b.name().toUpper().startsWith(QStringLiteral("E98")) ||
                        b.name().toUpper().startsWith(QStringLiteral("XG400")) ||
                        b.name().toUpper().startsWith(QStringLiteral("E98S"))) &&
                       !soleElliptical && filter) {
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
                if (!discoveryAgent->isActive())
                    emit searchingStop();
                userTemplateManager->start(soleElliptical);
                innerTemplateManager->start(soleElliptical);
            } else if (b.name().startsWith(QStringLiteral("Domyos")) &&
                       !b.name().startsWith(QStringLiteral("DomyosBr")) && !domyos && !domyosElliptical &&
                       !domyosBike && !domyosRower && filter) {
                settings.setValue(QStringLiteral("bluetooth_lastdevice_name"), b.name());
#ifndef Q_OS_IOS
                settings.setValue(QStringLiteral("bluetooth_lastdevice_address"), b.address().toString());
#else
                settings.setValue("bluetooth_lastdevice_address", b.deviceUuid().toString());
#endif

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
                if (!discoveryAgent->isActive())
                    emit searchingStop();
                userTemplateManager->start(domyos);
                innerTemplateManager->start(domyos);
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
                           b.name().toUpper().startsWith(QStringLiteral("KS-NGCH-X21C"))) &&
                       !kingsmithR2Treadmill && filter) {
                settings.setValue(QStringLiteral("bluetooth_lastdevice_name"), b.name());
#ifndef Q_OS_IOS
                settings.setValue(QStringLiteral("bluetooth_lastdevice_address"), b.address().toString());
#else
                settings.setValue("bluetooth_lastdevice_address", b.deviceUuid().toString());
#endif

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
                if (!discoveryAgent->isActive())
                    emit searchingStop();
                userTemplateManager->start(kingsmithR2Treadmill);
                innerTemplateManager->start(kingsmithR2Treadmill);
            } else if ((b.name().toUpper().startsWith(QStringLiteral("R1 PRO")) ||
                        b.name().toUpper().startsWith(QStringLiteral("KINGSMITH")) ||
                        !b.name().toUpper().compare(QStringLiteral("RE")) || // just "RE"
                        b.name().toUpper().startsWith(
                            QStringLiteral("KS-"))) && // Treadmill KingSmith WalkingPad R2 Pro KS-HCR1AA
                       !kingsmithR1ProTreadmill &&
                       !kingsmithR2Treadmill && filter) {
                settings.setValue(QStringLiteral("bluetooth_lastdevice_name"), b.name());
#ifndef Q_OS_IOS
                settings.setValue(QStringLiteral("bluetooth_lastdevice_address"), b.address().toString());
#else
                settings.setValue("bluetooth_lastdevice_address", b.deviceUuid().toString());
#endif

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
                if (!discoveryAgent->isActive())
                    emit searchingStop();
                userTemplateManager->start(kingsmithR1ProTreadmill);
                innerTemplateManager->start(kingsmithR1ProTreadmill);
            } else if ((b.name().toUpper().startsWith(QStringLiteral("ZW-"))) && !shuaA5Treadmill && filter) {
                settings.setValue(QStringLiteral("bluetooth_lastdevice_name"), b.name());
#ifndef Q_OS_IOS
                settings.setValue(QStringLiteral("bluetooth_lastdevice_address"), b.address().toString());
#else
                settings.setValue("bluetooth_lastdevice_address", b.deviceUuid().toString());
#endif

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
                if (!discoveryAgent->isActive())
                    emit searchingStop();
                userTemplateManager->start(shuaA5Treadmill);
                innerTemplateManager->start(shuaA5Treadmill);
            } else if ((b.name().toUpper().startsWith(QStringLiteral("TRUE")) ||
                        b.name().toUpper().startsWith(QStringLiteral("TREADMILL"))) &&
                       !trueTreadmill && filter) {
                settings.setValue(QStringLiteral("bluetooth_lastdevice_name"), b.name());
#ifndef Q_OS_IOS
                settings.setValue(QStringLiteral("bluetooth_lastdevice_address"), b.address().toString());
#else
                settings.setValue("bluetooth_lastdevice_address", b.deviceUuid().toString());
#endif

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
                if (!discoveryAgent->isActive())
                    emit searchingStop();
                userTemplateManager->start(trueTreadmill);
                innerTemplateManager->start(trueTreadmill);
            } else if ((b.name().toUpper().startsWith(QStringLiteral("F80")) ||
                        b.name().toUpper().startsWith(QStringLiteral("F65")) ||
                        b.name().toUpper().startsWith(QStringLiteral("TT8")) ||
                        b.name().toUpper().startsWith(QStringLiteral("F63")) ||
                        b.name().toUpper().startsWith(QStringLiteral("F85"))) &&
                       !soleF80 && filter) {
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
                if (!discoveryAgent->isActive()) {
                    emit searchingStop();
                }
                userTemplateManager->start(soleF80);
                innerTemplateManager->start(soleF80);
            } else if ((b.name().toUpper().startsWith(QStringLiteral("HORIZON")) ||
                        b.name().toUpper().startsWith(QStringLiteral("AFG SPORT")) ||
                        b.name().toUpper().startsWith(QStringLiteral("WLT2541")) ||
                        b.name().toUpper().startsWith(QStringLiteral("S77")) ||
                        b.name().toUpper().startsWith(QStringLiteral("T318_")) ||   // FTMS
                        b.name().toUpper().startsWith(QStringLiteral("T218_")) ||   // FTMS
                        b.name().toUpper().startsWith(QStringLiteral("TRX3500")) || // FTMS
                        b.name().toUpper().startsWith(QStringLiteral("JFTMPARAGON")) ||
                        b.name().toUpper().startsWith(QStringLiteral("JFTM")) ||    // FTMS
                        b.name().toUpper().startsWith(QStringLiteral("CT800")) ||   // FTMS
                        b.name().toUpper().startsWith(QStringLiteral("TRX4500")) || // FTMS
                        b.name().toUpper().startsWith(QStringLiteral("ESANGLINKER"))) &&
                       !horizonTreadmill && filter) {
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
                if (!discoveryAgent->isActive()) {
                    emit searchingStop();
                }
                userTemplateManager->start(horizonTreadmill);
                innerTemplateManager->start(horizonTreadmill);
            } else if ((b.name().toUpper().startsWith(QStringLiteral("MYRUN ")) ||
                        b.name().toUpper().startsWith(QStringLiteral("MERACH-U3")) // FTMS
                        ) &&
                       !technogymmyrunTreadmill && filter) {
                this->stopDiscovery();
                bool technogym_myrun_treadmill_experimental =
                    settings.value(QStringLiteral("technogym_myrun_treadmill_experimental"), false).toBool();
#ifndef Q_OS_IOS
                if (!technogym_myrun_treadmill_experimental)
#endif
                {
                    technogymmyrunTreadmill = new technogymmyruntreadmill(noWriteResistance, noHeartService);
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
                    if (!discoveryAgent->isActive()) {
                        emit searchingStop();
                    }
                    userTemplateManager->start(technogymmyrunTreadmill);
                    innerTemplateManager->start(technogymmyrunTreadmill);
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
                    if (!discoveryAgent->isActive()) {
                        emit searchingStop();
                    }
                    userTemplateManager->start(technogymmyrunrfcommTreadmill);
                    innerTemplateManager->start(technogymmyrunrfcommTreadmill);
                }
#endif
            } else if ((b.name().toUpper().startsWith("TACX NEO") ||
                        (b.name().toUpper().startsWith("TACX SMART BIKE"))) &&
                       !tacxneo2Bike && filter) {
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
                userTemplateManager->start(tacxneo2Bike);
                innerTemplateManager->start(tacxneo2Bike);
            } else if ((b.name().toUpper().startsWith(QStringLiteral(">CABLE")) ||
                        (b.name().toUpper().startsWith(QStringLiteral("MD")) && b.name().length() == 7) ||
                        // BIKE 1, BIKE 2, BIKE 3...
                        (b.name().toUpper().startsWith(QStringLiteral("BIKE")) && flywheel_life_fitness_ic8 == false &&
                         b.name().length() == 6)) &&
                       !npeCableBike && filter) {
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
                userTemplateManager->start(npeCableBike);
                innerTemplateManager->start(npeCableBike);
            } else if (((b.name().startsWith("FS-") && hammerRacerS) ||
                        (b.name().toUpper().startsWith("MKSM")) || // MKSM3600036
                        (b.name().toUpper().startsWith("WAHOO KICKR")) || (b.name().toUpper().startsWith("B94")) ||
                        (b.name().toUpper().startsWith("STAGES BIKE")) || (b.name().toUpper().startsWith("SUITO")) ||
                        (b.name().toUpper().startsWith("D2RIDE")) || (b.name().toUpper().startsWith("DIRETO XR")) ||
                        (b.name().toUpper().startsWith("SMB1")) || (b.name().toUpper().startsWith("INRIDE"))) &&
                       !ftmsBike && !snodeBike && !fitPlusBike && !stagesBike && filter) {
                this->stopDiscovery();
                ftmsBike = new ftmsbike(noWriteResistance, noHeartService, bikeResistanceOffset, bikeResistanceGain);
                emit deviceConnected(b);
                connect(ftmsBike, &bluetoothdevice::connectedAndDiscovered, this, &bluetooth::connectedAndDiscovered);
                // connect(trxappgateusb, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(ftmsBike, &ftmsbike::debug, this, &bluetooth::debug);
                ftmsBike->deviceDiscovered(b);
                userTemplateManager->start(ftmsBike);
                innerTemplateManager->start(ftmsBike);
            } else if ((b.name().toUpper().startsWith("KICKR SNAP") || b.name().toUpper().startsWith("KICKR BIKE") ||
                        b.name().toUpper().startsWith("KICKR ROLLR")) &&
                       !wahooKickrSnapBike && filter) {
                this->stopDiscovery();
                wahooKickrSnapBike =
                    new wahookickrsnapbike(noWriteResistance, noHeartService, bikeResistanceOffset, bikeResistanceGain);
                emit deviceConnected(b);
                connect(wahooKickrSnapBike, &bluetoothdevice::connectedAndDiscovered, this,
                        &bluetooth::connectedAndDiscovered);
                // connect(wahooKickrSnapBike, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(wahooKickrSnapBike, &wahookickrsnapbike::debug, this, &bluetooth::debug);
                wahooKickrSnapBike->deviceDiscovered(b);
                userTemplateManager->start(wahooKickrSnapBike);
                innerTemplateManager->start(wahooKickrSnapBike);
            } else if (((b.name().toUpper().startsWith("JFIC")) // HORIZON GR7
                        ) &&
                       !horizonGr7Bike && filter) {
                this->stopDiscovery();
                horizonGr7Bike =
                    new horizongr7bike(noWriteResistance, noHeartService, bikeResistanceOffset, bikeResistanceGain);
                emit deviceConnected(b);
                connect(horizonGr7Bike, &bluetoothdevice::connectedAndDiscovered, this,
                        &bluetooth::connectedAndDiscovered);
                // connect(trxappgateusb, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(horizonGr7Bike, &horizongr7bike::debug, this, &bluetooth::debug);
                horizonGr7Bike->deviceDiscovered(b);
                userTemplateManager->start(horizonGr7Bike);
                innerTemplateManager->start(horizonGr7Bike);
            } else if ((b.name().toUpper().startsWith(QStringLiteral("STAGES ")) ||
                        (b.name().toUpper().startsWith(QStringLiteral("ASSIOMA")) &&
                         powerSensorName.startsWith(QStringLiteral("Disabled")))) &&
                       !stagesBike && !ftmsBike && filter) {
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
                userTemplateManager->start(stagesBike);
                innerTemplateManager->start(stagesBike);
            } else if (b.name().startsWith(QStringLiteral("SMARTROW")) && !smartrowRower && filter) {
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
                userTemplateManager->start(smartrowRower);
                innerTemplateManager->start(smartrowRower);
            } else if ((b.name().toUpper().startsWith(QStringLiteral("PM5")) &&
                        b.name().toUpper().endsWith(QStringLiteral("SKI"))) &&
                       !concept2Skierg && filter) {
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
                userTemplateManager->start(concept2Skierg);
                innerTemplateManager->start(concept2Skierg);
            } else if ((b.name().toUpper().startsWith(QStringLiteral("CR 00")) ||
                        b.name().toUpper().startsWith(QStringLiteral("KAYAKPRO")) ||
                        b.name().toUpper().startsWith(QStringLiteral("WHIPR")) ||
                        b.name().toUpper().startsWith(QStringLiteral("I-ROWER")) ||
                        (b.name().toUpper().startsWith(QStringLiteral("PM5")) &&
                         b.name().toUpper().contains(QStringLiteral("ROW")))) &&
                       !ftmsRower && filter) {
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
                userTemplateManager->start(ftmsRower);
                innerTemplateManager->start(ftmsRower);
            } else if ((b.name().toUpper().startsWith(QLatin1String("ECH-STRIDE")) ||
                        b.name().toUpper().startsWith(QLatin1String("ECH-SD-SPT"))) &&
                       !echelonStride && filter) {
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
                userTemplateManager->start(echelonStride);
                innerTemplateManager->start(echelonStride);
            } else if ((b.name().toUpper().startsWith(QLatin1String("ZR7"))) && !octaneTreadmill && filter) {
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
                userTemplateManager->start(octaneTreadmill);
                innerTemplateManager->start(octaneTreadmill);
            } else if ((b.name().startsWith(QStringLiteral("ECH-ROW")) ||
                        b.name().startsWith(QStringLiteral("ROW-S"))) &&
                       !echelonRower && filter) {
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
                userTemplateManager->start(echelonRower);
                innerTemplateManager->start(echelonRower);
            } else if (b.name().startsWith(QStringLiteral("ECH")) && !echelonRower && !echelonStride &&
                       !echelonConnectSport && filter) {
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
                userTemplateManager->start(echelonConnectSport);
                innerTemplateManager->start(echelonConnectSport);
            } else if ((b.name().toUpper().startsWith(QStringLiteral("IC BIKE")) ||
                        (b.name().toUpper().startsWith(QStringLiteral("C7-")) && b.name().length() != 17) ||
                        b.name().toUpper().startsWith(QStringLiteral("C9/C10"))) &&
                       !schwinnIC4Bike && filter) {
                settings.setValue(QStringLiteral("bluetooth_lastdevice_name"), b.name());
#ifndef Q_OS_IOS
                settings.setValue(QStringLiteral("bluetooth_lastdevice_address"), b.address().toString());
#else
                settings.setValue("bluetooth_lastdevice_address", b.deviceUuid().toString());
#endif
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
                userTemplateManager->start(schwinnIC4Bike);
                innerTemplateManager->start(schwinnIC4Bike);
            } else if (b.name().toUpper().startsWith(QStringLiteral("EW-BK")) && !sportsTechBike && filter) {
                this->stopDiscovery();
                sportsTechBike = new sportstechbike(noWriteResistance, noHeartService);
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
                userTemplateManager->start(sportsTechBike);
                innerTemplateManager->start(sportsTechBike);
            } else if (b.name().toUpper().startsWith(QStringLiteral("CARDIOFIT")) && !sportsPlusBike && filter) {
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
                userTemplateManager->start(sportsPlusBike);
                innerTemplateManager->start(sportsPlusBike);
            } else if (b.name().startsWith(yesoulbike::bluetoothName) && !yesoulBike && filter) {
                this->stopDiscovery();
                yesoulBike = new yesoulbike(noWriteResistance, noHeartService);
                // stateFileRead();
                emit deviceConnected(b);
                connect(yesoulBike, &bluetoothdevice::connectedAndDiscovered, this, &bluetooth::connectedAndDiscovered);
                // connect(yesoulBike, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(yesoulBike, &yesoulbike::debug, this, &bluetooth::debug);
                // connect(echelonConnectSport, SIGNAL(speedChanged(double)), this, SLOT(speedChanged(double)));
                // connect(echelonConnectSport, SIGNAL(inclinationChanged(double)), this,
                // SLOT(inclinationChanged(double)));
                yesoulBike->deviceDiscovered(b);
                userTemplateManager->start(yesoulBike);
                innerTemplateManager->start(yesoulBike);
            } else if ((b.name().startsWith(QStringLiteral("I_EB")) || b.name().startsWith(QStringLiteral("I_SB"))) &&
                       !proformBike && filter) {
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
                userTemplateManager->start(proformBike);
                innerTemplateManager->start(proformBike);
            } else if ((b.name().startsWith(QStringLiteral("I_TL"))) && !proformTreadmill && filter) {
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
                userTemplateManager->start(proformTreadmill);
                innerTemplateManager->start(proformTreadmill);
            } else if (b.name().toUpper().startsWith(QStringLiteral("ESLINKER")) && !eslinkerTreadmill && filter) {
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
                userTemplateManager->start(eslinkerTreadmill);
                innerTemplateManager->start(eslinkerTreadmill);
            } else if (b.name().toUpper().startsWith(QStringLiteral("PAFERS_")) && !pafersTreadmill &&
                       pafers_treadmill && filter) {
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
                userTemplateManager->start(pafersTreadmill);
                innerTemplateManager->start(pafersTreadmill);
            } else if (b.name().toUpper().startsWith(QStringLiteral("BOWFLEX T216")) && !bowflexT216Treadmill &&
                       filter) {
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
                userTemplateManager->start(bowflexT216Treadmill);
                innerTemplateManager->start(bowflexT216Treadmill);
            } else if (b.name().toUpper().startsWith(QStringLiteral("NAUTILUS T")) && !nautilusTreadmill && filter) {
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
                userTemplateManager->start(nautilusTreadmill);
                innerTemplateManager->start(nautilusTreadmill);
            } else if ((b.name().startsWith(QStringLiteral("Flywheel")) ||
                        // BIKE 1, BIKE 2, BIKE 3...
                        (b.name().toUpper().startsWith(QStringLiteral("BIKE")) && flywheel_life_fitness_ic8 == true &&
                         b.name().length() == 6)) &&
                       !flywheelBike && filter) {
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
                userTemplateManager->start(flywheelBike);
                innerTemplateManager->start(flywheelBike);
            } else if ((b.name().toUpper().startsWith(QStringLiteral("MCF-"))) && !mcfBike && filter) {
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
                userTemplateManager->start(mcfBike);
                innerTemplateManager->start(mcfBike);
            } else if ((b.name().startsWith(QStringLiteral("TRX ROUTE KEY"))) && !toorx && filter) {
                this->stopDiscovery();
                toorx = new toorxtreadmill();
                emit deviceConnected(b);
                connect(toorx, &bluetoothdevice::connectedAndDiscovered, this, &bluetooth::connectedAndDiscovered);
                // connect(toorx, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(toorx, &toorxtreadmill::debug, this, &bluetooth::debug);
                toorx->deviceDiscovered(b);
                userTemplateManager->start(toorx);
                innerTemplateManager->start(toorx);
            } else if ((b.name().toUpper().startsWith(QStringLiteral("BH DUALKIT"))) && !iConceptBike && filter) {
                this->stopDiscovery();
                iConceptBike = new iconceptbike();
                emit deviceConnected(b);
                connect(iConceptBike, &bluetoothdevice::connectedAndDiscovered, this,
                        &bluetooth::connectedAndDiscovered);
                // connect(toorx, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(iConceptBike, &iconceptbike::debug, this, &bluetooth::debug);
                iConceptBike->deviceDiscovered(b);
                userTemplateManager->start(iConceptBike);
                innerTemplateManager->start(iConceptBike);
            } else if ((b.name().toUpper().startsWith(QStringLiteral("XT385")) ||
                        b.name().toUpper().startsWith(QStringLiteral("XT485")) ||
                        b.name().toUpper().startsWith(QStringLiteral("XT900"))) &&
                       !spiritTreadmill && filter) {
                this->stopDiscovery();
                spiritTreadmill = new spirittreadmill();
                emit deviceConnected(b);
                connect(spiritTreadmill, &bluetoothdevice::connectedAndDiscovered, this,
                        &bluetooth::connectedAndDiscovered);
                // connect(spiritTreadmill, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(spiritTreadmill, &spirittreadmill::debug, this, &bluetooth::debug);
                spiritTreadmill->deviceDiscovered(b);
                userTemplateManager->start(spiritTreadmill);
                innerTemplateManager->start(spiritTreadmill);
            } else if (b.name().toUpper().startsWith(QStringLiteral("RUNNERT")) && !activioTreadmill && filter) {
                this->stopDiscovery();
                activioTreadmill = new activiotreadmill();
                emit deviceConnected(b);
                connect(activioTreadmill, &bluetoothdevice::connectedAndDiscovered, this,
                        &bluetooth::connectedAndDiscovered);
                // connect(activioTreadmill, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(activioTreadmill, &activiotreadmill::debug, this, &bluetooth::debug);
                activioTreadmill->deviceDiscovered(b);
                userTemplateManager->start(activioTreadmill);
                innerTemplateManager->start(activioTreadmill);
            } else if (((b.name().startsWith(QStringLiteral("TOORX"))) ||
                        (b.name().startsWith(QStringLiteral("V-RUN"))) ||
                        (b.name().toUpper().startsWith(QStringLiteral("I-CONSOLE+"))) ||
                        (b.name().toUpper().startsWith(QStringLiteral("ICONSOLE+"))) ||
                        (b.name().toUpper().startsWith(QStringLiteral("I-RUNNING"))) ||
                        (b.name().toUpper().startsWith(QStringLiteral("DKN RUN"))) ||
                        (b.name().toUpper().startsWith(QStringLiteral("REEBOK")))) &&
                       !trxappgateusb && !trxappgateusbBike && !toorx_bike && filter) {
                this->stopDiscovery();
                trxappgateusb = new trxappgateusbtreadmill();
                emit deviceConnected(b);
                connect(trxappgateusb, &bluetoothdevice::connectedAndDiscovered, this,
                        &bluetooth::connectedAndDiscovered);
                // connect(trxappgateusb, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(trxappgateusb, &trxappgateusbtreadmill::debug, this, &bluetooth::debug);
                trxappgateusb->deviceDiscovered(b);
                userTemplateManager->start(trxappgateusb);
                innerTemplateManager->start(trxappgateusb);
            } else if ((b.name().toUpper().startsWith(QStringLiteral("TUN ")) ||
                        ((b.name().startsWith(QStringLiteral("TOORX")) ||
                          b.name().toUpper().startsWith(QStringLiteral("I-CONSOIE+")) ||
                          b.name().toUpper().startsWith(QStringLiteral("I-CONSOLE+")) ||
                          b.name().toUpper().startsWith(QStringLiteral("IBIKING+")) ||
                          b.name().toUpper().startsWith(QStringLiteral("ICONSOLE+")) ||
                          b.name().toUpper().startsWith(QStringLiteral("VIFHTR2.1")) ||
                          b.name().toUpper().contains(QStringLiteral("CR011R")) ||
                          b.name().toUpper().startsWith(QStringLiteral("DKN MOTION"))) &&
                         (toorx_bike))) &&
                       !trxappgateusb && !trxappgateusbBike && filter) {
                this->stopDiscovery();
                trxappgateusbBike =
                    new trxappgateusbbike(noWriteResistance, noHeartService, bikeResistanceOffset, bikeResistanceGain);
                emit deviceConnected(b);
                connect(trxappgateusbBike, &bluetoothdevice::connectedAndDiscovered, this,
                        &bluetooth::connectedAndDiscovered);
                // connect(trxappgateusb, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(trxappgateusbBike, &trxappgateusbbike::debug, this, &bluetooth::debug);
                trxappgateusbBike->deviceDiscovered(b);
                userTemplateManager->start(trxappgateusbBike);
                innerTemplateManager->start(trxappgateusbBike);
            } else if ((b.name().toUpper().startsWith(QStringLiteral("X-BIKE"))) && !ultraSportBike && filter) {
                this->stopDiscovery();
                ultraSportBike =
                    new ultrasportbike(noWriteResistance, noHeartService, bikeResistanceOffset, bikeResistanceGain);
                emit deviceConnected(b);
                connect(ultraSportBike, &bluetoothdevice::connectedAndDiscovered, this,
                        &bluetooth::connectedAndDiscovered);
                // connect(ultraSportBike, SIGNAL(disconnected()), this, SLOT(restart()));
                // connect(ultraSportBike, &solebike::debug, this, &bluetooth::debug);
                ultraSportBike->deviceDiscovered(b);
                userTemplateManager->start(ultraSportBike);
                innerTemplateManager->start(ultraSportBike);
            } else if ((b.name().toUpper().startsWith(QStringLiteral("KEEP_BIKE_"))) && !keepBike && filter) {
                this->stopDiscovery();
                keepBike = new keepbike(noWriteResistance, noHeartService, bikeResistanceOffset, bikeResistanceGain);
                emit deviceConnected(b);
                connect(keepBike, &bluetoothdevice::connectedAndDiscovered, this, &bluetooth::connectedAndDiscovered);
                // connect(keepBike, SIGNAL(disconnected()), this, SLOT(restart()));
                // connect(keepBike, &solebike::debug, this, &bluetooth::debug);
                keepBike->deviceDiscovered(b);
                userTemplateManager->start(keepBike);
                innerTemplateManager->start(keepBike);
            } else if ((b.name().toUpper().startsWith(QStringLiteral("LCB")) ||
                        b.name().toUpper().startsWith(QStringLiteral("R92"))) &&
                       !soleBike && filter) {
                this->stopDiscovery();
                soleBike = new solebike(noWriteResistance, noHeartService, bikeResistanceOffset, bikeResistanceGain);
                emit deviceConnected(b);
                connect(soleBike, &bluetoothdevice::connectedAndDiscovered, this, &bluetooth::connectedAndDiscovered);
                // connect(soleBike, SIGNAL(disconnected()), this, SLOT(restart()));
                // connect(soleBike, &solebike::debug, this, &bluetooth::debug);
                soleBike->deviceDiscovered(b);
                userTemplateManager->start(soleBike);
                innerTemplateManager->start(soleBike);
            } else if (b.name().toUpper().startsWith(QStringLiteral("BFCP")) && !skandikaWiriBike && filter) {
                this->stopDiscovery();
                skandikaWiriBike =
                    new skandikawiribike(noWriteResistance, noHeartService, bikeResistanceOffset, bikeResistanceGain);
                emit deviceConnected(b);
                connect(skandikaWiriBike, &bluetoothdevice::connectedAndDiscovered, this,
                        &bluetooth::connectedAndDiscovered);
                // connect(skandikaWiriBike, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(skandikaWiriBike, &skandikawiribike::debug, this, &bluetooth::debug);
                skandikaWiriBike->deviceDiscovered(b);
                userTemplateManager->start(skandikaWiriBike);
                innerTemplateManager->start(skandikaWiriBike);
            } else if (((b.name().toUpper().startsWith("RQ") && b.name().length() == 5) ||
                        (b.name().toUpper().startsWith("SCH130")) || // not a renpho bike an FTMS one
                        ((b.name().startsWith(QStringLiteral("TOORX"))) && toorx_ftms)) &&
                       !renphoBike && !snodeBike && !fitPlusBike && filter) {
                this->stopDiscovery();
                renphoBike = new renphobike(noWriteResistance, noHeartService);
                emit(deviceConnected(b));
                connect(renphoBike, SIGNAL(connectedAndDiscovered()), this, SLOT(connectedAndDiscovered()));
                // connect(trxappgateusb, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(renphoBike, SIGNAL(debug(QString)), this, SLOT(debug(QString)));
                renphoBike->deviceDiscovered(b);
                userTemplateManager->start(renphoBike);
                innerTemplateManager->start(renphoBike);
            } else if ((b.name().toUpper().startsWith("PAFERS_")) && !pafersBike && !pafers_treadmill && filter) {
                this->stopDiscovery();
                pafersBike =
                    new pafersbike(noWriteResistance, noHeartService, bikeResistanceOffset, bikeResistanceGain);
                emit(deviceConnected(b));
                connect(pafersBike, SIGNAL(connectedAndDiscovered()), this, SLOT(connectedAndDiscovered()));
                // connect(pafersBike, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(pafersBike, SIGNAL(debug(QString)), this, SLOT(debug(QString)));
                pafersBike->deviceDiscovered(b);
                userTemplateManager->start(pafersBike);
                innerTemplateManager->start(pafersBike);
            } else if (((b.name().startsWith(QStringLiteral("FS-")) && snode_bike) ||
                        b.name().startsWith(QStringLiteral("TF-"))) && // TF-769DF2
                       !snodeBike &&
                       !ftmsBike && !fitPlusBike && filter) {
                this->stopDiscovery();
                snodeBike = new snodebike(noWriteResistance, noHeartService);
                emit deviceConnected(b);
                connect(snodeBike, &bluetoothdevice::connectedAndDiscovered, this, &bluetooth::connectedAndDiscovered);
                // connect(trxappgateusb, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(snodeBike, &snodebike::debug, this, &bluetooth::debug);
                snodeBike->deviceDiscovered(b);
                userTemplateManager->start(snodeBike);
                innerTemplateManager->start(snodeBike);
            } else if ((b.name().startsWith(QStringLiteral("FS-")) && fitplus_bike) && !fitPlusBike && !ftmsBike &&
                       !snodeBike && filter) {
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
                userTemplateManager->start(fitPlusBike);
                innerTemplateManager->start(fitPlusBike);
            } else if (((b.name().startsWith(QStringLiteral("FS-")) && !snode_bike && !fitplus_bike && !ftmsBike) ||
                        (b.name().startsWith(QStringLiteral("SW")) && b.name().length() == 14) ||
                        (b.name().startsWith(QStringLiteral("BF70")))) &&
                       !fitshowTreadmill && filter) {
                this->stopDiscovery();
                fitshowTreadmill = new fitshowtreadmill(this->pollDeviceTime, noConsole, noHeartService);
                emit deviceConnected(b);
                connect(fitshowTreadmill, &bluetoothdevice::connectedAndDiscovered, this,
                        &bluetooth::connectedAndDiscovered);
                connect(fitshowTreadmill, &fitshowtreadmill::debug, this, &bluetooth::debug);
                fitshowTreadmill->deviceDiscovered(b);
                connect(this, &bluetooth::searchingStop, fitshowTreadmill, &fitshowtreadmill::searchingStop);
                if (!discoveryAgent->isActive())
                    emit searchingStop();
                userTemplateManager->start(fitshowTreadmill);
                innerTemplateManager->start(fitshowTreadmill);
            } else if (b.name().toUpper().startsWith(QStringLiteral("IC")) && b.name().length() == 8 && !inspireBike &&
                       filter) {
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
                if (!discoveryAgent->isActive()) {
                    emit searchingStop();
                }
                userTemplateManager->start(inspireBike);
                innerTemplateManager->start(inspireBike);
            } else if (b.name().toUpper().startsWith(QStringLiteral("CHRONO ")) && !chronoBike && filter) {
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
                if (!discoveryAgent->isActive()) {
                    emit searchingStop();
                }
                userTemplateManager->start(chronoBike);
                innerTemplateManager->start(chronoBike);
            }
        }
    }
}

void bluetooth::connectedAndDiscovered() {

    static bool firstConnected = true;
    QSettings settings;
    QString heartRateBeltName =
        settings.value(QStringLiteral("heart_rate_belt_name"), QStringLiteral("Disabled")).toString();
    QString ftmsAccessoryName =
        settings.value(QStringLiteral("ftms_accessory_name"), QStringLiteral("Disabled")).toString();
    bool csc_as_bike = settings.value(QStringLiteral("cadence_sensor_as_bike"), false).toBool();
    QString cscName = settings.value(QStringLiteral("cadence_sensor_name"), QStringLiteral("Disabled")).toString();
    bool power_as_bike = settings.value(QStringLiteral("power_sensor_as_bike"), false).toBool();
    bool power_as_treadmill = settings.value(QStringLiteral("power_sensor_as_treadmill"), false).toBool();
    QString powerSensorName =
        settings.value(QStringLiteral("power_sensor_name"), QStringLiteral("Disabled")).toString();
    QString eliteRizerName = settings.value(QStringLiteral("elite_rizer_name"), QStringLiteral("Disabled")).toString();
    QString eliteSterzoSmartName =
        settings.value(QStringLiteral("elite_sterzo_smart_name"), QStringLiteral("Disabled")).toString();
    bool fitmetriaFanfitEnabled = settings.value(QStringLiteral("fitmetria_fanfit_enable"), false).toBool();

    // only at the first very connection, setting the user default resistance
    if (device() && firstConnected && device()->deviceType() == bluetoothdevice::BIKE &&
        settings.value(QStringLiteral("bike_resistance_start"), 1).toUInt() != 1) {
        qobject_cast<bike *>(device())->changeResistance(
            settings.value(QStringLiteral("bike_resistance_start"), 1).toUInt());
    } else if (device() && firstConnected && device()->deviceType() == bluetoothdevice::ELLIPTICAL &&
               settings.value(QStringLiteral("bike_resistance_start"), 1).toUInt() != 1) {
        qobject_cast<elliptical *>(device())->changeResistance(
            settings.value(QStringLiteral("bike_resistance_start"), 1).toUInt());
    }

    if (heartRateBeltName.startsWith(QStringLiteral("Disabled"))) {
        if (!settings.value(QStringLiteral("hrm_lastdevice_name"), "").toString().isEmpty()) {
            settings.setValue(QStringLiteral("hrm_lastdevice_name"), "");
        }
        if (!settings.value(QStringLiteral("hrm_lastdevice_address"), "").toString().isEmpty()) {
            settings.setValue(QStringLiteral("hrm_lastdevice_address"), "");
        }
    }

    if (this->device() != nullptr) {

#ifdef Q_OS_IOS
        if (settings.value(QStringLiteral("ios_cache_heart_device"), true).toBool()) {
            QString heartRateBeltName = settings.value("heart_rate_belt_name", "Disabled").toString();
            QString b = settings.value("hrm_lastdevice_name", "").toString();
            qDebug() << "last hrm name" << b;
            if (!b.compare(heartRateBeltName) && b.length()) {

                heartRateBelt = new heartratebelt();
                // connect(heartRateBelt, SIGNAL(disconnected()), this, SLOT(restart()));

                connect(heartRateBelt, SIGNAL(debug(QString)), this, SLOT(debug(QString)));
                connect(heartRateBelt, SIGNAL(heartRate(uint8_t)), this->device(), SLOT(heartRate(uint8_t)));
                QBluetoothDeviceInfo bt;
                bt.setDeviceUuid(QBluetoothUuid(settings.value("hrm_lastdevice_address", "").toString()));
                qDebug() << "UUID" << bt.deviceUuid();
                heartRateBelt->deviceDiscovered(bt);
            }
        }
#endif
        for (const QBluetoothDeviceInfo &b : qAsConst(devices)) {
            if (((b.name().startsWith(heartRateBeltName))) && !heartRateBelt &&
                !heartRateBeltName.startsWith(QStringLiteral("Disabled"))) {
                settings.setValue(QStringLiteral("hrm_lastdevice_name"), b.name());

#ifndef Q_OS_IOS
                settings.setValue(QStringLiteral("hrm_lastdevice_address"), b.address().toString());
#else
                settings.setValue("hrm_lastdevice_address", b.deviceUuid().toString());
#endif
                heartRateBelt = new heartratebelt();
                // connect(heartRateBelt, SIGNAL(disconnected()), this, SLOT(restart()));

                connect(heartRateBelt, &heartratebelt::debug, this, &bluetooth::debug);
                connect(heartRateBelt, &heartratebelt::heartRate, this->device(), &bluetoothdevice::heartRate);
                heartRateBelt->deviceDiscovered(b);

                break;
            }
        }

        for (const QBluetoothDeviceInfo &b : qAsConst(devices)) {
            if (((b.name().startsWith(ftmsAccessoryName))) && !ftmsAccessory &&
                !ftmsAccessoryName.startsWith(QStringLiteral("Disabled"))) {
                settings.setValue(QStringLiteral("ftms_accessory_lastdevice_name"), b.name());

#ifndef Q_OS_IOS
                settings.setValue(QStringLiteral("ftms_accessory_address"), b.address().toString());
#else
                settings.setValue("ftms_accessory_address", b.deviceUuid().toString());
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
                }
            }
        }

        if (!csc_as_bike) {
            for (const QBluetoothDeviceInfo &b : qAsConst(devices)) {
                if (((b.name().startsWith(cscName))) && !cadenceSensor &&
                    !cscName.startsWith(QStringLiteral("Disabled"))) {
                    settings.setValue(QStringLiteral("csc_sensor_lastdevice_name"), b.name());

#ifndef Q_OS_IOS
                    settings.setValue(QStringLiteral("csc_sensor_address"), b.address().toString());
#else
                    settings.setValue("csc_sensor_address", b.deviceUuid().toString());
#endif
                    cadenceSensor = new cscbike(false, false, true);
                    // connect(heartRateBelt, SIGNAL(disconnected()), this, SLOT(restart()));

                    connect(cadenceSensor, &cscbike::debug, this, &bluetooth::debug);
                    connect(cadenceSensor, &bluetoothdevice::cadenceChanged, this->device(),
                            &bluetoothdevice::cadenceSensor);
                    cadenceSensor->deviceDiscovered(b);
                    break;
                }
            }
        }
    }

    if (!power_as_bike && !power_as_treadmill) {
        for (const QBluetoothDeviceInfo &b : qAsConst(devices)) {
            if (((b.name().startsWith(powerSensorName))) && !powerSensor && !powerSensorRun &&
                !powerSensorName.startsWith(QStringLiteral("Disabled"))) {
                settings.setValue(QStringLiteral("power_sensor_lastdevice_name"), b.name());

#ifndef Q_OS_IOS
                settings.setValue(QStringLiteral("power_sensor_address"), b.address().toString());
#else
                settings.setValue("power_sensor_address", b.deviceUuid().toString());
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
                break;
            }
        }
    }

    for (const QBluetoothDeviceInfo &b : qAsConst(devices)) {
        if (((b.name().startsWith(eliteRizerName))) && !eliteRizer &&
            !eliteRizerName.startsWith(QStringLiteral("Disabled"))) {
            settings.setValue(QStringLiteral("elite_rizer_lastdevice_name"), b.name());

#ifndef Q_OS_IOS
            settings.setValue(QStringLiteral("elite_rizer_address"), b.address().toString());
#else
            settings.setValue("elite_rizer_address", b.deviceUuid().toString());
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
            settings.setValue(QStringLiteral("elite_sterzo_smart_lastdevice_name"), b.name());

#ifndef Q_OS_IOS
            settings.setValue(QStringLiteral("elite_sterzo_smart_address"), b.address().toString());
#else
            settings.setValue("elite_sterzo_smart_address", b.deviceUuid().toString());
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

#ifdef Q_OS_ANDROID
    if (settings.value(QStringLiteral("ant_cadence"), false).toBool() ||
        settings.value(QStringLiteral("ant_heart"), false).toBool()) {
        QAndroidJniObject activity = QAndroidJniObject::callStaticObjectMethod("org/qtproject/qt5/android/QtNative",
                                                                               "activity", "()Landroid/app/Activity;");
        KeepAwakeHelper::antObject(true)->callMethod<void>(
            "antStart", "(Landroid/app/Activity;ZZZ)V", activity.object<jobject>(),
            settings.value(QStringLiteral("ant_cadence"), false).toBool(),
            settings.value(QStringLiteral("ant_heart"), false).toBool(),
            settings.value(QStringLiteral("ant_garmin"), false).toBool());
    }
#endif

#ifdef Q_OS_IOS
    // in order to allow to populate the tiles with the IC BIKE auto connect feature
    if (firstConnected) {
        QBluetoothDeviceInfo bt;
        QString b = settings.value("bluetooth_lastdevice_name", "").toString();
        bt.setDeviceUuid(QBluetoothUuid(settings.value("bluetooth_lastdevice_address", "").toString()));
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

    if (settings.value(QStringLiteral("bluetooth_no_reconnection"), false).toBool()) {
        exit(EXIT_SUCCESS);
    }

    devices.clear();
    userTemplateManager->stop();
    innerTemplateManager->stop();

    if (device() && device()->VirtualDevice()) {
        if (device()->deviceType() == bluetoothdevice::TREADMILL) {

            delete static_cast<virtualtreadmill *>(device()->VirtualDevice());
        } else if (device()->deviceType() == bluetoothdevice::BIKE) {
            delete static_cast<virtualbike *>(device()->VirtualDevice());
        } else if (device()->deviceType() == bluetoothdevice::ELLIPTICAL) {
            delete static_cast<virtualtreadmill *>(device()->VirtualDevice());
        }
    }

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
    if (horizonTreadmill) {

        delete horizonTreadmill;
        horizonTreadmill = nullptr;
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
    if (domyosElliptical) {

        delete domyosElliptical;
        domyosElliptical = nullptr;
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
    if (sportsTechBike) {

        delete sportsTechBike;
        sportsTechBike = nullptr;
    }
    if (sportsPlusBike) {

        delete sportsPlusBike;
        sportsPlusBike = nullptr;
    }
    if (inspireBike) {

        delete inspireBike;
        inspireBike = nullptr;
    }
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
    } else if (domyosElliptical) {
        return domyosElliptical;
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
    } else if (proformWifiTreadmill) {
        return proformWifiTreadmill;
    } else if (nordictrackifitadbTreadmill) {
        return nordictrackifitadbTreadmill;
    } else if (nordictrackifitadbBike) {
        return nordictrackifitadbBike;
    } else if (powerBike) {
        return powerBike;
    } else if (powerTreadmill) {
        return powerTreadmill;
    } else if (fakeBike) {
        return fakeBike;
    } else if (fakeElliptical) {
        return fakeElliptical;
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
    } else if (spiritTreadmill) {
        return spiritTreadmill;
    } else if (activioTreadmill) {
        return activioTreadmill;
    } else if (trxappgateusb) {
        return trxappgateusb;
    } else if (trxappgateusbBike) {
        return trxappgateusbBike;
    } else if (soleBike) {
        return soleBike;
    } else if (keepBike) {
        return keepBike;
    } else if (ultraSportBike) {
        return ultraSportBike;
    } else if (horizonTreadmill) {
        return horizonTreadmill;
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
    } else if (echelonConnectSport) {
        return echelonConnectSport;
    } else if (echelonRower) {
        return echelonRower;
    } else if (echelonStride) {
        return echelonStride;
    } else if (octaneTreadmill) {
        return octaneTreadmill;
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
    } else if (sportsTechBike) {
        return sportsTechBike;
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
    } else if (skandikaWiriBike) {
        return skandikaWiriBike;
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
    return false;
}

#if (QT_VERSION >= QT_VERSION_CHECK(5, 12, 0))
void bluetooth::deviceUpdated(const QBluetoothDeviceInfo &device, QBluetoothDeviceInfo::Fields updateFields) {

    debug("deviceUpdated " + device.name() + " " + updateFields);
}
#endif
