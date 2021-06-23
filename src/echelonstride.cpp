#include "echelonstride.h"
#include "virtualtreadmill.h"
#include "keepawakehelper.h"
#include <QFile>
#include <QDateTime>
#include <QMetaEnum>
#include <QSettings>
#include <QBluetoothLocalDevice>
#include "ios/lockscreen.h"

echelonstride::echelonstride(uint32_t pollDeviceTime, bool noConsole, bool noHeartService, double forceInitSpeed, double forceInitInclination)
{
    m_watt.setType(metric::METRIC_WATT);
    Speed.setType(metric::METRIC_SPEED);
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

void echelonstride::writeCharacteristic(uint8_t* data, uint8_t data_len, QString info, bool disable_log, bool wait_for_response)
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

    if(gattCommunicationChannelService->state() != QLowEnergyService::ServiceState::ServiceDiscovered ||
       m_control->state() == QLowEnergyController::UnconnectedState)
    {
        debug("writeCharacteristic error because the connection is closed");
        return;
    }

    gattCommunicationChannelService->writeCharacteristic(gattWriteCharacteristic, QByteArray((const char*)data, data_len));

    if(!disable_log)
        debug(" >> " + QByteArray((const char*)data, data_len).toHex(' ') + " // " + info);

    loop.exec();

    if(timeout.isActive() == false)
        debug(" exit for timeout");
}

void echelonstride::updateDisplay(uint16_t elapsed)
{

}

void echelonstride::forceSpeedOrIncline(double requestSpeed, double requestIncline)
{

}

bool echelonstride::sendChangeFanSpeed(uint8_t speed)
{
   return false;
}

bool echelonstride::changeFanSpeed(uint8_t speed)
{
   requestFanSpeed = speed;

   return true;
}

void echelonstride::sendPoll()
{
    uint8_t noOpData[] = { 0xf0, 0xa0, 0x01, 0x00, 0x00 };

    noOpData[3] = counterPoll;

    for(uint8_t i=0; i<sizeof(noOpData)-1; i++)
    {
       noOpData[4] += noOpData[i]; // the last byte is a sort of a checksum
    }

    writeCharacteristic(noOpData, sizeof(noOpData), "noOp", false, true);

    counterPoll++;
    if(!counterPoll)
        counterPoll = 1;
}

void echelonstride::update()
{
    if(m_control->state() == QLowEnergyController::UnconnectedState)
    {
        emit disconnected();
        return;
    }

    if(initRequest)
    {
        initRequest = false;
        btinit();
    }
    else if(/*bluetoothDevice.isValid() &&*/
       m_control->state() == QLowEnergyController::DiscoveredState &&
       gattCommunicationChannelService &&
       gattWriteCharacteristic.isValid() &&
       gattNotify1Characteristic.isValid() &&
       gattNotify2Characteristic.isValid() &&
       initDone)
    {
        QSettings settings;
        // ******************************************* virtual treadmill init *************************************
        if(!firstInit && searchStopped && !virtualTreadMill)
        {
            bool virtual_device_enabled = settings.value("virtual_device_enabled", true).toBool();
            if(virtual_device_enabled)
            {
                debug("creating virtual treadmill interface...");
                virtualTreadMill = new virtualtreadmill(this, noHeartService);
                connect(virtualTreadMill,&virtualtreadmill::debug ,this,&echelonstride::debug);
                firstInit = 1;
            }
        }
        // ********************************************************************************************************

        //debug("Domyos Treadmill RSSI " + QString::number(bluetoothDevice.rssi()));

        update_metrics(true, watts(settings.value("weight", 75.0).toFloat()));

        // updating the treadmill console every second
        if(sec1Update++ >= (2000 / refresh->interval()))
        {
            sec1Update = 0;
            sendPoll();
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
               requestStart = -1;
               emit tapeStarted();
            }
            if(requestStop != -1)
            {
                debug("stopping...");
                requestStop = -1;
            }
            if(requestFanSpeed != -1)
            {
                debug("changing fan speed...");
                sendChangeFanSpeed(requestFanSpeed);
                requestFanSpeed = -1;
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
    }
}

void echelonstride::serviceDiscovered(const QBluetoothUuid &gatt)
{
    debug("serviceDiscovered " + gatt.toString());
}

void echelonstride::characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue)
{
    //qDebug() << "characteristicChanged" << characteristic.uuid() << newValue << newValue.length();
    QSettings settings;
    QString heartRateBeltName = settings.value("heart_rate_belt_name", "Disabled").toString();
    Q_UNUSED(characteristic);
    QByteArray value = newValue;

    qDebug() << " << " + newValue.toHex(' ');

    lastPacket = newValue;

    if(((unsigned char)newValue.at(0)) == 0xf0 && ((unsigned char)newValue.at(1)) == 0xd3)
    {
        Speed = (double)((newValue.at(3) << 8) | newValue.at(4)) / 1000.0;
        qDebug() << "Current Speed: " + QString::number(Speed.value());
        return;
    }
    else if(((unsigned char)newValue.at(0)) == 0xf0 && ((unsigned char)newValue.at(1)) == 0xd2)
    {
        Inclination = newValue.at(3);
        qDebug() << "Current Inclination: " + QString::number(Inclination.value());
        return;
    }

    /*if (newValue.length() != 21)
        return;*/

    /*if ((uint8_t)(newValue.at(0)) != 0xf0 && (uint8_t)(newValue.at(1)) != 0xd1)
        return;*/

    if(!firstCharacteristicChanged) {
        KCal += ((( (0.048 * ((double)watts(settings.value("weight", 75.0).toFloat())) + 1.19) * settings.value("weight", 75.0).toFloat() * 3.5) / 200.0 ) / (60000.0 / ((double)lastTimeCharacteristicChanged.msecsTo(QDateTime::currentDateTime())))); //(( (0.048* Output in watts +1.19) * body weight in kg * 3.5) / 200 ) / 60
        DistanceCalculated += ((Speed.value() / 3600.0) / ( 1000.0 / (lastTimeCharacteristicChanged.msecsTo(QDateTime::currentDateTime()))));
    }
    Distance = DistanceCalculated;

#ifdef Q_OS_ANDROID
    if(settings.value("ant_heart", false).toBool())
        Heart = (uint8_t)KeepAwakeHelper::heart();
    else
#endif
    {
        if(heartRateBeltName.startsWith("Disabled"))
        {
#ifdef Q_OS_IOS
#ifndef IO_UNDER_QT
            lockscreen h;
            long appleWatchHeartRate = h.heartRate();
            h.setKcal(KCal.value());
            h.setDistance(Distance.value());
            Heart = appleWatchHeartRate;
            qDebug() << "Current Heart from Apple Watch: " + QString::number(appleWatchHeartRate);
#endif
#endif
        }
    }

    qDebug() << "Current Calculate Distance: " + QString::number(Distance.value());
    qDebug() << "Current Watt: " + QString::number(watts(settings.value("weight", 75.0).toFloat()));

    if(m_control->error() != QLowEnergyController::NoError)
        qDebug() << "QLowEnergyController ERROR!!" << m_control->errorString();

    lastTimeCharacteristicChanged = QDateTime::currentDateTime();
    firstCharacteristicChanged = false;
}

void echelonstride::btinit()
{
    uint8_t initData1[] = { 0xf0, 0xa1, 0x00, 0x91 };
    uint8_t initData2[] = { 0xf0, 0xa3, 0x00, 0x93 };
    uint8_t initData3[] = { 0xf0, 0xb0, 0x01, 0x01, 0xa2 };
    //uint8_t initData4[] = { 0xf0, 0x60, 0x00, 0x50 }; // get sleep command

    // useless i guess
    //writeCharacteristic(initData4, sizeof(initData4), "get sleep", false, true);

    // in the snoof log it repeats this frame 4 times, i will have to analyze the response to understand if 4 times are enough
    writeCharacteristic(initData1, sizeof(initData1), "init", false, true);
    writeCharacteristic(initData1, sizeof(initData1), "init", false, true);
    writeCharacteristic(initData1, sizeof(initData1), "init", false, true);
    writeCharacteristic(initData1, sizeof(initData1), "init", false, true);

    writeCharacteristic(initData2, sizeof(initData2), "init", false, true);
    writeCharacteristic(initData1, sizeof(initData1), "init", false, true);
    writeCharacteristic(initData3, sizeof(initData3), "init", false, true);

    initDone = true;
}

void echelonstride::stateChanged(QLowEnergyService::ServiceState state)
{
    QBluetoothUuid _gattWriteCharacteristicId((QString)"0bf669f2-45f2-11e7-9598-0800200c9a66");
    QBluetoothUuid _gattNotify1CharacteristicId((QString)"0bf669f3-45f2-11e7-9598-0800200c9a66");
    QBluetoothUuid _gattNotify2CharacteristicId((QString)"0bf669f4-45f2-11e7-9598-0800200c9a66");

    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceState>();
    qDebug() << "BTLE stateChanged " + QString::fromLocal8Bit(metaEnum.valueToKey(state));

    if(state == QLowEnergyService::ServiceDiscovered)
    {
        //qDebug() << gattCommunicationChannelService->characteristics();

        gattWriteCharacteristic = gattCommunicationChannelService->characteristic(_gattWriteCharacteristicId);
        gattNotify1Characteristic = gattCommunicationChannelService->characteristic(_gattNotify1CharacteristicId);
        gattNotify2Characteristic = gattCommunicationChannelService->characteristic(_gattNotify2CharacteristicId);
        Q_ASSERT(gattWriteCharacteristic.isValid());
        Q_ASSERT(gattNotify1Characteristic.isValid());
        Q_ASSERT(gattNotify2Characteristic.isValid());

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
        gattCommunicationChannelService->writeDescriptor(gattNotify1Characteristic.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration), descriptor);
        gattCommunicationChannelService->writeDescriptor(gattNotify2Characteristic.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration), descriptor);
    }
}

void echelonstride::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue)
{
    debug("descriptorWritten " + descriptor.name() + " " + newValue.toHex(' '));

    initRequest = true;
    emit connectedAndDiscovered();
}

void echelonstride::characteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue)
{
    Q_UNUSED(characteristic);
    debug("characteristicWritten " + newValue.toHex(' '));
}

void echelonstride::serviceScanDone(void)
{
    qDebug() << "serviceScanDone";

    QBluetoothUuid _gattCommunicationChannelServiceId((QString)"0bf669f1-45f2-11e7-9598-0800200c9a66");

    gattCommunicationChannelService = m_control->createServiceObject(_gattCommunicationChannelServiceId);
    connect(gattCommunicationChannelService, SIGNAL(stateChanged(QLowEnergyService::ServiceState)), this, SLOT(stateChanged(QLowEnergyService::ServiceState)));
    gattCommunicationChannelService->discoverDetails();
}

void echelonstride::errorService(QLowEnergyService::ServiceError err)
{
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    debug("echelonstride::errorService " + QString::fromLocal8Bit(metaEnum.valueToKey(err)) + m_control->errorString());
}

void echelonstride::error(QLowEnergyController::Error err)
{
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    debug("echelonstride::error " + QString::fromLocal8Bit(metaEnum.valueToKey(err)) + m_control->errorString());
}

void echelonstride::deviceDiscovered(const QBluetoothDeviceInfo &device)
{
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

void echelonstride::controllerStateChanged(QLowEnergyController::ControllerState state)
{
    qDebug() << "controllerStateChanged" << state;
    if(state == QLowEnergyController::UnconnectedState && m_control)
    {
        qDebug() << "trying to connect back again...";
        initDone = false;
        m_control->connectToDevice();
    }
}

bool echelonstride::connected()
{
    if(!m_control)
        return false;
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

void* echelonstride::VirtualTreadMill()
{
    return virtualTreadMill;
}

void* echelonstride::VirtualDevice()
{
    return VirtualTreadMill();
}

double echelonstride::odometer()
{
    return DistanceCalculated;
}

void echelonstride::setLastSpeed(double speed)
{
    lastSpeed = speed;
}

void echelonstride::setLastInclination(double inclination)
{
    lastInclination = inclination;
}

void echelonstride::searchingStop()
{
    searchStopped = true;
}
