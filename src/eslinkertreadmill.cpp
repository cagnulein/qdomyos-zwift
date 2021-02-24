#include "eslinkertreadmill.h"
#include "virtualtreadmill.h"
#include "keepawakehelper.h"
#include <QFile>
#include <QDateTime>
#include <QMetaEnum>
#include <QSettings>
#include <QBluetoothLocalDevice>

eslinkertreadmill::eslinkertreadmill(uint32_t pollDeviceTime, bool noConsole, bool noHeartService, double forceInitSpeed, double forceInitInclination)
{
    m_watt.setType(metric::METRIC_WATT);
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

void eslinkertreadmill::writeCharacteristic(uint8_t* data, uint8_t data_len, QString info, bool disable_log, bool wait_for_response)
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

    gattCommunicationChannelService->writeCharacteristic(gattWriteCharacteristic, QByteArray::fromRawData((const char*)data, data_len), QLowEnergyService::WriteWithoutResponse);

    if(!disable_log)
        debug(" >> " + QByteArray((const char*)data, data_len).toHex(' ') + " // " + info);

    loop.exec();

    if(timeout.isActive() == false)
        debug(" exit for timeout");
}

void eslinkertreadmill::updateDisplay(uint16_t elapsed)
{
   uint8_t display[] = { 0xa9, 0xa0, 0x03, 0x02, 0x00, 0x00, 0x00 };

   display[4] = (uint8_t)((Speed.value() * 10));

   /*for(uint8_t i=0; i<sizeof(display)-1; i++)
   {
      display[6] += display[i]; // the last byte is a sort of a checksum
   }*/
   display[6] = display[4] + 0x08; // the last byte is a sort of a checksum

   writeCharacteristic(display, sizeof(display), "updateDisplay elapsed=" + QString::number(elapsed), false, false );
}

/*
void eslinkertreadmill::forceSpeed(double requestSpeed)
{
   uint8_t writeIncline[] = { 0xa9, 0x01, 0x01, 0x00, 0x00 };

   writeIncline[3] = ((uint8_t)(requestSpeed*10));

   for(uint8_t i=0; i<sizeof(writeIncline)-1; i++)
   {
      writeIncline[4] += writeIncline[i]; // the last byte is a sort of a checksum
   }

   writeCharacteristic(writeIncline, sizeof(writeIncline), "forceSpeed speed=" + QString::number(requestSpeed), false, false);
}
*/

void eslinkertreadmill::update()
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
                connect(virtualTreadMill,&virtualtreadmill::debug ,this,&eslinkertreadmill::debug);
                firstInit = 1;
            }
        }
        // ********************************************************************************************************

        debug("Domyos Treadmill RSSI " + QString::number(bluetoothDevice.rssi()));

        QDateTime current = QDateTime::currentDateTime();
        double deltaTime = (((double)lastTimeUpdate.msecsTo(current)) / ((double)1000.0));
        if(currentSpeed().value() > 0.0 && !firstUpdate && !paused)
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
            updateDisplay(elapsed.value());
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
                  //forceSpeedOrIncline(requestSpeed, inc);
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
                  //forceSpeedOrIncline(speed, requestInclination);
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
                debug("stopping...");
                //writeCharacteristic(initDataF0C800B8, sizeof(initDataF0C800B8), "stop tape", false, true);
                requestStop = -1;
            }
        }

        elevationAcc += (currentSpeed().value() / 3600.0) * 1000.0 * (currentInclination().value() / 100.0) * deltaTime;
    }

    firstUpdate = false;
}

void eslinkertreadmill::serviceDiscovered(const QBluetoothUuid &gatt)
{
    debug("serviceDiscovered " + gatt.toString());
}

void eslinkertreadmill::characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue)
{
    //qDebug() << "characteristicChanged" << characteristic.uuid() << newValue << newValue.length();
    QSettings settings;
    QString heartRateBeltName = settings.value("heart_rate_belt_name", "Disabled").toString();
    Q_UNUSED(characteristic);
    QByteArray value = newValue;

    debug(" << " + QString::number(value.length()) + " " + value.toHex(' '));

    emit packetReceived();

    if(newValue.length() != 17) return;

    double speed = GetSpeedFromPacket(value);
    double incline = GetInclinationFromPacket(value);
    double kcal = GetKcalFromPacket(value);
    //double distance = GetDistanceFromPacket(value);

#ifdef Q_OS_ANDROID
    if(settings.value("ant_heart", false).toBool())
        Heart = (uint8_t)KeepAwakeHelper::heart();
    else
#endif
    {
        /*if(heartRateBeltName.startsWith("Disabled"))
            Heart = value.at(18);*/
    }

    if(!firstCharacteristicChanged)
        DistanceCalculated += ((speed / 3600.0) / ( 1000.0 / (lastTimeCharacteristicChanged.msecsTo(QDateTime::currentDateTime()))));

    debug("Current speed: " + QString::number(speed));
    debug("Current incline: " + QString::number(incline));
    debug("Current heart: " + QString::number(Heart.value()));
    debug("Current KCal: " + QString::number(kcal));
    //debug("Current Distance: " + QString::number(distance));
    debug("Current Distance Calculated: " + QString::number(DistanceCalculated));

    if(m_control->error() != QLowEnergyController::NoError)
        qDebug() << "QLowEnergyController ERROR!!" << m_control->errorString();

    if(Speed.value() != speed)
    {
        emit speedChanged(speed);
    }
    Speed = speed;
    if(Inclination.value() != incline)
    {
        emit inclinationChanged(incline);
    }
    Inclination = incline;

    KCal = kcal;
    //Distance = distance;

    if(speed > 0)
    {
        lastSpeed = speed;
        lastInclination = incline;
    }

    lastTimeCharacteristicChanged = QDateTime::currentDateTime();
    firstCharacteristicChanged = false;
}

double eslinkertreadmill::GetSpeedFromPacket(QByteArray packet)
{
    uint8_t convertedData = (uint8_t)packet.at(14);
    double data = (double)convertedData / 10.0f;
    return data;
}

double eslinkertreadmill::GetKcalFromPacket(QByteArray packet)
{
    uint16_t convertedData = (packet.at(7) << 8) | packet.at(8);
    return (double)convertedData;
}

double eslinkertreadmill::GetDistanceFromPacket(QByteArray packet)
{
    uint16_t convertedData = (packet.at(12) << 8) | packet.at(13);
    double data = ((double)convertedData) / 10.0f;
    return data;
}

double eslinkertreadmill::GetInclinationFromPacket(QByteArray packet)
{
    uint16_t convertedData = packet.at(11);
    double data = convertedData;

    return data;
}

void eslinkertreadmill::btinit(bool startTape)
{
    // set speed and incline to 0
    uint8_t initData1[] = { 0x08, 0x01, 0x86 };
    uint8_t initData2[] = { 0xa9, 0x08, 0x01, 0x86, 0x26 };
    uint8_t initData3[] = { 0xa9, 0x80, 0x05, 0x05, 0xb0, 0x04, 0x52, 0xa9, 0x66 };
    uint8_t initData4[] = { 0xa9, 0x08, 0x04, 0xb2, 0x51, 0x03, 0x52, 0x17 };
    uint8_t initData5[] = { 0xa9, 0x1e, 0x01, 0xfe, 0x48 };
    uint8_t initData6[] = { 0xa9, 0x0a, 0x01, 0x01, 0xa3 };
    uint8_t initData7[] = { 0xa9, 0xf0, 0x01, 0x01, 0x59 };
    uint8_t initData8[] = { 0xa9, 0xa0, 0x03, 0xff, 0x00, 0x00, 0xf5 };
    uint8_t initData9[] = { 0xa9, 0xa0, 0x03, 0x00, 0x00, 0x00, 0x0a };
    uint8_t initData10[] = { 0xa9, 0xa0, 0x03, 0x01, 0x00, 0x00, 0x0b };
    uint8_t initData11[] = { 0xa9, 0x01, 0x01, 0x08, 0xa1 };
    uint8_t initData12[] = { 0xa9, 0xa0, 0x03, 0x02, 0x08, 0x00, 0x00 };

    writeCharacteristic(initData1, sizeof(initData1), "init", false, false);
    writeCharacteristic(initData2, sizeof(initData2), "init", false, true);
    writeCharacteristic(initData3, sizeof(initData3), "init", false, true);
    writeCharacteristic(initData4, sizeof(initData4), "init", false, true);
    writeCharacteristic(initData5, sizeof(initData5), "init", false, true);
    writeCharacteristic(initData6, sizeof(initData6), "init", false, true);
    writeCharacteristic(initData7, sizeof(initData7), "init", false, true);
    writeCharacteristic(initData8, sizeof(initData8), "init", false, true);
    writeCharacteristic(initData9, sizeof(initData9), "init", false, true);
    writeCharacteristic(initData7, sizeof(initData7), "init", false, true);
    writeCharacteristic(initData9, sizeof(initData9), "init", false, true);
    writeCharacteristic(initData10, sizeof(initData10), "init", false, true);
    writeCharacteristic(initData10, sizeof(initData10), "init", false, true);
    writeCharacteristic(initData11, sizeof(initData11), "init", false, true);
    writeCharacteristic(initData12, sizeof(initData12), "init", false, true);
    writeCharacteristic(initData12, sizeof(initData12), "init", false, true);

    initDone = true;
}

void eslinkertreadmill::stateChanged(QLowEnergyService::ServiceState state)
{
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceState>();
    debug("BTLE stateChanged " + QString::fromLocal8Bit(metaEnum.valueToKey(state)));
    if(state == QLowEnergyService::ServiceDiscovered)
    {
        QBluetoothUuid _gattWriteCharacteristicId((quint16)0xfff2);
        QBluetoothUuid _gattNotifyCharacteristicId((quint16)0xfff1);

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

void eslinkertreadmill::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue)
{
    debug("descriptorWritten " + descriptor.name() + " " + newValue.toHex(' '));

    initRequest = true;
    emit connectedAndDiscovered();
}

void eslinkertreadmill::characteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue)
{
    Q_UNUSED(characteristic);
    debug("characteristicWritten " + newValue.toHex(' '));
}

void eslinkertreadmill::serviceScanDone(void)
{
    debug("serviceScanDone");

    QBluetoothUuid _gattCommunicationChannelServiceId((quint16)0xfff0);
    gattCommunicationChannelService = m_control->createServiceObject(_gattCommunicationChannelServiceId);
    connect(gattCommunicationChannelService, SIGNAL(stateChanged(QLowEnergyService::ServiceState)), this, SLOT(stateChanged(QLowEnergyService::ServiceState)));
    gattCommunicationChannelService->discoverDetails();
}

void eslinkertreadmill::errorService(QLowEnergyService::ServiceError err)
{
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    debug("eslinkertreadmill::errorService " + QString::fromLocal8Bit(metaEnum.valueToKey(err)) + m_control->errorString());
}

void eslinkertreadmill::error(QLowEnergyController::Error err)
{
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    debug("eslinkertreadmill::error " + QString::fromLocal8Bit(metaEnum.valueToKey(err)) + m_control->errorString());
}

void eslinkertreadmill::deviceDiscovered(const QBluetoothDeviceInfo &device)
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

void eslinkertreadmill::controllerStateChanged(QLowEnergyController::ControllerState state)
{
    qDebug() << "controllerStateChanged" << state;
    if(state == QLowEnergyController::UnconnectedState && m_control)
    {
        qDebug() << "trying to connect back again...";
        initDone = false;
        m_control->connectToDevice();
    }
}

bool eslinkertreadmill::connected()
{
    if(!m_control)
        return false;
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

void* eslinkertreadmill::VirtualTreadMill()
{
    return virtualTreadMill;
}

void* eslinkertreadmill::VirtualDevice()
{
    return VirtualTreadMill();
}

double eslinkertreadmill::odometer()
{
    return DistanceCalculated;
}

void eslinkertreadmill::setLastSpeed(double speed)
{
    lastSpeed = speed;
}

void eslinkertreadmill::setLastInclination(double inclination)
{
    lastInclination = inclination;
}

void eslinkertreadmill::searchingStop()
{
    searchStopped = true;
}
