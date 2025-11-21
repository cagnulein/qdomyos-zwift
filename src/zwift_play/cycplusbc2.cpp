#include "homeform.h"
#include "cycplusbc2.h"
#include <QBluetoothLocalDevice>
#include <QDateTime>
#include <QEventLoop>
#include <QFile>
#include <QMetaEnum>
#include <QSettings>
#include <QThread>

using namespace std::chrono_literals;

#ifdef Q_OS_IOS
extern quint8 QZ_EnableDiscoveryCharsAndDescripttors;
#endif

// Nordic UART Service UUIDs
static const QBluetoothUuid NORDIC_UART_SERVICE_UUID(QStringLiteral("6e400001-b5a3-f393-e0a9-e50e24dcca9e"));
static const QBluetoothUuid NORDIC_UART_TX_CHAR_UUID(QStringLiteral("6e400003-b5a3-f393-e0a9-e50e24dcca9e"));  // Device -> App (Notify)
static const QBluetoothUuid NORDIC_UART_RX_CHAR_UUID(QStringLiteral("6e400002-b5a3-f393-e0a9-e50e24dcca9e"));  // App -> Device (Write, unused)

cycplusbc2::cycplusbc2(bluetoothdevice *parentDevice) {
#ifdef Q_OS_IOS
    QZ_EnableDiscoveryCharsAndDescripttors = true;
#endif
    this->parentDevice = parentDevice;

    refresh = new QTimer(this);
    connect(refresh, &QTimer::timeout, this, &cycplusbc2::update);
    refresh->start(1000ms);
}

void cycplusbc2::update() {
    if(initDone) {
        countRxTimeout++;
        if(countRxTimeout == 10) {
            if(homeform::singleton())
                homeform::singleton()->setToastRequested("CYCPLUS BC2: Connection timeout!");
        }
    }
}

void cycplusbc2::serviceDiscovered(const QBluetoothUuid &gatt) {
    emit debug(QStringLiteral("serviceDiscovered ") + gatt.toString());
}

void cycplusbc2::disconnectBluetooth() {
    qDebug() << QStringLiteral("cycplusbc2::disconnect") << m_control;

    if (m_control) {
        m_control->disconnectFromDevice();
    }
}

void cycplusbc2::characteristicChanged(const QLowEnergyCharacteristic &characteristic,
                                       const QByteArray &newValue) {
    handleCharacteristicValueChanged(characteristic.uuid(), newValue);
}

void cycplusbc2::handleCharacteristicValueChanged(const QBluetoothUuid &uuid, const QByteArray &newValue) {
    emit packetReceived();
    countRxTimeout = 0;

    qDebug() << QStringLiteral("CYCPLUS BC2 << ") << newValue.toHex(' ') << uuid.toString();

    // Check if this is the TX characteristic (device -> app)
    if(uuid == NORDIC_UART_TX_CHAR_UUID) {
        // Verify packet is 8 bytes as expected
        if(newValue.length() >= 8) {
            uint8_t shiftUpState = static_cast<uint8_t>(newValue[6]);
            uint8_t shiftDownState = static_cast<uint8_t>(newValue[7]);

            // Check for Shift Up state change (pressed to different pressed value)
            if(shiftUpState != 0x00 && lastShiftUpState != 0x00 && shiftUpState != lastShiftUpState) {
                qDebug() << "CYCPLUS BC2: Shift UP triggered" << QString::number(shiftUpState, 16);
                emit plus();
            }

            // Check for Shift Down state change (pressed to different pressed value)
            if(shiftDownState != 0x00 && lastShiftDownState != 0x00 && shiftDownState != lastShiftDownState) {
                qDebug() << "CYCPLUS BC2: Shift DOWN triggered" << QString::number(shiftDownState, 16);
                emit minus();
            }

            // Update last states
            lastShiftUpState = shiftUpState;
            lastShiftDownState = shiftDownState;
        } else {
            qDebug() << "CYCPLUS BC2: Unexpected packet length:" << newValue.length();
        }
    }
}

void cycplusbc2::writeCharacteristic(QLowEnergyService *service, QLowEnergyCharacteristic *writeChar,
                                     uint8_t *data, uint8_t data_len, const QString &info, bool disable_log,
                                     bool wait_for_response) {
#ifdef Q_OS_IOS
#ifndef IO_UNDER_QT
    iOS_cycplusbc2->zwiftClickRemote_WriteCharacteristic(data, data_len, this); // Pass 'this' pointer
    if (!disable_log) {
        QByteArray buffer((const char *)data, data_len);
        qDebug() << QStringLiteral(" >> ") + buffer.toHex(' ') + QStringLiteral(" // ") + info;
    }
    return;
#endif
#endif

    QEventLoop loop;
    QTimer timeout;

    if (service == nullptr || writeChar->isValid() == false) {
        qDebug() << QStringLiteral(
            "cycplusbc2 trying to write before the connection is established");
        return;
    }

    if (wait_for_response) {
        connect(service, &QLowEnergyService::characteristicChanged, &loop, &QEventLoop::quit);
        timeout.singleShot(300ms, &loop, &QEventLoop::quit);
    } else {
        connect(service, &QLowEnergyService::characteristicWritten, &loop, &QEventLoop::quit);
        timeout.singleShot(300ms, &loop, &QEventLoop::quit);
    }

    if (service->state() != QLowEnergyService::ServiceState::ServiceDiscovered ||
        m_control->state() == QLowEnergyController::UnconnectedState) {
        qDebug() << QStringLiteral("writeCharacteristic error because the connection is closed");
        return;
    }

    if (!writeChar->isValid()) {
        qDebug() << QStringLiteral("gattWriteCharacteristic is invalid");
        return;
    }

    if (writeBuffer) {
        delete writeBuffer;
    }
    writeBuffer = new QByteArray((const char *)data, data_len);

    service->writeCharacteristic(*writeChar, *writeBuffer, QLowEnergyService::WriteWithoutResponse);

    if (!disable_log) {
        qDebug() << QStringLiteral(" >> ") + writeBuffer->toHex(' ') + QStringLiteral(" // ") + info;
    }

    if(wait_for_response)
        loop.exec();
}

void cycplusbc2::stateChanged(QLowEnergyService::ServiceState state) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceState>();
    emit debug(QStringLiteral("BTLE stateChanged ") + QString::fromLocal8Bit(metaEnum.valueToKey(state)));

#ifndef Q_OS_IOS
    for (QLowEnergyService *s : qAsConst(gattCommunicationChannelService)) {
        qDebug() << QStringLiteral("stateChanged") << s->serviceUuid() << state;
        if (s->serviceUuid() == NORDIC_UART_SERVICE_UUID) {
            qDebug() << QStringLiteral("cycplusbc2 service discovered");

            if (state != QLowEnergyService::ServiceDiscovered) {
                qDebug() << QStringLiteral("cycplusbc2 service not yet discovered");
                return;
            }

            gattService = s;

            // Get TX characteristic (device -> app, notifications)
            gattNotifyCharacteristic = gattService->characteristic(NORDIC_UART_TX_CHAR_UUID);
            if (!gattNotifyCharacteristic.isValid()) {
                qDebug() << QStringLiteral("cycplusbc2 TX characteristic not found");
                return;
            }

            // Get RX characteristic (app -> device, write - unused for button reading)
            gattWriteCharacteristic = gattService->characteristic(NORDIC_UART_RX_CHAR_UUID);

            connect(gattService, &QLowEnergyService::characteristicChanged, this, &cycplusbc2::characteristicChanged);
            connect(gattService, &QLowEnergyService::characteristicWritten, this, &cycplusbc2::characteristicWritten);
            connect(gattService, &QLowEnergyService::error, this, &cycplusbc2::errorService);
            connect(gattService, &QLowEnergyService::descriptorWritten, this, &cycplusbc2::descriptorWritten);

            // Enable notifications on TX characteristic
            QByteArray descriptor;
            descriptor.append((char)0x01);
            descriptor.append((char)0x00);
            gattService->writeDescriptor(
                gattNotifyCharacteristic.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration),
                descriptor);

            initDone = true;
            emit debug(QStringLiteral("cycplusbc2 initialized and listening for button presses"));
        }
    }
#endif
}

void cycplusbc2::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    emit debug(QStringLiteral("descriptorWritten ") + descriptor.name() + QStringLiteral(" ") + newValue.toHex(' '));
    emit packetReceived();

    initRequest = false;
}

void cycplusbc2::characteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    Q_UNUSED(characteristic);
    emit debug(QStringLiteral("characteristicWritten ") + newValue.toHex(' '));
}

void cycplusbc2::serviceScanDone(void) {
    emit debug(QStringLiteral("serviceScanDone"));

#ifdef Q_OS_ANDROID
    QLowEnergyConnectionParameters c;
    c.setIntervalRange(24, 40);
    c.setLatency(0);
    c.setSupervisionTimeout(420);
    m_control->requestConnectionUpdate(c);
#endif

    if (gattCommunicationChannelService.isEmpty()) {
        qDebug() << QStringLiteral("cycplusbc2 service not found");
        return;
    }

    for (QLowEnergyService *s : qAsConst(gattCommunicationChannelService)) {
        if (s)
            s->discoverDetails();
    }
}

void cycplusbc2::errorService(QLowEnergyService::ServiceError err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    emit debug(QStringLiteral("cycplusbc2::errorService") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void cycplusbc2::error(QLowEnergyController::Error err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    emit debug(QStringLiteral("cycplusbc2::error") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());

    if(err == QLowEnergyController::ConnectionError) {
        emit disconnected();
    }
}

void cycplusbc2::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    emit debug(QStringLiteral("Found device: ") + device.name());

    if (device.name().startsWith(QStringLiteral("CYCPLUS BC2"))) {
        qDebug() << QStringLiteral("CYCPLUS BC2 device found");

        bluetoothDevice = device;

        m_control = QLowEnergyController::createCentral(bluetoothDevice, this);
        connect(m_control, &QLowEnergyController::serviceDiscovered, this, &cycplusbc2::serviceDiscovered);
        connect(m_control, &QLowEnergyController::discoveryFinished, this, &cycplusbc2::serviceScanDone);
        connect(m_control,
                static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error),
                this, &cycplusbc2::error);
        connect(m_control, &QLowEnergyController::stateChanged, this, &cycplusbc2::controllerStateChanged);
        connect(m_control,
                static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error),
                this, [this](QLowEnergyController::Error error) {
                    Q_UNUSED(error);
                    Q_UNUSED(this);
                    emit disconnected();
                });

        m_control->connectToDevice();
    }
}

bool cycplusbc2::connected() {
    if (!m_control) {
        return false;
    }
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

void cycplusbc2::controllerStateChanged(QLowEnergyController::ControllerState state) {
    qDebug() << QStringLiteral("controllerStateChanged") << state;
    if (state == QLowEnergyController::UnconnectedState && m_control) {
        qDebug() << QStringLiteral("trying to connect back again...");

        initDone = false;
        initRequest = false;

        m_control->connectToDevice();
    } else if (state == QLowEnergyController::ConnectedState) {
        emit debug(QStringLiteral("connected to the CYCPLUS BC2"));
    } else if (state == QLowEnergyController::DiscoveredState) {
        qDebug() << QStringLiteral("cycplusbc2 services discovered");

        // Nordic UART Service
        if (!gattCommunicationChannelService.contains(m_control->createServiceObject(NORDIC_UART_SERVICE_UUID))) {
            gattCommunicationChannelService.append(m_control->createServiceObject(NORDIC_UART_SERVICE_UUID));
            connect(gattCommunicationChannelService.constLast(), &QLowEnergyService::stateChanged, this,
                    &cycplusbc2::stateChanged);
            gattCommunicationChannelService.constLast()->discoverDetails();
        }
    }
}
