#include "heartratebelt.h"
#include "homeform.h"
#include <QBluetoothLocalDevice>
#include <QDateTime>
#include <QEventLoop>
#include <QFile>
#include <QMetaEnum>
#include <QSettings>
#include <QThread>

heartratebelt::heartratebelt() {
    // Initialize the update timer to run every second
    updateTimer = new QTimer(this);
    updateTimer->setInterval(1000);  // 1 second
    connect(updateTimer, &QTimer::timeout, this, &heartratebelt::update);
    updateTimer->start();
}

heartratebelt::~heartratebelt() {
    if (updateTimer) {
        updateTimer->stop();
        delete updateTimer;
    }
}

void heartratebelt::update() {
    QSettings settings;
    
    // Check if we are in connecting state and more than 10 seconds have passed
    if (m_control && 
        m_control->state() == QLowEnergyController::ConnectingState && 
        !connectingTime.isNull() &&
        connectingTime.msecsTo(QDateTime::currentDateTime()) > CONNECTION_TIMEOUT
#ifdef Q_OS_IOS
        && !settings.value(QZSettings::ios_cache_heart_device, QZSettings::default_ios_cache_heart_device).toBool()
#endif
        ) {
        
        emit debug(QStringLiteral("Connection timeout in ConnectingState - disconnecting and retrying..."));
        disconnectBluetooth();
        connectingTime = QDateTime();  // Reset the timestamp
        
        // Reconnect after 1 second
        QTimer::singleShot(1000, this, [this]() {
            if (m_control) {
                emit debug(QStringLiteral("Attempting to reconnect after timeout..."));
                m_control->connectToDevice();
            }
        });
    }
}

void heartratebelt::serviceDiscovered(const QBluetoothUuid &gatt) {
    emit debug(QStringLiteral("serviceDiscovered ") + gatt.toString());
}

void heartratebelt::disconnectBluetooth() {
    qDebug() << QStringLiteral("heartratebelt::disconnect") << m_control;

    if (m_control) {
        m_control->disconnectFromDevice();
    }
}

void heartratebelt::characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    // qDebug() << "characteristicChanged" << characteristic.uuid() << newValue << newValue.length();
    emit packetReceived();

    emit debug(QStringLiteral(" << ") + newValue.toHex(' '));

    // Handle Battery Service
    if (characteristic.uuid() == QBluetoothUuid((quint16)0x2A19)) {
        if(newValue.length() > 0) {
            uint8_t battery = (uint8_t)newValue.at(0);
            if(battery != battery_level) {
                if(homeform::singleton())
                    homeform::singleton()->setToastRequested(bluetoothDevice.name() + QStringLiteral(" Battery Level ") + QString::number(battery) + " %");
            }
            battery_level = battery;
            qDebug() << QStringLiteral("battery: ") << battery;
        }
        return;
    }

    // Handle Heart Rate Measurement according to Bluetooth Heart Rate Profile
    if (newValue.length() > 1) {
        uint8_t flags = (uint8_t)newValue[0];
        int index = 1;

        // Bit 0: Heart Rate Value Format
        // 0 = UINT8, 1 = UINT16
        bool hrFormat16bit = (flags & 0x01) != 0;

        if (hrFormat16bit && newValue.length() > 2) {
            // 16-bit heart rate value
            Heart = (uint16_t)(((uint8_t)newValue[2] << 8) | (uint8_t)newValue[1]);
            index = 3;
        } else {
            // 8-bit heart rate value
            Heart = (uint8_t)newValue[1];
            index = 2;
        }
        emit heartRate((uint8_t)Heart.value());

        // Bit 3: Energy Expended Status
        // If set, 2 bytes of Energy Expended follow the HR value
        bool energyExpendedPresent = (flags & 0x08) != 0;
        if (energyExpendedPresent) {
            index += 2; // Skip 2 bytes of energy expended
        }

        // Bit 4: RR-Interval
        // If set, one or more RR-Interval values are present (2 bytes each)
        // RR-Interval is in units of 1/1024 seconds
        bool rrIntervalPresent = (flags & 0x10) != 0;
        if (rrIntervalPresent) {
            while (index + 1 < newValue.length()) {
                uint16_t rrRaw = (uint16_t)(((uint8_t)newValue[index + 1] << 8) | (uint8_t)newValue[index]);
                // Convert from 1/1024 seconds to milliseconds
                double rrMs = (rrRaw / 1024.0) * 1000.0;
                emit debug(QStringLiteral("RR-Interval: ") + QString::number(rrMs, 'f', 1) + QStringLiteral(" ms"));
                emit rrIntervalReceived(rrMs);
                index += 2;
            }
        }
    }

    emit debug(QStringLiteral("Current heart: ") + QString::number(Heart.value()));
}

void heartratebelt::stateChanged(QLowEnergyService::ServiceState state) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceState>();
    emit debug(QStringLiteral("BTLE stateChanged ") + QString::fromLocal8Bit(metaEnum.valueToKey(state)));

    if (state == QLowEnergyService::ServiceDiscovered) {
        // Check if this is the Battery Service
        QLowEnergyService* service = qobject_cast<QLowEnergyService*>(sender());
        if (service && service == gattBatteryService) {
            // Handle Battery Service
            auto characteristics_list = gattBatteryService->characteristics();
            for (const QLowEnergyCharacteristic &c : qAsConst(characteristics_list)) {
                emit debug(QStringLiteral("battery characteristic ") + c.uuid().toString());
            }

            connect(gattBatteryService, &QLowEnergyService::characteristicChanged, this,
                    &heartratebelt::characteristicChanged);
            connect(gattBatteryService,
                    static_cast<void (QLowEnergyService::*)(QLowEnergyService::ServiceError)>(&QLowEnergyService::error),
                    this, &heartratebelt::errorService);

            // Enable notifications for battery level
            for (const QLowEnergyCharacteristic &c : qAsConst(characteristics_list)) {
                if ((c.properties() & QLowEnergyCharacteristic::Notify) == QLowEnergyCharacteristic::Notify) {
                    QByteArray descriptor;
                    descriptor.append((char)0x01);
                    descriptor.append((char)0x00);
                    gattBatteryService->writeDescriptor(
                        c.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration), descriptor);
                }
            }
            return;
        }

        // Original code for Heart Rate Service
        auto characteristics_list = gattCommunicationChannelService->characteristics();
        for (const QLowEnergyCharacteristic &c : qAsConst(characteristics_list)) {
            emit debug(QStringLiteral("characteristic ") + c.uuid().toString());
        }

        gattNotifyCharacteristic =
            gattCommunicationChannelService->characteristic(QBluetoothUuid(QBluetoothUuid::HeartRateMeasurement));
        if(!gattNotifyCharacteristic.isValid()) {
            qDebug() << "gattNotifyCharacteristic not valid for HR";
            return;
        }

        // establish hook into notifications
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicChanged, this,
                &heartratebelt::characteristicChanged);
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicWritten, this,
                &heartratebelt::characteristicWritten);
        connect(gattCommunicationChannelService,
                static_cast<void (QLowEnergyService::*)(QLowEnergyService::ServiceError)>(&QLowEnergyService::error),
                this, &heartratebelt::errorService);
        connect(gattCommunicationChannelService, &QLowEnergyService::descriptorWritten, this,
                &heartratebelt::descriptorWritten);

        QByteArray descriptor;
        descriptor.append((char)0x01);
        descriptor.append((char)0x00);
        gattCommunicationChannelService->writeDescriptor(
            gattNotifyCharacteristic.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration), descriptor);
    }
}

void heartratebelt::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    emit debug(QStringLiteral("descriptorWritten ") + descriptor.name() + " " + newValue.toHex(' '));
}

void heartratebelt::characteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    Q_UNUSED(characteristic);
    emit debug(QStringLiteral("characteristicWritten ") + newValue.toHex(' '));
}

void heartratebelt::serviceScanDone(void) {
    emit debug(QStringLiteral("serviceScanDone"));

    auto services_list = m_control->services();
    for (const QBluetoothUuid &s : qAsConst(services_list)) {
        qDebug() << QStringLiteral("heartRateBelt services ") << s.toString();
        if (s == QBluetoothUuid::HeartRate) {
            QBluetoothUuid _gattCommunicationChannelServiceId(QBluetoothUuid::HeartRate);
            gattCommunicationChannelService = m_control->createServiceObject(_gattCommunicationChannelServiceId);
            connect(gattCommunicationChannelService, &QLowEnergyService::stateChanged, this,
                    &heartratebelt::stateChanged);
            gattCommunicationChannelService->discoverDetails();
        }
        else if (s == QBluetoothUuid::BatteryService) {
            QBluetoothUuid _gattBatteryServiceId(QBluetoothUuid::BatteryService);
            gattBatteryService = m_control->createServiceObject(_gattBatteryServiceId);
            connect(gattBatteryService, &QLowEnergyService::stateChanged, this,
                    &heartratebelt::stateChanged);
            gattBatteryService->discoverDetails();
        }
    }
}

void heartratebelt::errorService(QLowEnergyService::ServiceError err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    emit debug(QStringLiteral("heartratebelt::errorService") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void heartratebelt::error(QLowEnergyController::Error err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    emit debug(QStringLiteral("heartratebelt::error") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void heartratebelt::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    QSettings settings;
    
    emit debug(QStringLiteral("Found new device: ") + device.name() + QStringLiteral(" (") +
               device.address().toString() + ')');

    if(homeform::singleton())
        homeform::singleton()->setToastRequested(device.name() + QStringLiteral(" connected!"));

    // if(device.name().startsWith(heartRateBeltName))
    {
        bluetoothDevice = device;
        m_control = QLowEnergyController::createCentral(bluetoothDevice, this);
        connect(m_control, &QLowEnergyController::serviceDiscovered, this, &heartratebelt::serviceDiscovered);
        connect(m_control, &QLowEnergyController::discoveryFinished, this, &heartratebelt::serviceScanDone);
        connect(m_control,
                static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error),
                this, &heartratebelt::error);
        connect(m_control, &QLowEnergyController::stateChanged, this, &heartratebelt::controllerStateChanged);

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

bool heartratebelt::connected() {
    if (!m_control) {
        return false;
    }
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

void heartratebelt::controllerStateChanged(QLowEnergyController::ControllerState state) {
    qDebug() << QStringLiteral("controllerStateChanged") << state;
    
    if (state == QLowEnergyController::ConnectingState) {
        connectingTime = QDateTime::currentDateTime();
    } else {
        connectingTime = QDateTime();  // Reset timestamp for other states
    }
    
    if (state == QLowEnergyController::UnconnectedState && m_control) {
        qDebug() << QStringLiteral("trying to connect back again...");
        m_control->connectToDevice();
    }
}
