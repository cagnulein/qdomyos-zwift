#include "flywheelbike.h"
#include "virtualbike.h"
#include <QFile>
#include <QDateTime>
#include <QMetaEnum>
#include <QSettings>
#include <QBluetoothLocalDevice>
#include <math.h>
#include "ios/lockscreen.h"
#include "keepawakehelper.h"

flywheelbike::flywheelbike(bool noWriteResistance, bool noHeartService)
{
    m_watt.setType(metric::METRIC_WATT);
    refresh = new QTimer(this);
    this->noWriteResistance = noWriteResistance;
    this->noHeartService = noHeartService;
    initDone = false;
    connect(refresh, SIGNAL(timeout()), this, SLOT(update()));
    refresh->start(200);
}

void flywheelbike::writeCharacteristic(uint8_t* data, uint8_t data_len, QString info, bool disable_log, bool wait_for_response)
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

void flywheelbike::update()
{
    qDebug() << m_control->state() << bluetoothDevice.isValid() <<
                gattCommunicationChannelService <<
                gattWriteCharacteristic.isValid() <<
                gattNotify1Characteristic.isValid() <<
                initDone;

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
       gattNotify1Characteristic.isValid() &&
       initDone)
    {
        update_metrics(true, watts());

        // updating the treadmill console every second
        /*if(sec1Update++ == (500 / refresh->interval()))
        {
            sec1Update = 0;
            //updateDisplay(elapsed);
        }*/

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

void flywheelbike::serviceDiscovered(const QBluetoothUuid &gatt)
{
    debug("serviceDiscovered " + gatt.toString());
}

void flywheelbike::flushDataframe(BikeDataframe *dataFrame)
{
    memset(dataFrame->buffer, 0, 256);

    dataFrame->crc = 0;
    dataFrame->len = 0;
    dataFrame->message_id = 0;
}

void flywheelbike::decodeReceivedData(QByteArray buffer)
{
    for (int i = 0; i < buffer.length(); ++i) {
        int8_t b = buffer[i];

        qDebug() << "rxState" << rxState << b;
        switch (rxState) {
            case WFSYNC_1:
                if (b == -1) {
                    flushDataframe(&bikeData);
                    errorState = MSG_NO_ERROR;
                    rxState = WFLENGTH;
                } else {
                    errorState = MSG_NO_ERROR;
                    rxState = WFSYNC_1;
                }
                break;
            case WFLENGTH:
                if (b == 0) {
                    errorState = MSG_WFSIZE;
                    rxState = WFSYNC_1;
                } else {
                    dataPacketIndex = 0;
                    dataPacketLength = b;
                    bikeData.len = b - 2;
                    rxState = WFID;
                }
                break;
            case WFID:
                if (b >= 0) {
                    bikeData.message_id = b;
                    if (dataPacketLength != 0) {
                        rxState = DATA;
                    } else {
                        rxState = CHECKSUM;
                    }
                } else {
                    errorState = MSG_UNKNOWN_ID;
                    rxState = WFSYNC_1;
                }
                break;
            case DATA:
                if (dataPacketLength == 0) {
                    rxState = CHECKSUM;
                }

                bikeData.buffer[dataPacketIndex] = b;
                dataPacketIndex++;
                dataPacketLength--;
                break;
            case CHECKSUM:
                /* TODO: Implement checksum */
                rxState = EOF_1;
                break;
            case EOF_1:
                /* End of frame byte is 0x55 */
                if (b == 0x55) {
                    errorState = MSG_COMPLETE;
                    rxState = WFSYNC_1;
                }
                break;
        }
    }
}

void flywheelbike::updateStats()
{
    QSettings settings;
    QString heartRateBeltName = settings.value("heart_rate_belt_name", "Disabled").toString();

    // calculate the acculamator every time on the current data, in order to avoid holes in peloton or strava
    KCal += ((( (0.048 * ((double)watts()) + 1.19) * settings.value("weight", 75.0).toFloat() * 3.5) / 200.0 ) / (60000.0 / ((double)lastRefreshCharacteristicChanged.msecsTo(QDateTime::currentDateTime())))); //(( (0.048* Output in watts +1.19) * body weight in kg * 3.5) / 200 ) / 60
    Distance += ((Speed.value() / 3600000.0) * ((double)lastRefreshCharacteristicChanged.msecsTo(QDateTime::currentDateTime())) );

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
    bool ios_peloton_workaround = settings.value("ios_peloton_workaround", false).toBool();
    if(ios_peloton_workaround && cadence && h && firstStateChanged)
    {
        h->virtualbike_setCadence(currentCrankRevolutions(),lastCrankEventTime());
        h->virtualbike_setHeartRate((uint8_t)currentHeart().value());
    }
#endif
#endif

    debug("Current Resistance: " + QString::number(Resistance.value()));
    debug("Current Speed: " + QString::number(Speed.value()));
    debug("Current Calculate Distance: " + QString::number(Distance.value()));
    debug("Current Cadence: " + QString::number(Cadence.value()));
    //debug("Current Distance: " + QString::number(distance));
    debug("Current CrankRevs: " + QString::number(CrankRevs));
    debug("Last CrankEventTime: " + QString::number(LastCrankEventTime));
    debug("Current Watt: " + QString::number(watts()));

    if(m_control->error() != QLowEnergyController::NoError)
        qDebug() << "QLowEnergyController ERROR!!" << m_control->errorString();
}

void flywheelbike::characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue)
{
    static uint8_t zero_fix_filter = 0;
    //qDebug() << "characteristicChanged" << characteristic.uuid() << newValue << newValue.length();
    Q_UNUSED(characteristic);
    QSettings settings;
    QString heartRateBeltName = settings.value("heart_rate_belt_name", "Disabled").toString();

    debug(" << " + newValue.toHex(' '));

    lastPacket = newValue;

    decodeReceivedData(newValue);

    qDebug() << "errorState" << errorState;

    if (errorState == MSG_COMPLETE) {
        ICGLiveStreamData *parsedData = (ICGLiveStreamData*) bikeData.buffer;

        if (bikeData.message_id == SEND_ICG_LIVE_STREAM_DATA) {
            double distance = GetDistanceFromPacket(newValue);

#ifdef Q_OS_ANDROID
            if(settings.value("ant_heart", false).toBool())
                Heart = (uint8_t)KeepAwakeHelper::heart();
#endif

            uint16_t power = ((parsedData->power >> 8) & 0xFF);
            power += ((parsedData->power & 0xFF) << 8);
            uint16_t speed = ((parsedData->speed >> 8) & 0xFF);
            speed += ((parsedData->speed & 0xFF) << 8);

            if(zero_fix_filter < settings.value("flywheel_filter", 2).toUInt() && (parsedData->cadence == 0 || speed == 0 || power == 0))
            {
                qDebug() << "filtering crappy values";
                zero_fix_filter++;
            }
            else
            {
                zero_fix_filter = 0;

                Resistance = parsedData->brake_level;
                emit resistanceRead(Resistance.value());
                if(settings.value("cadence_sensor_name", "Disabled").toString().startsWith("Disabled"))
                    Cadence = parsedData->cadence;
                m_watts = power;
                if(!settings.value("speed_power_based", false).toBool())
                    Speed = ((double)speed) / 10.0;
                else
                    Speed = metric::calculateSpeedFromPower(m_watt.value());

                // https://www.facebook.com/groups/149984563348738/permalink/174268944253633/?comment_id=174366620910532&reply_comment_id=174666314213896
                m_pelotonResistance = (Resistance.value() * 0.8173) + 9.2712;
            }
            updateStats();
            return;
        }

        if (bikeData.message_id == BRAKE_CALIBRATION_RESET) {
            //[_delegate didUpdateStatus:BIKE_NEEDS_CALIBRATION];
            return;
        }

        if (bikeData.message_id == SEND_ICG_AGGREGATED_STREAM_DATA) {

        }

        if (bikeData.message_id == REQUEST_DISCONNECT) {
            //[self disconnectBike];
            return;
        }

        //[_delegate didUpdateStatus:BIKE_MESSAGE_UNKNOWN];
    }
    updateStats();
}

double flywheelbike::GetDistanceFromPacket(QByteArray packet)
{
    uint16_t convertedData = (packet.at(2) << 8) | packet.at(3);
    double data = ((double)convertedData) / 100.0f;
    return data;
}

void flywheelbike::btinit()
{
    uint8_t initData1[] = { 0xf5, 0x20, 0x20, 0x40, 0xf6 };

    writeCharacteristic(initData1, sizeof(initData1), "init", false, true);
    initDone = true;
}

void flywheelbike::stateChanged(QLowEnergyService::ServiceState state)
{
    QBluetoothUuid _gattNotify1CharacteristicId((QString)"6E400003-B5A3-F393-E0A9-E50E24DCCA9E");

    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceState>();
    debug("BTLE stateChanged " + QString::fromLocal8Bit(metaEnum.valueToKey(state)));

    if(state == QLowEnergyService::ServiceDiscovered)
    {
        //qDebug() << gattCommunicationChannelService->characteristics();

        gattNotify1Characteristic = gattCommunicationChannelService->characteristic(_gattNotify1CharacteristicId);
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
            bool ios_peloton_workaround = settings.value("ios_peloton_workaround", false).toBool();
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
                //connect(virtualBike,&virtualbike::debug ,this,&flywheelbike::debug);
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

void flywheelbike::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue)
{
    debug("descriptorWritten " + descriptor.name() + " " + newValue.toHex(' '));

    initRequest = true;
    emit connectedAndDiscovered();
}

void flywheelbike::characteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue)
{
    Q_UNUSED(characteristic);
    debug("characteristicWritten " + newValue.toHex(' '));
}

void flywheelbike::serviceScanDone(void)
{
    debug("serviceScanDone");

    QBluetoothUuid _gattCommunicationChannelServiceId((QString)"6E400001-B5A3-F393-E0A9-E50E24DCCA9E");

    gattCommunicationChannelService = m_control->createServiceObject(_gattCommunicationChannelServiceId);
    connect(gattCommunicationChannelService, SIGNAL(stateChanged(QLowEnergyService::ServiceState)), this, SLOT(stateChanged(QLowEnergyService::ServiceState)));
    gattCommunicationChannelService->discoverDetails();
}

void flywheelbike::errorService(QLowEnergyService::ServiceError err)
{
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    debug("flywheelbike::errorService" + QString::fromLocal8Bit(metaEnum.valueToKey(err)) + m_control->errorString());
}

void flywheelbike::error(QLowEnergyController::Error err)
{
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    debug("flywheelbike::error" + QString::fromLocal8Bit(metaEnum.valueToKey(err)) + m_control->errorString());
}

void flywheelbike::deviceDiscovered(const QBluetoothDeviceInfo &device)
{
    debug("Found new device: " + device.name() + " (" + device.address().toString() + ')');
    if(device.name().startsWith("Flywheel"))
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

bool flywheelbike::connected()
{
    if(!m_control)
        return false;
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

void* flywheelbike::VirtualBike()
{
    return virtualBike;
}

void* flywheelbike::VirtualDevice()
{
    return VirtualBike();
}

uint16_t flywheelbike::watts()
{
    if(currentCadence().value() == 0) return 0;

    return m_watts;
}

void flywheelbike::controllerStateChanged(QLowEnergyController::ControllerState state)
{
    qDebug() << "controllerStateChanged" << state;
    if(state == QLowEnergyController::UnconnectedState && m_control)
    {
        qDebug() << "trying to connect back again...";
        initDone = false;
        m_control->connectToDevice();
    }
}
