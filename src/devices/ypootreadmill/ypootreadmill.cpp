#include "ypootreadmill.h"
#include "homeform.h"
#include "devices/ftmsbike/ftmsbike.h"
#include "virtualdevices/virtualbike.h"
#include "virtualdevices/virtualtreadmill.h"
#include <QEventLoop>
#include <QSettings>
#include <QThread>
#ifdef Q_OS_ANDROID
#include "keepawakehelper.h"
#endif
#ifdef Q_OS_IOS
extern quint8 QZ_EnableDiscoveryCharsAndDescripttors;
#endif

ypootreadmill::ypootreadmill(bool noWriteResistance, bool noHeartService)
    : noHeartService(noHeartService) {
    Q_UNUSED(noWriteResistance);
    m_watt.setType(metric::METRIC_WATT, deviceType());
    Speed.setType(metric::METRIC_SPEED);
    ypooM5Clock.start();
    connect(&refresh, &QTimer::timeout, this, &ypootreadmill::update);
    refresh.start(200);
}

void ypootreadmill::writeCharacteristic(QLowEnergyService *service, QLowEnergyCharacteristic characteristic,
                                       uint8_t *data, uint8_t size, QString info) {
    if (!service || !characteristic.isValid())
        return;
    QEventLoop loop;
    ypooM5WritePending = true;
    const uint8_t opcode = data[0];
    if (characteristic.uuid() == QBluetoothUuid(quint16(0x2AD9))) {
        if (opcode == FTMS_REQUEST_CONTROL)
            ypooM5ControlGranted = false;
        connect(service, &QLowEnergyService::characteristicChanged, &loop,
                [&loop, opcode](const QLowEnergyCharacteristic &replyCharacteristic, const QByteArray &reply) {
                    if (replyCharacteristic.uuid() == QBluetoothUuid(quint16(0x2AD9)) && reply.size() >= 3 &&
                        uint8_t(reply.at(0)) == FTMS_RESPONSE_CODE && uint8_t(reply.at(1)) == opcode)
                        loop.quit();
                });
        QTimer::singleShot(1000, &loop, &QEventLoop::quit);
    } else {
        connect(service, &QLowEnergyService::characteristicWritten, &loop, &QEventLoop::quit);
        QTimer::singleShot(300, &loop, &QEventLoop::quit);
    }
    const QByteArray packet(reinterpret_cast<const char *>(data), size);
    service->writeCharacteristic(characteristic, packet,
        characteristic.properties() & QLowEnergyCharacteristic::WriteNoResponse
            ? QLowEnergyService::WriteWithoutResponse : QLowEnergyService::WriteWithResponse);
    emit debug(QStringLiteral(" >> ") + packet.toHex(' ') + QStringLiteral(" // ") + info);
    loop.exec();
    ypooM5WritePending = false;
}

void ypootreadmill::btinit() {
    initDone = false;
    if (auto virtualTreadmill = qobject_cast<virtualtreadmill *>(VirtualDevice()))
        virtualTreadmill->clearControlPointWrites();
    if (!gattFTMSService || !gattWriteCharControlPointId.isValid() ||
        !gattWriteCharUnlock.isValid()) {
        emit debug(QStringLiteral("YPOO M5: FTMS control/unlock characteristic unavailable"));
        return;
    }
    requestStart = -1;
    requestSpeed = -1;
    requestInclination = -100;
    for (unsigned attempt = 0; attempt < 3; ++attempt) {
        uint8_t unlock[] = {0x01, 0x00, 0x02, 0x02, 0x00, 0x05, 0x01, 0x01};
        writeCharacteristic(gattFTMSService, gattWriteCharUnlock, unlock, sizeof(unlock),
                            QStringLiteral("YPOO M5 unlock"));
        QThread::msleep(500);
        uint8_t control[] = {FTMS_REQUEST_CONTROL};
        writeCharacteristic(gattFTMSService, gattWriteCharControlPointId, control, sizeof(control),
                            QStringLiteral("YPOO M5 requestControl"));
        if (ypooM5ControlGranted && !ypooM5Control.hasFreshTelemetry(ypooM5Clock.elapsed())) {
            QEventLoop telemetry;
            ypooM5WritePending = true;
            connect(gattFTMSService, &QLowEnergyService::characteristicChanged, &telemetry,
                    [this, &telemetry](const QLowEnergyCharacteristic &, const QByteArray &) {
                        if (ypooM5Control.hasFreshTelemetry(ypooM5Clock.elapsed()))
                            telemetry.quit();
                    });
            QTimer::singleShot(1500, &telemetry, &QEventLoop::quit);
            telemetry.exec();
            ypooM5WritePending = false;
        }
        if (ypooM5ControlGranted && ypooM5Control.hasFreshTelemetry(ypooM5Clock.elapsed())) {
            initDone = true;
            requestStart = -1; // An early click must not replay after an initialization retry.
            requestSpeed = -1;
            requestInclination = -100;
            emit debug(QStringLiteral("YPOO M5: control granted and live telemetry received; ready"));
            if (homeform::singleton())
                homeform::singleton()->setToastRequested(QObject::tr("YPOO M5 ready"));
            return;
        }
        emit debug(QStringLiteral("YPOO M5: initialization attempt %1 unconfirmed; no motor commands sent")
                       .arg(attempt + 1));
    }
    requestStart = -1;
    requestSpeed = -1;
    requestInclination = -100;
    emit debug(QStringLiteral("YPOO M5: initialization failed; motor commands disabled"));
    if (homeform::singleton())
        homeform::singleton()->setToastRequested(QObject::tr("YPOO M5 initialization failed; reconnect required"));
    return;
}

void ypootreadmill::update() {
    if (ypooM5WritePending || !m_control)
        return;
    if (m_control->state() == QLowEnergyController::UnconnectedState) {
        emit disconnected();
        return;
    }
    if (initRequest) {
        initRequest = false;
        btinit();
        return;
    }
    QSettings settings;
    update_metrics(!powerReceivedFromPowerSensor,
                   watts(settings.value(QZSettings::weight, QZSettings::default_weight).toFloat()));
    updateYpooM5Control();
}

void ypootreadmill::updateYpooM5Control() {
    if (!initDone)
        return;

    // Stop takes priority over simultaneous starts and queued workout targets.
    if (requestStop != -1 || requestPause != -1) {
        ypooM5Control.requestStop();
        requestStart = requestStop = requestPause = -1;
        requestSpeed = -1;
        requestInclination = -100;
    } else {
        if (requestStart != -1) {
            ypooM5Control.requestStart();
            requestStart = -1;
        }
        if (requestSpeed != -1) {
            if (!ypooM5Control.requestSpeed(requestSpeed))
                emit debug(QStringLiteral("YPOO M5: speed target rejected (range or treadmill stopped)"));
            requestSpeed = -1;
        }
        if (requestInclination != -100) {
            const double target = treadmillInclinationOverrideReverse(requestInclination);
            if (YP_M1552 && (target < 0.0 || target > 12.0)) {
                emit debug(QStringLiteral("YP-M1552: incline target outside 0-12% range"));
            } else if (!ypooM5Control.requestIncline(target)) {
                emit debug(QStringLiteral("YPOO M5: incline target rejected (range or treadmill stopped)"));
            }
            requestInclination = -100;
        }
    }

    const auto command = ypooM5Control.next(ypooM5Clock.elapsed(), YP_M1552);
    const auto failure = ypooM5Control.takeFailure();
    if (failure != YpooM5Control::Action::None)
        emit debug(QStringLiteral("YPOO M5: command %1 not confirmed by telemetry; retries stopped")
                       .arg(static_cast<int>(failure)));

    switch (command.action) {
    case YpooM5Control::Action::Start: {
        lastStart = QDateTime::currentMSecsSinceEpoch();
        uint8_t start[] = {FTMS_START_RESUME};
        writeCharacteristic(gattFTMSService, gattWriteCharControlPointId, start, sizeof(start),
                            QStringLiteral("YPOO M5 start"));
        break;
    }
    case YpooM5Control::Action::Stop: {
        lastStop = QDateTime::currentMSecsSinceEpoch();
        uint8_t stop[] = {FTMS_STOP_PAUSE, 0x01};
        writeCharacteristic(gattFTMSService, gattWriteCharControlPointId, stop, sizeof(stop),
                            QStringLiteral("YPOO M5 stop"));
        break;
    }
    case YpooM5Control::Action::Speed:
        forceSpeed(command.value);
        break;
    case YpooM5Control::Action::Incline:
        forceIncline(command.value);
        break;
    case YpooM5Control::Action::None:
        break;
    }
}


void ypootreadmill::forceSpeed(double speed) {
    const uint16_t raw = static_cast<uint16_t>(std::lround(speed * 100.0));
    uint8_t packet[] = {FTMS_SET_TARGET_SPEED, uint8_t(raw), uint8_t(raw >> 8)};
    writeCharacteristic(gattFTMSService, gattWriteCharControlPointId, packet, sizeof(packet),
                        QStringLiteral("YPOO M5 speed"));
}

void ypootreadmill::forceIncline(double incline) {
    const double value = YP_M1552 ? YpooM5Control::percentToLevel(incline) : incline;
    const uint16_t raw = static_cast<uint16_t>(static_cast<int16_t>(std::lround(value * 10.0)));
    uint8_t packet[] = {FTMS_SET_TARGET_INCLINATION, uint8_t(raw), uint8_t(raw >> 8)};
    writeCharacteristic(gattFTMSService, gattWriteCharControlPointId, packet, sizeof(packet),
                        QStringLiteral("YPOO M5 incline"));
}

void ypootreadmill::characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &value) {
    emit debug(QStringLiteral(" << ") + characteristic.uuid().toString() + QStringLiteral(" ") + value.toHex(' '));
    if (characteristic.uuid() == QBluetoothUuid(quint16(0x2AD9))) {
        if (value.size() >= 3 && uint8_t(value.at(0)) == FTMS_RESPONSE_CODE) {
            if (uint8_t(value.at(1)) == FTMS_STOP_PAUSE)
                ypooM5Control.stopResponse(uint8_t(value.at(2)) == FTMS_SUCCESS);
            else if (uint8_t(value.at(1)) == FTMS_REQUEST_CONTROL)
                ypooM5ControlGranted = uint8_t(value.at(2)) == FTMS_SUCCESS;
        }
    } else if (characteristic.uuid() == QBluetoothUuid(quint16(0x2ACD))) {
        processTelemetry(value, ypooM5Clock.elapsed());
    }
}

void ypootreadmill::processTelemetry(const QByteArray &value, qint64 now) {
    double speed = 0;
    double incline = 0;
    const auto data = reinterpret_cast<const uint8_t *>(value.constData());
    if (!YpooM5Control::readTelemetry(data, value.size(), speed, incline))
        return;
    if (YP_M1552)
        incline = YpooM5Control::levelToPercent(incline);
    const bool wasRunning = ypooM5Control.running();
    ypooM5Control.telemetry(speed, incline, now);
    if (!wasRunning && ypooM5Control.running())
        emit tapeStarted();

    parseSpeed(speed);
    parseInclination(treadmillInclinationOverride(incline));
    QSettings settings;
    const bool directDistance = settings.value(QZSettings::treadmill_direct_distance,
                                              QZSettings::default_treadmill_direct_distance).toBool();
    const uint16_t flags = data[0] | (uint16_t(data[1]) << 8);
    unsigned offset = 4 + ((flags & 2) ? 2 : 0);
    if (flags & 4) {
        if (directDistance)
            Distance = (data[offset] | (uint32_t(data[offset + 1]) << 8) |
                        (uint32_t(data[offset + 2]) << 16)) / 1000.0;
        offset += 3;
    }
    const double distance = ypooM5Distance.update(now, Speed.value(), !isPaused());
    if (!directDistance)
        Distance += distance;
    offset += 4; // Inclination and ramp angle; readTelemetry validated these fields.
    if (flags & 0x10) offset += 4;
    if (flags & 0x20) offset += 1;
    if (flags & 0x40) offset += 1;
    if (flags & 0x80) offset += 5;

    // Preserve software calories and external heart-rate/cadence integration from the Horizon path.
    const double weight = settings.value(QZSettings::weight, QZSettings::default_weight).toDouble();
    if (lastMetricsUpdate >= 0 && watts(weight))
        KCal += ((0.048 * watts(weight) + 1.19) * weight * 3.5 / 200.0) *
                (now - lastMetricsUpdate) / 60000.0;
    lastMetricsUpdate = now;
    const bool disableHeart = settings.value(QZSettings::heart_ignore_builtin,
                                             QZSettings::default_heart_ignore_builtin).toBool();
    double heart = (flags & 0x100) && !disableHeart ? data[offset] : 0;
#ifdef Q_OS_ANDROID
    if (settings.value(QZSettings::ant_heart, QZSettings::default_ant_heart).toBool())
        heart = KeepAwakeHelper::heart();
#endif
    if (settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name)
            .toString().startsWith(QStringLiteral("Disabled"))) {
        if (heart == 0 || disableHeart)
            update_hr_from_external();
        else
            Heart = heart;
    }
    cadenceFromAppleWatch();
    if (Speed.value() > 0 &&
        !settings.value(QZSettings::garmin_companion, QZSettings::default_garmin_companion).toBool() &&
        settings.value(QZSettings::power_sensor_name, QZSettings::default_power_sensor_name)
            .toString().startsWith(QStringLiteral("Disabled"))) {
        const double cadence = calculateCadenceFromSpeed(Speed.value());
        if (cadence > 0) {
            evaluateStepCount();
            parseCadence(cadence);
        }
    }
    if (speed > 0)
        lastStart = 0;
    else
        lastStop = 0;
    emit debug(QStringLiteral("Current Speed: %1; Inclination: %2; Distance: %3")
                   .arg(Speed.value()).arg(Inclination.value()).arg(Distance.value()));
}

bool ypootreadmill::autoPauseWhenSpeedIsZero() {
    return lastStart == 0 || QDateTime::currentMSecsSinceEpoch() > lastStart + 10000;
}

bool ypootreadmill::autoStartWhenSpeedIsGreaterThenZero() {
    // Start_inner(false) resumes recording without sending another motor Start.
    return initDone && requestStop == -1 && requestPause == -1 && requestSpeed != 0 &&
           ypooM5Control.canResumeRecording(ypooM5Clock.elapsed());
}

bool ypootreadmill::connected() {
    return m_control && m_control->state() == QLowEnergyController::DiscoveredState;
}

void ypootreadmill::stopWhenIdle() {
    // Cancel physical countdown/motion without repeating the UI's workout finalization.
    stop(false);
}

bluetoothdevice::VirtualTreadmillControl ypootreadmill::virtualTreadmillControl() const {
    VirtualTreadmillControl control;
    control.enabled = true;
    control.speedRange = QByteArray::fromHex("640058070a00");
    control.inclineRange = QByteArray::fromHex(YP_M1552 ? "000078000a00" : "c4ff96000a00");
    return control;
}

void ypootreadmill::changeInclinationRequested(double grade, double percentage) {
    changeInclination(grade, qMax(percentage, YP_M1552 ? 0.0 : -6.0));
}

void ypootreadmill::ftmsCharacteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &value) {
    Q_UNUSED(characteristic);
    if (!initDone || value.isEmpty())
        return;
    if (uint8_t(value.at(0)) == FTMS_START_RESUME)
        start();
    else if (uint8_t(value.at(0)) == FTMS_STOP_PAUSE && value.size() >= 2)
        stop(false);
}

void ypootreadmill::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    bluetoothDevice = device;
    YP_M1552 = device.name().startsWith(QStringLiteral("YP-M1552-"), Qt::CaseInsensitive);
#ifdef Q_OS_IOS
    QZ_EnableDiscoveryCharsAndDescripttors = true;
#endif
    m_control = QLowEnergyController::createCentral(device, this);
    connect(m_control, &QLowEnergyController::discoveryFinished, this, &ypootreadmill::serviceScanDone);
    connect(m_control, &QLowEnergyController::stateChanged, this, &ypootreadmill::controllerStateChanged);
    connect(m_control, static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(
                           &QLowEnergyController::error), this, [this](QLowEnergyController::Error) {
        emit debug(QStringLiteral("YPOO M5: ") + m_control->errorString());
        emit disconnected();
    });
    connect(m_control, &QLowEnergyController::connected, this, [this]() { m_control->discoverServices(); });
    connect(m_control, &QLowEnergyController::disconnected, this, &ypootreadmill::disconnected);
    m_control->connectToDevice();
}

void ypootreadmill::serviceScanDone() {
    initRequest = false;
    gattFTMSService = m_control->createServiceObject(QBluetoothUuid(quint16(0x1826)), this);
    if (!gattFTMSService) {
        emit debug(QStringLiteral("YPOO M5: FTMS service unavailable"));
        return;
    }
    connect(gattFTMSService, &QLowEnergyService::stateChanged, this, &ypootreadmill::stateChanged);
    gattFTMSService->discoverDetails();
}

void ypootreadmill::stateChanged(QLowEnergyService::ServiceState state) {
    if (state != QLowEnergyService::ServiceDiscovered)
        return;
    gattWriteCharControlPointId = gattFTMSService->characteristic(QBluetoothUuid(quint16(0x2AD9)));
    gattWriteCharUnlock = gattFTMSService->characteristic(
        QBluetoothUuid(QStringLiteral("d18d2c10-c44c-11e8-a355-529269fb1459")));
    if (!gattWriteCharControlPointId.isValid() || !gattWriteCharUnlock.isValid() ||
        !(gattWriteCharUnlock.properties() & (QLowEnergyCharacteristic::Write | QLowEnergyCharacteristic::WriteNoResponse))) {
        emit debug(QStringLiteral("YPOO M5: FTMS control/unlock characteristic unavailable"));
        return;
    }
    connect(gattFTMSService, &QLowEnergyService::characteristicChanged, this, &ypootreadmill::characteristicChanged);
    connect(gattFTMSService, &QLowEnergyService::descriptorWritten, this, &ypootreadmill::descriptorWritten);
    connect(gattFTMSService, static_cast<void (QLowEnergyService::*)(QLowEnergyService::ServiceError)>(
                                 &QLowEnergyService::error), this, [this](QLowEnergyService::ServiceError) {
        emit debug(QStringLiteral("YPOO M5: FTMS service error"));
    });
    // Subscribe to Control Point indications before treadmill telemetry, as verified on hardware.
    notificationSubscribed = 2;
    for (const auto uuid : {quint16(0x2AD9), quint16(0x2ACD)}) {
        const auto characteristic = gattFTMSService->characteristic(QBluetoothUuid(uuid));
        const auto descriptor = characteristic.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration);
        if (!descriptor.isValid()) {
            emit debug(QStringLiteral("YPOO M5: required subscription unavailable"));
            return;
        }
        gattFTMSService->writeDescriptor(descriptor, QByteArray::fromHex(uuid == 0x2AD9 ? "0200" : "0100"));
    }
    if (!hasVirtualDevice()) {
        QSettings settings;
        if (settings.value(QZSettings::virtual_device_enabled, QZSettings::default_virtual_device_enabled).toBool()) {
            if (!settings.value(QZSettings::virtual_device_force_bike, QZSettings::default_virtual_device_force_bike).toBool()) {
                auto device = new virtualtreadmill(this, noHeartService);
                connect(device, &virtualtreadmill::debug, this, &ypootreadmill::debug);
                connect(device, &virtualtreadmill::changeInclination, this, &ypootreadmill::changeInclinationRequested);
                connect(device, &virtualtreadmill::ftmsCharacteristicChanged, this, &ypootreadmill::ftmsCharacteristicChanged);
                setVirtualDevice(device, VIRTUAL_DEVICE_MODE::PRIMARY);
            } else {
                auto device = new virtualbike(this);
                connect(device, &virtualbike::changeInclination, this, &ypootreadmill::changeInclinationRequested);
                setVirtualDevice(device, VIRTUAL_DEVICE_MODE::ALTERNATIVE);
            }
        }
    }
}

void ypootreadmill::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &value) {
    Q_UNUSED(descriptor);
    Q_UNUSED(value);
    if (notificationSubscribed && --notificationSubscribed == 0) {
        initRequest = true;
        emit connectedAndDiscovered();
    }
}

void ypootreadmill::controllerStateChanged(QLowEnergyController::ControllerState state) {
    if (state != QLowEnergyController::UnconnectedState)
        return;
    initDone = false;
    initRequest = false;
    ypooM5Control.reset();
    ypooM5Distance.reset();
    ypooM5ControlGranted = false;
    lastMetricsUpdate = -1;
    if (auto device = qobject_cast<virtualtreadmill *>(VirtualDevice()))
        device->clearControlPointWrites();
    requestStart = requestStop = requestPause = -1;
    requestSpeed = -1;
    requestInclination = -100;
    m_control->connectToDevice();
}
