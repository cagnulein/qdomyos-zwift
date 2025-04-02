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
    if (data.size() != 11) {
        qDebug() << QStringLiteral("Invalid button data size: ") << data.size();
        return;
    }

    // Process each byte individually
    for (int i = 0; i < data.size(); i++) {
        int value = static_cast<unsigned char>(data[i]);

        // Process the button value
        // The position in the array determines which button(s) this value corresponds to
        // For simplicity, we're mapping array position to button IDs directly:
        // Position 0-10 in the array maps to buttonIds starting at multiples of 2

        // Each button has 2 states - odd values for press, even values for release
        int buttonIndex = i * 2 + 1; // Map to our button enum values

        // Check if the value has changed from the previously known state
        if (value != buttonState[buttonIndex]) {
            handleButtonStateChange(buttonIndex, value);
            buttonState[buttonIndex] = value;
        }

        // Check the next button in the pair if applicable
        if (buttonIndex + 1 < buttonState.size()) {
            if (value != buttonState[buttonIndex + 1]) {
                handleButtonStateChange(buttonIndex + 1, value);
                buttonState[buttonIndex + 1] = value;
            }
        }
    }
}

void elitesquarecontroller::handleButtonStateChange(int buttonId, int newValue) {
    // Determine if this is a press (odd value) or release (even value)
    bool isPressed = (newValue % 2 == 1);

    // Emit the general button signal
    emit buttonActivated(buttonId, isPressed);

    // Handle special button functionality
    switch (buttonId) {
    case BUTTON_LEFT_SHIFT_1:
        if (isPressed) {
            // Shift down 1 gear
            emit minus();
        }
        break;

    case BUTTON_LEFT_SHIFT_2:
        if (isPressed) {
            // Shift down 3 gears (call minus() 3 times)
            emit minus();
            emit minus();
            emit minus();
        }
        break;

    case BUTTON_RIGHT_SHIFT_1:
        if (isPressed) {
            // Shift up 1 gear
            emit plus();
        }
        break;

    case BUTTON_RIGHT_SHIFT_2:
        if (isPressed) {
            // Shift up 3 gears (call plus() 3 times)
            emit plus();
            emit plus();
            emit plus();
        }
        break;

    case BUTTON_X:
    case BUTTON_LEFT_CAMPAGNOLO:
        emit steeringLeft(isPressed); // Left steering (active while pressed)
        break;

    case BUTTON_CIRCLE:
    case BUTTON_RIGHT_CAMPAGNOLO:
        emit steeringRight(isPressed); // Right steering (active while pressed)
        break;

        // The rest of the buttons will use the general buttonActivated signal
        // They will be mapped to Zwift functions elsewhere in the code
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
            connect(
                s, static_cast<void (QLowEnergyService::*)(QLowEnergyService::ServiceError)>(&QLowEnergyService::error),
                this, &elitesquarecontroller::errorService);
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
        connect(m_control,
                static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error),
                this, &elitesquarecontroller::error);
        connect(m_control, &QLowEnergyController::stateChanged, this, &elitesquarecontroller::controllerStateChanged);

        connect(m_control,
                static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error),
                this, [this](QLowEnergyController::Error error) {
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

void elitesquarecontroller::processButtonEvent(int buttonId, EliteSquareButtonState state) {
    // This can be used to manually trigger button events (for testing)
    bool isPressed = (state == PRESSED);

    // Update the button state
    buttonState[buttonId] = isPressed ? 1 : 0;

    // Process the button event
    handleButtonStateChange(buttonId, isPressed ? 1 : 0);
}
