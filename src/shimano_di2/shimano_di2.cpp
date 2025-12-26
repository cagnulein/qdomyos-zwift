#include "homeform.h"
#include "shimano_di2.h"
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

shimano_di2::shimano_di2(bluetoothdevice *parentDevice) {
#ifdef Q_OS_IOS
    QZ_EnableDiscoveryCharsAndDescripttors = true;
#endif
    this->parentDevice = parentDevice;

    refresh = new QTimer(this);
    connect(refresh, &QTimer::timeout, this, &shimano_di2::update);
    refresh->start(1000ms);
}

void shimano_di2::update() {
    // Periodic update function - currently not needed for Shimano Di2
    // The device works via characteristic notifications
}

void shimano_di2::serviceDiscovered(const QBluetoothUuid &gatt) {
    emit debug(QStringLiteral("serviceDiscovered ") + gatt.toString());
}

void shimano_di2::disconnectBluetooth() {
    qDebug() << QStringLiteral("shimano_di2::disconnect") << m_control;

    if (m_control) {
        m_control->disconnectFromDevice();
    }
}

void shimano_di2::characteristicChanged(const QLowEnergyCharacteristic &characteristic,
                                        const QByteArray &newValue) {
    emit packetReceived();

    QSettings settings;
    bool gears_volume_debouncing = settings.value(QZSettings::gears_volume_debouncing, QZSettings::default_gears_volume_debouncing).toBool();
    bool zwiftplay_swap = settings.value(QZSettings::zwiftplay_swap, QZSettings::default_zwiftplay_swap).toBool();

    qDebug() << QStringLiteral(" << ") << newValue.toHex(' ') << characteristic.uuid().toString();

    // D-Fly Channel UUID
    QBluetoothUuid dFlyChannelUuid(QStringLiteral("00002ac2-5348-494d-414e-4f5f424c4500"));

    if(characteristic.uuid() == dFlyChannelUuid && newValue.length() > 1) {
        // Skip first byte as per Dart implementation
        QByteArray channelData = newValue.mid(1);

        if(!initDone) {
            // First data reception - just initialize state
            for(int i = 0; i < channelData.length(); i++) {
                lastButtons[i] = (uint8_t)channelData.at(i);
            }
            initDone = true;
            qDebug() << "Shimano Di2 initialized with" << channelData.length() << "channels";
            return;
        }

        // Process channel changes
        for(int i = 0; i < channelData.length(); i++) {
            uint8_t currentValue = (uint8_t)channelData.at(i);

            // Check if this channel changed
            if(lastButtons.contains(i) && lastButtons[i] != currentValue) {
                int channelNumber = i + 1; // Channel number is index + 1
                qDebug() << "Shimano Di2 channel" << channelNumber << "changed from"
                         << lastButtons[i] << "to" << currentValue;

                // Determine if this is a gear up or down
                // Channel 1 is typically configured for gear up, Channel 2 for gear down
                // But users can configure any channel in E-TUBE app
                // For now, we'll use a simple convention: odd channels = up, even = down
                // Users can swap behavior using the zwiftplay_swap setting

                bool isUpChannel = (channelNumber % 2 == 1);

                if(!gears_volume_debouncing || currentValue > 0) {
                    if((isUpChannel && !zwiftplay_swap) || (!isUpChannel && zwiftplay_swap)) {
                        emit plus();
                        qDebug() << "Shimano Di2 gear UP";
                    } else {
                        emit minus();
                        qDebug() << "Shimano Di2 gear DOWN";
                    }
                }
            }

            // Update last state
            lastButtons[i] = currentValue;
        }
    }
}

void shimano_di2::stateChanged(QLowEnergyService::ServiceState state) {
    QBluetoothUuid dFlyChannelUuid(QStringLiteral("00002ac2-5348-494d-414e-4f5f424c4500"));

    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceState>();
    emit debug(QStringLiteral("BTLE stateChanged ") + QString::fromLocal8Bit(metaEnum.valueToKey(state)));

#ifndef Q_OS_IOS
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
            connect(s, &QLowEnergyService::characteristicChanged, this, &shimano_di2::characteristicChanged);
            connect(s, &QLowEnergyService::characteristicRead, this, &shimano_di2::characteristicChanged);
            connect(s, &QLowEnergyService::characteristicWritten, this, &shimano_di2::characteristicWritten);
            connect(
                s, static_cast<void (QLowEnergyService::*)(QLowEnergyService::ServiceError)>(&QLowEnergyService::error),
                this, &shimano_di2::errorService);
            connect(s, &QLowEnergyService::descriptorWritten, this, &shimano_di2::descriptorWritten);

            qDebug() << s->serviceUuid() << QStringLiteral("connected!");

            auto characteristics_list = s->characteristics();
            for (const QLowEnergyCharacteristic &c : qAsConst(characteristics_list)) {
                qDebug() << QStringLiteral("char uuid") << c.uuid() << QStringLiteral("handle") << c.handle();
                auto descriptors_list = c.descriptors();
                for (const QLowEnergyDescriptor &d : qAsConst(descriptors_list)) {
                    qDebug() << QStringLiteral("descriptor uuid") << d.uuid() << QStringLiteral("handle") << d.handle();
                }

                // Subscribe to D-Fly Channel notifications
                if (c.uuid() == dFlyChannelUuid) {
                    qDebug() << QStringLiteral("D-Fly Channel found, subscribing to indications");
                    gattNotifyCharacteristic = c;

                    if ((c.properties() & QLowEnergyCharacteristic::Indicate) == QLowEnergyCharacteristic::Indicate) {
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
                    } else if ((c.properties() & QLowEnergyCharacteristic::Notify) == QLowEnergyCharacteristic::Notify) {
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
                    }
                }
            }
        }
    }
#endif
}

void shimano_di2::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    emit debug(QStringLiteral("descriptorWritten ") + descriptor.name() + " " + newValue.toHex(' '));
    initRequest = true;
}

void shimano_di2::characteristicWritten(const QLowEnergyCharacteristic &characteristic,
                                        const QByteArray &newValue) {
    Q_UNUSED(characteristic);
    emit debug(QStringLiteral("characteristicWritten ") + newValue.toHex(' '));
}

void shimano_di2::serviceScanDone(void) {
    emit debug(QStringLiteral("serviceScanDone"));

    initRequest = false;

#ifndef Q_OS_IOS
    // Shimano Di2 service UUIDs
    QBluetoothUuid serviceUuid1(QStringLiteral("000018ef-5348-494d-414e-4f5f424c4500"));
    QBluetoothUuid serviceUuid2(QStringLiteral("000018ff-5348-494d-414e-4f5f424c4500"));

    auto services_list = m_control->services();
    for (const QBluetoothUuid &s : qAsConst(services_list)) {
        // Look for Shimano Di2 services
        if(s == serviceUuid1 || s == serviceUuid2) {
            qDebug() << "Found Shimano Di2 service:" << s.toString();
            gattCommunicationChannelService.append(m_control->createServiceObject(s));
            if (gattCommunicationChannelService.constLast()) {
                connect(gattCommunicationChannelService.constLast(), &QLowEnergyService::stateChanged, this,
                        &shimano_di2::stateChanged);
                gattCommunicationChannelService.constLast()->discoverDetails();
            } else {
                m_control->disconnectFromDevice();
            }
        }
    }
#endif
}

void shimano_di2::errorService(QLowEnergyService::ServiceError err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    emit debug(QStringLiteral("shimano_di2::errorService") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void shimano_di2::error(QLowEnergyController::Error err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    emit debug(QStringLiteral("shimano_di2::error") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void shimano_di2::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    QSettings settings;
    emit debug(QStringLiteral("Found new device: ") + device.name() + QStringLiteral(" (") +
               device.address().toString() + ')');

#ifdef Q_OS_IOS
#ifndef IO_UNDER_QT
    iOS_shimanoDi2 = new lockscreen();
    // iOS implementation would go here if needed
    // For now, we'll use the standard Qt implementation
#endif
#endif

    {
        bluetoothDevice = device;
        m_control = QLowEnergyController::createCentral(bluetoothDevice, this);
        connect(m_control, &QLowEnergyController::serviceDiscovered, this, &shimano_di2::serviceDiscovered);
        connect(m_control, &QLowEnergyController::discoveryFinished, this, &shimano_di2::serviceScanDone);
        connect(m_control,
                static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error),
                this, &shimano_di2::error);
        connect(m_control, &QLowEnergyController::stateChanged, this, &shimano_di2::controllerStateChanged);

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

bool shimano_di2::connected() {
#ifdef Q_OS_IOS
    return true;
#endif

    if (!m_control) {
        return false;
    }
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

void shimano_di2::controllerStateChanged(QLowEnergyController::ControllerState state) {
    qDebug() << QStringLiteral("controllerStateChanged") << state;
    if (state == QLowEnergyController::UnconnectedState) {
        qDebug() << QStringLiteral("trying to connect back again...");
        initRequest = false;
        initDone = false;
        lastButtons.clear();

        if(m_control)
            m_control->connectToDevice();
    }
}
