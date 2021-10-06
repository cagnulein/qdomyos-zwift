#include "trxappgateusbtreadmill.h"
#include "keepawakehelper.h"
#include "virtualtreadmill.h"
#include <QBluetoothLocalDevice>
#include <QDateTime>
#include <QEventLoop>
#include <QFile>
#include <QMetaEnum>
#include <QSettings>
#include <QThread>
#include <chrono>

using namespace std::chrono_literals;

trxappgateusbtreadmill::trxappgateusbtreadmill() {
    m_watt.setType(metric::METRIC_WATT);
    Speed.setType(metric::METRIC_SPEED);
    refresh = new QTimer(this);
    initDone = false;
    connect(refresh, &QTimer::timeout, this, &trxappgateusbtreadmill::update);
    refresh->start(200ms);
}

void trxappgateusbtreadmill::writeCharacteristic(uint8_t *data, uint8_t data_len, const QString &info, bool disable_log,
                                                 bool wait_for_response) {
    QEventLoop loop;
    QTimer timeout;

    if (wait_for_response) {
        connect(this, &trxappgateusbtreadmill::packetReceived, &loop, &QEventLoop::quit);
        timeout.singleShot(300ms, &loop, &QEventLoop::quit);
    } else {
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicWritten, &loop, &QEventLoop::quit);
        timeout.singleShot(300ms, &loop, &QEventLoop::quit);
    }

    gattCommunicationChannelService->writeCharacteristic(gattWriteCharacteristic,
                                                         QByteArray((const char *)data, data_len));

    if (!disable_log) {
        emit debug(QStringLiteral(" >> ") + QByteArray((const char *)data, data_len).toHex(' ') +
                   QStringLiteral(" // ") + info);
    }

    loop.exec();

    if (timeout.isActive() == false) {
        emit debug(QStringLiteral(" exit for timeout"));
    }
}

void trxappgateusbtreadmill::forceSpeedOrIncline(double requestSpeed, double requestIncline) {
    Q_UNUSED(requestSpeed);
    Q_UNUSED(requestIncline);
}

bool trxappgateusbtreadmill::changeFanSpeed(uint8_t speed) {
    Q_UNUSED(speed);
    return false;
}

void trxappgateusbtreadmill::update() {
    // qDebug() << treadmill.isValid() << m_control->state() << gattCommunicationChannelService <<
    // gattWriteCharacteristic.isValid() << gattNotifyCharacteristic.isValid() << initDone;

    if (!m_control) {
        return;
    }

    if (m_control->state() == QLowEnergyController::UnconnectedState) {
        emit disconnected();
        return;
    }

    if (initRequest) {
        initRequest = false;
        btinit(false);
    } else if (bluetoothDevice.isValid() && m_control->state() == QLowEnergyController::DiscoveredState &&
               gattCommunicationChannelService && gattWriteCharacteristic.isValid() &&
               gattNotifyCharacteristic.isValid() && initDone) {
        QSettings settings;
        update_metrics(true, watts(settings.value(QStringLiteral("weight"), 75.0).toFloat()));

        // updating the treadmill console every second
        if (sec1update++ == (1000 / refresh->interval())) {
            sec1update = 0;
            // updateDisplay(elapsed);
        }

        bool toorx30 = settings.value(QStringLiteral("toorx_3_0"), false).toBool();
        if (toorx30 == false) {
            const uint8_t noOpData[] = {0xf0, 0xa2, 0x01, 0xd3, 0x66};
            writeCharacteristic((uint8_t *)noOpData, sizeof(noOpData), QStringLiteral("noOp"), false, true);
        } else {
            const uint8_t noOpData[] = {0xf0, 0xa2, 0x23, 0xd3, 0x88};
            writeCharacteristic((uint8_t *)noOpData, sizeof(noOpData), QStringLiteral("noOp"), false, true);
        }

        if (requestSpeed != -1) {
            if (requestSpeed != currentSpeed().value()) {
                emit debug(QStringLiteral("writing speed ") + QString::number(requestSpeed));
                double inc = Inclination.value();
                if (requestInclination != -1) {
                    inc = requestInclination;
                    requestInclination = -1;
                }
                forceSpeedOrIncline(requestSpeed, inc);
            }
            requestSpeed = -1;
        }
        if (requestInclination != -1) {
            if (requestInclination != currentInclination().value()) {
                emit debug(QStringLiteral("writing incline ") + QString::number(requestInclination));
                double speed = currentSpeed().value();
                if (requestSpeed != -1) {
                    speed = requestSpeed;
                    requestSpeed = -1;
                }
                forceSpeedOrIncline(speed, requestInclination);
            }
            requestInclination = -1;
        }
        if (requestStart != -1) {
            emit debug(QStringLiteral("starting..."));
            // btinit(true);
            if (toorx30 == false) {
                const uint8_t startTape[] = {0xf0, 0xa5, 0x01, 0xd3, 0x02, 0x6b};
                writeCharacteristic((uint8_t *)startTape, sizeof(startTape), QStringLiteral("startTape"), false, true);
            } else {
                const uint8_t startTape[] = {0xf0, 0xa5, 0x23, 0xd3, 0x02, 0x8d};
                writeCharacteristic((uint8_t *)startTape, sizeof(startTape), QStringLiteral("startTape"), false, true);
            }

            requestStart = -1;
            emit tapeStarted();
        }
        if (requestStop != -1) {
            emit debug(QStringLiteral("stopping..."));
            // writeCharacteristic(initDataF0C800B8, sizeof(initDataF0C800B8), "stop tape");
            requestStop = -1;
        }
        if (requestIncreaseFan != -1) {
            emit debug(QStringLiteral("increasing fan speed..."));
            changeFanSpeed(FanSpeed + 1);
            requestIncreaseFan = -1;
        } else if (requestDecreaseFan != -1) {
            emit debug(QStringLiteral("decreasing fan speed..."));
            changeFanSpeed(FanSpeed - 1);
            requestDecreaseFan = -1;
        }
    }
}

void trxappgateusbtreadmill::serviceDiscovered(const QBluetoothUuid &gatt) {
    emit debug(QStringLiteral("serviceDiscovered ") + gatt.toString());
}

void trxappgateusbtreadmill::characteristicChanged(const QLowEnergyCharacteristic &characteristic,
                                                   const QByteArray &newValue) {
    // qDebug() << "characteristicChanged" << characteristic.uuid() << newValue << newValue.length();
    Q_UNUSED(characteristic);
    QSettings settings;
    QString heartRateBeltName =
        settings.value(QStringLiteral("heart_rate_belt_name"), QStringLiteral("Disabled")).toString();
    emit packetReceived();

    emit debug(QStringLiteral(" << ") + newValue.toHex(' '));

    lastPacket = newValue;
    if (newValue.length() != 19) {
        return;
    }

    if (treadmill_type == TYPE::IRUNNING) {
        if (newValue.at(15) == 0x03 && newValue.at(16) == 0x02 && readyToStart == false) {
            readyToStart = true;
            requestStart = 1;
        }
    } else {
        if (newValue.at(16) == 0x04 && newValue.at(17) == 0x03 && readyToStart == false) {
            readyToStart = true;
            requestStart = 1;
        }
    }

    double speed = GetSpeedFromPacket(newValue);
    double incline = GetInclinationFromPacket(newValue);
    double kcal = GetKcalFromPacket(newValue);
    double distance = GetDistanceFromPacket(newValue);

#ifdef Q_OS_ANDROID
    if (settings.value("ant_heart", false).toBool())
        Heart = (uint8_t)KeepAwakeHelper::heart();
    else
#endif
    {
        if (heartRateBeltName.startsWith(QStringLiteral("Disabled"))) {
            Heart = 0;
        }
    }
    FanSpeed = 0;

    if (!firstCharChanged) {
        DistanceCalculated += ((speed / 3600.0) / (1000.0 / (lastTimeCharChanged.msecsTo(QTime::currentTime()))));
    }

    emit debug(QStringLiteral("Current speed: ") + QString::number(speed));
    emit debug(QStringLiteral("Current incline: ") + QString::number(incline));
    emit debug(QStringLiteral("Current heart: ") + QString::number(Heart.value()));
    emit debug(QStringLiteral("Current KCal: ") + QString::number(kcal));
    emit debug(QStringLiteral("Current Distance: ") + QString::number(distance));
    emit debug(QStringLiteral("Current Elapsed from the treadmill (not used): ") +
               QString::number(GetElapsedFromPacket(newValue)));
    emit debug(QStringLiteral("Current Distance Calculated: ") + QString::number(DistanceCalculated));

    if (m_control->error() != QLowEnergyController::NoError) {
        qDebug() << QStringLiteral("QLowEnergyController ERROR!!") << m_control->errorString();
    }

    Speed = speed;
    Inclination = incline;
    KCal = kcal;
    Distance = distance;

    lastTimeCharChanged = QTime::currentTime();
    firstCharChanged = false;
}

uint16_t trxappgateusbtreadmill::GetElapsedFromPacket(const QByteArray &packet) {
    uint16_t convertedData = (packet.at(4) - 1);
    convertedData += ((packet.at(5) - 1) * 60);
    return convertedData;
}

double trxappgateusbtreadmill::GetSpeedFromPacket(const QByteArray &packet) {
    uint16_t convertedData = (packet.at(13) - 1) + ((packet.at(12) - 1) * 100);
    double data = (double)(convertedData) / 10.0f;
    return data;
}

double trxappgateusbtreadmill::GetKcalFromPacket(const QByteArray &packet) {
    uint16_t convertedData = ((packet.at(8) - 1) << 8) | (packet.at(9) - 1);
    return (double)(convertedData);
}

double trxappgateusbtreadmill::GetDistanceFromPacket(const QByteArray &packet) {
    uint16_t convertedData = ((packet.at(6) - 1) << 8) | (packet.at(7) - 1);
    double data = ((double)(convertedData)) / 100.0f;
    return data;
}

double trxappgateusbtreadmill::GetInclinationFromPacket(const QByteArray &packet) {
    uint16_t convertedData = packet.at(14);
    double data = (convertedData - 1);
    if (data < 0) {
        return 0;
    }
    return data;
}

void trxappgateusbtreadmill::btinit(bool startTape) {
    Q_UNUSED(startTape);
    QSettings settings;
    bool toorx30 = settings.value(QStringLiteral("toorx_3_0"), false).toBool();

    if (toorx30 == false) {
        const uint8_t initData1[] = {0xf0, 0xa0, 0x01, 0x01, 0x92};
        const uint8_t initData2[] = {0xf0, 0xa5, 0x01, 0xd3, 0x04, 0x6d};
        const uint8_t initData3[] = {0xf0, 0xa0, 0x01, 0xd3, 0x64};
        const uint8_t initData4[] = {0xf0, 0xa1, 0x01, 0xd3, 0x65};
        const uint8_t initData5[] = {0xf0, 0xa3, 0x01, 0xd3, 0x01, 0x15, 0x01, 0x02, 0x51, 0x01, 0x51, 0x23};
        const uint8_t initData6[] = {0xf0, 0xa4, 0x01, 0xd3, 0x01, 0x01, 0x01, 0x01,
                                     0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x73};
        const uint8_t initData7[] = {0xf0, 0xaf, 0x01, 0xd3, 0x02, 0x75};

        writeCharacteristic((uint8_t *)initData1, sizeof(initData1), QStringLiteral("init"), false, true);
        if (treadmill_type == TYPE::IRUNNING) {
            QThread::msleep(400);
        }
        writeCharacteristic((uint8_t *)initData2, sizeof(initData2), QStringLiteral("init"), false, true);
        if (treadmill_type == TYPE::IRUNNING) {
            QThread::msleep(400);
        }
        writeCharacteristic((uint8_t *)initData3, sizeof(initData3), QStringLiteral("init"), false, true);
        if (treadmill_type == TYPE::IRUNNING) {
            QThread::msleep(400);
        }
        writeCharacteristic((uint8_t *)initData4, sizeof(initData4), QStringLiteral("init"), false, true);
        if (treadmill_type == TYPE::IRUNNING) {
            QThread::msleep(400);
        }
        writeCharacteristic((uint8_t *)initData3, sizeof(initData3), QStringLiteral("init"), false, true);
        if (treadmill_type == TYPE::IRUNNING) {
            QThread::msleep(400);
        }
        if (treadmill_type == TYPE::IRUNNING) {
            writeCharacteristic((uint8_t *)initData4, sizeof(initData4), QStringLiteral("init"), false, true);
            QThread::msleep(400);
            writeCharacteristic((uint8_t *)initData3, sizeof(initData3), QStringLiteral("init"), false, true);
            QThread::msleep(400);
            writeCharacteristic((uint8_t *)initData3, sizeof(initData3), QStringLiteral("init"), false, true);
            QThread::msleep(400);
        }
        writeCharacteristic((uint8_t *)initData5, sizeof(initData5), QStringLiteral("init"), false, true);
        if (treadmill_type == TYPE::IRUNNING) {
            QThread::msleep(400);
        }
        writeCharacteristic((uint8_t *)initData6, sizeof(initData6), QStringLiteral("init"), false, true);
        if (treadmill_type == TYPE::IRUNNING) {
            QThread::msleep(400);
        }
        writeCharacteristic((uint8_t *)initData7, sizeof(initData7), QStringLiteral("init"), false, true);
        if (treadmill_type == TYPE::IRUNNING) {
            QThread::msleep(400);
        }
    } else {
        const uint8_t initData1[] = {0xf0, 0xa0, 0x01, 0x01, 0x92};
        const uint8_t initData2[] = {0xf0, 0xa5, 0x23, 0xd3, 0x04, 0x8f};
        const uint8_t initData3[] = {0xf0, 0xa0, 0x23, 0xd3, 0x86};
        const uint8_t initData4[] = {0xf0, 0xa1, 0x23, 0xd3, 0x87};
        const uint8_t initData5[] = {0xf0, 0xa3, 0x23, 0xd3, 0x01, 0x15, 0x01, 0x02, 0x51, 0x01, 0x51, 0x45};
        const uint8_t initData6[] = {0xf0, 0xa4, 0x23, 0xd3, 0x01, 0x01, 0x01, 0x01,
                                     0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x95};
        const uint8_t initData7[] = {0xf0, 0xaf, 0x23, 0xd3, 0x02, 0x97};

        writeCharacteristic((uint8_t *)initData1, sizeof(initData1), QStringLiteral("init"), false, true);
        writeCharacteristic((uint8_t *)initData2, sizeof(initData2), QStringLiteral("init"), false, true);
        writeCharacteristic((uint8_t *)initData3, sizeof(initData3), QStringLiteral("init"), false, true);
        writeCharacteristic((uint8_t *)initData4, sizeof(initData4), QStringLiteral("init"), false, true);
        writeCharacteristic((uint8_t *)initData5, sizeof(initData5), QStringLiteral("init"), false, true);
        writeCharacteristic((uint8_t *)initData6, sizeof(initData6), QStringLiteral("init"), false, true);
        writeCharacteristic((uint8_t *)initData7, sizeof(initData7), QStringLiteral("init"), false, true);
    }
    initDone = true;
}

void trxappgateusbtreadmill::stateChanged(QLowEnergyService::ServiceState state) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceState>();
    emit debug(QStringLiteral("BTLE stateChanged ") + QString::fromLocal8Bit(metaEnum.valueToKey(state)));

    if (state == QLowEnergyService::ServiceDiscovered) {
        auto characteristics_list = gattCommunicationChannelService->characteristics();
        for (const QLowEnergyCharacteristic &c : qAsConst(characteristics_list)) {
            emit debug("characteristic " + c.uuid().toString());
        }

        QString uuidWrite = QStringLiteral("0000fff2-0000-1000-8000-00805f9b34fb");
        QString uuidNotify = QStringLiteral("0000fff1-0000-1000-8000-00805f9b34fb");

        if (treadmill_type == TYPE::IRUNNING) {
            uuidWrite = QStringLiteral("49535343-8841-43f4-a8d4-ecbe34729bb3");
            uuidNotify = QStringLiteral("49535343-1E4D-4BD9-BA61-23C647249616");
        }

        QBluetoothUuid _gattWriteCharacteristicId((QString)uuidWrite);
        QBluetoothUuid _gattNotifyCharacteristicId((QString)uuidNotify);

        gattWriteCharacteristic = gattCommunicationChannelService->characteristic(_gattWriteCharacteristicId);
        gattNotifyCharacteristic = gattCommunicationChannelService->characteristic(_gattNotifyCharacteristicId);
        Q_ASSERT(gattWriteCharacteristic.isValid());
        Q_ASSERT(gattNotifyCharacteristic.isValid());

        // establish hook into notifications
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicChanged, this,
                &trxappgateusbtreadmill::characteristicChanged);
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicWritten, this,
                &trxappgateusbtreadmill::characteristicWritten);
        connect(gattCommunicationChannelService,
                static_cast<void (QLowEnergyService::*)(QLowEnergyService::ServiceError)>(&QLowEnergyService::error),
                this, &trxappgateusbtreadmill::errorService);
        connect(gattCommunicationChannelService, &QLowEnergyService::descriptorWritten, this,
                &trxappgateusbtreadmill::descriptorWritten);

        // ******************************************* virtual treadmill init *************************************
        if (!firstVirtualTreadmill && !virtualTreadMill) {
            QSettings settings;
            bool virtual_device_enabled = settings.value(QStringLiteral("virtual_device_enabled"), true).toBool();
            if (virtual_device_enabled) {
                emit debug(QStringLiteral("creating virtual treadmill interface..."));
                virtualTreadMill = new virtualtreadmill(this, false);
                connect(virtualTreadMill, &virtualtreadmill::debug, this, &trxappgateusbtreadmill::debug);
            }
        }
        firstVirtualTreadmill = 1;
        // ********************************************************************************************************

        QByteArray descriptor;
        descriptor.append((char)0x01);
        descriptor.append((char)0x00);
        gattCommunicationChannelService->writeDescriptor(
            gattNotifyCharacteristic.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration), descriptor);
    }
}

void trxappgateusbtreadmill::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    emit debug(QStringLiteral("descriptorWritten ") + descriptor.name() + " " + newValue.toHex(' '));

    initRequest = true;
    emit connectedAndDiscovered();
}

void trxappgateusbtreadmill::characteristicWritten(const QLowEnergyCharacteristic &characteristic,
                                                   const QByteArray &newValue) {
    Q_UNUSED(characteristic);
    emit debug(QStringLiteral("characteristicWritten ") + newValue.toHex(' '));
}

void trxappgateusbtreadmill::serviceScanDone(void) {
    emit debug(QStringLiteral("serviceScanDone"));

    QString uuid = QStringLiteral("0000fff0-0000-1000-8000-00805f9b34fb");
    if (treadmill_type == TYPE::IRUNNING) {
        uuid = QStringLiteral("49535343-FE7D-4AE5-8FA9-9FAFD205E455");
    }

    QBluetoothUuid _gattCommunicationChannelServiceId((QString)uuid);
    gattCommunicationChannelService = m_control->createServiceObject(_gattCommunicationChannelServiceId);

    if (gattCommunicationChannelService == nullptr) {
        qDebug() << QStringLiteral("invalid service") << uuid;
        return;
    }

    connect(gattCommunicationChannelService, &QLowEnergyService::stateChanged, this,
            &trxappgateusbtreadmill::stateChanged);
    gattCommunicationChannelService->discoverDetails();
}

void trxappgateusbtreadmill::errorService(QLowEnergyService::ServiceError err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    emit debug(QStringLiteral("trxappgateusbtreadmill::errorService") +
               QString::fromLocal8Bit(metaEnum.valueToKey(err)) + m_control->errorString());
}

void trxappgateusbtreadmill::error(QLowEnergyController::Error err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    emit debug(QStringLiteral("trxappgateusbtreadmill::error") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void trxappgateusbtreadmill::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    emit debug(QStringLiteral("Found new device: ") + device.name() + QStringLiteral(" (") +
               device.address().toString() + ')');
    if (device.name().startsWith(QStringLiteral("TOORX")) || device.name().startsWith(QStringLiteral("V-RUN")) ||
        device.name().startsWith(QStringLiteral("FS-")) || device.name().startsWith(QStringLiteral("i-Console+")) ||
        device.name().startsWith(QStringLiteral("i-Running")) || device.name().startsWith(QStringLiteral("F63")) ||
        device.name().toUpper().startsWith(QStringLiteral("XT485"))) {
        if (device.name().startsWith(QStringLiteral("i-Running")) ||
            device.name().toUpper().startsWith(QStringLiteral("ICONSOLE+")) ||
            device.name().startsWith(QStringLiteral("i-Console+")) || device.name().startsWith(QStringLiteral("F63")) ||
            device.name().toUpper().startsWith(QStringLiteral("XT485"))) {
            treadmill_type = TYPE::IRUNNING;
        } else {
            treadmill_type = TYPE::TRXAPPGATE;
        }

        bluetoothDevice = device;
        m_control = QLowEnergyController::createCentral(bluetoothDevice, this);
        connect(m_control, &QLowEnergyController::serviceDiscovered, this, &trxappgateusbtreadmill::serviceDiscovered);
        connect(m_control, &QLowEnergyController::discoveryFinished, this, &trxappgateusbtreadmill::serviceScanDone);
        connect(m_control,
                static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error),
                this, &trxappgateusbtreadmill::error);
        connect(m_control, &QLowEnergyController::stateChanged, this, &trxappgateusbtreadmill::controllerStateChanged);

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

bool trxappgateusbtreadmill::connected() {
    if (!m_control) {
        return false;
    }
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

void *trxappgateusbtreadmill::VirtualTreadMill() { return virtualTreadMill; }

void *trxappgateusbtreadmill::VirtualDevice() { return VirtualTreadMill(); }

double trxappgateusbtreadmill::odometer() { return DistanceCalculated; }

void trxappgateusbtreadmill::controllerStateChanged(QLowEnergyController::ControllerState state) {
    qDebug() << QStringLiteral("controllerStateChanged") << state;
    if (state == QLowEnergyController::UnconnectedState && m_control) {
        qDebug() << QStringLiteral("trying to connect back again...");
        initDone = false;
        m_control->connectToDevice();
    }
}
