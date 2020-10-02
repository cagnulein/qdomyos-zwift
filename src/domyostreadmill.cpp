#include "domyostreadmill.h"
#include "virtualtreadmill.h"


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
uint8_t initDataStart5[] = { 0xf0, 0xcb, 0x03, 0x00, 0x00, 0xff, 0x01, 0x00, 0x00, 0x02,
                             0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x01, 0x01, 0x00 };
uint8_t initDataStart6[] = { 0x00, 0x01, 0xff, 0xff, 0xff, 0xff, 0xc1 };
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

extern volatile double currentSpeed;
extern volatile double currentIncline;
extern volatile uint8_t currentHeart;
extern volatile double requestSpeed;
extern volatile double requestIncline;
extern volatile int8_t requestStart;
extern volatile int8_t requestStop;

domyostreadmill::domyostreadmill()
{
    QTimer* refresh = new QTimer(this);

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

void domyostreadmill::forceSpeedOrIncline(double requestSpeed, double requestIncline, uint16_t elapsed)
{
   uint8_t writeIncline[] = {0xf0, 0xcb, 0x03, 0x00, 0x00, 0xff, 0x01, 0x00, 0x00, 0x02,
			     0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x01, 0x01, 0x00,
                             (uint8_t)(requestSpeed * 10), 0x01, 0xff, 0xff, 0xff, 0xff, 0x00};

   writeIncline[3] = (elapsed >> 8) & 0xFF; // high byte for elapsed time (in seconds)
   writeIncline[4] = (elapsed & 0xFF); // low byte for elasped time (in seconds)

   writeIncline[12] = currentHeart;

   writeIncline[16] = (uint8_t)(requestIncline * 10);

   for(uint8_t i=0; i<sizeof(writeIncline)-1; i++)
   {
      //qDebug() << QString::number(writeIncline[i], 16);
      writeIncline[26] += writeIncline[i]; // the last byte is a sort of a checksum
   }

   //qDebug() << "writeIncline crc" << QString::number(writeIncline[26], 16);

   gattCommunicationChannelService->writeCharacteristic(gattWriteCharacteristic, QByteArray::fromRawData((const char*)writeIncline, sizeof(writeIncline)));
}

void domyostreadmill::update()
{
    static uint8_t first = 0;
    static virtualtreadmill* v;
    static uint32_t counter = 0;
    Q_UNUSED(v);
    //qDebug() << treadmill.isValid() << m_control->state() << gattCommunicationChannelService << gattWriteCharacteristic.isValid() << gattNotifyCharacteristic.isValid() << initDone;
    if(treadmill.isValid() &&
       (m_control->state() == QLowEnergyController::ConnectedState || m_control->state() == QLowEnergyController::DiscoveredState) &&
       gattCommunicationChannelService &&
       gattWriteCharacteristic.isValid() &&
       gattNotifyCharacteristic.isValid() &&
       initDone)
    {
        counter++;
        if(!first)
        {
           qDebug() << "creating virtual treadmill interface...";
           v = new virtualtreadmill();
        }
        first = 1;
        gattCommunicationChannelService->writeCharacteristic(gattWriteCharacteristic, QByteArray::fromRawData((const char*)noOpData, sizeof(noOpData)));

        // byte 3 - 4 = elapsed time
        // byte 17    = inclination

        if(requestSpeed != -1)
        {
           qDebug() << "writing speed" << requestSpeed;
           forceSpeedOrIncline(requestSpeed, currentIncline, counter/5);
           requestSpeed = -1;
        }
        if(requestIncline != -1)
        {
           qDebug() << "writing incline" << requestIncline;
           forceSpeedOrIncline(currentSpeed, requestIncline, counter/5);
           requestIncline = -1;
        }
        if(requestStart != -1)
        {
           qDebug() << "starting...";
           btinit();
           requestStart = -1;
        }
        if(requestStop != -1)
        {
            qDebug() << "stopping...";
            gattCommunicationChannelService->writeCharacteristic(gattWriteCharacteristic, QByteArray::fromRawData((const char*)initDataF0C800B8, sizeof(initDataF0C800B8)));
            requestStop = -1;
        }
    }
}

void domyostreadmill::serviceDiscovered(const QBluetoothUuid &gatt)
{
    qDebug() << "serviceDiscovered" << gatt;
}

static QByteArray lastPacket;
void domyostreadmill::characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue)
{
    //qDebug() << "characteristicChanged" << characteristic.uuid() << newValue << newValue.length();
    Q_UNUSED(characteristic);

    if (lastPacket.length() && lastPacket == newValue)
        return;

    lastPacket = newValue;
    if (newValue.length() != 26)
        return;

    if (newValue.at(22) == 0x06)
    {
        qDebug() << "START PRESSED!";
        requestStart = 1;
    }
    else if (newValue.at(22) == 0x07)
    {
        qDebug() << "STOP PRESSED!";
        requestStop = 1;
    }

    /*if ((uint8_t)newValue.at(1) != 0xbc && newValue.at(2) != 0x04)  // intense run, these are the bytes for the inclination and speed status
        return;*/

    double speed = GetSpeedFromPacket(newValue);
    double incline = GetInclinationFromPacket(newValue);
    //var isStartPressed = GetIsStartPressedFromPacket(currentPacket);
    //var isStopPressed = GetIsStopPressedFromPacket(currentPacket);

#if DEBUG
    Debug.WriteLine(args.CharacteristicValue.ToArray().HexDump());
#endif

    currentHeart = newValue.at(18);

    qDebug() << "Current speed: " << speed;
    qDebug() << "Current incline: " << incline;
    qDebug() << "Current heart:" << currentHeart;

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
    qDebug() << convertedData;
    double data = ((double)convertedData - 1000.0f) / 10.0f;
    if (data < 0) return 0;
    return data;
}

void domyostreadmill::btinit()
{
    gattCommunicationChannelService->writeCharacteristic(gattWriteCharacteristic, QByteArray::fromRawData((const char*)initData1, sizeof(initData1)));
    gattCommunicationChannelService->writeCharacteristic(gattWriteCharacteristic, QByteArray::fromRawData((const char*)initData2, sizeof(initData2)));
    gattCommunicationChannelService->writeCharacteristic(gattWriteCharacteristic, QByteArray::fromRawData((const char*)initDataStart, sizeof(initDataStart)));
    gattCommunicationChannelService->writeCharacteristic(gattWriteCharacteristic, QByteArray::fromRawData((const char*)initDataStart2, sizeof(initDataStart2)));
    gattCommunicationChannelService->writeCharacteristic(gattWriteCharacteristic, QByteArray::fromRawData((const char*)initDataStart3, sizeof(initDataStart3)));
    gattCommunicationChannelService->writeCharacteristic(gattWriteCharacteristic, QByteArray::fromRawData((const char*)initDataStart4, sizeof(initDataStart4)));
    gattCommunicationChannelService->writeCharacteristic(gattWriteCharacteristic, QByteArray::fromRawData((const char*)initDataStart5, sizeof(initDataStart5)));
    gattCommunicationChannelService->writeCharacteristic(gattWriteCharacteristic, QByteArray::fromRawData((const char*)initDataStart6, sizeof(initDataStart6)));
    gattCommunicationChannelService->writeCharacteristic(gattWriteCharacteristic, QByteArray::fromRawData((const char*)initDataStart7, sizeof(initDataStart7)));
    gattCommunicationChannelService->writeCharacteristic(gattWriteCharacteristic, QByteArray::fromRawData((const char*)initDataStart8, sizeof(initDataStart8)));
    gattCommunicationChannelService->writeCharacteristic(gattWriteCharacteristic, QByteArray::fromRawData((const char*)initDataStart9, sizeof(initDataStart9)));
    gattCommunicationChannelService->writeCharacteristic(gattWriteCharacteristic, QByteArray::fromRawData((const char*)initDataStart10, sizeof(initDataStart10)));
    gattCommunicationChannelService->writeCharacteristic(gattWriteCharacteristic, QByteArray::fromRawData((const char*)initDataStart11, sizeof(initDataStart11)));
    gattCommunicationChannelService->writeCharacteristic(gattWriteCharacteristic, QByteArray::fromRawData((const char*)initDataStart12, sizeof(initDataStart12)));
    gattCommunicationChannelService->writeCharacteristic(gattWriteCharacteristic, QByteArray::fromRawData((const char*)initDataStart13, sizeof(initDataStart13)));
}

void domyostreadmill::stateChanged(QLowEnergyService::ServiceState state)
{
    qDebug() << "stateChanged" << state;
    if(state == QLowEnergyService::ServiceDiscovered)
    {
	    //qDebug() << gattCommunicationChannelService->characteristics();

	    gattWriteCharacteristic = gattCommunicationChannelService->characteristic(_gattWriteCharacteristicId);
	    gattNotifyCharacteristic = gattCommunicationChannelService->characteristic(_gattNotifyCharacteristicId);

	    // establish hook into notifications
	    connect(gattCommunicationChannelService, SIGNAL(characteristicChanged(QLowEnergyCharacteristic,QByteArray)),
        	    this, SLOT(characteristicChanged(QLowEnergyCharacteristic,QByteArray)));

	    // await _gattNotifyCharacteristic.WriteClientCharacteristicConfigurationDescriptorAsync(GattClientCharacteristicConfigurationDescriptorValue.Notify);
	    QByteArray descriptor;
	    descriptor.append((char)0x01);
	    descriptor.append((char)0x00);
	    gattCommunicationChannelService->writeDescriptor(gattNotifyCharacteristic.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration), descriptor);

        btinit();

	    initDone = true;

    }
}

void domyostreadmill::serviceScanDone(void)
{
    qDebug() << "serviceScanDone";

    gattCommunicationChannelService = m_control->createServiceObject(_gattCommunicationChannelServiceId);
    connect(gattCommunicationChannelService, SIGNAL(stateChanged(QLowEnergyService::ServiceState)), this, SLOT(stateChanged(QLowEnergyService::ServiceState)));
    gattCommunicationChannelService->discoverDetails();
}

void domyostreadmill::deviceDiscovered(const QBluetoothDeviceInfo &device)
{
    qDebug() << "Found new device:" << device.name() << '(' << device.address().toString() << ')';
    if(device.name().startsWith("Domyos"))
    {
        discoveryAgent->stop();
        treadmill = device;
        m_control = QLowEnergyController::createCentral(treadmill, this);
        connect(m_control, SIGNAL(serviceDiscovered(const QBluetoothUuid &)),
                this, SLOT(serviceDiscovered(const QBluetoothUuid &)));
        connect(m_control, SIGNAL(discoveryFinished()),
                this, SLOT(serviceScanDone()));

        connect(m_control, static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error),
                this, [this](QLowEnergyController::Error error) {
            Q_UNUSED(error);
            Q_UNUSED(this);
            qDebug() << "Cannot connect to remote device.";
            exit(1);
        });
        connect(m_control, &QLowEnergyController::connected, this, [this]() {
            Q_UNUSED(this);
            qDebug() << "Controller connected. Search services...";
            m_control->discoverServices();
        });
        connect(m_control, &QLowEnergyController::disconnected, this, [this]() {
            Q_UNUSED(this);
            qDebug() << "LowEnergy controller disconnected";
            exit(2);
        });

        // Connect
        m_control->connectToDevice();
        return;
    }
}
