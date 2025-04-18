#include "homeform.h"
#include "elitesquarecontroller.h"
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

// Define static constants
const QString elitesquarecontroller::DEVICE_NAME = "SQUARE";
const QBluetoothUuid elitesquarecontroller::SERVICE_UUID = QBluetoothUuid(QStringLiteral("347b0001-7635-408b-8918-8ff3949ce592"));
const QBluetoothUuid elitesquarecontroller::CHARACTERISTIC_UUID = QBluetoothUuid(QStringLiteral("347b0045-7635-408b-8918-8ff3949ce592"));

elitesquarecontroller::elitesquarecontroller(bluetoothdevice *parentDevice) {
#ifdef Q_OS_IOS
    QZ_EnableDiscoveryCharsAndDescripttors = true;
#endif
    this->parentDevice = parentDevice;

    // Initialize button state vector (24 buttons from 0-23)
    buttonState.resize(24);
    buttonState.fill(0);

    // Set up refresh timer
    refresh = new QTimer(this);
    connect(refresh, &QTimer::timeout, this, &elitesquarecontroller::update);
    refresh->start(1000ms);
}

void elitesquarecontroller::update() {
    // Just a simple heartbeat check - no handshake needed for Elite Square
    if (m_control && m_control->state() == QLowEnergyController::UnconnectedState) {
        // Try to reconnect if disconnected
        qDebug() << QStringLiteral("Elite Square disconnected, attempting to reconnect...");
        m_control->connectToDevice();
    }
}

void elitesquarecontroller::serviceDiscovered(const QBluetoothUuid &gatt) {
    emit debug(QStringLiteral("serviceDiscovered ") + gatt.toString());

    // Check if the discovered service is the Elite Square service
    if (gatt == SERVICE_UUID) {
        qDebug() << QStringLiteral("Elite Square service discovered");
    }
}

void elitesquarecontroller::disconnectBluetooth() {
    qDebug() << QStringLiteral("elitesquarecontroller::disconnect") << m_control;

    if (m_control) {
        m_control->disconnectFromDevice();
    }
}

void elitesquarecontroller::characteristicChanged(const QLowEnergyCharacteristic &characteristic,
                                               const QByteArray &newValue) {
    Q_UNUSED(characteristic);
    emit packetReceived();

    qDebug() << QStringLiteral(" << ") << newValue.toHex(' ') << QString(newValue);

    // Process the Elite Square button data
    if (characteristic.uuid() == CHARACTERISTIC_UUID) {
        // Process the raw bytes directly
        parseButtonData(newValue);
    }
}

void elitesquarecontroller::parseButtonData(const QByteArray &data) {
    // The data comes as raw bytes, with 11 bytes total
    if (data.size() < 11) {
        qDebug() << QStringLiteral("Invalid button data size: ") << data.size();
        return;
    }

    // Log the hex data for better debugging
    qDebug() << QStringLiteral("Processing button data: ") << data.toHex(' ');

    // Extract the important bytes (based on the provided Android code)
    uint8_t leftByte = data[5];
    uint8_t rightByte = data[10];

    // Extract the nibbles from these bytes
    uint8_t leftLargeValue = (leftByte >> 4) & 0xF;   // Left Shift Button 1
    uint8_t leftSmallValue = leftByte & 0xF;          // Left Shift Button 2
    uint8_t rightLargeValue = (rightByte >> 4) & 0xF; // Right Shift Button 1
    uint8_t rightSmallValue = rightByte & 0xF;        // Right Shift Button 2

    qDebug() << QStringLiteral("Button values: leftLarge=") << leftLargeValue
             << QStringLiteral(" leftSmall=") << leftSmallValue
             << QStringLiteral(" rightLarge=") << rightLargeValue
             << QStringLiteral(" rightSmall=") << rightSmallValue;

    // Check if the left large shift button (Left Shift Button 1) is pressed
    // According to the code, odd values indicate press events
    if (leftLargeValue != buttonState[BUTTON_LEFT_SHIFT_1] && leftLargeValue % 2 == 1) {
        qDebug() << QStringLiteral("Left Shift Button 1 pressed (shift down 1)");
        emit minus();
    }
    buttonState[BUTTON_LEFT_SHIFT_1] = leftLargeValue;

    // Check if the left small shift button (Left Shift Button 2) is pressed
    if (leftSmallValue != buttonState[BUTTON_LEFT_SHIFT_2] && leftSmallValue % 2 == 1) {
        qDebug() << QStringLiteral("Left Shift Button 2 pressed (shift down 3)");
        // Emit minus three times for triple downshift
        emit minus();
        emit minus();
        emit minus();
    }
    buttonState[BUTTON_LEFT_SHIFT_2] = leftSmallValue;

    // Check if the right large shift button (Right Shift Button 1) is pressed
    if (rightLargeValue != buttonState[BUTTON_RIGHT_SHIFT_1] && rightLargeValue % 2 == 1) {
        qDebug() << QStringLiteral("Right Shift Button 1 pressed (shift up 1)");
        emit plus();
    }
    buttonState[BUTTON_RIGHT_SHIFT_1] = rightLargeValue;

    // Check if the right small shift button (Right Shift Button 2) is pressed
    if (rightSmallValue != buttonState[BUTTON_RIGHT_SHIFT_2] && rightSmallValue % 2 == 1) {
        qDebug() << QStringLiteral("Right Shift Button 2 pressed (shift up 3)");
        // Emit plus three times for triple upshift
        emit plus();
        emit plus();
        emit plus();
    }
    buttonState[BUTTON_RIGHT_SHIFT_2] = rightSmallValue;

    // Check for steering buttons and other controls
    // Assuming byte 3 might contain steering information as in our previous implementation
    uint8_t controlByte = data[3];

    // These conditions would need to be adjusted based on actual behavior
    if (controlByte == 0x60 && data[3] != buttonState[BUTTON_X]) {
        emit steeringLeft(true);
        buttonState[BUTTON_X] = controlByte;
    } else if (controlByte == 0x20 && data[3] != buttonState[BUTTON_CIRCLE]) {
        // Need to make sure this doesn't conflict with shift button detection
        emit steeringRight(true);
        buttonState[BUTTON_CIRCLE] = controlByte;
    } else if (controlByte == 0x00) {
        emit steeringLeft(false);
        emit steeringRight(false);
        buttonState[BUTTON_X] = 0;
        buttonState[BUTTON_CIRCLE] = 0;
    }
}

void elitesquarecontroller::stateChanged(QLowEnergyService::ServiceState state) {
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
            connect(s, &QLowEnergyService::characteristicChanged, this, &elitesquarecontroller::characteristicChanged);
            
            // Updated for Qt6: Use errorOccurred instead of error signal
            connect(s, &QLowEnergyService::errorOccurred, this, &elitesquarecontroller::errorService);
            
            connect(s, &QLowEnergyService::descriptorWritten, this, &elitesquarecontroller::descriptorWritten);

            qDebug() << s->serviceUuid() << QStringLiteral("connected!");

            // Check if this is the Elite Square service
            if (s->serviceUuid() == SERVICE_UUID) {
                gattService = s;

                // Find the notification characteristic
                auto characteristics_list = s->characteristics();
                for (const QLowEnergyCharacteristic &c : qAsConst(characteristics_list)) {
                    if (c.uuid() == CHARACTERISTIC_UUID) {
                        gattNotifyCharacteristic = c;

                        // Subscribe to notifications
                        if ((c.properties() & QLowEnergyCharacteristic::Notify) == QLowEnergyCharacteristic::Notify) {
                            QByteArray descriptor;
                            descriptor.append((char)0x01);
                            descriptor.append((char)0x00);
                            if (c.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration).isValid()) {
                                s->writeDescriptor(c.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration), descriptor);
                                connectionEstablished = true;
                                qDebug() << QStringLiteral("Elite Square notification subscribed!");
                            } else {
                                qDebug() << QStringLiteral("ClientCharacteristicConfiguration is not valid");
                            }
                        }
                    }
                }
            }
        }
    }
}

void elitesquarecontroller::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    emit debug(QStringLiteral("descriptorWritten ") + descriptor.name() + " " + newValue.toHex(' '));
}

void elitesquarecontroller::serviceScanDone(void) {
    emit debug(QStringLiteral("serviceScanDone"));

    auto services_list = m_control->services();
    for (const QBluetoothUuid &s : qAsConst(services_list)) {
        gattCommunicationChannelService.append(m_control->createServiceObject(s));
        if (gattCommunicationChannelService.constLast()) {
            connect(gattCommunicationChannelService.constLast(), &QLowEnergyService::stateChanged, this,
                    &elitesquarecontroller::stateChanged);
            gattCommunicationChannelService.constLast()->discoverDetails();
        } else {
            m_control->disconnectFromDevice();
        }
    }
}

void elitesquarecontroller::errorService(QLowEnergyService::ServiceError err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    emit debug(QStringLiteral("elitesquarecontroller::errorService") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void elitesquarecontroller::error(QLowEnergyController::Error err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    emit debug(QStringLiteral("elitesquarecontroller::error") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void elitesquarecontroller::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    emit debug(QStringLiteral("Found new device: ") + device.name() + QStringLiteral(" (") +
               device.address().toString() + ')');

    // Check if this is the Elite Square device
    if (device.name() == DEVICE_NAME) {
        bluetoothDevice = device;
        m_control = QLowEnergyController::createCentral(bluetoothDevice, this);
        connect(m_control, &QLowEnergyController::serviceDiscovered, this, &elitesquarecontroller::serviceDiscovered);
        connect(m_control, &QLowEnergyController::discoveryFinished, this, &elitesquarecontroller::serviceScanDone);
        
        // Updated for Qt6: Use errorOccurred instead of error signal
        connect(m_control, &QLowEnergyController::errorOccurred, this, &elitesquarecontroller::error);
        
        connect(m_control, &QLowEnergyController::stateChanged, this, &elitesquarecontroller::controllerStateChanged);

        // Updated for Qt6: Use errorOccurred instead of error signal
        connect(m_control, &QLowEnergyController::errorOccurred, this, [this](QLowEnergyController::Error error) {
            Q_UNUSED(error);
            Q_UNUSED(this);
            emit debug(QStringLiteral("Cannot connect to Elite Square device."));
            emit disconnected();
        });
        
        connect(m_control, &QLowEnergyController::connected, this, [this]() {
            Q_UNUSED(this);
            emit debug(QStringLiteral("Elite Square controller connected. Searching services..."));
            m_control->discoverServices();
        });
        
        connect(m_control, &QLowEnergyController::disconnected, this, [this]() {
            Q_UNUSED(this);
            emit debug(QStringLiteral("Elite Square controller disconnected"));
            connectionEstablished = false;
            emit disconnected();
        });

        // Connect to the device
        m_control->connectToDevice();
    }
}

bool elitesquarecontroller::connected() {
    if (!m_control) {
        return false;
    }
    return connectionEstablished && (m_control->state() == QLowEnergyController::DiscoveredState);
}

void elitesquarecontroller::controllerStateChanged(QLowEnergyController::ControllerState state) {
    qDebug() << QStringLiteral("controllerStateChanged") << state;
    if (state == QLowEnergyController::UnconnectedState && m_control) {
        qDebug() << QStringLiteral("trying to connect back again...");
        connectionEstablished = false;
        m_control->connectToDevice();
    }
}