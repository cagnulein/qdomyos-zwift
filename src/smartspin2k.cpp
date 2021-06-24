#include "smartspin2k.h"
#include "virtualbike.h"
#include <QFile>
#include <QDateTime>
#include <QMetaEnum>
#include <QSettings>
#include <QBluetoothLocalDevice>
#include <math.h>
#include <QThread>
#include "ios/lockscreen.h"
#ifdef Q_OS_ANDROID
#include <QLowEnergyConnectionParameters>
#endif
#include "keepawakehelper.h"

smartspin2k::smartspin2k(bool noWriteResistance, bool noHeartService)
{
    m_watt.setType(metric::METRIC_WATT);
    Speed.setType(metric::METRIC_SPEED);
    refresh = new QTimer(this);
    this->noWriteResistance = noWriteResistance;
    this->noHeartService = noHeartService;
    initDone = false;
    connect(refresh, SIGNAL(timeout()), this, SLOT(update()));
    refresh->start(200);
}

void smartspin2k::resistanceReadFromTheBike(int8_t resistance)
{
    if(startupResistance == -1)
        startupResistance = resistance;
    Resistance = resistance;
}

void smartspin2k::writeCharacteristic(uint8_t* data, uint8_t data_len, QString info, bool disable_log, bool wait_for_response)
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
}

void smartspin2k::forceResistance(int8_t requestResistance)
{
    //uint8_t write[] = { FTMS_SET_INDOOR_BIKE_SIMULATION_PARAMS, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    //write[3] = ((uint16_t)(requestResistance - startupResistance) * 100) & 0xFF;
    //write[4] = ((uint16_t)(requestResistance - startupResistance) * 100) >> 8;

    uint8_t write[] = { 0x02, 0x17, 0x00, 0x00 };
    write[2] = (uint8_t)(requestResistance & 0xFF);
    write[3] = (uint8_t)(requestResistance >> 8);

    writeCharacteristic(write, sizeof(write), "forceResistance " + QString::number(requestResistance));
}

void smartspin2k::update()
{
    if(m_control->state() == QLowEnergyController::UnconnectedState)
    {
        emit disconnected();
        return;
    }

    if(initRequest)
    {
        //uint8_t write[] = { 0x02, 0x1A, 0x01 };
        //writeCharacteristic(write, sizeof(write), "BLE_externalControl enabling");
        initRequest = false;
    }
    else if(bluetoothDevice.isValid() &&
       m_control->state() == QLowEnergyController::DiscoveredState //&&
       //gattCommunicationChannelService &&
       //gattWriteCharacteristic.isValid() &&
       //gattNotify1Characteristic.isValid() &&
       /*initDone*/)
    {
        update_metrics(true, watts());

        // updating the treadmill console every second
        if(sec1Update++ == (500 / refresh->interval()))
        {
            sec1Update = 0;
            //updateDisplay(elapsed);
        }

        if(requestResistance != -1)
        {
            if(requestResistance > 100) requestResistance = 100; // TODO, use the bluetooth value
           else if(requestResistance == 0) requestResistance = 1;

           if(requestResistance != currentResistance().value())
           {
              debug("writing resistance " + QString::number(requestResistance));
              forceResistance(requestResistance);
           }
           requestResistance = -1;
        }
        else {
            uint8_t read[] = { 0x01, 0x17 };
            writeCharacteristic(read, sizeof(read), QStringLiteral("polling"));
        }
    }
}

void smartspin2k::serviceDiscovered(const QBluetoothUuid &gatt)
{
    debug("serviceDiscovered " + gatt.toString());
}

void smartspin2k::characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue)
{
    Q_UNUSED(characteristic);

    debug(" << " + newValue.toHex(' '));

    lastPacket = newValue;

    if(newValue.length() >= 4 ) {
        Resistance = (int16_t)(((uint16_t)newValue.at(2)) + (((uint16_t)newValue.at(3)) << 8));
        qDebug() << "Resistance received from SS2k:" << Resistance.value();
    }
}

void smartspin2k::stateChanged(QLowEnergyService::ServiceState state)
{
    QBluetoothUuid _gattWriteCharacteristicId((QString)"77776277-7877-7774-4466-896665500001");
    //QBluetoothUuid _gattNotify1CharacteristicId((QString)"77776277-7877-7774-4466-896665500001");

    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceState>();
    qDebug() << "BTLE stateChanged " + QString::fromLocal8Bit(metaEnum.valueToKey(state));

    if(state == QLowEnergyService::ServiceDiscovered)
    {
        //qDebug() << gattCommunicationChannelService->characteristics();

        gattWriteCharacteristic = gattCommunicationChannelService->characteristic(_gattWriteCharacteristicId);
        //gattNotify1Characteristic = gattCommunicationChannelService->characteristic(_gattNotify1CharacteristicId);
        Q_ASSERT(gattWriteCharacteristic.isValid());
        //Q_ASSERT(gattNotify1Characteristic.isValid());

        // establish hook into notifications
        connect(gattCommunicationChannelService, SIGNAL(characteristicChanged(QLowEnergyCharacteristic,QByteArray)),
                this, SLOT(characteristicChanged(QLowEnergyCharacteristic,QByteArray)));
        connect(gattCommunicationChannelService, SIGNAL(characteristicWritten(const QLowEnergyCharacteristic, const QByteArray)),
                this, SLOT(characteristicWritten(const QLowEnergyCharacteristic, const QByteArray)));
        connect(gattCommunicationChannelService, SIGNAL(error(QLowEnergyService::ServiceError)),
                this, SLOT(errorService(QLowEnergyService::ServiceError)));
        connect(gattCommunicationChannelService, SIGNAL(descriptorWritten(const QLowEnergyDescriptor, const QByteArray)), this,
                SLOT(descriptorWritten(const QLowEnergyDescriptor, const QByteArray)));

        firstStateChanged = 1;

        QByteArray descriptor;
        descriptor.append((char)0x02);
        descriptor.append((char)0x00);
        gattCommunicationChannelService->writeDescriptor(gattWriteCharacteristic.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration), descriptor);
    }
}

void smartspin2k::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue)
{
    debug("descriptorWritten " + descriptor.name() + " " + newValue.toHex(' '));

    initRequest = true;
    emit connectedAndDiscovered();
}

void smartspin2k::descriptorRead(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue)
{
    qDebug() << "descriptorRead " << descriptor.name() << descriptor.uuid() << newValue.toHex(' ');
}

void smartspin2k::characteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue)
{
    Q_UNUSED(characteristic);
    debug("characteristicWritten " + newValue.toHex(' '));
}

void smartspin2k::characteristicRead(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue)
{
    qDebug() << "characteristicRead " << characteristic.uuid() << newValue.toHex(' ');
}

void smartspin2k::serviceScanDone(void)
{
    qDebug() << "serviceScanDone";

    QBluetoothUuid _gattCommunicationChannelServiceId((QString)"77776277-7877-7774-4466-896665500000");

    gattCommunicationChannelService = m_control->createServiceObject(_gattCommunicationChannelServiceId);
    connect(gattCommunicationChannelService, SIGNAL(stateChanged(QLowEnergyService::ServiceState)), this, SLOT(stateChanged(QLowEnergyService::ServiceState)));
    gattCommunicationChannelService->discoverDetails();
}

void smartspin2k::errorService(QLowEnergyService::ServiceError err)
{
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    debug("smartspin2k::errorService" + QString::fromLocal8Bit(metaEnum.valueToKey(err)) + m_control->errorString());
}

void smartspin2k::error(QLowEnergyController::Error err)
{
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    debug("smartspin2k::error" + QString::fromLocal8Bit(metaEnum.valueToKey(err)) + m_control->errorString());
}

void smartspin2k::deviceDiscovered(const QBluetoothDeviceInfo &device)
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

bool smartspin2k::connected()
{
    if(!m_control)
        return false;
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

void* smartspin2k::VirtualBike()
{
    return virtualBike;
}

void* smartspin2k::VirtualDevice()
{
    return VirtualBike();
}

uint16_t smartspin2k::watts()
{
    if(currentCadence().value() == 0) return 0;

    return m_watt.value();
}

void smartspin2k::controllerStateChanged(QLowEnergyController::ControllerState state)
{
    qDebug() << "controllerStateChanged" << state;
    if(state == QLowEnergyController::UnconnectedState && m_control)
    {
        qDebug() << "trying to connect back again...";
        initDone = false;
        m_control->connectToDevice();
    }
}
