#include "bluetooth.h"
#include <QFile>
#include <QDateTime>
#include <QMetaEnum>
#include <QBluetoothLocalDevice>
#include <QtXml>

bluetooth::bluetooth(bool logs, QString deviceName, bool noWriteResistance, bool noHeartService, uint32_t pollDeviceTime, bool noConsole, bool testResistance, uint8_t bikeResistanceOffset, uint8_t bikeResistanceGain)
{
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

        // Start a discovery
        discoveryAgent->start();
    }
}

void bluetooth::canceled()
{
    debug("BTLE scanning stops");
    emit searchingStop();
}

void bluetooth::debug(QString text)
{
    QString debug = QDateTime::currentDateTime().toString() + " " + QString::number(QDateTime::currentMSecsSinceEpoch()) + " " + text;
    if(logs)
        qDebug() << debug;
}

void bluetooth::deviceDiscovered(const QBluetoothDeviceInfo &device)
{
    QSettings settings;
    QString heartRateBeltName = settings.value("heart_rate_belt_name", "Disabled").toString();
    bool heartRateBeltFound = heartRateBeltName.startsWith("Disabled");

    if(!heartRateBeltFound)
    {
        foreach(QBluetoothDeviceInfo b, devices)
        {
            if(!heartRateBeltName.compare(b.name()))
            {
                heartRateBeltFound = true;
                break;
            }
        }
    }

    bool found = false;
    foreach(QBluetoothDeviceInfo b, devices)
    {
        if(!device.name().compare(b.name()))
        {
            found = true;
            break;
        }
    }
    if(!found)
        devices.append(device);

    emit deviceFound(device.name());
    debug("Found new device: " + device.name() + " (" + device.address().toString() + ')' + " " + device.majorDeviceClass() + ":" + device.minorDeviceClass());

    bool filter = true;
    if(filterDevice.length())
    {
        filter = (device.name().compare(filterDevice, Qt::CaseInsensitive) == 0);
    }

    if(heartRateBeltFound)
    {
        foreach(QBluetoothDeviceInfo b, devices)
        {
            if(b.name().startsWith("Domyos-Bike") && !b.name().startsWith("DomyosBridge") && !domyosBike && filter)
            {
                discoveryAgent->stop();
                domyosBike = new domyosbike(noWriteResistance, noHeartService, testResistance, bikeResistanceOffset, bikeResistanceGain);
                emit(deviceConnected());
                connect(domyosBike, SIGNAL(connectedAndDiscovered()), this, SLOT(connectedAndDiscovered()));
                connect(domyosBike, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(domyosBike, SIGNAL(debug(QString)), this, SLOT(debug(QString)));
                domyosBike->deviceDiscovered(b);
                connect(this, SIGNAL(searchingStop()), domyosBike, SLOT(searchingStop()));
                if(!discoveryAgent->isActive())
                    emit searchingStop();
            }
            else if(b.name().startsWith("Domyos") && !b.name().startsWith("DomyosBr") && !domyos && filter)
            {
                discoveryAgent->stop();
                domyos = new domyostreadmill(this->pollDeviceTime, noConsole, noHeartService);
                stateFileRead();
                emit(deviceConnected());
                connect(domyos, SIGNAL(connectedAndDiscovered()), this, SLOT(connectedAndDiscovered()));
                connect(domyos, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(domyos, SIGNAL(debug(QString)), this, SLOT(debug(QString)));
                connect(domyos, SIGNAL(speedChanged(double)), this, SLOT(speedChanged(double)));
                connect(domyos, SIGNAL(inclinationChanged(double)), this, SLOT(inclinationChanged(double)));
                domyos->deviceDiscovered(b);
                connect(this, SIGNAL(searchingStop()), domyos, SLOT(searchingStop()));
                if(!discoveryAgent->isActive())
                    emit searchingStop();
            }
            else if(b.name().startsWith("ECH") && !echelonConnectSport && filter)
            {
                discoveryAgent->stop();
                echelonConnectSport = new echelonconnectsport(noWriteResistance, noHeartService);
                //stateFileRead();
                emit(deviceConnected());
                connect(echelonConnectSport, SIGNAL(connectedAndDiscovered()), this, SLOT(connectedAndDiscovered()));
                connect(echelonConnectSport, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(echelonConnectSport, SIGNAL(debug(QString)), this, SLOT(debug(QString)));
                //connect(echelonConnectSport, SIGNAL(speedChanged(double)), this, SLOT(speedChanged(double)));
                //connect(echelonConnectSport, SIGNAL(inclinationChanged(double)), this, SLOT(inclinationChanged(double)));
                echelonConnectSport->deviceDiscovered(b);
            }
            else if((b.name().startsWith("TRX ROUTE KEY")) && !toorx && filter)
            {
                discoveryAgent->stop();
                toorx = new toorxtreadmill();
                emit(deviceConnected());
                connect(toorx, SIGNAL(connectedAndDiscovered()), this, SLOT(connectedAndDiscovered()));
                connect(toorx, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(toorx, SIGNAL(debug(QString)), this, SLOT(debug(QString)));
                toorx->deviceDiscovered(b);
            }
            else if(((b.name().startsWith("TOORX")) || (b.name().startsWith("V-RUN")) || (b.name().startsWith("i-Running"))) && !trxappgateusb && filter)
            {
                discoveryAgent->stop();
                trxappgateusb = new trxappgateusbtreadmill();
                emit(deviceConnected());
                connect(trxappgateusb, SIGNAL(connectedAndDiscovered()), this, SLOT(connectedAndDiscovered()));
                connect(trxappgateusb, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(trxappgateusb, SIGNAL(debug(QString)), this, SLOT(debug(QString)));
                trxappgateusb->deviceDiscovered(b);
            }
        }
    }
}

void bluetooth::connectedAndDiscovered()
{
    QSettings settings;
    QString heartRateBeltName = settings.value("heart_rate_belt_name", "Disabled").toString();

    if(this->device() != nullptr)
    {
        foreach(QBluetoothDeviceInfo b, devices)
        {
            if(((b.name().startsWith(heartRateBeltName))) && !heartRateBelt && !heartRateBeltName.startsWith("Disabled"))
            {
                heartRateBelt = new heartratebelt();
                connect(heartRateBelt, SIGNAL(disconnected()), this, SLOT(restart()));
                connect(heartRateBelt, SIGNAL(debug(QString)), this, SLOT(debug(QString)));
                connect(heartRateBelt, SIGNAL(heartRate(uint8_t)), this->device(), SLOT(heartRate(uint8_t)));
                heartRateBelt->deviceDiscovered(b);

                break;
            }
        }
    }
}

void bluetooth::heartRate(u_int8_t heart)
{

}

void bluetooth::restart()
{
    QSettings settings;
    if(settings.value("bluetooth_no_reconnection", false).toBool())
        exit(0);

    devices.clear();

    if(device()->VirtualDevice())
    {
        if(device()->deviceType() == bluetoothdevice::TREADMILL)
            delete (virtualtreadmill*)device()->VirtualDevice();
        else if(device()->deviceType() == bluetoothdevice::BIKE)
            delete (virtualbike*)device()->VirtualDevice();
    }

    if(domyos)
    {        
        delete domyos;        
        domyos = 0;
    }
    if(domyosBike)
    {
        delete domyosBike;
        domyosBike = 0;
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
    if(echelonConnectSport)
    {
        delete echelonConnectSport;
        echelonConnectSport = 0;
    }
    if(heartRateBelt)
    {
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
    else if(toorx)
        return toorx;
    else if(trxappgateusb)
        return trxappgateusb;
    else if(echelonConnectSport)
        return echelonConnectSport;
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
    docTreadmill.setAttribute("Speed", QString::number(device()->currentSpeed(), 'f', 1));
    docTreadmill.setAttribute("Incline", QString::number(((treadmill*)device())->currentInclination(), 'f', 1));
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
