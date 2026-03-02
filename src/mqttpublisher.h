#ifndef MQTTPUBLISHER_H
#define MQTTPUBLISHER_H

#include <QObject>
#include <QTimer>
#include "mqtt/qmqttclient.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QSettings>
#include <QHash>
#include <QVariant>
#include "bluetoothdevice.h"
#include "devices/bike.h"
#include "devices/treadmill.h"
#include "devices/rower.h"
#include "homeform.h"
#include "bluetooth.h"

class MQTTPublisher : public QObject {
    Q_OBJECT

public:
    explicit MQTTPublisher(const QString& host = "localhost", quint16 port = 1883, QString username = "", QString password = "", bluetooth* manager = nullptr, QObject *parent = nullptr);
    ~MQTTPublisher();

    void start();
    void stop();
    void setHost(const QString& host);
    void setPort(quint16 port);
    bool isConnected() const;
    void setDevice(bluetoothdevice* device);
    void subscribeToControlTopics();
    void publishDiscoveryConfig();

private slots:
    void onConnected();
    void onDisconnected();
    void onError(QMqttClient::ClientError error);
    void publishWorkoutData();
    void onMessageReceived(const QByteArray &message, const QMqttTopicName &topic);

private:
    const QString STATUS_TOPIC = "status";
    const QString STATUS_ONLINE = "online";
    const QString STATUS_OFFLINE = "offline";
    void setupLastWillMessage();
    void publishOnlineStatus();
    void setupMQTTClient();
    void publishToTopic(const QString& topic, const QVariant& value);
    void connectToHost();
    QString getUserNickname() const;
    QString getBaseTopic() const;
    QString getStatusTopic() const;

    // Hash map to store last published values
    QHash<QString, QVariant> m_lastPublishedValues;

    // Helper method to check if value has changed
    bool hasValueChanged(const QString& topic, const QVariant& newValue);

    // Helper method to store last published value
    void updateLastPublishedValue(const QString& topic, const QVariant& value);
    
    // Control command handlers
    void handleControlCommand(const QString& command, const QVariant& value);
    void processDeviceCommand(const QString& deviceType, const QString& command, const QVariant& value);
    QString getControlTopic() const;
    
    // Home Assistant Discovery helpers
    void publishSensorDiscovery(const QString& sensorType, const QString& name, const QString& stateTopic, const QString& unit = "", const QString& deviceClass = "", const QString& icon = "");
    void publishBinarySensorDiscovery(const QString& sensorType, const QString& name, const QString& stateTopic, const QString& deviceClass = "", const QString& icon = "");
    void publishNumberDiscovery(const QString& entityType, const QString& name, const QString& stateTopic, const QString& commandTopic, double min = 0, double max = 100, double step = 1, const QString& unit = "", const QString& icon = "");
    void publishSwitchDiscovery(const QString& entityType, const QString& name, const QString& stateTopic, const QString& commandTopic, const QString& icon = "");
    QJsonObject getDeviceInfo() const;
    QString getDiscoveryTopic(const QString& component, const QString& objectId) const;
    void removeDiscoveryConfig();

    QMqttClient* m_client;
    QTimer* m_timer;
    QString m_host;
    quint16 m_port;
    QString m_username;
    QString m_password;
    QString m_userNickname;
    bluetoothdevice* m_device;
    bluetooth* m_manager;
};

#endif // MQTTPUBLISHER_H
