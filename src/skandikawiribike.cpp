#include "skandikawiribike.h"
#include "virtualbike.h"
#include "keepawakehelper.h"
#include <math.h>
#include <QFile>
#include <QDateTime>
#include <QMetaEnum>
#include <QBluetoothLocalDevice>
#include <QSettings>

skandikawiribike::skandikawiribike(bool noWriteResistance, bool noHeartService, uint8_t bikeResistanceOffset, uint8_t bikeResistanceGain)
{
    m_watt.setType(metric::METRIC_WATT);
    refresh = new QTimer(this);

    this->noWriteResistance = noWriteResistance;
    this->noHeartService = noHeartService;
    this->bikeResistanceGain = bikeResistanceGain;
    this->bikeResistanceOffset = bikeResistanceOffset;

    initDone = false;
    connect(refresh, SIGNAL(timeout()), this, SLOT(update()));
    refresh->start(300);
}

skandikawiribike::~skandikawiribike()
{
    qDebug() << "~skandikawiribike()" << virtualBike;
    if(virtualBike)
        delete virtualBike;
}

void skandikawiribike::writeCharacteristic(uint8_t* data, uint8_t data_len, QString info, bool disable_log, bool wait_for_response)
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

    gattCommunicationChannelService->writeCharacteristic(gattWriteCharacteristic, QByteArray::fromRawData((const char*)data, data_len));

    if(!disable_log)
        debug(" >> " + QByteArray((const char*)data, data_len).toHex(' ') + " // " + info);

    loop.exec();

    if(timeout.isActive() == false)
        debug(" exit for timeout");
}

/*
void skandikawiribike::forceResistance(int8_t requestResistance)
{

}
*/
void skandikawiribike::update()
{
    uint8_t noOpData[] = { 0x20, 0x00, 0x00, 0x00, 0x00 };

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
    else if(bluetoothDevice.isValid() &&
       m_control->state() == QLowEnergyController::DiscoveredState &&
       gattCommunicationChannelService &&
       gattWriteCharacteristic.isValid() &&
       gattNotify1Characteristic.isValid() &&
       initDone)
    {
        QDateTime current = QDateTime::currentDateTime();
        double deltaTime = (((double)lastTimeUpdate.msecsTo(current)) / ((double)1000.0));
        if(currentSpeed().value() > 0.0 && !firstUpdate && !paused)
        {
           elapsed += deltaTime;
           m_watt = (double)watts();
           m_jouls.setValue(m_jouls.value() + (m_watt.value() * deltaTime));
        }
        lastTimeUpdate = current;

        // updating the treadmill console every second
        if(sec1Update++ == (1000 / refresh->interval()))
        {
            sec1Update = 0;
            //updateDisplay(elapsed.value());
        }
        else
        {
            noOpData[1] = 0x01;
            noOpData[2] = lastRequestedResistance().value();
            if(noOpData[2] == 0)
                noOpData[2] = 1;

            for(uint8_t i=0; i<sizeof(noOpData)-1; i++)
            {
               noOpData[4] += noOpData[i]; // the last byte is a sort of a checksum
            }

            writeCharacteristic(noOpData, sizeof(noOpData), "noOp", true, true);
        }

        if(requestResistance != -1)
        {
           if(requestResistance > 32) requestResistance = 32;
           else if(requestResistance < 1) requestResistance = 1;

           if(requestResistance != currentResistance().value())
           {
              debug("writing resistance " + QString::number(requestResistance));
              //forceResistance(requestResistance);
           }
           requestResistance = -1;
        }
        if(requestStart != -1)
        {
           debug("starting...");
           requestStart = -1;
        }
        if(requestStop != -1)
        {
            debug("stopping...");
            requestStop = -1;
        }
    }

    firstUpdate = false;
}

void skandikawiribike::serviceDiscovered(const QBluetoothUuid &gatt)
{
    debug("serviceDiscovered " + gatt.toString());
}

void skandikawiribike::characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue)
{
    //qDebug() << "characteristicChanged" << characteristic.uuid() << newValue << newValue.length();
    Q_UNUSED(characteristic);
    QSettings settings;
    QString heartRateBeltName = settings.value("heart_rate_belt_name", "Disabled").toString();

    debug(" << " + newValue.toHex(' '));

    lastPacket = newValue;
    if (newValue.length() == 5)
    {
        if(newValue.at(2) < 33)
        {
            Resistance = newValue.at(2);
            debug("Current resistance: " + QString::number(Resistance.value()));
        }
        return;
    }
    else if (newValue.length() != 12)
        return;

    if(newValue.at(1) == 0x00)
    {
        double speed = GetSpeedFromPacket(newValue);
        debug("Current speed: " + QString::number(speed));
        Speed = speed;
    }
    else if(newValue.at(1) == 0x10)
    {
        // same position
        double watt = GetSpeedFromPacket(newValue);
        debug("Current watt: " + QString::number(watt));
        m_watts = watt;
    }

    double kcal = GetKcalFromPacket(newValue);

    Cadence = ((uint8_t)newValue.at(10));
    if(Resistance.value() < 1)
    {
        debug("invalid resistance value " + QString::number(Resistance.value()) + " putting to default");
        Resistance = 1;
    }

#ifdef Q_OS_ANDROID
    if(settings.value("ant_heart", false).toBool())
        Heart = (uint8_t)KeepAwakeHelper::heart();
    else
#endif
    {
        if(heartRateBeltName.startsWith("Disabled"))
            Heart = 0;
    }

    if(Cadence.value() > 0)
    {
        CrankRevs++;
        LastCrankEventTime += (uint16_t)(1024.0 / (((double)(Cadence.value())) / 60.0));
    }
    lastRefreshCharacteristicChanged = QDateTime::currentDateTime();

    debug("Current cadence: " + QString::number(Cadence.value()));
    debug("Current heart: " + QString::number(Heart.value()));
    debug("Current KCal: " + QString::number(kcal));
    debug("Current CrankRevs: " + QString::number(CrankRevs));
    debug("Last CrankEventTime: " + QString::number(LastCrankEventTime));
    debug("Current Watt: " + QString::number(watts()));

    if(m_control->error() != QLowEnergyController::NoError)
        qDebug() << "QLowEnergyController ERROR!!" << m_control->errorString();

    KCal = kcal;
    Distance += ((Speed.value() / 3600000.0) * ((double)lastRefreshCharacteristicChanged.msecsTo(QDateTime::currentDateTime())) );
}

double skandikawiribike::GetSpeedFromPacket(QByteArray packet)
{
    uint16_t convertedData = (packet.at(2) << 8) | packet.at(3);
    double data = (double)convertedData / 100.0f;
    return data;
}

double skandikawiribike::GetKcalFromPacket(QByteArray packet)
{
    uint16_t convertedData = (packet.at(6) << 8) | packet.at(7);
    return (double)convertedData;
}

void skandikawiribike::btinit()
{
    uint8_t initData1[] = { 0x40, 0x00, 0x9a, 0x24, 0xfe };

    // in the snoof log it repeats this frame 4 times, i will have to analyze the response to understand if 4 times are enough
    writeCharacteristic(initData1, sizeof(initData1), "init", false, true);

    initDone = true;
}


void skandikawiribike::stateChanged(QLowEnergyService::ServiceState state)
{
    QBluetoothUuid _gattWriteCharacteristicId((quint16)0xfff2);
    QBluetoothUuid _gattNotifyCharacteristicId((quint16)0xfff1);

    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceState>();
    debug("BTLE stateChanged " + QString::fromLocal8Bit(metaEnum.valueToKey(state)));

    if(state == QLowEnergyService::ServiceDiscovered)
    {
        //qDebug() << gattCommunicationChannelService->characteristics();

        gattWriteCharacteristic = gattCommunicationChannelService->characteristic(_gattWriteCharacteristicId);
        gattNotify1Characteristic = gattCommunicationChannelService->characteristic(_gattNotifyCharacteristicId);
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
        if(!firstStateChanged && !virtualBike
        #ifdef Q_OS_IOS
        #ifndef IO_UNDER_QT
                && !h
        #endif
        #endif
        )
        {
            QSettings settings;
            bool virtual_device_enabled = settings.value("virtual_device_enabled", true).toBool();
#ifdef Q_OS_IOS
#ifndef IO_UNDER_QT
            bool cadence = settings.value("bike_cadence_sensor", false).toBool();
            bool ios_peloton_workaround = settings.value("ios_peloton_workaround", true).toBool();
            if(ios_peloton_workaround && cadence)
            {
                qDebug() << "ios_peloton_workaround activated!";
                h = new lockscreen();
                h->virtualbike_ios();
            }
            else
#endif
#endif
                if(virtual_device_enabled)
            {
                debug("creating virtual bike interface...");
                virtualBike = new virtualbike(this, noWriteResistance, noHeartService, bikeResistanceOffset, bikeResistanceGain);
                connect(virtualBike,&virtualbike::debug ,this,&skandikawiribike::debug);
            }
        }
        firstStateChanged = 1;
        // ********************************************************************************************************

        QByteArray descriptor;
        descriptor.append((char)0x01);
        descriptor.append((char)0x00);
        gattCommunicationChannelService->writeDescriptor(gattNotify1Characteristic.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration), descriptor);
    }
}

void skandikawiribike::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue)
{
    debug("descriptorWritten " + descriptor.name() + " " + newValue.toHex(' '));

    initRequest = true;
    emit connectedAndDiscovered();
}

void skandikawiribike::characteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue)
{
    Q_UNUSED(characteristic);
    debug("characteristicWritten " + newValue.toHex(' '));
}

void skandikawiribike::serviceScanDone(void)
{
    debug("serviceScanDone");

    QBluetoothUuid _gattCommunicationChannelServiceId((quint16)0xfff0);

    gattCommunicationChannelService = m_control->createServiceObject(_gattCommunicationChannelServiceId);
    connect(gattCommunicationChannelService, SIGNAL(stateChanged(QLowEnergyService::ServiceState)), this, SLOT(stateChanged(QLowEnergyService::ServiceState)));
    gattCommunicationChannelService->discoverDetails();
}

void skandikawiribike::errorService(QLowEnergyService::ServiceError err)
{
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    debug("skandikawiribike::errorService" + QString::fromLocal8Bit(metaEnum.valueToKey(err)) + m_control->errorString());
}

void skandikawiribike::error(QLowEnergyController::Error err)
{
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    debug("skandikawiribike::error" + QString::fromLocal8Bit(metaEnum.valueToKey(err)) + m_control->errorString());
}

void skandikawiribike::deviceDiscovered(const QBluetoothDeviceInfo &device)
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

bool skandikawiribike::connected()
{
    if(!m_control)
        return false;
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

void* skandikawiribike::VirtualBike()
{
    return virtualBike;
}

void* skandikawiribike::VirtualDevice()
{
    return VirtualBike();
}

uint16_t skandikawiribike::watts()
{
    QSettings settings;
    double v = 0;
    //const uint8_t max_resistance = 15;
    // ref https://translate.google.com/translate?hl=it&sl=en&u=https://support.wattbike.com/hc/en-us/articles/115001881825-Power-Resistance-and-Cadence-Tables&prev=search&pto=aue

    if(currentSpeed().value() <= 0) return 0;

    return m_watts;
}

void skandikawiribike::controllerStateChanged(QLowEnergyController::ControllerState state)
{
    qDebug() << "controllerStateChanged" << state;
    if(state == QLowEnergyController::UnconnectedState && m_control)
    {
        qDebug() << "trying to connect back again...";
        initDone = false;
        m_control->connectToDevice();
    }
}
