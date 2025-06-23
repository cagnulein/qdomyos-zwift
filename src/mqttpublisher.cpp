#include "mqttpublisher.h"
#include "qzsettings.h"
#include "homeform.h"
#include "devices/elliptical.h"
#include <QDebug>

MQTTPublisher::MQTTPublisher(const QString& host, quint16 port, QString username, QString password, bluetooth* manager, QObject *parent)
    : QObject(parent)
    , m_host(host)
    , m_port(port)
    , m_device(nullptr)
{
    m_client = new QMqttClient();
    m_timer = new QTimer();
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

void MQTTPublisher::onConnected() {
    qDebug() << "MQTT Client Connected";
    m_lastPublishedValues.clear();  // Reset stored values
    publishOnlineStatus();
    subscribeToControlTopics();
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
    } else if(mainCommand == "fan") {
        m_device->changeFanSpeed(value.toInt());
    } else if(mainCommand == "inclination") {
        m_device->changeInclination(value.toDouble(), value.toDouble());
    }
}

void MQTTPublisher::processDeviceCommand(const QString& deviceType, const QString& command, const QVariant& value) {
    if(!m_device) return;
    
    if(deviceType == "bike" && m_device->deviceType() == bluetoothdevice::BIKE) {
        bike* bikeDevice = static_cast<bike*>(m_device);
        
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
        
    } else if(deviceType == "treadmill" && m_device->deviceType() == bluetoothdevice::TREADMILL) {
        treadmill* treadDevice = static_cast<treadmill*>(m_device);
        
        if(command == "speed") {
            treadDevice->changeSpeed(value.toDouble());
        } else if(command == "inclination") {
            treadDevice->changeInclination(value.toDouble(), value.toDouble());
        } else if(command == "power") {
            treadDevice->changePower(value.toInt());
        }
        
    } else if(deviceType == "rowing" && m_device->deviceType() == bluetoothdevice::ROWING) {
        rower* rowDevice = static_cast<rower*>(m_device);
        
        if(command == "resistance") {
            rowDevice->changeResistance(value.toInt());
        } else if(command == "power") {
            rowDevice->changePower(value.toInt());
        } else if(command == "cadence") {
            rowDevice->changeCadence(value.toInt());
        } else if(command == "speed") {
            rowDevice->changeSpeed(value.toDouble());
        }
        
    } else if(deviceType == "elliptical" && m_device->deviceType() == bluetoothdevice::ELLIPTICAL) {
        elliptical* ellipticalDevice = static_cast<elliptical*>(m_device);
        
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
    } else if(command == "fan") {
        m_device->changeFanSpeed(value.toInt());
    }
}

void MQTTPublisher::publishWorkoutData() {

    if(!m_device && m_manager && m_manager->device()) {
        m_device = m_manager->device();
    }

    if (!isConnected() || !m_device) return;

    // Device Information
    publishToTopic("device/id", QVariant(m_device->bluetoothDevice.address().toString()));
    publishToTopic("device/name", QVariant(m_device->bluetoothDevice.name()));
    publishToTopic("device/rssi", m_device->bluetoothDevice.rssi());
    publishToTopic("device/type", static_cast<int>(m_device->deviceType()));
    publishToTopic("device/connected", m_device->connected());
    publishToTopic("device/paused", m_device->isPaused());

    // Time Metrics
    QTime elapsedTime = m_device->elapsedTime();
    publishToTopic("elapsed/seconds", elapsedTime.second());
    publishToTopic("elapsed/minutes", elapsedTime.minute());
    publishToTopic("elapsed/hours", elapsedTime.hour());

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
        case bluetoothdevice::BIKE: {
            bike* bikeDevice = static_cast<bike*>(m_device);
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
        case bluetoothdevice::TREADMILL: {
            treadmill* treadDevice = static_cast<treadmill*>(m_device);
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
        case bluetoothdevice::ROWING: {
            rower* rowDevice = static_cast<rower*>(m_device);
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
