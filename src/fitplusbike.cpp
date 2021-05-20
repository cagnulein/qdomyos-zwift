#include "fitplusbike.h"
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

fitplusbike::fitplusbike(bool noWriteResistance, bool noHeartService, uint8_t bikeResistanceOffset, double bikeResistanceGain)
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

void fitplusbike::writeCharacteristic(uint8_t* data, uint8_t data_len, QString info, bool disable_log, bool wait_for_response)
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

void fitplusbike::forceResistance(int8_t requestResistance)
{
    /*uint8_t noOpData[] = { 0xf0, 0xb1, 0x01, 0x00, 0x00 };

    noOpData[3] = requestResistance;

    for(uint8_t i=0; i<sizeof(noOpData)-1; i++)
    {
       noOpData[4] += noOpData[i]; // the last byte is a sort of a checksum
    }

    writeCharacteristic(noOpData, sizeof(noOpData), "force resistance", false, true);*/
}

void fitplusbike::update()
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
        QSettings settings;
        update_metrics(true, watts());

        if(Heart.value() > 0)
        {
            int avgP = ((settings.value("power_hr_pwr1", 200).toDouble() * settings.value("power_hr_hr2",170).toDouble()) - (settings.value("power_hr_pwr2",230).toDouble() * settings.value("power_hr_hr1",150).toDouble())) / (settings.value("power_hr_hr2",170).toDouble() - settings.value("power_hr_hr1",150).toDouble()) + (Heart.value() * ((settings.value("power_hr_pwr1",200).toDouble() - settings.value("power_hr_pwr2",230).toDouble()) / (settings.value("power_hr_hr1",150).toDouble() - settings.value("power_hr_hr2",170).toDouble())));
            if (avgP < 50)
                avgP = 50;
            m_watt = avgP;
            qDebug() << "Current Watt: " + QString::number(m_watt.value());
        }

        // sending poll every 2 seconds
        if(sec1Update++ >= (2000 / refresh->interval()))
        {
            sec1Update = 0;
            //updateDisplay(elapsed);
        }

        uint8_t noOpData[] = { 0x02, 0x42, 0x42, 0x03 };
        uint8_t noOpData1[] = { 0x02, 0x43, 0x01, 0x42, 0x03 };
        switch(counterPoll)
        {
        case 0:
            writeCharacteristic(noOpData, sizeof(noOpData), "noOp", false, true);
            break;
        default:
            writeCharacteristic(noOpData1, sizeof(noOpData1), "noOp", false, true);
            break;
        }

        counterPoll++;
        if(counterPoll > 1)
            counterPoll = 0;

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

void fitplusbike::serviceDiscovered(const QBluetoothUuid &gatt)
{
    qDebug() << "serviceDiscovered " + gatt.toString();
}

void fitplusbike::characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue)
{
    //qDebug() << "characteristicChanged" << characteristic.uuid() << newValue << newValue.length();
    Q_UNUSED(characteristic);
    QSettings settings;
    QString heartRateBeltName = settings.value("heart_rate_belt_name", "Disabled").toString();

    qDebug() << " << " + newValue.toHex(' ');

    lastPacket = newValue;

    if (newValue.length() != 14)
        return;

    /*if ((uint8_t)(newValue.at(0)) != 0xf0 && (uint8_t)(newValue.at(1)) != 0xd1)
        return;*/

    Resistance = 1;
    m_pelotonResistance = 1;
    emit resistanceRead(Resistance.value());
    if(settings.value("cadence_sensor_name", "Disabled").toString().startsWith("Disabled"))
        Cadence = ((uint8_t)newValue.at(8));
    if(!settings.value("speed_power_based", false).toBool())
        Speed = (double)((((uint8_t)newValue.at(7)) << 8) | ((uint8_t)newValue.at(6))) / 10.0;
    else
        Speed = metric::calculateSpeedFromPower(m_watt.value());
    KCal += ((( (0.048 * ((double)watts()) + 1.19) * settings.value("weight", 75.0).toFloat() * 3.5) / 200.0 ) / (60000.0 / ((double)lastRefreshCharacteristicChanged.msecsTo(QDateTime::currentDateTime())))); //(( (0.048* Output in watts +1.19) * body weight in kg * 3.5) / 200 ) / 60
    Distance += ((Speed.value() / 3600000.0) * ((double)lastRefreshCharacteristicChanged.msecsTo(QDateTime::currentDateTime())) );

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
        h->virtualbike_setHeartRate((uint8_t)currentHeart().value());
    }
#endif
#endif

    qDebug() << "Current Speed: " + QString::number(Speed.value());
    qDebug() << "Current Calculate Distance: " + QString::number(Distance.value());
    qDebug() << "Current Cadence: " + QString::number(Cadence.value());
    qDebug() << "Current CrankRevs: " + QString::number(CrankRevs);
    qDebug() << "Last CrankEventTime: " + QString::number(LastCrankEventTime);
    qDebug() << "Current Watt: " + QString::number(watts());

    if(m_control->error() != QLowEnergyController::NoError)
        qDebug() << "QLowEnergyController ERROR!!" << m_control->errorString();
}

void fitplusbike::btinit()
{
    uint8_t initData1[] = { 0x02, 0x44, 0x01, 0x45, 0x03 };
    uint8_t initData2[] = { 0x02, 0x44, 0x0b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4f, 0x03 };
    uint8_t initData3[] = { 0x02, 0x44, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x46, 0xaa, 0x19, 0x00, 0xbb, 0x03 };
    uint8_t initData4[] = { 0x02, 0x44, 0x02, 0x46, 0x03 };
    uint8_t initData5[] = { 0x02, 0x41, 0x02, 0x43, 0x03 };

    writeCharacteristic(initData1, sizeof(initData1), "init", false, true);
    writeCharacteristic(initData2, sizeof(initData2), "init", false, true);
    writeCharacteristic(initData3, sizeof(initData3), "init", false, true);
    writeCharacteristic(initData4, sizeof(initData4), "init", false, true);
    writeCharacteristic(initData5, sizeof(initData5), "init", false, true);

    initDone = true;

    if(lastResistanceBeforeDisconnection != -1)
    {
        qDebug() << "forcing resistance to " + QString::number(lastResistanceBeforeDisconnection) + ". It was the last value before the disconnection.";
        forceResistance(lastResistanceBeforeDisconnection);
        lastResistanceBeforeDisconnection = -1;
    }
}

void fitplusbike::stateChanged(QLowEnergyService::ServiceState state)
{
    QBluetoothUuid _gattWriteCharacteristicId((quint16)0xfff2);
    QBluetoothUuid _gattNotify1CharacteristicId((quint16)0xfff1);

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
                //connect(virtualBike,&virtualbike::debug ,this,&fitplusbike::debug);
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

void fitplusbike::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue)
{
    qDebug() << "descriptorWritten " + descriptor.name() + " " + newValue.toHex(' ');

    initRequest = true;
    emit connectedAndDiscovered();
}

void fitplusbike::characteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue)
{
    Q_UNUSED(characteristic);
    qDebug() << "characteristicWritten " + newValue.toHex(' ');
}

void fitplusbike::serviceScanDone(void)
{
    qDebug() << "serviceScanDone";

    QBluetoothUuid _gattCommunicationChannelServiceId((quint16)0xfff0);

    gattCommunicationChannelService = m_control->createServiceObject(_gattCommunicationChannelServiceId);
    connect(gattCommunicationChannelService, SIGNAL(stateChanged(QLowEnergyService::ServiceState)), this, SLOT(stateChanged(QLowEnergyService::ServiceState)));
    gattCommunicationChannelService->discoverDetails();
}

void fitplusbike::errorService(QLowEnergyService::ServiceError err)
{
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    qDebug() << "fitplusbike::errorService" + QString::fromLocal8Bit(metaEnum.valueToKey(err)) + m_control->errorString();
}

void fitplusbike::error(QLowEnergyController::Error err)
{
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    qDebug() << "fitplusbike::error" + QString::fromLocal8Bit(metaEnum.valueToKey(err)) + m_control->errorString();
}

void fitplusbike::deviceDiscovered(const QBluetoothDeviceInfo &device)
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

bool fitplusbike::connected()
{
    if(!m_control)
        return false;
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

void* fitplusbike::VirtualBike()
{
    return virtualBike;
}

void* fitplusbike::VirtualDevice()
{
    return VirtualBike();
}

uint16_t fitplusbike::watts()
{
    if(currentCadence().value() == 0) return 0;

    return m_watt.value();
}

void fitplusbike::controllerStateChanged(QLowEnergyController::ControllerState state)
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
