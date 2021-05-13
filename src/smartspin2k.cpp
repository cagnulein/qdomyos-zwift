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
}

void smartspin2k::writeCharacteristic(uint8_t* data, uint8_t data_len, QString info, bool disable_log, bool wait_for_response)
{
    QEventLoop loop;
    QTimer timeout;
    if(wait_for_response)
    {
        connect(gattFTMSService, SIGNAL(characteristicChanged(QLowEnergyCharacteristic,QByteArray)),
                &loop, SLOT(quit()));
        timeout.singleShot(300, &loop, SLOT(quit()));
    }
    else
    {
        connect(gattFTMSService, SIGNAL(characteristicWritten(QLowEnergyCharacteristic,QByteArray)),
                &loop, SLOT(quit()));
        timeout.singleShot(300, &loop, SLOT(quit()));
    }

    gattFTMSService->writeCharacteristic(gattWriteCharControlPointId, QByteArray((const char*)data, data_len));

    if(!disable_log)
        debug(" >> " + QByteArray((const char*)data, data_len).toHex(' ') + " // " + info);

    loop.exec();
}

void smartspin2k::forceResistance(int8_t requestResistance)
{
   uint8_t write[] = { FTMS_SET_INDOOR_BIKE_SIMULATION_PARAMS, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

   write[3] = ((uint16_t)(requestResistance - startupResistance) * 100) & 0xFF;
   write[4] = ((uint16_t)(requestResistance - startupResistance) * 100) >> 8;

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
}

void smartspin2k::serviceDiscovered(const QBluetoothUuid &gatt)
{
    debug("serviceDiscovered " + gatt.toString());
}

void smartspin2k::characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue)
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

    if(Flags.instantCadence)
    {
        if(settings.value("cadence_sensor_name", "Disabled").toString().startsWith("Disabled"))
            Cadence = ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) | (uint16_t)((uint8_t)newValue.at(index)))) / 2.0;
        index += 2;
        debug("Current Cadence: " + QString::number(Cadence.value()));
    }

    if(Flags.avgCadence)
    {
        double avgCadence;
        avgCadence = ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) | (uint16_t)((uint8_t)newValue.at(index)))) / 2.0;
        index += 2;
        debug("Current Average Cadence: " + QString::number(avgCadence));
    }

    if(Flags.totDistance)
    {
        Distance = ((double)((((uint32_t)((uint8_t)newValue.at(index + 2)) << 16) | (uint32_t)((uint8_t)newValue.at(index + 1)) << 8) | (uint32_t)((uint8_t)newValue.at(index)))) / 1000.0;
        index += 3;
    }
    else
    {
        Distance += ((Speed.value() / 3600000.0) * ((double)lastRefreshCharacteristicChanged.msecsTo(QDateTime::currentDateTime())) );
    }

    debug("Current Distance: " + QString::number(Distance.value()));

    if(Flags.resistanceLvl)
    {
        Resistance = ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) | (uint16_t)((uint8_t)newValue.at(index))));
        index += 2;
        debug("Current Resistance: " + QString::number(Resistance.value()));
    }

    if(Flags.instantPower)
    {
        m_watt = ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) | (uint16_t)((uint8_t)newValue.at(index))));
        index += 2;
        debug("Current Watt: " + QString::number(m_watt.value()));
    }

    if(Flags.avgPower)
    {
        double avgPower;
        avgPower = ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) | (uint16_t)((uint8_t)newValue.at(index))));
        index += 2;
        debug("Current Average Watt: " + QString::number(avgPower));
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
    h.setKcal(KCal.value());
    h.setDistance(Distance.value());
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

void smartspin2k::stateChanged(QLowEnergyService::ServiceState state)
{
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceState>();
    debug("BTLE stateChanged " + QString::fromLocal8Bit(metaEnum.valueToKey(state)));

    foreach(QLowEnergyService* s, gattCommunicationChannelService)
    {
        qDebug() << "stateChanged" << s->serviceUuid() << s->state();
        if(s->state() != QLowEnergyService::ServiceDiscovered && s->state() != QLowEnergyService::InvalidService)
        {
            qDebug() << "not all services discovered";
            return;
        }
    }

    qDebug() << "all services discovered!";

    foreach(QLowEnergyService* s, gattCommunicationChannelService)
    {
        if(s->state() == QLowEnergyService::ServiceDiscovered)
        {
            // establish hook into notifications
            connect(s, SIGNAL(characteristicChanged(QLowEnergyCharacteristic,QByteArray)),
                    this, SLOT(characteristicChanged(QLowEnergyCharacteristic,QByteArray)));
            connect(s, SIGNAL(characteristicWritten(const QLowEnergyCharacteristic, const QByteArray)),
                    this, SLOT(characteristicWritten(const QLowEnergyCharacteristic, const QByteArray)));
            connect(s, SIGNAL(characteristicRead(const QLowEnergyCharacteristic, const QByteArray)),
                    this, SLOT(characteristicRead(const QLowEnergyCharacteristic, const QByteArray)));
            connect(s, SIGNAL(error(QLowEnergyService::ServiceError)),
                    this, SLOT(errorService(QLowEnergyService::ServiceError)));
            connect(s, SIGNAL(descriptorWritten(const QLowEnergyDescriptor, const QByteArray)), this,
                    SLOT(descriptorWritten(const QLowEnergyDescriptor, const QByteArray)));
            connect(s, SIGNAL(descriptorRead(const QLowEnergyDescriptor, const QByteArray)), this,
                    SLOT(descriptorRead(const QLowEnergyDescriptor, const QByteArray)));

            qDebug() << s->serviceUuid() << "connected!";

            foreach(QLowEnergyCharacteristic c, s->characteristics())
            {
                qDebug() << "char uuid" << c.uuid() << "handle" << c.handle();
                foreach(QLowEnergyDescriptor d, c.descriptors())
                    qDebug() << "descriptor uuid" << d.uuid() << "handle" << d.handle();

                if((c.properties() & QLowEnergyCharacteristic::Notify) == QLowEnergyCharacteristic::Notify)
                {
                    QByteArray descriptor;
                    descriptor.append((char)0x01);
                    descriptor.append((char)0x00);
                    if(c.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration).isValid())
                        s->writeDescriptor(c.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration), descriptor);
                    else
                        qDebug() << "ClientCharacteristicConfiguration" << c.uuid() << c.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration).uuid() << c.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration).handle() << " is not valid";

                    qDebug() << s->serviceUuid() << c.uuid() << "notification subscribed!";
                }
                else if((c.properties() & QLowEnergyCharacteristic::Indicate) == QLowEnergyCharacteristic::Indicate)
                {
                    QByteArray descriptor;
                    descriptor.append((char)0x02);
                    descriptor.append((char)0x00);
                    if(c.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration).isValid())
                        s->writeDescriptor(c.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration), descriptor);
                    else
                        qDebug() << "ClientCharacteristicConfiguration" << c.uuid() << c.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration).uuid() << c.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration).handle() << " is not valid";

                    qDebug() << s->serviceUuid() << c.uuid() << "indication subscribed!";
                }
                else if((c.properties() & QLowEnergyCharacteristic::Read) == QLowEnergyCharacteristic::Read)
                {
                    //s->readCharacteristic(c);
                    //qDebug() << s->serviceUuid() << c.uuid() << "reading!";
                }

                QBluetoothUuid _gattWriteCharControlPointId((quint16)0x2AD9);
                if(c.properties() & QLowEnergyCharacteristic::Write && c.uuid() == _gattWriteCharControlPointId)
                {
                    qDebug() << "FTMS service and Control Point found";
                    gattWriteCharControlPointId = c;
                    gattFTMSService = s;
                }
            }
        }
    }

#if 0
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
                //connect(virtualBike,&virtualbike::debug ,this,&smartspin2k::debug);
            }
    }
#endif
    firstStateChanged = 1;
    // ********************************************************************************************************
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
    debug("serviceScanDone");

#ifdef Q_OS_ANDROID
    QLowEnergyConnectionParameters c;
    c.setIntervalRange(24,40);
    c.setLatency(0);
    c.setSupervisionTimeout(420);
    m_control->requestConnectionUpdate(c);
#endif

    foreach(QBluetoothUuid s, m_control->services())
    {
        gattCommunicationChannelService.append(m_control->createServiceObject(s));
        connect(gattCommunicationChannelService.last(), SIGNAL(stateChanged(QLowEnergyService::ServiceState)), this, SLOT(stateChanged(QLowEnergyService::ServiceState)));
        gattCommunicationChannelService.last()->discoverDetails();
    }
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
