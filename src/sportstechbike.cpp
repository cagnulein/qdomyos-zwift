#include "sportstechbike.h"
#include "virtualbike.h"
#include <QFile>
#include <QThread>
#include <QDateTime>
#include <QMetaEnum>
#include <QEventLoop>
#include <QBluetoothLocalDevice>
#include <QSettings>
#include "keepawakehelper.h"

sportstechbike::sportstechbike(bool noWriteResistance, bool noHeartService)
{
    m_watt.setType(metric::METRIC_WATT);
    refresh = new QTimer(this);
    this->noWriteResistance = noWriteResistance;
    this->noHeartService = noHeartService;
    initDone = false;
    connect(refresh, SIGNAL(timeout()), this, SLOT(update()));
    refresh->start(200);
}

void sportstechbike::writeCharacteristic(uint8_t* data, uint8_t data_len, QString info, bool disable_log, bool wait_for_response)
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

void sportstechbike::forceResistance(int8_t requestResistance)
{
    /*
    uint8_t resistance[] = { 0xf0, 0xa6, 0x01, 0x01, 0x00, 0x00 };
    resistance[4] = requestResistance + 1;
    for(uint8_t i=0; i<sizeof(resistance)-1; i++)
    {
       resistance[5] += resistance[i]; // the last byte is a sort of a checksum
    }
    writeCharacteristic((uint8_t*)resistance, sizeof(resistance), "resistance " + QString::number(requestResistance), false, true);
    */
}

void sportstechbike::update()
{
    //qDebug() << bike.isValid() << m_control->state() << gattCommunicationChannelService << gattWriteCharacteristic.isValid() << gattNotifyCharacteristic.isValid() << initDone;

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
       gattNotify1Characteristic.isValid() &&
       initDone)
    {
        if(currentSpeed().value() > 0.0 && !firstUpdate)
           elapsed += ((double)lastTimeUpdate.msecsTo(QTime::currentTime()) / 1000.0);

        // updating the bike console every second
        if(sec1update++ == (1000 / refresh->interval()))
        {
            sec1update = 0;
            //updateDisplay(elapsed);
        }

        QSettings settings;
        const uint8_t noOpData[] = { 0xf2, 0xc3, 0x07, 0x04, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xbe };
        writeCharacteristic((uint8_t*)noOpData, sizeof(noOpData), "noOp", false, true);

        if(requestResistance != -1)
        {
           if(requestResistance > 32) requestResistance = 32;
           else if(requestResistance < 1) requestResistance = 1;

           if(requestResistance != currentResistance().value())
           {
              debug("writing resistance " + QString::number(requestResistance));
              forceResistance(requestResistance);
           }
           requestResistance = -1;
        }
    }

    lastTimeUpdate = QTime::currentTime();
    firstUpdate = false;
}

void sportstechbike::serviceDiscovered(const QBluetoothUuid &gatt)
{
    debug("serviceDiscovered " + gatt.toString());
}

void sportstechbike::characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue)
{
    //qDebug() << "characteristicChanged" << characteristic.uuid() << newValue << newValue.length();
    Q_UNUSED(characteristic);
    QSettings settings;
    QString heartRateBeltName = settings.value("heart_rate_belt_name", "Disabled").toString();
    emit packetReceived();

    debug(" << " + newValue.toHex(' '));

    lastPacket = newValue;
    if (newValue.length() != 20)
        return;

    double speed = GetSpeedFromPacket(newValue);
    double cadence = GetCadenceFromPacket(newValue);
    double resistance = GetResistanceFromPacket(newValue);
    double kcal = GetKcalFromPacket(newValue);
    double watt = GetWattFromPacket(newValue);

#ifdef Q_OS_ANDROID
    if(settings.value("ant_heart", false).toBool())
        Heart = (uint8_t)KeepAwakeHelper::heart();
    else
#endif
    {
        if(heartRateBeltName.startsWith("Disabled"))
            Heart = 0;
    }
    FanSpeed = 0;

    if(!firstCharChanged)
        DistanceCalculated += ((speed / 3600.0) / ( 1000.0 / (lastTimeCharChanged.msecsTo(QTime::currentTime()))));

    debug("Current speed: " + QString::number(speed));
    debug("Current cadence: " + QString::number(cadence));
    debug("Current resistance: " + QString::number(resistance));
    debug("Current heart: " + QString::number(Heart.value()));
    debug("Current KCal: " + QString::number(kcal));
    debug("Current watt: " + QString::number(watt));
    debug("Current Elapsed from the bike (not used): " + QString::number(GetElapsedFromPacket(newValue)));
    debug("Current Distance Calculated: " + QString::number(DistanceCalculated));

    if(m_control->error() != QLowEnergyController::NoError)
        qDebug() << "QLowEnergyController ERROR!!" << m_control->errorString();

    Speed = speed;
    Resistance = resistance;
    KCal = kcal;
    Distance = DistanceCalculated;
    Cadence = cadence;
    m_watt = watt;

    lastTimeCharChanged = QTime::currentTime();
    firstCharChanged = false;
}

uint16_t sportstechbike::GetElapsedFromPacket(QByteArray packet)
{
    uint16_t convertedData = (packet.at(3));
    convertedData += (((uint16_t)packet.at(4)) << 8);
    return convertedData;
}

double sportstechbike::GetSpeedFromPacket(QByteArray packet)
{
    uint16_t convertedData = (packet.at(9) << 8) | packet.at(10);
    double data = (double)(convertedData) / 10.0f;
    return data;
}

double sportstechbike::GetKcalFromPacket(QByteArray packet)
{
    uint16_t convertedData = (packet.at(7) << 8) | packet.at(8);
    return (double)(convertedData);
}

double sportstechbike::GetWattFromPacket(QByteArray packet)
{
    uint16_t convertedData = (packet.at(12) << 8) | packet.at(13);
    double data = ((double)(convertedData));
    return data;
}

double sportstechbike::GetCadenceFromPacket(QByteArray packet)
{
    uint16_t convertedData = packet.at(17);
    double data = (convertedData);
    if (data < 0) return 0;
    return data;
}

double sportstechbike::GetResistanceFromPacket(QByteArray packet)
{
    uint16_t convertedData = packet.at(18);
    double data = (convertedData);
    if (data < 0) return 0;
    return data;
}

void sportstechbike::btinit(bool startTape)
{
    Q_UNUSED(startTape);
    QSettings settings;

    const uint8_t initData1[] = { 0xf2, 0xc0, 0x00, 0xb2 };
    const uint8_t initData2[] = { 0xf2, 0xc1, 0x05, 0x01, 0xff, 0xff, 0xff, 0xff, 0xb5 };
    const uint8_t initData3[] = { 0xf2, 0xc4, 0x0d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xc0 };
    const uint8_t initData4[] = { 0xf2, 0xc3, 0x07, 0x01, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xbb };

    writeCharacteristic((uint8_t*)initData1, sizeof(initData1), "init", false, true);
    writeCharacteristic((uint8_t*)initData2, sizeof(initData2), "init", false, true);
    writeCharacteristic((uint8_t*)initData3, sizeof(initData3), "init", false, true);
    writeCharacteristic((uint8_t*)initData4, sizeof(initData4), "init", false, true);

    initDone = true;
}

void sportstechbike::stateChanged(QLowEnergyService::ServiceState state)
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
        QString uuidNotify1 = "0000fff1-0000-1000-8000-00805f9b34fb";

        QBluetoothUuid _gattWriteCharacteristicId((QString)uuidWrite);
        QBluetoothUuid _gattNotify1CharacteristicId((QString)uuidNotify1);

        gattWriteCharacteristic = gattCommunicationChannelService->characteristic(_gattWriteCharacteristicId);
        gattNotify1Characteristic = gattCommunicationChannelService->characteristic(_gattNotify1CharacteristicId);
        Q_ASSERT(gattWriteCharacteristic.isValid());
        Q_ASSERT(gattNotify1Characteristic.isValid());

        // establish hook into notifications
        connect(gattCommunicationChannelService, SIGNAL(characteristicChanged(QLowEnergyCharacteristic,QByteArray)),
                this, SLOT(characteristicChanged(QLowEnergyCharacteristic,QByteArray)));
        connect(gattCommunicationChannelService, SIGNAL(characteristicWritten(const QLowEnergyCharacteristic, const QByteArray)),
                this, SLOT(characteristicWritten(const QLowEnergyCharacteristic, const QByteArray)));
        connect(gattCommunicationChannelService, SIGNAL(error(QLowEnergyService::ServiceError)),
                this, SLOT(errorService(QLowEnergyService::ServiceError)));
        connect(gattCommunicationChannelService, SIGNAL(descriptorWritten(const QLowEnergyDescriptor, const QByteArray)), this,
                SLOT(descriptorWritten(const QLowEnergyDescriptor, const QByteArray)));

        // ******************************************* virtual bike init *************************************
        if(!firstVirtualBike && !virtualBike)
        {
            QSettings settings;
            bool virtual_device_enabled = settings.value("virtual_device_enabled", true).toBool();
            if(virtual_device_enabled)
            {
                debug("creating virtual bike interface...");
                virtualBike = new virtualbike(this, noWriteResistance, noHeartService);
                connect(virtualBike,&virtualbike::debug ,this,&sportstechbike::debug);
            }
        }
        firstVirtualBike = 1;
        // ********************************************************************************************************

        QByteArray descriptor;
        descriptor.append((char)0x01);
        descriptor.append((char)0x00);
        gattCommunicationChannelService->writeDescriptor(gattNotify1Characteristic.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration), descriptor);
    }
}

void sportstechbike::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue)
{
    debug("descriptorWritten " + descriptor.name() + " " + newValue.toHex(' '));

    initRequest = true;
    emit connectedAndDiscovered();
}

void sportstechbike::characteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue)
{
    Q_UNUSED(characteristic);
    debug("characteristicWritten " + newValue.toHex(' '));
}

void sportstechbike::serviceScanDone(void)
{
    debug("serviceScanDone");

    QString uuid = "0000fff0-0000-1000-8000-00805f9b34fb";

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

void sportstechbike::errorService(QLowEnergyService::ServiceError err)
{
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    debug("sportstechbike::errorService" + QString::fromLocal8Bit(metaEnum.valueToKey(err)) + m_control->errorString());
}

void sportstechbike::error(QLowEnergyController::Error err)
{
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    debug("sportstechbike::error" + QString::fromLocal8Bit(metaEnum.valueToKey(err)) + m_control->errorString());
}

void sportstechbike::deviceDiscovered(const QBluetoothDeviceInfo &device)
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

uint16_t sportstechbike::watts()
{
    if(currentCadence().value() == 0) return 0;

    return m_watt.value();
}

bool sportstechbike::connected()
{
    if(!m_control)
        return false;
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

void* sportstechbike::VirtualBike()
{
    return virtualBike;
}

void* sportstechbike::VirtualDevice()
{
    return VirtualBike();
}

double sportstechbike::odometer()
{
    return DistanceCalculated;
}

void sportstechbike::controllerStateChanged(QLowEnergyController::ControllerState state)
{
    qDebug() << "controllerStateChanged" << state;
    if(state == QLowEnergyController::UnconnectedState && m_control)
    {
        qDebug() << "trying to connect back again...";
        initDone = false;
        m_control->connectToDevice();
    }
}


