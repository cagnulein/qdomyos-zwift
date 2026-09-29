#include "sramAXSController.h"
#include <QBluetoothLocalDevice>
#include <QDateTime>
#include <QEventLoop>
#include <QFile>
#include <QMetaEnum>
#include <QSettings>
#include <QThread>
#include <QTimer>
#include <QEventLoop>
#include <chrono>

using namespace std::chrono_literals;

sramaxscontroller::sramaxscontroller() {}

void sramaxscontroller::update() {}

void sramaxscontroller::serviceDiscovered(const QBluetoothUuid &gatt) {
    emit debug(QStringLiteral("serviceDiscovered ") + gatt.toString());
}

void sramaxscontroller::writeCharacteristic(QLowEnergyService *service, QLowEnergyCharacteristic characteristic,
                                           uint8_t *data, uint8_t data_len, QString info, bool disable_log,
                                           bool wait_for_response) {
    QEventLoop loop;
    QTimer timeout;

    if (!service) {
        qDebug() << "no gattCustomService available";
        return;
    }

    if (wait_for_response) {
        connect(service, &QLowEnergyService::characteristicChanged, &loop, &QEventLoop::quit);
        timeout.singleShot(300ms, &loop, &QEventLoop::quit);
    } else {
        connect(service, &QLowEnergyService::characteristicWritten, &loop, &QEventLoop::quit);
        timeout.singleShot(300ms, &loop, &QEventLoop::quit);
    }

    if (writeBuffer) {
        delete writeBuffer;
    }
    writeBuffer = new QByteArray((const char *)data, data_len);

    if (characteristic.properties() & QLowEnergyCharacteristic::WriteNoResponse) {
        service->writeCharacteristic(characteristic, *writeBuffer, QLowEnergyService::WriteWithoutResponse);
    } else {
        service->writeCharacteristic(characteristic, *writeBuffer);
    }

    if (!disable_log)
        qDebug() << " >> " << writeBuffer->toHex(' ') << " // " << info;

    loop.exec();
}


void sramaxscontroller::disconnectBluetooth() {
    qDebug() << QStringLiteral("sramaxscontroller::disconnect") << m_control;

    if (m_control) {
        m_control->disconnectFromDevice();
    }
}

void sramaxscontroller::characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    // qDebug() << "characteristicChanged" << characteristic.uuid() << newValue << newValue.length();
    Q_UNUSED(characteristic);
    emit packetReceived();

    qDebug() << QStringLiteral(" << ") << characteristic.uuid() << newValue.toHex(' ');

    QBluetoothUuid sx(QStringLiteral("d905000b-90aa-4c7c-b036-1e01fb8eb7ee"));
    QBluetoothUuid dx(QStringLiteral("d9050054-90aa-4c7c-b036-1e01fb8eb7ee"));
    if(characteristic.uuid() == sx) {
        emit plus();
    } else if(characteristic.uuid() == dx) {
        emit minus();
    }
}

void sramaxscontroller::stateChanged(QLowEnergyService::ServiceState state) {
    QBluetoothUuid _gattWriteCharCustomService (QStringLiteral("d905ee51-90aa-4c7c-b036-1e01fb8eb7ee"));
    QBluetoothUuid _gattWriteCharControlPointId(QStringLiteral("d905ee52-90aa-4c7c-b036-1e01fb8eb7ee"));
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceState>();
    emit debug(QStringLiteral("BTLE stateChanged ") + QString::fromLocal8Bit(metaEnum.valueToKey(state)));

    for (QLowEnergyService *s : qAsConst(gattCommunicationChannelService)) {
        qDebug() << QStringLiteral("stateChanged") << s->serviceUuid() << s->state();

        if (s->state() != QLowEnergyService::RemoteServiceDiscovered && s->state() != QLowEnergyService::InvalidService) {
            qDebug() << QStringLiteral("not all services discovered");
            return;
        }
    }

    qDebug() << QStringLiteral("all services discovered!");

    notificationSubscribed = 0;

    for (QLowEnergyService *s : qAsConst(gattCommunicationChannelService)) {
        if (s->state() == QLowEnergyService::RemoteServiceDiscovered) {
            // establish hook into notifications
            connect(s, &QLowEnergyService::characteristicChanged, this, &sramaxscontroller::characteristicChanged);
            connect(s, &QLowEnergyService::characteristicWritten, this, &sramaxscontroller::characteristicWritten);
            connect(
                s, &QLowEnergyService::errorOccurred,
                this, &sramaxscontroller::errorService);
            connect(s, &QLowEnergyService::descriptorWritten, this, &sramaxscontroller::descriptorWritten);

            qDebug() << s->serviceUuid() << QStringLiteral("connected!");

            auto characteristics_list = s->characteristics();
            for (const QLowEnergyCharacteristic &c : qAsConst(characteristics_list)) {
                qDebug() << QStringLiteral("char uuid") << c.uuid() << c.properties();

                if (c.uuid() == _gattWriteCharControlPointId) {
                    qDebug() << QStringLiteral("Custom service found");
                    gattWriteCharControlPointId = c;
                    gattWriteCharCustomService = s;
                }
            }
        }
    }

    for (QLowEnergyService *s : qAsConst(gattCommunicationChannelService)) {
        if (s->state() == QLowEnergyService::RemoteServiceDiscovered) {
            auto characteristics_list = s->characteristics();
            for (const QLowEnergyCharacteristic &c : qAsConst(characteristics_list)) {
                auto descriptors_list = c.descriptors();
                for (const QLowEnergyDescriptor &d : qAsConst(descriptors_list)) {
                    qDebug() << QStringLiteral("descriptor uuid") << d.uuid();
                }

                if ((c.properties() & QLowEnergyCharacteristic::Notify) == QLowEnergyCharacteristic::Notify) {
                    QByteArray descriptor;
                    descriptor.append((char)0x01);
                    descriptor.append((char)0x00);
                    if (c.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration).isValid()) {
                        s->writeDescriptor(c.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration), descriptor);
                        notificationSubscribed++;
                    } else {
                        qDebug() << QStringLiteral("ClientCharacteristicConfiguration") << c.uuid()
                                 << c.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration).uuid()
                                 
                                 << QStringLiteral(" is not valid");
                    }

                    qDebug() << s->serviceUuid() << c.uuid() << QStringLiteral("notification subscribed!");
                } else if ((c.properties() & QLowEnergyCharacteristic::Indicate) == QLowEnergyCharacteristic::Indicate) {
                    QByteArray descriptor;
                    descriptor.append((char)0x02);
                    descriptor.append((char)0x00);
                    if (c.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration).isValid()) {
                        s->writeDescriptor(c.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration), descriptor);
                        notificationSubscribed++;
                    } else {
                        qDebug() << QStringLiteral("ClientCharacteristicConfiguration") << c.uuid()
                                 << c.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration).uuid()
                                 
                                 << QStringLiteral(" is not valid");
                    }

                    qDebug() << s->serviceUuid() << c.uuid() << QStringLiteral("indication subscribed!");
                }
            }
        }
    }
}

void sramaxscontroller::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    emit debug(QStringLiteral("descriptorWritten ") + descriptor.name() + " " + newValue.toHex(' '));

    if (notificationSubscribed)
        notificationSubscribed--;

    if (!notificationSubscribed) {
        uint8_t d = 0x73;
        if(gattWriteCharCustomService) {
            writeCharacteristic(gattWriteCharCustomService, gattWriteCharControlPointId, &d, 1, "init", false, true);
        } else {
            qDebug() << "ERROR! no custom services found!";
        }
    }
}

void sramaxscontroller::characteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    Q_UNUSED(characteristic);
    emit debug(QStringLiteral("characteristicWritten ") + newValue.toHex(' '));
}

void sramaxscontroller::serviceScanDone(void) {
    emit debug(QStringLiteral("serviceScanDone"));

    auto services_list = m_control->services();
    for (const QBluetoothUuid &s : qAsConst(services_list)) {
        qDebug() << s << "discovering...";
        gattCommunicationChannelService.append(m_control->createServiceObject(s));
        connect(gattCommunicationChannelService.constLast(), &QLowEnergyService::stateChanged, this,
                &sramaxscontroller::stateChanged);
        gattCommunicationChannelService.constLast()->discoverDetails();
    }
}

void sramaxscontroller::errorService(QLowEnergyService::ServiceError err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    emit debug(QStringLiteral("sramaxscontroller::errorService") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void sramaxscontroller::error(QLowEnergyController::Error err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    emit debug(QStringLiteral("sramaxscontroller::error") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void sramaxscontroller::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    QSettings settings;
    // QString sramaxscontrollerName = settings.value(QZSettings::heart_rate_belt_name),
    // QStringLiteral("Disabled")).toString();//NOTE: clazy-unused-non-trivial-variable
    emit debug(QStringLiteral("Found new device: ") + device.name() + QStringLiteral(" (") +
               device.address().toString() + ')');
    // if(device.name().startsWith(sramaxscontrollerName))
    {
        bluetoothDevice = device;
        m_control = QLowEnergyController::createCentral(bluetoothDevice, this);
        connect(m_control, &QLowEnergyController::serviceDiscovered, this, &sramaxscontroller::serviceDiscovered);
        connect(m_control, &QLowEnergyController::discoveryFinished, this, &sramaxscontroller::serviceScanDone);
        connect(m_control,
                &QLowEnergyController::errorOccurred,
                this, &sramaxscontroller::error);
        connect(m_control, &QLowEnergyController::stateChanged, this, &sramaxscontroller::controllerStateChanged);

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

bool sramaxscontroller::connected() {
    if (!m_control) {
        return false;
    }
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

void sramaxscontroller::controllerStateChanged(QLowEnergyController::ControllerState state) {
    qDebug() << QStringLiteral("controllerStateChanged") << state;
    if (state == QLowEnergyController::UnconnectedState && m_control) {
        qDebug() << QStringLiteral("trying to connect back again...");
        m_control->connectToDevice();
    }
}
