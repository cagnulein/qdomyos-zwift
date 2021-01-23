#include "trxappgateusbtreadmill.h"
#include "virtualtreadmill.h"
#include <QFile>
#include <QThread>
#include <QDateTime>
#include <QMetaEnum>
#include <QEventLoop>
#include <QBluetoothLocalDevice>
#include <QSettings>

trxappgateusbtreadmill::trxappgateusbtreadmill()
{
    refresh = new QTimer(this);
    initDone = false;
    connect(refresh, SIGNAL(timeout()), this, SLOT(update()));
    refresh->start(200);
}

void trxappgateusbtreadmill::writeCharacteristic(uint8_t* data, uint8_t data_len, QString info, bool disable_log, bool wait_for_response)
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

void trxappgateusbtreadmill::forceSpeedOrIncline(double requestSpeed, double requestIncline)
{
    Q_UNUSED(requestSpeed);
    Q_UNUSED(requestIncline);
}

bool trxappgateusbtreadmill::changeFanSpeed(uint8_t speed)
{
   Q_UNUSED(speed);
   return false;
}


void trxappgateusbtreadmill::update()
{
    //qDebug() << treadmill.isValid() << m_control->state() << gattCommunicationChannelService << gattWriteCharacteristic.isValid() << gattNotifyCharacteristic.isValid() << initDone;

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
        if(currentSpeed().value() > 0.0 && !firstUpdate)
           elapsed += ((double)lastTimeUpdate.msecsTo(QTime::currentTime()) / 1000.0);

        // updating the treadmill console every second
        if(sec1update++ == (1000 / refresh->interval()))
        {
            sec1update = 0;
            //updateDisplay(elapsed);
        }

        QSettings settings;
        bool toorx30 = settings.value("toorx_3_0", false).toBool();
        if(toorx30 == false)
        {
            const uint8_t noOpData[] = { 0xf0, 0xa2, 0x01, 0xd3, 0x66 };
            writeCharacteristic((uint8_t*)noOpData, sizeof(noOpData), "noOp", false, true);
        }
        else
        {
            const uint8_t noOpData[] = { 0xf0, 0xa2, 0x23, 0xd3, 0x88 };
            writeCharacteristic((uint8_t*)noOpData, sizeof(noOpData), "noOp", false, true);
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
           //btinit(true);        
           if(toorx30 == false)
           {
               const uint8_t startTape[] = { 0xf0, 0xa5, 0x01, 0xd3, 0x02, 0x6b };
               writeCharacteristic((uint8_t*)startTape, sizeof(startTape), "startTape", false, true);
           }
           else
           {
               const uint8_t startTape[] = { 0xf0, 0xa5, 0x23, 0xd3, 0x02, 0x8d };
               writeCharacteristic((uint8_t*)startTape, sizeof(startTape), "startTape", false, true);
           }

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

        elevationAcc += (currentSpeed().value() / 3600.0) * 1000 * (currentInclination().value() / 100) * (refresh->interval() / 1000);
    }

    lastTimeUpdate = QTime::currentTime();
    firstUpdate = false;
}

void trxappgateusbtreadmill::serviceDiscovered(const QBluetoothUuid &gatt)
{
    debug("serviceDiscovered " + gatt.toString());
}

void trxappgateusbtreadmill::characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue)
{
    //qDebug() << "characteristicChanged" << characteristic.uuid() << newValue << newValue.length();
    Q_UNUSED(characteristic);
    QSettings settings;
    QString heartRateBeltName = settings.value("heart_rate_belt_name", "Disabled").toString();
    emit packetReceived();

    debug(" << " + newValue.toHex(' '));

    lastPacket = newValue;
    if (newValue.length() != 19)
        return;

    if(treadmill_type == TYPE::IRUNNING)
    {
        if(newValue.at(15) == 0x03 && newValue.at(16) == 0x02 && readyToStart == false)
        {
            readyToStart = true;
            requestStart = 1;
        }
    }
    else
    {
        if(newValue.at(16) == 0x04 && newValue.at(17) == 0x03 && readyToStart == false)
        {
            readyToStart = true;
            requestStart = 1;
        }
    }

    double speed = GetSpeedFromPacket(newValue);
    double incline = GetInclinationFromPacket(newValue);
    double kcal = GetKcalFromPacket(newValue);
    double distance = GetDistanceFromPacket(newValue);

    if(heartRateBeltName.startsWith("Disabled"))
        Heart = 0;
    FanSpeed = 0;

    if(!firstCharChanged)
        DistanceCalculated += ((speed / 3600.0) / ( 1000.0 / (lastTimeCharChanged.msecsTo(QTime::currentTime()))));

    debug("Current speed: " + QString::number(speed));
    debug("Current incline: " + QString::number(incline));
    debug("Current heart: " + QString::number(Heart.value()));
    debug("Current KCal: " + QString::number(kcal));
    debug("Current Distance: " + QString::number(distance));
    debug("Current Elapsed from the treadmill (not used): " + QString::number(GetElapsedFromPacket(newValue)));
    debug("Current Distance Calculated: " + QString::number(DistanceCalculated));

    if(m_control->error() != QLowEnergyController::NoError)
        qDebug() << "QLowEnergyController ERROR!!" << m_control->errorString();

    Speed = speed;
    Inclination = incline;
    KCal = kcal;
    Distance = distance;

    lastTimeCharChanged = QTime::currentTime();
    firstCharChanged = false;
}

uint16_t trxappgateusbtreadmill::GetElapsedFromPacket(QByteArray packet)
{
    uint16_t convertedData = (packet.at(4) - 1);
    convertedData += ((packet.at(5) - 1) * 60);
    return convertedData;
}

double trxappgateusbtreadmill::GetSpeedFromPacket(QByteArray packet)
{
    uint16_t convertedData = (packet.at(13) - 1) + ((packet.at(12) - 1) * 100);
    double data = (double)(convertedData) / 10.0f;
    return data;
}

double trxappgateusbtreadmill::GetKcalFromPacket(QByteArray packet)
{
    uint16_t convertedData = ((packet.at(8) - 1) << 8) | (packet.at(9) - 1);
    return (double)(convertedData);
}

double trxappgateusbtreadmill::GetDistanceFromPacket(QByteArray packet)
{
    uint16_t convertedData = ((packet.at(6) - 1) << 8) | (packet.at(7) - 1);
    double data = ((double)(convertedData)) / 100.0f;
    return data;
}

double trxappgateusbtreadmill::GetInclinationFromPacket(QByteArray packet)
{
    uint16_t convertedData = packet.at(14);
    double data = (convertedData - 1);
    if (data < 0) return 0;
    return data;
}

void trxappgateusbtreadmill::btinit(bool startTape)
{
    Q_UNUSED(startTape);
    QSettings settings;
    bool toorx30 = settings.value("toorx_3_0", false).toBool();

    if(toorx30 == false)
    {
        const uint8_t initData1[] = { 0xf0, 0xa0, 0x01, 0x01, 0x92 };
        const uint8_t initData2[] = { 0xf0, 0xa5, 0x01, 0xd3, 0x04, 0x6d };
        const uint8_t initData3[] = { 0xf0, 0xa0, 0x01, 0xd3, 0x64 };
        const uint8_t initData4[] = { 0xf0, 0xa1, 0x01, 0xd3, 0x65 };
        const uint8_t initData5[] = { 0xf0, 0xa3, 0x01, 0xd3, 0x01, 0x15, 0x01, 0x02, 0x51, 0x01, 0x51, 0x23 };
        const uint8_t initData6[] = { 0xf0, 0xa4, 0x01, 0xd3, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x73 };
        const uint8_t initData7[] = { 0xf0, 0xaf, 0x01, 0xd3, 0x02, 0x75 };

        writeCharacteristic((uint8_t*)initData1, sizeof(initData1), "init", false, true);
        if(treadmill_type == TYPE::IRUNNING) QThread::msleep(400);
        writeCharacteristic((uint8_t*)initData2, sizeof(initData2), "init", false, true);
        if(treadmill_type == TYPE::IRUNNING) QThread::msleep(400);
        writeCharacteristic((uint8_t*)initData3, sizeof(initData3), "init", false, true);
        if(treadmill_type == TYPE::IRUNNING) QThread::msleep(400);
        writeCharacteristic((uint8_t*)initData4, sizeof(initData4), "init", false, true);
        if(treadmill_type == TYPE::IRUNNING) QThread::msleep(400);
        writeCharacteristic((uint8_t*)initData3, sizeof(initData3), "init", false, true);
        if(treadmill_type == TYPE::IRUNNING) QThread::msleep(400);
        if(treadmill_type == TYPE::IRUNNING)
        {
            writeCharacteristic((uint8_t*)initData4, sizeof(initData4), "init", false, true);
            QThread::msleep(400);
            writeCharacteristic((uint8_t*)initData3, sizeof(initData3), "init", false, true);
            QThread::msleep(400);
            writeCharacteristic((uint8_t*)initData3, sizeof(initData3), "init", false, true);
            QThread::msleep(400);
        }
        writeCharacteristic((uint8_t*)initData5, sizeof(initData5), "init", false, true);
        if(treadmill_type == TYPE::IRUNNING) QThread::msleep(400);
        writeCharacteristic((uint8_t*)initData6, sizeof(initData6), "init", false, true);
        if(treadmill_type == TYPE::IRUNNING) QThread::msleep(400);
        writeCharacteristic((uint8_t*)initData7, sizeof(initData7), "init", false, true);
        if(treadmill_type == TYPE::IRUNNING) QThread::msleep(400);
    }
    else
    {
        const uint8_t initData1[] = { 0xf0, 0xa0, 0x01, 0x01, 0x92 };
        const uint8_t initData2[] = { 0xf0, 0xa5, 0x23, 0xd3, 0x04, 0x8f };
        const uint8_t initData3[] = { 0xf0, 0xa0, 0x23, 0xd3, 0x86 };
        const uint8_t initData4[] = { 0xf0, 0xa1, 0x23, 0xd3, 0x87 };
        const uint8_t initData5[] = { 0xf0, 0xa3, 0x23, 0xd3, 0x01, 0x15, 0x01, 0x02, 0x51, 0x01, 0x51, 0x45 };
        const uint8_t initData6[] = { 0xf0, 0xa4, 0x23, 0xd3, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x95 };
        const uint8_t initData7[] = { 0xf0, 0xaf, 0x23, 0xd3, 0x02, 0x97 };

        writeCharacteristic((uint8_t*)initData1, sizeof(initData1), "init", false, true);
        writeCharacteristic((uint8_t*)initData2, sizeof(initData2), "init", false, true);
        writeCharacteristic((uint8_t*)initData3, sizeof(initData3), "init", false, true);
        writeCharacteristic((uint8_t*)initData4, sizeof(initData4), "init", false, true);
        writeCharacteristic((uint8_t*)initData5, sizeof(initData5), "init", false, true);
        writeCharacteristic((uint8_t*)initData6, sizeof(initData6), "init", false, true);
        writeCharacteristic((uint8_t*)initData7, sizeof(initData7), "init", false, true);
    }
    initDone = true;
}

void trxappgateusbtreadmill::stateChanged(QLowEnergyService::ServiceState state)
{
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceState>();
    debug("BTLE stateChanged " + QString::fromLocal8Bit(metaEnum.valueToKey(state)));

    if(state == QLowEnergyService::ServiceDiscovered)
    {
        foreach(QLowEnergyCharacteristic c,gattCommunicationChannelService->characteristics())
        {
            debug("characteristic " + c.uuid().toString());
        }

        QString uuidWrite  = "0000fff2-0000-1000-8000-00805f9b34fb";
        QString uuidNotify = "0000fff1-0000-1000-8000-00805f9b34fb";

        if(treadmill_type == TYPE::IRUNNING)
        {
            uuidWrite      = "49535343-8841-43f4-a8d4-ecbe34729bb3";
            uuidNotify     = "49535343-1E4D-4BD9-BA61-23C647249616";
        }

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
                connect(virtualTreadMill,&virtualtreadmill::debug ,this,&trxappgateusbtreadmill::debug);
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

void trxappgateusbtreadmill::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue)
{
    debug("descriptorWritten " + descriptor.name() + " " + newValue.toHex(' '));

    initRequest = true;
    emit connectedAndDiscovered();
}

void trxappgateusbtreadmill::characteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue)
{
    Q_UNUSED(characteristic);
    debug("characteristicWritten " + newValue.toHex(' '));
}

void trxappgateusbtreadmill::serviceScanDone(void)
{
    debug("serviceScanDone");

    QString uuid = "0000fff0-0000-1000-8000-00805f9b34fb";
    if(treadmill_type == TYPE::IRUNNING)
            uuid = "49535343-FE7D-4AE5-8FA9-9FAFD205E455";

    QBluetoothUuid _gattCommunicationChannelServiceId((QString)uuid);
    gattCommunicationChannelService = m_control->createServiceObject(_gattCommunicationChannelServiceId);
    connect(gattCommunicationChannelService, SIGNAL(stateChanged(QLowEnergyService::ServiceState)), this, SLOT(stateChanged(QLowEnergyService::ServiceState)));
    gattCommunicationChannelService->discoverDetails();
}

void trxappgateusbtreadmill::errorService(QLowEnergyService::ServiceError err)
{
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    debug("trxappgateusbtreadmill::errorService" + QString::fromLocal8Bit(metaEnum.valueToKey(err)) + m_control->errorString());
}

void trxappgateusbtreadmill::error(QLowEnergyController::Error err)
{
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    debug("trxappgateusbtreadmill::error" + QString::fromLocal8Bit(metaEnum.valueToKey(err)) + m_control->errorString());
}

void trxappgateusbtreadmill::deviceDiscovered(const QBluetoothDeviceInfo &device)
{
    debug("Found new device: " + device.name() + " (" + device.address().toString() + ')');
    if(device.name().startsWith("TOORX") || device.name().startsWith("V-RUN") || device.name().startsWith("FS-") || device.name().startsWith("i-Console+") || device.name().startsWith("i-Running"))
    {
        if(device.name().startsWith("i-Running") || device.name().startsWith("i-Console+"))
            treadmill_type = TYPE::IRUNNING;
        else
            treadmill_type = TYPE::TRXAPPGATE;

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

bool trxappgateusbtreadmill::connected()
{
    if(!m_control)
        return false;
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

void* trxappgateusbtreadmill::VirtualTreadMill()
{
    return virtualTreadMill;
}

void* trxappgateusbtreadmill::VirtualDevice()
{
    return VirtualTreadMill();
}

double trxappgateusbtreadmill::odometer()
{
    return DistanceCalculated;
}

void trxappgateusbtreadmill::controllerStateChanged(QLowEnergyController::ControllerState state)
{
    qDebug() << "controllerStateChanged" << state;
    if(state == QLowEnergyController::UnconnectedState && m_control)
    {
        qDebug() << "trying to connect back again...";
        initDone = false;
        m_control->connectToDevice();
    }
}
