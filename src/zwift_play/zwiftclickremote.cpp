#include "homeform.h"
#include "zwiftclickremote.h"
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

volatile int8_t AbstractZapDevice::risingEdge = 0;
QTimer* AbstractZapDevice::autoRepeatTimer = nullptr;
bool AbstractZapDevice::lastButtonPlus = false;
QDateTime AbstractZapDevice::lastFrame = QDateTime::currentDateTime();
bool AbstractZapDevice::lastRideGearPlus = false;
QDateTime AbstractZapDevice::lastRideGearFrame;

zwiftclickremote::zwiftclickremote(bluetoothdevice *parentDevice, AbstractZapDevice::ZWIFT_PLAY_TYPE typeZap) {
#ifdef Q_OS_IOS
    QZ_EnableDiscoveryCharsAndDescripttors = true;
#endif
    this->parentDevice = parentDevice;
    this->typeZap = typeZap;

    refresh = new QTimer(this);
    connect(refresh, &QTimer::timeout, this, &zwiftclickremote::update);
    refresh->start(1000ms);
}

void zwiftclickremote::update() {
    if (initRequest && !initDone) {
        initRequest = false;
        QByteArray s = playDevice->buildHandshakeStart();
        writeCharacteristic(gattWrite1Service, &gattWrite1Characteristic, (uint8_t *) s.data(), s.length(), "handshakeStart");
        // For left controller: send ff0400 unlock-ack so an already-unlocked device stays in unlocked mode.
        // OpenBikeControl sends this only when they know the device was unlocked within the last 24h;
        // we send it unconditionally — it's a no-op on a locked device and maintains unlock on an unlocked one.
        if(typeZap == AbstractZapDevice::LEFT) {
            QByteArray unlockAck = QByteArray::fromHex("ff0400");
            writeCharacteristic(gattWrite1Service, &gattWrite1Characteristic,
                               (uint8_t*)unlockAck.data(), unlockAck.length(), "unlockAck");
        }
        initDone = true;
        keepAliveCounter = 0;
    } else if(initDone) {
        countRxTimeout++;
        if(countRxTimeout == 10) {
            if(homeform::singleton())
                homeform::singleton()->setToastRequested("Zwift device: UPGRADE THE FIRMWARE!");
        }
        // Keepalive: re-send RideOn every 3 seconds to prevent Click V2 deep-sleep (causes HCI 0x08 drop).
        // Applies to LEFT and RIGHT types only; NONE is the old Zwift Click which uses a different protocol.
        if(typeZap != AbstractZapDevice::NONE && ++keepAliveCounter >= 3) {
            keepAliveCounter = 0;
            QByteArray s = playDevice->buildHandshakeStart();
            writeCharacteristic(gattWrite1Service, &gattWrite1Characteristic,
                               (uint8_t*)s.data(), s.length(), "keepAlive", true);
        }
    }
}

void zwiftclickremote::serviceDiscovered(const QBluetoothUuid &gatt) {
    emit debug(QStringLiteral("serviceDiscovered ") + gatt.toString());
}

void zwiftclickremote::disconnectBluetooth() {
    qDebug() << QStringLiteral("zwiftclickremote::disconnect") << m_control;

    if (m_control) {
        m_control->disconnectFromDevice();
    }
}

void zwiftclickremote::characteristicChanged(const QLowEnergyCharacteristic &characteristic,
                                             const QByteArray &newValue) {
    handleCharacteristicValueChanged(characteristic.uuid(), newValue);
}

void zwiftclickremote::handleCharacteristicValueChanged(const QBluetoothUuid &uuid, const QByteArray &newValue) {
    emit packetReceived();
    countRxTimeout = 0;

    qDebug() << QStringLiteral(" << ") << newValue.toHex(' ') << QString(newValue) << uuid.toString() << typeZap;

    if(uuid == QBluetoothUuid(QStringLiteral("00000002-19CA-4651-86E5-FA29DCDD09D1"))) {
        playDevice->processCharacteristic("Async", newValue, typeZap);
    } else if(uuid == QBluetoothUuid(QStringLiteral("00000004-19CA-4651-86E5-FA29DCDD09D1"))) {
        // Unlock detection: when the device is in unencrypted (unlocked) mode it echoes back
        // the RideOn handshake on SYNC_TX, starting with 0x52 ('R'). A locked device instead
        // sends a crypto challenge starting with 0xFF. See OpenBikeControl zwift_ride.dart,
        // commit 2cb079fc ("attempt to improve Zwift Click V2 unlock").
        if(!deviceUnlocked && !newValue.isEmpty() && (uint8_t)newValue[0] == 0x52) {
            deviceUnlocked = true;
            qDebug() << QStringLiteral("zwiftclickremote: device UNLOCKED (RideOn echo) typeZap=") << typeZap;
            if(typeZap == AbstractZapDevice::LEFT && homeform::singleton())
                homeform::singleton()->setToastRequested("Zwift Click V2: Left controller unlocked!");
        }
        playDevice->processCharacteristic("SyncTx", newValue, typeZap);
    } else if(uuid == QBluetoothUuid::BatteryLevel) {

    }
}

void zwiftclickremote::writeCharacteristic(QLowEnergyService *service, QLowEnergyCharacteristic *writeChar,
                                           uint8_t *data, uint8_t data_len, const QString &info, bool disable_log,
                                           bool wait_for_response) {
#ifdef Q_OS_IOS
#ifndef IO_UNDER_QT
    iOS_zwiftClickRemote->zwiftClickRemote_WriteCharacteristic(data, data_len, this); // Pass 'this' pointer
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
            "zwiftclickremote trying to change the fan speed before the connection is estabilished");
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

    if(wait_for_response) // without this, it crashes on ios after sometimes
        loop.exec();
}

void zwiftclickremote::stateChanged(QLowEnergyService::ServiceState state) {
    QBluetoothUuid _syncRxChar(QStringLiteral("00000003-19CA-4651-86E5-FA29DCDD09D1"));
    QBluetoothUuid _syncTxChar(QStringLiteral("00000004-19CA-4651-86E5-FA29DCDD09D1"));
    QBluetoothUuid _asyncChar(QStringLiteral("00000002-19CA-4651-86E5-FA29DCDD09D1"));

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
            connect(s, &QLowEnergyService::characteristicChanged, this, &zwiftclickremote::characteristicChanged);
            connect(s, &QLowEnergyService::characteristicRead, this, &zwiftclickremote::characteristicChanged);
            connect(s, &QLowEnergyService::characteristicWritten, this, &zwiftclickremote::characteristicWritten);
            connect(
                s, static_cast<void (QLowEnergyService::*)(QLowEnergyService::ServiceError)>(&QLowEnergyService::error),
                this, &zwiftclickremote::errorService);
            connect(s, &QLowEnergyService::descriptorWritten, this, &zwiftclickremote::descriptorWritten);

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
                }
                
                /*if ((c.properties() & QLowEnergyCharacteristic::Read) == QLowEnergyCharacteristic::Read &&
                    c.uuid() != _syncTxChar && c.uuid() != _asyncChar) {
                    s->readCharacteristic(c);
                    qDebug() << s->serviceUuid() << c.uuid() << "reading!";
                }*/

                if (c.uuid() == _syncRxChar) {
                    qDebug() << QStringLiteral("_syncRxChar found");
                    gattWrite1Characteristic = c;
                    gattWrite1Service = s;
                }
            }
        }
    }
#endif
}

void zwiftclickremote::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    emit debug(QStringLiteral("descriptorWritten ") + descriptor.name() + " " + newValue.toHex(' '));
    if(!initDone)
        initRequest = true;
}

void zwiftclickremote::characteristicWritten(const QLowEnergyCharacteristic &characteristic,
                                               const QByteArray &newValue) {
    Q_UNUSED(characteristic);
    emit debug(QStringLiteral("characteristicWritten ") + newValue.toHex(' '));
}

void zwiftclickremote::serviceScanDone(void) {
    emit debug(QStringLiteral("serviceScanDone"));

    initRequest = false;

#ifndef Q_OS_IOS
    auto services_list = m_control->services();
    for (const QBluetoothUuid &s : qAsConst(services_list)) {
        gattCommunicationChannelService.append(m_control->createServiceObject(s));
        if (gattCommunicationChannelService.constLast()) {
            connect(gattCommunicationChannelService.constLast(), &QLowEnergyService::stateChanged, this,
                    &zwiftclickremote::stateChanged);
            gattCommunicationChannelService.constLast()->discoverDetails();
        } else {
            m_control->disconnectFromDevice();
        }
    }
#endif
}

void zwiftclickremote::errorService(QLowEnergyService::ServiceError err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    emit debug(QStringLiteral("zwiftclickremote::errorService") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void zwiftclickremote::error(QLowEnergyController::Error err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    emit debug(QStringLiteral("zwiftclickremote::error") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void zwiftclickremote::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    QSettings settings;
    emit debug(QStringLiteral("Found new device: ") + device.name() + QStringLiteral(" (") +
               device.address().toString() + ')');
    
#ifdef Q_OS_IOS
#ifndef IO_UNDER_QT
    iOS_zwiftClickRemote = new lockscreen();
      iOS_zwiftClickRemote->zwiftClickRemote(
          device.name().toStdString().c_str(),
          device.deviceUuid().toString().toUpper().replace('{',"").replace('}',"").toStdString().c_str(),
          this
      );
    return;
#endif
#endif

    {
        bluetoothDevice = device;
        m_control = QLowEnergyController::createCentral(bluetoothDevice, this);
        connect(m_control, &QLowEnergyController::serviceDiscovered, this, &zwiftclickremote::serviceDiscovered);
        connect(m_control, &QLowEnergyController::discoveryFinished, this, &zwiftclickremote::serviceScanDone);
        connect(m_control,
                static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error),
                this, &zwiftclickremote::error);
        connect(m_control, &QLowEnergyController::stateChanged, this, &zwiftclickremote::controllerStateChanged);

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

bool zwiftclickremote::connected() {
#ifdef Q_OS_IOS
    return true;
#endif

    if (!m_control) {
        return false;
    }
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

void zwiftclickremote::controllerStateChanged(QLowEnergyController::ControllerState state) {
    qDebug() << QStringLiteral("controllerStateChanged") << state;
    if (state == QLowEnergyController::UnconnectedState) {
        qDebug() << QStringLiteral("trying to connect back again...");
        initRequest = false;
        initDone = false;
        keepAliveCounter = 0;
        deviceUnlocked = false;

        if(m_control)
            m_control->connectToDevice();
    }
}

void zwiftclickremote::vibrate(uint8_t pattern) {
    if(!initDone) return;
    
    QSettings settings;
    bool zwift_play_vibration = settings.value(QZSettings::zwift_play_vibration, QZSettings::default_zwift_play_vibration).toBool();
    if(!zwift_play_vibration) return;
    
    QByteArray s = QByteArray::fromHex("1212080A060802100018");
    s.append(pattern);
    writeCharacteristic(gattWrite1Service, &gattWrite1Characteristic, (uint8_t *) s.data(), s.length(), "vibrate", false, false);
}
