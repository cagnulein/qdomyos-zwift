#include "domyostreadmill.h"
#include "virtualtreadmill.h"
#include <QFile>
#include <QDateTime>
#include <QMetaEnum>

// set speed and incline to 0
uint8_t initData1[] = { 0xf0, 0xc8, 0x01, 0xb9 };
uint8_t initData2[] = { 0xf0, 0xc9, 0xb9 };

uint8_t noOpData[] = { 0xf0, 0xac, 0x9c };

// stop tape
uint8_t initDataF0C800B8[] = { 0xf0, 0xc8, 0x00, 0xb8 };

#if 0
uint8_t initDataStart[] = { 0xf0, 0xc8, 0x00, 0xb8 };
uint8_t initDataStart2[] = { 0xf0, 0xcb, 0x01, 0x00, 0x00, 0x02, 0xff, 0xff, 0xff, 0xff,
                             0xff, 0xff, 0xff, 0xff, 0x01, 0x00, 0x00, 0x01, 0x01, 0x00 };
uint8_t initDataStart3[] = { 0x00, 0x01, 0xff, 0xff, 0xff, 0xff, 0xb6 };
uint8_t initDataStart4[] = { 0xf0, 0xc8, 0x00, 0xb8 };
uint8_t initDataStart5[] = { 0xf0, 0xc8, 0x01, 0xb9 };
uint8_t initDataStart6[] =
{
        0xf0, 0xad, 0xff, 0xff, 0x00, 0x0a, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};
uint8_t initDataStart7[] = { 0xff, 0xff, 0x95 };
uint8_t initDataStart8[] =
{
        0xf0, 0xad, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff
};
uint8_t initDataStart9[] = { 0xff, 0xff, 0x8b };
uint8_t initDataStart10[] =
{
        0xf0, 0xad, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0x03, 0xfc, 0xff, 0xff, 0xff, 0xff, 0xff
};
uint8_t initDataStart11[] = { 0xff, 0xff, 0x8a };
uint8_t initDataStart12[] =
{
        0xf0, 0xad, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0x04, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff
};
uint8_t initDataStart13[] = { 0xff, 0xff, 0x9e };
#endif

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

QBluetoothUuid _gattCommunicationChannelServiceId((QString)"49535343-fe7d-4ae5-8fa9-9fafd205e455");
QBluetoothUuid _gattWriteCharacteristicId((QString)"49535343-8841-43f4-a8d4-ecbe34729bb3");
QBluetoothUuid _gattNotifyCharacteristicId((QString)"49535343-1e4d-4bd9-ba61-23c647249616");

QBluetoothDeviceInfo treadmill;
QLowEnergyController* m_control = 0;
QLowEnergyService* gattCommunicationChannelService = 0;
QLowEnergyCharacteristic gattWriteCharacteristic;
QLowEnergyCharacteristic gattNotifyCharacteristic;
QBluetoothDeviceDiscoveryAgent *discoveryAgent;

bool initDone = false;
bool initRequest = false;

extern volatile double currentSpeed;
extern volatile double currentIncline;
extern volatile uint8_t currentHeart;
extern volatile double requestSpeed;
extern volatile double requestIncline;
extern volatile int8_t requestStart;
extern volatile int8_t requestStop;

QFile* debugCommsLog;

domyostreadmill::domyostreadmill()
{
    QLoggingCategory::setFilterRules(QStringLiteral("qt.bluetooth* = true"));
    QTimer* refresh = new QTimer(this);
    debugCommsLog = new QFile("debug-" + QDateTime::currentDateTime().toString() + ".log");
    debugCommsLog->open(QIODevice::WriteOnly | QIODevice::Unbuffered);

    initDone = false;

    // Create a discovery agent and connect to its signals
    discoveryAgent = new QBluetoothDeviceDiscoveryAgent(this);
    connect(discoveryAgent, SIGNAL(deviceDiscovered(QBluetoothDeviceInfo)),
            this, SLOT(deviceDiscovered(QBluetoothDeviceInfo)));

    // Start a discovery
    discoveryAgent->start(QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);

    connect(refresh, SIGNAL(timeout()), this, SLOT(update()));
    refresh->start(200);
}

void domyostreadmill::debug(QString text)
{
    QString debug = QDateTime::currentDateTime().toString() + text + '\n';
    debugCommsLog->write(debug.toLocal8Bit());
    qDebug() << debug;
}

void domyostreadmill::writeCharacteristic(uint8_t* data, uint8_t data_len, QString info, bool disable_log)
{
    QEventLoop loop;
    connect(gattCommunicationChannelService, SIGNAL(characteristicWritten(QLowEnergyCharacteristic,QByteArray)),
            &loop, SLOT(quit()));

    gattCommunicationChannelService->writeCharacteristic(gattWriteCharacteristic, QByteArray::fromRawData((const char*)data, data_len));

    if(!disable_log)
        debug(" >> " + QByteArray((const char*)data, data_len).toHex(' ') + " // " + info);    

    loop.exec();
}

void domyostreadmill::updateDisplay(uint16_t elapsed)
{
   uint8_t writeIncline[] = {0xf0, 0xcb, 0x03, 0x00, 0x00, 0xff, 0x01, 0x00, 0x00, 0x02,
			     0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x01, 0x01, 0x00,
                             0x00, 0x01, 0xff, 0xff, 0xff, 0xff, 0x00};

   writeIncline[3] = (elapsed >> 8) & 0xFF; // high byte for elapsed time (in seconds)
   writeIncline[4] = (elapsed & 0xFF); // low byte for elasped time (in seconds)

   writeIncline[12] = currentHeart;

   for(uint8_t i=0; i<sizeof(writeIncline)-1; i++)
   {
      //qDebug() << QString::number(writeIncline[i], 16);
      writeIncline[26] += writeIncline[i]; // the last byte is a sort of a checksum
   }

   //qDebug() << "writeIncline crc" << QString::number(writeIncline[26], 16);


   writeCharacteristic(writeIncline, 20, "updateDisplay speed=" + QString::number(requestSpeed) + " incline=" + QString::number(requestIncline) + " elapsed=" + QString::number(elapsed) );
   writeCharacteristic(&writeIncline[20], sizeof (writeIncline) - 20, "updateDisplay speed=" + QString::number(requestSpeed) + " incline=" + QString::number(requestIncline) + " elapsed=" + QString::number(elapsed) );
}

void domyostreadmill::forceSpeedOrIncline(double requestSpeed, double requestIncline)
{
   uint8_t writeIncline[] = {0xf0, 0xad, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff,
			     0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                             0xff, 0xff, 0x00};

   writeIncline[4] = ((uint16_t)(requestSpeed*10) >> 8) & 0xFF;
   writeIncline[5] = ((uint16_t)(requestSpeed*10) & 0xFF);

   writeIncline[13] = ((uint16_t)(requestIncline*10) >> 8) & 0xFF;
   writeIncline[14] = ((uint16_t)(requestIncline*10) & 0xFF);

   for(uint8_t i=0; i<sizeof(writeIncline)-1; i++)
   {
      //qDebug() << QString::number(writeIncline[i], 16);
      writeIncline[22] += writeIncline[i]; // the last byte is a sort of a checksum
   }

   //qDebug() << "writeIncline crc" << QString::number(writeIncline[26], 16);


   writeCharacteristic(writeIncline, 20, "forceSpeedOrIncline speed=" + QString::number(requestSpeed) + " incline=" + QString::number(requestIncline));
   writeCharacteristic(&writeIncline[20], sizeof (writeIncline) - 20, "forceSpeedOrIncline speed=" + QString::number(requestSpeed) + " incline=" + QString::number(requestIncline));
}


void domyostreadmill::update()
{        
    static uint32_t counter = 0;

    //qDebug() << treadmill.isValid() << m_control->state() << gattCommunicationChannelService << gattWriteCharacteristic.isValid() << gattNotifyCharacteristic.isValid() << initDone;

    if(initRequest)
    {
        initRequest = false;
        btinit();
    }
    else if(treadmill.isValid() &&
       m_control->state() == QLowEnergyController::DiscoveredState &&
       gattCommunicationChannelService &&
       gattWriteCharacteristic.isValid() &&
       gattNotifyCharacteristic.isValid() &&
       initDone)
    {
        if(currentSpeed > 0.0)
            counter++;

        writeCharacteristic(noOpData, sizeof(noOpData), "noOp", true);

        // byte 3 - 4 = elapsed time
        // byte 17    = inclination

        if(requestSpeed != -1)
        {
           debug("writing speed " + QString::number(requestSpeed));
           forceSpeedOrIncline(requestSpeed, currentIncline);
           requestSpeed = -1;
        }
        if(requestIncline != -1)
        {
           debug("writing incline " + QString::number(requestIncline));
           forceSpeedOrIncline(currentSpeed, requestIncline);
           requestIncline = -1;
        }
        if(requestStart != -1)
        {
           debug("starting...");
           btinit();
           requestStart = -1;
        }
        if(requestStop != -1)
        {
            debug("stopping...");
            writeCharacteristic(initDataF0C800B8, sizeof(initDataF0C800B8), "stop tape");
            requestStop = -1;
        }
    }
}

void domyostreadmill::serviceDiscovered(const QBluetoothUuid &gatt)
{
    debug("serviceDiscovered " + gatt.toString());
}

static QByteArray lastPacket;
void domyostreadmill::characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue)
{
    //qDebug() << "characteristicChanged" << characteristic.uuid() << newValue << newValue.length();
    Q_UNUSED(characteristic);

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
    double incline = GetInclinationFromPacket(newValue);

    currentHeart = newValue.at(18);

    debug("Current speed: " + QString::number(speed));
    debug("Current incline: " + QString::number(incline));
    debug("Current heart: " + QString::number(currentHeart));

    if(m_control->error() != QLowEnergyController::NoError)
        qDebug() << "QLowEnergyController ERROR!!" << m_control->errorString();

    currentSpeed = speed;
    currentIncline = incline;
}

double domyostreadmill::GetSpeedFromPacket(QByteArray packet)
{
    uint8_t convertedData = (uint8_t)packet.at(7);
    double data = (double)convertedData / 10.0f;
    return data;
}

double domyostreadmill::GetInclinationFromPacket(QByteArray packet)
{
    uint16_t convertedData = (packet.at(2) << 8) | packet.at(3);
    double data = ((double)convertedData - 1000.0f) / 10.0f;
    if (data < 0) return 0;
    return data;
}

void domyostreadmill::btinit()
{
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
    writeCharacteristic(initDataStart11, sizeof(initDataStart11), "init");
    writeCharacteristic(initDataStart12, sizeof(initDataStart12), "init");
    writeCharacteristic(initDataStart13, sizeof(initDataStart13), "init");

    initDone = true;
}

void domyostreadmill::stateChanged(QLowEnergyService::ServiceState state)
{
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

        // ******************************************* virtual treadmill init *************************************
        static uint8_t first = 0;
        static virtualtreadmill* v;
        Q_UNUSED(v);
        if(!first)
        {
           debug("creating virtual treadmill interface...");
           v = new virtualtreadmill();
        }
        first = 1;
        // ********************************************************************************************************

	    QByteArray descriptor;
	    descriptor.append((char)0x01);
	    descriptor.append((char)0x00);
	    gattCommunicationChannelService->writeDescriptor(gattNotifyCharacteristic.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration), descriptor);
    }
}

void domyostreadmill::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue)
{
    debug("descriptorWritten " + descriptor.name() + " " + newValue.toHex(' '));

    initRequest = true;
}

void domyostreadmill::characteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue)
{
    Q_UNUSED(characteristic);
    debug("characteristicWritten " + newValue.toHex(' '));
}

void domyostreadmill::serviceScanDone(void)
{
    debug("serviceScanDone");

    gattCommunicationChannelService = m_control->createServiceObject(_gattCommunicationChannelServiceId);
    connect(gattCommunicationChannelService, SIGNAL(stateChanged(QLowEnergyService::ServiceState)), this, SLOT(stateChanged(QLowEnergyService::ServiceState)));
    gattCommunicationChannelService->discoverDetails();
}

void domyostreadmill::errorService(QLowEnergyService::ServiceError err)
{
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    debug("domyostreadmill::errorService" + QString::fromLocal8Bit(metaEnum.valueToKey(err)) + m_control->errorString());
}

void domyostreadmill::error(QLowEnergyController::Error err)
{
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    debug("domyostreadmill::error" + QString::fromLocal8Bit(metaEnum.valueToKey(err)) + m_control->errorString());
}

void domyostreadmill::deviceDiscovered(const QBluetoothDeviceInfo &device)
{
    debug("Found new device: " + device.name() + " (" + device.address().toString() + ')');
    if(device.name().startsWith("Domyos"))
    {
        discoveryAgent->stop();
        treadmill = device;
        m_control = QLowEnergyController::createCentral(treadmill, this);
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
            exit(1);
        });
        connect(m_control, &QLowEnergyController::connected, this, [this]() {
            Q_UNUSED(this);
            debug("Controller connected. Search services...");
            m_control->discoverServices();
        });
        connect(m_control, &QLowEnergyController::disconnected, this, [this]() {
            Q_UNUSED(this);
            debug("LowEnergy controller disconnected");
            exit(2);
        });

        // Connect
        m_control->connectToDevice();
        return;
    }
}
