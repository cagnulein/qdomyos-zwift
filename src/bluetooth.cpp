#include "bluetooth.h"
#include <QFile>
#include <QDateTime>
#include <QMetaEnum>
#include <QBluetoothLocalDevice>
#include <QtXml>
#ifdef Q_OS_ANDROID
#include "keepawakehelper.h"
#include <QAndroidJniObject>
#endif

bluetooth::bluetooth(bool logs, QString deviceName, bool noWriteResistance, bool noHeartService, uint32_t pollDeviceTime, bool noConsole, bool testResistance, uint8_t bikeResistanceOffset, double bikeResistanceGain)
{
    QSettings settings;
    bool trx_route_key = settings.value("trx_route_key", false).toBool();

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

#if !defined(WIN32) && !defined(Q_OS_IOS)
    if(!QBluetoothLocalDevice::allDevices().count())
    {
        debug("no bluetooth dongle found!");
    }
    else
#endif
    {
        // Create a discovery agent and connect to its signals
        discoveryAgent = new QBluetoothDeviceDiscoveryAgent(this);
        connect(discoveryAgent, SIGNAL(deviceDiscovered(QBluetoothDeviceInfo)),
                this, SLOT(deviceDiscovered(QBluetoothDeviceInfo)));
#if (QT_VERSION >= QT_VERSION_CHECK(5, 12, 0))
        connect(discoveryAgent, SIGNAL(deviceUpdated(const QBluetoothDeviceInfo&, QBluetoothDeviceInfo::Fields)),
                this, SLOT(deviceUpdated(const QBluetoothDeviceInfo&, QBluetoothDeviceInfo::Fields)));
#endif
        connect(discoveryAgent, SIGNAL(canceled()),
                this, SLOT(canceled()));
        connect(discoveryAgent, SIGNAL(finished()),
                this, SLOT(finished()));

        // Start a discovery
        discoveryAgent->setLowEnergyDiscoveryTimeout(10000);

        if(!settings.value("bluetooth_lastdevice_name", "").toString().compare(settings.value("filter_device", "Disabled").toString()))
        {
            discoveryAgent->stop();
            domyos = new domyostreadmill(this->pollDeviceTime, noConsole, noHeartService);
    #if !defined(Q_OS_ANDROID) && !defined(Q_OS_IOS)
            stateFileRead();
    #endif
            emit(deviceConnected());
            connect(domyos, SIGNAL(connectedAndDiscovered()), this, SLOT(connectedAndDiscovered()));
            //connect(domyos, SIGNAL(disconnected()), this, SLOT(restart()));
            connect(domyos, SIGNAL(debug(QString)), this, SLOT(debug(QString)));
            connect(domyos, SIGNAL(speedChanged(double)), this, SLOT(speedChanged(double)));
            connect(domyos, SIGNAL(inclinationChanged(double)), this, SLOT(inclinationChanged(double)));
            domyos->deviceDiscovered(settings.value("bluetooth_lastdevice_address", "").toString());
            connect(this, SIGNAL(searchingStop()), domyos, SLOT(searchingStop()));
            if(!discoveryAgent->isActive())
                emit searchingStop();
            qDebug() << "connecting directly";
        }
        else
        {
            if(!trx_route_key)
                discoveryAgent->start(QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);
            else
                discoveryAgent->start(QBluetoothDeviceDiscoveryAgent::ClassicMethod | QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);
        }
    }
}

bluetooth::~bluetooth()
{
    /*if(device())
    {
        device()->disconnectBluetooth();
    }*/
}

void bluetooth::finished()
{
    debug("BTLE scanning finished");
    QSettings settings;
    QString heartRateBeltName = settings.value("heart_rate_belt_name", "Disabled").toString();
    bool trx_route_key = settings.value("trx_route_key", false).toBool();
    bool heartRateBeltFound = heartRateBeltName.startsWith("Disabled");

    if(!heartRateBeltFound && !heartRateBeltAvaiable())
    {
        // force heartRateBelt off
        forceHeartBeltOffForTimeout = true;
    }

    if(!trx_route_key)
        discoveryAgent->start(QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);
    else
        discoveryAgent->start(QBluetoothDeviceDiscoveryAgent::ClassicMethod | QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);
}

void bluetooth::canceled()
{
    debug("BTLE scanning stops");
    emit searchingStop();
}

void bluetooth::debug(QString text)
{
    if(logs)
        qDebug() << text;
}

bool bluetooth::heartRateBeltAvaiable()
{
    QSettings settings;
    QString heartRateBeltName = settings.value("heart_rate_belt_name", "Disabled").toString();

    foreach(QBluetoothDeviceInfo b, devices)
    {
        if(!heartRateBeltName.compare(b.name()))
        {
            return true;
        }
    }
    return false;
}

void bluetooth::deviceDiscovered(const QBluetoothDeviceInfo &device)
{
    QSettings settings;
    QString heartRateBeltName = settings.value("heart_rate_belt_name", "Disabled").toString();
    bool heartRateBeltFound = heartRateBeltName.startsWith("Disabled");
    bool toorx_bike = settings.value("toorx_bike", false).toBool();
    bool snode_bike = settings.value("snode_bike", false).toBool();

    if(!heartRateBeltFound)
    {
        heartRateBeltFound = heartRateBeltAvaiable();
    }

    bool found = false;
    QMutableListIterator<QBluetoothDeviceInfo> i(devices);
    while (i.hasNext()) {
        QBluetoothDeviceInfo b = i.next();
        if(SAME_BLUETOOTH_DEVICE(b,device))
        {
            found = true;
            break;
        }
    }
    if(!found)
        devices.append(device);

    emit deviceFound(device.name());
    debug("Found new device: " + device.name() + " (" + device.address().toString() + ')' + " " + device.majorDeviceClass() + ":" + device.minorDeviceClass());

    if(onlyDiscover) return;

    if(heartRateBeltFound || forceHeartBeltOffForTimeout)
    {
        foreach(QBluetoothDeviceInfo b, devices)
        {
            bool filter = true;
            if(filterDevice.length() && !filterDevice.startsWith("Disabled"))
            {
                filter = (b.name().compare(filterDevice, Qt::CaseInsensitive) == 0);
            }
            if(b.name().startsWith("M3") && !m3iBike && filter)
            {
                if (m3ibike::isCorrectUnit(b)) {
                    discoveryAgent->stop();
                    m3iBike = new m3ibike(noWriteResistance, noHeartService);
                    emit(deviceConnected());
                    connect(m3iBike, SIGNAL(connectedAndDiscovered()), this, SLOT(connectedAndDiscovered()));
                    //connect(domyosBike, SIGNAL(disconnected()), this, SLOT(restart()));
                    connect(m3iBike, SIGNAL(debug(QString)), this, SLOT(debug(QString)));
                    m3iBike->deviceDiscovered(b);
                    connect(this, SIGNAL(searchingStop()), m3iBike, SLOT(searchingStop()));
                    if(!discoveryAgent->isActive())
                        emit searchingStop();
                }
            }
            else if(b.name().startsWith("Domyos-Bike") && !b.name().startsWith("DomyosBridge") && !domyosBike && filter)
            {
                discoveryAgent->stop();
                domyosBike = new domyosbike(noWriteResistance, noHeartService, testResistance, bikeResistanceOffset, bikeResistanceGain);
                emit(deviceConnected());
                connect(domyosBike, SIGNAL(connectedAndDiscovered()), this, SLOT(connectedAndDiscovered()));
                //connect(domyosBike, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(domyosBike, SIGNAL(debug(QString)), this, SLOT(debug(QString)));
                domyosBike->deviceDiscovered(b);
                connect(this, SIGNAL(searchingStop()), domyosBike, SLOT(searchingStop()));
                if(!discoveryAgent->isActive())
                    emit searchingStop();
            }
            else if(b.name().startsWith("Domyos-EL") && !b.name().startsWith("DomyosBridge") && !domyosElliptical && filter)
            {
                discoveryAgent->stop();
                domyosElliptical = new domyoselliptical(noWriteResistance, noHeartService, testResistance, bikeResistanceOffset, bikeResistanceGain);
                emit(deviceConnected());
                connect(domyosElliptical, SIGNAL(connectedAndDiscovered()), this, SLOT(connectedAndDiscovered()));
                //connect(domyosElliptical, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(domyosElliptical, SIGNAL(debug(QString)), this, SLOT(debug(QString)));
                domyosElliptical->deviceDiscovered(b);
                connect(this, SIGNAL(searchingStop()), domyosElliptical, SLOT(searchingStop()));
                if(!discoveryAgent->isActive())
                    emit searchingStop();
            }
            else if(b.name().startsWith("Domyos") && !b.name().startsWith("DomyosBr") && !domyos && !domyosElliptical && !domyosBike && filter)
            {
                settings.setValue("bluetooth_lastdevice_name", b.name());
#ifndef Q_OS_IOS
                settings.setValue("bluetooth_lastdevice_address", b.address().toString());
#else
                settings.setValue("bluetooth_lastdevice_address", b.deviceUuid().toString());
#endif

                discoveryAgent->stop();
                domyos = new domyostreadmill(this->pollDeviceTime, noConsole, noHeartService);
#if !defined(Q_OS_ANDROID) && !defined(Q_OS_IOS)
                stateFileRead();
#endif
                emit(deviceConnected());
                connect(domyos, SIGNAL(connectedAndDiscovered()), this, SLOT(connectedAndDiscovered()));
                //connect(domyos, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(domyos, SIGNAL(debug(QString)), this, SLOT(debug(QString)));
                connect(domyos, SIGNAL(speedChanged(double)), this, SLOT(speedChanged(double)));
                connect(domyos, SIGNAL(inclinationChanged(double)), this, SLOT(inclinationChanged(double)));
                domyos->deviceDiscovered(b);
                connect(this, SIGNAL(searchingStop()), domyos, SLOT(searchingStop()));
                if(!discoveryAgent->isActive())
                    emit searchingStop();
            }
            else if((b.name().toUpper().startsWith("HORIZON") || b.name().toUpper().startsWith("F80")) && !horizonTreadmill && filter)
            {
                discoveryAgent->stop();
                horizonTreadmill = new horizontreadmill(noWriteResistance, noHeartService);
#if !defined(Q_OS_ANDROID) && !defined(Q_OS_IOS)
                stateFileRead();
#endif
                emit(deviceConnected());
                connect(horizonTreadmill, SIGNAL(connectedAndDiscovered()), this, SLOT(connectedAndDiscovered()));
                //connect(horizonTreadmill, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(horizonTreadmill, SIGNAL(debug(QString)), this, SLOT(debug(QString)));
                connect(horizonTreadmill, SIGNAL(speedChanged(double)), this, SLOT(speedChanged(double)));
                connect(horizonTreadmill, SIGNAL(inclinationChanged(double)), this, SLOT(inclinationChanged(double)));
                horizonTreadmill->deviceDiscovered(b);
                connect(this, SIGNAL(searchingStop()), horizonTreadmill, SLOT(searchingStop()));
                if(!discoveryAgent->isActive())
                    emit searchingStop();
            }
            else if(b.name().startsWith("ECH") && !echelonConnectSport && filter)
            {
                discoveryAgent->stop();
                echelonConnectSport = new echelonconnectsport(noWriteResistance, noHeartService, bikeResistanceOffset, bikeResistanceGain);
                //stateFileRead();
                emit(deviceConnected());
                connect(echelonConnectSport, SIGNAL(connectedAndDiscovered()), this, SLOT(connectedAndDiscovered()));
                //connect(echelonConnectSport, SIGNAL(disconnected()), this, SLOT(restart()));
                //connect(echelonConnectSport, SIGNAL(debug(QString)), this, SLOT(debug(QString)));
                //connect(echelonConnectSport, SIGNAL(speedChanged(double)), this, SLOT(speedChanged(double)));
                //connect(echelonConnectSport, SIGNAL(inclinationChanged(double)), this, SLOT(inclinationChanged(double)));
                echelonConnectSport->deviceDiscovered(b);
            }
            else if((b.name().toUpper().startsWith("IC BIKE") || b.name().toUpper().startsWith("C7-")) && !schwinnIC4Bike && filter)
            {
                discoveryAgent->stop();
                schwinnIC4Bike = new schwinnic4bike(noWriteResistance, noHeartService);
                //stateFileRead();
                emit(deviceConnected());
                connect(schwinnIC4Bike, SIGNAL(connectedAndDiscovered()), this, SLOT(connectedAndDiscovered()));
                //connect(echelonConnectSport, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(schwinnIC4Bike, SIGNAL(debug(QString)), this, SLOT(debug(QString)));
                //connect(echelonConnectSport, SIGNAL(speedChanged(double)), this, SLOT(speedChanged(double)));
                //connect(echelonConnectSport, SIGNAL(inclinationChanged(double)), this, SLOT(inclinationChanged(double)));
                schwinnIC4Bike->deviceDiscovered(b);
            }
            else if(b.name().toUpper().startsWith("EW-BK") && !sportsTechBike && filter)
            {
                discoveryAgent->stop();
                sportsTechBike = new sportstechbike(noWriteResistance, noHeartService);
                //stateFileRead();
                emit(deviceConnected());
                connect(sportsTechBike, SIGNAL(connectedAndDiscovered()), this, SLOT(connectedAndDiscovered()));
                //connect(echelonConnectSport, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(sportsTechBike, SIGNAL(debug(QString)), this, SLOT(debug(QString)));
                //connect(echelonConnectSport, SIGNAL(speedChanged(double)), this, SLOT(speedChanged(double)));
                //connect(echelonConnectSport, SIGNAL(inclinationChanged(double)), this, SLOT(inclinationChanged(double)));
                sportsTechBike->deviceDiscovered(b);
            }
            else if(b.name().startsWith("YESOUL") && !yesoulBike && filter)
            {
                discoveryAgent->stop();
                yesoulBike = new yesoulbike(noWriteResistance, noHeartService);
                //stateFileRead();
                emit(deviceConnected());
                connect(yesoulBike, SIGNAL(connectedAndDiscovered()), this, SLOT(connectedAndDiscovered()));
                //connect(yesoulBike, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(yesoulBike, SIGNAL(debug(QString)), this, SLOT(debug(QString)));
                //connect(echelonConnectSport, SIGNAL(speedChanged(double)), this, SLOT(speedChanged(double)));
                //connect(echelonConnectSport, SIGNAL(inclinationChanged(double)), this, SLOT(inclinationChanged(double)));
                yesoulBike->deviceDiscovered(b);
            }
            else if(b.name().startsWith("I_EB") && !proformBike && filter)
            {
                discoveryAgent->stop();
                proformBike = new proformbike(noWriteResistance, noHeartService);
                //stateFileRead();
                emit(deviceConnected());
                connect(proformBike, SIGNAL(connectedAndDiscovered()), this, SLOT(connectedAndDiscovered()));
                //connect(proformBike, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(proformBike, SIGNAL(debug(QString)), this, SLOT(debug(QString)));
                //connect(proformBike, SIGNAL(speedChanged(double)), this, SLOT(speedChanged(double)));
                //connect(proformBike, SIGNAL(inclinationChanged(double)), this, SLOT(inclinationChanged(double)));
                proformBike->deviceDiscovered(b);
            }
            else if(b.name().startsWith("I_TL") && !proformTreadmill && filter)
            {
                discoveryAgent->stop();
                proformTreadmill = new proformtreadmill(noWriteResistance, noHeartService);
                //stateFileRead();
                emit(deviceConnected());
                connect(proformTreadmill, SIGNAL(connectedAndDiscovered()), this, SLOT(connectedAndDiscovered()));
                //connect(proformtreadmill, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(proformTreadmill, SIGNAL(debug(QString)), this, SLOT(debug(QString)));
                //connect(proformtreadmill, SIGNAL(speedChanged(double)), this, SLOT(speedChanged(double)));
                //connect(proformtreadmill, SIGNAL(inclinationChanged(double)), this, SLOT(inclinationChanged(double)));
                proformTreadmill->deviceDiscovered(b);
            }
            else if(b.name().toUpper().startsWith("ESLINKER") && !eslinkerTreadmill && filter)
            {
                discoveryAgent->stop();
                eslinkerTreadmill = new eslinkertreadmill(noWriteResistance, noHeartService);
                //stateFileRead();
                emit(deviceConnected());
                connect(eslinkerTreadmill, SIGNAL(connectedAndDiscovered()), this, SLOT(connectedAndDiscovered()));
                //connect(proformtreadmill, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(eslinkerTreadmill, SIGNAL(debug(QString)), this, SLOT(debug(QString)));
                //connect(proformtreadmill, SIGNAL(speedChanged(double)), this, SLOT(speedChanged(double)));
                //connect(proformtreadmill, SIGNAL(inclinationChanged(double)), this, SLOT(inclinationChanged(double)));
                eslinkerTreadmill->deviceDiscovered(b);
            }
            else if(b.name().startsWith("Flywheel") && !flywheelBike && filter)
            {
                discoveryAgent->stop();
                flywheelBike = new flywheelbike(noWriteResistance, noHeartService);
                //stateFileRead();
                emit(deviceConnected());
                connect(flywheelBike, SIGNAL(connectedAndDiscovered()), this, SLOT(connectedAndDiscovered()));
                //connect(flywheelBike, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(flywheelBike, SIGNAL(debug(QString)), this, SLOT(debug(QString)));
                //connect(echelonConnectSport, SIGNAL(speedChanged(double)), this, SLOT(speedChanged(double)));
                //connect(echelonConnectSport, SIGNAL(inclinationChanged(double)), this, SLOT(inclinationChanged(double)));
                flywheelBike->deviceDiscovered(b);
            }
            else if((b.name().startsWith("TRX ROUTE KEY")) && !toorx && filter)
            {
                discoveryAgent->stop();
                toorx = new toorxtreadmill();
                emit(deviceConnected());
                connect(toorx, SIGNAL(connectedAndDiscovered()), this, SLOT(connectedAndDiscovered()));
                //connect(toorx, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(toorx, SIGNAL(debug(QString)), this, SLOT(debug(QString)));
                toorx->deviceDiscovered(b);
            }
            else if(((b.name().startsWith("TOORX")) || b.name().toUpper().startsWith("XT485") || (b.name().startsWith("V-RUN")) || (b.name().startsWith("i-Console+")) || (b.name().startsWith("i-Running"))  || (device.name().startsWith("F63"))) && !trxappgateusb && !trxappgateusbBike && !toorx_bike && filter)
            {
                discoveryAgent->stop();
                trxappgateusb = new trxappgateusbtreadmill();
                emit(deviceConnected());
                connect(trxappgateusb, SIGNAL(connectedAndDiscovered()), this, SLOT(connectedAndDiscovered()));
                //connect(trxappgateusb, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(trxappgateusb, SIGNAL(debug(QString)), this, SLOT(debug(QString)));
                trxappgateusb->deviceDiscovered(b);
            }
            else if((((b.name().startsWith("TOORX") || b.name().toUpper().startsWith("I-CONSOLE+") || b.name().toUpper().startsWith("ICONSOLE+")) && toorx_bike)) && !trxappgateusb && !trxappgateusbBike && filter)
            {
                discoveryAgent->stop();
                trxappgateusbBike = new trxappgateusbbike(noWriteResistance, noHeartService);
                emit(deviceConnected());
                connect(trxappgateusbBike, SIGNAL(connectedAndDiscovered()), this, SLOT(connectedAndDiscovered()));
                //connect(trxappgateusb, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(trxappgateusbBike, SIGNAL(debug(QString)), this, SLOT(debug(QString)));
                trxappgateusbBike->deviceDiscovered(b);
            }
            else if(b.name().toUpper().startsWith("BFCP") && !skandikaWiriBike && filter)
            {
                discoveryAgent->stop();
                skandikaWiriBike = new skandikawiribike(noWriteResistance, noHeartService, bikeResistanceOffset, bikeResistanceGain);
                emit(deviceConnected());
                connect(skandikaWiriBike, SIGNAL(connectedAndDiscovered()), this, SLOT(connectedAndDiscovered()));
                //connect(skandikaWiriBike, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(skandikaWiriBike, SIGNAL(debug(QString)), this, SLOT(debug(QString)));
                skandikaWiriBike->deviceDiscovered(b);
            }
            else if((b.name().startsWith("FS-") && snode_bike) && !snodeBike && filter)
            {
                discoveryAgent->stop();
                snodeBike = new snodebike(noWriteResistance, noHeartService);
                emit(deviceConnected());
                connect(snodeBike, SIGNAL(connectedAndDiscovered()), this, SLOT(connectedAndDiscovered()));
                //connect(trxappgateusb, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(snodeBike, SIGNAL(debug(QString)), this, SLOT(debug(QString)));
                snodeBike->deviceDiscovered(b);
            }
            else if(((b.name().startsWith("FS-") && !snode_bike) || (b.name().startsWith("SW") && b.name().length() == 14)) && !fitshowTreadmill && filter)
            {
                discoveryAgent->stop();
                fitshowTreadmill = new fitshowtreadmill(this->pollDeviceTime, noConsole, noHeartService);
                emit(deviceConnected());
                connect(fitshowTreadmill, SIGNAL(connectedAndDiscovered()), this, SLOT(connectedAndDiscovered()));
                connect(fitshowTreadmill, SIGNAL(debug(QString)), this, SLOT(debug(QString)));
                fitshowTreadmill->deviceDiscovered(b);
                connect(this, SIGNAL(searchingStop()), fitshowTreadmill, SLOT(searchingStop()));
                if(!discoveryAgent->isActive())
                    emit searchingStop();
            }
            else if(b.name().toUpper().startsWith("IC") && b.name().length() == 8 && !inspireBike && filter)
            {
                discoveryAgent->stop();
                inspireBike = new inspirebike(noWriteResistance, noHeartService);
#if !defined(Q_OS_ANDROID) && !defined(Q_OS_IOS)
                stateFileRead();
#endif
                emit(deviceConnected());
                connect(inspireBike, SIGNAL(connectedAndDiscovered()), this, SLOT(connectedAndDiscovered()));
                connect(inspireBike, SIGNAL(debug(QString)), this, SLOT(debug(QString)));
                connect(inspireBike, SIGNAL(speedChanged(double)), this, SLOT(speedChanged(double)));
                connect(inspireBike, SIGNAL(inclinationChanged(double)), this, SLOT(inclinationChanged(double)));
                inspireBike->deviceDiscovered(b);
                connect(this, SIGNAL(searchingStop()), inspireBike, SLOT(searchingStop()));
                if(!discoveryAgent->isActive())
                    emit searchingStop();
            }
        }
    }
}

void bluetooth::connectedAndDiscovered()
{
    static bool firstConnected = true;
    QSettings settings;
    QString heartRateBeltName = settings.value("heart_rate_belt_name", "Disabled").toString();

    // only at the first very connection, setting the user default resistance
    if(device() && firstConnected &&
       (device()->deviceType() == bluetoothdevice::BIKE || device()->deviceType() == bluetoothdevice::ELLIPTICAL) &&
       settings.value("bike_resistance_start", 1).toUInt() != 1)
    {
        ((bike*)device())->changeResistance(settings.value("bike_resistance_start", 1).toUInt());
    }

    if(this->device() != nullptr)
    {
        foreach(QBluetoothDeviceInfo b, devices)
        {
            if(((b.name().startsWith(heartRateBeltName))) && !heartRateBelt && !heartRateBeltName.startsWith("Disabled"))
            {
                heartRateBelt = new heartratebelt();
                //connect(heartRateBelt, SIGNAL(disconnected()), this, SLOT(restart()));

                connect(heartRateBelt, SIGNAL(debug(QString)), this, SLOT(debug(QString)));
                connect(heartRateBelt, SIGNAL(heartRate(uint8_t)), this->device(), SLOT(heartRate(uint8_t)));
                heartRateBelt->deviceDiscovered(b);

                break;
            }
        }
    }

#ifdef Q_OS_ANDROID
    if(settings.value("ant_cadence", false).toBool() || settings.value("ant_heart", false).toBool())
    {
        QAndroidJniObject activity = QAndroidJniObject::callStaticObjectMethod("org/qtproject/qt5/android/QtNative", "activity", "()Landroid/app/Activity;");
        KeepAwakeHelper::antObject(true)->callMethod<void>("antStart","(Landroid/app/Activity;ZZZ)V", activity.object<jobject>(), settings.value("ant_cadence", false).toBool(), settings.value("ant_heart", false).toBool(), settings.value("ant_garmin", false).toBool());
    }
#endif

    firstConnected = false;
}

void bluetooth::heartRate(uint8_t heart)
{
    Q_UNUSED(heart)
}

void bluetooth::restart()
{
    QSettings settings;

    if(onlyDiscover)
    {
        onlyDiscover = false;
        discoveryAgent->start();
        return;
    }

    if(settings.value("bluetooth_no_reconnection", false).toBool())
        exit(0);

    devices.clear();

    if(device() && device()->VirtualDevice())
    {
        if(device()->deviceType() == bluetoothdevice::TREADMILL)
            delete (virtualtreadmill*)device()->VirtualDevice();
        else if(device()->deviceType() == bluetoothdevice::BIKE)
            delete (virtualbike*)device()->VirtualDevice();
        else if(device()->deviceType() == bluetoothdevice::ELLIPTICAL)
            delete (virtualtreadmill*)device()->VirtualDevice();
    }

    if(domyos)
    {        
        delete domyos;        
        domyos = 0;
    }
    if(m3iBike)
    {
        delete m3iBike;
        m3iBike = 0;
    }
    if(fitshowTreadmill)
    {
        delete fitshowTreadmill;
        fitshowTreadmill = 0;
    }
    if(horizonTreadmill)
    {
        delete horizonTreadmill;
        horizonTreadmill = 0;
    }
    if(domyosBike)
    {
        delete domyosBike;
        domyosBike = 0;
    }
    if(domyosElliptical)
    {
        delete domyosElliptical;
        domyosElliptical = 0;
    }
    if(toorx)
    {
        delete toorx;
        toorx = 0;
    }
    if(trxappgateusb)
    {
        delete trxappgateusb;
        trxappgateusb = 0;
    }
    if(trxappgateusbBike)
    {
        delete trxappgateusbBike;
        trxappgateusbBike = 0;
    }
    if(echelonConnectSport)
    {
        delete echelonConnectSport;
        echelonConnectSport = 0;
    }
    if(yesoulBike)
    {
        delete yesoulBike;
        yesoulBike = 0;
    }
    if(proformBike)
    {
        delete proformBike;
        proformBike = 0;
    }
    if(proformTreadmill)
    {
        delete proformTreadmill;
        proformTreadmill = 0;
    }
    if(eslinkerTreadmill)
    {
        delete eslinkerTreadmill;
        eslinkerTreadmill = 0;
    }
    if(flywheelBike)
    {
        delete flywheelBike;
        flywheelBike = 0;
    }
    if(schwinnIC4Bike)
    {
        delete schwinnIC4Bike;
        schwinnIC4Bike = 0;
    }
    if(sportsTechBike)
    {
        delete sportsTechBike;
        sportsTechBike = 0;
    }
    if(inspireBike)
    {
        delete inspireBike;
        inspireBike = 0;
    }
    if(snodeBike)
    {
        delete snodeBike;
        snodeBike = 0;
    }
    if(skandikaWiriBike)
    {
        delete skandikaWiriBike;
        skandikaWiriBike = 0;
    }
    if(heartRateBelt)
    {
        //heartRateBelt->disconnectBluetooth(); // to test
        delete heartRateBelt;
        heartRateBelt = 0;
    }
    discoveryAgent->start();
}

bluetoothdevice* bluetooth::device()
{
    if(domyos)
        return domyos;
    else if(domyosBike)
        return domyosBike;
    else if(fitshowTreadmill)
        return fitshowTreadmill;
    else if(domyosElliptical)
        return domyosElliptical;
    else if(toorx)
        return toorx;
    else if(trxappgateusb)
        return trxappgateusb;
    else if(trxappgateusbBike)
        return trxappgateusbBike;
    else if(horizonTreadmill)
        return horizonTreadmill;
    else if(echelonConnectSport)
        return echelonConnectSport;
    else if(yesoulBike)
        return yesoulBike;
    else if(proformBike)
        return proformBike;
    else if(proformTreadmill)
        return proformTreadmill;
    else if(eslinkerTreadmill)
        return eslinkerTreadmill;
    else if(flywheelBike)
        return flywheelBike;
    else if(schwinnIC4Bike)
        return schwinnIC4Bike;
    else if(sportsTechBike)
        return sportsTechBike;
    else if(inspireBike)
        return inspireBike;
    else if(m3iBike)
        return m3iBike;
    else if(snodeBike)
        return snodeBike;
    else if(skandikaWiriBike)
        return skandikaWiriBike;
    return nullptr;
}

bool bluetooth::handleSignal(int signal)
{
    if(signal == SIGNALS::SIG_INT)
    {
        qDebug() << "SIGINT";
        QFile::remove("status.xml");
        exit(0);
    }
    // Let the signal propagate as though we had not been there
    return false;
}

void bluetooth::stateFileRead()
{
    if(!device()) return;

    QFile* log;
    QDomDocument xmlBOM;
    log = new QFile("status.xml");
    if(!log->open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "Open status.xml for writing failed";
        return;
    }
    xmlBOM.setContent(log);
    QDomElement root=xmlBOM.documentElement();

    // Get root names and attributes
    QString Type=root.tagName();
    QString lastUpdated = root.attribute("Updated", QDateTime::currentDateTime().toString());

    QDomElement machine=root.firstChild().toElement();
    // Loop while there is a child
    while(!machine.isNull())
    {
        // Check if the child tag name is COMPONENT
        if (machine.tagName()=="Treadmill")
        {
            // Read and display the component ID
            double speed = machine.attribute("Speed", "0.0").toDouble();
            double inclination = machine.attribute("Incline", "0.0").toDouble();

            ((domyostreadmill*)device())->setLastSpeed(speed);
            ((domyostreadmill*)device())->setLastInclination(inclination);
        }

        // Next component
        machine = machine.nextSibling().toElement();
    }

    log->close();
}

void bluetooth::stateFileUpdate()
{
    if(!device()) return;
    if(device()->deviceType() != bluetoothdevice::TREADMILL) return;

    QFile* log;
    QDomDocument docStatus;
    QDomElement docRoot;
    QDomElement docTreadmill;
    QDomElement docHeart;
    log = new QFile("status.xml");
    if(!log->open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qDebug() << "Open status.xml for writing failed";
        return;
    }
    docRoot = docStatus.createElement("Gym");
    docStatus.appendChild(docRoot);
    docTreadmill = docStatus.createElement("Treadmill");
    docTreadmill.setAttribute("Speed", QString::number(device()->currentSpeed().value(), 'f', 1));
    docTreadmill.setAttribute("Incline", QString::number(((treadmill*)device())->currentInclination().value(), 'f', 1));
    docRoot.appendChild(docTreadmill);
    //docHeart = docStatus.createElement("Heart");
    //docHeart.setAttribute("Rate", QString::number(currentHeart));
    //docRoot.appendChild(docHeart);
    docRoot.setAttribute("Updated", QDateTime::currentDateTime().toString());
    QTextStream stream(log);
    stream << docStatus.toString();
    log->flush();
    log->close();
}

void bluetooth::speedChanged(double speed)
{
    Q_UNUSED(speed);
    stateFileUpdate();
}

void bluetooth::inclinationChanged(double inclination)
{
    Q_UNUSED(inclination);
    stateFileUpdate();
}

#if (QT_VERSION >= QT_VERSION_CHECK(5, 12, 0))
void bluetooth::deviceUpdated(const QBluetoothDeviceInfo &device, QBluetoothDeviceInfo::Fields updateFields)
{
    debug("deviceUpdated " + device.name() + " " + updateFields);
}
#endif
