#include "mqttpublisher.h"
#include "qzsettings.h"
#include "devices/elliptical.h"
#include <QDebug>
#include <QJsonObject>
#include <QJsonDocument>
#include <QCoreApplication>

MQTTPublisher::MQTTPublisher(const QString& host, quint16 port, QString username, QString password, bluetooth* manager, QObject *parent)
    : QObject(parent)
    , m_host(host)
    , m_port(port)
    , m_device(nullptr)
{
    m_client = new QMqttClient(this);
    m_timer = new QTimer(this);
    m_manager = manager;
    m_username = username;
    m_password = password;
    m_userNickname = getUserNickname();

    // Setup timer for periodic publishing
    m_timer->setInterval(500); // 500ms interval
    connect(m_timer, &QTimer::timeout, this, &MQTTPublisher::publishWorkoutData);

    // Setup MQTT client connections
    connect(m_client, &QMqttClient::connected, this, &MQTTPublisher::onConnected);
    connect(m_client, &QMqttClient::disconnected, this, &MQTTPublisher::onDisconnected);
    connect(m_client, &QMqttClient::errorChanged, this, &MQTTPublisher::onError);
    connect(m_client, &QMqttClient::messageReceived, this, &MQTTPublisher::onMessageReceived);
    if (m_manager) {
        connect(m_manager, &bluetooth::bluetoothDeviceConnected, this, &MQTTPublisher::onBluetoothDeviceConnected);
        connect(m_manager, &bluetooth::bluetoothDeviceDisconnected, this, &MQTTPublisher::onBluetoothDeviceDisconnected);
        connect(m_manager, &bluetooth::zwiftClickPlus, this, [this]() {
            publishMomentaryButtonPress("bike/zwift_click/plus");
        });
        connect(m_manager, &bluetooth::zwiftClickMinus, this, [this]() {
            publishMomentaryButtonPress("bike/zwift_click/minus");
        });
        connect(m_manager, &bluetooth::zwiftPlayPlus, this, [this]() {
            publishMomentaryButtonPress("bike/zwift_play/plus");
        });
        connect(m_manager, &bluetooth::zwiftPlayMinus, this, [this]() {
            publishMomentaryButtonPress("bike/zwift_play/minus");
        });
        connect(m_manager, &bluetooth::zwiftPlayLeftUp, this, [this](bool pressed) {
            publishToTopic("bike/zwift_play/left/up", pressed);
        });
        connect(m_manager, &bluetooth::zwiftPlayLeftDown, this, [this](bool pressed) {
            publishToTopic("bike/zwift_play/left/down", pressed);
        });
        connect(m_manager, &bluetooth::zwiftPlayLeftLeft, this, [this](bool pressed) {
            publishToTopic("bike/zwift_play/left/left", pressed);
        });
        connect(m_manager, &bluetooth::zwiftPlayLeftRight, this, [this](bool pressed) {
            publishToTopic("bike/zwift_play/left/right", pressed);
        });
        connect(m_manager, &bluetooth::zwiftPlayLeftShoulder, this, [this](bool pressed) {
            publishToTopic("bike/zwift_play/left/shoulder", pressed);
        });
        connect(m_manager, &bluetooth::zwiftPlayLeftPower, this, [this](bool pressed) {
            publishToTopic("bike/zwift_play/left/power", pressed);
        });
        connect(m_manager, &bluetooth::zwiftPlayLeftPaddle, this, [this](int value) {
            publishToTopic("bike/zwift_play/left/paddle", value);
        });
        connect(m_manager, &bluetooth::zwiftRideLeftShiftUp, this, [this](bool pressed) {
            publishToTopic("bike/zwift_ride/left/shift_up", pressed);
        });
        connect(m_manager, &bluetooth::zwiftRideLeftShiftDown, this, [this](bool pressed) {
            publishToTopic("bike/zwift_ride/left/shift_down", pressed);
        });
        connect(m_manager, &bluetooth::zwiftRideLeftPowerUp, this, [this](bool pressed) {
            publishToTopic("bike/zwift_ride/left/power_up", pressed);
        });
        connect(m_manager, &bluetooth::zwiftRideLeftOnOff, this, [this](bool pressed) {
            publishToTopic("bike/zwift_ride/left/on_off", pressed);
        });
        connect(m_manager, &bluetooth::zwiftPlayRightY, this, [this](bool pressed) {
            publishToTopic("bike/zwift_play/right/y", pressed);
        });
        connect(m_manager, &bluetooth::zwiftPlayRightZ, this, [this](bool pressed) {
            publishToTopic("bike/zwift_play/right/z", pressed);
        });
        connect(m_manager, &bluetooth::zwiftPlayRightA, this, [this](bool pressed) {
            publishToTopic("bike/zwift_play/right/a", pressed);
        });
        connect(m_manager, &bluetooth::zwiftPlayRightB, this, [this](bool pressed) {
            publishToTopic("bike/zwift_play/right/b", pressed);
        });
        connect(m_manager, &bluetooth::zwiftPlayRightShoulder, this, [this](bool pressed) {
            publishToTopic("bike/zwift_play/right/shoulder", pressed);
        });
        connect(m_manager, &bluetooth::zwiftPlayRightPower, this, [this](bool pressed) {
            publishToTopic("bike/zwift_play/right/power", pressed);
        });
        connect(m_manager, &bluetooth::zwiftPlayRightPaddle, this, [this](int value) {
            publishToTopic("bike/zwift_play/right/paddle", value);
        });
        connect(m_manager, &bluetooth::zwiftRideRightZAlt, this, [this](bool pressed) {
            publishToTopic("bike/zwift_ride/right/z_alt", pressed);
        });
        connect(m_manager, &bluetooth::zwiftRideRightShiftUp, this, [this](bool pressed) {
            publishToTopic("bike/zwift_ride/right/shift_up", pressed);
        });
        connect(m_manager, &bluetooth::zwiftRideRightShiftDown, this, [this](bool pressed) {
            publishToTopic("bike/zwift_ride/right/shift_down", pressed);
        });
        connect(m_manager, &bluetooth::zwiftRideRightPower, this, [this](bool pressed) {
            publishToTopic("bike/zwift_ride/right/power", pressed);
        });
        connect(m_manager, &bluetooth::zwiftRideRightPowerUp, this, [this](bool pressed) {
            publishToTopic("bike/zwift_ride/right/power_up", pressed);
        });
        connect(m_manager, &bluetooth::zwiftRideRightOnOff, this, [this](bool pressed) {
            publishToTopic("bike/zwift_ride/right/on_off", pressed);
        });
        m_device = m_manager->device();
    }

    setupMQTTClient();
    start();
}

void MQTTPublisher::setupLastWillMessage() {
    QString statusTopic = getStatusTopic() + STATUS_TOPIC;
    
    // Set up Last Will and Testament message
    m_client->setWillTopic(statusTopic);
    m_client->setWillMessage(STATUS_OFFLINE.toUtf8());
    m_client->setWillQoS(1);
    m_client->setWillRetain(true);
}

void MQTTPublisher::publishOnlineStatus() {
    QString statusTopic = getStatusTopic() + STATUS_TOPIC;
    
    // Publish online status with retain flag
    m_client->publish(
        QMqttTopicName(statusTopic),
        STATUS_ONLINE.toUtf8(),
        1,  // QoS 1
        true  // retain flag
    );
}


MQTTPublisher::~MQTTPublisher() {
    stop();
}

void MQTTPublisher::setDevice(bluetoothdevice* device) {
    m_device = device;
}

void MQTTPublisher::onBluetoothDeviceConnected(bluetoothdevice *device) {
    m_device = device;
}

void MQTTPublisher::onBluetoothDeviceDisconnected() {
    m_device = nullptr;
    publishDefaultZwiftControllerStates();
}

QString MQTTPublisher::getUserNickname() const {
    QSettings settings;
    return settings.value(QZSettings::mqtt_deviceid, QZSettings::default_mqtt_deviceid).toString();
}

QString MQTTPublisher::getStatusTopic() const {
    return QString("QZ/%1/").arg(m_userNickname);
}

QString MQTTPublisher::getBaseTopic() const {
    return QString("QZ/%1/workout/").arg(m_userNickname);
}

QString MQTTPublisher::getControlTopic() const {
    return QString("QZ/%1/control/").arg(m_userNickname);
}

void MQTTPublisher::setupMQTTClient() {
    m_client->setHostname(m_host);
    m_client->setPort(m_port);
    if(m_username.length())
        m_client->setUsername(m_username);
    if(m_password.length())
        m_client->setPassword(m_password);
    
    setupLastWillMessage();
}

void MQTTPublisher::start() {
    connectToHost();
    m_timer->start();
}

void MQTTPublisher::stop() {
    m_timer->stop();
    if (m_client->state() == QMqttClient::Connected) {
        //m_client->disconnect();
    }
}

void MQTTPublisher::setHost(const QString& host) {
    m_host = host;
    m_client->setHostname(host);
}

void MQTTPublisher::setPort(quint16 port) {
    m_port = port;
    m_client->setPort(port);
}

bool MQTTPublisher::isConnected() const {
    return m_client->state() == QMqttClient::Connected;
}

void MQTTPublisher::connectToHost() {
    if (m_client->state() == QMqttClient::Disconnected) {
        m_client->connectToHost();
    }
}

void MQTTPublisher::onDisconnected() {
    qDebug() << "MQTT Client Disconnected";
}

void MQTTPublisher::onError(QMqttClient::ClientError error) {
    qDebug() << "MQTT Client Error:" << error;
}

bool MQTTPublisher::hasValueChanged(const QString& topic, const QVariant& newValue) {
    if (!m_lastPublishedValues.contains(topic)) {
        return true;
    }

    const QVariant& oldValue = m_lastPublishedValues[topic];

    // Handle floating point comparison with small epsilon
    if (newValue.userType() == QMetaType::Float || newValue.userType() == QMetaType::Double) {
        const double epsilon = 0.0001;
        return qAbs(newValue.toDouble() - oldValue.toDouble()) > epsilon;
    }

    return oldValue != newValue;
}

void MQTTPublisher::updateLastPublishedValue(const QString& topic, const QVariant& value) {
    m_lastPublishedValues[topic] = value;
}

void MQTTPublisher::publishToTopic(const QString& topic, const QVariant& value) {
    if (!isConnected()) return;

    QString fullTopic = getBaseTopic() + topic;

    // Only publish if value has changed
    if (!hasValueChanged(fullTopic, value)) {
        return;
    }

    QString payload;
    if (value.userType() == QMetaType::QJsonObject || value.userType() == QMetaType::QJsonArray) {
        payload = QJsonDocument::fromVariant(value).toJson(QJsonDocument::Compact);
    } else {
        switch (value.userType()) {
        case QMetaType::Bool:
            payload = value.toBool() ? "true" : "false";
            break;
        case QMetaType::Int:
        case QMetaType::UInt:
        case QMetaType::Long:
        case QMetaType::LongLong:
        case QMetaType::ULong:
        case QMetaType::ULongLong:
            payload = QString::number(value.toLongLong());
            break;
        case QMetaType::Float:
        case QMetaType::Double:
            payload = QString::number(value.toDouble(), 'f', 2);
            break;
        default:
            payload = value.toString();
            break;
        }
    }

    m_client->publish(QMqttTopicName(fullTopic), payload.toUtf8());
    updateLastPublishedValue(fullTopic, value);
}

void MQTTPublisher::publishMomentaryButtonPress(const QString& topic) {
    publishToTopic(topic, true);
    QTimer::singleShot(150, this, [this, topic]() {
        publishToTopic(topic, false);
    });
}

void MQTTPublisher::publishDefaultZwiftControllerStates() {
    publishToTopic("bike/zwift_click/plus", false);
    publishToTopic("bike/zwift_click/minus", false);
    publishToTopic("bike/zwift_play/plus", false);
    publishToTopic("bike/zwift_play/minus", false);
    publishToTopic("bike/zwift_play/left/up", false);
    publishToTopic("bike/zwift_play/left/down", false);
    publishToTopic("bike/zwift_play/left/left", false);
    publishToTopic("bike/zwift_play/left/right", false);
    publishToTopic("bike/zwift_play/left/shoulder", false);
    publishToTopic("bike/zwift_play/left/power", false);
    publishToTopic("bike/zwift_play/left/paddle", 0);
    publishToTopic("bike/zwift_ride/left/shift_up", false);
    publishToTopic("bike/zwift_ride/left/shift_down", false);
    publishToTopic("bike/zwift_ride/left/power_up", false);
    publishToTopic("bike/zwift_ride/left/on_off", false);
    publishToTopic("bike/zwift_play/right/y", false);
    publishToTopic("bike/zwift_play/right/z", false);
    publishToTopic("bike/zwift_play/right/a", false);
    publishToTopic("bike/zwift_play/right/b", false);
    publishToTopic("bike/zwift_play/right/shoulder", false);
    publishToTopic("bike/zwift_play/right/power", false);
    publishToTopic("bike/zwift_play/right/paddle", 0);
    publishToTopic("bike/zwift_ride/right/z_alt", false);
    publishToTopic("bike/zwift_ride/right/shift_up", false);
    publishToTopic("bike/zwift_ride/right/shift_down", false);
    publishToTopic("bike/zwift_ride/right/power", false);
    publishToTopic("bike/zwift_ride/right/power_up", false);
    publishToTopic("bike/zwift_ride/right/on_off", false);
}

void MQTTPublisher::onConnected() {
    qDebug() << "MQTT Client Connected";
    m_lastPublishedValues.clear();  // Reset stored values
    publishOnlineStatus();
    publishDefaultZwiftControllerStates();
    subscribeToControlTopics();
    
    // Delay discovery config to allow device initialization
    QTimer::singleShot(2000, this, &MQTTPublisher::publishDiscoveryConfig);
}

void MQTTPublisher::subscribeToControlTopics() {
    if(!isConnected()) return;
    
    QString controlTopic = getControlTopic() + "+";
    QString deviceControlTopic = getControlTopic() + "+/+";
    
    qDebug() << "Subscribing to control topics:" << controlTopic << deviceControlTopic;
    
    m_client->subscribe(QMqttTopicFilter(controlTopic), 1);
    m_client->subscribe(QMqttTopicFilter(deviceControlTopic), 1);
}

void MQTTPublisher::onMessageReceived(const QByteArray &message, const QMqttTopicName &topic) {
    QString topicName = topic.name();
    QString controlPrefix = getControlTopic();
    
    if(!topicName.startsWith(controlPrefix)) {
        return;
    }
    
    QString command = topicName.mid(controlPrefix.length());
    QVariant value = QString::fromUtf8(message);
    
    qDebug() << "Received control command:" << command << "value:" << value.toString();
    
    handleControlCommand(command, value);
}

void MQTTPublisher::handleControlCommand(const QString& command, const QVariant& value) {
    if(!m_device) return;
    
    QStringList parts = command.split('/');
    if(parts.isEmpty()) return;
    
    // Handle device-specific commands (e.g., "bike/resistance", "treadmill/speed")
    if(parts.size() == 2) {
        processDeviceCommand(parts[0], parts[1], value);
        return;
    }
    
    // Handle general commands
    QString mainCommand = parts[0];
    
    if(mainCommand == "start") {
        m_device->start();
    } else if(mainCommand == "stop") {
        m_device->stop(false);
    } else if(mainCommand == "pause") {
        m_device->setPaused(value.toBool());
    } else if(mainCommand == "resistance") {
        m_device->changeResistance(value.toInt());
    } else if(mainCommand == "power") {
        m_device->changePower(value.toInt());
    } else if(mainCommand == "fan" || mainCommand == "fan_speed") {
        m_device->changeFanSpeed(value.toInt());
    } else if(mainCommand == "inclination") {
        m_device->changeInclination(value.toDouble(), value.toDouble());
    }
}

void MQTTPublisher::processDeviceCommand(const QString& deviceType, const QString& command, const QVariant& value) {
    if(!m_device) return;
    
    if(deviceType == "bike" && m_device->deviceType() == BIKE) {
        bike* bikeDevice = static_cast<bike*>(m_device.data());
        
        if(command == "resistance") {
            bikeDevice->changeResistance(value.toInt());
        } else if(command == "power") {
            bikeDevice->changePower(value.toInt());
        } else if(command == "cadence") {
            bikeDevice->changeCadence(value.toInt());
        } else if(command == "peloton_resistance") {
            bikeDevice->changeRequestedPelotonResistance(value.toInt());
        } else if(command == "inclination") {
            bikeDevice->changeInclination(value.toDouble(), value.toDouble());
        } else if(command == "gears_up") {
            bikeDevice->gearUp();
        } else if(command == "gears_down") {
            bikeDevice->gearDown();
        }
        
    } else if(deviceType == "treadmill" && m_device->deviceType() == TREADMILL) {
        treadmill* treadDevice = static_cast<treadmill*>(m_device.data());
        
        if(command == "speed") {
            treadDevice->changeSpeed(value.toDouble());
        } else if(command == "inclination") {
            treadDevice->changeInclination(value.toDouble(), value.toDouble());
        } else if(command == "power") {
            treadDevice->changePower(value.toInt());
        }
        
    } else if(deviceType == "rowing" && m_device->deviceType() == ROWING) {
        rower* rowDevice = static_cast<rower*>(m_device.data());
        
        if(command == "resistance") {
            rowDevice->changeResistance(value.toInt());
        } else if(command == "power") {
            rowDevice->changePower(value.toInt());
        } else if(command == "cadence") {
            rowDevice->changeCadence(value.toInt());
        } else if(command == "speed") {
            rowDevice->changeSpeed(value.toDouble());
        }
        
    } else if(deviceType == "elliptical" && m_device->deviceType() == ELLIPTICAL) {
        elliptical* ellipticalDevice = static_cast<elliptical*>(m_device.data());
        
        if(command == "resistance") {
            ellipticalDevice->changeResistance(value.toInt());
        } else if(command == "power") {
            ellipticalDevice->changePower(value.toInt());
        } else if(command == "speed") {
            ellipticalDevice->changeSpeed(value.toDouble());
        } else if(command == "inclination") {
            ellipticalDevice->changeInclination(value.toDouble(), value.toDouble());
        } else if(command == "cadence") {
            ellipticalDevice->changeCadence(value.toInt());
        }
    } else if(command == "fan" || command == "fan_speed") {
        m_device->changeFanSpeed(value.toInt());
    }
}

QJsonObject MQTTPublisher::getDeviceInfo() const {
    QJsonObject device;
    
    if (m_device) {
        device["identifiers"] = QJsonArray{m_device->bluetoothDevice.address().toString()};
        device["name"] = QString("QZ Fitness Device (%1)").arg(m_userNickname);
        device["model"] = m_device->bluetoothDevice.name();
        device["manufacturer"] = "QDomyos-Zwift";
        device["sw_version"] = QCoreApplication::applicationVersion();
    } else {
        device["identifiers"] = QJsonArray{m_userNickname};
        device["name"] = QString("QZ Fitness Device (%1)").arg(m_userNickname);
        device["manufacturer"] = "QDomyos-Zwift";
        device["sw_version"] = QCoreApplication::applicationVersion();
    }
    
    return device;
}

QString MQTTPublisher::getDiscoveryTopic(const QString& component, const QString& objectId) const {
    return QString("homeassistant/%1/qz_%2_%3/config").arg(component, m_userNickname, objectId);
}

void MQTTPublisher::publishSensorDiscovery(const QString& sensorType, const QString& name, const QString& stateTopic, const QString& unit, const QString& deviceClass, const QString& icon) {
    if (!isConnected()) return;
    
    QJsonObject config;
    config["name"] = name;
    config["unique_id"] = QString("qz_%1_%2").arg(m_userNickname, sensorType);
    config["state_topic"] = stateTopic;
    config["device"] = getDeviceInfo();
    
    if (!unit.isEmpty()) config["unit_of_measurement"] = unit;
    if (!deviceClass.isEmpty()) config["device_class"] = deviceClass;
    if (!icon.isEmpty()) config["icon"] = icon;
    
    QString topic = getDiscoveryTopic("sensor", sensorType);
    QJsonDocument doc(config);
    
    m_client->publish(QMqttTopicName(topic), doc.toJson(QJsonDocument::Compact), 1, true);
}

void MQTTPublisher::publishBinarySensorDiscovery(const QString& sensorType, const QString& name, const QString& stateTopic, const QString& deviceClass, const QString& icon) {
    if (!isConnected()) return;
    
    QJsonObject config;
    config["name"] = name;
    config["unique_id"] = QString("qz_%1_%2").arg(m_userNickname, sensorType);
    config["state_topic"] = stateTopic;
    config["device"] = getDeviceInfo();
    config["payload_on"] = "true";
    config["payload_off"] = "false";
    
    if (!deviceClass.isEmpty()) config["device_class"] = deviceClass;
    if (!icon.isEmpty()) config["icon"] = icon;
    
    QString topic = getDiscoveryTopic("binary_sensor", sensorType);
    QJsonDocument doc(config);
    
    m_client->publish(QMqttTopicName(topic), doc.toJson(QJsonDocument::Compact), 1, true);
}

void MQTTPublisher::publishNumberDiscovery(const QString& entityType, const QString& name, const QString& stateTopic, const QString& commandTopic, double min, double max, double step, const QString& unit, const QString& icon) {
    if (!isConnected()) return;
    
    QJsonObject config;
    config["name"] = name;
    config["unique_id"] = QString("qz_%1_%2").arg(m_userNickname, entityType);
    config["state_topic"] = stateTopic;
    config["command_topic"] = commandTopic;
    config["device"] = getDeviceInfo();
    config["min"] = min;
    config["max"] = max;
    config["step"] = step;
    config["mode"] = "box";
    
    if (!unit.isEmpty()) config["unit_of_measurement"] = unit;
    if (!icon.isEmpty()) config["icon"] = icon;
    
    QString topic = getDiscoveryTopic("number", entityType);
    QJsonDocument doc(config);
    
    m_client->publish(QMqttTopicName(topic), doc.toJson(QJsonDocument::Compact), 1, true);
}

void MQTTPublisher::publishSwitchDiscovery(const QString& entityType, const QString& name, const QString& stateTopic, const QString& commandTopic, const QString& icon) {
    if (!isConnected()) return;
    
    QJsonObject config;
    config["name"] = name;
    config["unique_id"] = QString("qz_%1_%2").arg(m_userNickname, entityType);
    config["state_topic"] = stateTopic;
    config["command_topic"] = commandTopic;
    config["device"] = getDeviceInfo();
    config["payload_on"] = "true";
    config["payload_off"] = "false";
    
    if (!icon.isEmpty()) config["icon"] = icon;
    
    QString topic = getDiscoveryTopic("switch", entityType);
    QJsonDocument doc(config);
    
    m_client->publish(QMqttTopicName(topic), doc.toJson(QJsonDocument::Compact), 1, true);
}

void MQTTPublisher::removeDiscoveryConfig() {
    if (!isConnected()) return;
    
    // Remove all discovery configs by publishing empty messages
    QStringList components = {"sensor", "binary_sensor", "number", "switch", "button"};
    QStringList entities = {
        "speed_current", "speed_avg", "distance", "calories", "elapsed_time", "elapsed_total_seconds", "heart_current", "heart_avg",
        "watts_current", "watts_avg", "connected", "paused", "resistance", "cadence", "inclination",
        "power", "fan_speed", "start", "stop", "pause",
        "zwift_click_plus", "zwift_click_minus", "zwift_play_plus", "zwift_play_minus",
        "zwift_play_left_up", "zwift_play_left_down", "zwift_play_left_left", "zwift_play_left_right",
        "zwift_play_left_shoulder", "zwift_play_left_power", "zwift_play_left_paddle",
        "zwift_play_right_y", "zwift_play_right_z", "zwift_play_right_a", "zwift_play_right_b",
        "zwift_play_right_shoulder", "zwift_play_right_power", "zwift_play_right_paddle"
    };
    
    for (const QString& component : components) {
        for (const QString& entity : entities) {
            QString topic = getDiscoveryTopic(component, entity);
            m_client->publish(QMqttTopicName(topic), QByteArray(), 1, true);
        }
    }
}

void MQTTPublisher::publishWorkoutData() {
    if (m_manager) {
        bluetoothdevice *currentDevice = m_manager->device();
        if (currentDevice != m_device.data()) {
            m_device = currentDevice;
        }
    }

    if (!isConnected() || !m_device) return;

    // Device Information
    publishToTopic("device/id", QVariant(m_device->bluetoothDevice.address().toString()));
    publishToTopic("device/name", QVariant(m_device->bluetoothDevice.name()));
    publishToTopic("device/rssi", m_device->bluetoothDevice.rssi());
    publishToTopic("device/type", static_cast<int>(m_device->deviceType()));
    publishToTopic("device/connected", m_device->connected());
    publishToTopic("device/paused", m_device->isPaused());
    publishToTopic("device/fan_speed", m_device->fanSpeed());

    // Time Metrics
    QTime elapsedTime = m_device->elapsedTime();
    publishToTopic("elapsed/seconds", elapsedTime.second());
    publishToTopic("elapsed/minutes", elapsedTime.minute());
    publishToTopic("elapsed/hours", elapsedTime.hour());
    publishToTopic("elapsed/total_seconds", QTime(0, 0).secsTo(elapsedTime));

    QTime lapTime = m_device->lapElapsedTime();
    publishToTopic("lap/elapsed/seconds", lapTime.second());
    publishToTopic("lap/elapsed/minutes", lapTime.minute());
    publishToTopic("lap/elapsed/hours", lapTime.hour());

    // Current Pace
    QTime currentPace = m_device->currentPace();
    publishToTopic("pace/current/seconds", currentPace.second());
    publishToTopic("pace/current/minutes", currentPace.minute());
    publishToTopic("pace/current/hours", currentPace.hour());

    // Average Pace
    QTime avgPace = m_device->averagePace();
    publishToTopic("pace/avg/seconds", avgPace.second());
    publishToTopic("pace/avg/minutes", avgPace.minute());
    publishToTopic("pace/avg/hours", avgPace.hour());

    // Moving Time
    QTime movingTime = m_device->movingTime();
    publishToTopic("moving/seconds", movingTime.second());
    publishToTopic("moving/minutes", movingTime.minute());
    publishToTopic("moving/hours", movingTime.hour());

    // Basic Metrics
    publishToTopic("speed/current", m_device->currentSpeed().value());
    publishToTopic("speed/avg", m_device->currentSpeed().average());
    publishToTopic("speed/lap_avg", m_device->currentSpeed().lapAverage());
    publishToTopic("speed/lap_max", m_device->currentSpeed().lapMax());

    publishToTopic("calories", m_device->calories().value());
    publishToTopic("distance", m_device->odometer());
    publishToTopic("jouls", m_device->jouls().value());
    publishToTopic("elevation", m_device->elevationGain().value());
    publishToTopic("difficult", m_device->difficult());

    // Heart Rate
    metric heart = m_device->currentHeart();
    publishToTopic("heart/current", heart.value());
    publishToTopic("heart/avg", heart.average());
    publishToTopic("heart/lap_avg", heart.lapAverage());
    publishToTopic("heart/max", heart.max());
    publishToTopic("heart/lap_max", heart.lapMax());

    // Power Metrics
    metric watts = m_device->wattsMetric();
    publishToTopic("watts/current", watts.value());
    publishToTopic("watts/avg", watts.average());
    publishToTopic("watts/lap_avg", watts.lapAverage());
    publishToTopic("watts/max", watts.max());
    publishToTopic("watts/lap_max", watts.lapMax());

    // Power per KG
    metric wattsKg = m_device->wattKg();
    publishToTopic("kgwatts/current", wattsKg.value());
    publishToTopic("kgwatts/avg", wattsKg.average());
    publishToTopic("kgwatts/max", wattsKg.max());

    // Location Data
    QGeoCoordinate coord = m_device->currentCordinate();
    if (coord.isValid()) {
        publishToTopic("location/latitude", coord.latitude());
        publishToTopic("location/longitude", coord.longitude());
        publishToTopic("location/altitude", coord.altitude());
    }

    // Device Specific Metrics
    switch (m_device->deviceType()) {
        case BIKE: {
            bike* bikeDevice = static_cast<bike*>(m_device.data());
            publishToTopic("bike/gears", bikeDevice->gears());
            publishToTopic("bike/target_resistance", bikeDevice->lastRequestedResistance().value());
            publishToTopic("bike/target_peloton_resistance", bikeDevice->lastRequestedPelotonResistance().value());
            publishToTopic("bike/target_cadence", bikeDevice->lastRequestedCadence().value());
            publishToTopic("bike/target_power", bikeDevice->lastRequestedPower().value());

            metric powerZone = m_device->currentPowerZone();
            publishToTopic("bike/power_zone", powerZone.value());
            publishToTopic("bike/power_zone_lapavg", powerZone.lapAverage());
            publishToTopic("bike/power_zone_lapmax", powerZone.lapMax());

            metric pelotonRes = bikeDevice->pelotonResistance();
            publishToTopic("bike/peloton_resistance/current", pelotonRes.value());
            publishToTopic("bike/peloton_resistance/avg", pelotonRes.average());
            publishToTopic("bike/peloton_resistance/lap_avg", pelotonRes.lapAverage());
            publishToTopic("bike/peloton_resistance/lap_max", pelotonRes.lapMax());

            metric cadence = m_device->currentCadence();
            publishToTopic("bike/cadence/current", cadence.value());
            publishToTopic("bike/cadence/avg", cadence.average());
            publishToTopic("bike/cadence/lap_avg", cadence.lapAverage());
            publishToTopic("bike/cadence/lap_max", cadence.lapMax());

            metric resistance = bikeDevice->currentResistance();
            publishToTopic("bike/resistance/current", resistance.value());
            publishToTopic("bike/resistance/avg", resistance.average());
            publishToTopic("bike/resistance/lap_avg", resistance.lapAverage());
            publishToTopic("bike/resistance/lap_max", resistance.lapMax());

            publishToTopic("bike/cranks", bikeDevice->currentCrankRevolutions());
            publishToTopic("bike/cranktime", bikeDevice->lastCrankEventTime());
            break;
        }
        case TREADMILL: {
            treadmill* treadDevice = static_cast<treadmill*>(m_device.data());
            publishToTopic("treadmill/target_speed", treadDevice->lastRequestedSpeed().value());
            publishToTopic("treadmill/target_inclination", treadDevice->lastRequestedInclination().value());

            metric inclination = treadDevice->currentInclination();
            publishToTopic("treadmill/inclination/current", inclination.value());
            publishToTopic("treadmill/inclination/avg", inclination.average());
            publishToTopic("treadmill/inclination/lap_avg", inclination.lapAverage());
            publishToTopic("treadmill/inclination/lap_max", inclination.lapMax());

            metric cadence = m_device->currentCadence();
            publishToTopic("treadmill/cadence/current", cadence.value());
            publishToTopic("treadmill/cadence/avg", cadence.average());
            publishToTopic("treadmill/cadence/lap_avg", cadence.lapAverage());
            publishToTopic("treadmill/cadence/lap_max", cadence.lapMax());

            publishToTopic("treadmill/stride_length", treadDevice->currentStrideLength().value());
            publishToTopic("treadmill/ground_contact", treadDevice->currentGroundContact().value());
            publishToTopic("treadmill/vertical_oscillation", treadDevice->currentVerticalOscillation().value());
            break;
        }
        case ROWING: {
            rower* rowDevice = static_cast<rower*>(m_device.data());
            metric cadence = m_device->currentCadence();
            publishToTopic("rowing/cadence/current", cadence.value());
            publishToTopic("rowing/cadence/avg", cadence.average());
            publishToTopic("rowing/cadence/lap_avg", cadence.lapAverage());
            publishToTopic("rowing/cadence/lap_max", cadence.lapMax());

            metric resistance = rowDevice->currentResistance();
            publishToTopic("rowing/resistance/current", resistance.value());
            publishToTopic("rowing/resistance/avg", resistance.average());

            publishToTopic("rowing/strokes_count", rowDevice->currentStrokesCount().value());
            publishToTopic("rowing/strokes_length", rowDevice->currentStrokesLength().value());
            publishToTopic("rowing/cranks", rowDevice->currentCrankRevolutions());
            publishToTopic("rowing/cranktime", rowDevice->lastCrankEventTime());
            break;
        }
        default:
            break;
    }
}

void MQTTPublisher::publishDiscoveryConfig() {
    if (!isConnected()) return;
    
    qDebug() << "Publishing Home Assistant discovery configuration";
    
    QString baseTopic = getBaseTopic();
    QString controlTopic = getControlTopic();
    
    // Common sensors for all devices
    publishSensorDiscovery("speed_current", "Speed", baseTopic + "speed/current", "km/h", "", "mdi:speedometer");
    publishSensorDiscovery("speed_avg", "Average Speed", baseTopic + "speed/avg", "km/h", "", "mdi:speedometer");
    publishSensorDiscovery("distance", "Distance", baseTopic + "distance", "km", "distance", "mdi:map-marker-distance");
    publishSensorDiscovery("calories", "Calories", baseTopic + "calories", "kcal", "", "mdi:fire");
    publishSensorDiscovery("elapsed_time", "Elapsed Time", baseTopic + "elapsed/minutes", "min", "duration", "mdi:timer");
    publishSensorDiscovery("elapsed_total_seconds", "Elapsed Total Seconds", baseTopic + "elapsed/total_seconds", "s", "duration", "mdi:timer");
    publishSensorDiscovery("heart_current", "Heart Rate", baseTopic + "heart/current", "bpm", "", "mdi:heart-pulse");
    publishSensorDiscovery("heart_avg", "Average Heart Rate", baseTopic + "heart/avg", "bpm", "", "mdi:heart-pulse");
    publishSensorDiscovery("watts_current", "Power", baseTopic + "watts/current", "W", "power", "mdi:flash");
    publishSensorDiscovery("watts_avg", "Average Power", baseTopic + "watts/avg", "W", "power", "mdi:flash");
    publishSensorDiscovery("elevation", "Elevation Gain", baseTopic + "elevation", "m", "", "mdi:elevation-rise");
    
    // Binary sensors
    QString statusBaseTopic = QString("QZ/%1/").arg(m_userNickname);
    publishBinarySensorDiscovery("connected", "Device Connected", statusBaseTopic + "device/connected", "connectivity", "mdi:bluetooth");
    publishBinarySensorDiscovery("paused", "Workout Paused", statusBaseTopic + "device/paused", "", "mdi:pause");
    
    // Control entities based on device type
    if (m_device) {
        switch (m_device->deviceType()) {
            case BIKE: {
                // Bike-specific sensors
                publishSensorDiscovery("bike_resistance", "Resistance", baseTopic + "bike/resistance/current", "", "", "mdi:tune");
                publishSensorDiscovery("bike_cadence", "Cadence", baseTopic + "bike/cadence/current", "rpm", "", "mdi:rotate-right");
                publishSensorDiscovery("bike_gears", "Gears", baseTopic + "bike/gears", "", "", "mdi:cog");
                publishSensorDiscovery("bike_power_zone", "Power Zone", baseTopic + "bike/power_zone", "", "", "mdi:target");
                publishBinarySensorDiscovery("zwift_click_plus", "Zwift Click Plus", baseTopic + "bike/zwift_click/plus", "", "mdi:plus-circle");
                publishBinarySensorDiscovery("zwift_click_minus", "Zwift Click Minus", baseTopic + "bike/zwift_click/minus", "", "mdi:minus-circle");
                publishBinarySensorDiscovery("zwift_play_plus", "Zwift Play Plus", baseTopic + "bike/zwift_play/plus", "", "mdi:plus-circle");
                publishBinarySensorDiscovery("zwift_play_minus", "Zwift Play Minus", baseTopic + "bike/zwift_play/minus", "", "mdi:minus-circle");
                publishBinarySensorDiscovery("zwift_play_left_up", "Zwift Play Left Up", baseTopic + "bike/zwift_play/left/up", "", "mdi:arrow-up-bold-circle");
                publishBinarySensorDiscovery("zwift_play_left_down", "Zwift Play Left Down", baseTopic + "bike/zwift_play/left/down", "", "mdi:arrow-down-bold-circle");
                publishBinarySensorDiscovery("zwift_play_left_left", "Zwift Play Left Left", baseTopic + "bike/zwift_play/left/left", "", "mdi:arrow-left-bold-circle");
                publishBinarySensorDiscovery("zwift_play_left_right", "Zwift Play Left Right", baseTopic + "bike/zwift_play/left/right", "", "mdi:arrow-right-bold-circle");
                publishBinarySensorDiscovery("zwift_play_left_shoulder", "Zwift Play Left Shoulder", baseTopic + "bike/zwift_play/left/shoulder", "", "mdi:button-pointer");
                publishBinarySensorDiscovery("zwift_play_left_power", "Zwift Play Left Power", baseTopic + "bike/zwift_play/left/power", "", "mdi:power");
                publishSensorDiscovery("zwift_play_left_paddle", "Zwift Play Left Paddle", baseTopic + "bike/zwift_play/left/paddle", "", "", "mdi:gamepad-round");
                publishBinarySensorDiscovery("zwift_ride_left_shift_up", "Zwift Ride Left Shift Up", baseTopic + "bike/zwift_ride/left/shift_up", "", "mdi:arrow-up-bold");
                publishBinarySensorDiscovery("zwift_ride_left_shift_down", "Zwift Ride Left Shift Down", baseTopic + "bike/zwift_ride/left/shift_down", "", "mdi:arrow-down-bold");
                publishBinarySensorDiscovery("zwift_ride_left_power_up", "Zwift Ride Left Power Up", baseTopic + "bike/zwift_ride/left/power_up", "", "mdi:flash");
                publishBinarySensorDiscovery("zwift_ride_left_on_off", "Zwift Ride Left On/Off", baseTopic + "bike/zwift_ride/left/on_off", "", "mdi:power");
                publishBinarySensorDiscovery("zwift_play_right_y", "Zwift Play Right Y", baseTopic + "bike/zwift_play/right/y", "", "mdi:alpha-y-circle");
                publishBinarySensorDiscovery("zwift_play_right_z", "Zwift Play Right Z", baseTopic + "bike/zwift_play/right/z", "", "mdi:alpha-z-circle");
                publishBinarySensorDiscovery("zwift_play_right_a", "Zwift Play Right A", baseTopic + "bike/zwift_play/right/a", "", "mdi:alpha-a-circle");
                publishBinarySensorDiscovery("zwift_play_right_b", "Zwift Play Right B", baseTopic + "bike/zwift_play/right/b", "", "mdi:alpha-b-circle");
                publishBinarySensorDiscovery("zwift_play_right_shoulder", "Zwift Play Right Shoulder", baseTopic + "bike/zwift_play/right/shoulder", "", "mdi:button-pointer");
                publishBinarySensorDiscovery("zwift_play_right_power", "Zwift Play Right Power", baseTopic + "bike/zwift_play/right/power", "", "mdi:power");
                publishSensorDiscovery("zwift_play_right_paddle", "Zwift Play Right Paddle", baseTopic + "bike/zwift_play/right/paddle", "", "", "mdi:gamepad-round");
                publishBinarySensorDiscovery("zwift_ride_right_z_alt", "Zwift Ride Right Z Alt", baseTopic + "bike/zwift_ride/right/z_alt", "", "mdi:alpha-z-circle-outline");
                publishBinarySensorDiscovery("zwift_ride_right_shift_up", "Zwift Ride Right Shift Up", baseTopic + "bike/zwift_ride/right/shift_up", "", "mdi:arrow-up-bold");
                publishBinarySensorDiscovery("zwift_ride_right_shift_down", "Zwift Ride Right Shift Down", baseTopic + "bike/zwift_ride/right/shift_down", "", "mdi:arrow-down-bold");
                publishBinarySensorDiscovery("zwift_ride_right_power", "Zwift Ride Right Power", baseTopic + "bike/zwift_ride/right/power", "", "mdi:power");
                publishBinarySensorDiscovery("zwift_ride_right_power_up", "Zwift Ride Right Power Up", baseTopic + "bike/zwift_ride/right/power_up", "", "mdi:flash");
                publishBinarySensorDiscovery("zwift_ride_right_on_off", "Zwift Ride Right On/Off", baseTopic + "bike/zwift_ride/right/on_off", "", "mdi:power-standby");
                
                // Bike controls
                publishNumberDiscovery("bike_resistance", "Bike Resistance", baseTopic + "bike/resistance/current", controlTopic + "bike/resistance", 0, 32, 1, "", "mdi:tune");
                publishNumberDiscovery("bike_power", "Target Power", baseTopic + "bike/target_power", controlTopic + "bike/power", 0, 1000, 5, "W", "mdi:flash");
                publishNumberDiscovery("bike_cadence", "Target Cadence", baseTopic + "bike/target_cadence", controlTopic + "bike/cadence", 0, 200, 1, "rpm", "mdi:rotate-right");
                
                // Bike buttons
                QJsonObject gearUpConfig;
                gearUpConfig["name"] = "Gear Up";
                gearUpConfig["unique_id"] = QString("qz_%1_gear_up").arg(m_userNickname);
                gearUpConfig["command_topic"] = controlTopic + "bike/gears_up";
                gearUpConfig["device"] = getDeviceInfo();
                gearUpConfig["icon"] = "mdi:plus";
                QString gearUpTopic = getDiscoveryTopic("button", "gear_up");
                m_client->publish(QMqttTopicName(gearUpTopic), QJsonDocument(gearUpConfig).toJson(QJsonDocument::Compact), 1, true);
                
                QJsonObject gearDownConfig;
                gearDownConfig["name"] = "Gear Down";
                gearDownConfig["unique_id"] = QString("qz_%1_gear_down").arg(m_userNickname);
                gearDownConfig["command_topic"] = controlTopic + "bike/gears_down";
                gearDownConfig["device"] = getDeviceInfo();
                gearDownConfig["icon"] = "mdi:minus";
                QString gearDownTopic = getDiscoveryTopic("button", "gear_down");
                m_client->publish(QMqttTopicName(gearDownTopic), QJsonDocument(gearDownConfig).toJson(QJsonDocument::Compact), 1, true);
                
                break;
            }
            case TREADMILL: {
                // Treadmill-specific sensors
                publishSensorDiscovery("treadmill_inclination", "Inclination", baseTopic + "treadmill/inclination/current", "%", "", "mdi:angle-acute");
                publishSensorDiscovery("treadmill_cadence", "Cadence", baseTopic + "treadmill/cadence/current", "spm", "", "mdi:run");
                publishSensorDiscovery("stride_length", "Stride Length", baseTopic + "treadmill/stride_length", "cm", "", "mdi:ruler");
                
                // Treadmill controls
                publishNumberDiscovery("treadmill_speed", "Treadmill Speed", baseTopic + "treadmill/target_speed", controlTopic + "treadmill/speed", 0, 25, 0.1, "km/h", "mdi:speedometer");
                publishNumberDiscovery("treadmill_inclination", "Treadmill Inclination", baseTopic + "treadmill/inclination/current", controlTopic + "treadmill/inclination", -10, 40, 0.5, "%", "mdi:angle-acute");
                
                break;
            }
            case ROWING: {
                // Rowing-specific sensors
                publishSensorDiscovery("rowing_resistance", "Resistance", baseTopic + "rowing/resistance/current", "", "", "mdi:tune");
                publishSensorDiscovery("rowing_cadence", "Stroke Rate", baseTopic + "rowing/cadence/current", "spm", "", "mdi:rowing");
                publishSensorDiscovery("strokes_count", "Strokes Count", baseTopic + "rowing/strokes_count", "", "", "mdi:counter");
                
                // Rowing controls
                publishNumberDiscovery("rowing_resistance", "Rowing Resistance", baseTopic + "rowing/resistance/current", controlTopic + "rowing/resistance", 0, 32, 1, "", "mdi:tune");
                publishNumberDiscovery("rowing_power", "Target Power", baseTopic + "rowing/target_power", controlTopic + "rowing/power", 0, 1000, 5, "W", "mdi:flash");
                
                break;
            }
            case ELLIPTICAL: {
                // Elliptical-specific sensors  
                publishSensorDiscovery("elliptical_resistance", "Resistance", baseTopic + "elliptical/resistance/current", "", "", "mdi:tune");
                publishSensorDiscovery("elliptical_cadence", "Cadence", baseTopic + "elliptical/cadence/current", "rpm", "", "mdi:rotate-right");
                
                // Elliptical controls
                publishNumberDiscovery("elliptical_resistance", "Elliptical Resistance", baseTopic + "elliptical/resistance/current", controlTopic + "elliptical/resistance", 0, 32, 1, "", "mdi:tune");
                publishNumberDiscovery("elliptical_speed", "Elliptical Speed", baseTopic + "elliptical/target_speed", controlTopic + "elliptical/speed", 0, 25, 0.1, "km/h", "mdi:speedometer");
                
                break;
            }
            default:
                break;
        }
    }
    
    // Common control entities
    publishNumberDiscovery("fan_speed", "Fan Speed", statusBaseTopic + "device/fan_speed", controlTopic + "fan_speed", 0, 100, 1, "%", "mdi:fan");
    
    // Control buttons
    QJsonObject startConfig;
    startConfig["name"] = "Start Workout";
    startConfig["unique_id"] = QString("qz_%1_start").arg(m_userNickname);
    startConfig["command_topic"] = controlTopic + "start";
    startConfig["device"] = getDeviceInfo();
    startConfig["icon"] = "mdi:play";
    QString startTopic = getDiscoveryTopic("button", "start");
    m_client->publish(QMqttTopicName(startTopic), QJsonDocument(startConfig).toJson(QJsonDocument::Compact), 1, true);
    
    QJsonObject stopConfig;
    stopConfig["name"] = "Stop Workout";
    stopConfig["unique_id"] = QString("qz_%1_stop").arg(m_userNickname);
    stopConfig["command_topic"] = controlTopic + "stop";
    stopConfig["device"] = getDeviceInfo();
    stopConfig["icon"] = "mdi:stop";
    QString stopTopic = getDiscoveryTopic("button", "stop");
    m_client->publish(QMqttTopicName(stopTopic), QJsonDocument(stopConfig).toJson(QJsonDocument::Compact), 1, true);
    
    // Pause switch
    publishSwitchDiscovery("pause", "Pause Workout", statusBaseTopic + "device/paused", controlTopic + "pause", "mdi:pause");
    
    qDebug() << "Home Assistant discovery configuration published";
}
