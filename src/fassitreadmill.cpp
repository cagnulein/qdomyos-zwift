#include "fassitreadmill.h"
#include "virtualtreadmill.h"
#include <QFile>
#include <QDateTime>
#include <QMetaEnum>
#include <QSettings>
#include <QBluetoothLocalDevice>

fassitreadmill::fassitreadmill(uint32_t pollDeviceTime, bool noConsole, bool noHeartService, double forceInitSpeed, double forceInitInclination)
{
    this->noConsole = noConsole;
    this->noHeartService = noHeartService;

    if(forceInitSpeed > 0)
        lastSpeed = forceInitSpeed;

    if(forceInitInclination > 0)
        lastInclination = forceInitInclination;

    refresh = new QTimer(this);
    initDone = false;
    connect(refresh, SIGNAL(timeout()), this, SLOT(update()));
    refresh->start(pollDeviceTime);
}

void fassitreadmill::writeCharacteristic(uint8_t* data, uint8_t data_len, QString info, bool disable_log, bool wait_for_response)
{
    QEventLoop loop;
    QTimer timeout;

    if(wait_for_response)
    {
        connect(this, SIGNAL(packetReceived()),
                &loop, SLOT(quit()));
        timeout.singleShot(300, &loop, SLOT(quit()));
    }
    else
    {
        connect(gattCommunicationChannelService, SIGNAL(characteristicWritten(QLowEnergyCharacteristic,QByteArray)),
                &loop, SLOT(quit()));
        timeout.singleShot(300, &loop, SLOT(quit()));
    }

    gattCommunicationChannelService->writeCharacteristic(gattWriteCharacteristic, QByteArray::fromRawData((const char*)data, data_len));

    if(!disable_log)
        debug(" >> " + QByteArray((const char*)data, data_len).toHex(' ') + " // " + info);

    loop.exec();

    if(timeout.isActive() == false)
        debug(" exit for timeout");
}

void fassitreadmill::updateDisplay(uint16_t elapsed)
{
    Q_UNUSED(elapsed)
}

void fassitreadmill::forceSpeedOrIncline(double requestSpeed, double requestIncline)
{
    Q_UNUSED(requestSpeed)
    Q_UNUSED(requestIncline)
}

bool fassitreadmill::sendChangeFanSpeed(uint8_t speed)
{
    Q_UNUSED(speed)
    return false;
}

bool fassitreadmill::changeFanSpeed(uint8_t speed)
{
   if(speed > FanSpeed)
       requestIncreaseFan = 1;
   else if(speed < FanSpeed)
       requestDecreaseFan = 1;

   return true;
}


void fassitreadmill::update()
{
    if(m_control->state() == QLowEnergyController::UnconnectedState)
    {
        emit disconnected();
        return;
    }

    if(initRequest)
    {
        initRequest = false;
        btinit((lastSpeed > 0 ? true : false));
    }
    else if(bluetoothDevice.isValid() &&
       m_control->state() == QLowEnergyController::DiscoveredState &&
       gattCommunicationChannelService &&
       gattWriteCharacteristic.isValid() &&
       gattNotifyCharacteristic.isValid() &&
       initDone)
    {
        // ******************************************* virtual treadmill init *************************************
        if(!firstInit && searchStopped && !virtualTreadMill)
        {
            QSettings settings;
            bool virtual_device_enabled = settings.value("virtual_device_enabled", true).toBool();
            if(virtual_device_enabled)
            {
                debug("creating virtual treadmill interface...");
                virtualTreadMill = new virtualtreadmill(this, noHeartService);
                connect(virtualTreadMill,&virtualtreadmill::debug ,this,&fassitreadmill::debug);
                firstInit = 1;
            }
        }
        // ********************************************************************************************************

        debug("fassi Treadmill RSSI " + QString::number(bluetoothDevice.rssi()));

        QDateTime current = QDateTime::currentDateTime();
        double deltaTime = (((double)lastTimeUpdate.msecsTo(current)) / ((double)1000.0));
        if(currentSpeed().value() > 0.0 && !firstUpdate)
        {
            QSettings settings;
           elapsed += deltaTime;
           m_watt = (double)watts(settings.value("weight", 75.0).toFloat());
           m_jouls += (m_watt.value() * deltaTime);
        }
        lastTimeUpdate = current;

        // updating the treadmill console every second
        if(sec1Update++ >= (1000 / refresh->interval()))
        {
            if(incompletePackets == false && noConsole == false)
            {
                sec1Update = 0;
                updateDisplay(elapsed);
            }
        }
        else
        {
            uint8_t noOpData[] = { 0x02, 0x51, 0x51, 0x03 };
            if(incompletePackets == false)
                writeCharacteristic(noOpData, sizeof(noOpData), "noOp", false, true);
        }

        // byte 3 - 4 = elapsed time
        // byte 17    = inclination
        if(incompletePackets == false)
        {
            if(requestSpeed != -1)
            {
               if(requestSpeed != currentSpeed().value() && requestSpeed >= 0 && requestSpeed <= 22)
               {
                  debug("writing speed " + QString::number(requestSpeed));
                  double inc = Inclination.value();
                  if(requestInclination != -1)
                  {
                      inc = requestInclination;
                      requestInclination = -1;
                  }
                  forceSpeedOrIncline(requestSpeed, inc);
               }
               requestSpeed = -1;
            }
            if(requestInclination != -1)
            {
               if(requestInclination != currentInclination().value() && requestInclination >= 0 && requestInclination <= 15)
               {
                  debug("writing incline " + QString::number(requestInclination));
                  double speed = currentSpeed().value();
                  if(requestSpeed != -1)
                  {
                      speed = requestSpeed;
                      requestSpeed = -1;
                  }
                  forceSpeedOrIncline(speed, requestInclination);
               }
               requestInclination = -1;
            }
            if(requestStart != -1)
            {
               debug("starting...");
               if(lastSpeed == 0.0)
                   lastSpeed = 0.5;
               btinit(true);
               requestStart = -1;
               emit tapeStarted();
            }
            if(requestStop != -1)
            {
                uint8_t stopTape[] = { 0x02, 0x53, 0x03, 0x50, 0x03 }; // to verify
                debug("stopping...");
                writeCharacteristic(stopTape, sizeof(stopTape), "stop tape", false, true);
                requestStop = -1;
            }
            if(requestIncreaseFan != -1)
            {
                debug("increasing fan speed...");
                sendChangeFanSpeed(FanSpeed + 1);
                requestIncreaseFan = -1;
            }
            else if(requestDecreaseFan != -1)
            {
                debug("decreasing fan speed...");
                sendChangeFanSpeed(FanSpeed - 1);
                requestDecreaseFan = -1;
            }
        }

        elevationAcc += (currentSpeed().value() / 3600.0) * 1000.0 * (currentInclination().value() / 100.0) * deltaTime;
    }

    firstUpdate = false;
}

void fassitreadmill::serviceDiscovered(const QBluetoothUuid &gatt)
{
    debug("serviceDiscovered " + gatt.toString());
}

void fassitreadmill::characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue)
{
    //qDebug() << "characteristicChanged" << characteristic.uuid() << newValue << newValue.length();
    QSettings settings;
    QString heartRateBeltName = settings.value("heart_rate_belt_name", "Disabled").toString();
    Q_UNUSED(characteristic);
    QByteArray value = newValue;

    debug(" << " + QString::number(value.length()) + " " + value.toHex(' '));

    debug("packetReceived!");
    emit packetReceived();

    lastPacket = value;

    if (value.length() != 17)
    {
        debug("packet ignored");
        return;
    }

    uint16_t seconds_elapsed = (value.at(5) << 8) | value.at(6);
    double speed = GetSpeedFromPacket(value);
    double incline = GetInclinationFromPacket(value);
    double distance = GetDistanceFromPacket(value);

    if(incline > 15.0 || incline < 0.0)
    {
        qDebug() << "inclination out of range, resetting it to 0..." << incline;
        incline = 0;
    }

    if(!firstCharacteristicChanged)
        DistanceCalculated += ((speed / 3600.0) / ( 1000.0 / (lastTimeCharacteristicChanged.msecsTo(QDateTime::currentDateTime()))));

    debug("Current elapsed from treadmill: " + QString::number(seconds_elapsed));
    debug("Current speed: " + QString::number(speed));
    debug("Current incline: " + QString::number(incline));
    debug("Current heart: " + QString::number(Heart.value()));
    debug("Current Distance: " + QString::number(distance));
    debug("Current Distance Calculated: " + QString::number(DistanceCalculated));

    if(m_control->error() != QLowEnergyController::NoError)
        qDebug() << "QLowEnergyController ERROR!!" << m_control->errorString();

    if(Speed.value() != speed)
    {
        Speed = speed;
        emit speedChanged(speed);
    }
    if(Inclination.value() != incline)
    {
        Inclination = incline;
        emit inclinationChanged(incline);
    }

    Distance = distance;

    if(speed > 0)
    {
        lastSpeed = speed;
        lastInclination = incline;
    }

    lastTimeCharacteristicChanged = QDateTime::currentDateTime();
    firstCharacteristicChanged = false;
}

double fassitreadmill::GetSpeedFromPacket(QByteArray packet)
{
    uint8_t convertedData = (uint8_t)packet.at(3);
    double data = (double)convertedData / 10.0f;
    return data;
}

double fassitreadmill::GetDistanceFromPacket(QByteArray packet)
{
    uint16_t convertedData = (packet.at(9) << 8) | packet.at(10);
    double data = ((double)convertedData) / 10.0f;
    return data;
}

double fassitreadmill::GetInclinationFromPacket(QByteArray packet)
{
    uint16_t convertedData = packet.at(4);
    double data = convertedData;
    return data;
}

void fassitreadmill::btinit(bool startTape)
{
    uint8_t initDataStart[] = { 0x02, 0x50, 0x05, 0x55, 0x03};

    uint8_t startTape1[] = { 0x02, 0x53, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x52, 0x03 }; // to verify
    uint8_t startTape2[] = { 0x02, 0x53, 0x00, 0xaa, 0x13, 0x6e, 0x00, 0x46, 0xaa, 0x68, 0x03 }; // to verify

    writeCharacteristic(initDataStart, sizeof(initDataStart), "init", false, true);
    if(startTape)
    {
        writeCharacteristic(startTape1, sizeof(startTape1), "init", false, true);
        writeCharacteristic(startTape2, sizeof(startTape2), "init", false, true);
        forceSpeedOrIncline(lastSpeed, lastInclination);
    }

    initDone = true;
}

void fassitreadmill::stateChanged(QLowEnergyService::ServiceState state)
{
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceState>();
    debug("BTLE stateChanged " + QString::fromLocal8Bit(metaEnum.valueToKey(state)));
    if(state == QLowEnergyService::ServiceDiscovered)
    {
        foreach(QLowEnergyCharacteristic c, gattCommunicationChannelService->characteristics())
        {
            qDebug() << c.uuid();
            foreach(QLowEnergyDescriptor d, c.descriptors())
                qDebug() << d.uuid();
        }

        QBluetoothUuid _gattWriteCharacteristicId((quint16)0xffe1);
        QBluetoothUuid _gattNotifyCharacteristicId((quint16)0xffe4);
        gattWriteCharacteristic = gattCommunicationChannelService->characteristic(_gattWriteCharacteristicId);
        gattNotifyCharacteristic = gattCommunicationChannelService->characteristic(_gattNotifyCharacteristicId);
        if(!gattWriteCharacteristic.isValid())
        {
            qDebug() << "gattWriteCharacteristic not valid";
            return;
        }
        if(!gattNotifyCharacteristic.isValid())
        {
            qDebug() << "gattNotifyCharacteristic not valid";
            return;
        }

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

void fassitreadmill::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue)
{
    debug("descriptorWritten " + descriptor.name() + " " + newValue.toHex(' '));

    initRequest = true;
    emit connectedAndDiscovered();
}

void fassitreadmill::characteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue)
{
    Q_UNUSED(characteristic);
    debug("characteristicWritten " + newValue.toHex(' '));
}

void fassitreadmill::serviceScanDone(void)
{
    QBluetoothUuid _gattCommunicationChannelServiceId((QString)"0000ffe0-0000-1000-8000-00805f9b34fb");
    debug("serviceScanDone");

    gattCommunicationChannelService = m_control->createServiceObject(_gattCommunicationChannelServiceId);
    connect(gattCommunicationChannelService, SIGNAL(stateChanged(QLowEnergyService::ServiceState)), this, SLOT(stateChanged(QLowEnergyService::ServiceState)));
    gattCommunicationChannelService->discoverDetails();
}

void fassitreadmill::errorService(QLowEnergyService::ServiceError err)
{
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    debug("fassitreadmill::errorService " + QString::fromLocal8Bit(metaEnum.valueToKey(err)) + m_control->errorString());
}

void fassitreadmill::error(QLowEnergyController::Error err)
{
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    debug("fassitreadmill::error " + QString::fromLocal8Bit(metaEnum.valueToKey(err)) + m_control->errorString());
}

void fassitreadmill::deviceDiscovered(const QBluetoothDeviceInfo &device)
{
    debug("Found new device: " + device.name() + " (" + device.address().toString() + ')');
    if(device.name().startsWith("SW") && device.name().length() == 14)
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

bool fassitreadmill::connected()
{
    if(!m_control)
        return false;
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

void* fassitreadmill::VirtualTreadMill()
{
    return virtualTreadMill;
}

void* fassitreadmill::VirtualDevice()
{
    return VirtualTreadMill();
}

double fassitreadmill::odometer()
{
    return DistanceCalculated;
}

void fassitreadmill::setLastSpeed(double speed)
{
    lastSpeed = speed;
}

void fassitreadmill::setLastInclination(double inclination)
{
    lastInclination = inclination;
}

void fassitreadmill::searchingStop()
{
    searchStopped = true;
}

void fassitreadmill::controllerStateChanged(QLowEnergyController::ControllerState state)
{
    qDebug() << "controllerStateChanged" << state;
    if(state == QLowEnergyController::UnconnectedState && m_control)
    {
        qDebug() << "trying to connect back again...";
        initDone = false;
        m_control->connectToDevice();
    }
}
