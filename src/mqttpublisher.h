#ifndef MQTTPUBLISHER_H
#define MQTTPUBLISHER_H

#include <QObject>
#include <QTimer>
#include "mqtt/qmqttclient.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QSettings>
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

private slots:
    void onConnected();
    void onDisconnected();
    void onError(QMqttClient::ClientError error);
    void publishWorkoutData();

private:
    void setupMQTTClient();
    void publishToTopic(const QString& topic, const QVariant& value);
    void connectToHost();
    QString getUserNickname() const;
    QString getBaseTopic() const;

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
