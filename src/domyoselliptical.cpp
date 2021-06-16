#include "domyoselliptical.h"
#include "virtualtreadmill.h"
#include "keepawakehelper.h"
#include <QFile>
#include <QDateTime>
#include <QMetaEnum>
#include <QBluetoothLocalDevice>
#include <QSettings>

domyoselliptical::domyoselliptical(bool noWriteResistance, bool noHeartService, bool testResistance, uint8_t bikeResistanceOffset, double bikeResistanceGain)
{
    m_watt.setType(metric::METRIC_WATT);
    Speed.setType(metric::METRIC_SPEED);
    refresh = new QTimer(this);

    this->testResistance = testResistance;
    this->noWriteResistance = noWriteResistance;
    this->noHeartService = noHeartService;
    this->bikeResistanceGain = bikeResistanceGain;
    this->bikeResistanceOffset = bikeResistanceOffset;

    initDone = false;
    connect(refresh, SIGNAL(timeout()), this, SLOT(update()));
    refresh->start(300);
}

domyoselliptical::~domyoselliptical()
{
    qDebug() << "~domyoselliptical()" << virtualTreadmill;
    if(virtualTreadmill)
        delete virtualTreadmill;
}

void domyoselliptical::writeCharacteristic(uint8_t* data, uint8_t data_len, QString info, bool disable_log, bool wait_for_response)
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

    if(timeout.isActive() == false)
        debug(" exit for timeout");
}

void domyoselliptical::updateDisplay(uint16_t elapsed)
{
    //if(bike_type == CHANG_YOW)
    {
        uint8_t display2[] = {0xf0, 0xcd, 0x01, 0x00, 0x00, 0x01, 0xff, 0xff, 0xff, 0xff,
                              0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                              0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00};

        display2[3] = ((((uint16_t)(odometer() * 10))) >> 8) & 0xFF;
        display2[4] = (((uint16_t)(odometer() * 10))) & 0xFF;

        for(uint8_t i=0; i<sizeof(display2)-1; i++)
        {
            display2[26] += display2[i]; // the last byte is a sort of a checksum
        }

        writeCharacteristic(display2, 20, "updateDisplay2", false, false);
        writeCharacteristic(&display2[20], sizeof (display2) - 20, "updateDisplay2", false, true);
    }

    uint8_t display[] = {0xf0, 0xcb, 0x03, 0x00, 0x00, 0xff, 0x01, 0x00, 0x00, 0x02,
                         0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x01, 0x01, 0x00,
                         0x00, 0x01, 0xff, 0xff, 0xff, 0xff, 0x00};

    display[3] = (elapsed / 60) & 0xFF; // high byte for elapsed time (in seconds)
    display[4] = (elapsed % 60 & 0xFF); // low byte for elasped time (in seconds)

    display[7] = ((uint8_t)((uint16_t)(currentSpeed().value()) >> 8)) & 0xFF;
    display[8] = (uint8_t)(currentSpeed().value()) & 0xFF;

    display[12] = (uint8_t)currentHeart().value();

    //display[13] = ((((uint8_t)calories())) >> 8) & 0xFF;
    //display[14] = (((uint8_t)calories())) & 0xFF;

    display[16] = (uint8_t)currentCadence();

    display[19] = ((((uint16_t)calories())) >> 8) & 0xFF;
    display[20] = (((uint16_t)calories())) & 0xFF;

    for(uint8_t i=0; i<sizeof(display)-1; i++)
    {
       display[26] += display[i]; // the last byte is a sort of a checksum
    }

    writeCharacteristic(display, 20, "updateDisplay elapsed=" + QString::number(elapsed), false, false );
    writeCharacteristic(&display[20], sizeof (display) - 20, "updateDisplay elapsed=" + QString::number(elapsed), false, true );

}

void domyoselliptical::forceResistanceAndInclination(int8_t requestResistance, uint8_t inclination)
{
   uint8_t write[] = {0xf0, 0xad, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                      0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x01, 0xff,
                      0xff, 0xff, 0x00};

   write[10] = requestResistance;

   //write[13] = ((uint16_t)(inclination*10) >> 8) & 0xFF;
   //write[14] = inclination; //need a hci snoof log about it

   for(uint8_t i=0; i<sizeof(write)-1; i++)
   {
      write[22] += write[i]; // the last byte is a sort of a checksum
   }

   writeCharacteristic(write, 20, "forceResistance " + QString::number(requestResistance) + " Inclination " + inclination);
   writeCharacteristic(&write[20], sizeof (write) - 20, "forceResistance " + QString::number(requestResistance) + " Inclination " + inclination);
}

void domyoselliptical::update()
{
    uint8_t noOpData[] = { 0xf0, 0xac, 0x9c };

    // stop tape
    uint8_t initDataF0C800B8[] = { 0xf0, 0xc8, 0x00, 0xb8 };

    if(m_control->state() == QLowEnergyController::UnconnectedState)
    {
        emit disconnected();
        return;
    }

    if(initRequest)
    {
        initRequest = false;
        //if(bike_type == CHANG_YOW)
            btinit_changyow(false);
        //else
        //    btinit_telink(false);
    }
    else if(bluetoothDevice.isValid() &&
       m_control->state() == QLowEnergyController::DiscoveredState &&
       gattCommunicationChannelService &&
       gattWriteCharacteristic.isValid() &&
       gattNotifyCharacteristic.isValid() &&
       initDone)
    {
        update_metrics(true, watts());

        // ******************************************* virtual bike init *************************************
        if(!firstVirtual && searchStopped && !virtualTreadmill)
        {
            QSettings settings;
            bool virtual_device_enabled = settings.value("virtual_device_enabled", true).toBool();
            if(virtual_device_enabled)
            {
                debug("creating virtual treadmill interface...");
                virtualTreadmill = new virtualtreadmill(this, noHeartService);
                connect(virtualTreadmill,&virtualtreadmill::debug ,this,&domyoselliptical::debug);
                firstVirtual = 1;
            }
        }
        // ********************************************************************************************************

        // updating the treadmill console every second
        if(sec1Update++ == (1000 / refresh->interval()))
        {
            sec1Update = 0;
            updateDisplay(elapsed.value());
        }
        else
        {
            writeCharacteristic(noOpData, sizeof(noOpData), "noOp", true, true);
        }

        if(testResistance)
        {
            if((((int)elapsed.value()) % 5) == 0)
            {
                uint8_t new_res = currentResistance() + 1;
                if(new_res > 15)
                    new_res = 1;
                forceResistanceAndInclination(new_res, currentInclination().value());
            }
        }

        if(requestResistance != -1)
        {
           if(requestResistance > 15) requestResistance = 15;
           else if(requestResistance == 0) requestResistance = 1;

           if(requestResistance != currentResistance())
           {
              debug("writing resistance " + QString::number(requestResistance));
              forceResistanceAndInclination(requestResistance, currentInclination().value());
           }
           requestResistance = -1;
        }
        else if(requestInclination != -1)
        {
           if(requestInclination > 15) requestInclination = 15;
           else if(requestInclination == 0) requestInclination = 1;

           if(requestInclination != currentInclination().value())
           {
              debug("writing inclination " + QString::number(requestInclination));
              forceResistanceAndInclination(currentResistance(), requestInclination);
           }
           requestInclination = -1;
        }
        if(requestStart != -1)
        {
           debug("starting...");

           //if(bike_type == CHANG_YOW)
               btinit_changyow(true);
           //else
           //    btinit_telink(true);

           requestStart = -1;
           emit bikeStarted();
        }
        if(requestStop != -1)
        {
            debug("stopping...");
            writeCharacteristic(initDataF0C800B8, sizeof(initDataF0C800B8), "stop tape");
            requestStop = -1;
        }
    }
}

void domyoselliptical::serviceDiscovered(const QBluetoothUuid &gatt)
{
    debug("serviceDiscovered " + gatt.toString());
}

void domyoselliptical::characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue)
{
    //qDebug() << "characteristicChanged" << characteristic.uuid() << newValue << newValue.length();
    Q_UNUSED(characteristic);    
    QSettings settings;
    QString heartRateBeltName = settings.value("heart_rate_belt_name", "Disabled").toString();

    debug(" << " + newValue.toHex(' '));

    lastPacket = newValue;
    if (newValue.length() != 26)
        return;

    if (newValue.at(22) == 0x06)
    {
        debug("inclination up button pressed!");
        //requestStart = 1;
    }
    else if (newValue.at(22) == 0x07)
    {
        debug("inclination down button pressed!"); // i guess it should be the inclination down
        //requestStop = 1;
    }

    /*if ((uint8_t)newValue.at(1) != 0xbc && newValue.at(2) != 0x04)  // intense run, these are the bytes for the inclination and speed status
        return;*/

    double speed = GetSpeedFromPacket(newValue) * settings.value("domyos_elliptical_speed_ratio", 1.0).toDouble();
    double kcal = GetKcalFromPacket(newValue);
    double distance = GetDistanceFromPacket(newValue) * settings.value("domyos_elliptical_speed_ratio", 1.0).toDouble();

    if(settings.value("cadence_sensor_name", "Disabled").toString().startsWith("Disabled"))
        Cadence = ((uint8_t)newValue.at(9));
    Resistance = newValue.at(14);
    Inclination = newValue.at(21);
    if(Resistance < 1)
    {
        debug("invalid resistance value " + QString::number(Resistance) + " putting to default");
        Resistance = 1;
    }

#ifdef Q_OS_ANDROID
    if(settings.value("ant_heart", false).toBool())
        Heart = (uint8_t)KeepAwakeHelper::heart();
    else
#endif
    {
        if(heartRateBeltName.startsWith("Disabled"))
            Heart = ((uint8_t)newValue.at(18));
    }

    CrankRevs++;
    LastCrankEventTime += (uint16_t)(1024.0 / (((double)(Cadence)) / 60.0));
    lastRefreshCharacteristicChanged = QDateTime::currentDateTime();

    debug("Current speed: " + QString::number(speed));
    debug("Current cadence: " + QString::number(Cadence));
    debug("Current resistance: " + QString::number(Resistance));
    debug("Current inclination: " + QString::number(Inclination.value()));
    debug("Current heart: " + QString::number(Heart.value()));
    debug("Current KCal: " + QString::number(kcal));
    debug("Current Distance: " + QString::number(distance));
    debug("Current CrankRevs: " + QString::number(CrankRevs));
    debug("Last CrankEventTime: " + QString::number(LastCrankEventTime));
    debug("Current Watt: " + QString::number(watts()));

    if(m_control->error() != QLowEnergyController::NoError)
        qDebug() << "QLowEnergyController ERROR!!" << m_control->errorString();

    Speed = speed;
    KCal = kcal;
    Distance = distance;    
}

double domyoselliptical::GetSpeedFromPacket(QByteArray packet)
{
    uint16_t convertedData = (packet.at(6) << 8) | packet.at(7);
    double data = (double)convertedData / 10.0f;
    return data;
}

double domyoselliptical::GetKcalFromPacket(QByteArray packet)
{
    uint16_t convertedData = (packet.at(10) << 8) | ((uint8_t)packet.at(11));
    return (double)convertedData;
}

double domyoselliptical::GetDistanceFromPacket(QByteArray packet)
{
    uint16_t convertedData = (packet.at(12) << 8) | packet.at(13);
    double data = ((double)convertedData) / 10.0f;
    return data;
}

void domyoselliptical::btinit_changyow(bool startTape)
{
    // set speed and incline to 0
    uint8_t initData1[] = { 0xf0, 0xc8, 0x01, 0xb9 };
    uint8_t initData2[] = { 0xf0, 0xc9, 0xb9 };

    // main startup sequence
    uint8_t initDataStart[] = { 0xf0, 0xa3, 0x93 };
    uint8_t initDataStart2[] = { 0xf0, 0xa4, 0x94 };
    uint8_t initDataStart3[] = { 0xf0, 0xa5, 0x95 };
    uint8_t initDataStart4[] = { 0xf0, 0xab, 0x9b };
    uint8_t initDataStart5[] = { 0xf0, 0xc4, 0x03, 0xb7 };
    uint8_t initDataStart6[] =
    {
            0xf0, 0xad, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
            0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x01, 0xff
    };
    uint8_t initDataStart7[] = { 0xff, 0xff, 0x8b }; // power on bt icon
    uint8_t initDataStart8[] =
    {
            0xf0, 0xcb, 0x02, 0x00, 0x08, 0xff, 0xff, 0xff, 0xff, 0xff,
            0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x01, 0x00
    };
    uint8_t initDataStart9[] = { 0x00, 0x01, 0xff, 0xff, 0xff, 0xff, 0xb6 }; // power on bt word
    uint8_t initDataStart10[] =
    {
            0xf0, 0xad, 0xff, 0xff, 0x00, 0x05, 0xff, 0xff, 0xff, 0xff,
            0xff, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0xff, 0x01, 0xff
    };
    uint8_t initDataStart11[] = { 0xff, 0xff, 0x94 }; // start tape
    uint8_t initDataStart12[] =
    {
            0xf0, 0xcb, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
            0xff, 0xff, 0xff, 0xff, 0x01, 0x00, 0x14, 0x01, 0xff, 0xff
    };
    uint8_t initDataStart13[] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xbd };

    writeCharacteristic(initData1, sizeof(initData1), "init", false, true);
    writeCharacteristic(initData2, sizeof(initData2), "init", false, true);
    writeCharacteristic(initDataStart, sizeof(initDataStart), "init", false, true);
    writeCharacteristic(initDataStart2, sizeof(initDataStart2), "init", false, true);
    writeCharacteristic(initDataStart3, sizeof(initDataStart3), "init", false, true);
    writeCharacteristic(initDataStart4, sizeof(initDataStart4), "init", false, true);
    writeCharacteristic(initDataStart5, sizeof(initDataStart5), "init", false, true);
    writeCharacteristic(initDataStart6, sizeof(initDataStart6), "init", false, false);
    writeCharacteristic(initDataStart7, sizeof(initDataStart7), "init", false, true);
    writeCharacteristic(initDataStart8, sizeof(initDataStart8), "init", false, false);
    writeCharacteristic(initDataStart9, sizeof(initDataStart9), "init", false, true);
    writeCharacteristic(initDataStart10, sizeof(initDataStart10), "init", false, false);
    if(startTape)
    {
        writeCharacteristic(initDataStart11, sizeof(initDataStart11), "init", false, true);
        writeCharacteristic(initDataStart12, sizeof(initDataStart12), "init", false, false);
        writeCharacteristic(initDataStart13, sizeof(initDataStart13), "init", false, true);
    }

    initDone = true;
}

void domyoselliptical::btinit_telink(bool startTape)
{
    Q_UNUSED(startTape)

    // set speed and incline to 0
    uint8_t initData1[] = { 0xf0, 0xc8, 0x01, 0xb9 };
    uint8_t initData2[] = { 0xf0, 0xc9, 0xb9 };
    uint8_t noOpData[] = { 0xf0, 0xac, 0x9c };

    // main startup sequence
    uint8_t initDataStart[] = { 0xf0, 0xcc, 0xff, 0xff, 0xff, 0xff, 0x01, 0xff, 0xb8 };

    writeCharacteristic(initData1, sizeof(initData1), "init");
    writeCharacteristic(initData2, sizeof(initData2), "init");
    writeCharacteristic(noOpData, sizeof(noOpData), "noOp");
    writeCharacteristic(initDataStart, sizeof(initDataStart), "init");
    updateDisplay(0);

    initDone = true;
}


void domyoselliptical::stateChanged(QLowEnergyService::ServiceState state)
{    
    QBluetoothUuid _gattWriteCharacteristicId((QString)"49535343-8841-43f4-a8d4-ecbe34729bb3");
    QBluetoothUuid _gattNotifyCharacteristicId((QString)"49535343-1e4d-4bd9-ba61-23c647249616");

    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceState>();
    debug("BTLE stateChanged " + QString::fromLocal8Bit(metaEnum.valueToKey(state)));

    if(state == QLowEnergyService::ServiceDiscovered)
    {
	    //qDebug() << gattCommunicationChannelService->characteristics();

	    gattWriteCharacteristic = gattCommunicationChannelService->characteristic(_gattWriteCharacteristicId);
	    gattNotifyCharacteristic = gattCommunicationChannelService->characteristic(_gattNotifyCharacteristicId);
        Q_ASSERT(gattWriteCharacteristic.isValid());
        Q_ASSERT(gattNotifyCharacteristic.isValid());

	    // establish hook into notifications
	    connect(gattCommunicationChannelService, SIGNAL(characteristicChanged(QLowEnergyCharacteristic,QByteArray)),
        	    this, SLOT(characteristicChanged(QLowEnergyCharacteristic,QByteArray)));
        connect(gattCommunicationChannelService, SIGNAL(characteristicWritten(const QLowEnergyCharacteristic, const QByteArray)),
                this, SLOT(characteristicWritten(const QLowEnergyCharacteristic, const QByteArray)));
        connect(gattCommunicationChannelService, SIGNAL(error(QLowEnergyService::ServiceError)),
                this, SLOT(errorService(QLowEnergyService::ServiceError)));
        connect(gattCommunicationChannelService, SIGNAL(descriptorWritten(const QLowEnergyDescriptor, const QByteArray)), this,
                SLOT(descriptorWritten(const QLowEnergyDescriptor, const QByteArray)));

	    QByteArray descriptor;
	    descriptor.append((char)0x01);
	    descriptor.append((char)0x00);
	    gattCommunicationChannelService->writeDescriptor(gattNotifyCharacteristic.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration), descriptor);
    }
}

void domyoselliptical::searchingStop()
{
    searchStopped = true;
}

void domyoselliptical::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue)
{
    debug("descriptorWritten " + descriptor.name() + " " + newValue.toHex(' '));

    initRequest = true;
    emit connectedAndDiscovered();
}

void domyoselliptical::characteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue)
{
    Q_UNUSED(characteristic);
    debug("characteristicWritten " + newValue.toHex(' '));
}

void domyoselliptical::serviceScanDone(void)
{
    debug("serviceScanDone");

    QBluetoothUuid _gattCommunicationChannelServiceId((QString)"49535343-fe7d-4ae5-8fa9-9fafd205e455");

    gattCommunicationChannelService = m_control->createServiceObject(_gattCommunicationChannelServiceId);
    connect(gattCommunicationChannelService, SIGNAL(stateChanged(QLowEnergyService::ServiceState)), this, SLOT(stateChanged(QLowEnergyService::ServiceState)));
    gattCommunicationChannelService->discoverDetails();
}

void domyoselliptical::errorService(QLowEnergyService::ServiceError err)
{
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    debug("domyoselliptical::errorService" + QString::fromLocal8Bit(metaEnum.valueToKey(err)) + m_control->errorString());
}

void domyoselliptical::error(QLowEnergyController::Error err)
{
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    debug("domyoselliptical::error" + QString::fromLocal8Bit(metaEnum.valueToKey(err)) + m_control->errorString());
}

void domyoselliptical::deviceDiscovered(const QBluetoothDeviceInfo &device)
{
    debug("Found new device: " + device.name() + " (" + device.address().toString() + ')');
    if(device.name().startsWith("Domyos-EL") && !device.name().startsWith("DomyosBridge"))
    {
        bluetoothDevice = device;

        if(device.address().toString().startsWith("57"))
        {
            debug("domyos telink bike found");
            bike_type = TELINK;
        }
        else
        {
            debug("domyos changyow bike found");
            bike_type = CHANG_YOW;
        }

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
            searchStopped = false;
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
            searchStopped = false;
            emit disconnected();
        });

        // Connect
        m_control->connectToDevice();
        return;
    }
}

bool domyoselliptical::connected()
{
    if(!m_control)
        return false;
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

void* domyoselliptical::VirtualTreadmill()
{
    return virtualTreadmill;
}

void* domyoselliptical::VirtualDevice()
{
    return VirtualTreadmill();
}

uint16_t domyoselliptical::watts()
{
    QSettings settings;
    const uint8_t max_resistance = 15;
    // ref https://translate.google.com/translate?hl=it&sl=en&u=https://support.wattbike.com/hc/en-us/articles/115001881825-Power-Resistance-and-Cadence-Tables&prev=search&pto=aue

    const uint16_t watt_cad40_min = 25;
    const uint16_t watt_cad40_max = 55;

    const uint16_t watt_cad45_min = 35;
    const uint16_t watt_cad45_max = 65;

    const uint16_t watt_cad50_min = 40;
    const uint16_t watt_cad50_max = 80;

    const uint16_t watt_cad55_min = 50;
    const uint16_t watt_cad55_max = 105;

    const uint16_t watt_cad60_min = 60;
    const uint16_t watt_cad60_max = 125;

    const uint16_t watt_cad65_min = 70;
    const uint16_t watt_cad65_max = 160;

    const uint16_t watt_cad70_min = 85;
    const uint16_t watt_cad70_max = 190;

    const uint16_t watt_cad75_min = 100;
    const uint16_t watt_cad75_max = 240;

    const uint16_t watt_cad80_min = 115;
    const uint16_t watt_cad80_max = 280;

    const uint16_t watt_cad85_min = 130;
    const uint16_t watt_cad85_max = 340;

    const uint16_t watt_cad90_min = 150;
    const uint16_t watt_cad90_max = 390;

    const uint16_t watt_cad95_min = 175;
    const uint16_t watt_cad95_max = 450;

    const uint16_t watt_cad100_min = 195;
    const uint16_t watt_cad100_max = 520;

    const uint16_t watt_cad105_min = 210;
    const uint16_t watt_cad105_max = 600;

    const uint16_t watt_cad110_min = 245;
    const uint16_t watt_cad110_max = 675;

    const uint16_t watt_cad115_min = 270;
    const uint16_t watt_cad115_max = 760;

    const uint16_t watt_cad120_min = 300;
    const uint16_t watt_cad120_max = 850;

    const uint16_t watt_cad125_min = 330;
    const uint16_t watt_cad125_max = 945;

    const uint16_t watt_cad130_min = 360;
    const uint16_t watt_cad130_max = 1045;

    if(currentSpeed().value() <= 0) return 0;

    double vwatts=((9.8* settings.value("weight",75).toDouble() * (currentInclination().value()/100.0)));

    if(currentCadence() < 41)
        return((((watt_cad40_max-watt_cad40_min) / (max_resistance - 1)) * (currentResistance() - 1))+watt_cad40_min) + vwatts;
    else if(currentCadence() < 46)
        return((((watt_cad45_max-watt_cad45_min) / (max_resistance - 1)) * (currentResistance() - 1))+watt_cad45_min) + vwatts;
    else if(currentCadence() < 51)
        return((((watt_cad50_max-watt_cad50_min) / (max_resistance - 1)) * (currentResistance() - 1))+watt_cad50_min) + vwatts;
    else if(currentCadence() < 56)
        return((((watt_cad55_max-watt_cad55_min) / (max_resistance - 1)) * (currentResistance() - 1))+watt_cad55_min) + vwatts;
    else if(currentCadence() < 61)
        return((((watt_cad60_max-watt_cad60_min) / (max_resistance - 1)) * (currentResistance() - 1))+watt_cad60_min) + vwatts;
    else if(currentCadence() < 66)
        return((((watt_cad65_max-watt_cad65_min) / (max_resistance - 1)) * (currentResistance() - 1))+watt_cad65_min) + vwatts;
    else if(currentCadence() < 71)
        return((((watt_cad70_max-watt_cad70_min) / (max_resistance - 1)) * (currentResistance() - 1))+watt_cad70_min) + vwatts;
    else if(currentCadence() < 76)
        return((((watt_cad75_max-watt_cad75_min) / (max_resistance - 1)) * (currentResistance() - 1))+watt_cad75_min) + vwatts;
    else if(currentCadence() < 81)
        return((((watt_cad80_max-watt_cad80_min) / (max_resistance - 1)) * (currentResistance() - 1))+watt_cad80_min) + vwatts;
    else if(currentCadence() < 86)
        return((((watt_cad85_max-watt_cad85_min) / (max_resistance - 1)) * (currentResistance() - 1))+watt_cad85_min) + vwatts;
    else if(currentCadence() < 91)
        return((((watt_cad90_max-watt_cad90_min) / (max_resistance - 1)) * (currentResistance() - 1))+watt_cad90_min) + vwatts;
    else if(currentCadence() < 96)
        return((((watt_cad95_max-watt_cad95_min) / (max_resistance - 1)) * (currentResistance() - 1))+watt_cad95_min) + vwatts;
    else if(currentCadence() < 101)
        return((((watt_cad100_max-watt_cad100_min) / (max_resistance - 1)) * (currentResistance() - 1))+watt_cad100_min) + vwatts;
    else if(currentCadence() < 106)
        return((((watt_cad105_max-watt_cad105_min) / (max_resistance - 1)) * (currentResistance() - 1))+watt_cad105_min) + vwatts;
    else if(currentCadence() < 111)
        return((((watt_cad110_max-watt_cad110_min) / (max_resistance - 1)) * (currentResistance() - 1))+watt_cad110_min) + vwatts;
    else if(currentCadence() < 116)
        return((((watt_cad115_max-watt_cad115_min) / (max_resistance - 1)) * (currentResistance() - 1))+watt_cad115_min) + vwatts;
    else if(currentCadence() < 121)
        return((((watt_cad120_max-watt_cad120_min) / (max_resistance - 1)) * (currentResistance() - 1))+watt_cad120_min) + vwatts;
    else if(currentCadence() < 126)
        return((((watt_cad125_max-watt_cad125_min) / (max_resistance - 1)) * (currentResistance() - 1))+watt_cad125_min) + vwatts;
    else
        return((((watt_cad130_max-watt_cad130_min) / (max_resistance - 1)) * (currentResistance() - 1))+watt_cad130_min) + vwatts;
    return 0;
}

void domyoselliptical::controllerStateChanged(QLowEnergyController::ControllerState state)
{
    qDebug() << "controllerStateChanged" << state;
    if(state == QLowEnergyController::UnconnectedState && m_control)
    {
        qDebug() << "trying to connect back again...";
        initDone = false;
        m_control->connectToDevice();
    }
}
