#include "coresensor.h"
#include "homeform.h"
#include <QBluetoothLocalDevice>
#include <QDateTime>
#include <QEventLoop>
#include <QFile>
#include <QMetaEnum>
#include <QSettings>
#include <QThread>

coresensor::coresensor() :
    m_sensorHeartRate(0),
    m_dataQuality(0),
    m_heartRateState(0),
    m_isCelsius(true) {
}

coresensor::~coresensor() {
    disconnectBluetooth();
}

bool coresensor::connected() {
    if (!m_control) {
        return false;
    }
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

metric coresensor::currentHeart() {
    // If we have a valid value from the sensor, create a metric with that value
    if (m_sensorHeartRate > 0) {
        metric heartMetric;
        heartMetric.setValue(m_sensorHeartRate);
        return heartMetric;
    }

    // Otherwise return the value from the base class
    return bluetoothdevice::currentHeart();
}

void coresensor::update() {
    // This method can be used for periodic tasks or calculations
    QSettings settings;
    // Future implementation if needed
}

void coresensor::disconnectBluetooth() {
    qDebug() << QStringLiteral("coresensor::disconnect") << m_control;

    if (m_control) {
        m_control->disconnectFromDevice();
    }
}

void coresensor::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    QSettings settings;

    qDebug() << QStringLiteral("Found new device: ") + device.name() + QStringLiteral(" (") +
               device.address().toString() + ')';

    if(homeform::singleton())
        homeform::singleton()->setToastRequested(device.name() + QStringLiteral(" connected!"));

    {
        bluetoothDevice = device;
        m_control = QLowEnergyController::createCentral(bluetoothDevice, this);
        connect(m_control, &QLowEnergyController::serviceDiscovered, this, &coresensor::serviceDiscovered);
        connect(m_control, &QLowEnergyController::discoveryFinished, this, &coresensor::serviceScanDone);
        connect(m_control,
                &QLowEnergyController::errorOccurred,
                this, &coresensor::handleError);
        connect(m_control, &QLowEnergyController::stateChanged, this, &coresensor::controllerStateChanged);

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

void coresensor::serviceDiscovered(const QBluetoothUuid &gatt) {
    qDebug() << QStringLiteral("serviceDiscovered ") + gatt.toString();
}

void coresensor::serviceScanDone() {
    qDebug() << QStringLiteral("serviceScanDone");

    auto services_list = m_control->services();
    for (const QBluetoothUuid &s : qAsConst(services_list)) {
        qDebug() << QStringLiteral("coresensor services ") << s.toString();

        // Look for the specific CORE sensor service
        if (s == QBluetoothUuid(QString(CORE_SERVICE_UUID))) {
            QBluetoothUuid coreServiceId(QString(CORE_SERVICE_UUID));
            m_coreService = m_control->createServiceObject(coreServiceId);
            connect(m_coreService, &QLowEnergyService::stateChanged, this,
                    &coresensor::serviceStateChanged);
            connect(m_coreService,
                    &QLowEnergyService::error,
                    this, &coresensor::handleServiceError);
            m_coreService->discoverDetails();
            return;
        }
    }

    // If we reach here, we didn't find the CORE service
    qDebug() << QStringLiteral("CORE service not found!");
}

void coresensor::serviceStateChanged(QLowEnergyService::ServiceState state) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceState>();
    qDebug() << QStringLiteral("BTLE stateChanged ") + QString::fromLocal8Bit(metaEnum.valueToKey(state));

    if (state == QLowEnergyService::ServiceDiscovered) {
        auto characteristics_list = m_coreService->characteristics();
        for (const QLowEnergyCharacteristic &c : qAsConst(characteristics_list)) {
            qDebug() << QStringLiteral("characteristic ") + c.uuid().toString();
        }

        // Find the Core Body Temperature characteristic
        m_coreTemperatureCharacteristic =
            m_coreService->characteristic(QBluetoothUuid(QString(CORE_TEMPERATURE_CHAR_UUID)));

        if (!m_coreTemperatureCharacteristic.isValid()) {
            qDebug() << "Core Body Temperature characteristic not valid";
            return;
        }

        // Find the Control Point characteristic
        m_coreControlPointCharacteristic =
            m_coreService->characteristic(QBluetoothUuid(QString(CORE_CONTROL_POINT_UUID)));

        if (!m_coreControlPointCharacteristic.isValid()) {
            qDebug() << "Core Control Point characteristic not valid";
            // We can continue without control point if only reading temperature
        }

        // Set up notifications and handle characteristic changes
        connect(m_coreService, &QLowEnergyService::characteristicChanged, this,
                &coresensor::characteristicChanged);
        connect(m_coreService, &QLowEnergyService::characteristicWritten, this,
                &coresensor::characteristicWritten);
        connect(m_coreService, &QLowEnergyService::descriptorWritten, this,
                &coresensor::descriptorWritten);

        // Enable notifications for Core Body Temperature characteristic
        QByteArray descriptor;
        descriptor.append((char)0x01);
        descriptor.append((char)0x00);
        m_coreService->writeDescriptor(
            m_coreTemperatureCharacteristic.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration),
            descriptor);

        // For Control Point, we need to enable indications (not notifications)
        if (m_coreControlPointCharacteristic.isValid()) {
            QByteArray cpDescriptor;
            cpDescriptor.append((char)0x02);  // 0x02 for indications (different from notifications)
            cpDescriptor.append((char)0x00);
            m_coreService->writeDescriptor(
                m_coreControlPointCharacteristic.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration),
                cpDescriptor);
        }
    }
}

void coresensor::characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    emit packetReceived();

    qDebug() << QStringLiteral(" << ") + newValue.toHex(' ');

    // Handle Core Body Temperature characteristic updates
    if (characteristic.uuid() == QBluetoothUuid(QString(CORE_TEMPERATURE_CHAR_UUID))) {
        if (newValue.length() < 1) {
            qDebug() << "Received invalid data packet (too short)";
            return;
        }

        // Parse flags field (first byte)
        uint8_t flags = newValue.at(0);
        bool hasSkinTemp = (flags & 0x01);
        bool hasCoreReserved = (flags & 0x02);
        bool hasQualityState = (flags & 0x04);
        bool isFahrenheit = (flags & 0x08);
        bool hasHeartRate = (flags & 0x10);
        bool hasHeatStrainIndex = (flags & 0x20);

        // Update temperature unit flag
        bool newIsCelsius = !isFahrenheit;
        if (m_isCelsius != newIsCelsius) {
            m_isCelsius = newIsCelsius;
            emit isCelsiusChanged(m_isCelsius);
        }

        int offset = 1; // Start after flags byte

        // Core body temperature (mandatory)
        if (newValue.length() >= (offset + 2)) {
            int16_t tempRaw = (int16_t)((uint8_t)newValue[offset] | ((uint8_t)newValue[offset+1] << 8));

            // Special value for 'Data not available'
            if (tempRaw == 0x7FFF) {
                qDebug() << "Core body temperature: Data not available";
            } else {
                double temp = tempRaw / 100.0; // Convert from 0.01°C/F to °C/F
                CoreBodyTemperature.setValue(temp);
                emit coreBodyTemperatureChanged(temp);
            }
            offset += 2;
        }

        // Skin temperature (optional)
        if (hasSkinTemp && newValue.length() >= (offset + 2)) {
            int16_t tempRaw = (int16_t)((uint8_t)newValue[offset] | ((uint8_t)newValue[offset+1] << 8));
            double temp = tempRaw / 100.0; // Convert from 0.01°C/F to °C/F
            SkinTemperature.setValue(temp);
            emit skinTemperatureChanged(temp);
            offset += 2;
        }

        // Core reserved (optional)
        if (hasCoreReserved && newValue.length() >= (offset + 2)) {
            int16_t tempRaw = (int16_t)((uint8_t)newValue[offset] | ((uint8_t)newValue[offset+1] << 8));
            double temp = tempRaw / 100.0; // Convert from 0.01°C/F to °C/F
            emit coreReservedChanged(temp);
            offset += 2;
        }

        // Quality and state (optional)
        if (hasQualityState && newValue.length() >= (offset + 1)) {
            uint8_t qualityState = newValue.at(offset);
            int newDataQuality = qualityState & 0x0F; // Lower 4 bits, actually using only 3 bits
            int newHeartRateState = (qualityState >> 4) & 0x03; // Bits 4-5

            if (m_dataQuality != newDataQuality) {
                m_dataQuality = newDataQuality;
                emit dataQualityChanged(m_dataQuality);
            }

            if (m_heartRateState != newHeartRateState) {
                m_heartRateState = newHeartRateState;
                emit heartRateStateChanged(m_heartRateState);
            }
            offset += 1;
        }

        // Heart rate (optional)
        if (hasHeartRate && newValue.length() >= (offset + 1)) {
            uint8_t hr = newValue.at(offset);
            if (m_sensorHeartRate != hr) {
                m_sensorHeartRate = hr;
                emit heartRateChanged(m_sensorHeartRate);

                // Forward heart rate to base class if it's a valid value
                if (hr > 0) {
                    bluetoothdevice::heartRate(hr);
                }
            }
            offset += 1;
        }

        // Heat Strain Index (optional)
        if (hasHeatStrainIndex && newValue.length() >= (offset + 1)) {
            uint8_t hsiRaw = newValue.at(offset);
            double hsi = hsiRaw / 10.0; // Convert from 0.1 units to actual value
            if (HeatStrainIndex.value() != hsi) {
                HeatStrainIndex = hsi;
                emit heatStrainIndexChanged(HeatStrainIndex.value());
            }
            offset += 1;
        }

        // Log data for debugging
        QString logMsg = QStringLiteral("CORE Sensor: ");
        logMsg += QStringLiteral("Temperature: ") + QString::number(CoreBodyTemperature.value(), 'f', 2) +
                 (m_isCelsius ? "°C" : "°F");

        if (hasSkinTemp) {
            logMsg += QStringLiteral(" Skin: ") + QString::number(SkinTemperature.value(), 'f', 2) +
                     (m_isCelsius ? "°C" : "°F");
        }

        if (hasHeartRate) {
            logMsg += QStringLiteral(" HR: ") + QString::number(m_sensorHeartRate) + QStringLiteral(" BPM");
        }

        if (hasHeatStrainIndex) {
            logMsg += QStringLiteral(" HSI: ") + QString::number(HeatStrainIndex.value(), 'f', 1);
        }

        qDebug() << logMsg;

    } else if (characteristic.uuid() == QBluetoothUuid(QString(CORE_CONTROL_POINT_UUID))) {
        // Handle Control Point responses
        handleControlPointResponse(newValue);
    }
}

void coresensor::handleControlPointResponse(const QByteArray &value) {
    if (value.length() < 3) {
        qDebug() << "Invalid Control Point response (too short)";
        return;
    }

    // First byte should be 0x80 (response code)
    if ((uint8_t)value.at(0) != 0x80) {
        qDebug() << "Invalid Control Point response (expected 0x80 response code)";
        return;
    }

    uint8_t requestOpCode = value.at(1);
    uint8_t resultCode = value.at(2);

    QString result;
    switch (resultCode) {
        case 0x01: result = "Success"; break;
        case 0x02: result = "Op Code not supported"; break;
        case 0x03: result = "Invalid Parameter"; break;
        case 0x04: result = "Operation Failed"; break;
        default: result = "Unknown result code"; break;
    }

    qDebug() << "Control Point response for OpCode" << QString("0x%1").arg(requestOpCode, 2, 16, QChar('0'))
             << "Result:" << result;

    m_operationInProgress = false;
}

void coresensor::characteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    Q_UNUSED(characteristic);
    qDebug() << QStringLiteral("characteristicWritten ") + newValue.toHex(' ');
}

void coresensor::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    qDebug() << QStringLiteral("descriptorWritten ") + descriptor.name() + " " + newValue.toHex(' ');
}

void coresensor::controllerStateChanged(QLowEnergyController::ControllerState state) {
    qDebug() << QStringLiteral("controllerStateChanged") << state;

    if (state == QLowEnergyController::UnconnectedState && m_control) {
        qDebug() << QStringLiteral("trying to connect back again...");
        m_control->connectToDevice();
    }
}

void coresensor::handleError(QLowEnergyController::Error err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    qDebug() << QStringLiteral("coresensor::error ") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString();
}

void coresensor::handleServiceError(QLowEnergyService::ServiceError err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    qDebug() << QStringLiteral("coresensor::serviceError ") + QString::fromLocal8Bit(metaEnum.valueToKey(err));
}

bool coresensor::setExternalHeartRate(uint8_t bpm) {
    if (!m_coreControlPointCharacteristic.isValid() || m_operationInProgress) {
        return false;
    }

    QByteArray request;
    request.append((char)0x13);  // OpCode for external heart rate
    request.append((char)bpm);   // Heart rate value

    m_operationInProgress = true;
    m_coreService->writeCharacteristic(m_coreControlPointCharacteristic, request, QLowEnergyService::WriteWithResponse);

    return true;
}

bool coresensor::disableExternalHeartRate() {
    if (!m_coreControlPointCharacteristic.isValid() || m_operationInProgress) {
        return false;
    }

    QByteArray request;
    request.append((char)0x13);  // OpCode for external heart rate
    // No additional data means disable external heart rate

    m_operationInProgress = true;
    m_coreService->writeCharacteristic(m_coreControlPointCharacteristic, request, QLowEnergyService::WriteWithResponse);

    return true;
}

bool coresensor::scanForBLEHeartRateMonitors() {
    if (!m_coreControlPointCharacteristic.isValid() || m_operationInProgress) {
        return false;
    }

    QByteArray request;
    request.append((char)0x0D);  // OpCode for scan BLE HRMs
    request.append((char)0xFF);  // Parameter for starting scan

    m_operationInProgress = true;
    m_coreService->writeCharacteristic(m_coreControlPointCharacteristic, request, QLowEnergyService::WriteWithResponse);

    return true;
}

bool coresensor::scanForANTHeartRateMonitors() {
    if (!m_coreControlPointCharacteristic.isValid() || m_operationInProgress) {
        return false;
    }

    QByteArray request;
    request.append((char)0x0A);  // OpCode for scan ANT+ HRMs
    request.append((char)0xFF);  // Parameter for starting scan

    m_operationInProgress = true;
    m_coreService->writeCharacteristic(m_coreControlPointCharacteristic, request, QLowEnergyService::WriteWithResponse);

    return true;
}

int coresensor::getNumberOfPairedHeartRateMonitors() {
    // This is a simple implementation that requests the number of paired BLE HRMs
    // In a complete implementation, we would make an async request and store the result

    if (!m_coreControlPointCharacteristic.isValid() || m_operationInProgress) {
        return -1; // Error condition
    }

    // Request total number of paired BLE heart rate monitors
    QByteArray request;
    request.append((char)0x08);  // OpCode for get total number of paired BLE HRMs

    m_operationInProgress = true;
    m_coreService->writeCharacteristic(m_coreControlPointCharacteristic, request, QLowEnergyService::WriteWithResponse);

    // Note: in a real implementation, we would need to handle the response asynchronously
    // and store the result for a getter to access. This would require additional state tracking.

    return 0; // Placeholder value - in reality, would return cached value from last request
}
