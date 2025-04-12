#include "lifefitnesstreadmill.h"

#include "devices/ftmsbike/ftmsbike.h"
#include "ios/lockscreen.h"
#include "virtualdevices/virtualtreadmill.h"
#include <QBluetoothLocalDevice>
#include <QDateTime>
#include <QFile>
#include <QMetaEnum>
#include <QSettings>

#include <QThread>
#include <math.h>
#ifdef Q_OS_ANDROID
#include <QLowEnergyConnectionParameters>
#endif
#include "keepawakehelper.h"
#include <chrono>

using namespace std::chrono_literals;

#ifdef Q_OS_IOS
extern quint8 QZ_EnableDiscoveryCharsAndDescripttors;
#endif

lifefitnesstreadmill::lifefitnesstreadmill(bool noWriteResistance, bool noHeartService) {
#ifdef Q_OS_IOS
    QZ_EnableDiscoveryCharsAndDescripttors = true;
#endif

    m_watt.setType(metric::METRIC_WATT);
    Speed.setType(metric::METRIC_SPEED);
    refresh = new QTimer(this);
    this->noWriteResistance = noWriteResistance;
    this->noHeartService = noHeartService;
    initDone = false;
    connect(refresh, &QTimer::timeout, this, &lifefitnesstreadmill::update);
    refresh->start(200ms);
}

void lifefitnesstreadmill::writeCharacteristic(QLowEnergyService *service, QLowEnergyCharacteristic characteristic,
                                               uint8_t *data, uint8_t data_len, QString info, bool disable_log,
                                               bool wait_for_response) {
    QEventLoop loop;
    QTimer timeout;

    if (!service) {
        qDebug() << "no gattCustomService available";
        return;
    }

    if (wait_for_response) {
        connect(this, &lifefitnesstreadmill::packetReceived, &loop, &QEventLoop::quit);
        timeout.singleShot(3000, &loop, SLOT(quit()));
    } else {
        connect(service, SIGNAL(characteristicWritten(QLowEnergyCharacteristic, QByteArray)), &loop, SLOT(quit()));
        timeout.singleShot(3000, &loop, SLOT(quit()));
    }

    if (writeBuffer) {
        delete writeBuffer;
    }
    writeBuffer = new QByteArray((const char *)data, data_len);

    if (characteristic.properties() & QLowEnergyCharacteristic::WriteNoResponse)
        service->writeCharacteristic(characteristic, *writeBuffer, QLowEnergyService::WriteWithoutResponse);
    else
        service->writeCharacteristic(characteristic, *writeBuffer);

    if (!disable_log)
        qDebug() << " >> " << writeBuffer->toHex(' ') << " // " << info;

    loop.exec();
}

void lifefitnesstreadmill::waitForAPacket() {
    QEventLoop loop;
    QTimer timeout;
    connect(this, &lifefitnesstreadmill::packetReceived, &loop, &QEventLoop::quit);
    timeout.singleShot(3000, &loop, SLOT(quit()));
    loop.exec();
}

void lifefitnesstreadmill::btinit() {
    QByteArray descriptor;
    descriptor.append((char)0x01);
    descriptor.append((char)0x00);

    if (gattWriteChar4CustomService2.isValid()) {

        uint8_t initData1[1] = {0x01};
        uint8_t initData2a[20] = {0x38, 0x66, 0x65, 0x64, 0x61, 0x38, 0x38, 0x39, 0x31, 0x64,
                                  0x62, 0x61, 0x34, 0x30, 0x31, 0x66, 0x38, 0x39, 0x39, 0x30};
        uint8_t initData2b[12] = {0x30, 0x32, 0x33, 0x30, 0x37, 0x39, 0x35, 0x66, 0x30, 0x38, 0x36, 0x30};
        uint8_t initData3[1] = {0x00};
        uint8_t initData4[7] = {0x00, 0x00, 0x00, 0x01, 0xb8, 0x5b, 0x5d};
        uint8_t initData5[1] = {0x02};

        writeCharacteristic(gattCustomService1, gattWriteChar1CustomService1, initData1, sizeof(initData1),
                            QStringLiteral("init"), false, false);
        writeCharacteristic(gattCustomService2, gattWriteChar3CustomService2, initData2a, sizeof(initData2a),
                            QStringLiteral("init"), false, false);
        writeCharacteristic(gattCustomService2, gattWriteChar3CustomService2, initData2b, sizeof(initData2b),
                            QStringLiteral("init"), false, false);
        writeCharacteristic(gattCustomService2, gattWriteChar4CustomService2, initData3, sizeof(initData3),
                            QStringLiteral("init"), false, false);
        writeCharacteristic(gattCustomService1, gattWriteChar2CustomService1, initData4, sizeof(initData4),
                            QStringLiteral("init"), false, false);
        writeCharacteristic(gattCustomService1, gattWriteChar1CustomService1, initData5, sizeof(initData5),
                            QStringLiteral("init"), false, false);
    } else if(lifet5) {
        // From pkt5841 (after first 12 bytes)
        uint8_t initData1[5] = {0xf1, 0x00, 0x01, 0x11, 0xee}; // |.@....|

        // From pkt5849 (after first 12 bytes)
        uint8_t initData2[7] = {0xf1, 0x00, 0x03, 0x02, 0x00, 0x08, 0xf3}; // |.@....|

        // From pkt5851 (after first 12 bytes)
        uint8_t initData3[21] = {0xf1, 0x00, 0x11, 0x04, 0x00, 0x00, 0x4e, 0x6f, 0x72, 0x77, 0x65, 
                                0x67, 0x69, 0x61, 0x6e, 0x20, 0x34, 0x78, 0x34, 0x00, 0x41}; // |.@....Norweg|, |ian 4x4.A|

        // From pkt5854 (after first 12 bytes)
        uint8_t initData4[28] = {0xf1, 0x00, 0x18, 0x04, 0x00, 0x01, 0x54, 0x72, 0x65, 0x61, 0x64, 
                                0x6d, 0x69, 0x6c, 0x6c, 0x20, 0x30, 0x33, 0x2f, 0x30, 0x38, 0x2f, 
                                0x32, 0x30, 0x32, 0x34, 0x00, 0x34}; // |.@....Treadm|, |ill 03/0|, |8/2024.4|

        // From pkt5856 (after first 12 bytes)
        uint8_t initData5[20] = {0xf1, 0x00, 0x10, 0x04, 0x00, 0x02, 0x35, 0x20, 0x78, 0x20, 0x31, 
                                0x6b, 0x6d, 0x20, 0x61, 0x74, 0x20, 0x39, 0x00, 0xa6}; // |.@....5 x 1k|, |m at 9..|

        // From pkt5859 (after first 12 bytes)
        uint8_t initData6[19] = {0xf1, 0x00, 0x0f, 0x04, 0x00, 0x03, 0x35, 0x20, 0x78, 0x20, 0x31, 
                                0x6b, 0x6d, 0x20, 0x40, 0x20, 0x39, 0x00, 0x3b}; // |.@....5 x 1k|, |m @ 9.;|

        // From pkt5861 (after first 12 bytes)
        uint8_t initData7[21] = {0xf1, 0x00, 0x11, 0x04, 0x00, 0x04, 0x36, 0x20, 0x78, 0x20, 0x35, 
                                0x30, 0x30, 0x6d, 0x20, 0x40, 0x20, 0x31, 0x30, 0x00, 0x16}; // |.@....6 x 50|, |0m @ 10.|

        // From pkt5864 (after first 12 bytes)
        uint8_t initData8[20] = {0xf1, 0x00, 0x10, 0x04, 0x00, 0x05, 0x36, 0x20, 0x78, 0x20, 0x38, 
                                0x30, 0x30, 0x6d, 0x20, 0x40, 0x20, 0x39, 0x00, 0x3b}; // |.@....6 x 80|, |0m @ 9.;|

        // From pkt5866 (after first 12 bytes)
        uint8_t initData9[33] = {0xf1, 0x00, 0x1d, 0x04, 0x00, 0x06, 0x32, 0x30, 0x6d, 0x69, 0x6e, 
                                0x20, 0x69, 0x6e, 0x74, 0x65, 0x72, 0x76, 0x61, 0x6c, 0x73, 0x20, 
                                0x28, 0x31, 0x30, 0x20, 0x72, 0x75, 0x6e, 0x73, 0x29, 0x00, 0x81}; // |.@....20min |, |intervals|, |s (10 ru|, |ns)..|

        // From pkt5869 (after first 12 bytes)
        uint8_t initData10[20] = {0xf1, 0x00, 0x10, 0x04, 0x00, 0x07, 0x36, 0x20, 0x78, 0x20, 0x32, 
                                    0x20, 0x6d, 0x69, 0x6e, 0x20, 0x40, 0x39, 0x00, 0xc8}; // |.@....6 x 2 |, |min @9..|

        // From pkt5876 (after first 12 bytes)
        uint8_t initData11[5] = {0x6e, 0x64, 0x65, 0x72, 0xa6}; // |.@nder.|

        // From pkt5888 (after first 12 bytes)
        uint8_t initData12[5] = {0x4c, 0x49, 0x42, 0x52, 0x8d}; // |.@LIBR.|

        // From pkt5894 (after first 12 bytes)
        uint8_t initData13[5] = {0x5f, 0x4c, 0x49, 0x42, 0x22}; // |.@_LIB"|

        // From pkt5895 (after first 12 bytes)
        uint8_t initData14[202] = {0xf1, 0x00, 0xc6, 0x27, 0x02, 0x03, 0x52, 0x41, 0x52, 0x59, 0x22, 0x7d,
                                    0x2c, 0x7b, 0x22, 0x77, 0x6f, 0x72, 0x6b, 0x6f, 0x75, 0x74, 0x49, 0x64, 
                                    0x22, 0x3a, 0x37, 0x2c, 0x22, 0x66, 0x69, 0x6c, 0x65, 0x4e, 0x61, 0x6d, 
                                    0x65, 0x22, 0x3a, 0x22, 0x36, 0x20, 0x78, 0x20, 0x32, 0x20, 0x6d, 0x69, 
                                    0x6e, 0x20, 0x40, 0x39, 0x2e, 0x35, 0x6d, 0x70, 0x68, 0x20, 0x31, 0x20, 
                                    0x6d, 0x69, 0x6e, 0x20, 0x77, 0x61, 0x6c, 0x6b, 0x20, 0x72, 0x65, 0x73, 
                                    0x74, 0x22, 0x2c, 0x22, 0x77, 0x6f, 0x72, 0x6b, 0x6f, 0x75, 0x74, 0x4e, 
                                    0x61, 0x6d, 0x65, 0x22, 0x3a, 0x22, 0x36, 0x20, 0x78, 0x20, 0x32, 0x20, 
                                    0x6d, 0x69, 0x6e, 0x20, 0x40, 0x39, 0x2e, 0x35, 0x6d, 0x70, 0x68, 0x20, 
                                    0x31, 0x20, 0x6d, 0x69, 0x6e, 0x20, 0x77, 0x61, 0x6c, 0x6b, 0x20, 0x72, 
                                    0x65, 0x73, 0x74, 0x22, 0x2c, 0x22, 0x69, 0x6e, 0x64, 0x65, 0x78, 0x22, 
                                    0x3a, 0x37, 0x2c, 0x22, 0x77, 0x6f, 0x72, 0x6b, 0x6f, 0x75, 0x74, 0x54, 
                                    0x79, 0x70, 0x65, 0x22, 0x3a, 0x22, 0x57, 0x4f, 0x52, 0x4b, 0x4f, 0x55, 
                                    0x54, 0x5f, 0x4c, 0x49, 0x42, 0x52, 0x41, 0x52, 0x59, 0x22, 0x7d, 0x5d, 
                                    0x7d, 0x3c, 0x2f, 0x77, 0x6f, 0x72, 0x6b, 0x6f, 0x75, 0x74, 0x53, 0x75, 
                                    0x6d, 0x6d, 0x61, 0x72, 0x69, 0x65, 0x73, 0x3e, 0x0a, 0x3c, 0x2f, 0x64, 
                                    0x69, 0x73, 0x63, 0x6f, 0x76, 0x65, 0x72, 0x3e, 0x0a, 0xe6}; // |.R.@...'|, |..RARY"}|, |,{"worko|, and many more lines

        // From pkt6713 (after first 12 bytes)
        uint8_t initData15[5] = {0xf1, 0x00, 0x01, 0x0a, 0xf5}; // |.R.@....|

        // From pkt6929 (after first 12 bytes)
        uint8_t initData16[6] = {0xf1, 0x00, 0x02, 0x0c, 0x3e, 0xb4}; // |.R.@....|
        
        // Write all the initialization data sequentially
        writeCharacteristic(gattCustomService1, gattWriteChar1CustomService1, initData1, sizeof(initData1), QStringLiteral("init lifet5 1"), false, true);
        writeCharacteristic(gattCustomService1, gattWriteChar1CustomService1, initData2, sizeof(initData2), QStringLiteral("init lifet5 2"), false, true);
        writeCharacteristic(gattCustomService1, gattWriteChar1CustomService1, initData3, sizeof(initData3), QStringLiteral("init lifet5 3"), false, true);
        writeCharacteristic(gattCustomService1, gattWriteChar1CustomService1, initData4, sizeof(initData4), QStringLiteral("init lifet5 4"), false, true);
        writeCharacteristic(gattCustomService1, gattWriteChar1CustomService1, initData5, sizeof(initData5), QStringLiteral("init lifet5 5"), false, true);
        writeCharacteristic(gattCustomService1, gattWriteChar1CustomService1, initData6, sizeof(initData6), QStringLiteral("init lifet5 6"), false, true);
        writeCharacteristic(gattCustomService1, gattWriteChar1CustomService1, initData7, sizeof(initData7), QStringLiteral("init lifet5 7"), false, true);
        writeCharacteristic(gattCustomService1, gattWriteChar1CustomService1, initData8, sizeof(initData8), QStringLiteral("init lifet5 8"), false, true);
        writeCharacteristic(gattCustomService1, gattWriteChar1CustomService1, initData9, sizeof(initData9), QStringLiteral("init lifet5 9"), false, true);
        writeCharacteristic(gattCustomService1, gattWriteChar1CustomService1, initData10, sizeof(initData10), QStringLiteral("init lifet5 10"), false, true);
        writeCharacteristic(gattCustomService1, gattWriteChar1CustomService1, initData11, sizeof(initData11), QStringLiteral("init lifet5 11"), false, true);
        writeCharacteristic(gattCustomService1, gattWriteChar1CustomService1, initData12, sizeof(initData12), QStringLiteral("init lifet5 12"), false, true);
        writeCharacteristic(gattCustomService1, gattWriteChar1CustomService1, initData13, sizeof(initData13), QStringLiteral("init lifet5 13"), false, true);
        writeCharacteristic(gattCustomService1, gattWriteChar1CustomService1, initData14, sizeof(initData14), QStringLiteral("init lifet5 14"), false, true);
        writeCharacteristic(gattCustomService1, gattWriteChar1CustomService1, initData15, sizeof(initData15), QStringLiteral("init lifet5 15"), false, true);
        writeCharacteristic(gattCustomService1, gattWriteChar1CustomService1, initData16, sizeof(initData16), QStringLiteral("init lifet5 16"), false, true);

        QLowEnergyCharacteristic gattTreadmillData1 = gattCustomService1->characteristic(QBluetoothUuid(QStringLiteral("4a8ff3f1-c933-11e3-9c1a-0800200c9a66")));
        if (gattTreadmillData1.isValid()) {
            gattCustomService1->writeDescriptor(
                gattTreadmillData1.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration), descriptor);
        }

    }

    QBluetoothUuid _gattTreadmillDataId((quint16)0x2ACD);
    QBluetoothUuid _gattTrainingStatusId((quint16)0x2AD3);
    QBluetoothUuid _gattCrossTrainerDataId((quint16)0x2ACE);
    if(gattFTMSService == nullptr) {
        qDebug() << "gattFTMSService is empty!";
        return;
    }
    QLowEnergyCharacteristic gattTreadmillData = gattFTMSService->characteristic(_gattTreadmillDataId);
    QLowEnergyCharacteristic gattTrainingStatus = gattFTMSService->characteristic(_gattTrainingStatusId);
    QLowEnergyCharacteristic gattCrossTrainerData = gattFTMSService->characteristic(_gattCrossTrainerDataId);
    gattFTMSService->writeDescriptor(gattTrainingStatus.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration),
                                     descriptor);
    if (gattTreadmillData.isValid()) {
        gattFTMSService->writeDescriptor(
            gattTreadmillData.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration), descriptor);
        gattFTMSService->writeDescriptor(
            gattTreadmillData.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration), descriptor);
    } else if (gattCrossTrainerData.isValid()) {
        gattFTMSService->writeDescriptor(
            gattCrossTrainerData.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration), descriptor);
    }

    initDone = true;
}

void lifefitnesstreadmill::update() {
    if (m_control->state() == QLowEnergyController::UnconnectedState) {

        emit disconnected();
        return;
    }

    if (initRequest && firstStateChanged) {
        btinit();
        initRequest = false;
    } else if (bluetoothDevice.isValid() //&&

               // m_control->state() == QLowEnergyController::DiscoveredState //&&
               // gattCommunicationChannelService &&
               // gattWriteCharacteristic.isValid() &&
               // gattNotify1Characteristic.isValid() &&
               /*initDone*/) {

        QSettings settings;
        update_metrics(true, watts(settings.value(QZSettings::weight, QZSettings::default_weight).toFloat()));

        // updating the treadmill console every second
        if (sec1Update++ == (500 / refresh->interval())) {

            sec1Update = 0;
            // updateDisplay(elapsed);
        }

        if (requestSpeed != -1) {
            qDebug() << "requestSpeed=" << requestSpeed;
            if (requestSpeed != currentSpeed().value() &&
                fabs(requestSpeed - currentSpeed().value()) > minStepSpeed() && requestSpeed >= 0 &&
                requestSpeed <= 22) {
                emit debug(QStringLiteral("writing speed ") + QString::number(requestSpeed));
                forceSpeed(requestSpeed);
            }
            requestSpeed = -1;
        }
        if (requestInclination != -100) {
            qDebug() << "requestInclination=" << requestInclination;
            if (requestInclination < 0)
                requestInclination = 0;
            else {
                // the treadmill accepts only .5 steps
                requestInclination = std::llround(requestInclination * 2) / 2.0;
            }
            if (requestInclination != currentInclination().value() && requestInclination >= 0 &&
                requestInclination <= 15) {
                emit debug(QStringLiteral("writing incline ") + QString::number(requestInclination));
                forceIncline(requestInclination);
            }
            requestInclination = -100;
        }
        if (requestStart != -1) {
            emit debug(QStringLiteral("starting..."));
            if (lastSpeed == 0.0) {

                lastSpeed = 0.5;
            }
            requestStart = -1;
            emit tapeStarted();

            lastStart = QDateTime::currentMSecsSinceEpoch();
        }
        if (requestStop != -1) {
            emit debug(QStringLiteral("stopping..."));

            lastStop = QDateTime::currentMSecsSinceEpoch();

            requestStop = -1;
        }
        if (requestIncreaseFan != -1) {
            emit debug(QStringLiteral("increasing fan speed..."));

            // sendChangeFanSpeed(FanSpeed + 1);
            requestIncreaseFan = -1;
        } else if (requestDecreaseFan != -1) {
            emit debug(QStringLiteral("decreasing fan speed..."));

            // sendChangeFanSpeed(FanSpeed - 1);
            requestDecreaseFan = -1;
        }
    }
}

// example frame: 55aa320003050400532c00150000
void lifefitnesstreadmill::forceSpeed(double requestSpeed) {
    QSettings settings;
    /*
        if (gattFTMSService) {
            // for the Tecnogym Myrun
            uint8_t write[] = {FTMS_REQUEST_CONTROL};
            writeCharacteristic(gattFTMSService, gattWriteCharControlPointId, write, sizeof(write), "requestControl",
       false, true); write[0] = {FTMS_START_RESUME}; writeCharacteristic(gattFTMSService, gattWriteCharControlPointId,
       write, sizeof(write), "start simulation", false, true);

            uint8_t writeS[] = {FTMS_SET_TARGET_SPEED, 0x00, 0x00};
            writeS[1] = ((uint16_t)requestSpeed * 100) & 0xFF;
            writeS[2] = ((uint16_t)requestSpeed * 100) >> 8;

            writeCharacteristic(gattFTMSService, gattWriteCharControlPointId, writeS, sizeof(writeS),
                                QStringLiteral("forceSpeed"), false, true);
        }*/
}

void lifefitnesstreadmill::forceIncline(double requestIncline) {
    QSettings settings;
    /*
        if (gattFTMSService) {
            // for the Tecnogym Myrun
            uint8_t write[] = {FTMS_REQUEST_CONTROL};
            writeCharacteristic(gattFTMSService, gattWriteCharControlPointId, write, sizeof(write), "requestControl",
       false, true); write[0] = {FTMS_START_RESUME}; writeCharacteristic(gattFTMSService, gattWriteCharControlPointId,
       write, sizeof(write), "start simulation", false, true);

            uint8_t writeS[] = {FTMS_SET_TARGET_INCLINATION, 0x00, 0x00};
            writeS[1] = ((int16_t)requestIncline * 10) & 0xFF;
            writeS[2] = ((int16_t)requestIncline * 10) >> 8;

            writeCharacteristic(gattFTMSService, gattWriteCharControlPointId, writeS, sizeof(writeS),
                                QStringLiteral("forceIncline"), false, true);
        }*/
}

void lifefitnesstreadmill::serviceDiscovered(const QBluetoothUuid &gatt) {
    emit debug(QStringLiteral("serviceDiscovered ") + gatt.toString());
}

void lifefitnesstreadmill::characteristicChanged(const QLowEnergyCharacteristic &characteristic,
                                                 const QByteArray &newValue) {
    double heart = 0; // NOTE : Should be initialized with a value to shut clang-analyzer's
                      // UndefinedBinaryOperatorResult
    // qDebug() << "characteristicChanged" << characteristic.uuid() << newValue << newValue.length();

    bool distanceEval = false;
    QSettings settings;
    // bool horizon_paragon_x = settings.value(QZSettings::horizon_paragon_x,
    // QZSettings::default_horizon_paragon_x).toBool();
    bool disable_hr_frommachinery =
        settings.value(QZSettings::heart_ignore_builtin, QZSettings::default_heart_ignore_builtin).toBool();
    QString heartRateBeltName =
        settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name).toString();

    emit debug(QStringLiteral(" << ") + characteristic.uuid().toString() + " " + QString::number(newValue.length()) +
               " " + newValue.toHex(' '));

    if (characteristic.uuid() == QBluetoothUuid(QStringLiteral("4a8ff3f1-c933-11e3-9c1a-0800200c9a66")) && newValue.length() == 40) {
        Speed = ((double)newValue.at(32)) / 10.0;
        Inclination = ((double)newValue.at(27)) / 10.0;
        distanceEval = true;
        if (firstDistanceCalculated) {
            Distance += ((Speed.value() / 3600000.0) *
                     ((double)lastRefreshCharacteristicChanged.msecsTo(QDateTime::currentDateTime())));
            if(watts(settings.value(QZSettings::weight, QZSettings::default_weight).toFloat()))
                KCal +=
                    ((((0.048 *
                            ((double)watts(settings.value(QZSettings::weight, QZSettings::default_weight).toFloat())) +
                        1.19) *
                       settings.value(QZSettings::weight, QZSettings::default_weight).toFloat() * 3.5) /
                      200.0) /
                     (60000.0 /
                      ((double)lastRefreshCharacteristicChanged.msecsTo(
                          QDateTime::currentDateTime())))); //(( (0.048* Output in watts +1.19) * body weight in
                                                            // kg * 3.5) / 200 ) / 60
        }

#ifdef Q_OS_ANDROID
        if (settings.value(QZSettings::ant_heart, QZSettings::default_ant_heart).toBool())
            Heart = (uint8_t)KeepAwakeHelper::heart();
#endif

    } else if (characteristic.uuid() == QBluetoothUuid((quint16)0x2ACD)) {
        lastPacket = newValue;

        // default flags for this treadmill is 84 04

        union flags {
            struct {

                uint16_t moreData : 1;
                uint16_t avgSpeed : 1;
                uint16_t totalDistance : 1;
                uint16_t inclination : 1;
                uint16_t elevation : 1;
                uint16_t instantPace : 1;
                uint16_t averagePace : 1;
                uint16_t expEnergy : 1;
                uint16_t heartRate : 1;
                uint16_t metabolic : 1;
                uint16_t elapsedTime : 1;
                uint16_t remainingTime : 1;
                uint16_t forceBelt : 1;
                uint16_t spare : 3;
            };

            uint16_t word_flags;
        };

        flags Flags;
        int index = 0;
        Flags.word_flags = (newValue.at(1) << 8) | newValue.at(0);
        index += 2;

        if (!Flags.moreData) {
            Speed = ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) |
                              (uint16_t)((uint8_t)newValue.at(index)))) /
                    100.0;
            index += 2;
            emit debug(QStringLiteral("Current Speed: ") + QString::number(Speed.value()));
        }

        if (Flags.avgSpeed && newValue.length() > index + 1) {
            double avgSpeed;
            avgSpeed = ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) |
                                 (uint16_t)((uint8_t)newValue.at(index)))) /
                       100.0;
            index += 2;
            emit debug(QStringLiteral("Current Average Speed: ") + QString::number(avgSpeed));
        }

        if (Flags.totalDistance && newValue.length() > index + 2) {
            // ignoring the distance, because it's a total life odometer
            // Distance = ((double)((((uint32_t)((uint8_t)newValue.at(index + 2)) << 16) |
            // (uint32_t)((uint8_t)newValue.at(index + 1)) << 8) | (uint32_t)((uint8_t)newValue.at(index)))) / 1000.0;
            index += 3;
        }
        // else
        {
            if (firstDistanceCalculated)
                Distance += ((Speed.value() / 3600000.0) *
                             ((double)lastRefreshCharacteristicChanged.msecsTo(QDateTime::currentDateTime())));
            distanceEval = true;
        }

        emit debug(QStringLiteral("Current Distance: ") + QString::number(Distance.value()));

        if (Flags.inclination && newValue.length() > index + 1) {
            Inclination = ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) |
                                    (uint16_t)((uint8_t)newValue.at(index)))) /
                          10.0;
            index += 4; // the ramo value is useless
            emit debug(QStringLiteral("Current Inclination: ") + QString::number(Inclination.value()));
        }

        if (Flags.elevation && newValue.length() > index + 3) {
            index += 4; // TODO
        }

        if (Flags.instantPace && newValue.length() > index) {
            index += 1; // TODO
        }

        if (Flags.averagePace && newValue.length() > index) {
            index += 1; // TODO
        }

        if (Flags.expEnergy && newValue.length() > index + 1) {
            KCal = ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) |
                             (uint16_t)((uint8_t)newValue.at(index))));
            index += 2;

            // energy per hour
            index += 2;

            // energy per minute
            index += 1;
        } else {
            if (firstDistanceCalculated &&
                watts(settings.value(QZSettings::weight, QZSettings::default_weight).toFloat()))
                KCal +=
                    ((((0.048 *
                            ((double)watts(settings.value(QZSettings::weight, QZSettings::default_weight).toFloat())) +
                        1.19) *
                       settings.value(QZSettings::weight, QZSettings::default_weight).toFloat() * 3.5) /
                      200.0) /
                     (60000.0 /
                      ((double)lastRefreshCharacteristicChanged.msecsTo(
                          QDateTime::currentDateTime())))); //(( (0.048* Output in watts +1.19) * body weight in
                                                            // kg * 3.5) / 200 ) / 60
            distanceEval = true;
        }

        emit debug(QStringLiteral("Current KCal: ") + QString::number(KCal.value()));

#ifdef Q_OS_ANDROID
        if (settings.value(QZSettings::ant_heart, QZSettings::default_ant_heart).toBool())
            Heart = (uint8_t)KeepAwakeHelper::heart();
        else
#endif
        {
            if (Flags.heartRate) {
                if (index < newValue.length()) {

                    heart = ((double)(((uint8_t)newValue.at(index))));
                    emit debug(QStringLiteral("Current Heart: ") + QString::number(heart));
                } else {
                    emit debug(QStringLiteral("Error on parsing heart!"));
                }
                // index += 1; //NOTE: clang-analyzer-deadcode.DeadStores
            }
        }

        if (Flags.metabolic) {
            // todo
        }

        if (Flags.elapsedTime) {
            // todo
        }

        if (Flags.remainingTime) {
            // todo
        }

        if (Flags.forceBelt) {
            // todo
        }
    } else if (characteristic.uuid() == QBluetoothUuid((quint16)0x2ACE)) {
        union flags {
            struct {
                uint32_t moreData : 1;
                uint32_t avgSpeed : 1;
                uint32_t totDistance : 1;
                uint32_t stepCount : 1;
                uint32_t strideCount : 1;
                uint32_t elevationGain : 1;
                uint32_t rampAngle : 1;
                uint32_t resistanceLvl : 1;
                uint32_t instantPower : 1;
                uint32_t avgPower : 1;
                uint32_t expEnergy : 1;
                uint32_t heartRate : 1;
                uint32_t metabolicEq : 1;
                uint32_t elapsedTime : 1;
                uint32_t remainingTime : 1;
                uint32_t movementDirection : 1;
                uint32_t spare : 8;
            };

            uint32_t word_flags;
        };

        flags Flags;
        int index = 0;
        Flags.word_flags = (newValue.at(2) << 16) | (newValue.at(1) << 8) | newValue.at(0);
        index += 3;

        if (!Flags.moreData && newValue.length() > index + 1) {
            Speed = ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) |
                              (uint16_t)((uint8_t)newValue.at(index)))) /
                    100.0;
            emit debug(QStringLiteral("Current Speed: ") + QString::number(Speed.value()));
            index += 2;
        }

        if (Flags.avgSpeed && newValue.length() > index + 1) {
            double avgSpeed;
            avgSpeed = ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) |
                                 (uint16_t)((uint8_t)newValue.at(index)))) /
                       100.0;
            index += 2;
            emit debug(QStringLiteral("Current Average Speed: ") + QString::number(avgSpeed));
        }

        if (Flags.totDistance && newValue.length() > index + 2) {
            Distance = ((double)((((uint32_t)((uint8_t)newValue.at(index + 2)) << 16) |
                                  (uint32_t)((uint8_t)newValue.at(index + 1)) << 8) |
                                 (uint32_t)((uint8_t)newValue.at(index)))) /
                       1000.0;
            index += 3;
        } else {
            if (firstDistanceCalculated)
                Distance += ((Speed.value() / 3600000.0) *
                             ((double)lastRefreshCharacteristicChanged.msecsTo(QDateTime::currentDateTime())));
            distanceEval = true;
        }

        emit debug(QStringLiteral("Current Distance: ") + QString::number(Distance.value()));

        if (Flags.stepCount && newValue.length() > index + 1) {
            if (settings.value(QZSettings::cadence_sensor_name, QZSettings::default_cadence_sensor_name)
                    .toString()
                    .startsWith(QStringLiteral("Disabled"))) {
                Cadence = ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) |
                                    (uint16_t)((uint8_t)newValue.at(index))));
            }
            emit debug(QStringLiteral("Current Cadence: ") + QString::number(Cadence.value()));

            index += 2;
            index += 2;
        } else {
            cadenceFromAppleWatch();
        }

        if (Flags.strideCount) {
            index += 2;
        }

        if (Flags.elevationGain) {
            index += 2;
            index += 2;
        }

        if (Flags.rampAngle) {
            index += 2;
            index += 2;
        }

        if (Flags.resistanceLvl && newValue.length() > index + 1) {
            Resistance = ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) |
                                   (uint16_t)((uint8_t)newValue.at(index))));
            index += 2;
            emit debug(QStringLiteral("Current Resistance: ") + QString::number(Resistance.value()));
        }

        if (Flags.instantPower && newValue.length() > index + 1) {
            if (settings.value(QZSettings::power_sensor_name, QZSettings::default_power_sensor_name)
                    .toString()
                    .startsWith(QStringLiteral("Disabled")))
                m_watt = ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) |
                                   (uint16_t)((uint8_t)newValue.at(index))));
            emit debug(QStringLiteral("Current Watt: ") + QString::number(m_watt.value()));
            index += 2;
        }

        if (Flags.avgPower && newValue.length() > index + 1) {
            double avgPower;
            avgPower = ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) |
                                 (uint16_t)((uint8_t)newValue.at(index))));
            emit debug(QStringLiteral("Current Average Watt: ") + QString::number(avgPower));
            index += 2;
        }

        if (Flags.expEnergy && newValue.length() > index + 1) {
            KCal = ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) |
                             (uint16_t)((uint8_t)newValue.at(index))));
            index += 2;

            // energy per hour
            index += 2;

            // energy per minute
            index += 1;
        } else {
            if (firstDistanceCalculated &&
                watts(settings.value(QZSettings::weight, QZSettings::default_weight).toFloat()))
                KCal +=
                    ((((0.048 *
                            ((double)watts(settings.value(QZSettings::weight, QZSettings::default_weight).toFloat())) +
                        1.19) *
                       settings.value(QZSettings::weight, QZSettings::default_weight).toFloat() * 3.5) /
                      200.0) /
                     (60000.0 /
                      ((double)lastRefreshCharacteristicChanged.msecsTo(
                          QDateTime::currentDateTime())))); //(( (0.048* Output in watts +1.19) * body weight in
                                                            // kg * 3.5) / 200 ) / 60
            distanceEval = true;
        }

        emit debug(QStringLiteral("Current KCal: ") + QString::number(KCal.value()));

#ifdef Q_OS_ANDROID
        if (settings.value(QZSettings::ant_heart, QZSettings::default_ant_heart).toBool())
            Heart = (uint8_t)KeepAwakeHelper::heart();
        else
#endif
        {
            if (Flags.heartRate && !disable_hr_frommachinery && newValue.length() > index) {
                Heart = ((double)(((uint8_t)newValue.at(index))));
                // index += 1; // NOTE: clang-analyzer-deadcode.DeadStores
                emit debug(QStringLiteral("Current Heart: ") + QString::number(Heart.value()));
            } else {
                Flags.heartRate = false;
            }
            heart = Flags.heartRate;
        }

        if (Flags.metabolicEq) {
            // todo
        }

        if (Flags.elapsedTime) {
            // todo
        }

        if (Flags.remainingTime) {
            // todo
        }
    }

    if (heartRateBeltName.startsWith(QStringLiteral("Disabled"))) {
        if (heart == 0.0 ||
            settings.value(QZSettings::heart_ignore_builtin, QZSettings::default_heart_ignore_builtin).toBool()) {
            update_hr_from_external();
        } else {
            Heart = heart;
        }
    }

    if (Speed.value() > 0)
        lastStart = 0;
    else
        lastStop = 0;

    if (distanceEval) {
        firstDistanceCalculated = true;
        lastRefreshCharacteristicChanged = QDateTime::currentDateTime();
    }

    if (m_control->error() != QLowEnergyController::NoError) {
        qDebug() << QStringLiteral("QLowEnergyController ERROR!!") << m_control->errorString();
    }
}

void lifefitnesstreadmill::stateChanged(QLowEnergyService::ServiceState state) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceState>();
    QBluetoothUuid _gattWriteChar1CustomService1(QStringLiteral("5da558dc-9cb2-11e5-8994-feff819cdc9f"));
    QBluetoothUuid _gattWriteChar2CustomService1(QStringLiteral("5da569e2-9cb2-11e5-8994-feff819cdc9f"));
    QBluetoothUuid _gattWriteChar3CustomService2(QStringLiteral("5da54cf2-9cb2-11e5-8994-feff819cdc9f"));
    QBluetoothUuid _gattWriteChar4CustomService2(QStringLiteral("ce78e85e-9cb6-11e5-8994-feff819cdc9f"));
    QBluetoothUuid _gattWriteChar5CustomService3(QStringLiteral("c52d3161-d1a1-11e3-9c1a-0800200c9a66"));
    QBluetoothUuid _gattWriteCharControlPointId((quint16)0x2AD9);
    QBluetoothUuid _gattTreadmillDataId((quint16)0x2ACD);
    QBluetoothUuid _gattCrossTrainerDataId((quint16)0x2ACE);
    emit debug(QStringLiteral("BTLE stateChanged ") + QString::fromLocal8Bit(metaEnum.valueToKey(state)));

    for (QLowEnergyService *s : qAsConst(gattCommunicationChannelService)) {
        qDebug() << QStringLiteral("stateChanged") << s->serviceUuid() << s->state();
        if (s->state() != QLowEnergyService::ServiceDiscovered && s->state() != QLowEnergyService::InvalidService) {
            qDebug() << QStringLiteral("not all services discovered");
            return;
        }
    }

    qDebug() << QStringLiteral("all services discovered!");

    notificationSubscribed = 0;

    for (QLowEnergyService *s : qAsConst(gattCommunicationChannelService)) {
        if (s->state() == QLowEnergyService::ServiceDiscovered) {
            // establish hook into notifications
            connect(s, &QLowEnergyService::characteristicChanged, this, &lifefitnesstreadmill::characteristicChanged);
            connect(s, &QLowEnergyService::characteristicWritten, this, &lifefitnesstreadmill::characteristicWritten);
            connect(s, &QLowEnergyService::characteristicRead, this, &lifefitnesstreadmill::characteristicRead);
            connect(
                s, static_cast<void (QLowEnergyService::*)(QLowEnergyService::ServiceError)>(&QLowEnergyService::error),
                this, &lifefitnesstreadmill::errorService);
            connect(s, &QLowEnergyService::descriptorWritten, this, &lifefitnesstreadmill::descriptorWritten);
            connect(s, &QLowEnergyService::descriptorRead, this, &lifefitnesstreadmill::descriptorRead);

            qDebug() << s->serviceUuid() << QStringLiteral("connected!");

            auto characteristics_list = s->characteristics();
            for (const QLowEnergyCharacteristic &c : qAsConst(characteristics_list)) {
                qDebug() << QStringLiteral("char uuid") << c.uuid() << QStringLiteral("handle") << c.handle();

                if (c.properties() & QLowEnergyCharacteristic::Write && c.uuid() == _gattWriteCharControlPointId) {
                    qDebug() << QStringLiteral("FTMS service and Control Point found");
                    gattWriteCharControlPointId = c;
                    gattFTMSService = s;
                } else if (c.uuid() == _gattTreadmillDataId && gattFTMSService == nullptr) {
                    // some treadmills doesn't have the control point so i need anyway to get the FTMS Service at least
                    gattFTMSService = s;
                } else if (c.uuid() == _gattCrossTrainerDataId && gattFTMSService == nullptr) {
                    // some treadmills doesn't have the control point and also are Cross Trainer devices so i need
                    // anyway to get the FTMS Service at least
                    gattFTMSService = s;
                }

                if (c.properties() & QLowEnergyCharacteristic::Write && c.uuid() == _gattWriteChar1CustomService1) {
                    qDebug() << QStringLiteral("Custom service and Control Point 1 found");
                    gattWriteChar1CustomService1 = c;
                    gattCustomService1 = s;
                } else if (c.properties() & QLowEnergyCharacteristic::Write &&
                           c.uuid() == _gattWriteChar2CustomService1) {
                    qDebug() << QStringLiteral("Custom service and Control Point 2 found");
                    gattWriteChar2CustomService1 = c;
                    gattCustomService1 = s;
                } else if (c.properties() & QLowEnergyCharacteristic::Write &&
                           c.uuid() == _gattWriteChar3CustomService2) {
                    qDebug() << QStringLiteral("Custom service and Control Point 3 found");
                    gattWriteChar3CustomService2 = c;
                    gattCustomService2 = s;
                } else if (c.properties() & QLowEnergyCharacteristic::Write &&
                           c.uuid() == _gattWriteChar4CustomService2) {
                    qDebug() << QStringLiteral("Custom service and Control Point 4 found");
                    gattWriteChar4CustomService2 = c;
                    gattCustomService2 = s;
                } else if (c.uuid() == _gattWriteChar5CustomService3) {
                    qDebug() << QStringLiteral("Custom service and Control Point found");
                    gattWriteChar1CustomService1 = c;
                    gattCustomService1 = s;
                    lifet5 = true;
                    qDebug() << "Life Fitness T5 workaround enabled!";
                }
            }
        }
    }

    // ******************************************* virtual treadmill init *************************************
    if (!firstStateChanged && !this->hasVirtualDevice()
#ifdef Q_OS_IOS
#ifndef IO_UNDER_QT
        && !h
#endif
#endif
    ) {

        QSettings settings;
        bool virtual_device_enabled =
            settings.value(QZSettings::virtual_device_enabled, QZSettings::default_virtual_device_enabled).toBool();
        bool virtual_device_force_bike =
            settings.value(QZSettings::virtual_device_force_bike, QZSettings::default_virtual_device_force_bike)
                .toBool();
        if (virtual_device_enabled) {
            if (!virtual_device_force_bike) {
                debug("creating virtual treadmill interface...");
                auto virtualTreadmill = new virtualtreadmill(this, noHeartService);
                connect(virtualTreadmill, &virtualtreadmill::debug, this, &lifefitnesstreadmill::debug);
                connect(virtualTreadmill, &virtualtreadmill::changeInclination, this,
                        &lifefitnesstreadmill::changeInclinationRequested);
                this->setVirtualDevice(virtualTreadmill, VIRTUAL_DEVICE_MODE::PRIMARY);
            } else {
                debug("creating virtual bike interface...");
                auto virtualBike = new virtualbike(this);
                connect(virtualBike, &virtualbike::changeInclination, this,
                        &lifefitnesstreadmill::changeInclinationRequested);
                this->setVirtualDevice(virtualBike, VIRTUAL_DEVICE_MODE::ALTERNATIVE);
            }
        }
        firstStateChanged = 1;
        // ********************************************************************************************************
    }

    initRequest = true;
    emit connectedAndDiscovered();
}

void lifefitnesstreadmill::changeInclinationRequested(double grade, double percentage) {
    if (percentage < 0)
        percentage = 0;
    changeInclination(grade, percentage);
}

void lifefitnesstreadmill::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    emit debug(QStringLiteral("descriptorWritten ") + descriptor.name() + QStringLiteral(" ") + newValue.toHex(' '));
}

void lifefitnesstreadmill::descriptorRead(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    qDebug() << QStringLiteral("descriptorRead ") << descriptor.name() << descriptor.uuid() << newValue.toHex(' ');
}

void lifefitnesstreadmill::characteristicWritten(const QLowEnergyCharacteristic &characteristic,
                                                 const QByteArray &newValue) {
    Q_UNUSED(characteristic);
    emit debug(QStringLiteral("characteristicWritten ") + newValue.toHex(' '));
}

void lifefitnesstreadmill::characteristicRead(const QLowEnergyCharacteristic &characteristic,
                                              const QByteArray &newValue) {
    qDebug() << QStringLiteral("characteristicRead ") << characteristic.uuid() << newValue.toHex(' ');
}

void lifefitnesstreadmill::serviceScanDone(void) {
    emit debug(QStringLiteral("serviceScanDone"));

    initRequest = false;
    firstStateChanged = 0;
    auto services_list = m_control->services();
    for (const QBluetoothUuid &s : qAsConst(services_list)) {
        gattCommunicationChannelService.append(m_control->createServiceObject(s));
        connect(gattCommunicationChannelService.constLast(), &QLowEnergyService::stateChanged, this,
                &lifefitnesstreadmill::stateChanged);
        gattCommunicationChannelService.constLast()->discoverDetails();
    }
}

void lifefitnesstreadmill::errorService(QLowEnergyService::ServiceError err) {

    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    emit debug(QStringLiteral("lifefitnesstreadmill::errorService") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void lifefitnesstreadmill::error(QLowEnergyController::Error err) {

    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    emit debug(QStringLiteral("lifefitnesstreadmill::error") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void lifefitnesstreadmill::deviceDiscovered(const QBluetoothDeviceInfo &device) {

    // ***************************************************************************************************************
    // horizon treadmill and F80 treadmill, so if we want to add inclination support we have to separate the 2
    // devices
    // ***************************************************************************************************************
    emit debug(QStringLiteral("Found new device: ") + device.name() + QStringLiteral(" (") +
               device.address().toString() + ')');
    {
        bluetoothDevice = device;

        m_control = QLowEnergyController::createCentral(bluetoothDevice, this);
        connect(m_control, &QLowEnergyController::serviceDiscovered, this, &lifefitnesstreadmill::serviceDiscovered);
        connect(m_control, &QLowEnergyController::discoveryFinished, this, &lifefitnesstreadmill::serviceScanDone);
        connect(m_control,
                static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error),
                this, &lifefitnesstreadmill::error);
        connect(m_control, &QLowEnergyController::stateChanged, this, &lifefitnesstreadmill::controllerStateChanged);

        connect(m_control,
                static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error),
                this, [this](QLowEnergyController::Error error) {
                    Q_UNUSED(error);
                    Q_UNUSED(this);
                    emit debug(QStringLiteral("Cannot connect to remote device."));
                    emit disconnected();
                });
        connect(m_control, &QLowEnergyController::connected, this, [this]() {
            Q_UNUSED(this);
            emit debug(QStringLiteral("Controller connected. Search services..."));
            m_control->discoverServices();
        });
        connect(m_control, &QLowEnergyController::disconnected, this, [this]() {
            Q_UNUSED(this);
            emit debug(QStringLiteral("LowEnergy controller disconnected"));
            emit disconnected();
        });

        // Connect
        m_control->connectToDevice();
        return;
    }
}

bool lifefitnesstreadmill::connected() {
    if (!m_control) {

        return false;
    }
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

void lifefitnesstreadmill::controllerStateChanged(QLowEnergyController::ControllerState state) {
    qDebug() << QStringLiteral("controllerStateChanged") << state;
    if (state == QLowEnergyController::UnconnectedState && m_control) {
        qDebug() << QStringLiteral("trying to connect back again...");

        initDone = false;
        m_control->connectToDevice();
    }
}

double lifefitnesstreadmill::minStepInclination() { return 0.5; }
double lifefitnesstreadmill::minStepSpeed() { return 0.1; }
