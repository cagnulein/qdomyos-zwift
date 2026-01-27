/*
 *
    ACKNOWLEDGE(2),
    AGE(20),
    BOOSTER_ENABLES(80),
    BOOSTER_INTERVAL_TIME(82),
    BOOSTER_LEVEL_OFFSET(48),
    BOOSTER_REVS(83),
    BOOSTER_SETTINGS(84),
    BOOSTER_STATUS(81),
    CALORIES(13),
    CALORIES_HOUR(14),
    CHECK_SUM(1),
    CONSOLE_SERIAL_NUMBER(121),
    CONTROL_AUTO_STATUS(90),
    CONTROL_CONSOLE_ACTIVITY(95),
    CONTROL_COOL_DOWN(86),
    CONTROL_FAN(88),
    CONTROL_GO(89),
    CONTROL_QUICK_START(87),
    CONTROL_MIDDLE_BOARD_RESET(94),
    CONTROL_NEXT_SELECTION(96),
    CONTROL_PAUSE(93),
    CONTROL_RESUME(91),
    CONTROL_TERMINATE_WORKOUT(92),
    CROSS_CIRCUIT_FREE_STATUS(75),
    CROSS_CIRCUIT_TIME_STATUS(76),
    DISTANCE(11),
    DISTANCE_REVS(12),
    ERROR(4),
    EXERTION(37),
    GOAL_SETUP(73),
    GOAL_STATUS(74),
    USERS_HEART_RATE(17),
    HEART_RATE_AVG(18),
    HEART_RATE_MAX(19),
    DESIRED_HEART_RATE_TARGET(24),
    HEART_RATE_TARGET_STATUS(79),
    INCLINE_CONTROL(31),
    INCLINE_GET(36),
    INTERVAL_SETUP(65),
    INTERVAL_STATUS(66),
    LEVEL(9),
    LEVEL_CONTROL(27),
    LEVEL_TABLE(64),
    MACHINE_BASE_SERIAL(120),
    MACHINE_CONSOLE_SERIAL(121),
    MACHINE_PREFS(112),
    METS(16),
    METS_AVERAGE(29),
    METS_TARGET(45),
    MMA_SETUP(77),
    MMA_STATUS(78),
    MODEL_AND_VERSION(5),
    NONE(0),
    PROGRAM(22),
    SMART_STRIDE_DATA(85),
    SPEED(7),
    SPEED_AVERAGE(8),
    STATE(3),
    STRIDE(10),
    STRIDE_CONTROL(30),
    THIRTY_THIRTY_AVERAGES(72),
    THIRTY_THIRTY_STATUS(67),
    THIRTY_THIRTY_GROUP_DATA(69),
    THIRTY_THIRTY_INTERMISSION_TIME(70),
    THIRTY_THIRTY_INTERVAL_DATA(68),
    THRITY_THIRTY_INTERVAL_TIME(71),
    TIME(6),
    TIME_IN_ZONE(26),
    UNITS(23),
    WATTS(15),
    WATTS_AVERAGE(28),
    WATTS_TARGET(44),
    WEIGHT(21),
    WORKOUT_TIME(25),
    ZR_AVERAGE_PACE(36),
    ZR_AVERAGE_PACE_HOTH(61),
    ZR_PACE(35),
    ZR_LEFT_STEP_DIMENSION(32),
    ZR_LEFT_STRIDE_COORDS(34),
    ZR_LEFT_STEP_AVERAGE_DIMENSION(59),
    ZR_RIGHT_STEP_DIMENSION(57),
    ZR_RIGHT_STRIDE_COORDS(56),
    ZR_RIGHT_STEP_AVERAGE_DIMENSION(60),
    ZR_CADENCE(58),
    ZR_RUN_PLUS(33),
    DIAGNOSTICS_BRAKE_TEST(-110),
    DIAGNOSTICS_CONSOLE_BOARD_DATA_RESET(-79),
    DIAGNOSTICS_CONSOLE_BOARD_DATA(-80),
    DIAGNOSTICS_DIAGNOSTIC_KEY(-105),
    DIAGNOSTICS_DEBUG_INFO(-64),
    DIAGNOSTICS_ENTER_EXIT(128),
    DIAGNOSTICS_MACHINE_DISTANCE(-100),
    DIAGNOSTICS_HEART_RATE_STATUS(-125),
    DIAGNOSTICS_HOTH_CALIBRATION_DATA(-87),
    DIAGNOSTICS_HOTH_CALIBRATION_MODE(-88),
    DIAGNOSTICS_HOTH_LEVEL_POSITION(-99),
    DIAGNOSTICS_HOTH_SENSOR_DATA(-86),
    DIAGNOSTICS_INCLINE_ERRORS(-101),
    DIAGNOSTICS_INCLINE_POSITION(-104),
    DIAGNOSTICS_INCLINE_STARTS(-102),
    DIAGNOSTICS_INCLINE_TEST(-108),
    DIAGNOSTICS_INCLINE_TIME(-103),
    DIAGNOSTICS_LEFT_STRIDE_POSITION(-123),
    DIAGNOSTICS_LEFT_STRIDE_ERRORS(-117),
    DIAGNOSTICS_LEFT_STRIDE_STARTS(-118),
    DIAGNOSTICS_LEFT_STRIDE_TIME(-119),
    DIAGNOSTICS_LEVEL(-124),
    DIAGNOSTICS_MACHINE_TIME(-121),
    DIAGNOSTICS_MACHINE_REVS(-120),
    DIAGNOSTICS_PAC_BOARD_VERSION(-127),
    DIAGNOSTICS_PAC_POWER_DATA(-113),
    DIAGNOSTICS_PAC_RESET(-112),
    DIAGNOSTICS_PAC_SERIAL_NUMBER(-96),
    DIAGNOSTICS_PORTS_TEST(-106),
    DIAGNOSTICS_RADIO_TEST(-107),
    DIAGNOSTICS_RIGHT_STRIDE_POSITION(-122),
    DIAGNOSTICS_RIGHT_STRIDE_ERRORS(-114),
    DIAGNOSTICS_RIGHT_STRIDE_STARTS(-115),
    DIAGNOSTICS_RIGHT_STRIDE_TIME(-116),
    DIAGNOSTICS_RPM_STATUS(-126),
    DIAGNOSTICS_STRIDE_TEST(-109),
    DIAGNOSTICS_TEST_STATUS(-111),
    DIAGNOSTICS_UPDATE_FIRMWARE_CONTROL(-48),
    DIAGNOSTICS_UPDATE_FIRMWARE_DATA(-47),
    TV_SETUP(115),
    TV_PVS_CHANNEL_LIST(116),
    TV_PVS_CONTROL(117),
    TV_PVS_DEFAULT(118),
    TV_900_MHZ_CONTROL(119),
    TV_AUTO_STATUS(122),
    SELECT_USB(-39),
    PCB_REVISION(-46),
    FITLINXX_STATE(41),
    FITLINXX_USER_ID(42),
    FITLINXX_ANT_PRESET(43),
    HOTH_OPTIONAL_DATA_BYTE(46),
    HOTH_OPTIONAL_DATA_WORD(47),
    PROGRAM_TIME(49),
    CONTROL_TARGET_HEART_RATE(97),
    CONTROL_TARGET_WATTS(98),
    CONTROL_TARGET_METS(99),
    SALES_MODE(113),
    SIM_MODE(114),
    BTLE_RADIO_ID(TransportMediator.KEYCODE_MEDIA_PLAY),
    UPDATE_FIRMWARE_CONTROL_SMART_VERSION_NEW(-44),
    UPDATE_FIRMWARE_DATA_SMART(-43),
    SET_MAINTENANCE_MODE(-40),
    SET_COM_PORT(-36),
    ERASE_STORAGE_MEMORY(-34);
*/

#include "octanetreadmill.h"
#include "keepawakehelper.h"
#include "virtualdevices/virtualtreadmill.h"
#include <QBluetoothLocalDevice>
#include <QDateTime>
#include <QFile>
#include <QMetaEnum>
#include <QSettings>
#include <chrono>

using namespace std::chrono_literals;

octanetreadmill::octanetreadmill(uint32_t pollDeviceTime, bool noConsole, bool noHeartService, double forceInitSpeed,
                                 double forceInitInclination) {

    actualPaceSign.clear();
    actualPace2Sign.clear();
    actualPace3Sign.clear();

    // ZR_PACE
    actualPaceSign.append(0x02);
    actualPaceSign.append(0x23);
    actualPace2Sign.append(0x01);
    actualPace2Sign.append(0x23);
    actualPace3Sign.append((char)0x00);
    actualPace3Sign.append(0x23);    
    cadenceSign.append(0x3A);

    m_watt.setType(metric::METRIC_WATT, deviceType());
    Speed.setType(metric::METRIC_SPEED);
    this->noConsole = noConsole;
    this->noHeartService = noHeartService;

    if (forceInitSpeed > 0)
        lastSpeed = forceInitSpeed;

    if (forceInitInclination > 0)
        lastInclination = forceInitInclination;

    refresh = new QTimer(this);
    initDone = false;
    connect(refresh, &QTimer::timeout, this, &octanetreadmill::update);
    refresh->start(500ms);
}

void octanetreadmill::writeCharacteristic(uint8_t *data, uint8_t data_len, const QString &info, bool disable_log,
                                          bool wait_for_response) {
    QEventLoop loop;
    QTimer timeout;

    if (wait_for_response) {
        connect(this, &octanetreadmill::packetReceived, &loop, &QEventLoop::quit);
        timeout.singleShot(400ms, &loop, &QEventLoop::quit);
    } else {
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicWritten, &loop, &QEventLoop::quit);
        timeout.singleShot(400ms, &loop, &QEventLoop::quit);
    }

    if (writeBuffer) {
        delete writeBuffer;
    }
    writeBuffer = new QByteArray((const char *)data, data_len);

    gattCommunicationChannelService->writeCharacteristic(gattWriteCharacteristic, *writeBuffer);

    if (!disable_log) {
        emit debug(QStringLiteral(" >> ") + writeBuffer->toHex(' ') +
                   QStringLiteral(" // ") + info);
    }

    // packets sent from the characChanged event, i don't want to block everything
    if (wait_for_response) {
        loop.exec();

        if (timeout.isActive() == false)
            emit debug(QStringLiteral(" exit for timeout"));
    }
}

void octanetreadmill::updateDisplay(uint16_t elapsed) {
    Q_UNUSED(elapsed);
    // uint8_t noOpData[] = {0x55, 0x0d, 0x0a, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

    // writeCharacteristic(noOpData, sizeof(noOpData), QStringLiteral("noOp"));
}

void octanetreadmill::forceIncline(double requestIncline) {
    // uint8_t incline[] = {0x55, 0x11, 0x01, 0x06};
    // incline[3] = (uint8_t)requestIncline;
    // writeCharacteristic(incline, sizeof(incline), QStringLiteral("forceIncline ") + QString::number(requestIncline));
}

double octanetreadmill::minStepInclination() { return 1.0; }
double octanetreadmill::minStepSpeed() { return 1.0; }

void octanetreadmill::forceSpeed(double requestSpeed) {
    // uint8_t speed[] = {0x55, 0x0f, 0x02, 0x08, 0x00};
    // speed[3] = (uint8_t)requestSpeed;
    // writeCharacteristic(speed, sizeof(speed), QStringLiteral("forceSpeed ") + QString::number(requestSpeed));
}

void octanetreadmill::changeInclinationRequested(double grade, double percentage) {
    if (percentage < 0)
        percentage = 0;
    changeInclination(grade, percentage);
}

void octanetreadmill::update() {
    if (m_control && m_control->state() == QLowEnergyController::UnconnectedState) {
        emit disconnected();
        return;
    }

    qDebug() << m_control->state() << bluetoothDevice.isValid() << gattCommunicationChannelService
             << gattWriteCharacteristic.isValid() << initDone << requestSpeed << requestInclination;

    if (initRequest) {
        initRequest = false;
        btinit((lastSpeed > 0 ? true : false));
    } else if (bluetoothDevice.isValid() && m_control->state() == QLowEnergyController::DiscoveredState &&
               gattCommunicationChannelService && gattWriteCharacteristic.isValid() && initDone) {
        QSettings settings;
        // ******************************************* virtual treadmill init *************************************
        if (!firstInit && !this->hasVirtualDevice()) {
            bool virtual_device_enabled =
                settings.value(QZSettings::virtual_device_enabled, QZSettings::default_virtual_device_enabled).toBool();
            if (virtual_device_enabled) {
                emit debug(QStringLiteral("creating virtual treadmill interface..."));
                auto virtualTreadMill = new virtualtreadmill(this, noHeartService);
                connect(virtualTreadMill, &virtualtreadmill::debug, this, &octanetreadmill::debug);
                connect(virtualTreadMill, &virtualtreadmill::changeInclination, this,
                        &octanetreadmill::changeInclinationRequested);
                this->setVirtualDevice(virtualTreadMill, VIRTUAL_DEVICE_MODE::PRIMARY);
                firstInit = 1;
            }
        }
        // ********************************************************************************************************

        update_metrics(true, watts(settings.value(QZSettings::weight, QZSettings::default_weight).toFloat()));

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
            lastStart = QDateTime::currentMSecsSinceEpoch();
            // uint8_t start[] = {0x55, 0x0a, 0x01, 0x01};
            // writeCharacteristic(start, sizeof(start), "start", false, true);
            if (lastSpeed == 0.0) {
                lastSpeed = 0.5;
            }
            requestStart = -1;
            emit tapeStarted();
        } else if (requestStop != -1) {
            emit debug(QStringLiteral("stopping..."));
            // uint8_t stop[] = {0x55, 0x0a, 0x01, 0x02};
            // writeCharacteristic(stop, sizeof(stop), "stop", false, true);
            requestStop = -1;
            lastStop = QDateTime::currentMSecsSinceEpoch();
        } else if (sec1Update++ >= (400 / refresh->interval())) {
            updateDisplay(elapsed.value());
            sec1Update = 0;
        }
    }
}

void octanetreadmill::serviceDiscovered(const QBluetoothUuid &gatt) {
    emit debug(QStringLiteral("serviceDiscovered ") + gatt.toString());
}

void octanetreadmill::characteristicChanged(const QLowEnergyCharacteristic &characteristic,
                                            const QByteArray &newValue) {
    // qDebug() << "characteristicChanged" << characteristic.uuid() << newValue << newValue.length();
    QSettings settings;
    QString heartRateBeltName =
        settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name).toString();
    Q_UNUSED(characteristic);
    QByteArray value = newValue;

    emit debug(QStringLiteral(" << ") + QString::number(value.length()) + QStringLiteral(" ") + value.toHex(' '));

    emit packetReceived();

    if (ZR8 == false && lastTimeCharacteristicChanged.secsTo(QDateTime::currentDateTime()) > 5) {
        emit debug(QStringLiteral("resetting speed"));
        Speed = 0;
        Cadence = 0;
    }

    // ZR8: Packet reassembly logic for fragmented BLE messages
    if (ZR8 && (uint8_t)newValue[0] == 0xa5) {
        // Start of a new ZR8 packet
        if (newValue.length() >= 2) {
            uint8_t lengthByte = (uint8_t)newValue[1];

            // Map length byte to expected packet size
            int expectedLen = 0;
            if (lengthByte == 0x1d) {
                expectedLen = 29;  // A5 1D format
            } else if (lengthByte == 0x26) {
                expectedLen = 38;  // A5 26 format (Gait Analysis)
            } else if (lengthByte == 0x23) {
                expectedLen = 35;  // A5 23 format
            } else if (lengthByte == 0x20 || lengthByte == 0x21) {
                expectedLen = 32;  // A5 20/21 format
            }

            if (expectedLen > 0) {
                packetBuffer.clear();
                packetBuffer.append(newValue);
                expectedPacketLength = expectedLen;

                if (packetBuffer.length() >= expectedPacketLength) {
                    // Complete packet received
                    value = packetBuffer.mid(0, expectedPacketLength);
                    packetBuffer.clear();
                    expectedPacketLength = 0;
                    emit debug(QStringLiteral("ZR8: Complete packet assembled: ") +
                               QString::number(expectedLen) + QStringLiteral(" bytes"));
                } else {
                    // Incomplete, need more fragments
                    return;
                }
            } else {
                return;
            }
        } else {
            return;
        }
    } else if (ZR8 && expectedPacketLength > 0) {
        // Continuation of a fragmented packet
        packetBuffer.append(newValue);

        if (packetBuffer.length() >= expectedPacketLength) {
            // Complete packet now assembled
            value = packetBuffer.mid(0, expectedPacketLength);
            packetBuffer.clear();
            expectedPacketLength = 0;
            emit debug(QStringLiteral("ZR8: Complete packet assembled from fragments"));
        } else {
            return;
        }
    } else {
        // Non-ZR8 or non-A5 packet
        if ((value.length() != 20))
            return;

        if ((uint8_t)newValue[0] == 0xa5 && newValue[1] == 0x17)
            return;
    }

    // ZR8: Speed data is only in packets starting with a5 2[0123] 06
    // Other packets containing 02 23 signature do not have valid speed data
    if (ZR8) {
        if (newValue.length() < 18)
            return;

        // Check for valid speed packet header: a5 20 06, a5 21 06, or a5 23 06
        bool isValidSpeedPacket = ((uint8_t)newValue[0] == 0xa5);
        if (!isValidSpeedPacket) {
            if (value.contains(actualPaceSign) || value.contains(actualPace2Sign) || value.contains(actualPace3Sign)) {
                // Try to extract speed and check coherence
                int16_t idx = value.indexOf(actualPaceSign) + 2;
                if (idx <= 1)
                    idx = value.indexOf(actualPace2Sign) + 2;
                if (idx <= 1)
                    idx = value.indexOf(actualPace3Sign) + 2;

                if (idx + 1 < newValue.length()) {
                    double candidateSpeed = GetSpeedFromPacket(value, idx);
                    // Allow if coherent (e.g. within 3km/h) or if we are starting (lastSpeed approx 0)
                    if (std::abs(candidateSpeed - Speed.value()) < 3.0 || Speed.value() < 0.5) {
                        // Coherent, let it pass to the main extraction logic below
                        // Do NOT return
                        emit debug(QStringLiteral("ZR8: Recovering non-standard speed packet: ") +
                                   QString::number(candidateSpeed));
                    } else {
                        emit debug(QStringLiteral("ZR8: Ignoring incoherent speed packet: ") +
                                   QString::number(candidateSpeed) + QStringLiteral(" vs ") +
                                   QString::number(Speed.value()));
                        return;
                    }
                } else {
                    return;
                }
            } else {
                return; // Not a speed packet, ignore
            }
        }
    }

    // ZR8: Cadence parsing from 0x3A marker with anomaly filtering
    if (ZR8 && value.contains(cadenceSign)) {
        int16_t cadenceIdx = value.indexOf(cadenceSign) + 1;

        if (cadenceIdx >= value.length() || cadenceIdx < 1) {
            // Marker not found or invalid position
            emit debug(QStringLiteral("ZR8: Cadence marker (0x3A) not found or invalid"));
        } else {
            uint8_t rawCadence = (uint8_t)value.at(cadenceIdx);

            // Anomaly filtering: accept values in realistic range (20-200 RPM)
            // Values outside this range are likely parsing errors from packet format shifts
            if (rawCadence >= 20 && rawCadence <= 200) {
                Cadence = rawCadence;
                emit debug(QStringLiteral("ZR8: Cadence parsed: ") + QString::number(rawCadence));

                // Reset cadence zero timer when valid cadence is received
                if (!lastCadenceZeroTime.isNull()) {
                    lastCadenceZeroTime = QDateTime();
                }
            } else {
                // Anomalous cadence value - likely from packet format shift
                emit debug(QStringLiteral("ZR8: Cadence anomaly filtered: ") + QString::number(rawCadence) +
                           QStringLiteral(" RPM (outside 20-200 range)"));

                // Track time when cadence drops below 20 (potential stop condition)
                if (rawCadence < 20 && rawCadence > 0) {
                    if (lastCadenceZeroTime.isNull()) {
                        lastCadenceZeroTime = QDateTime::currentDateTime();
                        emit debug(QStringLiteral("ZR8: Cadence low (< 20 RPM), starting 5-second timer"));
                    } else {
                        qint64 secondsSinceLowCadence = lastCadenceZeroTime.secsTo(QDateTime::currentDateTime());
                        if (secondsSinceLowCadence >= 5 && Speed.value() > 0) {
                            emit debug(QStringLiteral("ZR8: Cadence < 20 for 5+ seconds, resetting speed to 0"));
                            Speed = 0;
                            emit speedChanged(0);
                        }
                    }
                } else if (rawCadence == 0) {
                    // Cadence explicitly zero - reset speed immediately
                    if (Speed.value() > 0) {
                        emit debug(QStringLiteral("ZR8: Cadence = 0, resetting speed"));
                        Speed = 0;
                        emit speedChanged(0);
                    }
                }
            }
        }
    }

    if (!value.contains(actualPaceSign) && !value.contains(actualPace2Sign) && !value.contains(actualPace3Sign))
        return;

    int16_t i = value.indexOf(actualPaceSign) + 2;
    if (i <= 1)
        i = value.indexOf(actualPace2Sign) + 2;
    if (i <= 1)
        i = value.indexOf(actualPace3Sign) + 2;

    if (i + 1 >= value.length())
        return;

    double speed = GetSpeedFromPacket(value, i);
    if (isinf(speed))
        return;

#ifdef Q_OS_ANDROID
    if (settings.value(QZSettings::ant_heart, QZSettings::default_ant_heart).toBool())
        Heart = (uint8_t)KeepAwakeHelper::heart();
    else
#endif
    {
        update_hr_from_external();
    }
    emit debug(QStringLiteral("Current speed: ") + QString::number(speed));

    if (Speed.value() != speed) {
        emit speedChanged(speed);
    }
    Speed = speed;

    if (speed > 0) {
        lastSpeed = speed;
    }

    // this treadmill has a bug that always send 1km/h even if the tape is stopped
    if (speed > 1.0) {
        lastStart = 0;
    } else {
        lastStop = 0;
    }

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

        Distance += ((Speed.value() / 3600.0) /
                     (1000.0 / (lastTimeCharacteristicChanged.msecsTo(QDateTime::currentDateTime()))));
    }

    // ZR8 has builtin cadence sensor
    if (!ZR8)
        cadenceFromAppleWatch();

    emit debug(QStringLiteral("Current Distance Calculated: ") + QString::number(Distance.value()));
    emit debug(QStringLiteral("Current KCal: ") + QString::number(KCal.value()));
    emit debug(QStringLiteral("Current Cadence: ") + QString::number(Cadence.value()));

    if (m_control && m_control->error() != QLowEnergyController::NoError) {
        qDebug() << QStringLiteral("QLowEnergyController ERROR!!") << m_control->errorString();
    }

    lastTimeCharacteristicChanged = QDateTime::currentDateTime();
    firstCharacteristicChanged = false;
}

double octanetreadmill::GetSpeedFromPacket(const QByteArray &packet, int index) {
    uint16_t convertedData = (packet.at(index + 1) << 8) | ((uint8_t)packet.at(index));
    return (1.0 / ((double)convertedData)) * 3600.0;
}

void octanetreadmill::btinit(bool startTape) {
    Q_UNUSED(startTape)
    uint8_t initData1[] = {0xa5, 0x04, 0x21, 0x05, 0x01, 0xd9};
    uint8_t initData2[] = {0xa5, 0x04, 0x21, 0x70, 0x01, 0x6e};
    writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, true);
    writeCharacteristic(initData2, sizeof(initData2), QStringLiteral("init"), false, true);

    initDone = true;
}

void octanetreadmill::stateChanged(QLowEnergyService::ServiceState state) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceState>();
    emit debug(QStringLiteral("BTLE stateChanged ") + QString::fromLocal8Bit(metaEnum.valueToKey(state)));
    if (state == QLowEnergyService::ServiceDiscovered) {
        QBluetoothUuid _gattWriteCharacteristicId(QStringLiteral("96dc867f-7a83-4c22-b6be-6381d727aeda"));
        QBluetoothUuid _gattNotify1CharacteristicId(QStringLiteral("96dc867e-7a83-4c22-b6be-6381d727aeda"));

        // qDebug() << gattCommunicationChannelService->characteristics();

        gattWriteCharacteristic = gattCommunicationChannelService->characteristic(_gattWriteCharacteristicId);
        gattNotify1Characteristic = gattCommunicationChannelService->characteristic(_gattNotify1CharacteristicId);
        Q_ASSERT(gattWriteCharacteristic.isValid());
        Q_ASSERT(gattNotify1Characteristic.isValid());

        // establish hook into notifications
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicChanged, this,
                &octanetreadmill::characteristicChanged);
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicWritten, this,
                &octanetreadmill::characteristicWritten);
        connect(gattCommunicationChannelService,
                static_cast<void (QLowEnergyService::*)(QLowEnergyService::ServiceError)>(&QLowEnergyService::error),
                this, &octanetreadmill::errorService);
        connect(gattCommunicationChannelService, &QLowEnergyService::descriptorWritten, this,
                &octanetreadmill::descriptorWritten);

        QByteArray descriptor;
        descriptor.append((char)0x01);
        descriptor.append((char)0x00);
        gattCommunicationChannelService->writeDescriptor(
            gattNotify1Characteristic.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration), descriptor);
    }
}

void octanetreadmill::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    emit debug(QStringLiteral("descriptorWritten ") + descriptor.name() + " " + newValue.toHex(' '));

    initRequest = true;
    emit connectedAndDiscovered();
}

void octanetreadmill::characteristicWritten(const QLowEnergyCharacteristic &characteristic,
                                            const QByteArray &newValue) {
    Q_UNUSED(characteristic);
    emit debug(QStringLiteral("characteristicWritten ") + newValue.toHex(' '));
}

void octanetreadmill::serviceScanDone(void) {
    emit debug(QStringLiteral("serviceScanDone"));

    QBluetoothUuid _gattCommunicationChannelServiceId(QStringLiteral("96dc867d-7a83-4c22-b6be-6381d727aeda"));
    gattCommunicationChannelService = m_control->createServiceObject(_gattCommunicationChannelServiceId);
    connect(gattCommunicationChannelService, &QLowEnergyService::stateChanged, this, &octanetreadmill::stateChanged);
    gattCommunicationChannelService->discoverDetails();
}

void octanetreadmill::errorService(QLowEnergyService::ServiceError err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    emit debug(QStringLiteral("octanetreadmill::errorService ") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void octanetreadmill::error(QLowEnergyController::Error err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    emit debug(QStringLiteral("octanetreadmill::error ") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void octanetreadmill::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    emit debug(QStringLiteral("Found new device: ") + device.name() + QStringLiteral(" (") +
               device.address().toString() + ')');
    {
        bluetoothDevice = device;

        if (device.name().toUpper().startsWith(QLatin1String("ZR8"))) {
            ZR8 = true;
            qDebug() << "ZR8 workaround activated";
        }

        m_control = QLowEnergyController::createCentral(bluetoothDevice, this);
        connect(m_control, &QLowEnergyController::serviceDiscovered, this, &octanetreadmill::serviceDiscovered);
        connect(m_control, &QLowEnergyController::discoveryFinished, this, &octanetreadmill::serviceScanDone);
        connect(m_control,
                static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error),
                this, &octanetreadmill::error);
        connect(m_control, &QLowEnergyController::stateChanged, this, &octanetreadmill::controllerStateChanged);

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

void octanetreadmill::controllerStateChanged(QLowEnergyController::ControllerState state) {
    qDebug() << QStringLiteral("controllerStateChanged") << state;
    if (state == QLowEnergyController::UnconnectedState && m_control) {
        qDebug() << QStringLiteral("trying to connect back again...");
        initDone = false;
        m_control->connectToDevice();
    }
}

bool octanetreadmill::connected() {
    if (!m_control) {
        return false;
    }
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

bool octanetreadmill::autoPauseWhenSpeedIsZero() {
    if (lastStart == 0 || QDateTime::currentMSecsSinceEpoch() > (lastStart + 10000))
        return true;
    else
        return false;
}

bool octanetreadmill::autoStartWhenSpeedIsGreaterThenZero() {
    if ((lastStop == 0 || QDateTime::currentMSecsSinceEpoch() > (lastStop + 25000)) && requestStop == -1)
        return true;
    else
        return false;
}
