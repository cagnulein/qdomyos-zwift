#include "domyostreadmill.h"
#include "keepawakehelper.h"
#include "virtualdevices/virtualbike.h"
#include "virtualdevices/virtualtreadmill.h"
#include <QBluetoothLocalDevice>
#include <QDateTime>
#include <QFile>
#include <QMetaEnum>
#include <QSettings>
#include <chrono>

using namespace std::chrono_literals;

// set speed and incline to 0
uint8_t initData1[] = {0xf0, 0xc8, 0x01, 0xb9};
uint8_t initData2[] = {0xf0, 0xc9, 0xb9};

uint8_t noOpData[] = {0xf0, 0xac, 0x9c};

// stop tape
uint8_t initDataF0C800B8[] = {0xf0, 0xc8, 0x00, 0xb8};

// main startup sequence
uint8_t initDataStart[] = {0xf0, 0xa3, 0x93};
uint8_t initDataStart2[] = {0xf0, 0xa4, 0x94};
uint8_t initDataStart3[] = {0xf0, 0xa5, 0x95};
uint8_t initDataStart4[] = {0xf0, 0xab, 0x9b};
uint8_t initDataStart5[] = {0xf0, 0xc4, 0x03, 0xb7};
uint8_t initDataStart6[] = {0xf0, 0xad, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                            0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x01, 0xff};

uint8_t initDataStart7[] = {0xff, 0xff, 0x8b}; // power on bt icon
uint8_t initDataStart8[] = {0xf0, 0xcb, 0x02, 0x00, 0x08, 0xff, 0xff, 0xff, 0xff, 0xff,
                            0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x01, 0x00};

uint8_t initDataStart9[] = {0x00, 0x01, 0xff, 0xff, 0xff, 0xff, 0xb6}; // power on bt word
uint8_t initDataStart10[] = {0xf0, 0xad, 0xff, 0xff, 0x00, 0x05, 0xff, 0xff, 0xff, 0xff,
                             0xff, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0xff, 0x01, 0xff};

uint8_t initDataStart11[] = {0xff, 0xff, 0x94}; // start tape
uint8_t initDataStart12[] = {0xf0, 0xcb, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                             0xff, 0xff, 0xff, 0xff, 0x01, 0x00, 0x14, 0x01, 0xff, 0xff};

uint8_t initDataStart13[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xbd};

QBluetoothUuid _gattCommunicationChannelServiceId(QStringLiteral("49535343-fe7d-4ae5-8fa9-9fafd205e455"));
QBluetoothUuid _gattWriteCharacteristicId(QStringLiteral("49535343-8841-43f4-a8d4-ecbe34729bb3"));
QBluetoothUuid _gattNotifyCharacteristicId(QStringLiteral("49535343-1e4d-4bd9-ba61-23c647249616"));

#ifdef Q_OS_IOS
extern quint8 QZ_EnableDiscoveryCharsAndDescripttors;
#endif

domyostreadmill::domyostreadmill(uint32_t pollDeviceTime, bool noConsole, bool noHeartService, double forceInitSpeed,
                                 double forceInitInclination) {
#ifdef Q_OS_IOS
    QZ_EnableDiscoveryCharsAndDescripttors = true;
#endif
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
    connect(refresh, &QTimer::timeout, this, &domyostreadmill::update);
    refresh->start(pollDeviceTime);
}

void domyostreadmill::writeCharacteristic(uint8_t *data, uint8_t data_len, const QString &info, bool disable_log,
                                          bool wait_for_response) {
    QEventLoop loop;
    QTimer timeout;

    if (wait_for_response) {
        connect(this, &domyostreadmill::packetReceived, &loop, &QEventLoop::quit);
        timeout.singleShot(300ms, &loop, &QEventLoop::quit);
    } else {
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicWritten, &loop, &QEventLoop::quit);
        timeout.singleShot(300ms, &loop, &QEventLoop::quit);
    }

    if (gattCommunicationChannelService->state() != QLowEnergyService::ServiceState::ServiceDiscovered ||
        m_control->state() == QLowEnergyController::UnconnectedState) {
        qDebug() << QStringLiteral("writeCharacteristic error because the connection is closed");

        return;
    }

    if (!gattWriteCharacteristic.isValid()) {
        qDebug() << QStringLiteral("gattWriteCharacteristic is invalid");
        return;
    }

    if (writeBuffer) {
        delete writeBuffer;
    }
    writeBuffer = new QByteArray((const char *)data, data_len);

    gattCommunicationChannelService->writeCharacteristic(gattWriteCharacteristic, *writeBuffer);

    if (!disable_log) {
        qDebug() << QStringLiteral(" >> ") + writeBuffer->toHex(' ')
                 << QStringLiteral(" // ") + info;
    }

    loop.exec();

    if (timeout.isActive() == false) {
        qDebug() << QStringLiteral(" exit for timeout");
    }
}

void domyostreadmill::updateDisplay(uint16_t elapsed) {
    uint8_t display[] = {0xf0, 0xcb, 0x03, 0x00, 0x00, 0xff, 0x01, 0x00, 0x00, 0x02, 0x01, 0x00, 0x00, 0x00,
                         0x01, 0x00, 0x05, 0x01, 0x01, 0x00, 0x0c, 0x01, 0x01, 0x00, 0x00, 0x01, 0x00};

    QSettings settings;
    bool distance =
        settings
            .value(QZSettings::domyos_treadmill_distance_display, QZSettings::default_domyos_treadmill_distance_display)
            .toBool();
    bool domyos_treadmill_display_invert =
        settings.value(QZSettings::domyos_treadmill_display_invert, QZSettings::default_domyos_treadmill_display_invert)
            .toBool();

    if (elapsed > 5999) // 99:59
    {
        display[3] = ((elapsed / 60) / 60) & 0xFF; // high byte for elapsed time (in seconds)
        display[4] = ((elapsed / 60) % 60) & 0xFF; // low byte for elapsed time (in seconds)
    } else {

        display[3] = (elapsed / 60) & 0xFF; // high byte for elapsed time (in seconds)
        display[4] = (elapsed % 60 & 0xFF); // low byte for elapsed time (in seconds)
    }

    if (distance) {
        if (!domyos_treadmill_display_invert) {
            if (odometer() < 10.0) {

                display[7] = ((uint8_t)((uint16_t)(odometer() * 100) >> 8)) & 0xFF;
                display[8] = (uint8_t)(odometer() * 100) & 0xFF;
                display[9] = 0x02; // decimal position
            } else if (odometer() < 100.0) {

                display[7] = ((uint8_t)(odometer() * 10) >> 8) & 0xFF;
                display[8] = (uint8_t)(odometer() * 10) & 0xFF;
                display[9] = 0x01; // decimal position
            } else {

                display[7] = ((uint8_t)(odometer()) >> 8) & 0xFF;
                display[8] = (uint8_t)(odometer()) & 0xFF;
                display[9] = 0x00; // decimal position
            }
        } else {
            display[7] = ((uint16_t)(calories().value()) >> 8) & 0xFF;
            display[8] = (uint8_t)(calories().value()) & 0xFF;
            display[9] = 0x00; // decimal position
        }
    } else {

        display[7] = 0x00;
        display[8] = 0x00;
        display[9] = 0x00; // decimal position
    }

    display[12] = (uint8_t)currentHeart().value();

    display[16] = (uint8_t)(currentInclination().value() * 10.0);

    display[20] = (uint8_t)(currentSpeed().value() * 10.0);

    if (!domyos_treadmill_display_invert) {
        display[23] = ((uint8_t)(calories().value()) >> 8) & 0xFF;
        display[24] = (uint8_t)(calories().value()) & 0xFF;
    } else {
        display[23] = ((uint8_t)(odometer() * 10) >> 8) & 0xFF;
        display[24] = (uint8_t)(odometer() * 10) & 0xFF;
    }

    for (uint8_t i = 0; i < sizeof(display) - 1; i++) {

        display[26] += display[i]; // the last byte is a sort of a checksum
    }

    writeCharacteristic(display, 20, QStringLiteral("updateDisplay elapsed=") + QString::number(elapsed), false, false);
    writeCharacteristic(&display[20], sizeof(display) - 20,
                        QStringLiteral("updateDisplay elapsed=") + QString::number(elapsed), false, true);
}

void domyostreadmill::forceSpeedOrIncline(double requestSpeed, double requestIncline) {
    uint8_t writeIncline[] = {0xf0, 0xad, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                              0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00};

    writeIncline[4] = ((uint16_t)(requestSpeed * 10) >> 8) & 0xFF;
    writeIncline[5] = ((uint16_t)(requestSpeed * 10) & 0xFF);

    writeIncline[13] = ((uint16_t)(requestIncline * 10) >> 8) & 0xFF;
    writeIncline[14] = ((uint16_t)(requestIncline * 10) & 0xFF);

    for (uint8_t i = 0; i < sizeof(writeIncline) - 1; i++) {

        // qDebug() << QString::number(writeIncline[i], 16);
        writeIncline[22] += writeIncline[i]; // the last byte is a sort of a checksum
    }

    // qDebug() << "writeIncline crc" << QString::number(writeIncline[26], 16);

    writeCharacteristic(writeIncline, 20,
                        QStringLiteral("forceSpeedOrIncline speed=") + QString::number(requestSpeed) +
                            " incline=" + QString::number(requestIncline),
                        false, false);
    writeCharacteristic(&writeIncline[20], sizeof(writeIncline) - 20,
                        QStringLiteral("forceSpeedOrIncline speed=") + QString::number(requestSpeed) +
                            QStringLiteral(" incline=") + QString::number(requestIncline),
                        false, true);
}

bool domyostreadmill::sendChangeFanSpeed(uint8_t speed) {

    uint8_t fanSpeed[] = {0xf0, 0xca, 0x00, 0x00};

    if (speed > 5) {
        return false;
    }

    fanSpeed[2] = speed;

    for (uint8_t i = 0; i < sizeof(fanSpeed) - 1; i++) {

        fanSpeed[3] += fanSpeed[i]; // the last byte is a sort of a checksum
    }

    writeCharacteristic(fanSpeed, 4, QStringLiteral("changeFanSpeed speed=") + QString::number(speed), false, true);

    return true;
}

bool domyostreadmill::changeFanSpeed(uint8_t speed) {

    requestFanSpeed = speed;

    return true;
}

void domyostreadmill::changeInclinationRequested(double grade, double percentage) {
    if (percentage < 0)
        percentage = 0;
    if (grade < 0)
        grade = 0;
    changeInclination(grade, percentage);
}

void domyostreadmill::update() {

    if (!m_control)
        return;

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
        if (!firstInit && searchStopped && !this->hasVirtualDevice()) {
            bool virtual_device_enabled =
                settings.value(QZSettings::virtual_device_enabled, QZSettings::default_virtual_device_enabled).toBool();
            bool virtual_device_force_bike =
                settings.value(QZSettings::virtual_device_force_bike, QZSettings::default_virtual_device_force_bike)
                    .toBool();
            if (virtual_device_enabled) {
                if (!virtual_device_force_bike) {
                    debug("creating virtual treadmill interface...");
                    auto virtualTreadMill = new virtualtreadmill(this, noHeartService);
                    connect(virtualTreadMill, &virtualtreadmill::debug, this, &domyostreadmill::debug);
                    connect(virtualTreadMill, &virtualtreadmill::changeInclination, this,
                            &domyostreadmill::changeInclinationRequested);
                    this->setVirtualDevice(virtualTreadMill, VIRTUAL_DEVICE_MODE::PRIMARY);
                } else {
                    debug("creating virtual bike interface...");
                    auto virtualBike = new virtualbike(this);
                    connect(virtualBike, &virtualbike::changeInclination, this,
                            &domyostreadmill::changeInclinationRequested);
                    this->setVirtualDevice(virtualBike, VIRTUAL_DEVICE_MODE::ALTERNATIVE);
                }
                firstInit = 1;
            }
        }
        // ********************************************************************************************************

        // debug("Domyos Treadmill RSSI " + QString::number(bluetoothDevice.rssi()));

        update_metrics(true, watts(settings.value(QZSettings::weight, QZSettings::default_weight).toFloat()));

        // updating the treadmill console every second
        if (sec1Update++ >= (1000 / refresh->interval())) {
            if (incompletePackets == false && noConsole == false) {

                sec1Update = 0;
                updateDisplay(elapsed.value());
            }
        } else {
            if (incompletePackets == false) {
                writeCharacteristic(noOpData, sizeof(noOpData), QStringLiteral("noOp"), false, true);
            }
        }

        // byte 3 - 4 = elapsed time
        // byte 17    = inclination
        double inclination_delay_seconds = settings.value(QZSettings::inclination_delay_seconds, QZSettings::default_inclination_delay_seconds).toDouble();
        if (incompletePackets == false) {
            if (requestSpeed != -1) {
                if (requestSpeed != currentSpeed().value() && requestSpeed >= 0 && requestSpeed <= 22) {
                    emit debug(QStringLiteral("writing speed ") + QString::number(requestSpeed));

                    double inc = Inclination.value();
                    if (requestInclination != -100 && lastInclinationChanged.secsTo(QDateTime::currentDateTime()) > inclination_delay_seconds) {
                        lastInclinationChanged = QDateTime::currentDateTime();
                        // only 0.5 steps ara available
                        requestInclination = treadmillInclinationOverrideReverse(requestInclination);
                        requestInclination = qRound(requestInclination * 2.0) / 2.0;
                        inc = requestInclination;
                        requestInclination = -100;
                    }
                    forceSpeedOrIncline(requestSpeed, inc);
                }
                requestSpeed = -1;
            }
            if (requestInclination != -100 && lastInclinationChanged.secsTo(QDateTime::currentDateTime()) > inclination_delay_seconds) {
                lastInclinationChanged = QDateTime::currentDateTime();
                if (requestInclination < 0)
                    requestInclination = 0;
                // only 0.5 steps ara available
                requestInclination = qRound(requestInclination * 2.0) / 2.0;
                if (requestInclination != currentInclination().value() && requestInclination >= 0 &&
                    requestInclination <= 20) {
                    emit debug(QStringLiteral("writing incline ") + QString::number(requestInclination));

                    double speed = currentSpeed().value();
                    if (requestSpeed != -1) {

                        speed = requestSpeed;
                        requestSpeed = -1;
                    }
                    forceSpeedOrIncline(speed, requestInclination);
                }
                requestInclination = -100;
            }
            if (requestStart != -1) {
                emit debug(QStringLiteral("starting..."));
                if (lastSpeed == 0.0) {

                    lastSpeed = 0.5;
                }
                btinit(true);
                requestStart = -1;
                emit tapeStarted();
            }
            if (requestStop != -1) {
                emit debug(QStringLiteral("stopping..."));
                writeCharacteristic(initDataF0C800B8, sizeof(initDataF0C800B8), QStringLiteral("stop tape"), false,
                                    true);
                requestStop = -1;
            }
            if (requestFanSpeed != -1) {
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
            }
        }
    }
}

void domyostreadmill::serviceDiscovered(const QBluetoothUuid &gatt) {
    emit debug(QStringLiteral("serviceDiscovered ") + gatt.toString());
}

void domyostreadmill::characteristicChanged(const QLowEnergyCharacteristic &characteristic,
                                            const QByteArray &newValue) {
    // qDebug() << "characteristicChanged" << characteristic.uuid() << newValue << newValue.length();
    QSettings settings;
    QString heartRateBeltName =
        settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name).toString();
    bool domyos_treadmill_buttons =
        settings.value(QZSettings::domyos_treadmill_buttons, QZSettings::default_domyos_treadmill_buttons).toBool();
    bool domyos_treadmill_t900a = settings.value(QZSettings::domyos_treadmill_t900a, QZSettings::default_domyos_treadmill_t900a).toBool();
    domyos_treadmill_sync_start = settings.value(QZSettings::domyos_treadmill_sync_start, QZSettings::default_domyos_treadmill_sync_start).toBool();
    Q_UNUSED(characteristic);
    QByteArray value = newValue;

    emit debug(QStringLiteral(" << ") + QString::number(value.length()) + QStringLiteral(" ") + value.toHex(' '));

    // for the init packets, the length is always less than 20
    // for the display and status packets, the length is always grater then 20 and there are 2 cases:
    // - intense run: it always send more than 20 bytes in one packets, so the length will be always != 20
    // - t900: it splits packets with length grater than 20 in two distinct packets, so the first one it has length of
    // 20,
    //         and the second one with the remained byte
    // so this simply condition will match all the cases, excluding the 20byte packet of the T900.
    if (newValue.length() != 20) {
        emit debug(QStringLiteral("packetReceived!"));

        emit packetReceived();
    }

    QByteArray startBytes;
    startBytes.append(0xf0);
    startBytes.append(0xbc);

    QByteArray startBytes2;
    startBytes2.append(0xf0);
    startBytes2.append(0xdb);

    // on some treadmills, the 26bytes has split in 2 packets
    if ((lastPacket.length() == 20 && lastPacket.startsWith(startBytes) && value.length() == 6) ||
        (lastPacket.length() == 20 && lastPacket.startsWith(startBytes2) && value.length() == 7)) {

        incompletePackets = false;
        emit debug(QStringLiteral("...final bytes received"));
        lastPacket.append(value);
        value = lastPacket;
    }

    lastPacket = value;

    if (value.length() != 26) {

        // semaphore for any writing packets (for example, update display)
        if (value.length() == 20 && (value.startsWith(startBytes) || value.startsWith(startBytes2))) {
            emit debug(QStringLiteral("waiting for other bytes..."));

            incompletePackets = true;
        }

        emit debug(QStringLiteral("packet ignored"));
        return;
    }

    if (value.at(22) == 0x06) {
        emit debug(QStringLiteral("start button pressed!"));

        requestStart = 1;
    } else if (value.at(22) == 0x07) {
        emit debug(QStringLiteral("stop button pressed!"));

        requestStop = 1;
    } else if (value.at(22) == 0x0b) {
        emit debug(QStringLiteral("increase speed fan pressed!"));

        requestIncreaseFan = 1;
    } else if (value.at(22) == 0x0a) {
        emit debug(QStringLiteral("decrease speed fan pressed!"));

        requestDecreaseFan = 1;
    } else if (value.at(22) == 0x08) {
        emit debug(QStringLiteral("increase speed button on console pressed!"));
        if (domyos_treadmill_buttons) {

            changeSpeed(currentSpeed().value() + 0.2);
        }
    } else if (value.at(22) == 0x09) {
        emit debug(QStringLiteral("decrease speed button on console pressed!"));
        if (domyos_treadmill_buttons) {

            changeSpeed(currentSpeed().value() - 0.2);
        }
    } else if (value.at(22) == 0x0c) {
        emit debug(QStringLiteral("increase inclination button on console pressed!"));
        if (domyos_treadmill_buttons) {

            changeInclination(currentInclination().value() + 0.5, currentInclination().value() + 0.5);
        }
    } else if (value.at(22) == 0x0d) {
        emit debug(QStringLiteral("decrease inclination button on console pressed!"));
        if (domyos_treadmill_buttons) {

            changeInclination(currentInclination().value() - 0.5, currentInclination().value() - 0.5);
        }
    } else if (value.at(22) == 0x11) {
        if (domyos_treadmill_buttons) {
            if(domyos_treadmill_t900a) {
                emit debug(QStringLiteral("10km/h speed button pressed!"));
                changeSpeed(10.0);
            } else {
                emit debug(QStringLiteral("22km/h speed button pressed!"));
                changeSpeed(settings.value(QZSettings::domyos_treadmill_button_22kmh, QZSettings::default_domyos_treadmill_button_22kmh).toDouble());
            }            
        }
    } else if (value.at(22) == 0x10) {
        if (domyos_treadmill_buttons) {
            if(domyos_treadmill_t900a) {
                emit debug(QStringLiteral("8km/h speed button pressed!"));
                changeSpeed(8.0);
            } else {
                emit debug(QStringLiteral("16km/h speed button pressed!"));
                changeSpeed(settings.value(QZSettings::domyos_treadmill_button_16kmh, QZSettings::default_domyos_treadmill_button_16kmh).toDouble());
            }            
        }
    } else if (value.at(22) == 0x0f) {
        if (domyos_treadmill_buttons) {
            if(domyos_treadmill_t900a) {
                emit debug(QStringLiteral("6km/h speed button pressed!"));
                changeSpeed(6.0);
            } else {
                emit debug(QStringLiteral("10km/h speed button pressed!"));
                changeSpeed(settings.value(QZSettings::domyos_treadmill_button_10kmh, QZSettings::default_domyos_treadmill_button_10kmh).toDouble());
            }            
        }
    } else if (value.at(22) == 0x0e) {        
        if (domyos_treadmill_buttons) {
            if(domyos_treadmill_t900a) {
                emit debug(QStringLiteral("4km/h speed button pressed!"));
                changeSpeed(4.0);
            } else {
                emit debug(QStringLiteral("5km/h speed button pressed!"));
                changeSpeed(settings.value(QZSettings::domyos_treadmill_button_5kmh, QZSettings::default_domyos_treadmill_button_5kmh).toDouble());
            }            
        }
    } else if (value.at(22) == 0x15) {
        if (domyos_treadmill_buttons) {
            if(domyos_treadmill_t900a) {
                emit debug(QStringLiteral("3% inclination button on console pressed!"));
                changeInclination(3.0, 3.0);
            } else {
                emit debug(QStringLiteral("15% inclination button on console pressed!"));
                changeInclination(15.0, 15.0);
            }            
        }        
    } else if (value.at(22) == 0x14) {
        if (domyos_treadmill_buttons) {
            if(domyos_treadmill_t900a) {
                emit debug(QStringLiteral("2% inclination button on console pressed!"));
                changeInclination(2.0, 2.0);
            } else {
                emit debug(QStringLiteral("10% inclination button on console pressed!"));
                changeInclination(10.0, 10.0);
            }            
        }        
    } else if (value.at(22) == 0x13) {
        if (domyos_treadmill_buttons) {
            if(domyos_treadmill_t900a) {
                emit debug(QStringLiteral("1% inclination button on console pressed!"));
                changeInclination(1.0, 1.0);
            } else {
                emit debug(QStringLiteral("5% inclination button on console pressed!"));
                changeInclination(5.0, 5.0);
            }            
        }        
    } else if (value.at(22) == 0x12) {
        if (domyos_treadmill_buttons) {
            if(domyos_treadmill_t900a) {
                emit debug(QStringLiteral("0% inclination button on console pressed!"));
                changeInclination(0.0, 0.0);
            } else {
                emit debug(QStringLiteral("0% inclination button on console pressed!"));
                changeInclination(0.0, 0.0);
            }            
        }        
    } else if (value.at(22) == 0x17) {
        if (domyos_treadmill_buttons) {
            if(domyos_treadmill_t900a) {
                emit debug(QStringLiteral("14km/h speed button pressed!"));
                changeSpeed(14.0);
            }            
        }        
    } else if (value.at(22) == 0x18) {
        if (domyos_treadmill_buttons) {
            if(domyos_treadmill_t900a) {
                emit debug(QStringLiteral("16km/h speed button on console pressed!"));
                changeSpeed(16.0);
            }            
        }        
    } else if (value.at(22) == 0x19) {
        if (domyos_treadmill_buttons) {
            if(domyos_treadmill_t900a) {
                emit debug(QStringLiteral("18km/h speed button pressed!"));
                changeSpeed(18.0);
            }            
        }        
    } else if (value.at(22) == 0x16) {
        if (domyos_treadmill_buttons) {
            if(domyos_treadmill_t900a) {
                emit debug(QStringLiteral("12km/h speed button pressed!"));
                changeSpeed(12.0);
            }            
        }        
    } else if (value.at(22) == 0x1a) {
        if (domyos_treadmill_buttons) {
            if(domyos_treadmill_t900a) {
                emit debug(QStringLiteral("4% inclination button on console pressed!"));
                changeInclination(4.0, 4.0);
            }            
        }        
    } else if (value.at(22) == 0x1b) {
        if (domyos_treadmill_buttons) {
            if(domyos_treadmill_t900a) {
                emit debug(QStringLiteral("6% inclination button on console pressed!"));
                changeInclination(6.0, 6.0);
            }            
        }        
    } else if (value.at(22) == 0x1c) {
        if (domyos_treadmill_buttons) {
            if(domyos_treadmill_t900a) {
                emit debug(QStringLiteral("8% inclination button on console pressed!"));
                changeInclination(8.0, 8.0);
            }            
        }        
    } else if (value.at(22) == 0x1d) {
        if (domyos_treadmill_buttons) {
            if(domyos_treadmill_t900a) {
                emit debug(QStringLiteral("10% inclination button on console pressed!"));
                changeInclination(10.0, 10.0);
            }            
        }        
    }

    /*if ((uint8_t)value.at(1) != 0xbc && value.at(2) != 0x04)  // intense run, these are the bytes for the inclination
       and speed status return;*/

    double speed = GetSpeedFromPacket(value);
    double incline = treadmillInclinationOverride(GetInclinationFromPacket(value));
    double kcal = GetKcalFromPacket(value);
    double distance = GetDistanceFromPacket(value);
    bool disable_hr_frommachinery =
        settings.value(QZSettings::heart_ignore_builtin, QZSettings::default_heart_ignore_builtin).toBool();

#ifdef Q_OS_ANDROID
    if (settings.value(QZSettings::ant_heart, QZSettings::default_ant_heart).toBool())
        Heart = (uint8_t)KeepAwakeHelper::heart();
    else
#endif
    {
        if (heartRateBeltName.startsWith(QStringLiteral("Disabled"))) {

            uint8_t heart = ((uint8_t)value.at(18));
            if (heart == 0 || disable_hr_frommachinery) {
                update_hr_from_external();
            } else {
                Heart = heart;
            }
        }
    }

    cadenceFromAppleWatch();

    FanSpeed = value.at(23);

    if (!firstCharacteristicChanged) {
        if (watts(settings.value(QZSettings::weight, QZSettings::default_weight).toFloat()))
            KCal +=
                ((((0.048 * ((double)watts(settings.value(QZSettings::weight, QZSettings::default_weight).toFloat())) +
                    1.19) *
                   settings.value(QZSettings::weight, QZSettings::default_weight).toFloat() * 3.5) /
                  200.0) /
                 (60000.0 / ((double)lastTimeCharacteristicChanged.msecsTo(
                                QDateTime::currentDateTime())))); //(( (0.048* Output in watts +1.19) * body weight in
                                                                  // kg * 3.5) / 200 ) / 60
        Distance += ((speed / (double)3600.0) /
                     ((double)1000.0 / (double)(lastTimeCharacteristicChanged.msecsTo(QDateTime::currentDateTime()))));
        lastTimeCharacteristicChanged = QDateTime::currentDateTime();
    }

    emit debug(QStringLiteral("Current speed: ") + QString::number(speed));
    emit debug(QStringLiteral("Current incline: ") + QString::number(incline));
    emit debug(QStringLiteral("Current heart: ") + QString::number(Heart.value()));
    emit debug(QStringLiteral("Current KCal: ") + QString::number(KCal.value()));
    emit debug(QStringLiteral("Current KCal from the machine: ") + QString::number(kcal));
    emit debug(QStringLiteral("Current Distance: ") + QString::number(distance));
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

    if (speed > 0) {

        lastSpeed = speed;
        lastInclination = incline;
    }

    firstCharacteristicChanged = false;
}

double domyostreadmill::GetSpeedFromPacket(const QByteArray &packet) {

    uint8_t convertedData = (uint8_t)packet.at(7);
    double data = (double)convertedData / 10.0f;
    return data;
}

double domyostreadmill::GetKcalFromPacket(const QByteArray &packet) {

    uint16_t convertedData = (packet.at(10) << 8) | ((uint8_t)packet.at(11));
    return (double)convertedData;
}

double domyostreadmill::GetDistanceFromPacket(const QByteArray &packet) {

    uint16_t convertedData = (packet.at(12) << 8) | packet.at(13);
    double data = ((double)convertedData) / 10.0f;
    return data;
}

double domyostreadmill::GetInclinationFromPacket(const QByteArray &packet) {

    uint16_t convertedData = (packet.at(2) << 8) | ((uint8_t)packet.at(3));
    double data;

    if (convertedData > 10000) {
        data = ((double)convertedData - 65512.0f) / 10.0f;
    } else {
        data = ((double)convertedData - 1000.0f) / 10.0f;
    }
    if (data < 0) {
        return 0;
    }
    return data;
}

void domyostreadmill::btinit(bool startTape) {
    writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, true);
    writeCharacteristic(initData2, sizeof(initData2), QStringLiteral("init"), false, true);
    writeCharacteristic(initDataStart, sizeof(initDataStart), QStringLiteral("init"), false, true);
    writeCharacteristic(initDataStart2, sizeof(initDataStart2), QStringLiteral("init"), false, true);
    writeCharacteristic(initDataStart3, sizeof(initDataStart3), QStringLiteral("init"), false, true);
    writeCharacteristic(initDataStart4, sizeof(initDataStart4), QStringLiteral("init"), false, true);
    writeCharacteristic(initDataStart5, sizeof(initDataStart5), QStringLiteral("init"), false, true);

    // Old behavior (before commit c90093046): these lines were always executed
    // New behavior (after commit c90093046): these lines are only executed if startTape is true
    if (domyos_treadmill_sync_start) {
        // Old behavior: always execute these lines
        forceSpeedOrIncline(lastSpeed, lastInclination);

        writeCharacteristic(initDataStart8, sizeof(initDataStart8), QStringLiteral("init"), false, false);
        writeCharacteristic(initDataStart9, sizeof(initDataStart9), QStringLiteral("init"), false, true);
    }

    if (startTape) {
        // writeCharacteristic(initDataStart6, sizeof(initDataStart6), "init", false, false);
        // writeCharacteristic(initDataStart7, sizeof(initDataStart7), "init", false, true);

        if (!domyos_treadmill_sync_start) {
            // New behavior: only execute if startTape is true
            forceSpeedOrIncline(lastSpeed, lastInclination);

            writeCharacteristic(initDataStart8, sizeof(initDataStart8), QStringLiteral("init"), false, false);
            writeCharacteristic(initDataStart9, sizeof(initDataStart9), QStringLiteral("init"), false, true);
        }

        writeCharacteristic(initDataStart10, sizeof(initDataStart10), QStringLiteral("init"), false, false);
        writeCharacteristic(initDataStart11, sizeof(initDataStart11), QStringLiteral("init"), false, true);
        writeCharacteristic(initDataStart12, sizeof(initDataStart12), QStringLiteral("init"), false, false);
        writeCharacteristic(initDataStart13, sizeof(initDataStart13), QStringLiteral("init"), false, true);

        forceSpeedOrIncline(lastSpeed, lastInclination);
    }

    initDone = true;
}

void domyostreadmill::stateChanged(QLowEnergyService::ServiceState state) {

    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceState>();
    emit debug(QStringLiteral("BTLE stateChanged ") + QString::fromLocal8Bit(metaEnum.valueToKey(state)));
    if (state == QLowEnergyService::RemoteServiceDiscovered) {

        // qDebug() << gattCommunicationChannelService->characteristics();

        gattWriteCharacteristic = gattCommunicationChannelService->characteristic(_gattWriteCharacteristicId);
        gattNotifyCharacteristic = gattCommunicationChannelService->characteristic(_gattNotifyCharacteristicId);
        Q_ASSERT(gattWriteCharacteristic.isValid());
        Q_ASSERT(gattNotifyCharacteristic.isValid());

        // establish hook into notifications
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicChanged, this,
                &domyostreadmill::characteristicChanged);
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicWritten, this,
                &domyostreadmill::characteristicWritten);
        connect(gattCommunicationChannelService,
                &QLowEnergyService::errorOccurred,
                this, &domyostreadmill::errorService);
        connect(gattCommunicationChannelService, &QLowEnergyService::descriptorWritten, this,
                &domyostreadmill::descriptorWritten);

        QByteArray descriptor;
        descriptor.append((char)0x01);
        descriptor.append((char)0x00);
        gattCommunicationChannelService->writeDescriptor(
            gattNotifyCharacteristic.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration), descriptor);
    }
}

void domyostreadmill::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    emit debug(QStringLiteral("descriptorWritten ") + descriptor.name() + QStringLiteral(" ") + newValue.toHex(' '));

    initRequest = true;
    emit connectedAndDiscovered();
}

void domyostreadmill::characteristicWritten(const QLowEnergyCharacteristic &characteristic,
                                            const QByteArray &newValue) {
    Q_UNUSED(characteristic);
    emit debug(QStringLiteral("characteristicWritten ") + newValue.toHex(' '));
}

void domyostreadmill::serviceScanDone(void) {
    emit debug(QStringLiteral("serviceScanDone"));

    gattCommunicationChannelService = m_control->createServiceObject(_gattCommunicationChannelServiceId);
    if (gattCommunicationChannelService) {
        connect(gattCommunicationChannelService, &QLowEnergyService::stateChanged, this, &domyostreadmill::stateChanged);
        gattCommunicationChannelService->discoverDetails();
    } else {
        emit debug(QStringLiteral("error on find Service"));
    }    
}

void domyostreadmill::errorService(QLowEnergyService::ServiceError err) {

    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    emit debug(QStringLiteral("domyostreadmill::errorService ") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void domyostreadmill::error(QLowEnergyController::Error err) {

    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    emit debug(QStringLiteral("domyostreadmill::error ") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void domyostreadmill::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    {

        bluetoothDevice = device;
        m_control = QLowEnergyController::createCentral(bluetoothDevice, this);
        connect(m_control, &QLowEnergyController::serviceDiscovered, this, &domyostreadmill::serviceDiscovered);
        connect(m_control, &QLowEnergyController::discoveryFinished, this, &domyostreadmill::serviceScanDone);
        connect(m_control,
                &QLowEnergyController::errorOccurred,
                this, &domyostreadmill::error);
        connect(m_control, &QLowEnergyController::stateChanged, this, &domyostreadmill::controllerStateChanged);

        connect(m_control,
                &QLowEnergyController::errorOccurred,
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

void domyostreadmill::controllerStateChanged(QLowEnergyController::ControllerState state) {
    qDebug() << QStringLiteral("controllerStateChanged") << state;
    if (state == QLowEnergyController::UnconnectedState && m_control) {
        qDebug() << QStringLiteral("trying to connect back again...");

        initDone = false;
        m_control->connectToDevice();
    }
}

bool domyostreadmill::connected() {
    if (!m_control) {

        return false;
    }
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

void domyostreadmill::searchingStop() { searchStopped = true; }
