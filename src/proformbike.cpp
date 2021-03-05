#include "proformbike.h"
#include "virtualbike.h"
#include <QFile>
#include <QDateTime>
#include <QMetaEnum>
#include <QSettings>
#include <QBluetoothLocalDevice>
#include <math.h>
#include <QThread>
#include "ios/lockscreen.h"
#include "keepawakehelper.h"

proformbike::proformbike(bool noWriteResistance, bool noHeartService)
{
    m_watt.setType(metric::METRIC_WATT);
    refresh = new QTimer(this);
    this->noWriteResistance = noWriteResistance;
    this->noHeartService = noHeartService;
    initDone = false;
    connect(refresh, SIGNAL(timeout()), this, SLOT(update()));
    refresh->start(200);
}

void proformbike::writeCharacteristic(uint8_t* data, uint8_t data_len, QString info, bool disable_log, bool wait_for_response)
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

    gattCommunicationChannelService->writeCharacteristic(gattWriteCharacteristic, QByteArray::fromRawData((const char*)data, data_len));

    if(!disable_log)
        debug(" >> " + QByteArray((const char*)data, data_len).toHex(' ') + " // " + info);

    loop.exec();
}

void proformbike::forceResistance(int8_t requestResistance)
{
    const uint8_t res1[] = { 0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01, 0x04, 0x32, 0x02, 0x00, 0x4b, 0x00, 0x00, 0x00, 0x00, 0x00 };
    const uint8_t res2[] = { 0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01, 0x04, 0xa3, 0x04, 0x00, 0xbe, 0x00, 0x00, 0x00, 0x00, 0x00 };
    const uint8_t res3[] = { 0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01, 0x04, 0x14, 0x07, 0x00, 0x32, 0x00, 0x00, 0x00, 0x00, 0x00 };
    const uint8_t res4[] = { 0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01, 0x04, 0x85, 0x09, 0x00, 0xa5, 0x00, 0x00, 0x00, 0x00, 0x00 };
    const uint8_t res5[] = { 0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01, 0x04, 0xf6, 0x0b, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00 };
    const uint8_t res6[] = { 0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01, 0x04, 0x67, 0x0e, 0x00, 0x8c, 0x00, 0x00, 0x00, 0x00, 0x00 };
    const uint8_t res7[] = { 0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01, 0x04, 0xd8, 0x10, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00 };
    const uint8_t res8[] = { 0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01, 0x04, 0x49, 0x13, 0x00, 0x73, 0x00, 0x00, 0x00, 0x00, 0x00 };
    const uint8_t res9[] = { 0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01, 0x04, 0xba, 0x15, 0x00, 0xe6, 0x00, 0x00, 0x00, 0x00, 0x00 };
    const uint8_t res10[] = { 0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01, 0x04, 0x2b, 0x18, 0x00, 0x5a, 0x00, 0x00, 0x00, 0x00, 0x00 };
    const uint8_t res11[] = { 0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01, 0x04, 0x9c, 0x1a, 0x00, 0xcd, 0x00, 0x00, 0x00, 0x00, 0x00 };
    const uint8_t res12[] = { 0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01, 0x04, 0x0d, 0x1d, 0x00, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00 };
    const uint8_t res13[] = { 0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01, 0x04, 0x7e, 0x1f, 0x00, 0xb4, 0x00, 0x00, 0x00, 0x00, 0x00 };
    const uint8_t res14[] = { 0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01, 0x04, 0xef, 0x21, 0x00, 0x27, 0x00, 0x00, 0x00, 0x00, 0x00 };
    const uint8_t res15[] = { 0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01, 0x04, 0x60, 0x24, 0x00, 0x9b, 0x00, 0x00, 0x00, 0x00, 0x00 };
    const uint8_t res16[] = { 0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01, 0x04, 0xd1, 0x26, 0x00, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00 };

    switch (requestResistance) {
    case 1:
        writeCharacteristic((uint8_t*)res1, sizeof(res1), "resistance1", false, true);
        break;
    case 2:
        writeCharacteristic((uint8_t*)res2, sizeof(res2), "resistance2", false, true);
        break;
    case 3:
        writeCharacteristic((uint8_t*)res3, sizeof(res3), "resistance3", false, true);
        break;
    case 4:
        writeCharacteristic((uint8_t*)res4, sizeof(res4), "resistance4", false, true);
        break;
    case 5:
        writeCharacteristic((uint8_t*)res5, sizeof(res5), "resistance5", false, true);
        break;
    case 6:
        writeCharacteristic((uint8_t*)res6, sizeof(res6), "resistance6", false, true);
        break;
    case 7:
        writeCharacteristic((uint8_t*)res7, sizeof(res7), "resistance7", false, true);
        break;
    case 8:
        writeCharacteristic((uint8_t*)res8, sizeof(res8), "resistance8", false, true);
        break;
    case 9:
        writeCharacteristic((uint8_t*)res9, sizeof(res9), "resistance9", false, true);
        break;
    case 10:
        writeCharacteristic((uint8_t*)res10, sizeof(res10), "resistance10", false, true);
        break;
    case 11:
        writeCharacteristic((uint8_t*)res11, sizeof(res11), "resistance11", false, true);
        break;
    case 12:
        writeCharacteristic((uint8_t*)res12, sizeof(res12), "resistance12", false, true);
        break;
    case 13:
        writeCharacteristic((uint8_t*)res13, sizeof(res13), "resistance13", false, true);
        break;
    case 14:
        writeCharacteristic((uint8_t*)res14, sizeof(res14), "resistance14", false, true);
        break;
    case 15:
        writeCharacteristic((uint8_t*)res15, sizeof(res15), "resistance15", false, true);
        break;
    case 16:
        writeCharacteristic((uint8_t*)res16, sizeof(res16), "resistance16", false, true);
        break;
    }
}

void proformbike::update()
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

        uint8_t noOpData1[] = { 0xfe, 0x02, 0x19, 0x03 };
        uint8_t noOpData2[] = { 0x00, 0x12, 0x02, 0x04, 0x02, 0x15, 0x07, 0x15, 0x02, 0x00, 0x0f, 0xbc, 0x90, 0x70, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00 };
        uint8_t noOpData3[] = { 0xff, 0x07, 0x00, 0x00, 0x00, 0x10, 0x00, 0x08, 0x5d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
        uint8_t noOpData4[] = { 0xfe, 0x02, 0x17, 0x03 };
        uint8_t noOpData5[] = { 0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x07, 0x13, 0x02, 0x00, 0x0d, 0x3c, 0x9c, 0x31, 0x00, 0x00, 0x40, 0x40, 0x00, 0x80 };
        uint8_t noOpData6[] = { 0xff, 0x05, 0x00, 0x80, 0x01, 0x00, 0xa9, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
        uint8_t noOpData7[] = { 0xfe, 0x02, 0x0d, 0x02 };

        switch(counterPoll)
        {
        case 0:
            writeCharacteristic(noOpData1, sizeof(noOpData1), "noOp");
            break;
        case 1:
            writeCharacteristic(noOpData2, sizeof(noOpData2), "noOp");
            break;
        case 2:
            writeCharacteristic(noOpData3, sizeof(noOpData3), "noOp");
            break;
        case 3:
            writeCharacteristic(noOpData4, sizeof(noOpData4), "noOp");
            break;
        case 4:
            writeCharacteristic(noOpData5, sizeof(noOpData5), "noOp");
            break;
        case 5:
            writeCharacteristic(noOpData6, sizeof(noOpData6), "noOp");
            break;
        case 6:
            writeCharacteristic(noOpData7, sizeof(noOpData7), "noOp");
            if(requestResistance != -1)
            {
               if(requestResistance > 16) requestResistance = 16;
               else if(requestResistance == 0) requestResistance = 1;

               if(requestResistance != currentResistance().value())
               {
                  debug("writing resistance " + QString::number(requestResistance));
                  forceResistance(requestResistance);
               }
               requestResistance = -1;
            }
            break;
        }

        counterPoll++;
        if(counterPoll > 6)
            counterPoll = 0;

        // updating the treadmill console every second
        if(sec1Update++ == (500 / refresh->interval()))
        {
            sec1Update = 0;
            //updateDisplay(elapsed);
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

    firstUpdate = false;
}

void proformbike::serviceDiscovered(const QBluetoothUuid &gatt)
{
    debug("serviceDiscovered " + gatt.toString());
}

void proformbike::characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue)
{
    //qDebug() << "characteristicChanged" << characteristic.uuid() << newValue << newValue.length();
    Q_UNUSED(characteristic);
    QSettings settings;
    QString heartRateBeltName = settings.value("heart_rate_belt_name", "Disabled").toString();

    debug(" << " + newValue.toHex(' '));

    lastPacket = newValue;

    if (newValue.length() != 20 || newValue.at(0) != 0x00 || newValue.at(1) != 0x12 || newValue.at(2) != 0x01 || newValue.at(3) != 0x04 ||
               (((uint8_t)newValue.at(12)) == 0xFF && ((uint8_t)newValue.at(13)) == 0xFF
             && ((uint8_t)newValue.at(14)) == 0xFF && ((uint8_t)newValue.at(15)) == 0xFF && ((uint8_t)newValue.at(16)) == 0xFF
             && ((uint8_t)newValue.at(17)) == 0xFF && ((uint8_t)newValue.at(18)) == 0xFF && ((uint8_t)newValue.at(19)) == 0xFF))
        return;

    m_watts = ((uint16_t)(((uint8_t)newValue.at(13)) << 8) + (uint16_t)((uint8_t) newValue.at(12)));

    // filter some strange values from proform
    if(m_watts > 3000)
    {
        m_watts = 0;
    }
    else
    {
        switch((uint8_t)newValue.at(11))
        {
        case 0x02:
         Resistance = 1;
         break;
        case 0x04:
         Resistance = 2;
         break;
        case 0x07:
         Resistance = 3;
         break;
        case 0x09:
         Resistance = 4;
         break;
        case 0x0b:
         Resistance = 5;
         break;
        case 0x0e:
         Resistance = 6;
         break;
        case 0x10:
         Resistance = 7;
         break;
        case 0x13:
         Resistance = 8;
         break;
        case 0x15:
         Resistance = 9;
         break;
        case 0x18:
         Resistance = 10;
         break;
        case 0x1a:
         Resistance = 11;
         break;
        case 0x1d:
         Resistance = 12;
         break;
        case 0x1f:
         Resistance = 13;
         break;
        case 0x21:
         Resistance = 14;
         break;
        case 0x24:
         Resistance = 15;
         break;
        case 0x26:
         Resistance = 16;
         break;
        default:
         Resistance = 0;
         break;
        }

        Cadence = ((uint8_t)newValue.at(18));

        Speed = (settings.value("proform_wheel_ratio", 0.33).toDouble()) * ((double)Cadence.value());
        KCal += ((( (0.048 * ((double)watts()) + 1.19) * settings.value("weight", 75.0).toFloat() * 3.5) / 200.0 ) / (60000.0 / ((double)lastRefreshCharacteristicChanged.msecsTo(QDateTime::currentDateTime())))); //(( (0.048* Output in watts +1.19) * body weight in kg * 3.5) / 200 ) / 60
        //KCal = (((uint16_t)((uint8_t)newValue.at(15)) << 8) + (uint16_t)((uint8_t) newValue.at(14)));
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
}

void proformbike::btinit()
{
    uint8_t initData1[] = { 0xfe, 0x02, 0x08, 0x02 };
    uint8_t initData2[] = { 0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x02, 0x04, 0x81, 0x87, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    uint8_t initData3[] = { 0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x07, 0x04, 0x80, 0x8b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    uint8_t initData4[] = { 0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x07, 0x04, 0x88, 0x93, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    uint8_t initData5[] = { 0xfe, 0x02, 0x0a, 0x02 };
    uint8_t initData6[] = { 0xff, 0x0a, 0x02, 0x04, 0x02, 0x06, 0x02, 0x06, 0x82, 0x00, 0x00, 0x8a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    uint8_t initData7[] = { 0xff, 0x0a, 0x02, 0x04, 0x02, 0x06, 0x02, 0x06, 0x84, 0x00, 0x00, 0x8c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    uint8_t initData8[] = { 0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x02, 0x04, 0x95, 0x9b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    uint8_t initData9[] = { 0xfe, 0x02, 0x2c, 0x04 };
    uint8_t initData10[] = { 0x00, 0x12, 0x02, 0x04, 0x02, 0x28, 0x07, 0x28, 0x90, 0x07, 0x01, 0x10, 0xcc, 0x7a, 0x3e, 0xf4, 0xb8, 0x66, 0x3a, 0xf8 };
    uint8_t initData11[] = { 0x01, 0x12, 0xb4, 0x72, 0x46, 0x1c, 0xf0, 0xbe, 0x92, 0x40, 0x3c, 0xea, 0xce, 0xa4, 0x88, 0x76, 0x4a, 0x28, 0x04, 0xe2 };
    uint8_t initData12[] = { 0xff, 0x08, 0xf6, 0xcc, 0xe0, 0x98, 0x02, 0x00, 0x00, 0xd1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

    writeCharacteristic(initData1, sizeof(initData1), "init", false, false);
    QThread::msleep(400);
    writeCharacteristic(initData2, sizeof(initData2), "init", false, false);
    QThread::msleep(400);
    writeCharacteristic(initData1, sizeof(initData1), "init", false, false);
    QThread::msleep(400);
    writeCharacteristic(initData3, sizeof(initData3), "init", false, false);
    QThread::msleep(400);
    writeCharacteristic(initData1, sizeof(initData1), "init", false, false);
    QThread::msleep(400);
    writeCharacteristic(initData4, sizeof(initData4), "init", false, false);
    QThread::msleep(400);
    writeCharacteristic(initData5, sizeof(initData5), "init", false, false);
    QThread::msleep(400);
    writeCharacteristic(initData6, sizeof(initData6), "init", false, false);
    QThread::msleep(400);
    writeCharacteristic(initData5, sizeof(initData5), "init", false, false);
    QThread::msleep(400);
    writeCharacteristic(initData7, sizeof(initData7), "init", false, false);
    QThread::msleep(400);
    writeCharacteristic(initData1, sizeof(initData1), "init", false, false);
    QThread::msleep(400);
    writeCharacteristic(initData8, sizeof(initData8), "init", false, false);
    QThread::msleep(400);
    writeCharacteristic(initData9, sizeof(initData9), "init", false, false);
    QThread::msleep(400);
    writeCharacteristic(initData10, sizeof(initData10), "init", false, false);
    QThread::msleep(400);
    writeCharacteristic(initData11, sizeof(initData11), "init", false, false);
    QThread::msleep(400);
    writeCharacteristic(initData12, sizeof(initData12), "init", false, false);
    QThread::msleep(400);

    initDone = true;
}

void proformbike::stateChanged(QLowEnergyService::ServiceState state)
{
    QBluetoothUuid _gattWriteCharacteristicId((QString)"00001534-1412-efde-1523-785feabcd123");
    QBluetoothUuid _gattNotify1CharacteristicId((QString)"00001535-1412-efde-1523-785feabcd123");

    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceState>();
    debug("BTLE stateChanged " + QString::fromLocal8Bit(metaEnum.valueToKey(state)));

    if(state == QLowEnergyService::ServiceDiscovered)
    {
        //qDebug() << gattCommunicationChannelService->characteristics();

        gattWriteCharacteristic = gattCommunicationChannelService->characteristic(_gattWriteCharacteristicId);
        gattNotify1Characteristic = gattCommunicationChannelService->characteristic(_gattNotify1CharacteristicId);
        Q_ASSERT(gattWriteCharacteristic.isValid());
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
                connect(virtualBike,&virtualbike::debug ,this,&proformbike::debug);
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

void proformbike::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue)
{
    debug("descriptorWritten " + descriptor.name() + " " + newValue.toHex(' '));

    initRequest = true;
    emit connectedAndDiscovered();
}

void proformbike::characteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue)
{
    Q_UNUSED(characteristic);
    debug("characteristicWritten " + newValue.toHex(' '));
}

void proformbike::serviceScanDone(void)
{
    debug("serviceScanDone");

    QBluetoothUuid _gattCommunicationChannelServiceId((QString)"00001533-1412-efde-1523-785feabcd123");

    gattCommunicationChannelService = m_control->createServiceObject(_gattCommunicationChannelServiceId);
    connect(gattCommunicationChannelService, SIGNAL(stateChanged(QLowEnergyService::ServiceState)), this, SLOT(stateChanged(QLowEnergyService::ServiceState)));
    gattCommunicationChannelService->discoverDetails();
}

void proformbike::errorService(QLowEnergyService::ServiceError err)
{
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    debug("proformbike::errorService" + QString::fromLocal8Bit(metaEnum.valueToKey(err)) + m_control->errorString());
}

void proformbike::error(QLowEnergyController::Error err)
{
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    debug("proformbike::error" + QString::fromLocal8Bit(metaEnum.valueToKey(err)) + m_control->errorString());
}

void proformbike::deviceDiscovered(const QBluetoothDeviceInfo &device)
{
    debug("Found new device: " + device.name() + " (" + device.address().toString() + ')');
    if(device.name().startsWith("I_EB"))
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

bool proformbike::connected()
{
    if(!m_control)
        return false;
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

void* proformbike::VirtualBike()
{
    return virtualBike;
}

void* proformbike::VirtualDevice()
{
    return VirtualBike();
}

uint16_t proformbike::watts()
{
    if(currentCadence().value() == 0) return 0;

    return m_watts;
}

void proformbike::controllerStateChanged(QLowEnergyController::ControllerState state)
{
    qDebug() << "controllerStateChanged" << state;
    if(state == QLowEnergyController::UnconnectedState && m_control)
    {
        qDebug() << "trying to connect back again...";
        initDone = false;
        m_control->connectToDevice();
    }
}
