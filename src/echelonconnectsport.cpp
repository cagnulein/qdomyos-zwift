#include "echelonconnectsport.h"
#include "virtualbike.h"
#include <QFile>
#include <QDateTime>
#include <QMetaEnum>
#include <QBluetoothLocalDevice>

echelonconnectsport::echelonconnectsport(bool noWriteResistance, bool noHeartService)
{
    refresh = new QTimer(this);
    this->noWriteResistance = noWriteResistance;
    this->noHeartService = noHeartService;
    initDone = false;
    connect(refresh, SIGNAL(timeout()), this, SLOT(update()));
    refresh->start(200);
}

void echelonconnectsport::writeCharacteristic(uint8_t* data, uint8_t data_len, QString info, bool disable_log, bool wait_for_response)
{
    QEventLoop loop;
    if(wait_for_response)
    {
        connect(gattCommunicationChannelService, SIGNAL(characteristicChanged(QLowEnergyCharacteristic,QByteArray)),
                &loop, SLOT(quit()));
    }
    else
    {
        connect(gattCommunicationChannelService, SIGNAL(characteristicWritten(QLowEnergyCharacteristic,QByteArray)),
                &loop, SLOT(quit()));
    }

    gattCommunicationChannelService->writeCharacteristic(gattWriteCharacteristic, QByteArray::fromRawData((const char*)data, data_len));

    if(!disable_log)
        debug(" >> " + QByteArray((const char*)data, data_len).toHex(' ') + " // " + info);

    loop.exec();
}

void echelonconnectsport::sendPoll()
{
    static uint8_t counter = 1;
    uint8_t noOpData[] = { 0xf0, 0xa0, 0x01, 0x00, 0x00 };

    noOpData[3] = counter;

    for(uint8_t i=0; i<sizeof(noOpData)-1; i++)
    {
       noOpData[4] += noOpData[i]; // the last byte is a sort of a checksum
    }

    writeCharacteristic(noOpData, sizeof(noOpData), "noOp", true);

    counter++;
    if(!counter)
        counter = 1;
}

void echelonconnectsport::update()
{
    static QDateTime lastTime;
    static bool first = true;

    static uint8_t sec1 = 0;

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
        if(currentSpeed() > 0.0 && !first)
           elapsed += (((double)lastTime.msecsTo(current)) / ((double)1000.0));
        lastTime = current;

        // updating the treadmill console every second
        if(sec1++ == (500 / refresh->interval()))
        {
            sec1 = 0;
            //updateDisplay(elapsed);
        }

        sendPoll();

        if(requestResistance != -1)
        {
           if(requestResistance > 15) requestResistance = 15;
           else if(requestResistance == 0) requestResistance = 1;

           if(requestResistance != currentResistance())
           {
              debug("writing resistance " + QString::number(requestResistance));
              //forceResistance(requestResistance);
           }
           requestResistance = -1;
        }
        if(requestStart != -1)
        {
           debug("starting...");

           btinit();

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

    first = false;
}

void echelonconnectsport::serviceDiscovered(const QBluetoothUuid &gatt)
{
    debug("serviceDiscovered " + gatt.toString());
}

static QByteArray lastPacket;
void echelonconnectsport::characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue)
{
    //qDebug() << "characteristicChanged" << characteristic.uuid() << newValue << newValue.length();
    Q_UNUSED(characteristic);
    static QDateTime lastRefresh = QDateTime::currentDateTime();

    debug(" << " + newValue.toHex(' '));

    if (lastPacket.length() && lastPacket == newValue)
        return;

    lastPacket = newValue;

    // resistance value is in another frame
    if(newValue.length() == 5 && newValue.at(0) == 0xf0 && newValue.at(1) == 0xd2)
    {
        Resistance = newValue.at(3);
        debug("Current resistance: " + QString::number(Resistance));
        return;
    }

    if (newValue.length() != 13)
        return;

    /*if ((uint8_t)(newValue.at(0)) != 0xf0 && (uint8_t)(newValue.at(1)) != 0xd1)
        return;*/

    double distance = GetDistanceFromPacket(newValue);

    Cadence = newValue.at(10);

    CrankRevs += ((double)(lastRefresh.msecsTo(QDateTime::currentDateTime())) * ((double)Cadence / 60000.0) );
    LastCrankEventTime += (uint16_t)((lastRefresh.msecsTo(QDateTime::currentDateTime())) * 1.024);
    lastRefresh = QDateTime::currentDateTime();

    debug("Current Local elapsed: " + GetElapsedFromPacket(newValue).toString());
    debug("Current cadence: " + QString::number(Cadence));
    debug("Current Distance: " + QString::number(distance));
    debug("Current CrankRevs: " + QString::number(CrankRevs));
    debug("Last CrankEventTime: " + QString::number(LastCrankEventTime));
    debug("Current Watt: " + QString::number(watts()));

    if(m_control->error() != QLowEnergyController::NoError)
        qDebug() << "QLowEnergyController ERROR!!" << m_control->errorString();

    Speed = 0;
    KCal = 0;
    Distance = distance;
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
    debug("BTLE stateChanged " + QString::fromLocal8Bit(metaEnum.valueToKey(state)));

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
        static uint8_t first = 0;
        if(!first)
        {
           debug("creating virtual bike interface...");
           virtualBike = new virtualbike(this, noWriteResistance, noHeartService);
           connect(virtualBike,&virtualbike::debug ,this,&echelonconnectsport::debug);
        }
        first = 1;
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
    debug("descriptorWritten " + descriptor.name() + " " + newValue.toHex(' '));

    initRequest = true;
}

void echelonconnectsport::characteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue)
{
    Q_UNUSED(characteristic);
    debug("characteristicWritten " + newValue.toHex(' '));
}

void echelonconnectsport::serviceScanDone(void)
{
    debug("serviceScanDone");

    QBluetoothUuid _gattCommunicationChannelServiceId((QString)"0bf669f1-45f2-11e7-9598-0800200c9a66");

    gattCommunicationChannelService = m_control->createServiceObject(_gattCommunicationChannelServiceId);
    connect(gattCommunicationChannelService, SIGNAL(stateChanged(QLowEnergyService::ServiceState)), this, SLOT(stateChanged(QLowEnergyService::ServiceState)));
    gattCommunicationChannelService->discoverDetails();
}

void echelonconnectsport::errorService(QLowEnergyService::ServiceError err)
{
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    debug("echelonconnectsport::errorService" + QString::fromLocal8Bit(metaEnum.valueToKey(err)) + m_control->errorString());
}

void echelonconnectsport::error(QLowEnergyController::Error err)
{
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    debug("echelonconnectsport::error" + QString::fromLocal8Bit(metaEnum.valueToKey(err)) + m_control->errorString());

    m_control->disconnect();
}

void echelonconnectsport::deviceDiscovered(const QBluetoothDeviceInfo &device)
{
    debug("Found new device: " + device.name() + " (" + device.address().toString() + ')');
    if(device.name().startsWith("ECH-SPORT"))
    {
        bluetoothDevice = device;

        m_control = QLowEnergyController::createCentral(bluetoothDevice, this);
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
    const uint8_t max_resistance = 33;
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

    if(currentCadence() == 0) return 0;

    if(currentCadence() < 41)
        return((((watt_cad40_max-watt_cad40_min) / (max_resistance - 1)) * (currentResistance() - 1))+watt_cad40_min);
    else if(currentCadence() < 46)
        return((((watt_cad45_max-watt_cad45_min) / (max_resistance - 1)) * (currentResistance() - 1))+watt_cad45_min);
    else if(currentCadence() < 51)
        return((((watt_cad50_max-watt_cad50_min) / (max_resistance - 1)) * (currentResistance() - 1))+watt_cad50_min);
    else if(currentCadence() < 56)
        return((((watt_cad55_max-watt_cad55_min) / (max_resistance - 1)) * (currentResistance() - 1))+watt_cad55_min);
    else if(currentCadence() < 61)
        return((((watt_cad60_max-watt_cad60_min) / (max_resistance - 1)) * (currentResistance() - 1))+watt_cad60_min);
    else if(currentCadence() < 66)
        return((((watt_cad65_max-watt_cad65_min) / (max_resistance - 1)) * (currentResistance() - 1))+watt_cad65_min);
    else if(currentCadence() < 71)
        return((((watt_cad70_max-watt_cad70_min) / (max_resistance - 1)) * (currentResistance() - 1))+watt_cad70_min);
    else if(currentCadence() < 76)
        return((((watt_cad75_max-watt_cad75_min) / (max_resistance - 1)) * (currentResistance() - 1))+watt_cad75_min);
    else if(currentCadence() < 81)
        return((((watt_cad80_max-watt_cad80_min) / (max_resistance - 1)) * (currentResistance() - 1))+watt_cad80_min);
    else if(currentCadence() < 86)
        return((((watt_cad85_max-watt_cad85_min) / (max_resistance - 1)) * (currentResistance() - 1))+watt_cad85_min);
    else if(currentCadence() < 91)
        return((((watt_cad90_max-watt_cad90_min) / (max_resistance - 1)) * (currentResistance() - 1))+watt_cad90_min);
    else if(currentCadence() < 96)
        return((((watt_cad95_max-watt_cad95_min) / (max_resistance - 1)) * (currentResistance() - 1))+watt_cad95_min);
    else if(currentCadence() < 101)
        return((((watt_cad100_max-watt_cad100_min) / (max_resistance - 1)) * (currentResistance() - 1))+watt_cad100_min);
    else if(currentCadence() < 106)
        return((((watt_cad105_max-watt_cad105_min) / (max_resistance - 1)) * (currentResistance() - 1))+watt_cad105_min);
    else if(currentCadence() < 111)
        return((((watt_cad110_max-watt_cad110_min) / (max_resistance - 1)) * (currentResistance() - 1))+watt_cad110_min);
    else if(currentCadence() < 116)
        return((((watt_cad115_max-watt_cad115_min) / (max_resistance - 1)) * (currentResistance() - 1))+watt_cad115_min);
    else if(currentCadence() < 121)
        return((((watt_cad120_max-watt_cad120_min) / (max_resistance - 1)) * (currentResistance() - 1))+watt_cad120_min);
    else if(currentCadence() < 126)
        return((((watt_cad125_max-watt_cad125_min) / (max_resistance - 1)) * (currentResistance() - 1))+watt_cad125_min);
    else
        return((((watt_cad130_max-watt_cad130_min) / (max_resistance - 1)) * (currentResistance() - 1))+watt_cad130_min);
    return 0;
}
