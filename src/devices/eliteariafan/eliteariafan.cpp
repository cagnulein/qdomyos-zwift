#include "eliteariafan.h"
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

// this module on iOS is completely handled from the ObjectiveC module in order to test if it's more stable than the Qt Bluetooth Implementation (crash midride)

eliteariafan::eliteariafan(bluetoothdevice *parentDevice) {
#ifdef Q_OS_IOS
    QZ_EnableDiscoveryCharsAndDescripttors = true;
#endif
    this->parentDevice = parentDevice;

#ifndef Q_OS_IOS
    refresh = new QTimer(this);
    connect(refresh, &QTimer::timeout, this, &eliteariafan::update);
    refresh->start(1000ms);
#endif
}

void eliteariafan::update() {
    if (initRequest) {
        initRequest = false;

        uint8_t init1[] = {0x02, 0x00, 0x00, 0x3d, 0x00};
        writeCharacteristic(gattWrite1Service, &gattWrite1Characteristic, init1, sizeof(init1), "init", false, true);

        uint8_t init2[] = {0x05, 0x00};
        writeCharacteristic(gattWrite1Service, &gattWrite2Characteristic, init2, sizeof(init2), "init", false, true);

        initDone = true;
    }
}

void eliteariafan::serviceDiscovered(const QBluetoothUuid &gatt) {
    emit debug(QStringLiteral("serviceDiscovered ") + gatt.toString());
}

void eliteariafan::disconnectBluetooth() {
    qDebug() << QStringLiteral("eliteariafan::disconnect") << m_control;

    if (m_control) {
        m_control->disconnectFromDevice();
    }
}

void eliteariafan::characteristicChanged(const QLowEnergyCharacteristic &characteristic,
                                               const QByteArray &newValue) {
    Q_UNUSED(characteristic);
    emit packetReceived();

    qDebug() << QStringLiteral(" << ") << newValue.toHex(' ');
}

void eliteariafan::fanSpeedRequest(uint8_t speed) {
    QSettings settings;
    if (speed > 100)
        speed = 100;
    double max = settings.value(QZSettings::fitmetria_fanfit_max, QZSettings::default_fitmetria_fanfit_max).toDouble();
    double min = settings.value(QZSettings::fitmetria_fanfit_min, QZSettings::default_fitmetria_fanfit_min).toDouble();

    uint16_t speed8 = (uint8_t)((double)speed * (max - min) / 100.0 + min);

#ifdef Q_OS_IOS
#ifndef IO_UNDER_QT
    iOS_EliteAriaFan->eliteAriaFan_fanSpeedRequest(speed8);
#endif
#else
    uint8_t init10[] = {0x03, 0x01, 0x0e};
    init10[2] = speed8;
    writeCharacteristic(gattWrite1Service, &gattWrite2Characteristic, init10, sizeof(init10),
                        "forcing fan" + QString::number(speed));
#endif                        
}

void eliteariafan::writeCharacteristic(QLowEnergyService *service, QLowEnergyCharacteristic *writeChar,
                                             uint8_t *data, uint8_t data_len, const QString &info, bool disable_log,
                                             bool wait_for_response) {
    QEventLoop loop;
    QTimer timeout;

    if (service == nullptr || writeChar->isValid() == false) {
        qDebug() << QStringLiteral(
            "eliteariafan trying to change the fan speed before the connection is established");
        return;
    }

           // if there are some crash here, maybe it's better to use 2 separate event for the characteristicChanged.
           // one for the resistance changed event (spontaneous), and one for the other ones.
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

    loop.exec();
}

void eliteariafan::stateChanged(QLowEnergyService::ServiceState state) {
    QBluetoothUuid _gattWriteCharacteristicId1(QStringLiteral("347b0012-7635-408b-8918-8ff3949ce592")); // handle 0x1d
    QBluetoothUuid _gattWriteCharacteristicId2(QStringLiteral("347b0040-7635-408b-8918-8ff3949ce592")); // handle 0x27

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
        if (s->state() == QLowEnergyService::ServiceDiscovered) {
            // establish hook into notifications
            connect(s, &QLowEnergyService::characteristicChanged, this, &eliteariafan::characteristicChanged);
            connect(s, &QLowEnergyService::characteristicWritten, this, &eliteariafan::characteristicWritten);
            connect(
                s, static_cast<void (QLowEnergyService::*)(QLowEnergyService::ServiceError)>(&QLowEnergyService::error),
                this, &eliteariafan::errorService);
            connect(s, &QLowEnergyService::descriptorWritten, this, &eliteariafan::descriptorWritten);

            qDebug() << s->serviceUuid() << QStringLiteral("connected!");

            auto characteristics_list = s->characteristics();
            for (const QLowEnergyCharacteristic &c : qAsConst(characteristics_list)) {
                qDebug() << QStringLiteral("char uuid") << c.uuid() << QStringLiteral("handle") << c.handle();
                auto descriptors_list = c.descriptors();
                for (const QLowEnergyDescriptor &d : qAsConst(descriptors_list)) {
                    qDebug() << QStringLiteral("descriptor uuid") << d.uuid() << QStringLiteral("handle") << d.handle();
                }

                if ((c.properties() & QLowEnergyCharacteristic::Notify) == QLowEnergyCharacteristic::Notify) {
                    QByteArray descriptor;
                    descriptor.append((char)0x01);
                    descriptor.append((char)0x00);
                    if (c.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration).isValid()) {
                        s->writeDescriptor(c.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration), descriptor);
                    } else {
                        qDebug() << QStringLiteral("ClientCharacteristicConfiguration") << c.uuid()
                                 << c.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration).uuid()
                                 << c.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration).handle()
                                 << QStringLiteral(" is not valid");
                    }

                    qDebug() << s->serviceUuid() << c.uuid() << QStringLiteral("notification subscribed!");
                } else if ((c.properties() & QLowEnergyCharacteristic::Indicate) ==
                           QLowEnergyCharacteristic::Indicate) {
                    QByteArray descriptor;
                    descriptor.append((char)0x02);
                    descriptor.append((char)0x00);
                    if (c.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration).isValid()) {
                        s->writeDescriptor(c.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration), descriptor);
                    } else {
                        qDebug() << QStringLiteral("ClientCharacteristicConfiguration") << c.uuid()
                                 << c.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration).uuid()
                                 << c.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration).handle()
                                 << QStringLiteral(" is not valid");
                    }

                    qDebug() << s->serviceUuid() << c.uuid() << QStringLiteral("indication subscribed!");
                } else if ((c.properties() & QLowEnergyCharacteristic::Read) == QLowEnergyCharacteristic::Read) {
                    // s->readCharacteristic(c);
                    // qDebug() << s->serviceUuid() << c.uuid() << "reading!";
                }

                if (c.uuid() == _gattWriteCharacteristicId1) {
                    qDebug() << QStringLiteral("_gattWriteCharacteristicId1 found");
                    gattWrite1Characteristic = c;
                    gattWrite1Service = s;
                } else if (c.uuid() == _gattWriteCharacteristicId2) {
                    qDebug() << QStringLiteral("_gattWriteCharacteristicId2 found");
                    gattWrite2Characteristic = c;
                }
            }
        }
    }
}

void eliteariafan::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    emit debug(QStringLiteral("descriptorWritten ") + descriptor.name() + " " + newValue.toHex(' '));
    initRequest = true;
}

void eliteariafan::characteristicWritten(const QLowEnergyCharacteristic &characteristic,
                                               const QByteArray &newValue) {
    Q_UNUSED(characteristic);
    emit debug(QStringLiteral("characteristicWritten ") + newValue.toHex(' '));
}

void eliteariafan::serviceScanDone(void) {
    emit debug(QStringLiteral("serviceScanDone"));

    initRequest = false;

    auto services_list = m_control->services();
    for (const QBluetoothUuid &s : qAsConst(services_list)) {
        gattCommunicationChannelService.append(m_control->createServiceObject(s));
        if (gattCommunicationChannelService.constLast()) {
            connect(gattCommunicationChannelService.constLast(), &QLowEnergyService::stateChanged, this,
                    &eliteariafan::stateChanged);
            gattCommunicationChannelService.constLast()->discoverDetails();
        } else {
            m_control->disconnectFromDevice();
        }
    }
}

void eliteariafan::errorService(QLowEnergyService::ServiceError err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    emit debug(QStringLiteral("eliteariafan::errorService") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void eliteariafan::error(QLowEnergyController::Error err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    emit debug(QStringLiteral("eliteariafan::error") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void eliteariafan::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    QSettings settings;
    emit debug(QStringLiteral("Found new device: ") + device.name() + QStringLiteral(" (") +
               device.address().toString() + ')');

#ifdef Q_OS_IOS
#ifndef IO_UNDER_QT
    iOS_EliteAriaFan = new lockscreen();
    iOS_EliteAriaFan->eliteAriaFan();
    return;
#endif
#endif
    {
        bluetoothDevice = device;
        m_control = QLowEnergyController::createCentral(bluetoothDevice, this);
        connect(m_control, &QLowEnergyController::serviceDiscovered, this, &eliteariafan::serviceDiscovered);
        connect(m_control, &QLowEnergyController::discoveryFinished, this, &eliteariafan::serviceScanDone);
        connect(m_control,
                static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error),
                this, &eliteariafan::error);
        connect(m_control, &QLowEnergyController::stateChanged, this, &eliteariafan::controllerStateChanged);

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

bool eliteariafan::connected() {
#ifdef Q_OS_IOS
    return true;
#endif               

    if (!m_control) {
        return false;
    }
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

void eliteariafan::controllerStateChanged(QLowEnergyController::ControllerState state) {
#ifdef Q_OS_IOS
    return;
#endif     
    
    qDebug() << QStringLiteral("controllerStateChanged") << state;
    if (state == QLowEnergyController::UnconnectedState && m_control) {
        qDebug() << QStringLiteral("trying to connect back again...");
        initRequest = false;
        initDone = false;

        m_control->connectToDevice();
    }
}
