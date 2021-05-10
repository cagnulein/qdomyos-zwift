#include "spirittreadmill.h"
#include "virtualtreadmill.h"
#include <QFile>
#include <QThread>
#include <QDateTime>
#include <QMetaEnum>
#include <QEventLoop>
#include <QBluetoothLocalDevice>
#include <QSettings>
#include "keepawakehelper.h"

spirittreadmill::spirittreadmill()
{
    m_watt.setType(metric::METRIC_WATT);
    refresh = new QTimer(this);
    initDone = false;
    connect(refresh, SIGNAL(timeout()), this, SLOT(update()));
    refresh->start(200);
}

void spirittreadmill::writeCharacteristic(uint8_t* data, uint8_t data_len, QString info, bool disable_log, bool wait_for_response)
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

    gattCommunicationChannelService->writeCharacteristic(gattWriteCharacteristic, QByteArray((const char*)data, data_len));

    if(!disable_log)
        debug(" >> " + QByteArray((const char*)data, data_len).toHex(' ') + " // " + info);

    loop.exec();

    if(timeout.isActive() == false)
        debug(" exit for timeout");
}

void spirittreadmill::forceSpeedOrIncline(double requestSpeed, double requestIncline)
{
    Q_UNUSED(requestIncline);
    if(requestSpeed > Speed.value())
    {
        uint8_t increaseSpeed[] = { 0x5b, 0x04, 0x00, 0x06, 0x4f, 0x4b, 0x5d };
        writeCharacteristic(increaseSpeed, sizeof(increaseSpeed), "increaseSpeed", false, true);
    }
    else
    {
        uint8_t decreaseSpeed[] = { 0x5b, 0x04, 0x00, 0x32, 0x4f, 0x4b, 0x5d };
        writeCharacteristic(decreaseSpeed, sizeof(decreaseSpeed), "decreaseSpeed", false, true);
    }
}

bool spirittreadmill::changeFanSpeed(uint8_t speed)
{
   Q_UNUSED(speed);
   return false;
}


void spirittreadmill::update()
{
    //qDebug() << treadmill.isValid() << m_control->state() << gattCommunicationChannelService << gattWriteCharacteristic.isValid() << gattNotifyCharacteristic.isValid() << initDone;

    if(!m_control)
        return;

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
        QSettings settings;
        update_metrics(true, watts(settings.value("weight", 75.0).toFloat()));

        // updating the treadmill console every second
        if(sec1update++ == (1000 / refresh->interval()))
        {
            sec1update = 0;
            //updateDisplay(elapsed);
        }
        else
        {
            uint8_t noOpData[] = { 0x5b, 0x04, 0x00, 0x10, 0x4f, 0x4b, 0x5d };
            uint8_t noOpData1[] = { 0x5b, 0x04, 0x00, 0x40, 0x4f, 0x4b, 0x5d };

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

        if(requestSpeed != -1)
        {
           if(requestSpeed != currentSpeed().value())
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
           if(requestInclination != currentInclination().value())
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
           requestStart = -1;
           emit tapeStarted();
        }
        if(requestStop != -1)
        {
            debug("stopping...");
            //writeCharacteristic(initDataF0C800B8, sizeof(initDataF0C800B8), "stop tape");
            requestStop = -1;
        }
        if(requestIncreaseFan != -1)
        {
            debug("increasing fan speed...");
            changeFanSpeed(FanSpeed + 1);
            requestIncreaseFan = -1;
        }
        else if(requestDecreaseFan != -1)
        {
            debug("decreasing fan speed...");
            changeFanSpeed(FanSpeed - 1);
            requestDecreaseFan = -1;
        }
    }
}

void spirittreadmill::serviceDiscovered(const QBluetoothUuid &gatt)
{
    debug("serviceDiscovered " + gatt.toString());
}

void spirittreadmill::characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue)
{
    //qDebug() << "characteristicChanged" << characteristic.uuid() << newValue << newValue.length();
    Q_UNUSED(characteristic);
    QSettings settings;
    QString heartRateBeltName = settings.value("heart_rate_belt_name", "Disabled").toString();
    emit packetReceived();

    debug(" << " + newValue.toHex(' '));

    lastPacket = newValue;
    if (newValue.length() != 18)
        return;

    double speed = GetSpeedFromPacket(newValue);
    Inclination = GetInclinationFromPacket(newValue);
    double kcal = GetKcalFromPacket(newValue);
    //double distance = GetDistanceFromPacket(newValue) * settings.value("domyos_elliptical_speed_ratio", 1.0).toDouble();

#ifdef Q_OS_ANDROID
    if(settings.value("ant_heart", false).toBool())
        Heart = (uint8_t)KeepAwakeHelper::heart();
    else
#endif
    {
        if(heartRateBeltName.startsWith("Disabled"))
            Heart = ((uint8_t)newValue.at(18));
    }

    Distance += ((Speed.value() / 3600000.0) * ((double)lastTimeCharChanged.msecsTo(QTime::currentTime())) );

    debug("Current speed: " + QString::number(speed));
    debug("Current inclination: " + QString::number(Inclination.value()));
    debug("Current heart: " + QString::number(Heart.value()));
    debug("Current KCal: " + QString::number(kcal));
    debug("Current Distance: " + QString::number(Distance.value()));
    debug("Current Watt: " + QString::number(watts(settings.value("weight", 75.0).toFloat())));

    if(m_control->error() != QLowEnergyController::NoError)
        qDebug() << "QLowEnergyController ERROR!!" << m_control->errorString();

    lastTimeCharChanged = QTime::currentTime();

    Speed = speed;
    KCal = kcal;
}

uint16_t spirittreadmill::GetElapsedFromPacket(QByteArray packet)
{
    uint16_t convertedData = (packet.at(3) << 8) | packet.at(4);
    return convertedData;
}

double spirittreadmill::GetSpeedFromPacket(QByteArray packet)
{
    uint16_t convertedData = packet.at(10);
    double data = (double)convertedData / 10.0f;
    return data;
}

double spirittreadmill::GetKcalFromPacket(QByteArray packet)
{
    uint16_t convertedData = (packet.at(7) << 8) | ((uint8_t)packet.at(8));
    return (double)convertedData / 10.0;
}

double spirittreadmill::GetDistanceFromPacket(QByteArray packet)
{
    uint16_t convertedData = (packet.at(12) << 8) | packet.at(13);
    double data = ((double)convertedData) / 10.0f;
    return data;
}

double spirittreadmill::GetInclinationFromPacket(QByteArray packet)
{
    uint16_t convertedData = packet.at(11);
    double data = (double)convertedData / 10.0f;
    return data;
}

void spirittreadmill::btinit(bool startTape)
{
    // set speed and incline to 0
    uint8_t initData1[] = { 0x5b, 0x01, 0xf0, 0x5d };
    uint8_t initData2[] = { 0x5b, 0x04, 0x00, 0x10, 0x4f, 0x4b, 0x5d };
    uint8_t initData3[] = { 0x5b, 0x02, 0x03, 0x01, 0x5d };
    uint8_t initData4[] = { 0x5b, 0x04, 0x00, 0x09, 0x4f, 0x4b, 0x5d };
    uint8_t initData5[] = { 0x5b, 0x06, 0x07, 0x00, 0x23, 0x00, 0x84, 0x40, 0x5d };
    uint8_t initData6[] = { 0x5b, 0x03, 0x08, 0x10, 0x01, 0x5d };
    uint8_t initData7[] = { 0x5b, 0x05, 0x04, 0x00, 0x00, 0x00, 0x00, 0x5d };
    uint8_t initData8[] = { 0x5b, 0x02, 0x22, 0x09, 0x5d };
    uint8_t initData9[] = { 0x5b, 0x02, 0x02, 0x02, 0x5d };

    writeCharacteristic(initData1, sizeof(initData1), "init", false, true);
    writeCharacteristic(initData2, sizeof(initData2), "init", false, true);
    writeCharacteristic(initData3, sizeof(initData3), "init", false, true);
    writeCharacteristic(initData3, sizeof(initData3), "init", false, true);
    writeCharacteristic(initData3, sizeof(initData3), "init", false, true);
    writeCharacteristic(initData2, sizeof(initData2), "init", false, true);
    writeCharacteristic(initData3, sizeof(initData3), "init", false, true);
    writeCharacteristic(initData4, sizeof(initData4), "init", false, true);
    writeCharacteristic(initData2, sizeof(initData2), "init", false, false);
    writeCharacteristic(initData4, sizeof(initData4), "init", false, true);
    writeCharacteristic(initData4, sizeof(initData4), "init", false, false);
    writeCharacteristic(initData5, sizeof(initData5), "init", false, true);
    writeCharacteristic(initData6, sizeof(initData6), "init", false, true);
    writeCharacteristic(initData7, sizeof(initData7), "init", false, true);
    writeCharacteristic(initData8, sizeof(initData8), "init", false, true);
    writeCharacteristic(initData9, sizeof(initData9), "init", false, true);

    initDone = true;
}

void spirittreadmill::stateChanged(QLowEnergyService::ServiceState state)
{
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceState>();
    debug("BTLE stateChanged " + QString::fromLocal8Bit(metaEnum.valueToKey(state)));

    if(state == QLowEnergyService::ServiceDiscovered)
    {
        foreach(QLowEnergyCharacteristic c,gattCommunicationChannelService->characteristics())
        {
            debug("characteristic " + c.uuid().toString());
        }

        QString uuidWrite      = "49535343-8841-43f4-a8d4-ecbe34729bb3";
        QString uuidNotify     = "49535343-1E4D-4BD9-BA61-23C647249616";

        QBluetoothUuid _gattWriteCharacteristicId((QString)uuidWrite);
        QBluetoothUuid _gattNotifyCharacteristicId((QString)uuidNotify);

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

        // ******************************************* virtual treadmill init *************************************
        if(!firstVirtualTreadmill && !virtualTreadMill)
        {
            QSettings settings;
            bool virtual_device_enabled = settings.value("virtual_device_enabled", true).toBool();
            if(virtual_device_enabled)
            {
                debug("creating virtual treadmill interface...");
                virtualTreadMill = new virtualtreadmill(this, false);
                connect(virtualTreadMill,&virtualtreadmill::debug ,this,&spirittreadmill::debug);
            }
        }
        firstVirtualTreadmill = 1;
        // ********************************************************************************************************

        QByteArray descriptor;
        descriptor.append((char)0x01);
        descriptor.append((char)0x00);
        gattCommunicationChannelService->writeDescriptor(gattNotifyCharacteristic.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration), descriptor);
    }
}

void spirittreadmill::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue)
{
    debug("descriptorWritten " + descriptor.name() + " " + newValue.toHex(' '));

    initRequest = true;
    emit connectedAndDiscovered();
}

void spirittreadmill::characteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue)
{
    Q_UNUSED(characteristic);
    debug("characteristicWritten " + newValue.toHex(' '));
}

void spirittreadmill::serviceScanDone(void)
{
    debug("serviceScanDone");

    QString uuid = "49535343-FE7D-4AE5-8FA9-9FAFD205E455";

    QBluetoothUuid _gattCommunicationChannelServiceId((QString)uuid);
    gattCommunicationChannelService = m_control->createServiceObject(_gattCommunicationChannelServiceId);

    if(gattCommunicationChannelService == nullptr)
    {
        qDebug() << "invalid service" << uuid;
        return;
    }

    connect(gattCommunicationChannelService, SIGNAL(stateChanged(QLowEnergyService::ServiceState)), this, SLOT(stateChanged(QLowEnergyService::ServiceState)));
    gattCommunicationChannelService->discoverDetails();
}

void spirittreadmill::errorService(QLowEnergyService::ServiceError err)
{
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    debug("spirittreadmill::errorService" + QString::fromLocal8Bit(metaEnum.valueToKey(err)) + m_control->errorString());
}

void spirittreadmill::error(QLowEnergyController::Error err)
{
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    debug("spirittreadmill::error" + QString::fromLocal8Bit(metaEnum.valueToKey(err)) + m_control->errorString());
}

void spirittreadmill::deviceDiscovered(const QBluetoothDeviceInfo &device)
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
            emit disconnected();
        });

        // Connect
        m_control->connectToDevice();
        return;
    }
}

bool spirittreadmill::connected()
{
    if(!m_control)
        return false;
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

void* spirittreadmill::VirtualTreadMill()
{
    return virtualTreadMill;
}

void* spirittreadmill::VirtualDevice()
{
    return VirtualTreadMill();
}

double spirittreadmill::odometer()
{
    return DistanceCalculated;
}

void spirittreadmill::controllerStateChanged(QLowEnergyController::ControllerState state)
{
    qDebug() << "controllerStateChanged" << state;
    if(state == QLowEnergyController::UnconnectedState && m_control)
    {
        qDebug() << "trying to connect back again...";
        initDone = false;
        m_control->connectToDevice();
    }
}
