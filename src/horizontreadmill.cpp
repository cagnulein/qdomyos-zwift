#include "horizontreadmill.h"

#include "virtualtreadmill.h"
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

horizontreadmill::horizontreadmill(bool noWriteResistance, bool noHeartService)
{
    m_watt.setType(metric::METRIC_WATT);
    refresh = new QTimer(this);
    this->noWriteResistance = noWriteResistance;
    this->noHeartService = noHeartService;
    initDone = false;
    connect(refresh, SIGNAL(timeout()), this, SLOT(update()));
    refresh->start(200);
}
/*
void horizontreadmill::writeCharacteristic(uint8_t* data, uint8_t data_len, QString info, bool disable_log, bool wait_for_response)
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
}*/

void horizontreadmill::update()
{
    if(m_control->state() == QLowEnergyController::UnconnectedState)
    {
        emit disconnected();
        return;
    }

    if(initRequest)
    {
        initRequest = false;
    }
    else if(bluetoothDevice.isValid() //&&
       //m_control->state() == QLowEnergyController::DiscoveredState //&&
       //gattCommunicationChannelService &&
       //gattWriteCharacteristic.isValid() &&
       //gattNotify1Characteristic.isValid() &&
       /*initDone*/)
    {
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
        if(sec1Update++ == (500 / refresh->interval()))
        {
            sec1Update = 0;
            //updateDisplay(elapsed);
        }

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
        if(requestIncreaseFan != -1)
        {
            debug("increasing fan speed...");
            //sendChangeFanSpeed(FanSpeed + 1);
            requestIncreaseFan = -1;
        }
        else if(requestDecreaseFan != -1)
        {
            debug("decreasing fan speed...");
            //sendChangeFanSpeed(FanSpeed - 1);
            requestDecreaseFan = -1;
        }

        elevationAcc += (currentSpeed().value() / 3600.0) * 1000.0 * (currentInclination().value() / 100.0) * deltaTime;
    }

    firstUpdate = false;
}

void horizontreadmill::forceSpeedOrIncline(double requestSpeed, double requestIncline)
{
    // TODO
}

void horizontreadmill::serviceDiscovered(const QBluetoothUuid &gatt)
{
    debug("serviceDiscovered " + gatt.toString());
}

void horizontreadmill::characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue)
{
    double heart;
    //qDebug() << "characteristicChanged" << characteristic.uuid() << newValue << newValue.length();
    Q_UNUSED(characteristic);
    QSettings settings;
    QString heartRateBeltName = settings.value("heart_rate_belt_name", "Disabled").toString();

    debug(" << " + newValue.toHex(' '));

    if(characteristic.uuid() != QBluetoothUuid((quint16)0x2ACD))
        return;

    lastPacket = newValue;

    // default flags for this treadmill is 84 04

    union flags
    {
       struct
       {
          uint16_t moreData         :1;
          uint16_t avgSpeed         :1;
          uint16_t totalDistance    :1;
          uint16_t inclination      :1;
          uint16_t elevation        :1;
          uint16_t instantPace      :1;
          uint16_t averagePace      :1;
          uint16_t expEnergy        :1;
          uint16_t heartRate        :1;
          uint16_t metabolic        :1;
          uint16_t elapsedTime      :1;
          uint16_t remainingTime    :1;
          uint16_t forceBelt        :1;
          uint16_t spare            :3;
       };

       uint16_t word_flags;
    };

    flags Flags;
    int index = 0;
    Flags.word_flags = (newValue.at(1) << 8) | newValue.at(0);
    index += 2;

    if(!Flags.moreData)
    {
        Speed = ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) | (uint16_t)((uint8_t)newValue.at(index)))) / 100.0;
        index += 2;
        debug("Current Speed: " + QString::number(Speed.value()));
    }

    if(Flags.avgSpeed)
    {
        double avgSpeed;
        avgSpeed = ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) | (uint16_t)((uint8_t)newValue.at(index)))) / 100.0;
        index += 2;
        debug("Current Average Speed: " + QString::number(avgSpeed));
    }

    if(Flags.totalDistance)
    {
        // ignoring the distance, because it's a total life odometer
        //Distance = ((double)((((uint32_t)((uint8_t)newValue.at(index + 2)) << 16) | (uint32_t)((uint8_t)newValue.at(index + 1)) << 8) | (uint32_t)((uint8_t)newValue.at(index)))) / 1000.0;
        index += 3;
    }
    //else
    {
        Distance += ((Speed.value() / 3600000.0) * ((double)lastRefreshCharacteristicChanged.msecsTo(QDateTime::currentDateTime())) );
    }

    debug("Current Distance: " + QString::number(Distance.value()));

    if(Flags.inclination)
    {
        Inclination = ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) | (uint16_t)((uint8_t)newValue.at(index)))) / 10.0;
        index += 4; // the ramo value is useless
        debug("Current Inclination: " + QString::number(Inclination.value()));
    }

    if(Flags.elevation)
    {
        index += 4; // TODO
    }

    if(Flags.instantPace)
    {
        index += 1; // TODO
    }

    if(Flags.averagePace)
    {
        index += 1; // TODO
    }

    if(Flags.expEnergy)
    {
        KCal = ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) | (uint16_t)((uint8_t)newValue.at(index))));
        index += 2;

        // energy per hour
        index += 2;

        // energy per minute
        index += 1;
    }
    else
    {
        KCal += ((( (0.048 * ((double)watts(settings.value("weight", 75.0).toFloat())) + 1.19) * 3.5) / 200.0 ) / (60000.0 / ((double)lastRefreshCharacteristicChanged.msecsTo(QDateTime::currentDateTime())))); //(( (0.048* Output in watts +1.19) * body weight in kg * 3.5) / 200 ) / 60
    }

    debug("Current KCal: " + QString::number(KCal.value()));

#ifdef Q_OS_ANDROID
    if(settings.value("ant_heart", false).toBool())
        Heart = (uint8_t)KeepAwakeHelper::heart();
    else
#endif
    {
        if(Flags.heartRate)
        {
            if(index < newValue.length())
            {
                heart = ((double)((newValue.at(index))));
                debug("Current Heart: " + QString::number(heart));
            }
            else
                debug("Error on parsing heart!");
            index += 1;            
        }
    }

    if(Flags.metabolic)
    {
        // todo
    }

    if(Flags.elapsedTime)
    {
        // todo
    }

    if(Flags.remainingTime)
    {
        // todo
    }

    if(Flags.forceBelt)
    {
        // todo
    }

    if(heartRateBeltName.startsWith("Disabled"))
    {
        if(heart == 0)
        {
#ifdef Q_OS_IOS
#ifndef IO_UNDER_QT
            lockscreen h;
        long appleWatchHeartRate = h.heartRate();
        Heart = appleWatchHeartRate;
        debug("Current Heart from Apple Watch: " + QString::number(appleWatchHeartRate));
#endif
#endif
        }
        else
        {
            Heart = heart;
        }
    }

    lastRefreshCharacteristicChanged = QDateTime::currentDateTime();

    if(m_control->error() != QLowEnergyController::NoError)
        qDebug() << "QLowEnergyController ERROR!!" << m_control->errorString();
}

void horizontreadmill::stateChanged(QLowEnergyService::ServiceState state)
{
    if(state != QLowEnergyService::ServiceDiscovered) return;

    QBluetoothUuid _gattNotify1CharacteristicId((quint16)0x2ACD);
    gattNotify1Characteristic = gattCommunicationChannelService->characteristic(_gattNotify1CharacteristicId);

    if(!gattNotify1Characteristic.isValid())
    {
        qDebug() << "invalid characteristic";
        return;
    }

    qDebug() << state;

    QByteArray descriptor;
    descriptor.append((char)0x01);
    descriptor.append((char)0x00);

    gattCommunicationChannelService->writeDescriptor(gattNotify1Characteristic.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration), descriptor);

    connect(gattCommunicationChannelService, SIGNAL(characteristicChanged(QLowEnergyCharacteristic,QByteArray)),
            this, SLOT(characteristicChanged(QLowEnergyCharacteristic,QByteArray)));

    initRequest = false;
    emit connectedAndDiscovered();


    // ******************************************* virtual treadmill init *************************************
    if(!firstStateChanged && !virtualTreadmill
        #ifdef Q_OS_IOS
        #ifndef IO_UNDER_QT
            && !h
        #endif
        #endif
            )
    {
        QSettings settings;
        bool virtual_device_enabled = settings.value("virtual_device_enabled", true).toBool();
            if(virtual_device_enabled)
            {
                debug("creating virtual treadmill interface...");
                virtualTreadmill = new virtualtreadmill(this, noHeartService);
                connect(virtualTreadmill,&virtualtreadmill::debug ,this,&horizontreadmill::debug);
            }
    }
    firstStateChanged = 1;
    // ********************************************************************************************************
}

void horizontreadmill::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue)
{
    debug("descriptorWritten " + descriptor.name() + " " + newValue.toHex(' '));

    initRequest = true;
    emit connectedAndDiscovered();
}

void horizontreadmill::descriptorRead(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue)
{
    qDebug() << "descriptorRead " << descriptor.name() << descriptor.uuid() << newValue.toHex(' ');
}

void horizontreadmill::characteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue)
{
    Q_UNUSED(characteristic);
    debug("characteristicWritten " + newValue.toHex(' '));
}

void horizontreadmill::characteristicRead(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue)
{
    qDebug() << "characteristicRead " << characteristic.uuid() << newValue.toHex(' ');
}

void horizontreadmill::serviceScanDone(void)
{
    debug("serviceScanDone");

    gattCommunicationChannelService = m_control->createServiceObject(QBluetoothUuid((quint16)0x1826));
    connect(gattCommunicationChannelService, SIGNAL(stateChanged(QLowEnergyService::ServiceState)), this, SLOT(stateChanged(QLowEnergyService::ServiceState)));
    gattCommunicationChannelService->discoverDetails();
}

void horizontreadmill::errorService(QLowEnergyService::ServiceError err)
{
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    debug("horizontreadmill::errorService" + QString::fromLocal8Bit(metaEnum.valueToKey(err)) + m_control->errorString());
}

void horizontreadmill::error(QLowEnergyController::Error err)
{
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    debug("horizontreadmill::error" + QString::fromLocal8Bit(metaEnum.valueToKey(err)) + m_control->errorString());
}

void horizontreadmill::deviceDiscovered(const QBluetoothDeviceInfo &device)
{
    // ***************************************************************************************************************
    // horizon treadmill and F80 treadmill, so if we want to add inclination support we have to separate the 2 devices
    // ***************************************************************************************************************
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

bool horizontreadmill::connected()
{
    if(!m_control)
        return false;
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

void* horizontreadmill::VirtualTreadmill()
{
    return virtualTreadmill;
}

void* horizontreadmill::VirtualDevice()
{
    return VirtualTreadmill();
}

void horizontreadmill::controllerStateChanged(QLowEnergyController::ControllerState state)
{
    qDebug() << "controllerStateChanged" << state;
    if(state == QLowEnergyController::UnconnectedState && m_control)
    {
        qDebug() << "trying to connect back again...";
        initDone = false;
        m_control->connectToDevice();
    }
}

