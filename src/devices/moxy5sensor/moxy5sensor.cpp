#include "moxy5sensor.h"
#include "homeform.h"
#include <QBluetoothLocalDevice>
#include <QDateTime>
#include <QEventLoop>
#include <QFile>
#include <QMetaEnum>
#include <QSettings>
#include <QThread>

moxy5sensor::moxy5sensor() : 
    m_currentSaturatedHemoglobin(0.0),
    m_previousSaturatedHemoglobin(0.0),
    m_totalHemoglobinConcentration(0.0) {
}

moxy5sensor::~moxy5sensor() {

}

void moxy5sensor::update() {
    QSettings settings;

}

void moxy5sensor::serviceDiscovered(const QBluetoothUuid &gatt) {
    qDebug() << QStringLiteral("serviceDiscovered ") + gatt.toString();
}

void moxy5sensor::disconnectBluetooth() {
    qDebug() << QStringLiteral("moxy5sensor::disconnect") << m_control;

    if (m_control) {
        m_control->disconnectFromDevice();
    }
}

void moxy5sensor::characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    Q_UNUSED(characteristic);
    emit packetReceived();

    qDebug() << QStringLiteral(" << ") + newValue.toHex(' ');

    // Check if there's enough data in the packet
    if (newValue.length() >= 8) {
        // Parse data according to Moxy 5 format
        // Assuming data is organized as percentages or scaled values
        
        // Extract values from packet bytes
        // Note: this parsing is an assumption and should be adapted to the actual Moxy 5 data structure
        double currentSaturation = (double)((uint16_t)((uint8_t)newValue[2] | ((uint8_t)newValue[3] << 8))) / 100.0;
        double previousSaturation = (double)((uint16_t)((uint8_t)newValue[4] | ((uint8_t)newValue[5] << 8))) / 100.0;
        double totalHemoglobin = (double)((uint16_t)((uint8_t)newValue[6] | ((uint8_t)newValue[7] << 8))) / 10.0;
        
        // Update internal variables
        if (m_currentSaturatedHemoglobin != currentSaturation) {
            m_currentSaturatedHemoglobin = currentSaturation;
            emit currentSaturatedHemoglobinChanged(m_currentSaturatedHemoglobin);
        }
        
        if (m_previousSaturatedHemoglobin != previousSaturation) {
            m_previousSaturatedHemoglobin = previousSaturation;
            emit previousSaturatedHemoglobinChanged(m_previousSaturatedHemoglobin);
        }
        
        if (m_totalHemoglobinConcentration != totalHemoglobin) {
            m_totalHemoglobinConcentration = totalHemoglobin;
            emit totalHemoglobinConcentrationChanged(m_totalHemoglobinConcentration);
        }
        
        // Debug log
        qDebug() << QStringLiteral("Current SmO2: ") + QString::number(m_currentSaturatedHemoglobin, 'f', 2) + "% " +
                  QStringLiteral("Previous SmO2: ") + QString::number(m_previousSaturatedHemoglobin, 'f', 2) + "% " +
                  QStringLiteral("THb: ") + QString::number(m_totalHemoglobinConcentration, 'f', 2) + " g/dL";

        homeform::singleton()->setToastRequested(QStringLiteral("Current SmO2: ") + QString::number(m_currentSaturatedHemoglobin, 'f', 2) + "% " +
                                                 QStringLiteral("Previous SmO2: ") + QString::number(m_previousSaturatedHemoglobin, 'f', 2) + "% " +
                                                 QStringLiteral("THb: ") + QString::number(m_totalHemoglobinConcentration, 'f', 2) + " g/dL");
    }
}

void moxy5sensor::stateChanged(QLowEnergyService::ServiceState state) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceState>();
    qDebug() << QStringLiteral("BTLE stateChanged ") + QString::fromLocal8Bit(metaEnum.valueToKey(state));

    if (state == QLowEnergyService::ServiceDiscovered) {
        auto characteristics_list = gattCommunicationChannelService->characteristics();
        for (const QLowEnergyCharacteristic &c : qAsConst(characteristics_list)) {
            qDebug() << QStringLiteral("characteristic ") + c.uuid().toString();
        }

        // Trova la caratteristica del sensore Moxy
        gattNotifyCharacteristic =
            gattCommunicationChannelService->characteristic(QBluetoothUuid(QString(MOXY5_CHARACTERISTIC_UUID)));
        
        if(!gattNotifyCharacteristic.isValid()) {
            qDebug() << "gattNotifyCharacteristic not valid for Moxy5";
            return;
        }

        // establish hook into notifications
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicChanged, this,
                &moxy5sensor::characteristicChanged);
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicWritten, this,
                &moxy5sensor::characteristicWritten);
        connect(gattCommunicationChannelService,
                &QLowEnergyService::errorOccurred,
                this, &moxy5sensor::errorService);
        connect(gattCommunicationChannelService, &QLowEnergyService::descriptorWritten, this,
                &moxy5sensor::descriptorWritten);

        // Abilita le notifiche per questa caratteristica
        QByteArray descriptor;
        descriptor.append((char)0x01);
        descriptor.append((char)0x00);
        gattCommunicationChannelService->writeDescriptor(
            gattNotifyCharacteristic.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration), descriptor);
    }
}

void moxy5sensor::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    qDebug() << QStringLiteral("descriptorWritten ") + descriptor.name() + " " + newValue.toHex(' ');
}

void moxy5sensor::characteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    Q_UNUSED(characteristic);
    qDebug() << QStringLiteral("characteristicWritten ") + newValue.toHex(' ');
}

void moxy5sensor::serviceScanDone(void) {
    qDebug() << QStringLiteral("serviceScanDone");

    auto services_list = m_control->services();
    for (const QBluetoothUuid &s : qAsConst(services_list)) {
        qDebug() << QStringLiteral("moxy5sensor services ") << s.toString();
        
        // Cerca il servizio specifico del Moxy 5
        if (s == QBluetoothUuid(QString(MOXY5_SERVICE_UUID))) {
            QBluetoothUuid _gattCommunicationChannelServiceId(QString(MOXY5_SERVICE_UUID));
            gattCommunicationChannelService = m_control->createServiceObject(_gattCommunicationChannelServiceId);
            connect(gattCommunicationChannelService, &QLowEnergyService::stateChanged, this,
                    &moxy5sensor::stateChanged);
            gattCommunicationChannelService->discoverDetails();
            return;
        }
    }
}

void moxy5sensor::errorService(QLowEnergyService::ServiceError err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    qDebug() << QStringLiteral("moxy5sensor::errorService") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString();
}

void moxy5sensor::error(QLowEnergyController::Error err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    qDebug() << QStringLiteral("moxy5sensor::error") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString();
}

void moxy5sensor::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    QSettings settings;
    
    qDebug() << QStringLiteral("Found new device: ") + device.name() + QStringLiteral(" (") +
               device.address().toString() + ')';

    if(homeform::singleton())
        homeform::singleton()->setToastRequested(device.name() + QStringLiteral(" connected!"));

    // Controlla se il dispositivo è un Moxy 5 o semplicemente collegati a qualsiasi dispositivo che trovi
    // (qui puoi aggiungere un filtro per il nome se necessario)
    {
        bluetoothDevice = device;
        m_control = QLowEnergyController::createCentral(bluetoothDevice, this);
        connect(m_control, &QLowEnergyController::serviceDiscovered, this, &moxy5sensor::serviceDiscovered);
        connect(m_control, &QLowEnergyController::discoveryFinished, this, &moxy5sensor::serviceScanDone);
        connect(m_control,
                &QLowEnergyController::errorOccurred,
                this, &moxy5sensor::error);
        connect(m_control, &QLowEnergyController::stateChanged, this, &moxy5sensor::controllerStateChanged);

        connect(m_control,
                &QLowEnergyController::errorOccurred,
                this, [this](QLowEnergyController::Error error) {
                    Q_UNUSED(error);
                    Q_UNUSED(this);
                    qDebug() << QStringLiteral("Cannot connect to remote device.");
                    emit disconnected();
                });
        connect(m_control, &QLowEnergyController::connected, this, [this]() {
            Q_UNUSED(this);
            qDebug() << QStringLiteral("Controller connected. Search services...");
            m_control->discoverServices();
        });
        connect(m_control, &QLowEnergyController::disconnected, this, [this]() {
            Q_UNUSED(this);
            qDebug() << QStringLiteral("LowEnergy controller disconnected");
            emit disconnected();
        });

        // Connect
        m_control->connectToDevice();
        return;
    }
}

bool moxy5sensor::connected() {
    if (!m_control) {
        return false;
    }
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

void moxy5sensor::controllerStateChanged(QLowEnergyController::ControllerState state) {
    qDebug() << QStringLiteral("controllerStateChanged") << state;
    
    if (state == QLowEnergyController::UnconnectedState && m_control) {
        qDebug() << QStringLiteral("trying to connect back again...");
        m_control->connectToDevice();
    }
}
