#include "fitplusrower.h"

#include "homeform.h"
#include "virtualdevices/virtualbike.h"
#include "virtualdevices/virtualrower.h"

#include <QMetaEnum>
#include <QSettings>
#include <chrono>
#include <math.h>

#ifdef Q_OS_ANDROID
#include "keepawakehelper.h"
#endif

using namespace std::chrono_literals;

fitplusrower::fitplusrower(bool noWriteResistance, bool noHeartService, int8_t bikeResistanceOffset,
                           double bikeResistanceGain) {
    m_watt.setType(metric::METRIC_WATT, deviceType());
    Speed.setType(metric::METRIC_SPEED);

    this->noWriteResistance = noWriteResistance;
    this->noHeartService = noHeartService;
    this->bikeResistanceOffset = bikeResistanceOffset;
    this->bikeResistanceGain = bikeResistanceGain;

    refresh = new QTimer(this);
    connect(refresh, &QTimer::timeout, this, &fitplusrower::update);
    refresh->start(200ms);
}

void fitplusrower::writeCharacteristic(uint8_t *data, uint8_t data_len, const QString &info, bool disable_log,
                                       bool wait_for_response) {
    if (!gattCommunicationChannelService || !m_control) {
        qDebug() << QStringLiteral("writeCharacteristic error because the service is not available");
        return;
    }

    QEventLoop loop;
    QTimer timeout;

    if (wait_for_response) {
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicChanged, &loop, &QEventLoop::quit);
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

    QByteArray writeBuffer((const char *)data, data_len);
    if (gattWriteCharacteristic.properties() & QLowEnergyCharacteristic::WriteNoResponse) {
        gattCommunicationChannelService->writeCharacteristic(gattWriteCharacteristic, writeBuffer,
                                                             QLowEnergyService::WriteWithoutResponse);
    } else {
        gattCommunicationChannelService->writeCharacteristic(gattWriteCharacteristic, writeBuffer);
    }

    if (!disable_log)
        qDebug() << QStringLiteral(" >> ") + writeBuffer.toHex(' ') + QStringLiteral(" // ") + info;

    loop.exec();
}

void fitplusrower::forceResistance(resistance_t requestResistance) {
    if (noWriteResistance)
        return;

    static const uint8_t resistanceCommands[32][7] = {
        {0x02, 0x44, 0x05, 0x01, 0xf9, 0xb9, 0x03}, {0x02, 0x44, 0x05, 0x02, 0xf9, 0xba, 0x03},
        {0x02, 0x44, 0x05, 0x03, 0xfa, 0xb8, 0x03}, {0x02, 0x44, 0x05, 0x04, 0xfb, 0xbe, 0x03},
        {0x02, 0x44, 0x05, 0x05, 0xfc, 0xb8, 0x03}, {0x02, 0x44, 0x05, 0x06, 0xfd, 0xba, 0x03},
        {0x02, 0x44, 0x05, 0x07, 0xfe, 0xb8, 0x03}, {0x02, 0x44, 0x05, 0x08, 0xff, 0xb6, 0x03},
        {0x02, 0x44, 0x05, 0x09, 0x00, 0x48, 0x03}, {0x02, 0x44, 0x05, 0x0a, 0x00, 0x4b, 0x03},
        {0x02, 0x44, 0x05, 0x0b, 0x01, 0x4b, 0x03}, {0x02, 0x44, 0x05, 0x0c, 0x03, 0x4e, 0x03},
        {0x02, 0x44, 0x05, 0x0d, 0x03, 0x4f, 0x03}, {0x02, 0x44, 0x05, 0x0e, 0x05, 0x4a, 0x03},
        {0x02, 0x44, 0x05, 0x0f, 0x05, 0x4b, 0x03}, {0x02, 0x44, 0x05, 0x10, 0x06, 0x57, 0x03},
        {0x02, 0x44, 0x05, 0x11, 0x07, 0x57, 0x03}, {0x02, 0x44, 0x05, 0x12, 0x08, 0x5b, 0x03},
        {0x02, 0x44, 0x05, 0x13, 0x09, 0x5b, 0x03}, {0x02, 0x44, 0x05, 0x14, 0x0a, 0x5f, 0x03},
        {0x02, 0x44, 0x05, 0x15, 0x0b, 0x5f, 0x03}, {0x02, 0x44, 0x05, 0x16, 0x0c, 0x5b, 0x03},
        {0x02, 0x44, 0x05, 0x17, 0x0d, 0x5b, 0x03}, {0x02, 0x44, 0x05, 0x18, 0x0e, 0x57, 0x03},
        {0x02, 0x44, 0x05, 0x19, 0x00, 0x58, 0x03}, {0x02, 0x44, 0x05, 0x1a, 0x00, 0x5b, 0x03},
        {0x02, 0x44, 0x05, 0x1b, 0x00, 0x5a, 0x03}, {0x02, 0x44, 0x05, 0x1c, 0x00, 0x5d, 0x03},
        {0x02, 0x44, 0x05, 0x1d, 0x00, 0x5c, 0x03}, {0x02, 0x44, 0x05, 0x1e, 0x00, 0x5f, 0x03},
        {0x02, 0x44, 0x05, 0x1f, 0x00, 0x5e, 0x03}, {0x02, 0x44, 0x05, 0x20, 0x00, 0x61, 0x03}};

    if (requestResistance < 1 || requestResistance > 32)
        return;

    const uint8_t *command = resistanceCommands[requestResistance - 1];
    writeCharacteristic(const_cast<uint8_t *>(command), 7, QStringLiteral("force resistance"), false, true);
}

void fitplusrower::update() {
    if (!m_control)
        return;

    if (m_control->state() == QLowEnergyController::UnconnectedState) {
        emit disconnected();
        return;
    }

    if (initRequest) {
        initRequest = false;
        btinit();
    } else if (bluetoothDevice.isValid() && m_control->state() == QLowEnergyController::DiscoveredState &&
               gattCommunicationChannelService && gattWriteCharacteristic.isValid() &&
               gattNotify1Characteristic.isValid() && initDone) {
        update_metrics(false, watts());

        uint8_t noOpData[] = {0x02, 0x42, 0x42, 0x03};
        uint8_t noOpData1[] = {0x02, 0x43, 0x01, 0x42, 0x03};
        if (counterPoll == 0)
            writeCharacteristic(noOpData, sizeof(noOpData), QStringLiteral("noOp"), false, true);
        else
            writeCharacteristic(noOpData1, sizeof(noOpData1), QStringLiteral("noOp"), false, true);
        counterPoll = counterPoll == 0 ? 1 : 0;

        if (requestResistance != -1) {
            if (requestResistance > max_resistance)
                requestResistance = max_resistance;
            else if (requestResistance <= 0)
                requestResistance = 1;

            if (requestResistance != currentResistance().value()) {
                qDebug() << QStringLiteral("writing resistance ") + QString::number(requestResistance);
                forceResistance(requestResistance);
            }
            requestResistance = -1;
        }

        if (requestStart != -1) {
            qDebug() << QStringLiteral("starting...");
            requestStart = -1;
            emit bikeStarted();
        }

        if (requestStop != -1) {
            qDebug() << QStringLiteral("stopping...");
            requestStop = -1;
        }
    }
}

void fitplusrower::serviceDiscovered(const QBluetoothUuid &gatt) {
    qDebug() << QStringLiteral("serviceDiscovered ") + gatt.toString();
}

void fitplusrower::characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    Q_UNUSED(characteristic);

    QDateTime now = QDateTime::currentDateTime();
    QSettings settings;
    QString heartRateBeltName =
        settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name).toString();

    qDebug() << QStringLiteral(" << ") + newValue.toHex(' ');
    lastPacket = newValue;

    if (newValue.length() != 15 && newValue.length() != 13)
        return;

    if (newValue.length() == 13)
        return;

    resistance_t res = (uint8_t)newValue.at(5);
    if (settings.value(QZSettings::gears_from_bike, QZSettings::default_gears_from_bike).toBool()) {
        qDebug() << QStringLiteral("gears_from_bike") << res << Resistance.value() << gears()
                 << lastRawRequestedResistanceValue << lastRequestedResistance().value() << requestResistance
                 << requestResistanceCompleted;
        if (res != qRound(Resistance.value()) &&
            ((lastRequestedResistance().value() != res && lastRequestedResistance().value() != 0 &&
              requestResistance == -1 && requestResistanceCompleted) ||
             (lastRawRequestedResistanceValue == -1 && requestResistance == -1 && requestResistanceCompleted)) &&
            qRound(Resistance.value()) > 1 && qAbs(res - qRound(Resistance.value())) < 6) {
            int8_t g = gears();
            g += (res - qRound(Resistance.value()));
            qDebug() << QStringLiteral("gears_from_bike APPLIED") << gears() << g;
            lastRawRequestedResistanceValue = -1;
            setGears(g);
        }
    }

    requestResistanceCompleted = true;
    Resistance = res;
    emit resistanceRead(Resistance.value());

    if (settings.value(QZSettings::cadence_sensor_name, QZSettings::default_cadence_sensor_name)
            .toString()
            .startsWith(QStringLiteral("Disabled"))) {
        Cadence = (uint8_t)newValue.at(6);
    }

    if (settings.value(QZSettings::power_sensor_name, QZSettings::default_power_sensor_name)
            .toString()
            .startsWith(QStringLiteral("Disabled"))) {
        m_watt = (double)((((uint8_t)newValue.at(10)) << 8) | ((uint8_t)newValue.at(9))) / 10.0;
    }

    if (m_watt.value() > 0 && Cadence.value() > 0) {
        double ac = 0.01243107769;
        double bc = 1.145964912;
        double cc = -23.50977444;
        double ar = 0.1469553975;
        double br = -5.841344538;
        double cr = 97.62165482;
        double cadenceFactor = ac * pow(Cadence.value(), 2.0) + bc * Cadence.value() + cc;

        if (cadenceFactor > 0) {
            double discriminant = pow(br, 2.0) -
                                  4.0 * ar * (cr - (m_watt.value() * 132.0 / cadenceFactor));
            if (discriminant >= 0) {
                m_pelotonResistance =
                    (((sqrt(discriminant) - br) / (2.0 * ar)) *
                         settings.value(QZSettings::peloton_gain, QZSettings::default_peloton_gain).toDouble()) +
                    settings.value(QZSettings::peloton_offset, QZSettings::default_peloton_offset).toDouble();
            }
        }
    } else {
        m_pelotonResistance = rowerResistanceToPeloton(Resistance.value());
    }

    Speed = metric::calculateSpeedFromPower(watts(), 0, Speed.value(),
                                            fabs(now.msecsTo(Speed.lastChanged()) / 1000.0), 20.0);

    if (watts()) {
        KCal += ((((0.048 * ((double)watts()) + 1.19) *
                   settings.value(QZSettings::weight, QZSettings::default_weight).toFloat() * 3.5) /
                  200.0) /
                 (60000.0 / ((double)lastRefreshCharacteristicChanged.msecsTo(now))));
    }

    Distance += ((Speed.value() / 3600000.0) * ((double)lastRefreshCharacteristicChanged.msecsTo(now)));

    if (Cadence.value() > 0) {
        CrankRevs++;
        LastCrankEventTime += (uint16_t)(1024.0 / (((double)Cadence.value()) / 60.0));
    }

    lastRefreshCharacteristicChanged = now;

#ifdef Q_OS_ANDROID
    if (settings.value(QZSettings::ant_heart, QZSettings::default_ant_heart).toBool())
        Heart = (uint8_t)KeepAwakeHelper::heart();
    else
#endif
    {
        if (heartRateBeltName.startsWith(QStringLiteral("Disabled")))
            update_hr_from_external();
    }

    qDebug() << QStringLiteral("Current Speed: ") + QString::number(Speed.value());
    qDebug() << QStringLiteral("Current Calculate Distance: ") + QString::number(Distance.value());
    qDebug() << QStringLiteral("Current Stroke Rate: ") + QString::number(Cadence.value());
    qDebug() << QStringLiteral("Current Watt: ") + QString::number(watts());
    qDebug() << QStringLiteral("Current Resistance: ") + QString::number(Resistance.value());

    if (m_control->error() != QLowEnergyController::NoError)
        qDebug() << QStringLiteral("QLowEnergyController ERROR!!") << m_control->errorString();
}

resistance_t fitplusrower::pelotonToBikeResistance(int pelotonResistance) {
    QSettings settings;
    double adjustedPelotonResistance =
        (pelotonResistance - settings.value(QZSettings::peloton_offset, QZSettings::default_peloton_offset).toDouble()) /
        settings.value(QZSettings::peloton_gain, QZSettings::default_peloton_gain).toDouble();
    return (adjustedPelotonResistance * max_resistance) / 100;
}

double fitplusrower::rowerResistanceToPeloton(double resistance) {
    QSettings settings;
    return (((resistance * 100) / max_resistance) *
            settings.value(QZSettings::peloton_gain, QZSettings::default_peloton_gain).toDouble()) +
           settings.value(QZSettings::peloton_offset, QZSettings::default_peloton_offset).toDouble();
}

void fitplusrower::btinit() {
    uint8_t initData1[] = {0xaa, 0x01, 0x00, 0x01, 0x55};
    writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, true);
    initDone = true;

    if (lastResistanceBeforeDisconnection != -1) {
        qDebug() << QStringLiteral("forcing resistance to ") + QString::number(lastResistanceBeforeDisconnection) +
                        QStringLiteral(". It was the last value before the disconnection.");
        forceResistance(lastResistanceBeforeDisconnection);
        lastResistanceBeforeDisconnection = -1;
    }
}

void fitplusrower::stateChanged(QLowEnergyService::ServiceState state) {
    QBluetoothUuid gattWriteCharacteristicId((quint16)0xfff2);
    QBluetoothUuid gattNotify1CharacteristicId((quint16)0xfff1);

    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceState>();
    qDebug() << QStringLiteral("BTLE stateChanged ") + QString::fromLocal8Bit(metaEnum.valueToKey(state));

    if (state != QLowEnergyService::ServiceDiscovered)
        return;

    gattWriteCharacteristic = gattCommunicationChannelService->characteristic(gattWriteCharacteristicId);
    gattNotify1Characteristic = gattCommunicationChannelService->characteristic(gattNotify1CharacteristicId);
    Q_ASSERT(gattWriteCharacteristic.isValid());
    Q_ASSERT(gattNotify1Characteristic.isValid());

    connect(gattCommunicationChannelService, &QLowEnergyService::characteristicChanged, this,
            &fitplusrower::characteristicChanged);
    connect(gattCommunicationChannelService, &QLowEnergyService::characteristicWritten, this,
            &fitplusrower::characteristicWritten);
    connect(gattCommunicationChannelService,
            static_cast<void (QLowEnergyService::*)(QLowEnergyService::ServiceError)>(&QLowEnergyService::error), this,
            &fitplusrower::errorService);
    connect(gattCommunicationChannelService, &QLowEnergyService::descriptorWritten, this,
            &fitplusrower::descriptorWritten);

    if (!firstStateChanged && !hasVirtualDevice()) {
        QSettings settings;
        bool virtualDeviceEnabled =
            settings.value(QZSettings::virtual_device_enabled, QZSettings::default_virtual_device_enabled).toBool();
        bool virtualDeviceRower =
            settings.value(QZSettings::virtual_device_rower, QZSettings::default_virtual_device_rower).toBool();

        if (virtualDeviceEnabled) {
            if (!virtualDeviceRower) {
                qDebug() << QStringLiteral("creating virtual bike interface...");
                auto virtualBike =
                    new virtualbike(this, noWriteResistance, noHeartService, bikeResistanceOffset, bikeResistanceGain);
                connect(virtualBike, &virtualbike::changeInclination, this, &fitplusrower::changeInclination);
                setVirtualDevice(virtualBike, VIRTUAL_DEVICE_MODE::PRIMARY);
            } else {
                qDebug() << QStringLiteral("creating virtual rower interface...");
                auto virtualRower = new virtualrower(this, noWriteResistance, noHeartService);
                setVirtualDevice(virtualRower, VIRTUAL_DEVICE_MODE::PRIMARY);
            }
        }
    }
    firstStateChanged = 1;

    QByteArray descriptor;
    descriptor.append((char)0x01);
    descriptor.append((char)0x00);
    gattCommunicationChannelService->writeDescriptor(
        gattNotify1Characteristic.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration), descriptor);
}

void fitplusrower::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    qDebug() << QStringLiteral("descriptorWritten ") + descriptor.name() + " " + newValue.toHex(' ');
    initRequest = true;
    emit connectedAndDiscovered();
}

void fitplusrower::characteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    Q_UNUSED(characteristic);
    qDebug() << QStringLiteral("characteristicWritten ") + newValue.toHex(' ');
}

void fitplusrower::serviceScanDone() {
    qDebug() << QStringLiteral("serviceScanDone");

    QBluetoothUuid gattCommunicationChannelServiceId((quint16)0xfff0);
    gattCommunicationChannelService = m_control->createServiceObject(gattCommunicationChannelServiceId);

    if (gattCommunicationChannelService) {
        connect(gattCommunicationChannelService, &QLowEnergyService::stateChanged, this, &fitplusrower::stateChanged);
        gattCommunicationChannelService->discoverDetails();
        return;
    }

    qDebug() << gattCommunicationChannelServiceId << "not found!";
    QLowEnergyService *ftmsService = m_control->createServiceObject(QBluetoothUuid((quint16)0x1826));
    if (ftmsService) {
        ftmsService->deleteLater();
        QSettings settings;
        settings.setValue(QZSettings::ftms_rower, bluetoothDevice.name());
        qDebug() << "forcing FTMS rower since it has FTMS";
        if (homeform::singleton())
            homeform::singleton()->setToastRequested("FTMS rower found, restart the app to apply the change!");
    }
}

void fitplusrower::errorService(QLowEnergyService::ServiceError err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    qDebug() << QStringLiteral("fitplusrower::errorService") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
                    m_control->errorString();
}

void fitplusrower::error(QLowEnergyController::Error err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    qDebug() << QStringLiteral("fitplusrower::error") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
                    m_control->errorString();
}

void fitplusrower::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    qDebug() << QStringLiteral("Found new device: ") + device.name() + " (" + device.address().toString() + ')';
    bluetoothDevice = device;

    qDebug() << QStringLiteral("Merach R28 FitPlus rower protocol enabled!");

    m_control = QLowEnergyController::createCentral(bluetoothDevice, this);
    connect(m_control, &QLowEnergyController::serviceDiscovered, this, &fitplusrower::serviceDiscovered);
    connect(m_control, &QLowEnergyController::discoveryFinished, this, &fitplusrower::serviceScanDone);
    connect(m_control,
            static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error), this,
            &fitplusrower::error);
    connect(m_control, &QLowEnergyController::stateChanged, this, &fitplusrower::controllerStateChanged);

    connect(m_control,
            static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error), this,
            [this](QLowEnergyController::Error error) {
                Q_UNUSED(error);
                qDebug() << QStringLiteral("Cannot connect to remote device.");
                emit disconnected();
            });
    connect(m_control, &QLowEnergyController::connected, this, [this]() {
        qDebug() << QStringLiteral("Controller connected. Search services...");
        m_control->discoverServices();
    });
    connect(m_control, &QLowEnergyController::disconnected, this, [this]() {
        qDebug() << QStringLiteral("LowEnergy controller disconnected");
        emit disconnected();
    });

    m_control->connectToDevice();
}

bool fitplusrower::connected() {
    if (!m_control)
        return false;
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

uint16_t fitplusrower::watts() {
    if (currentCadence().value() == 0)
        return 0;
    return m_watt.value();
}

void fitplusrower::controllerStateChanged(QLowEnergyController::ControllerState state) {
    qDebug() << QStringLiteral("controllerStateChanged") << state;
    if (state == QLowEnergyController::UnconnectedState && m_control) {
        lastResistanceBeforeDisconnection = Resistance.value();
        qDebug() << QStringLiteral("trying to connect back again...");
        initDone = false;
        m_control->connectToDevice();
    }
}
