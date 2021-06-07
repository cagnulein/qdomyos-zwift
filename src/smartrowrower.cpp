#include "smartrowrower.h"
#include "virtualbike.h"
#include "keepawakehelper.h"
#include <QFile>
#include <QDateTime>
#include <QMetaEnum>
#include <QSettings>
#include <QBluetoothLocalDevice>
#include <math.h>
#include "ios/lockscreen.h"

#ifdef Q_OS_IOS
extern quint8 QZ_EnableDiscoveryCharsAndDescripttors;
#endif

smartrowrower::smartrowrower(bool noWriteResistance, bool noHeartService, uint8_t bikeResistanceOffset, double bikeResistanceGain)
{
#ifdef Q_OS_IOS
    QZ_EnableDiscoveryCharsAndDescripttors = true;
#endif
    m_watt.setType(metric::METRIC_WATT);
    refresh = new QTimer(this);
    this->noWriteResistance = noWriteResistance;
    this->noHeartService = noHeartService;
    this->bikeResistanceGain = bikeResistanceGain;
    this->bikeResistanceOffset = bikeResistanceOffset;
    initDone = false;
    connect(refresh, SIGNAL(timeout()), this, SLOT(update()));
    refresh->start(200);
}

void smartrowrower::writeCharacteristic(uint8_t* data, uint8_t data_len, QString info, bool disable_log, bool wait_for_response)
{
    QEventLoop loop;
    QTimer timeout;

    // if there are some crash here, maybe it's better to use 2 separate event for the characteristicChanged.
    // one for the resistance changed event (spontaneous), and one for the other ones.
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

    if(gattCommunicationChannelService->state() != QLowEnergyService::ServiceState::ServiceDiscovered ||
       m_control->state() == QLowEnergyController::UnconnectedState)
    {
        qDebug() << "writeCharacteristic error because the connection is closed";
        return;
    }

    if(!gattWriteCharacteristic.isValid())
    {
        qDebug() << "gattWriteCharacteristic is invalid";
        return;
    }

    gattCommunicationChannelService->writeCharacteristic(gattWriteCharacteristic, QByteArray((const char*)data, data_len));

    if(!disable_log)
        qDebug() << " >> " + QByteArray((const char*)data, data_len).toHex(' ') + " // " + info;

    loop.exec();
}

void smartrowrower::forceResistance(int8_t requestResistance)
{/*
    uint8_t noOpData[] = { 0xf0, 0xb1, 0x01, 0x00, 0x00 };

    noOpData[3] = requestResistance;

    for(uint8_t i=0; i<sizeof(noOpData)-1; i++)
    {
       noOpData[4] += noOpData[i]; // the last byte is a sort of a checksum
    }

    writeCharacteristic(noOpData, sizeof(noOpData), "force resistance", false, true);*/
}

void smartrowrower::sendPoll()
{
    uint8_t noOpData[] = { 0x24 };
    writeCharacteristic(noOpData, sizeof(noOpData), "noOp", false, true);
}

void smartrowrower::update()
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
    else if(bluetoothDevice.isValid() &&
       m_control->state() == QLowEnergyController::DiscoveredState &&
       gattCommunicationChannelService &&
       gattWriteCharacteristic.isValid() &&
       gattNotify1Characteristic.isValid() &&
       initDone)
    {
        update_metrics(false, watts());

        // sending poll every 2 seconds
        if(sec1Update++ >= (2000 / refresh->interval()))
        {
            sec1Update = 0;
            sendPoll();
            //updateDisplay(elapsed);
        }

        if(requestResistance != -1)
        {
           if(requestResistance > max_resistance) requestResistance = max_resistance;
           else if(requestResistance <= 0) requestResistance = 1;

           if(requestResistance != currentResistance().value())
           {
              qDebug() << "writing resistance " + QString::number(requestResistance);
              forceResistance(requestResistance);
           }
           requestResistance = -1;
        }
        if(requestStart != -1)
        {
           qDebug() << "starting...";

           //btinit();

           requestStart = -1;
           emit bikeStarted();
        }
        if(requestStop != -1)
        {
            qDebug() << "stopping...";
            //writeCharacteristic(initDataF0C800B8, sizeof(initDataF0C800B8), "stop tape");
            requestStop = -1;
        }
    }
}

void smartrowrower::serviceDiscovered(const QBluetoothUuid &gatt)
{
    qDebug() << "serviceDiscovered " + gatt.toString();
}

int smartrowrower::pelotonToBikeResistance(int pelotonResistance)
{
    for(int i = 1; i<max_resistance-1; i++)
    {
        if(bikeResistanceToPeloton(i) <= pelotonResistance && bikeResistanceToPeloton(i+1) >= pelotonResistance)
            return i;
    }
    return Resistance.value();
}

uint8_t smartrowrower::resistanceFromPowerRequest(uint16_t power)
{
    qDebug() << "resistanceFromPowerRequest" << Cadence.value();

    for(int i = 1; i<max_resistance-1; i++)
    {
        if(wattsFromResistance(i) <= power && wattsFromResistance(i+1) >= power)
        {
            qDebug() << "resistanceFromPowerRequest" << wattsFromResistance(i) << wattsFromResistance(i+1) << power;
            return i;
        }
    }
    return Resistance.value();
}

double smartrowrower::bikeResistanceToPeloton(double resistance)
{
    //0,0097x3 - 0,4972x2 + 10,126x - 37,08
    double p = ((pow(resistance,3) * 0.0097) - (0.4972 * pow(resistance, 2)) + (10.126 * resistance) - 37.08);
    if(p < 0)
        p = 0;
    return p;
}

void smartrowrower::characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue)
{
    //qDebug() << "characteristicChanged" << characteristic.uuid() << newValue << newValue.length();
    Q_UNUSED(characteristic);
    QSettings settings;
    QString heartRateBeltName = settings.value("heart_rate_belt_name", "Disabled").toString();

    qDebug() << " << " + newValue.toHex(' ');

    lastPacket = newValue;

    if (newValue.length() != 17)
        return;

    double distance = GetDistanceFromPacket(newValue);
    QTime localTime;

    switch (newValue.at(0)) {
    case 'a':
        // elapsed time
        localTime = QTime(atoi(newValue.mid(6, 2)), atoi(newValue.mid(8, 2)), atoi(newValue.mid(10, 2)));
        break;
    case 'b':
        // work per stroke[6:11] / 10, stroke length [11:14]
        break;
    case 'c':
        // actual power
        m_watt = atoi(newValue.mid(6, 3));
        // average power / 10
        // ignore it
        break;
    case 'd':
        // strokes per minute
        if(settings.value("cadence_sensor_name", "Disabled").toString().startsWith("Disabled"))
            Cadence = atoi(newValue.mid(6, 3)) / 10.0;
        // strokes count [9:11]
        break;
    case 'e':
        // actual split time
        // average split time
        break;
    case 'f':
        // no row newValue.at(5) == '!'
        break;
    case 'x':
        // curve points
        break;
    case 'y':
        // curve points
        break;
    case 'z':
        // curve points
        break;
    }

    Speed = (0.37497622 * ((double)Cadence.value())) / 2.0;
    KCal += ((( (0.048 * ((double)watts()) + 1.19) * settings.value("weight", 75.0).toFloat() * 3.5) / 200.0 ) / (60000.0 / ((double)lastRefreshCharacteristicChanged.msecsTo(QDateTime::currentDateTime())))); //(( (0.048* Output in watts +1.19) * body weight in kg * 3.5) / 200 ) / 60
    //Distance += ((Speed.value() / 3600000.0) * ((double)lastRefreshCharacteristicChanged.msecsTo(QDateTime::currentDateTime())) );
    Distance = distance;

    if(Cadence.value() > 0)
    {
        CrankRevs++;
        LastCrankEventTime += (uint16_t)(1024.0 / (((double)(Cadence.value())) / 60.0));
    }

    lastRefreshCharacteristicChanged = QDateTime::currentDateTime();

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

#ifdef Q_OS_IOS
#ifndef IO_UNDER_QT
    bool cadence = settings.value("bike_cadence_sensor", false).toBool();
    bool ios_peloton_workaround = settings.value("ios_peloton_workaround", true).toBool();
    if(ios_peloton_workaround && cadence && h && firstStateChanged)
    {
        h->virtualbike_setCadence(currentCrankRevolutions(),lastCrankEventTime());
        h->virtualbike_setHeartRate((uint8_t)metrics_override_heartrate());
    }
#endif
#endif

    qDebug() << "Current Local elapsed: " + localTime.toString();
    qDebug() << "Current Speed: " + QString::number(Speed.value());
    qDebug() << "Current Calculate Distance: " + QString::number(Distance.value());
    qDebug() << "Current Cadence: " + QString::number(Cadence.value());
    qDebug() << "Current Distance: " + QString::number(distance);
    qDebug() << "Current CrankRevs: " + QString::number(CrankRevs);
    qDebug() << "Last CrankEventTime: " + QString::number(LastCrankEventTime);
    qDebug() << "Current Watt: " + QString::number(watts());

    if(m_control->error() != QLowEnergyController::NoError)
        qDebug() << "QLowEnergyController ERROR!!" << m_control->errorString();
}

double smartrowrower::GetDistanceFromPacket(QByteArray packet)
{
    uint32_t convertedData = atoi(packet.mid(1, 5));
    double data = ((double)convertedData);
    return data;
}

void smartrowrower::btinit()
{
    uint8_t initData1[] = { 0x0d, 0x56, 0x40, 0x0d };
    writeCharacteristic(initData1, sizeof(initData1), "init", false, true);

    initDone = true;

    if(lastResistanceBeforeDisconnection != -1)
    {
        qDebug() << "forcing resistance to " + QString::number(lastResistanceBeforeDisconnection) + ". It was the last value before the disconnection.";
        forceResistance(lastResistanceBeforeDisconnection);
        lastResistanceBeforeDisconnection = -1;
    }
}

void smartrowrower::stateChanged(QLowEnergyService::ServiceState state)
{
    QBluetoothUuid _gattWriteCharacteristicId((quint16)0x1235);
    QBluetoothUuid _gattNotify1CharacteristicId((quint16)0x1236);

    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceState>();
    qDebug() << "BTLE stateChanged " + QString::fromLocal8Bit(metaEnum.valueToKey(state));

    if(state == QLowEnergyService::ServiceDiscovered)
    {
        //qDebug() << gattCommunicationChannelService->characteristics();

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
                qDebug() << "creating virtual bike interface...";
                virtualBike = new virtualbike(this, noWriteResistance, noHeartService, bikeResistanceOffset, bikeResistanceGain);
                //connect(virtualBike,&virtualbike::debug ,this,&smartrowrower::debug);
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

void smartrowrower::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue)
{
    qDebug() << "descriptorWritten " + descriptor.name() + " " + newValue.toHex(' ');

    initRequest = true;
    emit connectedAndDiscovered();
}

void smartrowrower::characteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue)
{
    Q_UNUSED(characteristic);
    qDebug() << "characteristicWritten " + newValue.toHex(' ');
}

void smartrowrower::serviceScanDone(void)
{
    qDebug() << "serviceScanDone";

    QBluetoothUuid _gattCommunicationChannelServiceId((quint16)0x1234);

    gattCommunicationChannelService = m_control->createServiceObject(_gattCommunicationChannelServiceId);
    connect(gattCommunicationChannelService, SIGNAL(stateChanged(QLowEnergyService::ServiceState)), this, SLOT(stateChanged(QLowEnergyService::ServiceState)));
    gattCommunicationChannelService->discoverDetails();
}

void smartrowrower::errorService(QLowEnergyService::ServiceError err)
{
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    qDebug() << "smartrowrower::errorService" + QString::fromLocal8Bit(metaEnum.valueToKey(err)) + m_control->errorString();
}

void smartrowrower::error(QLowEnergyController::Error err)
{
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    qDebug() << "smartrowrower::error" + QString::fromLocal8Bit(metaEnum.valueToKey(err)) + m_control->errorString();
}

void smartrowrower::deviceDiscovered(const QBluetoothDeviceInfo &device)
{
    qDebug() << "Found new device: " + device.name() + " (" + device.address().toString() + ')';
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
            qDebug() << "Cannot connect to remote device.";
            emit disconnected();
        });
        connect(m_control, &QLowEnergyController::connected, this, [this]() {
            Q_UNUSED(this);
            qDebug() << "Controller connected. Search services...";
            m_control->discoverServices();
        });
        connect(m_control, &QLowEnergyController::disconnected, this, [this]() {
            Q_UNUSED(this);
            qDebug() << "LowEnergy controller disconnected";
            emit disconnected();
        });

        // Connect
        m_control->connectToDevice();
        return;
    }
}

bool smartrowrower::connected()
{
    if(!m_control)
        return false;
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

void* smartrowrower::VirtualBike()
{
    return virtualBike;
}

void* smartrowrower::VirtualDevice()
{
    return VirtualBike();
}

uint16_t smartrowrower::watts()
{
    if(currentCadence().value() == 0) return 0;
    return wattsFromResistance(Resistance.value());
}

uint16_t smartrowrower::wattsFromResistance(double resistance)
{
    // TODO: add lookup table in case we need to add the compatibility with peloton
    return m_watt.value();
}

void smartrowrower::controllerStateChanged(QLowEnergyController::ControllerState state)
{
    qDebug() << "controllerStateChanged" << state;
    if(state == QLowEnergyController::UnconnectedState && m_control)
    {
        lastResistanceBeforeDisconnection = Resistance.value();
        qDebug() << "trying to connect back again...";
        initDone = false;
        m_control->connectToDevice();
    }
}
