#include "cycplusbc2controller.h"
#include "homeform.h"
#include <QBluetoothLocalDevice>
#include <QDateTime>
#include <QEventLoop>
#include <QFile>
#include <QMetaEnum>
#include <QSettings>
#include <QThread>

using namespace std::chrono_literals;

const QBluetoothUuid cycplusbc2controller::SERVICE_UUID =
    QBluetoothUuid(QStringLiteral("6e400001-b5a3-f393-e0a9-e50e24dcca9e"));
const QBluetoothUuid cycplusbc2controller::TX_CHARACTERISTIC_UUID =
    QBluetoothUuid(QStringLiteral("6e400003-b5a3-f393-e0a9-e50e24dcca9e"));

cycplusbc2controller::cycplusbc2controller(bluetoothdevice *parentDevice) {
    this->parentDevice = parentDevice;
}

void cycplusbc2controller::serviceDiscovered(const QBluetoothUuid &gatt) {
    emit debug(QStringLiteral("serviceDiscovered ") + gatt.toString());
}

void cycplusbc2controller::disconnectBluetooth() {
    qDebug() << QStringLiteral("cycplusbc2controller::disconnect") << m_control;

    if (m_control) {
        m_control->disconnectFromDevice();
    }
}

void cycplusbc2controller::characteristicChanged(const QLowEnergyCharacteristic &characteristic,
                                                 const QByteArray &newValue) {
    if (characteristic.uuid() != TX_CHARACTERISTIC_UUID) {
        return;
    }

    qDebug() << QStringLiteral("cycplusbc2controller << ") << newValue.toHex(' ');

    if (newValue.size() <= 7) {
        emit debug(QStringLiteral("CYCPLUS BC2 received unexpected packet: ") + newValue.toHex(' '));
        return;
    }

    const quint8 shiftUpState = static_cast<quint8>(newValue.at(6));
    const quint8 shiftDownState = static_cast<quint8>(newValue.at(7));

    if (shiftUpState == 0x01 && lastShiftUpState != 0x01) {
        qDebug() << QStringLiteral("CYCPLUS BC2: Shift UP detected");
        emit plus();
    }

    if (shiftDownState == 0x01 && lastShiftDownState != 0x01) {
        qDebug() << QStringLiteral("CYCPLUS BC2: Shift DOWN detected");
        emit minus();
    }

    lastShiftUpState = shiftUpState;
    lastShiftDownState = shiftDownState;
}

void cycplusbc2controller::stateChanged(QLowEnergyService::ServiceState state) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceState>();
    emit debug(QStringLiteral("BTLE stateChanged ") + QString::fromLocal8Bit(metaEnum.valueToKey(state)));

    for (QLowEnergyService *s : qAsConst(gattCommunicationChannelService)) {
        qDebug() << QStringLiteral("stateChanged") << s->serviceUuid() << s->state();
        if (s->state() != QLowEnergyService::ServiceDiscovered && s->state() != QLowEnergyService::InvalidService) {
            qDebug() << QStringLiteral("not all services discovered");
            return;
        }
    }

    if (state != QLowEnergyService::ServiceState::ServiceDiscovered) {
        qDebug() << QStringLiteral("ignoring this state");
        return;
    }

    qDebug() << QStringLiteral("all services discovered!");

    for (QLowEnergyService *s : qAsConst(gattCommunicationChannelService)) {
        if (s->state() != QLowEnergyService::ServiceDiscovered || s->serviceUuid() != SERVICE_UUID) {
            continue;
        }

        connect(s, &QLowEnergyService::characteristicChanged, this, &cycplusbc2controller::characteristicChanged);
        connect(s, &QLowEnergyService::characteristicRead, this, &cycplusbc2controller::characteristicChanged);
        connect(s, static_cast<void (QLowEnergyService::*)(QLowEnergyService::ServiceError)>(&QLowEnergyService::error),
                this, &cycplusbc2controller::errorService);
        connect(s, &QLowEnergyService::descriptorWritten, this, &cycplusbc2controller::descriptorWritten);

        qDebug() << s->serviceUuid() << QStringLiteral("connected!");

        const auto characteristics_list = s->characteristics();
        for (const QLowEnergyCharacteristic &c : qAsConst(characteristics_list)) {
            qDebug() << QStringLiteral("char uuid") << c.uuid() << QStringLiteral("handle") << c.handle();

            if (c.uuid() != TX_CHARACTERISTIC_UUID) {
                continue;
            }

            gattNotifyCharacteristic = c;
            if ((c.properties() & QLowEnergyCharacteristic::Notify) == QLowEnergyCharacteristic::Notify) {
                const auto cccd = c.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration);
                if (cccd.isValid()) {
                    QByteArray descriptor;
                    descriptor.append((char)0x01);
                    descriptor.append((char)0x00);
                    s->writeDescriptor(cccd, descriptor);
                    qDebug() << s->serviceUuid() << c.uuid() << QStringLiteral("notification subscribed!");
                } else {
                    qDebug() << QStringLiteral("ClientCharacteristicConfiguration") << c.uuid()
                             << QStringLiteral(" is not valid");
                }
            }
        }
    }

    initDone = true;
}

void cycplusbc2controller::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    emit debug(QStringLiteral("descriptorWritten ") + descriptor.name() + " " + newValue.toHex(' '));
}

void cycplusbc2controller::serviceScanDone(void) {
    emit debug(QStringLiteral("serviceScanDone"));

    const auto services_list = m_control->services();
    for (const QBluetoothUuid &s : qAsConst(services_list)) {
        if (s != SERVICE_UUID) {
            continue;
        }

        gattCommunicationChannelService.append(m_control->createServiceObject(s));
        if (gattCommunicationChannelService.constLast()) {
            connect(gattCommunicationChannelService.constLast(), &QLowEnergyService::stateChanged, this,
                    &cycplusbc2controller::stateChanged);
            gattCommunicationChannelService.constLast()->discoverDetails();
        } else {
            m_control->disconnectFromDevice();
        }
    }
}

void cycplusbc2controller::errorService(QLowEnergyService::ServiceError err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    emit debug(QStringLiteral("cycplusbc2controller::errorService") +
               QString::fromLocal8Bit(metaEnum.valueToKey(err)) + m_control->errorString());
}

void cycplusbc2controller::error(QLowEnergyController::Error err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    emit debug(QStringLiteral("cycplusbc2controller::error") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void cycplusbc2controller::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    emit debug(QStringLiteral("Found new device: ") + device.name() + QStringLiteral(" (") +
               device.address().toString() + ')');

    bluetoothDevice = device;
    m_control = QLowEnergyController::createCentral(bluetoothDevice, this);
    connect(m_control, &QLowEnergyController::serviceDiscovered, this, &cycplusbc2controller::serviceDiscovered);
    connect(m_control, &QLowEnergyController::discoveryFinished, this, &cycplusbc2controller::serviceScanDone);
    connect(m_control,
            static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error),
            this, &cycplusbc2controller::error);
    connect(m_control, &QLowEnergyController::stateChanged, this, &cycplusbc2controller::controllerStateChanged);

    connect(m_control,
            static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error),
            this, [this](QLowEnergyController::Error error) {
                Q_UNUSED(error);
                emit debug(QStringLiteral("Cannot connect to remote device."));
                emit disconnected();
            });
    connect(m_control, &QLowEnergyController::connected, this, [this]() {
        emit debug(QStringLiteral("Controller connected. Search services..."));
        m_control->discoverServices();
    });
    connect(m_control, &QLowEnergyController::disconnected, this, [this]() {
        emit debug(QStringLiteral("LowEnergy controller disconnected"));
        emit disconnected();
    });

    m_control->connectToDevice();
}

bool cycplusbc2controller::connected() {
    if (!m_control) {
        return false;
    }
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

void cycplusbc2controller::controllerStateChanged(QLowEnergyController::ControllerState state) {
    qDebug() << QStringLiteral("controllerStateChanged") << state;
    if (state == QLowEnergyController::UnconnectedState) {
        qDebug() << QStringLiteral("trying to connect back again...");
        initDone = false;
        lastShiftUpState = 0x00;
        lastShiftDownState = 0x00;

        if (m_control) {
            m_control->connectToDevice();
        }
    }
}
