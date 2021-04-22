#include "trxappgateusbbike.h"
#include "virtualbike.h"
#include <QFile>
#include <QThread>
#include <QDateTime>
#include <QMetaEnum>
#include <QEventLoop>
#include <QBluetoothLocalDevice>
#include <QSettings>
#include "keepawakehelper.h"

trxappgateusbbike::trxappgateusbbike(bool noWriteResistance, bool noHeartService)
{
    m_watt.setType(metric::METRIC_WATT);
    refresh = new QTimer(this);
    this->noWriteResistance = noWriteResistance;
    this->noHeartService = noHeartService;
    initDone = false;
    connect(refresh, SIGNAL(timeout()), this, SLOT(update()));
    refresh->start(200);
}

void trxappgateusbbike::writeCharacteristic(uint8_t* data, uint8_t data_len, QString info, bool disable_log, bool wait_for_response)
{
    QEventLoop loop;
    QTimer timeout;

    if(wait_for_response)
    {
        connect(this, SIGNAL(packetReceived()),
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

void trxappgateusbbike::forceResistance(int8_t requestResistance)
{
    uint8_t resistance[] = { 0xf0, 0xa6, 0x01, 0x01, 0x00, 0x00 };
    resistance[4] = requestResistance + 1;
    for(uint8_t i=0; i<sizeof(resistance)-1; i++)
    {
       resistance[5] += resistance[i]; // the last byte is a sort of a checksum
    }
    writeCharacteristic((uint8_t*)resistance, sizeof(resistance), "resistance " + QString::number(requestResistance), false, true);
}

void trxappgateusbbike::update()
{
    //qDebug() << bike.isValid() << m_control->state() << gattCommunicationChannelService << gattWriteCharacteristic.isValid() << gattNotifyCharacteristic.isValid() << initDone;

    if(m_control->state() == QLowEnergyController::UnconnectedState)
    {
        emit disconnected();
        return;
    }

    if(initRequest)
    {
        initRequest = false;
        btinit(false);
    }
    else if(bluetoothDevice.isValid() &&
       m_control->state() == QLowEnergyController::DiscoveredState &&
       gattCommunicationChannelService &&
       gattWriteCharacteristic.isValid() &&
       gattNotify1Characteristic.isValid() &&
       //gattNotify2Characteristic.isValid() &&
       initDone)
    {
        if(currentSpeed().value() > 0.0 && !firstUpdate)
           elapsed += ((double)lastTimeUpdate.msecsTo(QTime::currentTime()) / 1000.0);

        lastTimeUpdate = QTime::currentTime();

        // updating the bike console every second
        if(sec1update++ == (1000 / refresh->interval()))
        {
            sec1update = 0;
            //updateDisplay(elapsed);
        }

        QSettings settings;
        bool toorx30 = settings.value("toorx_3_0", false).toBool();
        if(toorx30 == false && (bike_type == TYPE::IRUNNING || bike_type == TYPE::ICONSOLE))
        {
            const uint8_t noOpData[] = { 0xf0, 0xa2, 0x01, 0x01, 0x94 };
            writeCharacteristic((uint8_t*)noOpData, sizeof(noOpData), "noOp", false, true);
        }
        else if(bike_type == TYPE::DKN_MOTION)
        {
            const uint8_t noOpData[] = { 0xf0, 0xa2, 0x02, 0x01, 0x95 };
            writeCharacteristic((uint8_t*)noOpData, sizeof(noOpData), "noOp", false, true);
        }
        else if(bike_type == TYPE::CHANGYOW)
        {
            const uint8_t noOpData[] = { 0xf0, 0xa2, 0x23, 0x01, 0xb6 };
            writeCharacteristic((uint8_t*)noOpData, sizeof(noOpData), "noOp", false, true);
        }
        else if(bike_type == TYPE::JLL_IC400)
        {
            static unsigned char pollCounter = 0x0b;
            static unsigned char crc = 0x59;
            uint8_t noOpData[] = { 0xf0, 0xa2, 0x00, 0xc8, 0x00 };
            noOpData[2] = pollCounter;
            crc += 0x0c;
            noOpData[4] = crc;
            pollCounter += 0x0c;
            writeCharacteristic((uint8_t*)noOpData, sizeof(noOpData), "noOp", false, true);
        }
        else
        {
            const uint8_t noOpData[] = { 0xf0, 0xa2, 0x23, 0xd3, 0x88 };
            writeCharacteristic((uint8_t*)noOpData, sizeof(noOpData), "noOp", false, true);
        }

        if(requestResistance != -1)
        {
           if(requestResistance > 32) requestResistance = 32;
           else if(requestResistance < 1) requestResistance = 1;

           if(requestResistance != currentResistance().value())
           {
              debug("writing resistance " + QString::number(requestResistance));
              forceResistance(requestResistance);
           }
           requestResistance = -1;
        }
    }

    firstUpdate = false;
}

void trxappgateusbbike::serviceDiscovered(const QBluetoothUuid &gatt)
{
    debug("serviceDiscovered " + gatt.toString());
}

void trxappgateusbbike::characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue)
{
    //qDebug() << "characteristicChanged" << characteristic.uuid() << newValue << newValue.length();
    Q_UNUSED(characteristic);
    QSettings settings;
    QString heartRateBeltName = settings.value("heart_rate_belt_name", "Disabled").toString();
    emit packetReceived();

    debug(" << " + newValue.toHex(' '));

    lastPacket = newValue;
    if ((newValue.length() != 21 && bike_type != JLL_IC400) || (newValue.length() != 19 && bike_type == JLL_IC400))
        return;

    /*
    if(bike_type == TYPE::IRUNNING)
    {
        if(newValue.at(15) == 0x03 && newValue.at(16) == 0x02 && readyToStart == false)
        {
            readyToStart = true;
            requestStart = 1;
        }
    }
    else
    {
        if(newValue.at(16) == 0x04 && newValue.at(17) == 0x03 && readyToStart == false)
        {
            readyToStart = true;
            requestStart = 1;
        }
    }
*/

    double cadence = GetCadenceFromPacket(newValue);
    double speed = 0;
    double resistance = 0;
    double kcal = 0;
    double watt = 0;
    if(bike_type != JLL_IC400)
    {
        speed = GetSpeedFromPacket(newValue);
        resistance = GetResistanceFromPacket(newValue);
        kcal = GetKcalFromPacket(newValue);
        watt = GetWattFromPacket(newValue);
    }
    else
    {
        speed = cadence * 0.37407407407407407407407407407407;
        if(Heart.value() > 0)
        {
            int avgP = ((settings.value("power_hr_pwr1", 200).toDouble() * settings.value("power_hr_hr2",170).toDouble()) - (settings.value("power_hr_pwr2",230).toDouble() * settings.value("power_hr_hr1",150).toDouble())) / (settings.value("power_hr_hr2",170).toDouble() - settings.value("power_hr_hr1",150).toDouble()) + (Heart.value() * ((settings.value("power_hr_pwr1",200).toDouble() - settings.value("power_hr_pwr2",230).toDouble()) / (settings.value("power_hr_hr1",150).toDouble() - settings.value("power_hr_hr2",170).toDouble())));
            if(Speed.value() > 0)
                watt = avgP;
            else
                watt = 0;

            kcal = KCal.value() + ((( (0.048 * ((double)watts()) + 1.19) * settings.value("weight", 75.0).toFloat() * 3.5) / 200.0 ) / (60000.0 / ((double)lastTimeCharChanged.msecsTo(QTime::currentTime())))); //(( (0.048* Output in watts +1.19) * body weight in kg * 3.5) / 200 ) / 60
        }
    }

#ifdef Q_OS_ANDROID
    if(settings.value("ant_heart", false).toBool())
        Heart = (uint8_t)KeepAwakeHelper::heart();
    else
#endif
    {
        if(heartRateBeltName.startsWith("Disabled"))
        {
            if(bike_type != JLL_IC400)
                Heart = ((uint8_t)(newValue.at(15)) - 1);
            else
                Heart = ((uint8_t)(newValue.at(17))) + (((uint8_t)(newValue.at(16))) * 83);
        }
    }
    FanSpeed = 0;

    if(!firstCharChanged)
        DistanceCalculated += ((speed / 3600.0) / ( 1000.0 / (lastTimeCharChanged.msecsTo(QTime::currentTime()))));

    debug("Current speed: " + QString::number(speed));
    debug("Current cadence: " + QString::number(cadence));
    debug("Current heart: " + QString::number(Heart.value()));
    debug("Current KCal: " + QString::number(kcal));
    debug("Current watt: " + QString::number(watt));
    debug("Current Elapsed from the bike (not used): " + QString::number(GetElapsedFromPacket(newValue)));
    debug("Current Elapsed: " + QString::number(elapsed.value()));
    debug("Current Distance Calculated: " + QString::number(DistanceCalculated));

    if(m_control->error() != QLowEnergyController::NoError)
        qDebug() << "QLowEnergyController ERROR!!" << m_control->errorString();

    Speed = speed;
    KCal = kcal;
    Distance = DistanceCalculated;
    Cadence = cadence;
    m_watt = watt;

    double ac=0.01243107769;
    double bc=1.145964912;
    double cc=-23.50977444;

    double ar=0.1469553975;
    double br=-5.841344538;
    double cr=97.62165482;

    m_pelotonResistance = (((sqrt(pow(br,2.0)-4.0*ar*(cr-(m_watt.value()*132.0/(ac*pow(Cadence.value(),2.0)+bc*Cadence.value()+cc))))-br)/(2.0*ar)) * settings.value("peloton_gain", 1.0).toDouble()) + settings.value("peloton_offset", 0.0).toDouble();
    if(bike_type == JLL_IC400)
        resistance = m_pelotonResistance.value();

    Resistance = resistance;

    debug("Current resistance: " + QString::number(resistance));

    lastTimeCharChanged = QTime::currentTime();
    firstCharChanged = false;
}

uint16_t trxappgateusbbike::GetElapsedFromPacket(QByteArray packet)
{
    uint16_t convertedData = (packet.at(4) - 1);
    convertedData += ((packet.at(5) - 1) * 60);
    return convertedData;
}

double trxappgateusbbike::GetSpeedFromPacket(QByteArray packet)
{
    uint16_t convertedData = (packet.at(7) - 1) + ((packet.at(6) - 1) * 100);
    double data = (double)(convertedData) / 10.0f;
    return data;
}

double trxappgateusbbike::GetKcalFromPacket(QByteArray packet)
{
    uint16_t convertedData = ((packet.at(12) - 1) * 100) + (packet.at(13) - 1);
    return (double)(convertedData);
}

double trxappgateusbbike::GetWattFromPacket(QByteArray packet)
{
    uint16_t convertedData = ((packet.at(16) - 1) * 100) + (packet.at(17) - 1);
    double data = ((double)(convertedData)) / 10.0f;
    return data;
}

double trxappgateusbbike::GetCadenceFromPacket(QByteArray packet)
{
    uint16_t convertedData;
    if(bike_type != JLL_IC400)
        convertedData = (packet.at(9) - 1) + ((packet.at(8) - 1) * 100);
    else
        convertedData = ((uint16_t)packet.at(9)) + ((uint16_t)packet.at(8) * 100);
    double data = (convertedData);
    if (data < 0) return 0;
    return data;
}

double trxappgateusbbike::GetResistanceFromPacket(QByteArray packet)
{
    uint16_t convertedData = packet.at(18);
    double data = (convertedData - 1);
    if (data < 0) return 0;
    return data;
}

void trxappgateusbbike::btinit(bool startTape)
{
    Q_UNUSED(startTape);
    QSettings settings;
    bool toorx30 = settings.value("toorx_3_0", false).toBool();

    if(toorx30 == false && (bike_type == TYPE::IRUNNING || bike_type == TYPE::ICONSOLE))
    {
        const uint8_t initData1[] = { 0xf0, 0xa0, 0x01, 0x01, 0x92 };
        const uint8_t initData2[] = { 0xf0, 0xa1, 0x01, 0x01, 0x93 };
        const uint8_t initData3[] = { 0xf0, 0xa3, 0x01, 0x01, 0x01, 0x96 };
        const uint8_t initData4[] = { 0xf0, 0xa5, 0x01, 0x01, 0x02, 0x99 };
        const uint8_t initData5[] = { 0xf0, 0xa6, 0x01, 0x01, 0x06, 0x9e };

        writeCharacteristic((uint8_t*)initData1, sizeof(initData1), "init", false, true);
        if(bike_type == TYPE::IRUNNING) QThread::msleep(400);
        writeCharacteristic((uint8_t*)initData2, sizeof(initData2), "init", false, true);
        if(bike_type == TYPE::IRUNNING) QThread::msleep(400);
        writeCharacteristic((uint8_t*)initData3, sizeof(initData3), "init", false, true);
        if(bike_type == TYPE::IRUNNING) QThread::msleep(400);
        writeCharacteristic((uint8_t*)initData4, sizeof(initData4), "init", false, true);
        if(bike_type == TYPE::IRUNNING) QThread::msleep(400);
        writeCharacteristic((uint8_t*)initData5, sizeof(initData5), "init", false, true);
        if(bike_type == TYPE::IRUNNING) QThread::msleep(400);
    }
    else if(bike_type == TYPE::DKN_MOTION)
    {
        const uint8_t initData1[] = { 0xf0, 0xa0, 0x01, 0x01, 0x92 };
        const uint8_t initData2[] = { 0xf0, 0xa1, 0x01, 0x01, 0x93 };
        const uint8_t initData3[] = { 0xf0, 0xa3, 0x02, 0x01, 0x01, 0x97 };
        const uint8_t initData4[] = { 0xf0, 0xa5, 0x02, 0x01, 0x02, 0x9a };
        const uint8_t initData5[] = { 0x40, 0x00, 0x9a, 0x46, 0x20 };

        writeCharacteristic((uint8_t*)initData1, sizeof(initData1), "init", false, true);
        writeCharacteristic((uint8_t*)initData2, sizeof(initData2), "init", false, true);
        writeCharacteristic((uint8_t*)initData3, sizeof(initData3), "init", false, true);
        writeCharacteristic((uint8_t*)initData4, sizeof(initData4), "init", false, true);
        writeCharacteristic((uint8_t*)initData4, sizeof(initData4), "init", false, true);
        writeCharacteristic((uint8_t*)initData5, sizeof(initData5), "init", false, true);
    }
    else if(bike_type == TYPE::CHANGYOW)
    {
        const uint8_t initData1[] = { 0xf0, 0xa0, 0x01, 0x01, 0x92 };
        const uint8_t initData2[] = { 0xf0, 0xa0, 0x23, 0x01, 0xb4 };
        const uint8_t initData3[] = { 0xf0, 0xa1, 0x23, 0x01, 0xb5 };
        const uint8_t initData4[] = { 0xf0, 0xa3, 0x23, 0x01, 0x01, 0xb8 };
        const uint8_t initData5[] = { 0xf0, 0xa5, 0x23, 0x01, 0x02, 0xbb };
        const uint8_t initData6[] = { 0xf0, 0xa6, 0x23, 0x01, 0x06, 0xc0 };

        writeCharacteristic((uint8_t*)initData1, sizeof(initData1), "init", false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t*)initData2, sizeof(initData2), "init", false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t*)initData3, sizeof(initData3), "init", false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t*)initData4, sizeof(initData4), "init", false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t*)initData5, sizeof(initData5), "init", false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t*)initData6, sizeof(initData6), "init", false, true);
        QThread::msleep(400);
    }
    else if(bike_type == TYPE::JLL_IC400)
    {
        const uint8_t initData1[] = { 0xf0, 0xa0, 0x01, 0x01, 0x92 };
        const uint8_t initData2[] = { 0xf0, 0xa0, 0x03, 0xc9, 0x5c };
        const uint8_t initData3[] = { 0xf0, 0xa1, 0x00, 0xc8, 0x59 };
        const uint8_t initData4[] = { 0xf0, 0xa0, 0x01, 0xc9, 0x5a };
        const uint8_t initData5[] = { 0xf0, 0xa1, 0x05, 0xc8, 0x5e };
        const uint8_t initData6[] = { 0xf0, 0xa2, 0x22, 0xc8, 0x7c };
        const uint8_t initData7[] = { 0xf0, 0xa0, 0x39, 0xc9, 0x92 };

        writeCharacteristic((uint8_t*)initData1, sizeof(initData1), "init", false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t*)initData2, sizeof(initData2), "init", false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t*)initData3, sizeof(initData3), "init", false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t*)initData4, sizeof(initData4), "init", false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t*)initData3, sizeof(initData3), "init", false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t*)initData4, sizeof(initData4), "init", false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t*)initData3, sizeof(initData3), "init", false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t*)initData4, sizeof(initData4), "init", false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t*)initData3, sizeof(initData3), "init", false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t*)initData4, sizeof(initData4), "init", false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t*)initData5, sizeof(initData5), "init", false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t*)initData6, sizeof(initData6), "init", false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t*)initData7, sizeof(initData7), "init", false, true);
        QThread::msleep(400);
    }
    else
    {
        const uint8_t initData1[] = { 0xf0, 0xa0, 0x01, 0x01, 0x92 };
        const uint8_t initData2[] = { 0xf0, 0xa5, 0x23, 0xd3, 0x04, 0x8f };
        const uint8_t initData3[] = { 0xf0, 0xa0, 0x23, 0xd3, 0x86 };
        const uint8_t initData4[] = { 0xf0, 0xa1, 0x23, 0xd3, 0x87 };
        const uint8_t initData5[] = { 0xf0, 0xa3, 0x23, 0xd3, 0x01, 0x15, 0x01, 0x02, 0x51, 0x01, 0x51, 0x45 };
        const uint8_t initData6[] = { 0xf0, 0xa4, 0x23, 0xd3, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x95 };
        const uint8_t initData7[] = { 0xf0, 0xaf, 0x23, 0xd3, 0x02, 0x97 };

        writeCharacteristic((uint8_t*)initData1, sizeof(initData1), "init", false, true);
        writeCharacteristic((uint8_t*)initData2, sizeof(initData2), "init", false, true);
        writeCharacteristic((uint8_t*)initData3, sizeof(initData3), "init", false, true);
        writeCharacteristic((uint8_t*)initData4, sizeof(initData4), "init", false, true);
        writeCharacteristic((uint8_t*)initData5, sizeof(initData5), "init", false, true);
        writeCharacteristic((uint8_t*)initData6, sizeof(initData6), "init", false, true);
        writeCharacteristic((uint8_t*)initData7, sizeof(initData7), "init", false, true);
    }
    initDone = true;
}

void trxappgateusbbike::stateChanged(QLowEnergyService::ServiceState state)
{
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceState>();
    debug("BTLE stateChanged " + QString::fromLocal8Bit(metaEnum.valueToKey(state)));

    if(state == QLowEnergyService::ServiceDiscovered)
    {
        foreach(QLowEnergyCharacteristic c,gattCommunicationChannelService->characteristics())
        {
            debug("characteristic " + c.uuid().toString());
        }

        QString uuidWrite  = "0000fff2-0000-1000-8000-00805f9b34fb";
        QString uuidNotify1 = "0000fff1-0000-1000-8000-00805f9b34fb";
        QString uuidNotify2 = "49535343-4c8a-39b3-2f49-511cff073b7e";

        if(bike_type == TYPE::IRUNNING || bike_type == TYPE::CHANGYOW || bike_type == TYPE::ICONSOLE || bike_type == TYPE::JLL_IC400)
        {
            uuidWrite      = "49535343-8841-43f4-a8d4-ecbe34729bb3";
            uuidNotify1    = "49535343-1E4D-4BD9-BA61-23C647249616";
            uuidNotify2    = "49535343-4c8a-39b3-2f49-511cff073b7e";
        }

        QBluetoothUuid _gattWriteCharacteristicId((QString)uuidWrite);
        QBluetoothUuid _gattNotify1CharacteristicId((QString)uuidNotify1);
        QBluetoothUuid _gattNotify2CharacteristicId((QString)uuidNotify2);

        gattWriteCharacteristic = gattCommunicationChannelService->characteristic(_gattWriteCharacteristicId);
        gattNotify1Characteristic = gattCommunicationChannelService->characteristic(_gattNotify1CharacteristicId);
        if(bike_type == TYPE::IRUNNING || bike_type == TYPE::CHANGYOW)
            gattNotify2Characteristic = gattCommunicationChannelService->characteristic(_gattNotify2CharacteristicId);

        Q_ASSERT(gattWriteCharacteristic.isValid());
        Q_ASSERT(gattNotify1Characteristic.isValid());
        if(bike_type == TYPE::IRUNNING || bike_type == TYPE::CHANGYOW)
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
        if(!firstVirtualBike && !virtualBike)
        {
            QSettings settings;
            bool virtual_device_enabled = settings.value("virtual_device_enabled", true).toBool();
            if(virtual_device_enabled)
            {
                debug("creating virtual bike interface...");
                virtualBike = new virtualbike(this, noWriteResistance, noHeartService);
                //connect(virtualBike,&virtualbike::debug ,this,&trxappgateusbbike::debug);
            }
        }
        firstVirtualBike = 1;
        // ********************************************************************************************************

        QByteArray descriptor;
        descriptor.append((char)0x01);
        descriptor.append((char)0x00);
        gattCommunicationChannelService->writeDescriptor(gattNotify1Characteristic.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration), descriptor);
        if(bike_type == TYPE::IRUNNING || bike_type == TYPE::CHANGYOW)
            gattCommunicationChannelService->writeDescriptor(gattNotify2Characteristic.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration), descriptor);
    }
}

void trxappgateusbbike::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue)
{
    debug("descriptorWritten " + descriptor.name() + " " + newValue.toHex(' '));

    initRequest = true;
    emit connectedAndDiscovered();
}

void trxappgateusbbike::characteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue)
{
    Q_UNUSED(characteristic);
    debug("characteristicWritten " + newValue.toHex(' '));
}

void trxappgateusbbike::serviceScanDone(void)
{
    debug("serviceScanDone");

    QString uuid = "0000fff0-0000-1000-8000-00805f9b34fb";
    if(bike_type == TYPE::IRUNNING || bike_type == TYPE::CHANGYOW || bike_type == TYPE::ICONSOLE || bike_type == TYPE::JLL_IC400)
            uuid = "49535343-FE7D-4AE5-8FA9-9FAFD205E455";

    QBluetoothUuid _gattCommunicationChannelServiceId((QString)uuid);
    gattCommunicationChannelService = m_control->createServiceObject(_gattCommunicationChannelServiceId);

    if(gattCommunicationChannelService == nullptr)
    {
        qDebug() << "invalid service" << uuid;
        return;
    }

    connect(gattCommunicationChannelService, SIGNAL(stateChanged(QLowEnergyService::ServiceState)), this, SLOT(stateChanged(QLowEnergyService::ServiceState)));
    gattCommunicationChannelService->discoverDetails();
}

void trxappgateusbbike::errorService(QLowEnergyService::ServiceError err)
{
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    debug("trxappgateusbbike::errorService" + QString::fromLocal8Bit(metaEnum.valueToKey(err)) + m_control->errorString());
}

void trxappgateusbbike::error(QLowEnergyController::Error err)
{
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    debug("trxappgateusbbike::error" + QString::fromLocal8Bit(metaEnum.valueToKey(err)) + m_control->errorString());
}

void trxappgateusbbike::deviceDiscovered(const QBluetoothDeviceInfo &device)
{
    QSettings settings;
    bool JLL_IC400_bike = settings.value("jll_IC400_bike", false).toBool();
    debug("Found new device: " + device.name() + " (" + device.address().toString() + ')');
    //if(device.name().startsWith("TOORX") || device.name().startsWith("V-RUN") || device.name().startsWith("FS-") || device.name().startsWith("i-Console+") || device.name().startsWith("i-Running"))
    {
        //if(device.name().startsWith("i-Running") || device.name().startsWith("i-Console+"))
            bike_type = TYPE::IRUNNING;
        /*else
            bike_type = TYPE::TRXAPPGATE;*/

        if(JLL_IC400_bike)
        {
            refresh->start(500);
            bike_type = TYPE::JLL_IC400;
            qDebug() << "JLL_IC400 bike found";
        }
        else if(device.address().toString().toUpper().startsWith("E8"))
        {
            bike_type = TYPE::CHANGYOW;
            qDebug() << "CHANGYOW bike found";
        }
        else if(device.name().toUpper().startsWith("BFCP"))
        {
            bike_type = TYPE::SKANDIKAWIRY;
            qDebug() << "SKANDIKAWIRY bike found";
        }
        else if(device.name().toUpper().startsWith("ICONSOLE+"))
        {
            bike_type = TYPE::ICONSOLE;
            qDebug() << "ICONSOLE bike found";
        }
        else if(device.name().toUpper().startsWith("DKN MOTION"))
        {
            bike_type = TYPE::DKN_MOTION;
            qDebug() << "DKN MOTION bike found";
        }

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

uint16_t trxappgateusbbike::watts()
{
    if(currentCadence().value() == 0) return 0;

    return m_watt.value();
}

bool trxappgateusbbike::connected()
{
    if(!m_control)
        return false;
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

void* trxappgateusbbike::VirtualBike()
{
    return virtualBike;
}

void* trxappgateusbbike::VirtualDevice()
{
    return VirtualBike();
}

double trxappgateusbbike::odometer()
{
    return DistanceCalculated;
}

void trxappgateusbbike::controllerStateChanged(QLowEnergyController::ControllerState state)
{
    qDebug() << "controllerStateChanged" << state;
    if(state == QLowEnergyController::UnconnectedState && m_control)
    {
        qDebug() << "trying to connect back again...";
        initDone = false;
        m_control->connectToDevice();
    }
}
