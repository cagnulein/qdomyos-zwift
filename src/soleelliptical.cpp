#include "soleelliptical.h"
#include "virtualtreadmill.h"
#include "keepawakehelper.h"
#include <QFile>
#include <QDateTime>
#include <QMetaEnum>
#include <QBluetoothLocalDevice>
#include <QSettings>

soleelliptical::soleelliptical(bool noWriteResistance, bool noHeartService, bool testResistance, uint8_t bikeResistanceOffset, double bikeResistanceGain)
{
    m_watt.setType(metric::METRIC_WATT);
    refresh = new QTimer(this);

    this->testResistance = testResistance;
    this->noWriteResistance = noWriteResistance;
    this->noHeartService = noHeartService;
    this->bikeResistanceGain = bikeResistanceGain;
    this->bikeResistanceOffset = bikeResistanceOffset;

    initDone = false;
    connect(refresh, SIGNAL(timeout()), this, SLOT(update()));
    refresh->start(300);
}

soleelliptical::~soleelliptical()
{
    qDebug() << "~soleelliptical()" << virtualTreadmill;
    if(virtualTreadmill)
        delete virtualTreadmill;
}

void soleelliptical::writeCharacteristic(uint8_t* data, uint8_t data_len, QString info, bool disable_log, bool wait_for_response)
{
    QEventLoop loop;
    QTimer timeout;

    if(wait_for_response)
    {
        connect(gattCommunicationChannelService, SIGNAL(characteristicChanged(QLowEnergyCharacteristic,QByteArray)),
                &loop, SLOT(quit()));
        timeout.singleShot(300, &loop, SLOT(quit()));
    }
    else
    {
        connect(gattCommunicationChannelService, SIGNAL(characteristicWritten(QLowEnergyCharacteristic,QByteArray)),
                &loop, SLOT(quit()));
        timeout.singleShot(300, &loop, SLOT(quit()));
    }

    gattCommunicationChannelService->writeCharacteristic(gattWriteCharacteristic, QByteArray((const char*)data, data_len));

    if(!disable_log)
        debug(" >> " + QByteArray((const char*)data, data_len).toHex(' ') + " // " + info);

    loop.exec();

    if(timeout.isActive() == false)
        debug(" exit for timeout");
}

void soleelliptical::forceResistanceAndInclination(int8_t requestResistance, uint8_t inclination)
{
    // TODO do the level down command
   uint8_t write[] = {0x5b, 0x04, 0x00, 0x13, 0x4f, 0x4b, 0x5d};

   writeCharacteristic(write, 20, "forceResistance " + QString::number(requestResistance) + " Inclination " + inclination);
}

void soleelliptical::update()
{
    uint8_t noOpData[] = { 0x5b, 0x04, 0x00, 0x10, 0x4f, 0x4b, 0x5d };
    uint8_t noOpData1[] = { 0x5b, 0x04, 0x00, 0x06, 0x4f, 0x4b, 0x5d };

    // stop tape
    uint8_t initDataF0C800B8[] = { 0xf0, 0xc8, 0x00, 0xb8 };

    if(m_control->state() == QLowEnergyController::UnconnectedState)
    {
        emit disconnected();
        return;
    }

    if(initRequest)
    {
        initRequest = false;
        btinit(false);
    }
    else if(bluetoothDevice.isValid() &&
       m_control->state() == QLowEnergyController::DiscoveredState &&
       gattCommunicationChannelService &&
       gattWriteCharacteristic.isValid() &&
       gattNotifyCharacteristic.isValid() &&
       initDone)
    {
        QDateTime current = QDateTime::currentDateTime();
        double deltaTime = (((double)lastTimeUpdate.msecsTo(current)) / ((double)1000.0));
        if(currentSpeed().value() > 0.0 && !firstUpdate && !paused)
        {
           elapsed += deltaTime;
           m_watt = (double)watts();
           m_jouls += (m_watt.value() * deltaTime);
        }
        lastTimeUpdate = current;

        // ******************************************* virtual bike init *************************************
        if(!firstVirtual && searchStopped && !virtualTreadmill)
        {
            QSettings settings;
            bool virtual_device_enabled = settings.value("virtual_device_enabled", true).toBool();
            if(virtual_device_enabled)
            {
                debug("creating virtual treadmill interface...");
                virtualTreadmill = new virtualtreadmill(this, noHeartService);
                connect(virtualTreadmill,&virtualtreadmill::debug ,this,&soleelliptical::debug);
                firstVirtual = 1;
            }
        }
        // ********************************************************************************************************

        // updating the treadmill console every second
        if(sec1Update++ == (1000 / refresh->interval()))
        {
            sec1Update = 0;
        }
        else
        {
            switch(counterPoll)
            {
            case 0:
                writeCharacteristic(noOpData, sizeof(noOpData), "noOp", false, true);
                break;
            case 1:
                writeCharacteristic(noOpData, sizeof(noOpData1), "noOp", false, true);
                break;
            }
            counterPoll++;
            if(counterPoll > 1)
                counterPoll = 0;
        }

        if(testResistance)
        {
            if((((int)elapsed.value()) % 5) == 0)
            {
                uint8_t new_res = currentResistance() + 1;
                if(new_res > 15)
                    new_res = 1;
                forceResistanceAndInclination(new_res, currentInclination().value());
            }
        }

        if(requestResistance != -1)
        {
           if(requestResistance > 20) requestResistance = 20;
           else if(requestResistance == 0) requestResistance = 1;

           if(requestResistance != currentResistance())
           {
              debug("writing resistance " + QString::number(requestResistance));
              forceResistanceAndInclination(requestResistance, currentInclination().value());
           }
           requestResistance = -1;
        }
        else if(requestInclination != -1)
        {
           if(requestInclination > 15) requestInclination = 15;
           else if(requestInclination == 0) requestInclination = 1;

           if(requestInclination != currentInclination().value())
           {
              debug("writing inclination " + QString::number(requestInclination));
              forceResistanceAndInclination(currentResistance(), requestInclination);
           }
           requestInclination = -1;
        }
        if(requestStart != -1)
        {
           debug("starting...");

           btinit(true);

           requestStart = -1;
           emit bikeStarted();
        }
        if(requestStop != -1)
        {
            debug("stopping...");
            //writeCharacteristic(initDataF0C800B8, sizeof(initDataF0C800B8), "stop tape");
            requestStop = -1;
        }

        elevationAcc += (currentSpeed().value() / 3600.0) * 1000.0 * (currentInclination().value() / 100.0) * deltaTime;
    }

    firstUpdate = false;
}

void soleelliptical::serviceDiscovered(const QBluetoothUuid &gatt)
{
    debug("serviceDiscovered " + gatt.toString());
}

void soleelliptical::characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue)
{
    //qDebug() << "characteristicChanged" << characteristic.uuid() << newValue << newValue.length();
    Q_UNUSED(characteristic);
    QSettings settings;
    QString heartRateBeltName = settings.value("heart_rate_belt_name", "Disabled").toString();

    debug(" << " + newValue.toHex(' '));

    lastPacket = newValue;
    if (newValue.length() < 20)
        return;

    double speed = GetSpeedFromPacket(newValue) * settings.value("domyos_elliptical_speed_ratio", 1.0).toDouble();
    double kcal = GetKcalFromPacket(newValue);
    //double distance = GetDistanceFromPacket(newValue) * settings.value("domyos_elliptical_speed_ratio", 1.0).toDouble();
    uint16_t watt = (newValue.at(13) << 8) | newValue.at(14);

    Cadence = ((uint8_t)newValue.at(10));
    m_watt = watt;

    //Resistance = newValue.at(14);
    //Inclination = newValue.at(21);
    if(Resistance < 1)
    {
        debug("invalid resistance value " + QString::number(Resistance) + " putting to default");
        Resistance = 1;
    }

#ifdef Q_OS_ANDROID
    if(settings.value("ant_heart", false).toBool())
        Heart = (uint8_t)KeepAwakeHelper::heart();
    else
#endif
    {
        if(heartRateBeltName.startsWith("Disabled"))
            Heart = ((uint8_t)newValue.at(18));
    }

    Distance += ((Speed.value() / 3600000.0) * ((double)lastRefreshCharacteristicChanged.msecsTo(QDateTime::currentDateTime())) );

    CrankRevs++;
    LastCrankEventTime += (uint16_t)(1024.0 / (((double)(Cadence)) / 60.0));
    lastRefreshCharacteristicChanged = QDateTime::currentDateTime();

    debug("Current speed: " + QString::number(speed));
    debug("Current cadence: " + QString::number(Cadence));
    debug("Current resistance: " + QString::number(Resistance));
    debug("Current inclination: " + QString::number(Inclination.value()));
    debug("Current heart: " + QString::number(Heart.value()));
    debug("Current KCal: " + QString::number(kcal));
    debug("Current Distance: " + QString::number(Distance.value()));
    debug("Current CrankRevs: " + QString::number(CrankRevs));
    debug("Last CrankEventTime: " + QString::number(LastCrankEventTime));
    debug("Current Watt: " + QString::number(watts()));

    if(m_control->error() != QLowEnergyController::NoError)
        qDebug() << "QLowEnergyController ERROR!!" << m_control->errorString();

    Speed = speed;
    KCal = kcal;
}

double soleelliptical::GetSpeedFromPacket(QByteArray packet)
{
    uint16_t convertedData = (packet.at(11) << 8) | packet.at(12);
    double data = (double)convertedData / 100.0f;
    return data;
}

double soleelliptical::GetKcalFromPacket(QByteArray packet)
{
    uint16_t convertedData = (packet.at(7) << 8) | ((uint8_t)packet.at(8));
    return (double)convertedData / 10.0;
}

double soleelliptical::GetDistanceFromPacket(QByteArray packet)
{
    uint16_t convertedData = (packet.at(12) << 8) | packet.at(13);
    double data = ((double)convertedData) / 10.0f;
    return data;
}

void soleelliptical::btinit(bool startTape)
{
    Q_UNUSED(startTape)

    // set speed and incline to 0
    uint8_t initData1[] = { 0x5b, 0x01, 0xf0, 0x5d };
    uint8_t initData2[] = { 0x5b, 0x02, 0x03, 0x01, 0x5d };
    uint8_t initData3[] = { 0x5b, 0x06, 0x07, 0x01, 0x23, 0x00, 0x9b, 0xaa, 0x5d };
    uint8_t initData4[] = { 0x5b, 0x03, 0x08, 0x10, 0x01, 0x5d };
    uint8_t initData5[] = { 0x5b, 0x05, 0x04, 0x0a, 0x00, 0x00, 0x00, 0x5d };
    uint8_t initData6[] = { 0x5b, 0x02, 0x02, 0x02, 0x5d };
    uint8_t initData7[] = { 0x5b, 0x02, 0x03, 0x04, 0x5d };

    writeCharacteristic(initData1, sizeof(initData1), "init");
    writeCharacteristic(initData1, sizeof(initData1), "init");
    writeCharacteristic(initData1, sizeof(initData1), "init");
    writeCharacteristic(initData2, sizeof(initData2), "init");
    writeCharacteristic(initData2, sizeof(initData2), "init");
    writeCharacteristic(initData3, sizeof(initData3), "init");
    writeCharacteristic(initData4, sizeof(initData4), "init");
    writeCharacteristic(initData5, sizeof(initData5), "init");
    writeCharacteristic(initData6, sizeof(initData6), "init");
    writeCharacteristic(initData7, sizeof(initData7), "init");

    initDone = true;
}


void soleelliptical::stateChanged(QLowEnergyService::ServiceState state)
{
    QBluetoothUuid _gattWriteCharacteristicId((QString)"49535343-8841-43f4-a8d4-ecbe34729bb3");
    QBluetoothUuid _gattNotifyCharacteristicId((QString)"49535343-1e4d-4bd9-ba61-23c647249616");

    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceState>();
    debug("BTLE stateChanged " + QString::fromLocal8Bit(metaEnum.valueToKey(state)));

    if(state == QLowEnergyService::ServiceDiscovered)
    {
        //qDebug() << gattCommunicationChannelService->characteristics();

        gattWriteCharacteristic = gattCommunicationChannelService->characteristic(_gattWriteCharacteristicId);
        gattNotifyCharacteristic = gattCommunicationChannelService->characteristic(_gattNotifyCharacteristicId);
        Q_ASSERT(gattWriteCharacteristic.isValid());
        Q_ASSERT(gattNotifyCharacteristic.isValid());

        // establish hook into notifications
        connect(gattCommunicationChannelService, SIGNAL(characteristicChanged(QLowEnergyCharacteristic,QByteArray)),
                this, SLOT(characteristicChanged(QLowEnergyCharacteristic,QByteArray)));
        connect(gattCommunicationChannelService, SIGNAL(characteristicWritten(const QLowEnergyCharacteristic, const QByteArray)),
                this, SLOT(characteristicWritten(const QLowEnergyCharacteristic, const QByteArray)));
        connect(gattCommunicationChannelService, SIGNAL(error(QLowEnergyService::ServiceError)),
                this, SLOT(errorService(QLowEnergyService::ServiceError)));
        connect(gattCommunicationChannelService, SIGNAL(descriptorWritten(const QLowEnergyDescriptor, const QByteArray)), this,
                SLOT(descriptorWritten(const QLowEnergyDescriptor, const QByteArray)));

        QByteArray descriptor;
        descriptor.append((char)0x01);
        descriptor.append((char)0x00);
        gattCommunicationChannelService->writeDescriptor(gattNotifyCharacteristic.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration), descriptor);
    }
}

void soleelliptical::searchingStop()
{
    searchStopped = true;
}

void soleelliptical::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue)
{
    debug("descriptorWritten " + descriptor.name() + " " + newValue.toHex(' '));

    initRequest = true;
    emit connectedAndDiscovered();
}

void soleelliptical::characteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue)
{
    Q_UNUSED(characteristic);
    debug("characteristicWritten " + newValue.toHex(' '));
}

void soleelliptical::serviceScanDone(void)
{
    debug("serviceScanDone");

    QBluetoothUuid _gattCommunicationChannelServiceId((QString)"49535343-fe7d-4ae5-8fa9-9fafd205e455");

    gattCommunicationChannelService = m_control->createServiceObject(_gattCommunicationChannelServiceId);
    connect(gattCommunicationChannelService, SIGNAL(stateChanged(QLowEnergyService::ServiceState)), this, SLOT(stateChanged(QLowEnergyService::ServiceState)));
    gattCommunicationChannelService->discoverDetails();
}

void soleelliptical::errorService(QLowEnergyService::ServiceError err)
{
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    debug("soleelliptical::errorService" + QString::fromLocal8Bit(metaEnum.valueToKey(err)) + m_control->errorString());
}

void soleelliptical::error(QLowEnergyController::Error err)
{
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    debug("soleelliptical::error" + QString::fromLocal8Bit(metaEnum.valueToKey(err)) + m_control->errorString());
}

void soleelliptical::deviceDiscovered(const QBluetoothDeviceInfo &device)
{
    debug("Found new device: " + device.name() + " (" + device.address().toString() + ')');
    {
        bluetoothDevice = device;

        m_control = QLowEnergyController::createCentral(bluetoothDevice, this);
        connect(m_control, SIGNAL(serviceDiscovered(const QBluetoothUuid &)),
                this, SLOT(serviceDiscovered(const QBluetoothUuid &)));
        connect(m_control, SIGNAL(discoveryFinished()),
                this, SLOT(serviceScanDone()));
        connect(m_control, SIGNAL(error(QLowEnergyController::Error)),
                this, SLOT(error(QLowEnergyController::Error)));
        connect(m_control, SIGNAL(stateChanged(QLowEnergyController::ControllerState)), this, SLOT(controllerStateChanged(QLowEnergyController::ControllerState)));

        connect(m_control, static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error),
                this, [this](QLowEnergyController::Error error) {
            Q_UNUSED(error);
            Q_UNUSED(this);
            debug("Cannot connect to remote device.");
            searchStopped = false;
            emit disconnected();
        });
        connect(m_control, &QLowEnergyController::connected, this, [this]() {
            Q_UNUSED(this);
            debug("Controller connected. Search services...");
            m_control->discoverServices();
        });
        connect(m_control, &QLowEnergyController::disconnected, this, [this]() {
            Q_UNUSED(this);
            debug("LowEnergy controller disconnected");
            searchStopped = false;
            emit disconnected();
        });

        // Connect
        m_control->connectToDevice();
        return;
    }
}

bool soleelliptical::connected()
{
    if(!m_control)
        return false;
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

void* soleelliptical::VirtualTreadmill()
{
    return virtualTreadmill;
}

void* soleelliptical::VirtualDevice()
{
    return VirtualTreadmill();
}

uint16_t soleelliptical::watts()
{
    return m_watt.value();
}

void soleelliptical::controllerStateChanged(QLowEnergyController::ControllerState state)
{
    qDebug() << "controllerStateChanged" << state;
    if(state == QLowEnergyController::UnconnectedState && m_control)
    {
        qDebug() << "trying to connect back again...";
        initDone = false;
        m_control->connectToDevice();
    }
}
