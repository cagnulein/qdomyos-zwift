#include "heartratebelt.h"
#include <QBluetoothLocalDevice>
#include <QDateTime>
#include <QEventLoop>
#include <QFile>
#include <QMetaEnum>
#include <QSettings>
#include <QThread>

heartratebelt::heartratebelt() {}

void heartratebelt::update() {}

void heartratebelt::serviceDiscovered(const QBluetoothUuid &gatt) { debug("serviceDiscovered " + gatt.toString()); }

void heartratebelt::disconnectBluetooth() {
    qDebug() << "heartratebelt::disconnect" << m_control;

    if (m_control) {
        m_control->disconnectFromDevice();
    }
}

void heartratebelt::characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    // qDebug() << "characteristicChanged" << characteristic.uuid() << newValue << newValue.length();
    Q_UNUSED(characteristic);
    emit packetReceived();

    debug(" << " + newValue.toHex(' '));

    if (newValue.length() > 1) {
        Heart = newValue[1];
        emit heartRate((uint8_t)Heart.value());
    }

    debug("Current heart: " + QString::number(Heart.value()));
}

void heartratebelt::stateChanged(QLowEnergyService::ServiceState state) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceState>();
    debug("BTLE stateChanged " + QString::fromLocal8Bit(metaEnum.valueToKey(state)));

    if (state == QLowEnergyService::ServiceDiscovered) {
        foreach (QLowEnergyCharacteristic c, gattCommunicationChannelService->characteristics()) {
            debug("characteristic " + c.uuid().toString());
        }

        gattNotifyCharacteristic =
            gattCommunicationChannelService->characteristic(QBluetoothUuid(QBluetoothUuid::HeartRateMeasurement));
        Q_ASSERT(gattNotifyCharacteristic.isValid());

        // establish hook into notifications
        connect(gattCommunicationChannelService, SIGNAL(characteristicChanged(QLowEnergyCharacteristic, QByteArray)),
                this, SLOT(characteristicChanged(QLowEnergyCharacteristic, QByteArray)));
        connect(gattCommunicationChannelService,
                SIGNAL(characteristicWritten(const QLowEnergyCharacteristic, const QByteArray)), this,
                SLOT(characteristicWritten(const QLowEnergyCharacteristic, const QByteArray)));
        connect(gattCommunicationChannelService, SIGNAL(error(QLowEnergyService::ServiceError)), this,
                SLOT(errorService(QLowEnergyService::ServiceError)));
        connect(gattCommunicationChannelService,
                SIGNAL(descriptorWritten(const QLowEnergyDescriptor, const QByteArray)), this,
                SLOT(descriptorWritten(const QLowEnergyDescriptor, const QByteArray)));

        QByteArray descriptor;
        descriptor.append((char)0x01);
        descriptor.append((char)0x00);
        gattCommunicationChannelService->writeDescriptor(
            gattNotifyCharacteristic.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration), descriptor);
    }
}

void heartratebelt::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    debug("descriptorWritten " + descriptor.name() + " " + newValue.toHex(' '));
}

void heartratebelt::characteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    Q_UNUSED(characteristic);
    debug("characteristicWritten " + newValue.toHex(' '));
}

void heartratebelt::serviceScanDone(void) {
    debug("serviceScanDone");

    foreach (QBluetoothUuid s, m_control->services()) {
        qDebug() << "heartRateBelt services " << s.toString();
        if (s == QBluetoothUuid::HeartRate) {
            QBluetoothUuid _gattCommunicationChannelServiceId(QBluetoothUuid::HeartRate);
            gattCommunicationChannelService = m_control->createServiceObject(_gattCommunicationChannelServiceId);
            connect(gattCommunicationChannelService, SIGNAL(stateChanged(QLowEnergyService::ServiceState)), this,
                    SLOT(stateChanged(QLowEnergyService::ServiceState)));
            gattCommunicationChannelService->discoverDetails();
            return;
        }
    }
}

void heartratebelt::errorService(QLowEnergyService::ServiceError err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    debug("heartratebelt::errorService" + QString::fromLocal8Bit(metaEnum.valueToKey(err)) + m_control->errorString());
}

void heartratebelt::error(QLowEnergyController::Error err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    debug("heartratebelt::error" + QString::fromLocal8Bit(metaEnum.valueToKey(err)) + m_control->errorString());
}

void heartratebelt::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    QSettings settings;
    QString heartRateBeltName = settings.value("heart_rate_belt_name", "Disabled").toString();
    debug("Found new device: " + device.name() + " (" + device.address().toString() + ')');
    // if(device.name().startsWith(heartRateBeltName))
    {
        bluetoothDevice = device;
        m_control = QLowEnergyController::createCentral(bluetoothDevice, this);
        connect(m_control, SIGNAL(serviceDiscovered(const QBluetoothUuid &)), this,
                SLOT(serviceDiscovered(const QBluetoothUuid &)));
        connect(m_control, SIGNAL(discoveryFinished()), this, SLOT(serviceScanDone()));
        connect(m_control, SIGNAL(error(QLowEnergyController::Error)), this, SLOT(error(QLowEnergyController::Error)));
        connect(m_control, SIGNAL(stateChanged(QLowEnergyController::ControllerState)), this,
                SLOT(controllerStateChanged(QLowEnergyController::ControllerState)));

        connect(m_control,
                static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error),
                this, [this](QLowEnergyController::Error error) {
                    Q_UNUSED(error);
                    Q_UNUSED(this);
                    debug("Cannot connect to remote device.");
                    emit disconnected();
                });
        connect(m_control, &QLowEnergyController::connected, this, [this]() {
            Q_UNUSED(this);
            debug("Controller connected. Search services...");
            m_control->discoverServices();
        });
        connect(m_control, &QLowEnergyController::disconnected, this, [this]() {
            Q_UNUSED(this);
            debug("LowEnergy controller disconnected");
            emit disconnected();
        });

        // Connect
        m_control->connectToDevice();
        return;
    }
}

bool heartratebelt::connected() {
    if (!m_control)
        return false;
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

void heartratebelt::controllerStateChanged(QLowEnergyController::ControllerState state) {
    qDebug() << "controllerStateChanged" << state;
    if (state == QLowEnergyController::UnconnectedState && m_control) {
        qDebug() << "trying to connect back again...";
        m_control->connectToDevice();
    }
}
