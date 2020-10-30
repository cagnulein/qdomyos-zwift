#include "domyosbike.h"
#include "virtualbike.h"
#include <QFile>
#include <QDateTime>
#include <QMetaEnum>
#include <QBluetoothLocalDevice>

domyosbike::domyosbike()
{
    refresh = new QTimer(this);
    initDone = false;
    connect(refresh, SIGNAL(timeout()), this, SLOT(update()));
    refresh->start(200);
}

void domyosbike::writeCharacteristic(uint8_t* data, uint8_t data_len, QString info, bool disable_log)
{
    QEventLoop loop;
    connect(gattCommunicationChannelService, SIGNAL(characteristicWritten(QLowEnergyCharacteristic,QByteArray)),
            &loop, SLOT(quit()));

    gattCommunicationChannelService->writeCharacteristic(gattWriteCharacteristic, QByteArray::fromRawData((const char*)data, data_len));

    if(!disable_log)
        debug(" >> " + QByteArray((const char*)data, data_len).toHex(' ') + " // " + info);

    loop.exec();
}

void domyosbike::updateDisplay(uint16_t elapsed)
{
   uint8_t display[] = {0xf0, 0xcb, 0x03, 0x00, 0x00, 0xff, 0x01, 0x00, 0x00, 0x02,
                        0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x01, 0x01, 0x00,
                        0x00, 0x01, 0xff, 0xff, 0xff, 0xff, 0x00};

   display[3] = (elapsed / 60) & 0xFF; // high byte for elapsed time (in seconds)
   display[4] = (elapsed % 60 & 0xFF); // low byte for elasped time (in seconds)

   display[7] = ((uint8_t)((uint16_t)(currentSpeed()) >> 8)) & 0xFF;
   display[8] = (uint8_t)(currentSpeed()) & 0xFF;

   display[12] = currentHeart();

   //display[13] = ((((uint8_t)calories())) >> 8) & 0xFF;
   //display[14] = (((uint8_t)calories())) & 0xFF;

   display[16] = (uint8_t)currentCadence();

   display[19] = ((((uint16_t)odometer())) >> 8) & 0xFF;
   display[20] = (((uint16_t)odometer())) & 0xFF;

   for(uint8_t i=0; i<sizeof(display)-1; i++)
   {
      display[26] += display[i]; // the last byte is a sort of a checksum
   }

   writeCharacteristic(display, 20, "updateDisplay elapsed=" + QString::number(elapsed) );
   writeCharacteristic(&display[20], sizeof (display) - 20, "updateDisplay elapsed=" + QString::number(elapsed) );
}

void domyosbike::forceResistance(int8_t requestResistance)
{
   uint8_t write[] = {0xf0, 0xad, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                      0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x01, 0xff,
                      0xff, 0xff, 0x00};

   write[10] = requestResistance;

   for(uint8_t i=0; i<sizeof(write)-1; i++)
   {
      write[22] += write[i]; // the last byte is a sort of a checksum
   }

   writeCharacteristic(write, 20, "forceResistance " + QString::number(requestResistance));
   writeCharacteristic(&write[20], sizeof (write) - 20, "forceResistance " + QString::number(requestResistance));
}

void domyosbike::update()
{
    uint8_t noOpData[] = { 0xf0, 0xac, 0x9c };

    // stop tape
    uint8_t initDataF0C800B8[] = { 0xf0, 0xc8, 0x00, 0xb8 };

    static uint8_t sec1 = 0;
    //qDebug() << treadmill.isValid() << m_control->state() << gattCommunicationChannelService << gattWriteCharacteristic.isValid() << gattNotifyCharacteristic.isValid() << initDone;

    if(initRequest)
    {
        initRequest = false;
        btinit(false);
    }
    else if(btbike.isValid() &&
       m_control->state() == QLowEnergyController::DiscoveredState &&
       gattCommunicationChannelService &&
       gattWriteCharacteristic.isValid() &&
       gattNotifyCharacteristic.isValid() &&
       initDone)
    {
        if(currentSpeed() > 0.0)
           elapsed += ((double)refresh->interval() / 1000.0);

        // updating the treadmill console every second
        if(sec1++ == (1000 / refresh->interval()))
        {
            sec1 = 0;
            updateDisplay(elapsed);
        }

        writeCharacteristic(noOpData, sizeof(noOpData), "noOp", true);

        if(requestResistance != -1)
        {
           if(requestResistance != currentResistance())
           {
              debug("writing resistance " + QString::number(requestResistance));
              forceResistance(requestResistance);
           }
           requestResistance = -1;
        }
        if(requestStart != -1)
        {
           debug("starting...");
           btinit(true);
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

void domyosbike::serviceDiscovered(const QBluetoothUuid &gatt)
{
    debug("serviceDiscovered " + gatt.toString());
}

static QByteArray lastPacket;
void domyosbike::characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue)
{
    //qDebug() << "characteristicChanged" << characteristic.uuid() << newValue << newValue.length();
    Q_UNUSED(characteristic);
    static QTime lastRefresh = QTime::currentTime();

    debug(" << " + newValue.toHex(' '));

    if (lastPacket.length() && lastPacket == newValue)
        return;

    lastPacket = newValue;
    if (newValue.length() != 26)
        return;

    if (newValue.at(22) == 0x06)
    {
        debug("start button pressed!");
        requestStart = 1;
    }
    else if (newValue.at(22) == 0x07)
    {
        debug("stop button pressed!");
        requestStop = 1;
    }

    /*if ((uint8_t)newValue.at(1) != 0xbc && newValue.at(2) != 0x04)  // intense run, these are the bytes for the inclination and speed status
        return;*/

    double speed = GetSpeedFromPacket(newValue);
    double kcal = GetKcalFromPacket(newValue);
    double distance = GetDistanceFromPacket(newValue);

    Cadence = newValue.at(9);
    Resistance = newValue.at(14);
    Heart = newValue.at(18);

    CrankRevs += ((double)(lastRefresh.msecsTo(QTime::currentTime())) * ((double)Cadence / 60000.0) );
    LastCrankEventTime += (uint16_t)((lastRefresh.msecsTo(QTime::currentTime())) * 0.9765625);
    lastRefresh = QTime::currentTime();

    debug("Current speed: " + QString::number(speed));
    debug("Current cadence: " + QString::number(Cadence));
    debug("Current resistance: " + QString::number(Resistance));
    debug("Current heart: " + QString::number(Heart));
    debug("Current KCal: " + QString::number(kcal));
    debug("Current Distance: " + QString::number(distance));
    debug("Current CrankRevs: " + QString::number(CrankRevs));
    debug("Last CrankEventTime: " + QString::number(LastCrankEventTime));

    if(m_control->error() != QLowEnergyController::NoError)
        qDebug() << "QLowEnergyController ERROR!!" << m_control->errorString();

    Speed = speed;
    KCal = kcal;
    Distance = distance;    
}

double domyosbike::GetSpeedFromPacket(QByteArray packet)
{
    uint8_t convertedData = (uint8_t)packet.at(7);
    double data = (double)convertedData / 10.0f;
    return data;
}

double domyosbike::GetKcalFromPacket(QByteArray packet)
{
    uint16_t convertedData = (packet.at(10) << 8) | packet.at(11);
    return (double)convertedData;
}

double domyosbike::GetDistanceFromPacket(QByteArray packet)
{
    uint16_t convertedData = (packet.at(12) << 8) | packet.at(13);
    double data = ((double)convertedData) / 10.0f;
    return data;
}

void domyosbike::btinit(bool startTape)
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

    writeCharacteristic(initData1, sizeof(initData1), "init");
    writeCharacteristic(initData2, sizeof(initData2), "init");
    writeCharacteristic(initDataStart, sizeof(initDataStart), "init");
    writeCharacteristic(initDataStart2, sizeof(initDataStart2), "init");
    writeCharacteristic(initDataStart3, sizeof(initDataStart3), "init");
    writeCharacteristic(initDataStart4, sizeof(initDataStart4), "init");
    writeCharacteristic(initDataStart5, sizeof(initDataStart5), "init");
    writeCharacteristic(initDataStart6, sizeof(initDataStart6), "init");
    writeCharacteristic(initDataStart7, sizeof(initDataStart7), "init");
    writeCharacteristic(initDataStart8, sizeof(initDataStart8), "init");
    writeCharacteristic(initDataStart9, sizeof(initDataStart9), "init");
    writeCharacteristic(initDataStart10, sizeof(initDataStart10), "init");
    if(startTape)
    {
        writeCharacteristic(initDataStart11, sizeof(initDataStart11), "init");
        writeCharacteristic(initDataStart12, sizeof(initDataStart12), "init");
        writeCharacteristic(initDataStart13, sizeof(initDataStart13), "init");
    }

    initDone = true;
}

void domyosbike::stateChanged(QLowEnergyService::ServiceState state)
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

        // ******************************************* virtual bike init *************************************
        static uint8_t first = 0;
        if(!first)
        {
           debug("creating virtual bike interface...");
           virtualBike = new virtualbike(this);
           connect(virtualBike,&virtualbike::debug ,this,&domyosbike::debug);
        }
        first = 1;
        // ********************************************************************************************************

	    QByteArray descriptor;
	    descriptor.append((char)0x01);
	    descriptor.append((char)0x00);
	    gattCommunicationChannelService->writeDescriptor(gattNotifyCharacteristic.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration), descriptor);
    }
}

void domyosbike::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue)
{
    debug("descriptorWritten " + descriptor.name() + " " + newValue.toHex(' '));

    initRequest = true;
}

void domyosbike::characteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue)
{
    Q_UNUSED(characteristic);
    debug("characteristicWritten " + newValue.toHex(' '));
}

void domyosbike::serviceScanDone(void)
{
    debug("serviceScanDone");

    QBluetoothUuid _gattCommunicationChannelServiceId((QString)"49535343-fe7d-4ae5-8fa9-9fafd205e455");

    gattCommunicationChannelService = m_control->createServiceObject(_gattCommunicationChannelServiceId);
    connect(gattCommunicationChannelService, SIGNAL(stateChanged(QLowEnergyService::ServiceState)), this, SLOT(stateChanged(QLowEnergyService::ServiceState)));
    gattCommunicationChannelService->discoverDetails();
}

void domyosbike::errorService(QLowEnergyService::ServiceError err)
{
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    debug("domyosbike::errorService" + QString::fromLocal8Bit(metaEnum.valueToKey(err)) + m_control->errorString());
}

void domyosbike::error(QLowEnergyController::Error err)
{
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    debug("domyosbike::error" + QString::fromLocal8Bit(metaEnum.valueToKey(err)) + m_control->errorString());
}

void domyosbike::deviceDiscovered(const QBluetoothDeviceInfo &device)
{
    debug("Found new device: " + device.name() + " (" + device.address().toString() + ')');
    if(device.name().startsWith("Domyos-Bike") && !device.name().startsWith("DomyosBridge"))
    {
        btbike = device;
        m_control = QLowEnergyController::createCentral(btbike, this);
        connect(m_control, SIGNAL(serviceDiscovered(const QBluetoothUuid &)),
                this, SLOT(serviceDiscovered(const QBluetoothUuid &)));
        connect(m_control, SIGNAL(discoveryFinished()),
                this, SLOT(serviceScanDone()));
        connect(m_control, SIGNAL(error(QLowEnergyController::Error)),
                this, SLOT(error(QLowEnergyController::Error)));

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

bool domyosbike::connected()
{
    if(!m_control)
        return false;
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

void* domyosbike::VirtualBike()
{
    return virtualBike;
}

void* domyosbike::VirtualDevice()
{
    return VirtualBike();
}
