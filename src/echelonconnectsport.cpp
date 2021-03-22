#include "echelonconnectsport.h"
#include "virtualbike.h"
#include "keepawakehelper.h"
#include <QFile>
#include <QDateTime>
#include <QMetaEnum>
#include <QSettings>
#include <QBluetoothLocalDevice>
#include <math.h>
#include "ios/lockscreen.h"

echelonconnectsport::echelonconnectsport(bool noWriteResistance, bool noHeartService, uint8_t bikeResistanceOffset, double bikeResistanceGain)
{
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

void echelonconnectsport::writeCharacteristic(uint8_t* data, uint8_t data_len, QString info, bool disable_log, bool wait_for_response)
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

    gattCommunicationChannelService->writeCharacteristic(gattWriteCharacteristic, QByteArray((const char*)data, data_len));

    if(!disable_log)
        qDebug() << " >> " + QByteArray((const char*)data, data_len).toHex(' ') + " // " + info;

    loop.exec();
}

void echelonconnectsport::forceResistance(int8_t requestResistance)
{
    uint8_t noOpData[] = { 0xf0, 0xb1, 0x01, 0x00, 0x00 };

    noOpData[3] = requestResistance;

    for(uint8_t i=0; i<sizeof(noOpData)-1; i++)
    {
       noOpData[4] += noOpData[i]; // the last byte is a sort of a checksum
    }

    writeCharacteristic(noOpData, sizeof(noOpData), "force resistance", false, true);
}

void echelonconnectsport::sendPoll()
{    
    uint8_t noOpData[] = { 0xf0, 0xa0, 0x01, 0x00, 0x00 };

    noOpData[3] = counterPoll;

    for(uint8_t i=0; i<sizeof(noOpData)-1; i++)
    {
       noOpData[4] += noOpData[i]; // the last byte is a sort of a checksum
    }

    writeCharacteristic(noOpData, sizeof(noOpData), "noOp", false, true);

    counterPoll++;
    if(!counterPoll)
        counterPoll = 1;
}

void echelonconnectsport::update()
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
       gattNotify2Characteristic.isValid() &&
       initDone)
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

        // sending poll every 2 seconds
        if(sec1Update++ >= (2000 / refresh->interval()))
        {
            sec1Update = 0;
            sendPoll();
            //updateDisplay(elapsed);
        }        

        if(requestResistance != -1)
        {
           if(requestResistance > 32) requestResistance = 32;
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

    firstUpdate = false;
}

void echelonconnectsport::serviceDiscovered(const QBluetoothUuid &gatt)
{
    qDebug() << "serviceDiscovered " + gatt.toString();
}

void echelonconnectsport::characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue)
{
    //qDebug() << "characteristicChanged" << characteristic.uuid() << newValue << newValue.length();
    Q_UNUSED(characteristic);    
    QSettings settings;
    QString heartRateBeltName = settings.value("heart_rate_belt_name", "Disabled").toString();

    qDebug() << " << " + newValue.toHex(' ');

    lastPacket = newValue;

    // resistance value is in another frame
    if(newValue.length() == 5 && ((unsigned char)newValue.at(0)) == 0xf0 && ((unsigned char)newValue.at(1)) == 0xd2)
    {
        Resistance = newValue.at(3);

        //0,0097x3 - 0,4972x2 + 10,126x - 37,08
        double p = ((pow(Resistance.value(),3) * 0.0097) - (0.4972 * pow(Resistance.value(), 2)) + (10.126 * Resistance.value()) - 37.08);
        if(p < 0)
            p = 0;
        m_pelotonResistance = p;

        qDebug() << "Current resistance: " + QString::number(Resistance.value());
        return;
    }

    if (newValue.length() != 13)
        return;

    /*if ((uint8_t)(newValue.at(0)) != 0xf0 && (uint8_t)(newValue.at(1)) != 0xd1)
        return;*/

    double distance = GetDistanceFromPacket(newValue);

    Cadence = ((uint8_t)newValue.at(10));
    Speed = 0.37497622 * ((double)Cadence.value());
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
            Heart = appleWatchHeartRate;
            debug("Current Heart from Apple Watch: " + QString::number(appleWatchHeartRate));
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
    
    qDebug() << "Current Local elapsed: " + GetElapsedFromPacket(newValue).toString();
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

QTime echelonconnectsport::GetElapsedFromPacket(QByteArray packet)
{
    uint16_t convertedData = (packet.at(3) << 8) | packet.at(4);
    QTime t(0,convertedData / 60, convertedData % 60);
    return t;
}

double echelonconnectsport::GetDistanceFromPacket(QByteArray packet)
{
    uint16_t convertedData = (packet.at(7) << 8) | packet.at(8);
    double data = ((double)convertedData) / 100.0f;
    return data;
}

void echelonconnectsport::btinit()
{    
    uint8_t initData1[] = { 0xf0, 0xa1, 0x00, 0x91 };
    uint8_t initData2[] = { 0xf0, 0xa3, 0x00, 0x93 };
    uint8_t initData3[] = { 0xf0, 0xb0, 0x01, 0x01, 0xa2 };
    //uint8_t initData4[] = { 0xf0, 0x60, 0x00, 0x50 }; // get sleep command

    // useless i guess
    //writeCharacteristic(initData4, sizeof(initData4), "get sleep", false, true);

    // in the snoof log it repeats this frame 4 times, i will have to analyze the response to understand if 4 times are enough
    writeCharacteristic(initData1, sizeof(initData1), "init", false, true);
    writeCharacteristic(initData1, sizeof(initData1), "init", false, true);
    writeCharacteristic(initData1, sizeof(initData1), "init", false, true);
    writeCharacteristic(initData1, sizeof(initData1), "init", false, true);

    writeCharacteristic(initData2, sizeof(initData2), "init", false, true);
    writeCharacteristic(initData1, sizeof(initData1), "init", false, true);
    writeCharacteristic(initData3, sizeof(initData3), "init", false, true);

    initDone = true;
}

void echelonconnectsport::stateChanged(QLowEnergyService::ServiceState state)
{
    QBluetoothUuid _gattWriteCharacteristicId((QString)"0bf669f2-45f2-11e7-9598-0800200c9a66");
    QBluetoothUuid _gattNotify1CharacteristicId((QString)"0bf669f3-45f2-11e7-9598-0800200c9a66");
    QBluetoothUuid _gattNotify2CharacteristicId((QString)"0bf669f4-45f2-11e7-9598-0800200c9a66");

    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceState>();
    qDebug() << "BTLE stateChanged " + QString::fromLocal8Bit(metaEnum.valueToKey(state));

    if(state == QLowEnergyService::ServiceDiscovered)
    {
        //qDebug() << gattCommunicationChannelService->characteristics();

        gattWriteCharacteristic = gattCommunicationChannelService->characteristic(_gattWriteCharacteristicId);
        gattNotify1Characteristic = gattCommunicationChannelService->characteristic(_gattNotify1CharacteristicId);
        gattNotify2Characteristic = gattCommunicationChannelService->characteristic(_gattNotify2CharacteristicId);
        Q_ASSERT(gattWriteCharacteristic.isValid());
        Q_ASSERT(gattNotify1Characteristic.isValid());
        Q_ASSERT(gattNotify2Characteristic.isValid());

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
                //connect(virtualBike,&virtualbike::debug ,this,&echelonconnectsport::debug);
            }
        }
        firstStateChanged = 1;
        // ********************************************************************************************************

        QByteArray descriptor;
        descriptor.append((char)0x01);
        descriptor.append((char)0x00);
        gattCommunicationChannelService->writeDescriptor(gattNotify1Characteristic.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration), descriptor);
        gattCommunicationChannelService->writeDescriptor(gattNotify2Characteristic.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration), descriptor);
    }
}

void echelonconnectsport::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue)
{
    qDebug() << "descriptorWritten " + descriptor.name() + " " + newValue.toHex(' ');

    initRequest = true;
    emit connectedAndDiscovered();
}

void echelonconnectsport::characteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue)
{
    Q_UNUSED(characteristic);
    qDebug() << "characteristicWritten " + newValue.toHex(' ');
}

void echelonconnectsport::serviceScanDone(void)
{
    qDebug() << "serviceScanDone";

    QBluetoothUuid _gattCommunicationChannelServiceId((QString)"0bf669f1-45f2-11e7-9598-0800200c9a66");

    gattCommunicationChannelService = m_control->createServiceObject(_gattCommunicationChannelServiceId);
    connect(gattCommunicationChannelService, SIGNAL(stateChanged(QLowEnergyService::ServiceState)), this, SLOT(stateChanged(QLowEnergyService::ServiceState)));
    gattCommunicationChannelService->discoverDetails();
}

void echelonconnectsport::errorService(QLowEnergyService::ServiceError err)
{
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    qDebug() << "echelonconnectsport::errorService" + QString::fromLocal8Bit(metaEnum.valueToKey(err)) + m_control->errorString();
}

void echelonconnectsport::error(QLowEnergyController::Error err)
{
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    qDebug() << "echelonconnectsport::error" + QString::fromLocal8Bit(metaEnum.valueToKey(err)) + m_control->errorString();
}

void echelonconnectsport::deviceDiscovered(const QBluetoothDeviceInfo &device)
{
    qDebug() << "Found new device: " + device.name() + " (" + device.address().toString() + ')';
    if(device.name().startsWith("ECH"))
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

bool echelonconnectsport::connected()
{
    if(!m_control)
        return false;
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

void* echelonconnectsport::VirtualBike()
{
    return virtualBike;
}

void* echelonconnectsport::VirtualDevice()
{
    return VirtualBike();
}

uint16_t echelonconnectsport::watts()
{
    if(currentCadence().value() == 0) return 0;

    // https://github.com/cagnulein/qdomyos-zwift/issues/62#issuecomment-736913564
    /*if(currentCadence().value() < 90)
        return (uint16_t)((3.59 * exp(0.0217 * (double)(currentCadence().value()))) * exp(0.095 * (double)(currentResistance().value())) );
    else
        return (uint16_t)((3.59 * exp(0.0217 * (double)(currentCadence().value()))) * exp(0.088 * (double)(currentResistance().value())) );*/

    const double Epsilon = 4.94065645841247E-324;
    const int wattTableFirstDimension = 33;
    const int wattTableSecondDimension = 11;
    double wattTable[wattTableFirstDimension][wattTableSecondDimension] = {
        {Epsilon, 1.0, 2.2, 4.8, 9.5, 13.6, 16.7, 22.6, 26.3, 29.2, 47.0},
        {Epsilon, 1.0, 2.2, 4.8, 9.5, 13.6, 16.7, 22.6, 26.3, 29.2, 47.0},
        {Epsilon, 1.3, 3.0, 5.4, 10.4, 14.5, 18.5, 24.6, 27.6, 33.5, 49.5},
        {Epsilon, 1.5, 3.7, 6.7, 11.7, 15.9, 19.6, 26.1, 30.8, 35.2, 51.2},
        {Epsilon, 1.6, 4.7, 7.5, 13.7, 17.6, 22.6, 29.0, 36.9, 42.6, 57.2},
        {Epsilon, 1.8, 5.2, 8.0, 14.8, 19.1, 23.5, 32.5, 37.5, 50.8, 61.8},
        {Epsilon, 1.9, 5.7, 8.7, 15.6, 20.2, 25.5, 33.5, 39.6, 52.1, 65.3},
        {Epsilon, 2.0, 6.2, 9.5, 16.8, 21.8, 28.1, 37.0, 42.8, 57.8, 68.4},
        {Epsilon, 2.1, 6.8, 10.8, 18.2, 23.6, 29.5, 40.0, 47.6, 60.5, 72.1},
        {Epsilon, 2.2, 7.3, 11.5, 19.3, 26.3, 33.5, 45.3, 51.8, 66.7, 76.8},
        {Epsilon, 2.4, 7.9, 12.7, 20.8, 29.8, 37.6, 52.2, 56.2, 73.5, 83.6},
        {Epsilon, 2.6, 8.5, 13.5, 23.5, 33.6, 41.9, 55.1, 59.0, 78.6, 89.7},
        {Epsilon, 2.7, 9.1, 14.2, 25.6, 35.4, 45.3, 57.3, 62.8, 81.3, 95.0},
        {Epsilon, 2.9, 9.6, 16.8, 29.1, 37.5, 49.6, 62.5, 69.0, 84.7, 99.3},
        {Epsilon, 3.0, 10.0, 22.3, 31.2, 40.3, 51.8, 65.0, 70.0, 92.6, 108.2},
        {Epsilon, 3.2, 10.4, 24.0, 36.6, 42.5, 56.3, 74.0, 85.0, 98.2, 123.5},
        {Epsilon, 3.5, 10.9, 25.1, 38.5, 47.6, 65.4, 83.0, 93.0, 114.8, 136.8},
        {Epsilon, 3.7, 11.5, 26.0, 41.0, 53.2, 71.6, 90.0, 100.0, 121.7, 149.2},
        {Epsilon, 4.0, 12.1, 27.5, 43.6, 56.0, 82.3, 101.0, 113.6, 143.0, 162.8},
        {Epsilon, 4.2, 12.7, 29.7, 46.7, 64.2, 87.9, 109.2, 128.9, 154.0, 172.3},
        {Epsilon, 4.5, 13.7, 32.0, 50.0, 71.8, 95.6, 113.8, 135.6, 165.0, 185.0},
        {Epsilon, 4.7, 14.9, 34.5, 54.2, 77.0, 100.7, 127.0, 147.6, 180.0, 200.0},
        {Epsilon, 5.0, 15.8, 36.5, 58.3, 83.4, 110.1, 136.0, 168.1, 196.0, 213.5},
        {Epsilon, 5.6, 17.0, 39.5, 64.3, 88.8, 123.4, 154.0, 182.0, 210.0, 235.0},
        {Epsilon, 6.1, 18.2, 44.0, 70.7, 99.9, 133.3, 166.0, 198.0, 230.0, 253.5},
        {Epsilon, 6.8, 19.4, 49.0, 79.0, 108.8, 147.2, 185.0, 217.0, 255.2, 278.0},
        {Epsilon, 7.6, 22.0, 54.8, 88.0, 127.0, 167.0, 212.0, 244.0, 287.0, 305.0},
        {Epsilon, 8.7, 26.0, 62.0, 100.0, 145.0, 190.0, 242.0, 281.0, 315.1, 350.0},
        {Epsilon, 9.2, 30.0, 71.0, 114.4, 161.6, 215.1, 275.1, 317.0, 358.5, 390.0},
        {Epsilon, 9.8, 36.0, 82.5, 134.5, 195.3, 252.5, 313.7, 360.0, 420.3, 460.0},
        {Epsilon, 10.5, 43.0, 95.0, 157.1, 228.4, 300.1, 374.1, 403.8, 487.8, 540.0},
        {Epsilon, 12.5, 48.0, 99.3, 162.2, 232.9, 310.4, 400.3, 435.5, 530.5, 589.0},
        {Epsilon, 13.0, 53.0, 102.0, 170.3, 242.0, 320.0, 427.9, 475.2, 570.0, 625.0}};

    int level = Resistance.value();
    if (level < 0) {
        level = 0;
    }
    if (level >= wattTableFirstDimension) {
        level = wattTableFirstDimension - 1;
    }
    double* watts_of_level = wattTable[level];
    int watt_setp = (Cadence.value() / 10.0);
    if (watt_setp >= 10) {
        return (((double) Cadence.value()) / 100.0) * watts_of_level[wattTableSecondDimension - 1];
    }
    double watt_base = watts_of_level[watt_setp];
    return (((watts_of_level[watt_setp + 1] - watt_base) / 10.0) * ((double) (((int)(Cadence.value())) % 10))) + watt_base;
}

void echelonconnectsport::controllerStateChanged(QLowEnergyController::ControllerState state)
{
    qDebug() << "controllerStateChanged" << state;
    if(state == QLowEnergyController::UnconnectedState && m_control)
    {
        qDebug() << "trying to connect back again...";
        initDone = false;
        m_control->connectToDevice();
    }
}
