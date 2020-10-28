#include "trxappgateusbtreadmill.h"
#include "virtualtreadmill.h"
#include <QFile>
#include <QDateTime>
#include <QMetaEnum>
#include <QBluetoothLocalDevice>

trxappgateusbtreadmill::trxappgateusbtreadmill()
{
    refresh = new QTimer(this);
    initDone = false;
    connect(refresh, SIGNAL(timeout()), this, SLOT(update()));
    refresh->start(500);
}

void trxappgateusbtreadmill::writeCharacteristic(uint8_t* data, uint8_t data_len, QString info, bool disable_log)
{
    QEventLoop loop;
    connect(gattCommunicationChannelService, SIGNAL(characteristicWritten(QLowEnergyCharacteristic,QByteArray)),
            &loop, SLOT(quit()));

    gattCommunicationChannelService->writeCharacteristic(gattWriteCharacteristic, QByteArray::fromRawData((const char*)data, data_len));

    if(!disable_log)
        debug(" >> " + QByteArray((const char*)data, data_len).toHex(' ') + " // " + info);

    loop.exec();
}

void trxappgateusbtreadmill::forceSpeedOrIncline(double requestSpeed, double requestIncline)
{
    Q_UNUSED(requestSpeed);
    Q_UNUSED(requestIncline);
}

bool trxappgateusbtreadmill::changeFanSpeed(uint8_t speed)
{
   Q_UNUSED(speed);
   return false;
}


void trxappgateusbtreadmill::update()
{
    static uint8_t sec1 = 0;
    static QTime lastTime;
    static bool first = true;
    //qDebug() << treadmill.isValid() << m_control->state() << gattCommunicationChannelService << gattWriteCharacteristic.isValid() << gattNotifyCharacteristic.isValid() << initDone;

    if(initRequest)
    {
        initRequest = false;
        btinit(false);
    }
    else if(bttreadmill.isValid() &&
       m_control->state() == QLowEnergyController::DiscoveredState &&
       gattCommunicationChannelService &&
       gattWriteCharacteristic.isValid() &&
       gattNotifyCharacteristic.isValid() &&
       initDone)
    {
        if(currentSpeed() > 0.0 && !first)
           elapsed += ((double)lastTime.msecsTo(QTime::currentTime()) / 1000.0);

        // updating the treadmill console every second
        if(sec1++ == (1000 / refresh->interval()))
        {
            sec1 = 0;
            //updateDisplay(elapsed);
        }

        const uint8_t noOpData[] = { 0xf0, 0xa2, 0x01, 0xd3, 0x66 };
        writeCharacteristic((uint8_t*)noOpData, sizeof(noOpData), "noOp", true);

        if(requestSpeed != -1)
        {
           if(requestSpeed != currentSpeed())
           {
              debug("writing speed " + QString::number(requestSpeed));
              double inc = Inclination;
              if(requestInclination != -1)
              {
                  inc = requestInclination;
                  requestInclination = -1;
              }
              forceSpeedOrIncline(requestSpeed, inc);
           }
           requestSpeed = -1;
        }
        if(requestInclination != -1)
        {
           if(requestInclination != currentInclination())
           {
              debug("writing incline " + QString::number(requestInclination));
              double speed = currentSpeed();
              if(requestSpeed != -1)
              {
                  speed = requestSpeed;
                  requestSpeed = -1;
              }
              forceSpeedOrIncline(speed, requestInclination);
           }
           requestInclination = -1;
        }
        if(requestStart != -1)
        {
           debug("starting...");
           btinit(true);
           requestStart = -1;
           emit tapeStarted();
        }
        if(requestStop != -1)
        {
            debug("stopping...");
            //writeCharacteristic(initDataF0C800B8, sizeof(initDataF0C800B8), "stop tape");
            requestStop = -1;
        }
        if(requestIncreaseFan != -1)
        {
            debug("increasing fan speed...");
            changeFanSpeed(FanSpeed + 1);
            requestIncreaseFan = -1;
        }
        else if(requestDecreaseFan != -1)
        {
            debug("decreasing fan speed...");
            changeFanSpeed(FanSpeed - 1);
            requestDecreaseFan = -1;
        }

        elevationAcc += (currentSpeed() / 3600.0) * 1000 * (currentInclination() / 100) * (refresh->interval() / 1000);
    }

    lastTime = QTime::currentTime();
    first = false;
}

void trxappgateusbtreadmill::serviceDiscovered(const QBluetoothUuid &gatt)
{
    debug("serviceDiscovered " + gatt.toString());
}

static QByteArray lastPacket;
void trxappgateusbtreadmill::characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue)
{
    //qDebug() << "characteristicChanged" << characteristic.uuid() << newValue << newValue.length();
    Q_UNUSED(characteristic);
    static QTime lastTime;
    static bool first = true;

    debug(" << " + newValue.toHex(' '));

    if (lastPacket.length() && lastPacket == newValue)
        return;

    lastPacket = newValue;
    if (newValue.length() != 19)
        return;

    double speed = GetSpeedFromPacket(newValue);
    double incline = GetInclinationFromPacket(newValue);
    double kcal = GetKcalFromPacket(newValue);
    double distance = GetDistanceFromPacket(newValue);

    Heart = 0;
    FanSpeed = 0;

    if(!first)
        DistanceCalculated += ((speed / 3600.0) / ( 1000.0 / (lastTime.msecsTo(QTime::currentTime()))));

    debug("Current speed: " + QString::number(speed));
    debug("Current incline: " + QString::number(incline));
    debug("Current heart: " + QString::number(Heart));
    debug("Current KCal: " + QString::number(kcal));
    debug("Current Distance: " + QString::number(distance));
    debug("Current Distance Calculated: " + QString::number(DistanceCalculated));

    if(m_control->error() != QLowEnergyController::NoError)
        qDebug() << "QLowEnergyController ERROR!!" << m_control->errorString();

    Speed = speed;
    Inclination = incline;
    KCal = kcal;
    Distance = distance;

    lastTime = QTime::currentTime();
    first = false;
}

double trxappgateusbtreadmill::GetSpeedFromPacket(QByteArray packet)
{
    uint16_t convertedData = (packet.at(15) << 8) | packet.at(16);
    double data = (double)(convertedData - 1) / 10.0f;
    return data;
}

double trxappgateusbtreadmill::GetKcalFromPacket(QByteArray packet)
{
    uint16_t convertedData = (packet.at(8) << 8) | packet.at(9);
    return (double)(convertedData - 1);
}

double trxappgateusbtreadmill::GetDistanceFromPacket(QByteArray packet)
{
    uint16_t convertedData = (packet.at(6) << 8) | packet.at(7);
    double data = ((double)(convertedData - 1)) / 10.0f;
    return data;
}

double trxappgateusbtreadmill::GetInclinationFromPacket(QByteArray packet)
{
    uint16_t convertedData = packet.at(13);
    double data = convertedData - 1;
    if (data < 0) return 0;
    return data;
}

void trxappgateusbtreadmill::btinit(bool startTape)
{
    Q_UNUSED(startTape);

    const uint8_t initData1[] = { 0xf0, 0xa0, 0x01, 0x01, 0x92 };
    const uint8_t initData2[] = { 0xf0, 0xa5, 0x01, 0xd3, 0x04, 0x6d };
    const uint8_t initData3[] = { 0xf0, 0xa0, 0x01, 0xd3, 0x64 };
    const uint8_t initData4[] = { 0xf0, 0xa1, 0x01, 0xd3, 0x65 };
    const uint8_t initData5[] = { 0xf0, 0xa3, 0x01, 0xd3, 0x01, 0x15, 0x01, 0x02, 0x51, 0x01, 0x51, 0x23 };
    const uint8_t initData6[] = { 0xf0, 0xa4, 0x01, 0xd3, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x73 };
    const uint8_t initData7[] = { 0xf0, 0xaf, 0x01, 0xd3, 0x02, 0x75 };

    writeCharacteristic((uint8_t*)initData1, sizeof(initData1), "init");
    writeCharacteristic((uint8_t*)initData2, sizeof(initData2), "init");
    writeCharacteristic((uint8_t*)initData3, sizeof(initData3), "init");
    writeCharacteristic((uint8_t*)initData4, sizeof(initData4), "init");
    writeCharacteristic((uint8_t*)initData3, sizeof(initData3), "init");
    writeCharacteristic((uint8_t*)initData5, sizeof(initData5), "init");
    writeCharacteristic((uint8_t*)initData6, sizeof(initData6), "init");
    writeCharacteristic((uint8_t*)initData7, sizeof(initData7), "init");

    initDone = true;
}

void trxappgateusbtreadmill::stateChanged(QLowEnergyService::ServiceState state)
{
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceState>();
    debug("BTLE stateChanged " + QString::fromLocal8Bit(metaEnum.valueToKey(state)));

    if(state == QLowEnergyService::ServiceDiscovered)
    {
        foreach(QLowEnergyCharacteristic c,gattCommunicationChannelService->characteristics())
        {
            debug("characteristic " + c.uuid().toString());
        }

        QBluetoothUuid _gattWriteCharacteristicId((QString)"0000fff2-0000-1000-8000-00805f9b34fb");
        QBluetoothUuid _gattNotifyCharacteristicId((QString)"0000fff1-0000-1000-8000-00805f9b34fb");

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
           v = new virtualtreadmill(this);
           connect(v,&virtualtreadmill::debug ,this,&trxappgateusbtreadmill::debug);
        }
        first = 1;
        // ********************************************************************************************************

        QByteArray descriptor;
        descriptor.append((char)0x01);
        descriptor.append((char)0x00);
        gattCommunicationChannelService->writeDescriptor(gattNotifyCharacteristic.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration), descriptor);
    }
}

void trxappgateusbtreadmill::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue)
{
    debug("descriptorWritten " + descriptor.name() + " " + newValue.toHex(' '));

    initRequest = true;
}

void trxappgateusbtreadmill::characteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue)
{
    Q_UNUSED(characteristic);
    debug("characteristicWritten " + newValue.toHex(' '));
}

void trxappgateusbtreadmill::serviceScanDone(void)
{
    debug("serviceScanDone");

    QBluetoothUuid _gattCommunicationChannelServiceId((QString)"0000fff0-0000-1000-8000-00805f9b34fb");
    gattCommunicationChannelService = m_control->createServiceObject(_gattCommunicationChannelServiceId);
    connect(gattCommunicationChannelService, SIGNAL(stateChanged(QLowEnergyService::ServiceState)), this, SLOT(stateChanged(QLowEnergyService::ServiceState)));
    gattCommunicationChannelService->discoverDetails();
}

void trxappgateusbtreadmill::errorService(QLowEnergyService::ServiceError err)
{
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    debug("trxappgateusbtreadmill::errorService" + QString::fromLocal8Bit(metaEnum.valueToKey(err)) + m_control->errorString());
}

void trxappgateusbtreadmill::error(QLowEnergyController::Error err)
{
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    debug("trxappgateusbtreadmill::error" + QString::fromLocal8Bit(metaEnum.valueToKey(err)) + m_control->errorString());
}

void trxappgateusbtreadmill::deviceDiscovered(const QBluetoothDeviceInfo &device)
{
    debug("Found new device: " + device.name() + " (" + device.address().toString() + ')');
    if(device.name().startsWith("TOORX"))
    {
        bttreadmill = device;
        m_control = QLowEnergyController::createCentral(bttreadmill, this);
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

bool trxappgateusbtreadmill::connected()
{
    if(!m_control)
        return false;
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

void* trxappgateusbtreadmill::VirtualTreadMill()
{
    return virtualTreadMill;
}

double trxappgateusbtreadmill::odometer()
{
    return DistanceCalculated;
}
