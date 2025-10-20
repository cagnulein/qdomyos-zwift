#include "deerruntreadmill.h"
#include "virtualdevices/virtualbike.h"

#ifdef Q_OS_ANDROID
#include "keepawakehelper.h"
#endif
#include "virtualdevices/virtualtreadmill.h"
#include <QBluetoothLocalDevice>
#include <QDateTime>
#include <QFile>
#include <QMetaEnum>
#include <QSettings>
#include <chrono>

using namespace std::chrono_literals;

deerruntreadmill::deerruntreadmill(uint32_t pollDeviceTime, bool noConsole, bool noHeartService, double forceInitSpeed,
                                   double forceInitInclination) {
    m_watt.setType(metric::METRIC_WATT, deviceType());
    Speed.setType(metric::METRIC_SPEED);
    this->noConsole = noConsole;
    this->noHeartService = noHeartService;

    if (forceInitSpeed > 0) {
        lastSpeed = forceInitSpeed;
    }

    if (forceInitInclination > 0) {
        lastInclination = forceInitInclination;
    }

    refresh = new QTimer(this);
    initDone = false;
    connect(refresh, &QTimer::timeout, this, &deerruntreadmill::update);
    refresh->start(pollDeviceTime);
}

void deerruntreadmill::writeCharacteristic(const QLowEnergyCharacteristic characteristic, uint8_t *data,
                                           uint8_t data_len, const QString &info, bool disable_log,
                                           bool wait_for_response) {
    QEventLoop loop;
    QTimer timeout;

    if (wait_for_response) {
        connect(this, &deerruntreadmill::packetReceived, &loop, &QEventLoop::quit);
        timeout.singleShot(300ms, &loop, &QEventLoop::quit);
    } else {
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicWritten, &loop, &QEventLoop::quit);
        timeout.singleShot(300ms, &loop, &QEventLoop::quit);
    }

    if (gattCommunicationChannelService->state() != QLowEnergyService::ServiceState::ServiceDiscovered ||
        m_control->state() == QLowEnergyController::UnconnectedState) {
        emit debug(QStringLiteral("writeCharacteristic error because the connection is closed"));

        return;
    }

    if (writeBuffer) {
        delete writeBuffer;
    }
    writeBuffer = new QByteArray((const char *)data, data_len);

    gattCommunicationChannelService->writeCharacteristic(characteristic, *writeBuffer);

    if (!disable_log) {
        emit debug(QStringLiteral(" >> ") + writeBuffer->toHex(' ') +
                   QStringLiteral(" // ") + info);
    }

    loop.exec();

    if (timeout.isActive() == false) {
        emit debug(QStringLiteral(" exit for timeout"));
    }
}

void deerruntreadmill::waitForAPacket() {
    QEventLoop loop;
    QTimer timeout;
    connect(this, &deerruntreadmill::packetReceived, &loop, &QEventLoop::quit);
    timeout.singleShot(3000, &loop, SLOT(quit()));
    loop.exec();
}

void deerruntreadmill::writeUnlockCharacteristic(uint8_t *data, uint8_t data_len, const QString &info, bool disable_log) {
    QEventLoop loop;
    QTimer timeout;

    connect(unlock_service, &QLowEnergyService::characteristicWritten, &loop, &QEventLoop::quit);
    timeout.singleShot(300ms, &loop, &QEventLoop::quit);

    if (unlock_service->state() != QLowEnergyService::ServiceState::ServiceDiscovered ||
        m_control->state() == QLowEnergyController::UnconnectedState) {
        emit debug(QStringLiteral("writeUnlockCharacteristic error because the connection is closed"));
        return;
    }

    if (writeBuffer) {
        delete writeBuffer;
    }
    writeBuffer = new QByteArray((const char *)data, data_len);

    unlock_service->writeCharacteristic(unlock_characteristic, *writeBuffer);

    if (!disable_log) {
        emit debug(QStringLiteral(" >> unlock ") + writeBuffer->toHex(' ') +
                   QStringLiteral(" // ") + info);
    }

    loop.exec();

    if (timeout.isActive() == false) {
        emit debug(QStringLiteral(" exit for timeout"));
    }
}

uint8_t deerruntreadmill::calculateXOR(uint8_t arr[], size_t size) {
    uint8_t result = 0;

    if (size < 7) {
        qDebug() << QStringLiteral("array too small");
        return 0;
    }

    for (size_t i = 5; i <= size - 3; i++) {
        result ^= arr[i];
    }

    return result;
}

uint8_t deerruntreadmill::calculatePitPatChecksum(uint8_t arr[], size_t size) {
    uint8_t result = 0;

    if (size < 5) {
        qDebug() << QStringLiteral("array too small for PitPat checksum");
        return 0;
    }

    // For PitPat protocol:
    // 1. XOR from byte 5 to byte (size - 3) for long messages (>= 7 bytes)
    //    or from byte 2 to byte (size - 3) for short messages (< 7 bytes)
    // 2. XOR the result with byte 1
    size_t startIdx = (size < 7) ? 2 : 5;

    for (size_t i = startIdx; i <= size - 3; i++) {
        result ^= arr[i];
    }

    // XOR with byte 1 (command byte)
    result ^= arr[1];

    return result;
}


void deerruntreadmill::forceSpeed(double requestSpeed) {
    QSettings settings;

    if (pitpat) {
        // PitPat speed template
        // Pattern: 6a 17 00 00 00 00 [speed_high] [speed_low] 01 00 8a 00 04 00 00 00 00 00 12 2e 0c [checksum] 43
        // Speed encoding: speed value * 1000 (e.g., 2.0 km/h = 2000 = 0x07d0)
        uint8_t writeSpeed[] = {0x6a, 0x17, 0x00, 0x00, 0x00, 0x00, 0x07, 0x6c, 0x01, 0x00, 0x8a, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x12, 0x2e, 0x0c, 0xc3, 0x43};

        uint16_t speed = (uint16_t)(requestSpeed * 1000.0);
        writeSpeed[6] = (speed >> 8) & 0xFF;  // High byte
        writeSpeed[7] = speed & 0xFF;          // Low byte
        writeSpeed[21] = calculatePitPatChecksum(writeSpeed, sizeof(writeSpeed));  // Checksum at byte 21

        writeCharacteristic(gattWriteCharacteristic, writeSpeed, sizeof(writeSpeed),
                            QStringLiteral("forceSpeed PitPat speed=") + QString::number(requestSpeed), false, true);
    } else if (superun_ba04) {
        // Superun BA04 speed template
        uint8_t writeSpeed[] = {0x4d, 0x00, 0x14, 0x17, 0x6a, 0x17, 0x00, 0x00, 0x00, 0x00, 0x04, 0x4c, 0x01, 0x00, 0x50, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0xb5, 0x7c, 0xdb, 0x43};

        writeSpeed[2] = pollCounter;
        writeSpeed[10] = ((int)((requestSpeed * 100)) >> 8) & 0xFF;
        writeSpeed[11] = ((int)((requestSpeed * 100))) & 0xFF;
        writeSpeed[25] = calculateXOR(writeSpeed, sizeof(writeSpeed));

        writeCharacteristic(gattWriteCharacteristic, writeSpeed, sizeof(writeSpeed),
                            QStringLiteral("forceSpeed BA04 speed=") + QString::number(requestSpeed), false, false);
    } else {
        // Default speed template
        uint8_t writeSpeed[] = {0x4d, 0x00, 0xc9, 0x17, 0x6a, 0x17, 0x02, 0x00, 0x06, 0x40, 0x04, 0x4c, 0x01, 0x00, 0x50, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0a, 0x85, 0x11, 0xd8, 0x43};

        writeSpeed[2] = pollCounter;
        writeSpeed[10] = ((int)((requestSpeed * 100)) >> 8) & 0xFF;
        writeSpeed[11] = ((int)((requestSpeed * 100))) & 0xFF;
        writeSpeed[25] = calculateXOR(writeSpeed, sizeof(writeSpeed));

        writeCharacteristic(gattWriteCharacteristic, writeSpeed, sizeof(writeSpeed),
                            QStringLiteral("forceSpeed speed=") + QString::number(requestSpeed), false, false);
    }
}

void deerruntreadmill::forceIncline(double requestIncline) {

}

void deerruntreadmill::changeInclinationRequested(double grade, double percentage) {
    if (percentage < 0)
        percentage = 0;
    changeInclination(grade, percentage);
}

void deerruntreadmill::update() {
    if (m_control->state() == QLowEnergyController::UnconnectedState) {
        emit disconnected();
        return;
    }

    if (initRequest) {

        initRequest = false;
        btinit((lastSpeed > 0 ? true : false));
    } else if (/*bluetoothDevice.isValid() &&*/
               m_control->state() == QLowEnergyController::DiscoveredState && gattCommunicationChannelService &&
               gattWriteCharacteristic.isValid() && gattNotifyCharacteristic.isValid() && initDone) {

        QSettings settings;
        // ******************************************* virtual treadmill init *************************************
        if (!firstInit && !this->hasVirtualDevice()) {
            bool virtual_device_enabled =
                settings.value(QZSettings::virtual_device_enabled, QZSettings::default_virtual_device_enabled).toBool();
            bool virtual_device_force_bike =
                settings.value(QZSettings::virtual_device_force_bike, QZSettings::default_virtual_device_force_bike)
                    .toBool();
            if (virtual_device_enabled) {
                if (!virtual_device_force_bike) {
                    debug("creating virtual treadmill interface...");
                    auto virtualTreadMill = new virtualtreadmill(this, noHeartService);
                    connect(virtualTreadMill, &virtualtreadmill::debug, this, &deerruntreadmill::debug);
                    connect(virtualTreadMill, &virtualtreadmill::changeInclination, this,
                            &deerruntreadmill::changeInclinationRequested);
                    this->setVirtualDevice(virtualTreadMill, VIRTUAL_DEVICE_MODE::PRIMARY);
                } else {
                    debug("creating virtual bike interface...");
                    auto virtualBike = new virtualbike(this);
                    connect(virtualBike, &virtualbike::changeInclination, this,
                            &deerruntreadmill::changeInclinationRequested);
                    this->setVirtualDevice(virtualBike, VIRTUAL_DEVICE_MODE::ALTERNATIVE);
                }
                firstInit = 1;
            }
        }
        // ********************************************************************************************************

        // debug("Domyos Treadmill RSSI " + QString::number(bluetoothDevice.rssi()));

        update_metrics(true, watts(settings.value(QZSettings::weight, QZSettings::default_weight).toFloat()));

        {
            if (requestSpeed != -1) {
                if (requestSpeed != currentSpeed().value() && requestSpeed >= 0 && requestSpeed <= 22) {
                    emit debug(QStringLiteral("writing speed ") + QString::number(requestSpeed));
                    forceSpeed(requestSpeed);
                }
                requestSpeed = -1;
            } else if (requestInclination != -100) {
                if (requestInclination < 0)
                    requestInclination = 0;
                if (requestInclination != currentInclination().value() && requestInclination >= 0 &&
                    requestInclination <= 15) {
                    emit debug(QStringLiteral("writing incline ") + QString::number(requestInclination));
                    forceIncline(requestInclination);
                }
                requestInclination = -100;
            } else if (requestStart != -1) {
                emit debug(QStringLiteral("starting..."));
                if (lastSpeed == 0.0) {

                    lastSpeed = 0.5;
                }

                if (pitpat) {
                    forceSpeed(1.0);
                } else {
                    // should be:
                    // 0x49 = inited
                    // 0x8a = tape stopped after a pause
                    /*if (lastState == 0x49)*/ {
                        uint8_t initData2[] = {0x4d, 0x00, 0x0c, 0x17, 0x6a, 0x17, 0x02, 0x00, 0x06, 0x40, 0x03, 0xe8, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0a, 0x85, 0x11, 0x2a, 0x43};
                        initData2[2] = pollCounter;

                        writeCharacteristic(gattWriteCharacteristic, initData2, sizeof(initData2), QStringLiteral("start"),
                                            false, true);
                    } /*else {
                        uint8_t pause[] = {0x05, 0x00, 0x00, 0x00, 0x00, 0x2a, 0x07};

                        writeCharacteristic(gattWriteCharacteristic, pause, sizeof(pause), QStringLiteral("pause"), false,
                                            true);
                    }*/
                }

                requestStart = -1;
                emit tapeStarted();
            } else if (requestStop != -1) {
                emit debug(QStringLiteral("stopping... ") + paused);

                if (pitpat) {
                    uint8_t stop[] = {
                            0x6a, 0x17, 0x00, 0x00, 0x00,
                            0x00, 0x00, 0x00, 0x05, 0x00, 
                            0x8a, 0x00, 0x02, 0x00, 0x00, 
                            0x00, 0x00, 0x00, 0x12, 0x2e, 
                            0x0c, 0xaa, 0x43};
                    writeCharacteristic(gattWriteCharacteristic, initData2, sizeof(initData2), QStringLiteral("pitpat init 2"), false, true);
                } else {
                    uint8_t stop[] = {0x4d, 0x00, 0x48, 0x17, 0x6a, 0x17, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x00, 0x50, 0x00, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0a, 0x85, 0x11, 0xd6, 0x43};
                    stop[2] = pollCounter;

                    writeCharacteristic(gattWriteCharacteristic, stop, sizeof(stop), QStringLiteral("stop"), false,
                                        true);
                }

                requestStop = -1;
            } else {
                if (pitpat) {
                    uint8_t poll[] = {0x6a, 0x05, 0xfd, 0xf8, 0x43};
                    writeCharacteristic(gattWriteCharacteristic, poll, sizeof(poll), QStringLiteral("pitpat poll"), false, true);
                } else {
                    uint8_t poll[] = {0x4d, 0x00, 0x00, 0x05, 0x6a, 0x05, 0xfd, 0xf8, 0x43};
                    poll[2] = pollCounter;

                    writeCharacteristic(gattWriteCharacteristic, poll, sizeof(poll), QStringLiteral("poll"), false,
                                        true);
                }
            }

            pollCounter++;
            /*if (requestFanSpeed != -1) {
                emit debug(QStringLiteral("changing fan speed..."));

                sendChangeFanSpeed(requestFanSpeed);
                requestFanSpeed = -1;
            }
            if (requestIncreaseFan != -1) {
                emit debug(QStringLiteral("increasing fan speed..."));

                sendChangeFanSpeed(FanSpeed + 1);
                requestIncreaseFan = -1;
            } else if (requestDecreaseFan != -1) {
                emit debug(QStringLiteral("decreasing fan speed..."));

                sendChangeFanSpeed(FanSpeed - 1);
                requestDecreaseFan = -1;
            }*/
        }
    }
}

void deerruntreadmill::serviceDiscovered(const QBluetoothUuid &gatt) {
    emit debug(QStringLiteral("serviceDiscovered ") + gatt.toString());
}

void deerruntreadmill::characteristicChanged(const QLowEnergyCharacteristic &characteristic,
                                             const QByteArray &newValue) {
    // qDebug() << "characteristicChanged" << characteristic.uuid() << newValue << newValue.length();
    QSettings settings;
    QString heartRateBeltName =
        settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name).toString();
    Q_UNUSED(characteristic);
    QByteArray value = newValue;
    QDateTime now = QDateTime::currentDateTime();

    emit debug(QStringLiteral(" << ") + QString::number(value.length()) + QStringLiteral(" ") + value.toHex(' '));
    emit packetReceived();

    if ((newValue.length() < 51 && !pitpat) || (newValue.length() < 50 && pitpat))
        return;

    lastPacket = value;
    // lastState = value.at(0);

    double speed = ((double)(((value[9] << 8) & 0xff) + value[10]) / 100.0);
    if(pitpat) {
        speed = ((double)((value[3] << 8) | ((uint8_t)value[4])) / 1000.0);
    }
    double incline = 0.0;

#ifdef Q_OS_ANDROID
    if (settings.value(QZSettings::ant_heart, QZSettings::default_ant_heart).toBool())
        Heart = (uint8_t)KeepAwakeHelper::heart();
    else
#endif
    {
        if (heartRateBeltName.startsWith(QStringLiteral("Disabled"))) {

            uint8_t heart = 0;
            if (heart == 0) {
                update_hr_from_external();
            } else

                Heart = heart;
        }
    }

    if (!firstCharacteristicChanged) {
        if (watts(settings.value(QZSettings::weight, QZSettings::default_weight).toFloat()))
            KCal +=
                ((((0.048 * ((double)watts(settings.value(QZSettings::weight, QZSettings::default_weight).toFloat())) +
                    1.19) *
                   settings.value(QZSettings::weight, QZSettings::default_weight).toFloat() * 3.5) /
                  200.0) /
                 (60000.0 / ((double)lastTimeCharacteristicChanged.msecsTo(
                                now)))); //(( (0.048* Output in watts +1.19) * body weight in
                                                                  // kg * 3.5) / 200 ) / 60

        Distance += ((speed / (double)3600.0) /
                     ((double)1000.0 / (double)(lastTimeCharacteristicChanged.msecsTo(now))));
        lastTimeCharacteristicChanged = now;
    }

    emit debug(QStringLiteral("Current speed: ") + QString::number(speed));
    emit debug(QStringLiteral("Current incline: ") + QString::number(incline));
    emit debug(QStringLiteral("Current heart: ") + QString::number(Heart.value()));
    // emit debug(QStringLiteral("Current KCal: ") + QString::number(kcal));
    // emit debug(QStringLiteral("Current Distance: ") + QString::number(distance));
    emit debug(QStringLiteral("Current Distance Calculated: ") + QString::number(Distance.value()));

    if (m_control->error() != QLowEnergyController::NoError) {
        qDebug() << QStringLiteral("QLowEnergyController ERROR!!") << m_control->errorString();
    }

    if (Speed.value() != speed) {

        emit speedChanged(speed);
    }
    Speed = speed;
    if (Inclination.value() != incline) {

        emit inclinationChanged(0, incline);
    }
    Inclination = incline;

    emit debug(QStringLiteral("Current KCal: ") + QString::number(KCal.value()));

    emit debug(QStringLiteral("Current Distance: ") + QString::number(Distance.value()));

    if (speed > 0) {

        lastSpeed = speed;
        lastInclination = incline;
    }

    firstCharacteristicChanged = false;
}

void deerruntreadmill::btinit(bool startTape) {
    if (pitpat) {
        // PitPat treadmill initialization sequence
        uint8_t initData1[] = {0x6a, 0x05, 0xfd, 0xf8, 0x43};
        writeCharacteristic(gattWriteCharacteristic, initData1, sizeof(initData1), QStringLiteral("pitpat init 1"), false, true);

        uint8_t unlockData[] = {0x6b, 0x05, 0x9d, 0x98, 0x43};
        writeUnlockCharacteristic(unlockData, sizeof(unlockData), QStringLiteral("pitpat unlock"), false);

        uint8_t initData2[] = {0x6a, 0x05, 0xd7, 0xd2, 0x43};
        writeCharacteristic(gattWriteCharacteristic, initData2, sizeof(initData2), QStringLiteral("pitpat init 2"), false, true);

        uint8_t startData[] = {0x6a, 0x17, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x00, 0x81, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x93, 0x43};
        writeCharacteristic(gattWriteCharacteristic, startData, sizeof(startData), QStringLiteral("pitpat start"), false, true);
    } else if (superun_ba04) {
        // Superun BA04 treadmill initialization sequence
        // Wait for initial packet from treadmill before sending init
        emit debug(QStringLiteral("BA04: waiting for initial packet..."));
        waitForAPacket();

        // Init 1: pollCounter = 0
        uint8_t initData1[] = {0x4d, 0x00, 0x00, 0x05, 0x6a, 0x05, 0xfd, 0xf8, 0x43};
        initData1[2] = 0;  // pollCounter = 0
        writeCharacteristic(gattWriteCharacteristic, initData1, sizeof(initData1), QStringLiteral("BA04 init 1"), false, true);

        uint8_t initData2[] = {0x4d, 0x00, 0x00, 0x05, 0x6a, 0x05, 0xfd, 0xf8, 0x43};
        initData1[2] = 1;  // pollCounter = 0
        writeCharacteristic(gattWriteCharacteristic, initData2, sizeof(initData2), QStringLiteral("BA04 init 2"), false, true);

        // Init 2: pollCounter = 1
        uint8_t initData3[] = {0x4d, 0x00, 0x01, 0x17, 0x6a, 0x17, 0x00, 0x00, 0x00, 0x00, 0x03, 0xe8, 0x05, 0x00, 0x50, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0xb5, 0x7c, 0x7c, 0x43};
        initData3[2] = 2;  // pollCounter = 1
        writeCharacteristic(gattWriteCharacteristic, initData3, sizeof(initData3), QStringLiteral("BA04 init 3"), false, true);

        // Start pollCounter from 2 after init
        pollCounter = 3;
    }
    initDone = true;
}

double deerruntreadmill::minStepInclination() { return 1.0; }

void deerruntreadmill::stateChanged(QLowEnergyService::ServiceState state) {

    QBluetoothUuid _gattWriteCharacteristicId((quint16)0xfff1);
    QBluetoothUuid _gattNotifyCharacteristicId((quint16)0xfff2);
    QBluetoothUuid _pitpatWriteCharacteristicId((quint16)0xfba1);
    QBluetoothUuid _pitpatNotifyCharacteristicId((quint16)0xfba2);
    QBluetoothUuid _superunWriteCharacteristicId((quint16)0xff01);
    QBluetoothUuid _superunNotifyCharacteristicId((quint16)0xff02);
    QBluetoothUuid _unlockCharacteristicId((quint16)0x2b2a);

    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceState>();
    emit debug(QStringLiteral("BTLE stateChanged ") + QString::fromLocal8Bit(metaEnum.valueToKey(state)));
    if (state == QLowEnergyService::ServiceDiscovered) {

        QLowEnergyService* service = qobject_cast<QLowEnergyService*>(sender());
        if (service == unlock_service && pitpat) {
            // Handle unlock service characteristics
            auto characteristics_list = unlock_service->characteristics();
            for (const QLowEnergyCharacteristic &c : qAsConst(characteristics_list)) {
                qDebug() << QStringLiteral("unlock char uuid") << c.uuid() << QStringLiteral("handle") << c.handle()
                         << c.properties();
            }

            unlock_characteristic = unlock_service->characteristic(_unlockCharacteristicId);
            if (unlock_characteristic.isValid()) {
                emit debug(QStringLiteral("unlock characteristic found"));
            }
            return;
        }

        // qDebug() << gattCommunicationChannelService->characteristics();
        auto characteristics_list = gattCommunicationChannelService->characteristics();
        for (const QLowEnergyCharacteristic &c : qAsConst(characteristics_list)) {
            qDebug() << QStringLiteral("char uuid") << c.uuid() << QStringLiteral("handle") << c.handle()
                     << c.properties();
        }

        if (pitpat) {
            gattWriteCharacteristic = gattCommunicationChannelService->characteristic(_pitpatWriteCharacteristicId);
            gattNotifyCharacteristic = gattCommunicationChannelService->characteristic(_pitpatNotifyCharacteristicId);
        } else if (superun_ba04) {
            gattWriteCharacteristic = gattCommunicationChannelService->characteristic(_superunWriteCharacteristicId);
            gattNotifyCharacteristic = gattCommunicationChannelService->characteristic(_superunNotifyCharacteristicId);
        } else {
            gattWriteCharacteristic = gattCommunicationChannelService->characteristic(_gattWriteCharacteristicId);
            gattNotifyCharacteristic = gattCommunicationChannelService->characteristic(_gattNotifyCharacteristicId);
        }
        
        Q_ASSERT(gattWriteCharacteristic.isValid());
        Q_ASSERT(gattNotifyCharacteristic.isValid());

        // establish hook into notifications
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicChanged, this,
                &deerruntreadmill::characteristicChanged);
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicWritten, this,
                &deerruntreadmill::characteristicWritten);
        connect(gattCommunicationChannelService,
                static_cast<void (QLowEnergyService::*)(QLowEnergyService::ServiceError)>(&QLowEnergyService::error),
                this, &deerruntreadmill::errorService);
        connect(gattCommunicationChannelService, &QLowEnergyService::descriptorWritten, this,
                &deerruntreadmill::descriptorWritten);

        QByteArray descriptor;
        descriptor.append((char)0x01);
        descriptor.append((char)0x00);
        gattCommunicationChannelService->writeDescriptor(
            gattNotifyCharacteristic.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration), descriptor);
    }
}

void deerruntreadmill::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    emit debug(QStringLiteral("descriptorWritten ") + descriptor.name() + QStringLiteral(" ") + newValue.toHex(' '));

    initRequest = true;
    emit connectedAndDiscovered();
}

void deerruntreadmill::characteristicWritten(const QLowEnergyCharacteristic &characteristic,
                                             const QByteArray &newValue) {
    Q_UNUSED(characteristic);
    emit debug(QStringLiteral("characteristicWritten ") + newValue.toHex(' '));
}

void deerruntreadmill::serviceScanDone(void) {
    QBluetoothUuid _gattCommunicationChannelServiceId((quint16)0xfff0);
    QBluetoothUuid _pitpatServiceId((quint16)0xfba0);
    QBluetoothUuid _superunServiceId((quint16)0xffff);
    QBluetoothUuid _unlockServiceId((quint16)0x1801);
    emit debug(QStringLiteral("serviceScanDone"));

    auto services_list = m_control->services();
    emit debug("Services found:");
    for (const QBluetoothUuid &s : qAsConst(services_list)) {
        emit debug(s.toString());
    }

    // Check if this is a pitpat treadmill by looking for the 0xfba0 service
    if (services_list.contains(_pitpatServiceId)) {
        pitpat = true;
        emit debug(QStringLiteral("Detected pitpat treadmill variant"));
        gattCommunicationChannelService = m_control->createServiceObject(_pitpatServiceId);
        unlock_service = m_control->createServiceObject(_unlockServiceId);
    } else if (services_list.contains(_superunServiceId)) {
        superun_ba04 = true;
        pitpat = false;
        emit debug(QStringLiteral("Detected Superun BA04 treadmill variant"));
        gattCommunicationChannelService = m_control->createServiceObject(_superunServiceId);
    } else {
        pitpat = false;
        gattCommunicationChannelService = m_control->createServiceObject(_gattCommunicationChannelServiceId);
    }
    
    if (gattCommunicationChannelService) {
        connect(gattCommunicationChannelService, &QLowEnergyService::stateChanged, this,
                &deerruntreadmill::stateChanged);
        gattCommunicationChannelService->discoverDetails();
    } else {
        emit debug(QStringLiteral("error on find Service"));
    }
    
    if (pitpat && unlock_service) {
        connect(unlock_service, &QLowEnergyService::stateChanged, this,
                &deerruntreadmill::stateChanged);
        unlock_service->discoverDetails();
    }
}

void deerruntreadmill::errorService(QLowEnergyService::ServiceError err) {

    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    emit debug(QStringLiteral("deerruntreadmill::errorService ") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void deerruntreadmill::error(QLowEnergyController::Error err) {

    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    emit debug(QStringLiteral("deerruntreadmill::error ") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void deerruntreadmill::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    {

        bluetoothDevice = device;
        m_control = QLowEnergyController::createCentral(bluetoothDevice, this);
        connect(m_control, &QLowEnergyController::serviceDiscovered, this, &deerruntreadmill::serviceDiscovered);
        connect(m_control, &QLowEnergyController::discoveryFinished, this, &deerruntreadmill::serviceScanDone);
        connect(m_control,
                static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error),
                this, &deerruntreadmill::error);
        connect(m_control, &QLowEnergyController::stateChanged, this, &deerruntreadmill::controllerStateChanged);

        connect(m_control,
                static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error),
                this, [this](QLowEnergyController::Error error) {
                    Q_UNUSED(error);
                    Q_UNUSED(this);
                    emit debug(QStringLiteral("Cannot connect to remote device."));
                    searchStopped = false;
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
            searchStopped = false;
            emit disconnected();
        });

        // Connect
        m_control->connectToDevice();
        return;
    }
}

void deerruntreadmill::controllerStateChanged(QLowEnergyController::ControllerState state) {
    qDebug() << QStringLiteral("controllerStateChanged") << state;
    if (state == QLowEnergyController::UnconnectedState && m_control) {
        qDebug() << QStringLiteral("trying to connect back again...");

        initDone = false;
        m_control->connectToDevice();
    }
}

bool deerruntreadmill::connected() {
    if (!m_control) {

        return false;
    }
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

void deerruntreadmill::searchingStop() { searchStopped = true; }
