#include "schwinnic4bike.h"

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

schwinnic4bike::schwinnic4bike(bool noWriteResistance, bool noHeartService)
{
    refresh = new QTimer(this);
    this->noWriteResistance = noWriteResistance;
    this->noHeartService = noHeartService;
    initDone = false;
    connect(refresh, SIGNAL(timeout()), this, SLOT(update()));
    refresh->start(200);
}
/*
void schwinnic4bike::writeCharacteristic(uint8_t* data, uint8_t data_len, QString info, bool disable_log, bool wait_for_response)
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

void schwinnic4bike::update()
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
    else if(bluetoothDevice.isValid() &&
       m_control->state() == QLowEnergyController::DiscoveredState //&&
       //gattCommunicationChannelService &&
       //gattWriteCharacteristic.isValid() &&
       //gattNotify1Characteristic.isValid() &&
       /*initDone*/)
    {
        QDateTime current = QDateTime::currentDateTime();
        double deltaTime = (((double)lastTimeUpdate.msecsTo(current)) / ((double)1000.0));
        if(currentSpeed().value() > 0.0 && !firstUpdate && !paused)
        {
           elapsed += deltaTime;
           m_watt = (double)watts();
           m_jouls += (m_watt.value() * deltaTime);
        }
        lastTimeUpdate = current;

        // updating the treadmill console every second
        if(sec1Update++ == (500 / refresh->interval()))
        {
            sec1Update = 0;
            //updateDisplay(elapsed);
        }

        if(requestResistance != -1)
        {
           if(requestResistance > 15) requestResistance = 15;
           else if(requestResistance == 0) requestResistance = 1;

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

           //btinit();

           requestStart = -1;
           emit bikeStarted();
        }
        if(requestStop != -1)
        {
            debug("stopping...");
            //writeCharacteristic(initDataF0C800B8, sizeof(initDataF0C800B8), "stop tape");
            requestStop = -1;
        }
    }

    firstUpdate = false;
}

void schwinnic4bike::serviceDiscovered(const QBluetoothUuid &gatt)
{
    debug("serviceDiscovered " + gatt.toString());
}

void schwinnic4bike::characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue)
{
    //qDebug() << "characteristicChanged" << characteristic.uuid() << newValue << newValue.length();
    Q_UNUSED(characteristic);
    QSettings settings;
    QString heartRateBeltName = settings.value("heart_rate_belt_name", "Disabled").toString();

    debug(" << " + newValue.toHex(' '));

    if(characteristic.uuid() != QBluetoothUuid((quint16)0x2AD2))
        return;

    lastPacket = newValue;

    union flags
    {
       struct
       {
          uint16_t moreData         :1;
          uint16_t avgSpeed         :1;
          uint16_t instantCadence   :1;
          uint16_t avgCadence       :1;
          uint16_t totDistance      :1;
          uint16_t resistanceLvl    :1;
          uint16_t instantPower     :1;
          uint16_t avgPower         :1;
          uint16_t expEnergy        :1;
          uint16_t heartRate        :1;
          uint16_t metabolic        :1;
          uint16_t elapsedTime      :1;
          uint16_t remainingTime    :1;
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
        Speed = ((double)((newValue.at(index + 1) << 8) | newValue.at(index))) / 100.0;
        index += 2;
        debug("Current Speed: " + QString::number(Speed.value()));
    }

    if(Flags.avgSpeed)
    {
        double avgSpeed;
        avgSpeed = ((double)((newValue.at(index + 1) << 8) | newValue.at(index))) / 100.0;
        index += 2;
        debug("Current Average Speed: " + QString::number(avgSpeed));
    }

    if(Flags.instantCadence)
    {
        Cadence = ((double)((newValue.at(index + 1) << 8) | newValue.at(index))) / 2.0;
        index += 2;
        debug("Current Cadence: " + QString::number(Cadence.value()));
    }

    if(Flags.avgCadence)
    {
        double avgCadence;
        avgCadence = ((double)((newValue.at(index + 1) << 8) | newValue.at(index))) / 2.0;
        index += 2;
        debug("Current Average Cadence: " + QString::number(avgCadence));
    }

    if(Flags.totDistance)
    {
        Distance = ((double)((newValue.at(index + 2) << 16) | (newValue.at(index + 1) << 8) | newValue.at(index))) / 1000.0;
        index += 3;
    }
    else
    {
        Distance += ((Speed.value() / 3600000.0) * ((double)lastRefreshCharacteristicChanged.msecsTo(QDateTime::currentDateTime())) );
    }

    debug("Current Distance: " + QString::number(Distance.value()));

    if(Flags.resistanceLvl)
    {
        Resistance = ((double)((newValue.at(index + 1) << 8) | newValue.at(index)));
        index += 2;
        debug("Current Resistance: " + QString::number(Resistance.value()));
    }

    if(Flags.instantPower)
    {
        m_watt = ((double)((newValue.at(index + 1) << 8) | newValue.at(index)));
        index += 2;
        debug("Current Watt: " + QString::number(m_watt.value()));
    }

    if(Flags.avgPower)
    {
        double avgPower;
        avgPower = ((double)((newValue.at(index + 1) << 8) | newValue.at(index)));
        index += 2;
        debug("Current Average Watt: " + QString::number(avgPower));
    }

    if(Flags.expEnergy)
    {
        KCal = ((double)((newValue.at(index + 1) << 8) | newValue.at(index)));
        index += 2;

        // energy per hour
        index += 2;

        // energy per minute
        index += 1;
    }
    else
    {
        KCal += ((( (0.048 * ((double)watts()) + 1.19) * settings.value("weight", 75.0).toFloat() * 3.5) / 200.0 ) / (60000.0 / ((double)lastRefreshCharacteristicChanged.msecsTo(QDateTime::currentDateTime())))); //(( (0.048* Output in watts +1.19) * body weight in kg * 3.5) / 200 ) / 60
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
            Heart = ((double)((newValue.at(index))));
            index += 1;
            debug("Current Heart: " + QString::number(Heart.value()));
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

    if(Cadence.value() > 0)
    {
        CrankRevs++;
        LastCrankEventTime += (uint16_t)(1024.0 / (((double)(Cadence.value())) / 60.0));
    }

    lastRefreshCharacteristicChanged = QDateTime::currentDateTime();

    if(heartRateBeltName.startsWith("Disabled"))
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


    debug("Current CrankRevs: " + QString::number(CrankRevs));
    debug("Last CrankEventTime: " + QString::number(LastCrankEventTime));

    if(m_control->error() != QLowEnergyController::NoError)
        qDebug() << "QLowEnergyController ERROR!!" << m_control->errorString();
}

void schwinnic4bike::stateChanged(QLowEnergyService::ServiceState state)
{
    if(state != QLowEnergyService::ServiceDiscovered) return;

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
                virtualBike = new virtualbike(this, noWriteResistance, noHeartService);
                connect(virtualBike,&virtualbike::debug ,this,&schwinnic4bike::debug);
            }
    }
    firstStateChanged = 1;
    // ********************************************************************************************************
}

void schwinnic4bike::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue)
{
    debug("descriptorWritten " + descriptor.name() + " " + newValue.toHex(' '));

    initRequest = true;
    emit connectedAndDiscovered();
}

void schwinnic4bike::descriptorRead(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue)
{
    qDebug() << "descriptorRead " << descriptor.name() << descriptor.uuid() << newValue.toHex(' ');
}

void schwinnic4bike::characteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue)
{
    Q_UNUSED(characteristic);
    debug("characteristicWritten " + newValue.toHex(' '));
}

void schwinnic4bike::characteristicRead(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue)
{
    qDebug() << "characteristicRead " << characteristic.uuid() << newValue.toHex(' ');
}

void schwinnic4bike::serviceScanDone(void)
{
    debug("serviceScanDone");

    QByteArray descriptor;
    descriptor.append((char)0x01);
    descriptor.append((char)0x00);

    gattCommunicationChannelService = m_control->createServiceObject(QBluetoothUuid((quint16)0x1826));

    QSharedPointer<QLowEnergyServicePrivate> qzService = gattCommunicationChannelService->d_ptr;
    m_control->d_ptr->writeDescriptor(qzService, 0x30, 0x31, descriptor);
}

void schwinnic4bike::errorService(QLowEnergyService::ServiceError err)
{
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    debug("schwinnic4bike::errorService" + QString::fromLocal8Bit(metaEnum.valueToKey(err)) + m_control->errorString());
}

void schwinnic4bike::error(QLowEnergyController::Error err)
{
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    debug("schwinnic4bike::error" + QString::fromLocal8Bit(metaEnum.valueToKey(err)) + m_control->errorString());
}

void schwinnic4bike::deviceDiscovered(const QBluetoothDeviceInfo &device)
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

bool schwinnic4bike::connected()
{
    if(!m_control)
        return false;
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

void* schwinnic4bike::VirtualBike()
{
    return virtualBike;
}

void* schwinnic4bike::VirtualDevice()
{
    return VirtualBike();
}

uint16_t schwinnic4bike::watts()
{
    if(currentCadence().value() == 0) return 0;

    return m_watt.value();
}

void schwinnic4bike::controllerStateChanged(QLowEnergyController::ControllerState state)
{
    qDebug() << "controllerStateChanged" << state;
    if(state == QLowEnergyController::UnconnectedState && m_control)
    {
        qDebug() << "trying to connect back again...";
        initDone = false;
        m_control->connectToDevice();
    }
}

