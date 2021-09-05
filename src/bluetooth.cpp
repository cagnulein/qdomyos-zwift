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
    bool trx_route_key = settings.value(QStringLiteral("trx_route_key"), false).toBool();

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
        connect(discoveryAgent, &QBluetoothDeviceDiscoveryAgent::finished, this, &bluetooth::finished);

        // Start a discovery
        discoveryAgent->setLowEnergyDiscoveryTimeout(10000);

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
            emit(deviceConnected());
            connect(schwinnIC4Bike, SIGNAL(connectedAndDiscovered()), this, SLOT(connectedAndDiscovered()));
            // connect(echelonConnectSport, SIGNAL(disconnected()), this, SLOT(restart()));
            connect(schwinnIC4Bike, SIGNAL(debug(QString)), this, SLOT(debug(QString)));
            // connect(echelonConnectSport, SIGNAL(speedChanged(double)), this, SLOT(speedChanged(double)));
            // connect(echelonConnectSport, SIGNAL(inclinationChanged(double)), this, SLOT(inclinationChanged(double)));
            QBluetoothDeviceInfo bt;
            bt.setDeviceUuid(QBluetoothUuid(settings.value("bluetooth_lastdevice_address", "").toString()));
            qDebug() << "UUID" << bt.deviceUuid();
            schwinnIC4Bike->deviceDiscovered(bt);
            userTemplateManager->start(schwinnIC4Bike);
            innerTemplateManager->start(schwinnIC4Bike);
            qDebug() << "connecting directly";
        }
#endif

        if (!trx_route_key)
            discoveryAgent->start(QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);
        else
            discoveryAgent->start(QBluetoothDeviceDiscoveryAgent::ClassicMethod |
                                  QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);
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

    QSettings settings;
    QString heartRateBeltName =
        settings.value(QStringLiteral("heart_rate_belt_name"), QStringLiteral("Disabled")).toString();
    QString ftmsAccessoryName =
        settings.value(QStringLiteral("ftms_accessory_name"), QStringLiteral("Disabled")).toString();
    bool csc_as_bike = settings.value(QStringLiteral("cadence_sensor_as_bike"), false).toBool();
    QString cscName = settings.value(QStringLiteral("cadence_sensor_name"), QStringLiteral("Disabled")).toString();
    QString powerSensorName =
        settings.value(QStringLiteral("power_sensor_name"), QStringLiteral("Disabled")).toString();
    QString eliteRizerName = settings.value(QStringLiteral("elite_rizer_name"), QStringLiteral("Disabled")).toString();
    bool cscFound = cscName.startsWith(QStringLiteral("Disabled")) && !csc_as_bike;
    bool powerSensorFound = powerSensorName.startsWith(QStringLiteral("Disabled"));
    bool eliteRizerFound = eliteRizerName.startsWith(QStringLiteral("Disabled"));
    bool trx_route_key = settings.value(QStringLiteral("trx_route_key"), false).toBool();
    bool heartRateBeltFound = heartRateBeltName.startsWith(QStringLiteral("Disabled"));
    bool ftmsAccessoryFound = ftmsAccessoryName.startsWith(QStringLiteral("Disabled"));

    if ((!heartRateBeltFound && !heartRateBeltAvaiable()) || (!ftmsAccessoryFound && !ftmsAccessoryAvaiable()) ||
        (!cscFound && !cscSensorAvaiable()) || (!powerSensorFound && !powerSensorAvaiable()) ||
        (!eliteRizerFound && !eliteRizerAvaiable())) {

        // force heartRateBelt off
        forceHeartBeltOffForTimeout = true;
    }

    if (!trx_route_key) {
        discoveryAgent->start(QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);
    } else {
        discoveryAgent->start(QBluetoothDeviceDiscoveryAgent::ClassicMethod |
                              QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);
    }
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
    QString powerSensorName =
        settings.value(QStringLiteral("power_sensor_name"), QStringLiteral("Disabled")).toString();

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
    bool toorx_bike = settings.value(QStringLiteral("toorx_bike"), false).toBool();
    bool snode_bike = settings.value(QStringLiteral("snode_bike"), false).toBool();
    bool fitplus_bike = settings.value(QStringLiteral("fitplus_bike"), false).toBool();
    bool JLL_IC400_bike = settings.value(QStringLiteral("jll_IC400_bike"), false).toBool();
    bool csc_as_bike = settings.value(QStringLiteral("cadence_sensor_as_bike"), false).toBool();
    QString cscName = settings.value(QStringLiteral("cadence_sensor_name"), QStringLiteral("Disabled")).toString();
    bool cscFound = cscName.startsWith(QStringLiteral("Disabled")) || csc_as_bike;
    bool hammerRacerS = settings.value(QStringLiteral("hammer_racer_s"), false).toBool();
    bool flywheel_life_fitness_ic8 = settings.value(QStringLiteral("flywheel_life_fitness_ic8"), false).toBool();
    QString powerSensorName =
        settings.value(QStringLiteral("power_sensor_name"), QStringLiteral("Disabled")).toString();
    QString eliteRizerName = settings.value(QStringLiteral("elite_rizer_name"), QStringLiteral("Disabled")).toString();
    bool powerSensorFound = powerSensorName.startsWith(QStringLiteral("Disabled"));
    bool eliteRizerFound = eliteRizerName.startsWith(QStringLiteral("Disabled"));

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

    bool found = false;
    QMutableListIterator<QBluetoothDeviceInfo> i(devices);
    while (i.hasNext()) {
        QBluetoothDeviceInfo b = i.next();
        if (SAME_BLUETOOTH_DEVICE(b, device) && !b.name().isEmpty()) {

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

    if ((heartRateBeltFound && ftmsAccessoryFound && cscFound && powerSensorFound && eliteRizerFound) ||
        forceHeartBeltOffForTimeout) {
        for (const QBluetoothDeviceInfo &b : qAsConst(devices)) {

            bool filter = true;
            if (!filterDevice.isEmpty() && !filterDevice.startsWith(QStringLiteral("Disabled"))) {

                filter = (b.name().compare(filterDevice, Qt::CaseInsensitive) == 0);
            }
            if (b.name().startsWith(QStringLiteral("M3")) && !m3iBike && filter) {

                if (m3ibike::isCorrectUnit(b)) {
                    discoveryAgent->stop();
                    m3iBike = new m3ibike(noWriteResistance, noHeartService);
                    emit deviceConnected();
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
            } else if (csc_as_bike && b.name().startsWith(cscName) && !cscBike && filter) {

                discoveryAgent->stop();
                cscBike = new cscbike(noWriteResistance, noHeartService, false);
                emit deviceConnected();
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
            } else if (b.name().startsWith(QStringLiteral("Domyos-Bike")) &&
                       !b.name().startsWith(QStringLiteral("DomyosBridge")) && !domyosBike && filter) {
                discoveryAgent->stop();
                domyosBike = new domyosbike(noWriteResistance, noHeartService, testResistance, bikeResistanceOffset,
                                            bikeResistanceGain);
                emit deviceConnected();
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
                discoveryAgent->stop();
                domyosElliptical = new domyoselliptical(noWriteResistance, noHeartService, testResistance,
                                                        bikeResistanceOffset, bikeResistanceGain);
                emit deviceConnected();
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
            } else if (b.name().toUpper().startsWith(QStringLiteral("E95S")) && !soleElliptical && filter) {
                discoveryAgent->stop();
                soleElliptical = new soleelliptical(noWriteResistance, noHeartService, testResistance,
                                                    bikeResistanceOffset, bikeResistanceGain);
                emit deviceConnected();
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
                       !domyosBike && filter) {
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
                emit deviceConnected();
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
            } else if ((b.name().toUpper().startsWith(QStringLiteral("KS-R1AC")) ||
                        b.name().toUpper().startsWith(QStringLiteral("KS-HC-R1AA")) ||
                        b.name().toUpper().startsWith(QStringLiteral("KS-HC-R1AC"))) &&
                       !kingsmithR2Treadmill &&
                       filter) {
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
                emit deviceConnected();
                connect(kingsmithR2Treadmill, &bluetoothdevice::connectedAndDiscovered, this,
                        &bluetooth::connectedAndDiscovered);
                // connect(kingsmithR2Treadmill, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(kingsmithR2Treadmill, &kingsmithr2treadmill::debug, this, &bluetooth::debug);
                connect(kingsmithR2Treadmill, &kingsmithr2treadmill::speedChanged, this,
                        &bluetooth::speedChanged);
                connect(kingsmithR2Treadmill, &kingsmithr2treadmill::inclinationChanged, this,
                        &bluetooth::inclinationChanged);
                kingsmithR2Treadmill->deviceDiscovered(b);
                connect(this, &bluetooth::searchingStop, kingsmithR2Treadmill,
                        &kingsmithr2treadmill::searchingStop);
                if (!discoveryAgent->isActive())
                    emit searchingStop();
                userTemplateManager->start(kingsmithR2Treadmill);
                innerTemplateManager->start(kingsmithR2Treadmill);
            } else if ((b.name().toUpper().startsWith(QStringLiteral("R1 PRO")) ||
                        !b.name().toUpper().compare(QStringLiteral("RE")) || // just "RE"
                        b.name().toUpper().startsWith(
                            QStringLiteral("KS-"))) && // Treadmill KingSmith WalkingPad R2 Pro KS-HCR1AA
                       !kingsmithR1ProTreadmill &&
                       filter) {
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
                emit deviceConnected();
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
            } else if ((b.name().toUpper().startsWith(QStringLiteral("F80"))) && !soleF80 && filter) {
                discoveryAgent->stop();
                soleF80 = new solef80(noWriteResistance, noHeartService);
#if !defined(Q_OS_ANDROID) && !defined(Q_OS_IOS)
                stateFileRead();
#endif
                emit deviceConnected();
                connect(soleF80, &bluetoothdevice::connectedAndDiscovered, this, &bluetooth::connectedAndDiscovered);
                // connect(soleF80, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(soleF80, &solef80::debug, this, &bluetooth::debug);
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
                        b.name().toUpper().startsWith(QStringLiteral("ESANGLINKER"))) &&
                       !horizonTreadmill && filter) {
                discoveryAgent->stop();
                horizonTreadmill = new horizontreadmill(noWriteResistance, noHeartService);
#if !defined(Q_OS_ANDROID) && !defined(Q_OS_IOS)
                stateFileRead();
#endif
                emit deviceConnected();
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
            } else if (b.name().toUpper().startsWith("TACX NEO 2") && !tacxneo2Bike && filter) {
                discoveryAgent->stop();
                tacxneo2Bike = new tacxneo2(noWriteResistance, noHeartService);
                // stateFileRead();
                emit(deviceConnected());
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
                        (b.name().toUpper().startsWith(QStringLiteral("BIKE 1")) &&
                         flywheel_life_fitness_ic8 == false)) &&
                       !npeCableBike && filter) {
                discoveryAgent->stop();
                npeCableBike = new npecablebike(noWriteResistance, noHeartService);
                // stateFileRead();
                emit deviceConnected();
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
                        (b.name().toUpper().startsWith("WAHOO KICKR")) || (b.name().toUpper().startsWith("B94")) ||
                        (b.name().toUpper().startsWith("STAGES BIKE")) || (b.name().toUpper().startsWith("SMB1"))) &&
                       !ftmsBike && !snodeBike && !fitPlusBike && !stagesBike && filter) {
                discoveryAgent->stop();
                ftmsBike = new ftmsbike(noWriteResistance, noHeartService, bikeResistanceOffset, bikeResistanceGain);
                emit deviceConnected();
                connect(ftmsBike, &bluetoothdevice::connectedAndDiscovered, this, &bluetooth::connectedAndDiscovered);
                // connect(trxappgateusb, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(ftmsBike, &ftmsbike::debug, this, &bluetooth::debug);
                ftmsBike->deviceDiscovered(b);
                userTemplateManager->start(ftmsBike);
                innerTemplateManager->start(ftmsBike);
            } else if ((b.name().toUpper().startsWith(QStringLiteral("STAGES ")) ||
                        b.name().toUpper().startsWith(QStringLiteral("ASSIOMA"))) &&
                       !stagesBike && !ftmsBike && filter) {

                discoveryAgent->stop();
                stagesBike = new stagesbike(noWriteResistance, noHeartService, false);
                // stateFileRead();
                emit deviceConnected();
                connect(stagesBike, &bluetoothdevice::connectedAndDiscovered, this, &bluetooth::connectedAndDiscovered);
                // connect(stagesBike, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(stagesBike, &stagesbike::debug, this, &bluetooth::debug);
                // connect(stagesBike, SIGNAL(speedChanged(double)), this, SLOT(speedChanged(double)));
                // connect(stagesBike, SIGNAL(inclinationChanged(double)), this, SLOT(inclinationChanged(double)));
                stagesBike->deviceDiscovered(b);
                userTemplateManager->start(stagesBike);
                innerTemplateManager->start(stagesBike);
            } else if (b.name().startsWith(QStringLiteral("SMARTROW")) && !smartrowRower && filter) {

                discoveryAgent->stop();
                smartrowRower =
                    new smartrowrower(noWriteResistance, noHeartService, bikeResistanceOffset, bikeResistanceGain);
                // stateFileRead();
                emit deviceConnected();
                connect(smartrowRower, &bluetoothdevice::connectedAndDiscovered, this,
                        &bluetooth::connectedAndDiscovered);
                // connect(smartrowRower, SIGNAL(disconnected()), this, SLOT(restart()));
                // connect(smartrowRower, SIGNAL(debug(QString)), this, SLOT(debug(QString)));
                // connect(v, SIGNAL(speedChanged(double)), this, SLOT(speedChanged(double)));
                // connect(smartrowRower, SIGNAL(inclinationChanged(double)), this, SLOT(inclinationChanged(double)));
                smartrowRower->deviceDiscovered(b);
                userTemplateManager->start(smartrowRower);
                innerTemplateManager->start(smartrowRower);
            } else if ((b.name().toUpper().startsWith(QStringLiteral("CR 00")) ||
                        (b.name().toUpper().startsWith(QStringLiteral("PM5")) &&
                         b.name().toUpper().contains(QStringLiteral("ROW")))) &&
                       !ftmsRower && filter) {
                discoveryAgent->stop();
                ftmsRower = new ftmsrower(noWriteResistance, noHeartService);
                // stateFileRead();
                emit deviceConnected();
                connect(ftmsRower, &bluetoothdevice::connectedAndDiscovered, this, &bluetooth::connectedAndDiscovered);
                // connect(ftmsRower, SIGNAL(disconnected()), this, SLOT(restart()));
                // connect(ftmsRower, SIGNAL(debug(QString)), this, SLOT(debug(QString)));
                // connect(v, SIGNAL(speedChanged(double)), this, SLOT(speedChanged(double)));
                // connect(ftmsRower, SIGNAL(inclinationChanged(double)), this, SLOT(inclinationChanged(double)));
                ftmsRower->deviceDiscovered(b);
                userTemplateManager->start(ftmsRower);
                innerTemplateManager->start(ftmsRower);
            } else if (b.name().startsWith(QLatin1String("ECH-STRIDE")) && !echelonStride && filter) {

                discoveryAgent->stop();
                echelonStride = new echelonstride(this->pollDeviceTime, noConsole, noHeartService);
                // stateFileRead();
                emit deviceConnected();
                connect(echelonStride, &bluetoothdevice::connectedAndDiscovered, this,
                        &bluetooth::connectedAndDiscovered);
                // connect(echelonRower, SIGNAL(disconnected()), this, SLOT(restart())); connect(echelonStride,
                // SIGNAL(debug(QString)), this, SLOT(debug(QString))); connect(echelonRower,
                // SIGNAL(speedChanged(double)), this, SLOT(speedChanged(double))); connect(echelonRower,
                // SIGNAL(inclinationChanged(double)), this, SLOT(inclinationChanged(double)));
                echelonStride->deviceDiscovered(b);
                userTemplateManager->start(echelonStride);
                innerTemplateManager->start(echelonStride);
            } else if (b.name().startsWith(QStringLiteral("ECH-ROW")) && !echelonRower && filter) {

                discoveryAgent->stop();
                echelonRower =
                    new echelonrower(noWriteResistance, noHeartService, bikeResistanceOffset, bikeResistanceGain);
                // stateFileRead();
                emit deviceConnected();
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

                discoveryAgent->stop();
                echelonConnectSport = new echelonconnectsport(noWriteResistance, noHeartService, bikeResistanceOffset,
                                                              bikeResistanceGain);
                // stateFileRead();
                emit deviceConnected();
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
                        b.name().toUpper().startsWith(QStringLiteral("C7-"))) &&
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
                emit deviceConnected();
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

                discoveryAgent->stop();
                sportsTechBike = new sportstechbike(noWriteResistance, noHeartService);
                // stateFileRead();
                emit deviceConnected();
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
            } else if (b.name().startsWith(QStringLiteral("YESOUL")) && !yesoulBike && filter) {

                discoveryAgent->stop();
                yesoulBike = new yesoulbike(noWriteResistance, noHeartService);
                // stateFileRead();
                emit deviceConnected();
                connect(yesoulBike, &bluetoothdevice::connectedAndDiscovered, this, &bluetooth::connectedAndDiscovered);
                // connect(yesoulBike, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(yesoulBike, &yesoulbike::debug, this, &bluetooth::debug);
                // connect(echelonConnectSport, SIGNAL(speedChanged(double)), this, SLOT(speedChanged(double)));
                // connect(echelonConnectSport, SIGNAL(inclinationChanged(double)), this,
                // SLOT(inclinationChanged(double)));
                yesoulBike->deviceDiscovered(b);
                userTemplateManager->start(yesoulBike);
                innerTemplateManager->start(yesoulBike);
            } else if (b.name().startsWith(QStringLiteral("I_EB")) && !proformBike && filter) {

                discoveryAgent->stop();
                proformBike =
                    new proformbike(noWriteResistance, noHeartService, bikeResistanceOffset, bikeResistanceGain);
                // stateFileRead();
                emit deviceConnected();
                connect(proformBike, &bluetoothdevice::connectedAndDiscovered, this,
                        &bluetooth::connectedAndDiscovered);
                // connect(proformBike, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(proformBike, &proformbike::debug, this, &bluetooth::debug);
                // connect(proformBike, SIGNAL(speedChanged(double)), this, SLOT(speedChanged(double)));
                // connect(proformBike, SIGNAL(inclinationChanged(double)), this, SLOT(inclinationChanged(double)));
                proformBike->deviceDiscovered(b);
                userTemplateManager->start(proformBike);
                innerTemplateManager->start(proformBike);
            } else if (b.name().startsWith(QStringLiteral("I_TL")) && !proformTreadmill && filter) {

                discoveryAgent->stop();
                proformTreadmill = new proformtreadmill(noWriteResistance, noHeartService);
                // stateFileRead();
                emit deviceConnected();
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

                discoveryAgent->stop();
                eslinkerTreadmill = new eslinkertreadmill(noWriteResistance, noHeartService);
                // stateFileRead();
                emit deviceConnected();
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
            } else if ((b.name().startsWith(QStringLiteral("Flywheel")) ||
                        (b.name().toUpper().startsWith(QStringLiteral("BIKE 1")) &&
                         flywheel_life_fitness_ic8 == true)) &&
                       !flywheelBike && filter) {

                discoveryAgent->stop();
                flywheelBike = new flywheelbike(noWriteResistance, noHeartService);
                // stateFileRead();
                emit deviceConnected();
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
            } else if ((b.name().startsWith(QStringLiteral("TRX ROUTE KEY"))) && !toorx && filter) {

                discoveryAgent->stop();
                toorx = new toorxtreadmill();
                emit deviceConnected();
                connect(toorx, &bluetoothdevice::connectedAndDiscovered, this, &bluetooth::connectedAndDiscovered);
                // connect(toorx, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(toorx, &toorxtreadmill::debug, this, &bluetooth::debug);
                toorx->deviceDiscovered(b);
                userTemplateManager->start(toorx);
                innerTemplateManager->start(toorx);
            } else if (b.name().toUpper().startsWith(QStringLiteral("XT485")) && !spiritTreadmill && filter) {

                discoveryAgent->stop();
                spiritTreadmill = new spirittreadmill();
                emit deviceConnected();
                connect(spiritTreadmill, &bluetoothdevice::connectedAndDiscovered, this,
                        &bluetooth::connectedAndDiscovered);
                // connect(spiritTreadmill, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(spiritTreadmill, &spirittreadmill::debug, this, &bluetooth::debug);
                spiritTreadmill->deviceDiscovered(b);
                userTemplateManager->start(spiritTreadmill);
                innerTemplateManager->start(spiritTreadmill);
            } else if (((b.name().startsWith(QStringLiteral("TOORX"))) ||
                        (b.name().startsWith(QStringLiteral("V-RUN"))) ||
                        (b.name().startsWith(QStringLiteral("i-Console+"))) ||
                        (b.name().startsWith(QStringLiteral("i-Running"))) ||
                        (b.name().startsWith(QStringLiteral("F63")))) &&
                       !trxappgateusb && !trxappgateusbBike && !toorx_bike && !JLL_IC400_bike && filter) {
                discoveryAgent->stop();
                trxappgateusb = new trxappgateusbtreadmill();
                emit deviceConnected();
                connect(trxappgateusb, &bluetoothdevice::connectedAndDiscovered, this,
                        &bluetooth::connectedAndDiscovered);
                // connect(trxappgateusb, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(trxappgateusb, &trxappgateusbtreadmill::debug, this, &bluetooth::debug);
                trxappgateusb->deviceDiscovered(b);
                userTemplateManager->start(trxappgateusb);
                innerTemplateManager->start(trxappgateusb);
            } else if ((((b.name().startsWith(QStringLiteral("TOORX")) ||
                          b.name().toUpper().startsWith(QStringLiteral("I-CONSOLE+")) ||
                          b.name().toUpper().startsWith(QStringLiteral("IBIKING+")) ||
                          b.name().toUpper().startsWith(QStringLiteral("ICONSOLE+")) ||
                          b.name().toUpper().startsWith(QStringLiteral("DKN MOTION"))) &&
                         (toorx_bike || JLL_IC400_bike))) &&
                       !trxappgateusb && !trxappgateusbBike && filter) {
                discoveryAgent->stop();
                trxappgateusbBike = new trxappgateusbbike(noWriteResistance, noHeartService);
                emit deviceConnected();
                connect(trxappgateusbBike, &bluetoothdevice::connectedAndDiscovered, this,
                        &bluetooth::connectedAndDiscovered);
                // connect(trxappgateusb, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(trxappgateusbBike, &trxappgateusbbike::debug, this, &bluetooth::debug);
                trxappgateusbBike->deviceDiscovered(b);
                userTemplateManager->start(trxappgateusbBike);
                innerTemplateManager->start(trxappgateusbBike);
            } else if (b.name().toUpper().startsWith(QStringLiteral("BFCP")) && !skandikaWiriBike && filter) {

                discoveryAgent->stop();
                skandikaWiriBike =
                    new skandikawiribike(noWriteResistance, noHeartService, bikeResistanceOffset, bikeResistanceGain);
                emit deviceConnected();
                connect(skandikaWiriBike, &bluetoothdevice::connectedAndDiscovered, this,
                        &bluetooth::connectedAndDiscovered);
                // connect(skandikaWiriBike, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(skandikaWiriBike, &skandikawiribike::debug, this, &bluetooth::debug);
                skandikaWiriBike->deviceDiscovered(b);
                userTemplateManager->start(skandikaWiriBike);
                innerTemplateManager->start(skandikaWiriBike);
            } else if ((b.name().toUpper().startsWith("RQ") && b.name().length() == 5) && !renphoBike && !snodeBike &&
                       !fitPlusBike && filter) {

                discoveryAgent->stop();
                renphoBike = new renphobike(noWriteResistance, noHeartService);
                emit(deviceConnected());
                connect(renphoBike, SIGNAL(connectedAndDiscovered()), this, SLOT(connectedAndDiscovered()));
                // connect(trxappgateusb, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(renphoBike, SIGNAL(debug(QString)), this, SLOT(debug(QString)));
                renphoBike->deviceDiscovered(b);
                userTemplateManager->start(renphoBike);
                innerTemplateManager->start(renphoBike);
            } else if ((b.name().startsWith(QStringLiteral("FS-")) && snode_bike) && !snodeBike && !ftmsBike &&
                       !fitPlusBike && filter) {

                discoveryAgent->stop();
                snodeBike = new snodebike(noWriteResistance, noHeartService);
                emit deviceConnected();
                connect(snodeBike, &bluetoothdevice::connectedAndDiscovered, this, &bluetooth::connectedAndDiscovered);
                // connect(trxappgateusb, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(snodeBike, &snodebike::debug, this, &bluetooth::debug);
                snodeBike->deviceDiscovered(b);
                userTemplateManager->start(snodeBike);
                innerTemplateManager->start(snodeBike);
            } else if ((b.name().startsWith(QStringLiteral("FS-")) && fitplus_bike) && !fitPlusBike && !ftmsBike &&
                       !snodeBike && filter) {

                discoveryAgent->stop();
                fitPlusBike =
                    new fitplusbike(noWriteResistance, noHeartService, bikeResistanceOffset, bikeResistanceGain);
                emit deviceConnected();
                connect(fitPlusBike, &bluetoothdevice::connectedAndDiscovered, this,
                        &bluetooth::connectedAndDiscovered);
                // connect(fitPlusBike, SIGNAL(disconnected()), this, SLOT(restart()));
                // NOTE: Commented due to #358
                // connect(fitPlusBike, SIGNAL(debug(QString)), this, SLOT(debug(QString)));
                fitPlusBike->deviceDiscovered(b);
                userTemplateManager->start(fitPlusBike);
                innerTemplateManager->start(fitPlusBike);
            } else if (((b.name().startsWith(QStringLiteral("FS-")) && !snode_bike && !fitplus_bike && !ftmsBike) ||
                        (b.name().startsWith(QStringLiteral("SW")) && b.name().length() == 14)) &&
                       !fitshowTreadmill && filter) {
                discoveryAgent->stop();
                fitshowTreadmill = new fitshowtreadmill(this->pollDeviceTime, noConsole, noHeartService);
                emit deviceConnected();
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

                discoveryAgent->stop();
                inspireBike = new inspirebike(noWriteResistance, noHeartService);
#if !defined(Q_OS_ANDROID) && !defined(Q_OS_IOS)
                stateFileRead();
#endif
                emit deviceConnected();
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
                discoveryAgent->stop();
                chronoBike = new chronobike(noWriteResistance, noHeartService);
#if !defined(Q_OS_ANDROID) && !defined(Q_OS_IOS)
                stateFileRead();
#endif
                emit deviceConnected();
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
    QString powerSensorName =
        settings.value(QStringLiteral("power_sensor_name"), QStringLiteral("Disabled")).toString();
    QString eliteRizerName = settings.value(QStringLiteral("elite_rizer_name"), QStringLiteral("Disabled")).toString();

    // only at the first very connection, setting the user default resistance
    if (device() && firstConnected &&
        (device()->deviceType() == bluetoothdevice::BIKE || device()->deviceType() == bluetoothdevice::ELLIPTICAL) &&
        settings.value(QStringLiteral("bike_resistance_start"), 1).toUInt() != 1) {
        qobject_cast<bike *>(device())->changeResistance(
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
                ftmsAccessory = new smartspin2k(false, false);
                // connect(heartRateBelt, SIGNAL(disconnected()), this, SLOT(restart()));

                connect(ftmsAccessory, &smartspin2k::debug, this, &bluetooth::debug);

                connect(this->device(), SIGNAL(resistanceChanged(int8_t)), ftmsAccessory,
                        SLOT(changeResistance(int8_t)));
                connect(this->device(), SIGNAL(resistanceRead(int8_t)), ftmsAccessory,
                        SLOT(resistanceReadFromTheBike(int8_t)));
                emit ftmsAccessoryConnected(ftmsAccessory);
                ftmsAccessory->deviceDiscovered(b);
                break;
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

    for (const QBluetoothDeviceInfo &b : qAsConst(devices)) {
        if (((b.name().startsWith(powerSensorName))) && !powerSensor &&
            !powerSensorName.startsWith(QStringLiteral("Disabled"))) {
            settings.setValue(QStringLiteral("power_sensor_lastdevice_name"), b.name());

#ifndef Q_OS_IOS
            settings.setValue(QStringLiteral("power_sensor_address"), b.address().toString());
#else
            settings.setValue("power_sensor_address", b.deviceUuid().toString());
#endif
            powerSensor = new stagesbike(false, false, true);
            // connect(heartRateBelt, SIGNAL(disconnected()), this, SLOT(restart()));

            connect(powerSensor, &stagesbike::debug, this, &bluetooth::debug);
            connect(powerSensor, &bluetoothdevice::powerChanged, this->device(), &bluetoothdevice::powerSensor);
            powerSensor->deviceDiscovered(b);
            break;
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
            connect(this->device(), &bluetoothdevice::inclinationChanged, eliteRizer, &eliterizer::inclinationChanged);
            eliteRizer->deviceDiscovered(b);
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

        emit(deviceConnected());
    }
#endif

    firstConnected = false;
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
    if (domyosBike) {

        delete domyosBike;
        domyosBike = nullptr;
    }
    if (domyosElliptical) {

        delete domyosElliptical;
        domyosElliptical = nullptr;
    }
    if (soleElliptical) {

        delete soleElliptical;
        soleElliptical = nullptr;
    }
    if (cscBike) {

        delete cscBike;
        cscBike = nullptr;
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
    if (trxappgateusb) {

        delete trxappgateusb;
        trxappgateusb = nullptr;
    }
    if (spiritTreadmill) {

        delete spiritTreadmill;
        spiritTreadmill = nullptr;
    }
    if (trxappgateusbBike) {

        delete trxappgateusbBike;
        trxappgateusbBike = nullptr;
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
    if (ftmsRower) {

        delete ftmsRower;
        ftmsRower = nullptr;
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
    if (eslinkerTreadmill) {

        delete eslinkerTreadmill;
        eslinkerTreadmill = nullptr;
    }
    if (flywheelBike) {

        delete flywheelBike;
        flywheelBike = nullptr;
    }
    if (schwinnIC4Bike) {

        delete schwinnIC4Bike;
        schwinnIC4Bike = nullptr;
    }
    if (sportsTechBike) {

        delete sportsTechBike;
        sportsTechBike = nullptr;
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
    if (renphoBike) {

        delete renphoBike;
        renphoBike = nullptr;
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
    if (eliteRizer) {

        // heartRateBelt->disconnectBluetooth(); // to test
        delete eliteRizer;
        eliteRizer = nullptr;
    }
    discoveryAgent->start();
}

bluetoothdevice *bluetooth::device() {
    if (domyos) {

        return domyos;
    } else if (domyosBike) {
        return domyosBike;
    } else if (fitshowTreadmill) {
        return fitshowTreadmill;
    } else if (domyosElliptical) {
        return domyosElliptical;
    } else if (soleElliptical) {
        return soleElliptical;
    } else if (cscBike) {
        return cscBike;
    } else if (npeCableBike) {
        return npeCableBike;
    } else if (tacxneo2Bike) {
        return tacxneo2Bike;
    } else if (stagesBike) {
        return stagesBike;
    } else if (toorx) {
        return toorx;
    } else if (spiritTreadmill) {
        return spiritTreadmill;
    } else if (trxappgateusb) {
        return trxappgateusb;
    } else if (trxappgateusbBike) {
        return trxappgateusbBike;
    } else if (horizonTreadmill) {
        return horizonTreadmill;
    } else if (soleF80) {
        return soleF80;
    } else if (kingsmithR2Treadmill) {
        return kingsmithR2Treadmill;
    } else if (kingsmithR1ProTreadmill) {
        return kingsmithR1ProTreadmill;
    } else if (echelonConnectSport) {
        return echelonConnectSport;
    } else if (echelonRower) {
        return echelonRower;
    } else if (echelonStride) {
        return echelonStride;
    } else if (ftmsRower) {
        return ftmsRower;
    } else if (smartrowRower) {
        return smartrowRower;
    } else if (yesoulBike) {
        return yesoulBike;
    } else if (proformBike) {
        return proformBike;
    } else if (proformTreadmill) {
        return proformTreadmill;
    } else if (eslinkerTreadmill) {
        return eslinkerTreadmill;
    } else if (flywheelBike) {
        return flywheelBike;
    } else if (schwinnIC4Bike) {
        return schwinnIC4Bike;
    } else if (sportsTechBike) {
        return sportsTechBike;
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
    } else if (renphoBike) {
        return renphoBike;
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

            qobject_cast<domyostreadmill *>(device())->setLastSpeed(speed);
            qobject_cast<domyostreadmill *>(device())->setLastInclination(inclination);
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

#if (QT_VERSION >= QT_VERSION_CHECK(5, 12, 0))
void bluetooth::deviceUpdated(const QBluetoothDeviceInfo &device, QBluetoothDeviceInfo::Fields updateFields) {

    debug("deviceUpdated " + device.name() + " " + updateFields);
}
#endif
