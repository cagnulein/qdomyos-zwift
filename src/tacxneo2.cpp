#include "tacxneo2.h"
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

tacxneo2::tacxneo2(bool noWriteResistance, bool noHeartService)
{
    m_watt.setType(metric::METRIC_WATT);
    refresh = new QTimer(this);
    this->noWriteResistance = noWriteResistance;
    this->noHeartService = noHeartService;
    initDone = false;
    connect(refresh, SIGNAL(timeout()), this, SLOT(update()));
    refresh->start(200);
}

void tacxneo2::writeCharacteristic(uint8_t* data, uint8_t data_len, QString info, bool disable_log, bool wait_for_response)
{
    QEventLoop loop;
    QTimer timeout;
    if(wait_for_response)
    {
        connect(gattCustomService, SIGNAL(characteristicChanged(QLowEnergyCharacteristic,QByteArray)),
                &loop, SLOT(quit()));
        timeout.singleShot(300, &loop, SLOT(quit()));
    }
    else
    {
        connect(gattCustomService, SIGNAL(characteristicWritten(QLowEnergyCharacteristic,QByteArray)),
                &loop, SLOT(quit()));
        timeout.singleShot(300, &loop, SLOT(quit()));
    }

    gattCustomService->writeCharacteristic(gattWriteCharCustomId, QByteArray((const char*)data, data_len));

    if(!disable_log)
        debug(" >> " + QByteArray((const char*)data, data_len).toHex(' ') + " // " + info);

    loop.exec();
}

void tacxneo2::changePower(int32_t power)
{
    RequestedPower = power;

    if(power < 0) power = 0;
    uint8_t p[] = { 0xa4, 0x09, 0x4e, 0x05, 0x31, 0xff, 0xff, 0xff, 0xff, 0xd3, 0x4f, 0xff, 0x00 };
    p[9] = (uint8_t)(power & 0xFF);
    p[10] = (uint8_t)(power >> 8);
    for(uint8_t i=1; i<sizeof(p)-1; i++)
    {
       p[12] += p[i]; // the last byte is a sort of a checksum
    }
    p[12]++;

    writeCharacteristic(p, sizeof(p), "changePower", false, false);
}


void tacxneo2::changeInclination(double inclination)
{
    // TODO: inclination for bikes need to be managed on virtual bike interface
    //Inclination = inclination;

    inclination += 200;
    inclination = inclination * 100;
    uint8_t inc[] = { 0xa4, 0x09, 0x4e, 0x05, 0x33, 0xff, 0xff, 0xff, 0xff, 0xd3, 0x4f, 0xff, 0x00 };
    inc[9] = (uint8_t)(((uint16_t)inclination) & 0xFF);
    inc[10] = (uint8_t)(((uint16_t)inclination) >> 8);
    for(uint8_t i=1; i<sizeof(inc)-1; i++)
    {
       inc[12] += inc[i]; // the last byte is a sort of a checksum
    }
    inc[12]++;

    writeCharacteristic(inc, sizeof(inc), "changeInclination", false, false);
}

void tacxneo2::update()
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
        update_metrics(false, watts());

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
}

void tacxneo2::powerPacketReceived(QByteArray b)
{
    /*
    if(gattPowerService)
        writeCharacteristic((uint8_t*)b.constData(), b.length(), "powerPacketReceived bridge", false, false);
    else
        qDebug() << "no power service found" << b;
        */
}

void tacxneo2::serviceDiscovered(const QBluetoothUuid &gatt)
{
    debug("serviceDiscovered " + gatt.toString());
}

void tacxneo2::characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue)
{
    //qDebug() << "characteristicChanged" << characteristic.uuid() << newValue << newValue.length();
    Q_UNUSED(characteristic);
    QSettings settings;
    QString heartRateBeltName = settings.value("heart_rate_belt_name", "Disabled").toString();

    qDebug() << " << char " << characteristic.uuid();
    debug(" << " + newValue.toHex(' '));

    if(characteristic.uuid() == QBluetoothUuid((quint16)0x2A5B))
    {
        lastPacket = newValue;

        uint8_t index = 1;

        if(newValue.at(0) == 0x02 && newValue.length() < 4)
        {
            debug("Crank revolution data present with wrong bytes " + QString::number(newValue.length()));
            return;
        }
        else if(newValue.at(0) == 0x01 && newValue.length() < 6)
        {
            debug("Wheel revolution data present with wrong bytes " + QString::number(newValue.length()));
            return;
        }
        else if(newValue.at(0) == 0x00)
        {
            debug("Cadence sensor notification without datas " + QString::number(newValue.length()));
            return;
        }

        if(newValue.at(0) == 0x02)
            CrankRevsRead = (((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) | (uint16_t)((uint8_t)newValue.at(index)));
        else if(newValue.at(0) == 0x03) {
            index += 6;
            CrankRevsRead = (((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) | (uint16_t)((uint8_t)newValue.at(index)));
        }
        else
        {
            return;
            //CrankRevsRead = (((uint32_t)((uint8_t)newValue.at(index + 3)) << 24) | ((uint32_t)((uint8_t)newValue.at(index + 2)) << 16) | ((uint32_t)((uint8_t)newValue.at(index + 1)) << 8) | (uint32_t)((uint8_t)newValue.at(index)));
        }
        if(newValue.at(0) == 0x01)
            index += 4;
        else
            index += 2;
        LastCrankEventTime = (((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) | (uint16_t)((uint8_t)newValue.at(index)));

        int16_t deltaT = LastCrankEventTime - oldLastCrankEventTime;
        if(deltaT < 0)
        {
            deltaT = LastCrankEventTime + 65535 - oldLastCrankEventTime;
        }

        if(CrankRevsRead != oldCrankRevs && deltaT)
        {
            double cadence = (((double)CrankRevsRead - (double)oldCrankRevs) / (double)deltaT) * 1024.0 * 60.0;
            if(cadence >= 0)
                Cadence = cadence;
            lastGoodCadence = QDateTime::currentDateTime();
        }
        else if(lastGoodCadence.msecsTo(QDateTime::currentDateTime()) > 2000)
        {
            Cadence = 0;
        }

        oldLastCrankEventTime = LastCrankEventTime;
        oldCrankRevs = CrankRevsRead;

        Speed = Cadence.value() * settings.value("cadence_sensor_speed_ratio", 0.33).toDouble();

        Distance += ((Speed.value() / 3600000.0) * ((double)lastRefreshCharacteristicChanged.msecsTo(QDateTime::currentDateTime())) );

        //Resistance = ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) | (uint16_t)((uint8_t)newValue.at(index))));
        //debug("Current Resistance: " + QString::number(Resistance.value()));

        KCal += ((( (0.048 * ((double)watts()) + 1.19) * settings.value("weight", 75.0).toFloat() * 3.5) / 200.0 ) / (60000.0 / ((double)lastRefreshCharacteristicChanged.msecsTo(QDateTime::currentDateTime())))); //(( (0.048* Output in watts +1.19) * body weight in kg * 3.5) / 200 ) / 60
        lastRefreshCharacteristicChanged = QDateTime::currentDateTime();

        debug("Current CrankRevsRead: " + QString::number(CrankRevsRead));
        debug("Last CrankEventTime: " + QString::number(LastCrankEventTime));
        debug("Current Cadence: " + QString::number(Cadence.value()));
        debug("Current Speed: " + QString::number(Speed.value()));
        debug("Current Distance: " + QString::number(Distance.value()));
        debug("Current KCal: " + QString::number(KCal.value()));
    }
    else if(characteristic.uuid() == QBluetoothUuid::HeartRateMeasurement)
    {
        if(newValue.length() > 1)
            Heart = newValue[1];

        debug("Current heart: " + QString::number(Heart.value()));
    }
    else if(characteristic.uuid() == QBluetoothUuid::CyclingPowerMeasurement)
    {
        if(newValue.length() > 3)
            m_watt = (((uint16_t)((uint8_t)newValue.at(3)) << 8) | (uint16_t)((uint8_t)newValue.at(2)));

        debug("Current watt: " + QString::number(m_watt.value()));
    }

#ifdef Q_OS_ANDROID
    if(settings.value("ant_heart", false).toBool())
    {
        Heart = (uint8_t)KeepAwakeHelper::heart();
        debug("Current Heart: " + QString::number(Heart.value()));
    }
#endif
    if(heartRateBeltName.startsWith("Disabled") && Heart.value() == 0)
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


    if(Cadence.value() > 0)
    {
        CrankRevs++;
        LastCrankEventTime += (uint16_t)(1024.0 / (((double)(Cadence.value())) / 60.0));
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

void tacxneo2::stateChanged(QLowEnergyService::ServiceState state)
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

                if(c.properties() & QLowEnergyCharacteristic::Write && c.uuid() == QBluetoothUuid::CyclingPowerControlPoint)
                {
                    qDebug() << "CyclingPowerControlPoint found";
                    //gattWriteCharControlPointId = c;
                    //gattPowerService = s;
                }
                else if(c.properties() & QLowEnergyCharacteristic::Write && c.uuid() == QBluetoothUuid((QString)"6e40fec3-b5a3-f393-e0a9-e50e24dcca9e"))
                {
                    qDebug() << "CustomChar found";
                    gattWriteCharCustomId = c;
                    gattCustomService = s;
                }
            }
        }
    }

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
                virtualBike = new virtualbike(this, noWriteResistance, noHeartService, 4, 1);
                //connect(virtualBike, &virtualbike::powerPacketReceived, this, &tacxneo2::powerPacketReceived);
                //connect(virtualBike,&virtualbike::debug ,this,&tacxneo2::debug);
            }
    }
    firstStateChanged = 1;
    // ********************************************************************************************************
}

void tacxneo2::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue)
{
    debug("descriptorWritten " + descriptor.name() + " " + newValue.toHex(' '));

    initRequest = true;
    emit connectedAndDiscovered();
}

void tacxneo2::descriptorRead(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue)
{
    qDebug() << "descriptorRead " << descriptor.name() << descriptor.uuid() << newValue.toHex(' ');
}

void tacxneo2::characteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue)
{
    Q_UNUSED(characteristic);
    debug("characteristicWritten " + newValue.toHex(' '));
}

void tacxneo2::characteristicRead(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue)
{
    qDebug() << "characteristicRead " << characteristic.uuid() << newValue.toHex(' ');
}

void tacxneo2::serviceScanDone(void)
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

void tacxneo2::errorService(QLowEnergyService::ServiceError err)
{
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    debug("tacxneo2::errorService" + QString::fromLocal8Bit(metaEnum.valueToKey(err)) + m_control->errorString());
}

void tacxneo2::error(QLowEnergyController::Error err)
{
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    debug("tacxneo2::error" + QString::fromLocal8Bit(metaEnum.valueToKey(err)) + m_control->errorString());
}

void tacxneo2::deviceDiscovered(const QBluetoothDeviceInfo &device)
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

bool tacxneo2::connected()
{
    if(!m_control)
        return false;
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

void* tacxneo2::VirtualBike()
{
    return virtualBike;
}

void* tacxneo2::VirtualDevice()
{
    return VirtualBike();
}

uint16_t tacxneo2::watts()
{
    if(currentCadence().value() == 0) return 0;

    return m_watt.value();
}

void tacxneo2::controllerStateChanged(QLowEnergyController::ControllerState state)
{
    qDebug() << "controllerStateChanged" << state;
    if(state == QLowEnergyController::UnconnectedState && m_control)
    {
        qDebug() << "trying to connect back again...";
        initDone = false;
        m_control->connectToDevice();
    }
}
