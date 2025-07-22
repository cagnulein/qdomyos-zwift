#include "heartratebelt.h"
#include "homeform.h"
#include <QBluetoothLocalDevice>
#include <QDateTime>
#include <QEventLoop>
#include <QFile>
#include <QMetaEnum>
#include <QSettings>
#include <QThread>
#include <chrono>

using namespace std::chrono_literals;

heartratebelt::heartratebelt() {
    refresh = new QTimer(this);
    connect(refresh, &QTimer::timeout, this, &heartratebelt::update);
    refresh->start(500ms);
}

heartratebelt::~heartratebelt() {
    if (refresh) {
        refresh->stop();
        delete refresh;
    }
}

void heartratebelt::update() {
    if(m_control->state() == QLowEnergyController::DiscoveredState && gattCommunicationChannelService &&
            gattNotifyCharacteristic.isValid() && gattCommunicationChannelService->characteristic(QBluetoothUuid(QBluetoothUuid::HeartRateMeasurement)).properties() & QLowEnergyCharacteristic::Read) {
        gattCommunicationChannelService->readCharacteristic(gattCommunicationChannelService->characteristic(QBluetoothUuid(QBluetoothUuid::HeartRateMeasurement)));
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
    Q_UNUSED(characteristic);
    emit packetReceived();

    emit debug(QStringLiteral(" << ") + newValue.toHex(' '));

    if (newValue.length() > 1) {
        Heart = (uint8_t)newValue[1];
        emit heartRate((uint8_t)Heart.value());
    }

    emit debug(QStringLiteral("Current heart: ") + QString::number(Heart.value()));
}

void heartratebelt::stateChanged(QLowEnergyService::ServiceState state) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceState>();
    emit debug(QStringLiteral("BTLE stateChanged ") + QString::fromLocal8Bit(metaEnum.valueToKey(state)));

    if (state == QLowEnergyService::ServiceDiscovered) {
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
            return;
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
