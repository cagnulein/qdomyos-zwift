#include <QBluetoothLocalDevice>
#include <QDateTime>
#include <QFile>
#include <QMetaEnum>
#include "homeform.h"
#include <QtXml>
#ifdef Q_OS_ANDROID
#include "keepawakehelper.h"
#include <QAndroidJniObject>
#endif
#include "bluetooth.h"


bluetooth::bluetooth(bool logs, const QString &deviceName, bool noWriteResistance, bool noHeartService,
                     uint32_t pollDeviceTime, bool noConsole, bool testResistance, uint8_t bikeResistanceOffset,
                     double bikeResistanceGain) {
    QSettings settings;
    bool trx_route_key = settings.value(QStringLiteral("trx_route_key"), false).toBool();
    bool bh_spada_2 = settings.value(QStringLiteral("bh_spada_2"), false).toBool();
    bool technogym_myrun_treadmill_experimental =
            settings.value(QStringLiteral("technogym_myrun_treadmill_experimental"), false).toBool();

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

            discoveryAgent->stop();
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

#ifndef Q_OS_IOS
        if (!trx_route_key && !bh_spada_2 && !technogym_myrun_treadmill_experimental)
#endif
            discoveryAgent->start(QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);
#ifndef Q_OS_IOS
        else
            discoveryAgent->start(QBluetoothDeviceDiscoveryAgent::ClassicMethod |
                                  QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);
#endif
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
    bool trx_route_key = settings.value(QStringLiteral("trx_route_key"), false).toBool();
    bool bh_spada_2 = settings.value(QStringLiteral("bh_spada_2"), false).toBool();
    bool heartRateBeltFound = heartRateBeltName.startsWith(QStringLiteral("Disabled"));
    bool ftmsAccessoryFound = ftmsAccessoryName.startsWith(QStringLiteral("Disabled"));
    bool technogym_myrun_treadmill_experimental =
            settings.value(QStringLiteral("technogym_myrun_treadmill_experimental"), false).toBool();

    // since i can have multiple fanfit i can't wait more because i don't have the full list of the fanfit
    // devices connected to QZ
    // bool fitmetriaFanfitEnabled = settings.value(QStringLiteral("fitmetria_fanfit_enable"), false).toBool();

    if ((!heartRateBeltFound && !heartRateBeltAvailable()) || (!ftmsAccessoryFound && ftmsAccessoryAvailable()) ||
            (!cscFound && !cscSensorAvailable()) || (!powerSensorFound && powerSensorAvailable()) ||
            (!eliteRizerFound && !eliteRizerAvailable()) || (!eliteSterzoSmartFound && eliteSterzoSmartAvailable())) {

        // force heartRateBelt off
        forceHeartBeltOffForTimeout = true;
    }

#ifndef Q_OS_IOS
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

void bluetooth::canceled() {
    debug(QStringLiteral("BTLE scanning stops"));

    emit searchingStop();
}

void bluetooth::debug(const QString &text) {
    if (logs) {

        qDebug() << text;
    }
}

bool bluetooth::cscSensorAvailable() {

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

bool bluetooth::ftmsAccessoryAvailable() {

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

bool bluetooth::powerSensorAvailable() {

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

bool bluetooth::eliteRizerAvailable() {

    QSettings settings;
    QString eliteRizerName = settings.value(QStringLiteral("elite_rizer_name"), QStringLiteral("Disabled")).toString();

    Q_FOREACH (QBluetoothDeviceInfo b, devices) {
        if (!eliteRizerName.compare(b.name())) {

            return true;
        }
    }
    return false;
}

bool bluetooth::eliteSterzoSmartAvailable() {

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

bool bluetooth::heartRateBeltAvailable() {

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


bluetoothdevice * bluetooth::detect_m3iBike(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings) {
    auto m3iBike = dynamic_cast<m3ibike*>(this->device());
    if (b.name().startsWith(QStringLiteral("M3")) && !m3iBike && filter) {

        if (m3ibike::isCorrectUnit(b)) {
            discoveryAgent->stop();
            m3iBike = new m3ibike(noWriteResistance, noHeartService);
            emit deviceConnected(b);
            connect(m3iBike, &bluetoothdevice::connectedAndDiscovered, this,
                    &bluetooth::connectedAndDiscovered);
            // connect(domyosBike, SIGNAL(disconnected()), this, SLOT(restart()));
            connect(m3iBike, &m3ibike::debug, this, &bluetooth::debug);
            m3iBike->deviceDiscovered(b);
            connect(this, &bluetooth::searchingStop, m3iBike, &m3ibike::searchingStop);
            return m3iBike;
        }
    }
    return nullptr;
}

bluetoothdevice * bluetooth::detect_fakeBike(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings) {
    bool fake_bike = settings.value(QStringLiteral("applewatch_fakedevice"), false).toBool();

    if(!fake_bike) return nullptr;

    auto fakeBike = dynamic_cast<fakebike*>(this->device());
    if(fakeBike)
        return nullptr;

    discoveryAgent->stop();
    fakeBike = new fakebike(noWriteResistance, noHeartService, false);
    emit deviceConnected(b);
    connect(fakeBike, &bluetoothdevice::connectedAndDiscovered, this, &bluetooth::connectedAndDiscovered);
    connect(fakeBike, &fakebike::inclinationChanged, this, &bluetooth::inclinationChanged);
    // connect(cscBike, SIGNAL(disconnected()), this, SLOT(restart()));
    // connect(this, SIGNAL(searchingStop()), fakeBike, SLOT(searchingStop())); //NOTE: Commented due to
    // #358

    return fakeBike;
}

bluetoothdevice * bluetooth::detect_fakeElliptical(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings) {
    bool fakedevice_elliptical = settings.value(QStringLiteral("fakedevice_elliptical"), false).toBool();
    auto fakeElliptical = dynamic_cast<fakeelliptical*>(this->device());

    if(!fakedevice_elliptical || fakeElliptical)
        return nullptr;

    discoveryAgent->stop();
    fakeElliptical = new fakeelliptical(noWriteResistance, noHeartService, false);
    emit deviceConnected(b);
    connect(fakeElliptical, &bluetoothdevice::connectedAndDiscovered, this,
            &bluetooth::connectedAndDiscovered);
    connect(fakeElliptical, &fakeelliptical::inclinationChanged, this, &bluetooth::inclinationChanged);
    // connect(cscBike, SIGNAL(disconnected()), this, SLOT(restart()));
    // connect(this, SIGNAL(searchingStop()), fakeBike, SLOT(searchingStop())); //NOTE: Commented due to
    // #358

    return fakeElliptical;

}

bluetoothdevice * bluetooth::detect_proformWifiBike(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings) {
    QString proformtdf4ip = settings.value(QStringLiteral("proformtdf4ip"), "").toString();
    auto proformWifiBike = dynamic_cast<proformwifibike*>(this->device());

    if(proformtdf4ip.isEmpty() || proformWifiBike)
        return nullptr;

    discoveryAgent->stop();
    proformWifiBike =
            new proformwifibike(noWriteResistance, noHeartService, bikeResistanceOffset, bikeResistanceGain);
    emit deviceConnected(b);
    connect(proformWifiBike, &bluetoothdevice::connectedAndDiscovered, this,
            &bluetooth::connectedAndDiscovered);
    // connect(cscBike, SIGNAL(disconnected()), this, SLOT(restart()));
    connect(proformWifiBike, &proformwifibike::debug, this, &bluetooth::debug);
    proformWifiBike->deviceDiscovered(b);
    // connect(this, SIGNAL(searchingStop()), cscBike, SLOT(searchingStop())); //NOTE: Commented due to #358

    return proformWifiBike;
}

bluetoothdevice * bluetooth::detect_bhfitnesselliptical(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings) {
    auto bhFitnessElliptical = dynamic_cast<bhfitnesselliptical*>(this->device());

    if ((b.name().toUpper().startsWith(QStringLiteral("B01_"))) && !bhFitnessElliptical && filter) {
        discoveryAgent->stop();
        bhFitnessElliptical = new bhfitnesselliptical(noWriteResistance, noHeartService, bikeResistanceOffset,
                                                      bikeResistanceGain);
        emit deviceConnected(b);
        connect(bhFitnessElliptical, &bluetoothdevice::connectedAndDiscovered, this,
                &bluetooth::connectedAndDiscovered);
        // connect(bhFitnessElliptical, SIGNAL(disconnected()), this, SLOT(restart()));
        connect(bhFitnessElliptical, &bhfitnesselliptical::debug, this, &bluetooth::debug);
        bhFitnessElliptical->deviceDiscovered(b);
        // connect(this, &bluetooth::searchingStop, bhFitnessElliptical, &bhfitnesselliptical::searchingStop);
        return bhFitnessElliptical;
    }
    return nullptr;
}

bluetoothdevice * bluetooth::detect_bowflext216treadmill(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings) {
    auto bowflexT216Treadmill = dynamic_cast<bowflext216treadmill*>(this->device());

    if (b.name().toUpper().startsWith(QStringLiteral("BOWFLEX T216")) && !bowflexT216Treadmill &&
            filter) {
        discoveryAgent->stop();
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
        return bowflexT216Treadmill;
    }

    return nullptr;
}

bluetoothdevice * bluetooth::detect_fitshowtreadmill(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings) {
    auto fitshowTreadmill = dynamic_cast<fitshowtreadmill*>(this->device());
    auto ftmsBike = dynamic_cast<ftmsbike*>(this->device());

    bool snode_bike = settings.value(QStringLiteral("snode_bike"), false).toBool();
    bool fitplus_bike = settings.value(QStringLiteral("fitplus_bike"), false).toBool() ||
                        settings.value(QStringLiteral("virtufit_etappe"), false).toBool();

    if (((b.name().startsWith(QStringLiteral("FS-")) && !snode_bike && !fitplus_bike && !ftmsBike) ||
         (b.name().startsWith(QStringLiteral("SW")) && b.name().length() == 14) ||
         (b.name().startsWith(QStringLiteral("BF70")))) &&
            !fitshowTreadmill && filter) {
        discoveryAgent->stop();
        fitshowTreadmill = new fitshowtreadmill(this->pollDeviceTime, noConsole, noHeartService);
        emit deviceConnected(b);
        connect(fitshowTreadmill, &bluetoothdevice::connectedAndDiscovered, this,
                &bluetooth::connectedAndDiscovered);
        connect(fitshowTreadmill, &fitshowtreadmill::debug, this, &bluetooth::debug);
        fitshowTreadmill->deviceDiscovered(b);
        connect(this, &bluetooth::searchingStop, fitshowTreadmill, &fitshowtreadmill::searchingStop);
        return fitshowTreadmill;
    }

    return nullptr;
}

bluetoothdevice * bluetooth::detect_concept2skierg(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings) {
    auto concept2Skierg = dynamic_cast<concept2skierg*>(this->device());

    if ((b.name().toUpper().startsWith(QStringLiteral("PM5")) &&
         b.name().toUpper().endsWith(QStringLiteral("SKI"))) &&
            !concept2Skierg && filter) {
        discoveryAgent->stop();
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
        return concept2Skierg;
    }

    return nullptr;
}

bluetoothdevice * bluetooth::detect_domyostreadmill(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings) {
    auto domyosBike = dynamic_cast<domyosbike*>(this->device());
    auto domyosRower = dynamic_cast<domyosrower*>(this->device());
    auto domyosElliptical = dynamic_cast<domyoselliptical*>(this->device());
    auto domyos = dynamic_cast<domyostreadmill*>(this->device());

    if (b.name().startsWith(QStringLiteral("Domyos")) &&
            !b.name().startsWith(QStringLiteral("DomyosBr")) && !domyos && !domyosElliptical &&
            !domyosBike && !domyosRower && filter) {
        settings.setValue(QStringLiteral("bluetooth_lastdevice_name"), b.name());
#ifndef Q_OS_IOS
        settings.setValue(QStringLiteral("bluetooth_lastdevice_address"), b.address().toString());
#else
        settings.setValue("bluetooth_lastdevice_address", b.deviceUuid().toString());
#endif

        discoveryAgent->stop();
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
        return domyos;
    }
    return nullptr;
}

bluetoothdevice * bluetooth::detect_domyosbike(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings) {
    auto domyosBike = dynamic_cast<domyosbike*>(this->device());

    if (b.name().startsWith(QStringLiteral("Domyos-Bike")) &&
            !b.name().startsWith(QStringLiteral("DomyosBridge")) && !domyosBike && filter) {
        discoveryAgent->stop();
        domyosBike = new domyosbike(noWriteResistance, noHeartService, testResistance, bikeResistanceOffset,
                                    bikeResistanceGain);
        emit deviceConnected(b);
        connect(domyosBike, &bluetoothdevice::connectedAndDiscovered, this, &bluetooth::connectedAndDiscovered);
        // connect(domyosBike, SIGNAL(disconnected()), this, SLOT(restart()));
        // connect(domyosBike, SIGNAL(debug(QString)), this, SLOT(debug(QString)));//NOTE: Commented due to #358
        domyosBike->deviceDiscovered(b);
        connect(this, &bluetooth::searchingStop, domyosBike, &domyosbike::searchingStop);
        return domyosBike;
    }

    return nullptr;
}

bluetoothdevice * bluetooth::detect_domyosrower(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings) {
    auto domyosRower = dynamic_cast<domyosrower*>(this->device());
    if (b.name().toUpper().startsWith(QStringLiteral("DOMYOS-ROW")) &&
            !b.name().startsWith(QStringLiteral("DomyosBridge")) && !domyosRower && filter) {
        discoveryAgent->stop();
        domyosRower = new domyosrower(noWriteResistance, noHeartService, testResistance, bikeResistanceOffset,
                                      bikeResistanceGain);
        emit deviceConnected(b);
        connect(domyosRower, &bluetoothdevice::connectedAndDiscovered, this,
                &bluetooth::connectedAndDiscovered);
        // connect(domyosBike, SIGNAL(disconnected()), this, SLOT(restart()));
        connect(domyosRower, &domyosrower::debug, this, &bluetooth::debug);
        domyosRower->deviceDiscovered(b);
        connect(this, &bluetooth::searchingStop, domyosRower, &domyosrower::searchingStop);
        return domyosRower;
    }
    return nullptr;
}

bluetoothdevice * bluetooth::detect_domyoselliptical(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings) {
    auto domyosElliptical = dynamic_cast<domyoselliptical*>(this->device());
    if (b.name().startsWith(QStringLiteral("Domyos-EL")) &&
            !b.name().startsWith(QStringLiteral("DomyosBridge")) && !domyosElliptical && filter) {
        discoveryAgent->stop();
        domyosElliptical = new domyoselliptical(noWriteResistance, noHeartService, testResistance,
                                                bikeResistanceOffset, bikeResistanceGain);
        emit deviceConnected(b);
        connect(domyosElliptical, &bluetoothdevice::connectedAndDiscovered, this,
                &bluetooth::connectedAndDiscovered);
        // connect(domyosElliptical, SIGNAL(disconnected()), this, SLOT(restart()));
        connect(domyosElliptical, &domyoselliptical::debug, this, &bluetooth::debug);
        domyosElliptical->deviceDiscovered(b);
        connect(this, &bluetooth::searchingStop, domyosElliptical, &domyoselliptical::searchingStop);
        return domyosElliptical;
    }
    return nullptr;
}

bluetoothdevice * bluetooth::detect_toorxtreadmill(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings) {
    auto toorx = dynamic_cast<toorxtreadmill*>(this->device());

    if ((b.name().startsWith(QStringLiteral("TRX ROUTE KEY"))) && !toorx && filter) {
        discoveryAgent->stop();
        toorx = new toorxtreadmill();
        emit deviceConnected(b);
        connect(toorx, &bluetoothdevice::connectedAndDiscovered, this, &bluetooth::connectedAndDiscovered);
        // connect(toorx, SIGNAL(disconnected()), this, SLOT(restart()));
        connect(toorx, &toorxtreadmill::debug, this, &bluetooth::debug);
        toorx->deviceDiscovered(b);
        return toorx;
    }

    return nullptr;
}

bluetoothdevice * bluetooth::detect_iconceptbike(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings) {
    auto iConceptBike = dynamic_cast<iconceptbike*>(this->device());

    if ((b.name().toUpper().startsWith(QStringLiteral("BH DUALKIT"))) && !iConceptBike && filter) {
        discoveryAgent->stop();
        iConceptBike = new iconceptbike();
        emit deviceConnected(b);
        connect(iConceptBike, &bluetoothdevice::connectedAndDiscovered, this,
                &bluetooth::connectedAndDiscovered);
        // connect(toorx, SIGNAL(disconnected()), this, SLOT(restart()));
        connect(iConceptBike, &iconceptbike::debug, this, &bluetooth::debug);
        iConceptBike->deviceDiscovered(b);
        return iConceptBike;
    }

    return nullptr;
}

bluetoothdevice * bluetooth::detect_trxappgateusbtreadmill(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings) {
    auto trxappgateusb = dynamic_cast<trxappgateusbtreadmill*>(this->device());
    auto trxappgateusbBike = dynamic_cast<trxappgateusbbike*>(this->device());

    bool toorx_bike = toorxBikeAvailable(settings);

    if (((b.name().startsWith(QStringLiteral("TOORX"))) ||
         (b.name().startsWith(QStringLiteral("V-RUN"))) ||
         (b.name().toUpper().startsWith(QStringLiteral("I-CONSOLE+"))) ||
         (b.name().toUpper().startsWith(QStringLiteral("ICONSOLE+"))) ||
         (b.name().toUpper().startsWith(QStringLiteral("I-RUNNING"))) ||
         (b.name().toUpper().startsWith(QStringLiteral("DKN RUN"))) ||
         (b.name().toUpper().startsWith(QStringLiteral("REEBOK")))) &&
            !trxappgateusb && !trxappgateusbBike && !toorx_bike && filter) {
        discoveryAgent->stop();
        trxappgateusb = new trxappgateusbtreadmill();
        emit deviceConnected(b);
        connect(trxappgateusb, &bluetoothdevice::connectedAndDiscovered, this,
                &bluetooth::connectedAndDiscovered);
        // connect(trxappgateusb, SIGNAL(disconnected()), this, SLOT(restart()));
        connect(trxappgateusb, &trxappgateusbtreadmill::debug, this, &bluetooth::debug);
        trxappgateusb->deviceDiscovered(b);
        return trxappgateusb;
    }

    return nullptr;
}

bluetoothdevice * bluetooth::detect_spirittreadmill(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings) {
    auto spiritTreadmill = dynamic_cast<spirittreadmill*>(this->device());

    if ((b.name().toUpper().startsWith(QStringLiteral("XT385")) ||
         b.name().toUpper().startsWith(QStringLiteral("XT485")) ||
         b.name().toUpper().startsWith(QStringLiteral("XT900"))) &&
            !spiritTreadmill && filter) {
        discoveryAgent->stop();
        spiritTreadmill = new spirittreadmill();
        emit deviceConnected(b);
        connect(spiritTreadmill, &bluetoothdevice::connectedAndDiscovered, this,
                &bluetooth::connectedAndDiscovered);
        // connect(spiritTreadmill, SIGNAL(disconnected()), this, SLOT(restart()));
        connect(spiritTreadmill, &spirittreadmill::debug, this, &bluetooth::debug);
        spiritTreadmill->deviceDiscovered(b);
        return spiritTreadmill;
    }

    return nullptr;
}

bluetoothdevice * bluetooth::detect_activiotreadmill(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings) {
    auto activioTreadmill = dynamic_cast<activiotreadmill*>(this->device());
    if (b.name().toUpper().startsWith(QStringLiteral("RUNNERT")) && !activioTreadmill && filter) {
        discoveryAgent->stop();
        activioTreadmill = new activiotreadmill();
        emit deviceConnected(b);
        connect(activioTreadmill, &bluetoothdevice::connectedAndDiscovered, this,
                &bluetooth::connectedAndDiscovered);
        // connect(activioTreadmill, SIGNAL(disconnected()), this, SLOT(restart()));
        connect(activioTreadmill, &activiotreadmill::debug, this, &bluetooth::debug);
        activioTreadmill->deviceDiscovered(b);
        return activioTreadmill;
    }

    return nullptr;
}

bluetoothdevice * bluetooth::detect_nautilusbike(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings) {
    auto nautilusBike = dynamic_cast<nautilusbike*>(this->device());

    if ((b.name().toUpper().startsWith(QStringLiteral("NAUTILUS B"))) && !nautilusBike &&
            filter) { // NAUTILUS B628
        discoveryAgent->stop();
        nautilusBike = new nautilusbike(noWriteResistance, noHeartService, testResistance, bikeResistanceOffset,
                                        bikeResistanceGain);
        emit deviceConnected(b);
        connect(nautilusBike, &bluetoothdevice::connectedAndDiscovered, this,
                &bluetooth::connectedAndDiscovered);
        // connect(nautilusBike, SIGNAL(disconnected()), this, SLOT(restart()));
        connect(nautilusBike, &nautilusbike::debug, this, &bluetooth::debug);
        nautilusBike->deviceDiscovered(b);
        connect(this, &bluetooth::searchingStop, nautilusBike, &nautilusbike::searchingStop);
        return nautilusBike;
    }
    return nullptr;

}

bluetoothdevice * bluetooth::detect_nautiluselliptical(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings) {
    auto nautilusElliptical = dynamic_cast<nautiluselliptical*>(this->device());

    if ((b.name().toUpper().startsWith(QStringLiteral("NAUTILUS E"))) &&
            !nautilusElliptical && // NAUTILUS E616
            filter) {
        discoveryAgent->stop();
        nautilusElliptical = new nautiluselliptical(noWriteResistance, noHeartService, testResistance,
                                                    bikeResistanceOffset, bikeResistanceGain);
        emit deviceConnected(b);
        connect(nautilusElliptical, &bluetoothdevice::connectedAndDiscovered, this,
                &bluetooth::connectedAndDiscovered);
        // connect(nautilusElliptical, SIGNAL(disconnected()), this, SLOT(restart()));
        connect(nautilusElliptical, &nautiluselliptical::debug, this, &bluetooth::debug);
        nautilusElliptical->deviceDiscovered(b);
        connect(this, &bluetooth::searchingStop, nautilusElliptical, &nautiluselliptical::searchingStop);
        return nautilusElliptical;
    }
    return nullptr;
}

bluetoothdevice * bluetooth::detect_nautilustreadmill(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings) {
    auto nautilusTreadmill = dynamic_cast<nautilustreadmill*>(this->device());

    if (b.name().toUpper().startsWith(QStringLiteral("NAUTILUS T")) && !nautilusTreadmill && filter) {
        discoveryAgent->stop();
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
        return nautilusTreadmill;
    }

    return nullptr;
}

bluetoothdevice * bluetooth::detect_trxappgateusbbike(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings) {
    auto trxappgateusbBike = dynamic_cast<trxappgateusbbike*>(this->device());
    auto trxappgateusb = dynamic_cast<trxappgateusbtreadmill*>(this->device());

    bool toorx_bike = this->toorxBikeAvailable(settings);

    auto upperName = b.name().toUpper();
    if ((((b.name().startsWith(QStringLiteral("TOORX")) ||
           upperName.startsWith(QStringLiteral("I-CONSOIE+")) ||
           upperName.startsWith(QStringLiteral("I-CONSOLE+")) ||
           upperName.startsWith(QStringLiteral("IBIKING+")) ||
           upperName.startsWith(QStringLiteral("ICONSOLE+")) ||
           upperName.startsWith(QStringLiteral("VIFHTR2.1")) ||
           upperName.contains(QStringLiteral("CR011R")) ||
           upperName.startsWith(QStringLiteral("DKN MOTION"))) &&
          (toorx_bike))) &&
            !trxappgateusb && !trxappgateusbBike && filter) {
        discoveryAgent->stop();
        trxappgateusbBike =
                new trxappgateusbbike(noWriteResistance, noHeartService, bikeResistanceOffset, bikeResistanceGain);
        emit deviceConnected(b);
        connect(trxappgateusbBike, &bluetoothdevice::connectedAndDiscovered, this,
                &bluetooth::connectedAndDiscovered);
        // connect(trxappgateusb, SIGNAL(disconnected()), this, SLOT(restart()));
        connect(trxappgateusbBike, &trxappgateusbbike::debug, this, &bluetooth::debug);
        trxappgateusbBike->deviceDiscovered(b);
        return trxappgateusbBike;
    }

    return nullptr;
}

bluetoothdevice * bluetooth::detect_echelonconnectsport(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings) {
    auto echelonConnectSport = dynamic_cast<echelonconnectsport*>(this->device());
    auto echelonRower = dynamic_cast<echelonrower*>(this->device());
    auto echelonStride = dynamic_cast<echelonstride*>(this->device());

    if (b.name().startsWith(QStringLiteral("ECH")) && !echelonRower && !echelonStride &&
            !echelonConnectSport && filter) {
        discoveryAgent->stop();
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
        return echelonConnectSport;
    }

    return nullptr;
}

bluetoothdevice * bluetooth::detect_yesoulbike(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings) {
    auto yesoulBike = dynamic_cast<yesoulbike*>(this->device());

    if (b.name().startsWith(QStringLiteral("YESOUL")) && !yesoulBike && filter) {
        discoveryAgent->stop();
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
        return yesoulBike;
    }

    return nullptr;
}

bluetoothdevice * bluetooth::detect_flywheelbike(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings) {
    auto flywheelBike = dynamic_cast<flywheelbike*>(this->device());
    bool flywheel_life_fitness_ic8 = settings.value(QStringLiteral("flywheel_life_fitness_ic8"), false).toBool();
    if ((b.name().startsWith(QStringLiteral("Flywheel")) ||
         // BIKE 1, BIKE 2, BIKE 3...
         (b.name().toUpper().startsWith(QStringLiteral("BIKE")) && flywheel_life_fitness_ic8 == true &&
          b.name().length() == 6)) &&
            !flywheelBike && filter) {
        discoveryAgent->stop();
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
        return flywheelBike;
    }


    return nullptr;
}

bluetoothdevice * bluetooth::detect_nordictrackelliptical(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings) {
    auto nordictrackElliptical = dynamic_cast<nordictrackelliptical*>(this->device());

    if ((b.name().toUpper().startsWith(QStringLiteral("I_EL"))) &&
            !nordictrackElliptical && filter) {
        discoveryAgent->stop();
        nordictrackElliptical = new nordictrackelliptical(noWriteResistance, noHeartService,
                                                          bikeResistanceOffset, bikeResistanceGain);
        emit deviceConnected(b);
        connect(nordictrackElliptical, &bluetoothdevice::connectedAndDiscovered, this,
                &bluetooth::connectedAndDiscovered);
        // connect(nordictrackElliptical, SIGNAL(disconnected()), this, SLOT(restart()));
        connect(nordictrackElliptical, &nordictrackelliptical::debug, this, &bluetooth::debug);
        nordictrackElliptical->deviceDiscovered(b);
        // connect(this, &bluetooth::searchingStop, proformElliptical, &proformelliptical::searchingStop);
        return nordictrackElliptical;

    }
    return nullptr;
}

bluetoothdevice * bluetooth::detect_nordictrackifitadbtreadmill(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings) {
    auto nordictrackifitadbTreadmill = dynamic_cast<nordictrackifitadbtreadmill*>(this->device());
    QString nordictrack_2950_ip = settings.value(QStringLiteral("nordictrack_2950_ip"), "").toString();

    if (!nordictrack_2950_ip.isEmpty() && !nordictrackifitadbTreadmill) {
        discoveryAgent->stop();
        nordictrackifitadbTreadmill = new nordictrackifitadbtreadmill(noWriteResistance, noHeartService);
        emit deviceConnected(b);
        connect(nordictrackifitadbTreadmill, &bluetoothdevice::connectedAndDiscovered, this,
                &bluetooth::connectedAndDiscovered);
        connect(nordictrackifitadbTreadmill, &nordictrackifitadbtreadmill::debug, this, &bluetooth::debug);
        // nordictrackifitadbTreadmill->deviceDiscovered(b);
        // connect(this, SIGNAL(searchingStop()), cscBike, SLOT(searchingStop())); //NOTE: Commented due to #358
        return nordictrackifitadbTreadmill;
    }
    return nullptr;
}

bluetoothdevice * bluetooth::detect_octanetreadmill(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings) {
    auto octaneTreadmill = dynamic_cast<octanetreadmill*>(this->device());

    if ((b.name().toUpper().startsWith(QLatin1String("ZR7"))) && !octaneTreadmill && filter) {
        discoveryAgent->stop();
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
        return octaneTreadmill;
    }

    return nullptr;
}

bluetoothdevice * bluetooth::detect_proformrower(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings) {
    auto proformRower = dynamic_cast<proformrower*>(this->device());
    if ((b.name().toUpper().startsWith(QStringLiteral("I_RW"))) && !proformRower && filter) {
        discoveryAgent->stop();
        proformRower = new proformrower(noWriteResistance, noHeartService);
        emit deviceConnected(b);
        connect(proformRower, &bluetoothdevice::connectedAndDiscovered, this,
                &bluetooth::connectedAndDiscovered);
        // connect(proformRower, SIGNAL(disconnected()), this, SLOT(restart()));
        connect(proformRower, &proformrower::debug, this, &bluetooth::debug);
        proformRower->deviceDiscovered(b);
        // connect(this, &bluetooth::searchingStop, proformElliptical, &proformelliptical::searchingStop);
        return proformRower;
    }
    return nullptr;
}

bluetoothdevice * bluetooth::detect_proformbike(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings) {
    auto proformBike = dynamic_cast<proformbike*>(this->device());

    if ((b.name().startsWith(QStringLiteral("I_EB")) || b.name().startsWith(QStringLiteral("I_SB"))) &&
            !proformBike && filter) {
        discoveryAgent->stop();
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
        return proformBike;
    }

    return nullptr;
}

bluetoothdevice * bluetooth::detect_proformwifitreadmill(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings) {
    auto proformWifiTreadmill = dynamic_cast<proformwifitreadmill*>(this->device());
    QString proformtreadmillip = settings.value(QStringLiteral("proformtreadmillip"), "").toString();

    if (!proformtreadmillip.isEmpty() && !proformWifiTreadmill) {
        discoveryAgent->stop();
        proformWifiTreadmill = new proformwifitreadmill(noWriteResistance, noHeartService, bikeResistanceOffset,
                                                        bikeResistanceGain);
        emit deviceConnected(b);
        connect(proformWifiTreadmill, &bluetoothdevice::connectedAndDiscovered, this,
                &bluetooth::connectedAndDiscovered);
        // connect(cscBike, SIGNAL(disconnected()), this, SLOT(restart()));
        connect(proformWifiTreadmill, &proformwifitreadmill::debug, this, &bluetooth::debug);
        proformWifiTreadmill->deviceDiscovered(b);
        return proformWifiTreadmill;
    }
    return nullptr;

}

bluetoothdevice * bluetooth::detect_proformelliptical(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings) {
    auto proformElliptical = dynamic_cast<proformelliptical*>(this->device());
    if ((b.name().toUpper().startsWith(QStringLiteral("I_FS"))) &&
            !proformElliptical && filter) {
        discoveryAgent->stop();
        proformElliptical = new proformelliptical(noWriteResistance, noHeartService);
        emit deviceConnected(b);
        connect(proformElliptical, &bluetoothdevice::connectedAndDiscovered, this,
                &bluetooth::connectedAndDiscovered);
        // connect(proformElliptical, SIGNAL(disconnected()), this, SLOT(restart()));
        connect(proformElliptical, &proformelliptical::debug, this, &bluetooth::debug);
        proformElliptical->deviceDiscovered(b);
        // connect(this, &bluetooth::searchingStop, proformElliptical, &proformelliptical::searchingStop);
        return proformElliptical;
    }
    return nullptr;
}

bluetoothdevice * bluetooth::detect_proformellipticaltrainer(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings) {
    auto proformEllipticalTrainer = dynamic_cast<proformellipticaltrainer*>(this->device());

    if ((b.name().toUpper().startsWith(QStringLiteral("I_VE"))) && !proformEllipticalTrainer && filter) {
        discoveryAgent->stop();
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
        return proformEllipticalTrainer;
    }
    return nullptr;
}

bluetoothdevice * bluetooth::detect_proformtreadmill(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings) {
    auto proformTreadmill = dynamic_cast<proformtreadmill*>(this->device());

    if ((b.name().startsWith(QStringLiteral("I_TL"))) && !proformTreadmill && filter) {
        discoveryAgent->stop();
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
        return proformTreadmill;
    }

    return nullptr;
}

bluetoothdevice * bluetooth::detect_horizontreadmill(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings) {
    auto horizonTreadmill = dynamic_cast<horizontreadmill*>(this->device());
    auto upperName = b.name().toUpper();
    if ((upperName.startsWith(QStringLiteral("HORIZON")) ||
         upperName.startsWith(QStringLiteral("AFG SPORT")) ||
         upperName.startsWith(QStringLiteral("WLT2541")) ||
         upperName.startsWith(QStringLiteral("S77")) ||
         upperName.startsWith(QStringLiteral("T318_")) ||   // FTMS
         upperName.startsWith(QStringLiteral("T218_")) ||   // FTMS
         upperName.startsWith(QStringLiteral("TRX3500")) || // FTMS
         upperName.startsWith(QStringLiteral("JFTMPARAGON")) ||
         upperName.startsWith(QStringLiteral("JFTM")) ||    // FTMS
         upperName.startsWith(QStringLiteral("CT800")) ||   // FTMS
         upperName.startsWith(QStringLiteral("TRX4500")) || // FTMS
         upperName.startsWith(QStringLiteral("ESANGLINKER"))) &&
            !horizonTreadmill && filter) {
        discoveryAgent->stop();
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
        return horizonTreadmill;
    }

    return nullptr;
}

bluetoothdevice * bluetooth::detect_technogymmyruntreadmill(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings) {
    auto technogymmyrunTreadmill = dynamic_cast<technogymmyruntreadmill*>(this->device());

    if ((b.name().toUpper().startsWith(QStringLiteral("MYRUN ")) ||
         b.name().toUpper().startsWith(QStringLiteral("MERACH-U3")) // FTMS
         ) &&
            !technogymmyrunTreadmill && filter) {
        discoveryAgent->stop();

#ifndef Q_OS_IOS
        bool technogym_myrun_treadmill_experimental = settings.value(QStringLiteral("technogym_myrun_treadmill_experimental"), false).toBool();
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

            return technogymmyrunTreadmill;
        }
#ifndef Q_OS_IOS
        else {
            auto technogymmyrunrfcommTreadmill = new technogymmyruntreadmillrfcomm();
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
            return technogymmyrunrfcommTreadmill;
        }
#endif
    }



    return nullptr;
}

bluetoothdevice * bluetooth::detect_truetreadmill(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings) {
    auto trueTreadmill = dynamic_cast<truetreadmill*>(this->device());

    if ((b.name().toUpper().startsWith(QStringLiteral("TRUE")) ||
         b.name().toUpper().startsWith(QStringLiteral("TREADMILL"))) &&
            !trueTreadmill && filter) {
        settings.setValue(QStringLiteral("bluetooth_lastdevice_name"), b.name());
#ifndef Q_OS_IOS
        settings.setValue(QStringLiteral("bluetooth_lastdevice_address"), b.address().toString());
#else
        settings.setValue("bluetooth_lastdevice_address", b.deviceUuid().toString());
#endif

        discoveryAgent->stop();
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
        return trueTreadmill;
    }
    return nullptr;

}

bluetoothdevice * bluetooth::detect_horizongr7bike(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings) {
    auto horizonGr7Bike = dynamic_cast<horizongr7bike*>(this->device());

    if (((b.name().toUpper().startsWith("JFIC")) // HORIZON GR7
         ) &&
            !horizonGr7Bike && filter) {
        discoveryAgent->stop();
        horizonGr7Bike =
                new horizongr7bike(noWriteResistance, noHeartService, bikeResistanceOffset, bikeResistanceGain);
        emit deviceConnected(b);
        connect(horizonGr7Bike, &bluetoothdevice::connectedAndDiscovered, this,
                &bluetooth::connectedAndDiscovered);
        // connect(trxappgateusb, SIGNAL(disconnected()), this, SLOT(restart()));
        connect(horizonGr7Bike, &horizongr7bike::debug, this, &bluetooth::debug);
        horizonGr7Bike->deviceDiscovered(b);

        return horizonGr7Bike;
    }

    return nullptr;
}

bluetoothdevice * bluetooth::detect_schwinnic4bike(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings) {
    auto schwinnIC4Bike = dynamic_cast<schwinnic4bike*>(this->device());

    if ((b.name().toUpper().startsWith(QStringLiteral("IC BIKE")) ||
         (b.name().toUpper().startsWith(QStringLiteral("C7-")) && b.name().length() != 17) ||
         b.name().toUpper().startsWith(QStringLiteral("C9/C10"))) &&
            !schwinnIC4Bike && filter) {
        settings.setValue(QStringLiteral("bluetooth_lastdevice_name"), b.name());
#ifndef Q_OS_IOS
        settings.setValue(QStringLiteral("bluetooth_lastdevice_address"), b.address().toString());
#else
        settings.setValue("bluetooth_lastdevice_address", b.deviceUuid().toString());
#endif
        discoveryAgent->stop();
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
        return schwinnIC4Bike;
    }

    return nullptr;
}

bluetoothdevice * bluetooth::detect_sportstechbike(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings) {
    auto sportsTechBike = dynamic_cast<sportstechbike*>(this->device());

    if (b.name().toUpper().startsWith(QStringLiteral("EW-BK")) && !sportsTechBike && filter) {
        discoveryAgent->stop();
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
        return sportsTechBike;
    }

    return nullptr;
}

bluetoothdevice * bluetooth::detect_sportsplusbike(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings) {
    auto sportsPlusBike = dynamic_cast<sportsplusbike*>(this->device());

    if (b.name().toUpper().startsWith(QStringLiteral("CARDIOFIT")) && !sportsPlusBike && filter) {
        discoveryAgent->stop();
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
        return sportsPlusBike;
    }

    return nullptr;
}

bluetoothdevice * bluetooth::detect_inspirebike(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings) {
    auto inspireBike = dynamic_cast<inspirebike*>(this->device());

    if (b.name().toUpper().startsWith(QStringLiteral("IC")) && b.name().length() == 8 && !inspireBike &&
            filter) {
        discoveryAgent->stop();
        inspireBike = new inspirebike(noWriteResistance, noHeartService);
#if !defined(Q_OS_ANDROID) && !defined(Q_OS_IOS)
        stateFileRead();
#endif
        emit deviceConnected(b);
        connect(inspireBike, &bluetoothdevice::connectedAndDiscovered, this, &bluetooth::connectedAndDiscovered);
        connect(inspireBike, &inspirebike::debug, this, &bluetooth::debug);
        // NOTE: Commented due to #358
        // connect(inspireBike, SIGNAL(speedChanged(double)), this, SLOT(speedChanged(double)));
        // NOTE: Commented due to #358
        // connect(inspireBike, SIGNAL(inclinationChanged(double)), this, SLOT(inclinationChanged(double)));
        inspireBike->deviceDiscovered(b);
        // NOTE: Commented due to #358
        // connect(this, SIGNAL(searchingStop()), inspireBike, SLOT(searchingStop()));
        return inspireBike;
    }

    return nullptr;
}


bluetoothdevice * bluetooth::detect_snodebike(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings) {
    auto snodeBike = dynamic_cast<snodebike*>(this->device());
    auto ftmsBike = dynamic_cast<ftmsbike*>(this->device());
    auto fitPlusBike = dynamic_cast<fitplusbike*>(this->device());

    bool snode_bike = settings.value(QStringLiteral("snode_bike"), false).toBool();

    if (((b.name().startsWith(QStringLiteral("FS-")) && snode_bike) ||
         b.name().startsWith(QStringLiteral("TF-"))) && // TF-769DF2
            !snodeBike &&
            !ftmsBike && !fitPlusBike && filter) {
        discoveryAgent->stop();
        snodeBike = new snodebike(noWriteResistance, noHeartService);
        emit deviceConnected(b);
        connect(snodeBike, &bluetoothdevice::connectedAndDiscovered, this, &bluetooth::connectedAndDiscovered);
        // connect(trxappgateusb, SIGNAL(disconnected()), this, SLOT(restart()));
        connect(snodeBike, &snodebike::debug, this, &bluetooth::debug);
        snodeBike->deviceDiscovered(b);
        return snodeBike;
    }

    return nullptr;
}

bluetoothdevice * bluetooth::detect_eslinkertreadmill(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings) {
    auto eslinkerTreadmill = dynamic_cast<eslinkertreadmill*>(this->device());

    if (b.name().toUpper().startsWith(QStringLiteral("ESLINKER")) && !eslinkerTreadmill && filter) {
        discoveryAgent->stop();
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
        return eslinkerTreadmill;
    }

    return nullptr;
}


bluetoothdevice * bluetooth::detect_skandikawiribike(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings) {
    auto skandikaWiriBike = dynamic_cast<skandikawiribike*>(this->device());

    if (b.name().toUpper().startsWith(QStringLiteral("BFCP")) && !skandikaWiriBike && filter) {
        discoveryAgent->stop();
        skandikaWiriBike =
                new skandikawiribike(noWriteResistance, noHeartService, bikeResistanceOffset, bikeResistanceGain);
        emit deviceConnected(b);
        connect(skandikaWiriBike, &bluetoothdevice::connectedAndDiscovered, this,
                &bluetooth::connectedAndDiscovered);
        // connect(skandikaWiriBike, SIGNAL(disconnected()), this, SLOT(restart()));
        connect(skandikaWiriBike, &skandikawiribike::debug, this, &bluetooth::debug);
        skandikaWiriBike->deviceDiscovered(b);
        return skandikaWiriBike;

    }

    return nullptr;
}

bluetoothdevice * bluetooth::detect_cscbike(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings) {
    auto cscBike = dynamic_cast<cscbike*>(this->device());
    bool csc_as_bike = settings.value(QStringLiteral("cadence_sensor_as_bike"), false).toBool();
    QString cscName = settings.value(QStringLiteral("cadence_sensor_name"), QStringLiteral("Disabled")).toString();
    if (csc_as_bike && b.name().startsWith(cscName) && !cscBike && filter) {

        discoveryAgent->stop();
        cscBike = new cscbike(noWriteResistance, noHeartService, false);
        emit deviceConnected(b);
        connect(cscBike, &bluetoothdevice::connectedAndDiscovered, this, &bluetooth::connectedAndDiscovered);
        // connect(cscBike, SIGNAL(disconnected()), this, SLOT(restart()));
        connect(cscBike, &cscbike::debug, this, &bluetooth::debug);
        cscBike->deviceDiscovered(b);
        // connect(this, SIGNAL(searchingStop()), cscBike, SLOT(searchingStop())); //NOTE: Commented due to #358
        return cscBike;
    }

    return nullptr;
}

bluetoothdevice * bluetooth::detect_mcfbike(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings) {
    auto mcfBike = dynamic_cast<mcfbike*>(this->device());

    if ((b.name().toUpper().startsWith(QStringLiteral("MCF-"))) && !mcfBike && filter) {
        discoveryAgent->stop();
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
        return mcfBike;

    }

    return nullptr;
}

bluetoothdevice * bluetooth::detect_npecablebike(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings) {
    auto npeCableBike = dynamic_cast<npecablebike*>(this->device());
    bool flywheel_life_fitness_ic8 = settings.value(QStringLiteral("flywheel_life_fitness_ic8"), false).toBool();
    if ((b.name().toUpper().startsWith(QStringLiteral(">CABLE")) ||
         (b.name().toUpper().startsWith(QStringLiteral("MD")) && b.name().length() == 7) ||
         // BIKE 1, BIKE 2, BIKE 3...
         (b.name().toUpper().startsWith(QStringLiteral("BIKE")) && flywheel_life_fitness_ic8 == false &&
          b.name().length() == 6)) &&
            !npeCableBike && filter) {
        discoveryAgent->stop();
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

        return npeCableBike;
    }

    return nullptr;
}

bluetoothdevice * bluetooth::detect_stagesbike(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings) {
    auto stagesBike = dynamic_cast<stagesbike*>(this->device());
    auto ftmsBike = dynamic_cast<ftmsbike*>(this->device());

    QString powerSensorName =
            settings.value(QStringLiteral("power_sensor_name"), QStringLiteral("Disabled")).toString();

    if ((b.name().toUpper().startsWith(QStringLiteral("STAGES ")) ||
         (b.name().toUpper().startsWith(QStringLiteral("ASSIOMA")) &&
          powerSensorName.startsWith(QStringLiteral("Disabled")))) &&
            !stagesBike && !ftmsBike && filter) {
        discoveryAgent->stop();
        stagesBike = new stagesbike(noWriteResistance, noHeartService, false);
        // stateFileRead();
        emit deviceConnected(b);
        connect(stagesBike, &bluetoothdevice::connectedAndDiscovered, this, &bluetooth::connectedAndDiscovered);
        // connect(stagesBike, SIGNAL(disconnected()), this, SLOT(restart()));
        connect(stagesBike, &stagesbike::debug, this, &bluetooth::debug);
        // connect(stagesBike, SIGNAL(speedChanged(double)), this, SLOT(speedChanged(double)));
        // connect(stagesBike, SIGNAL(inclinationChanged(double)), this, SLOT(inclinationChanged(double)));
        stagesBike->deviceDiscovered(b);
        return stagesBike;
    }

    return nullptr;
}

bluetoothdevice * bluetooth::detect_solebike(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings) {
    auto soleBike = dynamic_cast<solebike*>(this->device());

    if ((b.name().toUpper().startsWith(QStringLiteral("LCB")) ||
         b.name().toUpper().startsWith(QStringLiteral("R92"))) &&
            !soleBike && filter) {
        discoveryAgent->stop();
        soleBike = new solebike(noWriteResistance, noHeartService, bikeResistanceOffset, bikeResistanceGain);
        emit deviceConnected(b);
        connect(soleBike, &bluetoothdevice::connectedAndDiscovered, this, &bluetooth::connectedAndDiscovered);
        // connect(soleBike, SIGNAL(disconnected()), this, SLOT(restart()));
        // connect(soleBike, &solebike::debug, this, &bluetooth::debug);
        soleBike->deviceDiscovered(b);
        return soleBike;
    }

    return nullptr;
}

bluetoothdevice * bluetooth::detect_soleelliptical(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings) {
    auto soleElliptical = dynamic_cast<soleelliptical*>(this->device());

    auto upperName = b.name().toUpper();

    if ((upperName.startsWith(QStringLiteral("E95S")) ||
         upperName.startsWith(QStringLiteral("E25")) ||
         upperName.startsWith(QStringLiteral("E35")) ||
         upperName.startsWith(QStringLiteral("E55")) ||
         upperName.startsWith(QStringLiteral("E95")) ||
         upperName.startsWith(QStringLiteral("E98")) ||
         upperName.startsWith(QStringLiteral("XG400")) ||
         upperName.startsWith(QStringLiteral("E98S"))) &&
            !soleElliptical && filter) {
        discoveryAgent->stop();
        soleElliptical = new soleelliptical(noWriteResistance, noHeartService, testResistance,
                                            bikeResistanceOffset, bikeResistanceGain);
        emit deviceConnected(b);
        connect(soleElliptical, &bluetoothdevice::connectedAndDiscovered, this,
                &bluetooth::connectedAndDiscovered);
        // connect(soleElliptical, SIGNAL(disconnected()), this, SLOT(restart()));
        connect(soleElliptical, &soleelliptical::debug, this, &bluetooth::debug);
        soleElliptical->deviceDiscovered(b);
        connect(this, &bluetooth::searchingStop, soleElliptical, &soleelliptical::searchingStop);
        return soleElliptical;
    }
    return nullptr;
}

bluetoothdevice * bluetooth::detect_solef80treadmill(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings) {
    auto soleF80 = dynamic_cast<solef80treadmill*>(this->device());
    auto upperName = b.name().toUpper();
    if ((upperName.startsWith(QStringLiteral("F80")) ||
         upperName.startsWith(QStringLiteral("F65")) ||
         upperName.startsWith(QStringLiteral("TT8")) ||
         upperName.startsWith(QStringLiteral("F63")) ||
         upperName.startsWith(QStringLiteral("F85"))) &&
            !soleF80 && filter) {
        discoveryAgent->stop();
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
        return soleF80;
    }

    return nullptr;
}

bluetoothdevice * bluetooth::detect_chronobike(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings) {
    auto chronoBike = dynamic_cast<chronobike*>(this->device());

    if (b.name().toUpper().startsWith(QStringLiteral("CHRONO ")) && !chronoBike && filter) {
        discoveryAgent->stop();
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
        return chronoBike;
    }

    return nullptr;
}

bluetoothdevice * bluetooth::detect_fitplusbike(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings) {
    auto fitPlusBike = dynamic_cast<fitplusbike*>(this->device());
    auto snodeBike = dynamic_cast<snodebike*>(this->device());
    auto ftmsBike = dynamic_cast<ftmsbike*>(this->device());

    bool fitplus_bike = settings.value(QStringLiteral("fitplus_bike"), false).toBool() ||
            settings.value(QStringLiteral("virtufit_etappe"), false).toBool();

    if ((b.name().startsWith(QStringLiteral("FS-")) && fitplus_bike) && !fitPlusBike && !ftmsBike && !snodeBike && filter) {
        discoveryAgent->stop();
        fitPlusBike =
                new fitplusbike(noWriteResistance, noHeartService, bikeResistanceOffset, bikeResistanceGain);
        emit deviceConnected(b);
        connect(fitPlusBike, &bluetoothdevice::connectedAndDiscovered, this,
                &bluetooth::connectedAndDiscovered);
        // connect(fitPlusBike, SIGNAL(disconnected()), this, SLOT(restart()));
        // NOTE: Commented due to #358
        // connect(fitPlusBike, SIGNAL(debug(QString)), this, SLOT(debug(QString)));
        fitPlusBike->deviceDiscovered(b);
        return fitPlusBike;
    }

    return nullptr;
}

bluetoothdevice * bluetooth::detect_echelonrower(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings) {
    auto echelonRower = dynamic_cast<echelonrower*>(this->device());

    if ((b.name().startsWith(QStringLiteral("ECH-ROW")) ||
         b.name().startsWith(QStringLiteral("ROW-S"))) &&
            !echelonRower && filter) {
        discoveryAgent->stop();
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
        return echelonRower;
    }

    return nullptr;
}

bluetoothdevice * bluetooth::detect_ftmsrower(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings) {
    auto ftmsRower = dynamic_cast<ftmsrower*>(this->device());

    if ((b.name().toUpper().startsWith(QStringLiteral("CR 00")) ||
         b.name().toUpper().startsWith(QStringLiteral("KAYAKPRO")) ||
         b.name().toUpper().startsWith(QStringLiteral("WHIPR")) ||
         b.name().toUpper().startsWith(QStringLiteral("I-ROWER")) ||
         (b.name().toUpper().startsWith(QStringLiteral("PM5")) &&
          b.name().toUpper().contains(QStringLiteral("ROW")))) &&
            !ftmsRower && filter) {
        discoveryAgent->stop();
        ftmsRower = new ftmsrower(noWriteResistance, noHeartService);
        // stateFileRead();
        emit deviceConnected(b);
        connect(ftmsRower, &bluetoothdevice::connectedAndDiscovered, this, &bluetooth::connectedAndDiscovered);
        // connect(ftmsRower, SIGNAL(disconnected()), this, SLOT(restart()));
        connect(ftmsRower, SIGNAL(debug(QString)), this, SLOT(debug(QString)));
        // connect(v, SIGNAL(speedChanged(double)), this, SLOT(speedChanged(double)));
        // connect(ftmsRower, SIGNAL(inclinationChanged(double)), this, SLOT(inclinationChanged(double)));
        ftmsRower->deviceDiscovered(b);
        return ftmsRower;
    }

    return nullptr;
}

bluetoothdevice * bluetooth::detect_smartrowrower(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings) {
    auto smartrowRower = dynamic_cast<smartrowrower*>(this->device());

    if (b.name().startsWith(QStringLiteral("SMARTROW")) && !smartrowRower && filter) {
        discoveryAgent->stop();
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
        return smartrowRower;
    }

    return nullptr;
}

bluetoothdevice * bluetooth::detect_echelonstride(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings) {
    auto echelonStride = dynamic_cast<echelonstride*>(this->device());

    if ((b.name().toUpper().startsWith(QLatin1String("ECH-STRIDE")) ||
         b.name().toUpper().startsWith(QLatin1String("ECH-SD-SPT"))) &&
            !echelonStride && filter) {
        discoveryAgent->stop();
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
        return echelonStride;
    }

    return nullptr;
}

bluetoothdevice * bluetooth::detect_keepbike(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings) {
    auto keepBike = dynamic_cast<keepbike*>(this->device());

    if ((b.name().toUpper().startsWith(QStringLiteral("KEEP_BIKE_"))) && !keepBike && filter) {
        discoveryAgent->stop();
        keepBike = new keepbike(noWriteResistance, noHeartService, bikeResistanceOffset, bikeResistanceGain);
        emit deviceConnected(b);
        connect(keepBike, &bluetoothdevice::connectedAndDiscovered, this, &bluetooth::connectedAndDiscovered);
        // connect(keepBike, SIGNAL(disconnected()), this, SLOT(restart()));
        // connect(keepBike, &solebike::debug, this, &bluetooth::debug);
        keepBike->deviceDiscovered(b);
        return keepBike;
    }

    return nullptr;
}

bluetoothdevice * bluetooth::detect_kingsmithr1protreadmill(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings) {
    auto kingsmithR2Treadmill = dynamic_cast<kingsmithr2treadmill*>(this->device());
    auto kingsmithR1ProTreadmill = dynamic_cast<kingsmithr1protreadmill*>(this->device());

    if ((b.name().toUpper().startsWith(QStringLiteral("R1 PRO")) ||
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

        discoveryAgent->stop();
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
        return kingsmithR1ProTreadmill;
    }

    return nullptr;
}

bluetoothdevice * bluetooth::detect_kingsmithr2treadmill(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings) {
    auto kingsmithR2Treadmill = dynamic_cast<kingsmithr2treadmill*>(this->device());
    auto upperName = b.name().toUpper();
    if ((
                // Xiaomi k12 pro treadmill KS-ST-K12PRO
                upperName.startsWith(QStringLiteral("KS-ST-K12PRO")) ||
                // KingSmith Walking Pad R2
                upperName.startsWith(QStringLiteral("KS-R1AC")) ||
                upperName.startsWith(QStringLiteral("KS-HC-R1AA")) ||
                upperName.startsWith(QStringLiteral("KS-HC-R1AC")) ||
                // KingSmith Walking Pad X21
                upperName.startsWith(QStringLiteral("KS-X21")) ||
                upperName.startsWith(QStringLiteral("KS-HDSC-X21C")) ||
                upperName.startsWith(QStringLiteral("KS-HDSY-X21C")) ||
                upperName.startsWith(QStringLiteral("KS-NGCH-X21C"))) &&
            !kingsmithR2Treadmill && filter) {
        settings.setValue(QStringLiteral("bluetooth_lastdevice_name"), b.name());
#ifndef Q_OS_IOS
        settings.setValue(QStringLiteral("bluetooth_lastdevice_address"), b.address().toString());
#else
        settings.setValue("bluetooth_lastdevice_address", b.deviceUuid().toString());
#endif

        discoveryAgent->stop();
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
        return kingsmithR2Treadmill;
    }

    return nullptr;
}

bluetoothdevice * bluetooth::detect_ftmsbike(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings) {
    auto ftmsBike = dynamic_cast<ftmsbike*>(this->device());
    auto stagesBike = dynamic_cast<stagesbike*>(this->device());
    auto fitPlusBike = dynamic_cast<fitplusbike*>(this->device());
    auto snodeBike = dynamic_cast<snodebike*>(this->device());

    bool hammerRacerS = settings.value(QStringLiteral("hammer_racer_s"), false).toBool();
    if (((b.name().startsWith("FS-") && hammerRacerS) ||
         (b.name().toUpper().startsWith("MKSM")) || // MKSM3600036
         (b.name().toUpper().startsWith("WAHOO KICKR")) || (b.name().toUpper().startsWith("B94")) ||
         (b.name().toUpper().startsWith("STAGES BIKE")) || (b.name().toUpper().startsWith("SUITO")) ||
         (b.name().toUpper().startsWith("D2RIDE")) || (b.name().toUpper().startsWith("DIRETO XR")) ||
         (b.name().toUpper().startsWith("SMB1")) || (b.name().toUpper().startsWith("INRIDE"))) &&
            !ftmsBike && !snodeBike && !fitPlusBike && !stagesBike && filter) {
        discoveryAgent->stop();
        ftmsBike = new ftmsbike(noWriteResistance, noHeartService, bikeResistanceOffset, bikeResistanceGain);
        emit deviceConnected(b);
        connect(ftmsBike, &bluetoothdevice::connectedAndDiscovered, this, &bluetooth::connectedAndDiscovered);
        // connect(trxappgateusb, SIGNAL(disconnected()), this, SLOT(restart()));
        connect(ftmsBike, &ftmsbike::debug, this, &bluetooth::debug);
        ftmsBike->deviceDiscovered(b);
        return ftmsBike;

    }

    return nullptr;
}

bluetoothdevice * bluetooth::detect_pafersbike(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings) {
    auto pafersBike = dynamic_cast<pafersbike*>(this->device());

    bool pafers_treadmill = settings.value(QStringLiteral("pafers_treadmill"), false).toBool();

    if ((b.name().toUpper().startsWith("PAFERS_")) && !pafersBike && !pafers_treadmill && filter) {
        discoveryAgent->stop();
        pafersBike =
                new pafersbike(noWriteResistance, noHeartService, bikeResistanceOffset, bikeResistanceGain);
        emit(deviceConnected(b));
        connect(pafersBike, SIGNAL(connectedAndDiscovered()), this, SLOT(connectedAndDiscovered()));
        // connect(pafersBike, SIGNAL(disconnected()), this, SLOT(restart()));
        connect(pafersBike, SIGNAL(debug(QString)), this, SLOT(debug(QString)));
        pafersBike->deviceDiscovered(b);
        return pafersBike;
    }

    return nullptr;
}

bluetoothdevice * bluetooth::detect_paferstreadmill(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings) {
    auto pafersTreadmill = dynamic_cast<paferstreadmill*>(this->device());
    bool pafers_treadmill = settings.value(QStringLiteral("pafers_treadmill"), false).toBool();
    if (b.name().toUpper().startsWith(QStringLiteral("PAFERS_")) && !pafersTreadmill &&
            pafers_treadmill && filter) {
        discoveryAgent->stop();
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
        return pafersTreadmill;
    }

    return nullptr;
}

bluetoothdevice * bluetooth::detect_tacxneo2(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings) {
    auto tacxneo2Bike = dynamic_cast<tacxneo2*>(this->device());

    if ((b.name().toUpper().startsWith("TACX NEO") ||
         (b.name().toUpper().startsWith("TACX SMART BIKE"))) &&
            !tacxneo2Bike && filter) {
        discoveryAgent->stop();
        tacxneo2Bike = new tacxneo2(noWriteResistance, noHeartService);
        // stateFileRead();
        emit(deviceConnected(b));
        connect(tacxneo2Bike, SIGNAL(connectedAndDiscovered()), this, SLOT(connectedAndDiscovered()));
        // connect(tacxneo2Bike, SIGNAL(disconnected()), this, SLOT(restart()));
        connect(tacxneo2Bike, SIGNAL(debug(QString)), this, SLOT(debug(QString)));
        // connect(tacxneo2Bike, SIGNAL(speedChanged(double)), this, SLOT(speedChanged(double)));
        // connect(tacxneo2Bike, SIGNAL(inclinationChanged(double)), this, SLOT(inclinationChanged(double)));
        tacxneo2Bike->deviceDiscovered(b);
        return tacxneo2Bike;
    }

    return nullptr;
}

bluetoothdevice * bluetooth::detect_renphobike(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings) {
    auto renphoBike = dynamic_cast<renphobike*>(this->device());
    auto snodeBike = dynamic_cast<snodebike*>(this->device());
    auto fitPlusBike = dynamic_cast<fitplusbike*>(this->device());

    bool toorx_ftms = settings.value(QStringLiteral("toorx_ftms"), false).toBool();

    if (((b.name().toUpper().startsWith("RQ") && b.name().length() == 5) ||
         (b.name().toUpper().startsWith("SCH130")) || // not a renpho bike an FTMS one
         ((b.name().startsWith(QStringLiteral("TOORX"))) && toorx_ftms)) &&
            !renphoBike && !snodeBike && !fitPlusBike && filter) {
        discoveryAgent->stop();
        renphoBike = new renphobike(noWriteResistance, noHeartService);
        emit(deviceConnected(b));
        connect(renphoBike, SIGNAL(connectedAndDiscovered()), this, SLOT(connectedAndDiscovered()));
        // connect(trxappgateusb, SIGNAL(disconnected()), this, SLOT(restart()));
        connect(renphoBike, SIGNAL(debug(QString)), this, SLOT(debug(QString)));
        renphoBike->deviceDiscovered(b);
        userTemplateManager->start(renphoBike);
        innerTemplateManager->start(renphoBike);
    }
    return nullptr;
}

bluetoothdevice * bluetooth::detect_shuaa5treadmill(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings) {
    auto shuaA5Treadmill = dynamic_cast<shuaa5treadmill*>(this->device());

    if ((b.name().toUpper().startsWith(QStringLiteral("ZW-"))) && !shuaA5Treadmill && filter) {
        settings.setValue(QStringLiteral("bluetooth_lastdevice_name"), b.name());
#ifndef Q_OS_IOS
        settings.setValue(QStringLiteral("bluetooth_lastdevice_address"), b.address().toString());
#else
        settings.setValue("bluetooth_lastdevice_address", b.deviceUuid().toString());
#endif

        discoveryAgent->stop();
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
        return shuaA5Treadmill;
    }
    return nullptr;
}

heartratebelt * bluetooth::detect_heartratebelt(const QBluetoothDeviceInfo& b, QSettings& settings) {
    auto heartRateBelt = dynamic_cast<heartratebelt*>(this->device());

    QString heartRateBeltName = settings.value("heart_rate_belt_name", "Disabled").toString();

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


        return heartRateBelt;
    }

    return nullptr;
}

smartspin2k * bluetooth::detect_smartspin2k(const QBluetoothDeviceInfo& b, QSettings& settings) {
    auto ftmsAccessory = dynamic_cast<smartspin2k*>(this->device());

    QString ftmsAccessoryName =
            settings.value(QStringLiteral("ftms_accessory_name"), QStringLiteral("Disabled")).toString();

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

        connect(this->device(), SIGNAL(resistanceChanged(int8_t)), ftmsAccessory,
                SLOT(changeResistance(int8_t)));
        connect(this->device(), SIGNAL(resistanceRead(int8_t)), ftmsAccessory,
                SLOT(resistanceReadFromTheBike(int8_t)));
        connect(ftmsAccessory, SIGNAL(resistanceRead(int8_t)), this->device(),
                SLOT(resistanceFromFTMSAccessory(int8_t)));
        return ftmsAccessory;
    }
    return nullptr;
}

bluetoothdevice * bluetooth::detect_powerTreadmill(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings) {
    auto powerTreadmill = dynamic_cast<strydrunpowersensor*>(this->device());
    QString powerSensorName =
            settings.value(QStringLiteral("power_sensor_name"), QStringLiteral("Disabled")).toString();

    bool power_as_treadmill = settings.value(QStringLiteral("power_sensor_as_treadmill"), false).toBool();
    if (power_as_treadmill && b.name().startsWith(powerSensorName) && !powerTreadmill && filter) {

        discoveryAgent->stop();
        powerTreadmill = new strydrunpowersensor(noWriteResistance, noHeartService, false);
        emit deviceConnected(b);
        connect(powerTreadmill, &bluetoothdevice::connectedAndDiscovered, this,
                &bluetooth::connectedAndDiscovered);
        // connect(cscBike, SIGNAL(disconnected()), this, SLOT(restart()));
        connect(powerTreadmill, &strydrunpowersensor::debug, this, &bluetooth::debug);
        powerTreadmill->deviceDiscovered(b);
        // connect(this, SIGNAL(searchingStop()), cscBike, SLOT(searchingStop())); //NOTE: Commented due to #358
        return powerTreadmill;
    }
    return nullptr;
}

bluetoothdevice * bluetooth::detect_ultrasportbike(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings) {
    auto ultraSportBike = dynamic_cast<ultrasportbike*>(this->device());

    if ((b.name().toUpper().startsWith(QStringLiteral("X-BIKE"))) && !ultraSportBike && filter) {
        discoveryAgent->stop();
        ultraSportBike =
                new ultrasportbike(noWriteResistance, noHeartService, bikeResistanceOffset, bikeResistanceGain);
        emit deviceConnected(b);
        connect(ultraSportBike, &bluetoothdevice::connectedAndDiscovered, this,
                &bluetooth::connectedAndDiscovered);
        // connect(ultraSportBike, SIGNAL(disconnected()), this, SLOT(restart()));
        // connect(ultraSportBike, &solebike::debug, this, &bluetooth::debug);
        ultraSportBike->deviceDiscovered(b);
        return ultraSportBike;
    }

    return nullptr;
}

bluetoothdevice * bluetooth::detect_wahookickrsnapbike(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings) {
    auto wahooKickrSnapBike = dynamic_cast<wahookickrsnapbike*>(this->device());

    if ((b.name().toUpper().startsWith("KICKR SNAP") || b.name().toUpper().startsWith("KICKR BIKE")) &&
            !wahooKickrSnapBike && filter) {
        discoveryAgent->stop();
        wahooKickrSnapBike =
                new wahookickrsnapbike(noWriteResistance, noHeartService, bikeResistanceOffset, bikeResistanceGain);
        emit deviceConnected(b);
        connect(wahooKickrSnapBike, &bluetoothdevice::connectedAndDiscovered, this,
                &bluetooth::connectedAndDiscovered);
        // connect(wahooKickrSnapBike, SIGNAL(disconnected()), this, SLOT(restart()));
        connect(wahooKickrSnapBike, &wahookickrsnapbike::debug, this, &bluetooth::debug);
        return wahooKickrSnapBike;
    }

    return nullptr;
}

eliterizer * bluetooth::detect_eliterizer(const QBluetoothDeviceInfo& b,  QSettings& settings) {
    auto eliteRizer = dynamic_cast<eliterizer*>(this->device());

    QString eliteRizerName = settings.value(QStringLiteral("elite_rizer_name"), QStringLiteral("Disabled")).toString();

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
              return eliteRizer;
            }

    return nullptr;
}

elitesterzosmart * bluetooth::detect_elitesterzosmart(const QBluetoothDeviceInfo& b,  QSettings& settings) {
    auto eliteSterzoSmart = dynamic_cast<elitesterzosmart*>(this->device());

    QString eliteSterzoSmartName =
            settings.value(QStringLiteral("elite_sterzo_smart_name"), QStringLiteral("Disabled")).toString();

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
        return eliteSterzoSmart;
    }


    return nullptr;
}


bluetoothdevice * bluetooth::detect_powerBike(const QBluetoothDeviceInfo& b, bool filter, QSettings& settings) {
    auto powerBike = dynamic_cast<stagesbike*>(this->device());
    bool power_as_bike = settings.value(QStringLiteral("power_sensor_as_bike"), false).toBool();
    QString powerSensorName = settings.value(QStringLiteral("power_sensor_name"), QStringLiteral("Disabled")).toString();
    if (power_as_bike && b.name().startsWith(powerSensorName) && !powerBike && filter) {

        discoveryAgent->stop();
        powerBike = new stagesbike(noWriteResistance, noHeartService, false);
        emit deviceConnected(b);
        connect(powerBike, &bluetoothdevice::connectedAndDiscovered, this, &bluetooth::connectedAndDiscovered);
        // connect(cscBike, SIGNAL(disconnected()), this, SLOT(restart()));
        connect(powerBike, &stagesbike::debug, this, &bluetooth::debug);
        powerBike->deviceDiscovered(b);
        // connect(this, SIGNAL(searchingStop()), cscBike, SLOT(searchingStop())); //NOTE: Commented due to #358
        return powerBike;

    }
    return nullptr;
}

cscbike * bluetooth::detect_cadenceSensor(const QBluetoothDeviceInfo& b, QSettings& settings) {

    QString cscName = settings.value(QStringLiteral("cadence_sensor_name"), QStringLiteral("Disabled")).toString();

    if (((b.name().startsWith(cscName))) &&
            !cscName.startsWith(QStringLiteral("Disabled"))) {
        settings.setValue(QStringLiteral("csc_sensor_lastdevice_name"), b.name());

#ifndef Q_OS_IOS
        settings.setValue(QStringLiteral("csc_sensor_address"), b.address().toString());
#else
        settings.setValue("csc_sensor_address", b.deviceUuid().toString());
#endif
        auto cadenceSensor = new cscbike(false, false, true);
        // connect(heartRateBelt, SIGNAL(disconnected()), this, SLOT(restart()));

        connect(cadenceSensor, &cscbike::debug, this, &bluetooth::debug);
        connect(cadenceSensor, &bluetoothdevice::cadenceChanged, this->device(),
                &bluetoothdevice::cadenceSensor);
        return cadenceSensor;
    }
    return nullptr;
}

bluetoothdevice *bluetooth::detect_powerSensor(const QBluetoothDeviceInfo &b, QSettings &settings)
{

    QString powerSensorName = settings.value(QStringLiteral("power_sensor_name"), QStringLiteral("Disabled")).toString();

    if (((b.name().startsWith(powerSensorName))) &&
            !powerSensorName.startsWith(QStringLiteral("Disabled"))) {
        settings.setValue(QStringLiteral("power_sensor_lastdevice_name"), b.name());

#ifndef Q_OS_IOS
        settings.setValue(QStringLiteral("power_sensor_address"), b.address().toString());
#else
        settings.setValue("power_sensor_address", b.deviceUuid().toString());
#endif
        if (device() && device()->deviceType() == bluetoothdevice::BIKE) {
            auto result = new stagesbike(false, false, true);
            // connect(heartRateBelt, SIGNAL(disconnected()), this, SLOT(restart()));

            connect(result, &stagesbike::debug, this, &bluetooth::debug);
            connect(result, &bluetoothdevice::powerChanged, this->device(), &bluetoothdevice::powerSensor);
            result->deviceDiscovered(b);
            return result;
        } else if (device() && device()->deviceType() == bluetoothdevice::TREADMILL) {
            auto result = new strydrunpowersensor(false, false, true);
            // connect(heartRateBelt, SIGNAL(disconnected()), this, SLOT(restart()));

            connect(result, &strydrunpowersensor::debug, this, &bluetooth::debug);
            connect(result, &bluetoothdevice::powerChanged, this->device(),
                    &bluetoothdevice::powerSensor);
            connect(result, &bluetoothdevice::cadenceChanged, this->device(),
                    &bluetoothdevice::cadenceSensor);
            connect(result, &bluetoothdevice::speedChanged, this->device(),
                    &bluetoothdevice::speedSensor);
            connect(result, &bluetoothdevice::instantaneousStrideLengthChanged, this->device(),
                    &bluetoothdevice::instantaneousStrideLengthSensor);
            connect(result, &bluetoothdevice::groundContactChanged, this->device(),
                    &bluetoothdevice::groundContactSensor);
            connect(result, &bluetoothdevice::verticalOscillationChanged, this->device(),
                    &bluetoothdevice::verticalOscillationSensor);
            result->deviceDiscovered(b);
            return result;
        }

    }
    return nullptr;
}

bool bluetooth::toorxBikeAvailable(const QSettings& settings)
{
    bool toorx_ftms = settings.value(QStringLiteral("toorx_ftms"), false).toBool();
    bool toorx_bike = (settings.value(QStringLiteral("toorx_bike"), false).toBool() ||
                       settings.value(QStringLiteral("jll_IC400_bike"), false).toBool() ||
                       settings.value(QStringLiteral("fytter_ri08_bike"), false).toBool() ||
                       settings.value(QStringLiteral("asviva_bike"), false).toBool() ||
                       settings.value(QStringLiteral("hertz_xr_770"), false).toBool()) &&
            !toorx_ftms;

    return toorx_bike;
}


void bluetooth::deviceDiscovered(const QBluetoothDeviceInfo &device) {

    QSettings settings;
    QString heartRateBeltName =
            settings.value(QStringLiteral("heart_rate_belt_name"), QStringLiteral("Disabled")).toString();
    QString ftmsAccessoryName =
            settings.value(QStringLiteral("ftms_accessory_name"), QStringLiteral("Disabled")).toString();
    bool heartRateBeltFound = heartRateBeltName.startsWith(QStringLiteral("Disabled"));
    bool ftmsAccessoryFound = ftmsAccessoryName.startsWith(QStringLiteral("Disabled"));

    //    bool snode_bike = settings.value(QStringLiteral("snode_bike"), false).toBool();
    //    bool fitplus_bike = settings.value(QStringLiteral("fitplus_bike"), false).toBool() ||
    settings.value(QStringLiteral("virtufit_etappe"), false).toBool();


    bool csc_as_bike = settings.value(QStringLiteral("cadence_sensor_as_bike"), false).toBool();
    QString cscName = settings.value(QStringLiteral("cadence_sensor_name"), QStringLiteral("Disabled")).toString();
    bool cscFound = cscName.startsWith(QStringLiteral("Disabled")) || csc_as_bike;

    //bool flywheel_life_fitness_ic8 = settings.value(QStringLiteral("flywheel_life_fitness_ic8"), false).toBool();


    QString eliteSterzoSmartName =
            settings.value(QStringLiteral("elite_sterzo_smart_name"), QStringLiteral("Disabled")).toString();

    bool power_as_bike = settings.value(QStringLiteral("power_sensor_as_bike"), false).toBool();
    QString powerSensorName =
            settings.value(QStringLiteral("power_sensor_name"), QStringLiteral("Disabled")).toString();

    bool power_as_treadmill = settings.value(QStringLiteral("power_sensor_as_treadmill"), false).toBool();
    bool powerSensorFound =
            powerSensorName.startsWith(QStringLiteral("Disabled")) || power_as_bike || power_as_treadmill;

    QString eliteRizerName = settings.value(QStringLiteral("elite_rizer_name"), QStringLiteral("Disabled")).toString();
    bool eliteRizerFound = eliteRizerName.startsWith(QStringLiteral("Disabled"));
    bool eliteSterzoSmartFound = eliteSterzoSmartName.startsWith(QStringLiteral("Disabled"));

    heartRateBeltFound |= heartRateBeltAvailable();
    ftmsAccessoryFound |= ftmsAccessoryAvailable();
    cscFound |= cscSensorAvailable();
    powerSensorFound |= powerSensorAvailable();
    eliteRizerFound |= eliteRizerAvailable();
    eliteSterzoSmartFound |= eliteSterzoSmartAvailable();


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

            bluetoothdevice* btd = nullptr;

            if(btd==nullptr) btd = this->detect_m3iBike(b, filter, settings);
            if(btd==nullptr) btd = this->detect_fakeBike(b, filter, settings);
            if(btd==nullptr) btd = this->detect_fakeElliptical(b, filter, settings);
            if(btd==nullptr) btd = this->detect_proformWifiBike(b, filter, settings);
            if(btd==nullptr) btd = this->detect_proformwifitreadmill(b, filter, settings);
            if(btd==nullptr) btd = this->detect_nordictrackifitadbtreadmill(b, filter, settings);
            if(btd==nullptr) btd = this->detect_nordictrackelliptical(b, filter, settings);
            if(btd==nullptr) btd = this->detect_cscbike(b, filter, settings);
            if(btd==nullptr) btd = this->detect_powerBike(b, filter, settings);
            if(btd==nullptr) btd = this->detect_powerTreadmill(b, filter, settings);
            if(btd==nullptr) btd = this->detect_domyosrower(b, filter, settings);
            if(btd==nullptr) btd = this->detect_domyosbike(b, filter, settings);
            if(btd==nullptr) btd = this->detect_domyoselliptical(b, filter, settings);
            if(btd==nullptr) btd = this->detect_nautiluselliptical(b, filter, settings);
            if(btd==nullptr) btd = this->detect_nautilusbike(b, filter, settings);
            if(btd==nullptr) btd = this->detect_proformelliptical(b, filter, settings);
            if(btd==nullptr) btd = this->detect_nordictrackelliptical(b, filter, settings);
            if(btd==nullptr) btd = this->detect_proformellipticaltrainer(b, filter, settings);
            if(btd==nullptr) btd = this->detect_proformrower(b, filter, settings);
            if(btd==nullptr) btd = this->detect_bhfitnesselliptical(b, filter, settings);
            if(btd==nullptr) btd = this->detect_soleelliptical(b, filter, settings);
            if(btd==nullptr) btd = this->detect_domyoselliptical(b, filter, settings);
            if(btd==nullptr) btd = this->detect_domyostreadmill(b, filter, settings);
            if(btd==nullptr) btd = this->detect_kingsmithr2treadmill(b, filter, settings);
            if(btd==nullptr) btd = this->detect_kingsmithr1protreadmill(b, filter, settings);
            if(btd==nullptr) btd = this->detect_shuaa5treadmill(b, filter, settings);
            if(btd==nullptr) btd = this->detect_truetreadmill(b, filter, settings);
            if(btd==nullptr) btd = this->detect_solef80treadmill(b, filter, settings);
            if(btd==nullptr) btd = this->detect_horizontreadmill(b, filter, settings);
            if(btd==nullptr) btd = this->detect_technogymmyruntreadmill(b, filter, settings);
            if(btd==nullptr) btd = this->detect_tacxneo2(b, filter, settings);
            if(btd==nullptr) btd = this->detect_npecablebike(b, filter, settings);
            if(btd==nullptr) btd = this->detect_ftmsbike(b, filter, settings);
            if(btd==nullptr) btd = this->detect_wahookickrsnapbike(b, filter, settings);
            if(btd==nullptr) btd = this->detect_horizongr7bike(b, filter, settings);
            if(btd==nullptr) btd = this->detect_stagesbike(b, filter, settings);
            if(btd==nullptr) btd = this->detect_smartrowrower(b, filter, settings);
            if(btd==nullptr) btd = this->detect_concept2skierg(b, filter, settings);
            if(btd==nullptr) btd = this->detect_ftmsrower(b, filter, settings);
            if(btd==nullptr) btd = this->detect_echelonstride(b, filter, settings);
            if(btd==nullptr) btd = this->detect_octanetreadmill(b, filter, settings);
            if(btd==nullptr) btd = this->detect_echelonrower(b, filter, settings);
            if(btd==nullptr) btd = this->detect_echelonconnectsport(b, filter, settings);
            if(btd==nullptr) btd = this->detect_schwinnic4bike(b, filter, settings);
            if(btd==nullptr) btd = this->detect_sportstechbike(b, filter, settings);
            if(btd==nullptr) btd = this->detect_sportsplusbike(b, filter, settings);
            if(btd==nullptr) btd = this->detect_yesoulbike(b, filter, settings);
            if(btd==nullptr) btd = this->detect_proformbike(b, filter, settings);
            if(btd==nullptr) btd = this->detect_proformtreadmill(b, filter, settings);
            if(btd==nullptr) btd = this->detect_eslinkertreadmill(b, filter, settings);
            if(btd==nullptr) btd = this->detect_paferstreadmill(b, filter, settings);
            if(btd==nullptr) btd = this->detect_bowflext216treadmill(b, filter, settings);
            if(btd==nullptr) btd = this->detect_nautilustreadmill(b, filter, settings);
            if(btd==nullptr) btd = this->detect_flywheelbike(b, filter, settings);
            if(btd==nullptr) btd = this->detect_mcfbike(b, filter, settings);
            if(btd==nullptr) btd = this->detect_toorxtreadmill(b, filter, settings);
            if(btd==nullptr) btd = this->detect_iconceptbike(b, filter, settings);
            if(btd==nullptr) btd = this->detect_spirittreadmill(b, filter, settings);
            if(btd==nullptr) btd = this->detect_activiotreadmill(b, filter, settings);
            if(btd==nullptr) btd = this->detect_trxappgateusbtreadmill(b, filter, settings);
            if(btd==nullptr) btd = this->detect_trxappgateusbbike(b, filter, settings);
            if(btd==nullptr) btd = this->detect_ultrasportbike(b, filter, settings);
            if(btd==nullptr) btd = this->detect_keepbike(b, filter, settings);
            if(btd==nullptr) btd = this->detect_solebike(b, filter, settings);
            if(btd==nullptr) btd = this->detect_skandikawiribike(b, filter, settings);
            if(btd==nullptr) btd = this->detect_renphobike(b, filter, settings);
            if(btd==nullptr) btd = this->detect_pafersbike(b, filter, settings);
            if(btd==nullptr) btd = this->detect_snodebike(b, filter, settings);
            if(btd==nullptr) btd = this->detect_fitplusbike(b, filter, settings);
            if(btd==nullptr) btd = this->detect_fitshowtreadmill(b, filter, settings);
            if(btd==nullptr) btd = this->detect_inspirebike(b, filter, settings);

            // Note that the former code for the chronobike did NOT add the object to the template managers!
            if(btd==nullptr) btd = this->detect_chronobike(b, filter, settings);

            if(btd) {
                if (!discoveryAgent->isActive())
                    emit searchingStop();
                userTemplateManager->start(btd);
                innerTemplateManager->start(btd);
            }
        }
    }
}

void bluetooth::connectedAndDiscovered() {

    static bool firstConnected = true;
    QSettings settings;
    QString heartRateBeltName =
            settings.value(QStringLiteral("heart_rate_belt_name"), QStringLiteral("Disabled")).toString();

    bool csc_as_bike = settings.value(QStringLiteral("cadence_sensor_as_bike"), false).toBool();

    bool power_as_bike = settings.value(QStringLiteral("power_sensor_as_bike"), false).toBool();
    bool power_as_treadmill = settings.value(QStringLiteral("power_sensor_as_treadmill"), false).toBool();
    QString powerSensorName =
            settings.value(QStringLiteral("power_sensor_name"), QStringLiteral("Disabled")).toString();
    QString eliteRizerName = settings.value(QStringLiteral("elite_rizer_name"), QStringLiteral("Disabled")).toString();

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

            // If there's no heart rate belt yet, see if this device is one.
            if(!this->heartRateBelt) {
                this->heartRateBelt = this->detect_heartratebelt(b, settings);
                if(this->heartRateBelt) this->heartRateBelt->deviceDiscovered(b);
            }

            // If there's no FTMS accessory (Smart Spin 2k) yet, see if this device is one.
            if(!this->ftmsAccessory) {
                this->ftmsAccessory = this->detect_smartspin2k(b, settings);
                if(this->ftmsAccessory){
                    emit ftmsAccessoryConnected(ftmsAccessory);
                    this->ftmsAccessory->deviceDiscovered(b);
                }
            }


            if(!csc_as_bike && !this->cadenceSensor) {
                this->cadenceSensor = this->detect_cadenceSensor(b, settings);
                this->cadenceSensor->deviceDiscovered(b);
            }

            if(!this->eliteRizer) {
                this->eliteRizer = this->detect_eliterizer(b, settings);
                if(this->eliteRizer) eliteRizer->deviceDiscovered(b);
            }

            if(!this->eliteSterzoSmart) {
                this->eliteSterzoSmart = this->detect_elitesterzosmart(b, settings);
                if(this->eliteSterzoSmart) eliteSterzoSmart->deviceDiscovered(b);
            }

            if (fitmetriaFanfitEnabled) {
                if (((b.name().startsWith("FITFAN-"))) && !fitmetria_fanfit_isconnected(b.name())) {
                    fitmetria_fanfit *f = new fitmetria_fanfit(this->device());

                    connect(f, &fitmetria_fanfit::debug, this, &bluetooth::debug);
                    connect(this->device(), SIGNAL(fanSpeedChanged(uint8_t)), f, SLOT(fanSpeedRequest(uint8_t)));

                    f->deviceDiscovered(b);
                    this->fitmetriaFanfit.append(f);

                }
            }

            if (!power_as_bike && !power_as_treadmill) {
                if(!this->powerSensor)
                    this->powerSensor = this->detect_powerSensor(b, settings);
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
}

void bluetooth::heartRate(uint8_t heart) { Q_UNUSED(heart) }





void bluetooth::restart() {

    QSettings settings;

    if (onlyDiscover) {

        onlyDiscover = false;
        discoveryAgent->start();
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

#define DELETE(X) if(X) { delete X; X=nullptr; }

    DELETE(this->bluetoothDevice);
    DELETE(this->heartRateBelt);
    DELETE(this->ftmsAccessory);
    DELETE(this->eliteRizer);
    DELETE(this->eliteSterzoSmart);
    DELETE(this->cadenceSensor);
    DELETE(this->powerSensor);

    if (fitmetriaFanfit.length()) {

        foreach (fitmetria_fanfit *f, fitmetriaFanfit) {
            DELETE(f);
        }
        fitmetriaFanfit.clear();
    }

    discoveryAgent->start();
}

bluetoothdevice *bluetooth::device() {
    return this->bluetoothDevice;
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
    foreach (fitmetria_fanfit *f, this->fitmetriaFanfit) {
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
