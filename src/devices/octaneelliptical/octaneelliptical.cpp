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

#include "octaneelliptical.h"
#include "keepawakehelper.h"
#include "virtualdevices/virtualtreadmill.h"
#include <QBluetoothLocalDevice>
#include <QDateTime>
#include <QFile>
#include <QMetaEnum>
#include <QSettings>
#include <chrono>

using namespace std::chrono_literals;

octaneelliptical::octaneelliptical(uint32_t pollDeviceTime, bool noConsole, bool noHeartService, double forceInitSpeed,
                                   double forceInitInclination) {

    actualPaceSign.clear();
    actualPace2Sign.clear();

    // SPEED
    actualPaceSign.append(0x01);
    actualPaceSign.append(0x07);
    actualPace2Sign.append((char)0x00);
    actualPace2Sign.append(0x07);

    actualHR.append((char)0x02);
    actualHR.append((char)0x11);

    actualResistance.append((char)0x01);
    actualResistance.append((char)0x09);

    actualOdometer.append((char)0x0b);

    m_watt.setType(metric::METRIC_WATT, deviceType());
    Speed.setType(metric::METRIC_SPEED);
    this->noConsole = noConsole;
    this->noHeartService = noHeartService;

    refresh = new QTimer(this);
    initDone = false;
    connect(refresh, &QTimer::timeout, this, &octaneelliptical::update);
    refresh->start(500ms);
}

void octaneelliptical::writeCharacteristic(uint8_t *data, uint8_t data_len, const QString &info, bool disable_log,
                                           bool wait_for_response) {
    QEventLoop loop;
    QTimer timeout;

    if (wait_for_response) {
        connect(this, &octaneelliptical::packetReceived, &loop, &QEventLoop::quit);
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

void octaneelliptical::updateDisplay(uint16_t elapsed) {
    Q_UNUSED(elapsed);
    // uint8_t noOpData[] = {0x55, 0x0d, 0x0a, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

    // writeCharacteristic(noOpData, sizeof(noOpData), QStringLiteral("noOp"));
}

void octaneelliptical::forceIncline(double requestIncline) {
    // uint8_t incline[] = {0x55, 0x11, 0x01, 0x06};
    // incline[3] = (uint8_t)requestIncline;
    // writeCharacteristic(incline, sizeof(incline), QStringLiteral("forceIncline ") + QString::number(requestIncline));
}

double octaneelliptical::minStepInclination() { return 1.0; }
double octaneelliptical::minStepSpeed() { return 1.0; }

void octaneelliptical::forceSpeed(double requestSpeed) {
    // uint8_t speed[] = {0x55, 0x0f, 0x02, 0x08, 0x00};
    // speed[3] = (uint8_t)requestSpeed;
    // writeCharacteristic(speed, sizeof(speed), QStringLiteral("forceSpeed ") + QString::number(requestSpeed));
}

void octaneelliptical::changeInclinationRequested(double grade, double percentage) {
    if (percentage < 0)
        percentage = 0;
    changeInclination(grade, percentage);
}

void octaneelliptical::update() {
    if (m_control->state() == QLowEnergyController::UnconnectedState) {
        emit disconnected();
        return;
    }

    qDebug() << m_control->state() << bluetoothDevice.isValid() << gattCommunicationChannelService
             << gattWriteCharacteristic.isValid() << initDone << requestSpeed << requestInclination;

    if (initRequest) {
        initRequest = false;
        btinit(false);
    } else if (bluetoothDevice.isValid() && m_control->state() == QLowEnergyController::DiscoveredState &&
               gattCommunicationChannelService && gattWriteCharacteristic.isValid() && initDone) {
        QSettings settings;
        // ******************************************* virtual treadmill init *************************************
        if (!firstInit && !this->hasVirtualDevice()) {
            bool virtual_device_enabled =
                settings.value(QZSettings::virtual_device_enabled, QZSettings::default_virtual_device_enabled).toBool();
            if (virtual_device_enabled) {
                emit debug(QStringLiteral("creating virtual treadmill interface..."));
                auto virtualTreadmill = new virtualtreadmill(this, noHeartService);
                connect(virtualTreadmill, &virtualtreadmill::debug, this, &octaneelliptical::debug);
                connect(virtualTreadmill, &virtualtreadmill::changeInclination, this,
                        &octaneelliptical::changeInclinationRequested);
                this->setVirtualDevice(virtualTreadmill, VIRTUAL_DEVICE_MODE::PRIMARY);
                firstInit = 1;
            }
        }
        // ********************************************************************************************************

        update_metrics(true, watts());

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
            requestStart = -1;
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

void octaneelliptical::serviceDiscovered(const QBluetoothUuid &gatt) {
    emit debug(QStringLiteral("serviceDiscovered ") + gatt.toString());
}

void octaneelliptical::characteristicChanged(const QLowEnergyCharacteristic &characteristic,
                                             const QByteArray &newValue) {
    // qDebug() << "characteristicChanged" << characteristic.uuid() << newValue << newValue.length();
    QSettings settings;
    QString heartRateBeltName =
        settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name).toString();
    Q_UNUSED(characteristic);
    QByteArray value = newValue;

    emit debug(QStringLiteral(" << ") + QString::number(value.length()) + QStringLiteral(" ") + value.toHex(' '));

    emit packetReceived();

    if (lastTimeCharacteristicChanged.secsTo(QDateTime::currentDateTime()) > 5) {
        emit debug(QStringLiteral("resetting speed"));
        Speed = 0;
    }

    if ((newValue.length() != 20))
        return;

    if (newValue.contains(actualResistance)) {
        int16_t i = newValue.indexOf(actualResistance) + 2;

        if (i + 1 < newValue.length()) {
            Resistance = ((uint8_t)value.at(i));
            emit debug(QStringLiteral("Current resistance: ") + QString::number(Resistance.value()));
        }
    }

    if (newValue.contains(actualHR)) {
        bool disable_hr_frommachinery = settings.value(QZSettings::heart_ignore_builtin, QZSettings::default_heart_ignore_builtin).toBool();

#ifdef Q_OS_ANDROID
        if (settings.value(QZSettings::ant_heart, QZSettings::default_ant_heart).toBool())
            Heart = (uint8_t)KeepAwakeHelper::heart();
        else
#endif
        {
            if (heartRateBeltName.startsWith(QStringLiteral("Disabled"))) {
                int16_t i = newValue.indexOf(actualHR) + 2;

                if (i + 1 < newValue.length()) {
                    uint8_t heart = ((uint8_t)value.at(i));
                    if (heart == 0 || disable_hr_frommachinery) {
                        update_hr_from_external();
                    } else {
                        Heart = heart;
                    }
                }
            }
        }
        emit debug(QStringLiteral("Current Heart: ") + QString::number(Heart.value()));
    }

    int16_t i = newValue.indexOf(actualPaceSign) + 2;
    /*if (i <= 1)
        i = newValue.indexOf(actualPace2Sign) + 1;*/

    if (i + 1 >= newValue.length() || i <= 1) {
        // fallback for a previous firmware version
        if(newValue.length() >= 20 && (uint8_t)newValue.at(0) == 0xa5) {
            if(newValue.at(6) == 0x07) {
                i = 7;
            } else if(newValue.at(15) == 0x07) {
                i = 16;
            } else {
                return;
            }
        } else {
            return;
        }
    }        

    Cadence = ((uint8_t)value.at(i));
    emit debug(QStringLiteral("Current Cadence: ") + QString::number(Cadence.value()));

    // Q37xi has a fixed stride length of 20.5 inches (52cm).
    Speed = (((Cadence.value() / 2.0) * 52.07 * 60) / 10000) * 1.1218;
    emit speedChanged(speed.value());
    emit debug(QStringLiteral("Current speed: ") + QString::number(Speed.value()));

    if (!firstCharacteristicChanged) {
        if (watts())
            KCal +=
                (((((0.048 * ((double)watts()) + 1.19) *
                   settings.value(QZSettings::weight, QZSettings::default_weight).toFloat() * 3.5) /
                  200.0) /
                 (60000.0 / ((double)lastTimeCharacteristicChanged.msecsTo(
                                QDateTime::currentDateTime())))) / 2.0); //(( (0.048* Output in watts +1.19) * body weight in
                                                                  // kg * 3.5) / 200 ) / 60
        Distance += ((Speed.value() / 3600000.0) *
                     ((double)lastTimeCharacteristicChanged.msecsTo(QDateTime::currentDateTime())));
        lastTimeCharacteristicChanged = QDateTime::currentDateTime();
    }

    emit debug(QStringLiteral("Current Distance: ") + QString::number(Distance.value()));
    emit debug(QStringLiteral("Current KCal: ") + QString::number(KCal.value()));

    if (m_control->error() != QLowEnergyController::NoError) {
        qDebug() << QStringLiteral("QLowEnergyController ERROR!!") << m_control->errorString();
    }

    firstCharacteristicChanged = false;
}

double octaneelliptical::GetSpeedFromPacket(const QByteArray &packet, int index) {
    uint16_t convertedData = ((uint8_t)packet.at(index));
    return ((double)convertedData) / 10.0;
}

void octaneelliptical::btinit(bool startTape) {
    Q_UNUSED(startTape)
    uint8_t initData1[] = {0xa5, 0x04, 0x21, 0x05, 0x01, 0xd9};
    uint8_t initData2[] = {0xa5, 0x04, 0x21, 0x54, 0x01, 0x8a};
    uint8_t initData3[] = {0xa5, 0x04, 0x21, 0x70, 0x01, 0x6e};
    writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, true);
    writeCharacteristic(initData2, sizeof(initData2), QStringLiteral("init"), false, true);
    writeCharacteristic(initData3, sizeof(initData3), QStringLiteral("init"), false, true);

    initDone = true;
}

void octaneelliptical::stateChanged(QLowEnergyService::ServiceState state) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceState>();
    emit debug(QStringLiteral("BTLE stateChanged ") + QString::fromLocal8Bit(metaEnum.valueToKey(state)));
    if (state == QLowEnergyService::RemoteServiceDiscovered) {
        QBluetoothUuid _gattWriteCharacteristicId(QStringLiteral("96dc867f-7a83-4c22-b6be-6381d727aeda"));
        QBluetoothUuid _gattNotify1CharacteristicId(QStringLiteral("96dc867e-7a83-4c22-b6be-6381d727aeda"));

        // qDebug() << gattCommunicationChannelService->characteristics();

        gattWriteCharacteristic = gattCommunicationChannelService->characteristic(_gattWriteCharacteristicId);
        gattNotify1Characteristic = gattCommunicationChannelService->characteristic(_gattNotify1CharacteristicId);
        Q_ASSERT(gattWriteCharacteristic.isValid());
        Q_ASSERT(gattNotify1Characteristic.isValid());

        // establish hook into notifications
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicChanged, this,
                &octaneelliptical::characteristicChanged);
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicWritten, this,
                &octaneelliptical::characteristicWritten);
        connect(gattCommunicationChannelService,
                &QLowEnergyService::errorOccurred,
                this, &octaneelliptical::errorService);
        connect(gattCommunicationChannelService, &QLowEnergyService::descriptorWritten, this,
                &octaneelliptical::descriptorWritten);

        QByteArray descriptor;
        descriptor.append((char)0x01);
        descriptor.append((char)0x00);
        gattCommunicationChannelService->writeDescriptor(
            gattNotify1Characteristic.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration), descriptor);
    }
}

void octaneelliptical::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    emit debug(QStringLiteral("descriptorWritten ") + descriptor.name() + " " + newValue.toHex(' '));

    initRequest = true;
    emit connectedAndDiscovered();
}

void octaneelliptical::characteristicWritten(const QLowEnergyCharacteristic &characteristic,
                                             const QByteArray &newValue) {
    Q_UNUSED(characteristic);
    emit debug(QStringLiteral("characteristicWritten ") + newValue.toHex(' '));
}

void octaneelliptical::serviceScanDone(void) {
    emit debug(QStringLiteral("serviceScanDone"));

    QBluetoothUuid _gattCommunicationChannelServiceId(QStringLiteral("96dc867d-7a83-4c22-b6be-6381d727aeda"));
    gattCommunicationChannelService = m_control->createServiceObject(_gattCommunicationChannelServiceId);
    connect(gattCommunicationChannelService, &QLowEnergyService::stateChanged, this, &octaneelliptical::stateChanged);
    gattCommunicationChannelService->discoverDetails();
}

void octaneelliptical::errorService(QLowEnergyService::ServiceError err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    emit debug(QStringLiteral("octaneelliptical::errorService ") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void octaneelliptical::error(QLowEnergyController::Error err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    emit debug(QStringLiteral("octaneelliptical::error ") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void octaneelliptical::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    emit debug(QStringLiteral("Found new device: ") + device.name() + QStringLiteral(" (") +
               device.address().toString() + ')');
    {
        bluetoothDevice = device;
        m_control = QLowEnergyController::createCentral(bluetoothDevice, this);
        connect(m_control, &QLowEnergyController::serviceDiscovered, this, &octaneelliptical::serviceDiscovered);
        connect(m_control, &QLowEnergyController::discoveryFinished, this, &octaneelliptical::serviceScanDone);
        connect(m_control,
                &QLowEnergyController::errorOccurred,
                this, &octaneelliptical::error);
        connect(m_control, &QLowEnergyController::stateChanged, this, &octaneelliptical::controllerStateChanged);

        connect(m_control,
                &QLowEnergyController::errorOccurred,
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

void octaneelliptical::controllerStateChanged(QLowEnergyController::ControllerState state) {
    qDebug() << QStringLiteral("controllerStateChanged") << state;
    if (state == QLowEnergyController::UnconnectedState && m_control) {
        qDebug() << QStringLiteral("trying to connect back again...");
        initDone = false;
        m_control->connectToDevice();
    }
}

bool octaneelliptical::connected() {
    if (!m_control) {
        return false;
    }
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

bool octaneelliptical::autoPauseWhenSpeedIsZero() {
    if (lastStart == 0 || QDateTime::currentMSecsSinceEpoch() > (lastStart + 10000))
        return true;
    else
        return false;
}

bool octaneelliptical::autoStartWhenSpeedIsGreaterThenZero() {
    if ((lastStop == 0 || QDateTime::currentMSecsSinceEpoch() > (lastStop + 25000)) && requestStop == -1)
        return true;
    else
        return false;
}
