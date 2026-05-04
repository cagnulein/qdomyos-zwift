/******************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtMqtt module.
**
** $QT_BEGIN_LICENSE:GPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
******************************************************************************/

#ifndef QTMQTTCLIENT_H
#define QTMQTTCLIENT_H

#include "qmqttglobal.h"
#include "qmqttauthenticationproperties.h"
#include "qmqttconnectionproperties.h"
#include "qmqttpublishproperties.h"
#include "qmqttsubscription.h"
#include "qmqttsubscriptionproperties.h"
#include "qmqtttopicfilter.h"

#include <QtCore/QIODevice>
#include <QtCore/QObject>
#include <QtCore/QSharedPointer>
#include <QtNetwork/QTcpSocket>
#ifndef QT_NO_SSL
#include <QtNetwork/QSslConfiguration>
#endif

QT_BEGIN_NAMESPACE

class QMqttClientPrivate;

class Q_MQTT_EXPORT QMqttClient : public QObject
{
public:
    enum TransportType {
        IODevice = 0,
        AbstractSocket,
        SecureSocket
    };

    enum ClientState {
        Disconnected = 0,
        Connecting,
        Connected
    };
    Q_ENUM(ClientState)

    enum ClientError {
        // Protocol states
        NoError                = 0,
        InvalidProtocolVersion = 1,
        IdRejected             = 2,
        ServerUnavailable      = 3,
        BadUsernameOrPassword  = 4,
        NotAuthorized          = 5,
        // Qt states
        TransportInvalid       = 256,
        ProtocolViolation,
        UnknownError,
        Mqtt5SpecificError
    };
    Q_ENUM(ClientError)

    enum ProtocolVersion {
        MQTT_3_1 = 3,
        MQTT_3_1_1 = 4,
        MQTT_5_0 = 5
    };
    Q_ENUM(ProtocolVersion)

private:
    Q_OBJECT
    Q_ENUMS(ClientState)
    Q_ENUMS(ClientError)
    Q_ENUMS(ProtocolVersion)
    Q_PROPERTY(QString clientId READ clientId WRITE setClientId NOTIFY clientIdChanged)
    Q_PROPERTY(QString hostname READ hostname WRITE setHostname NOTIFY hostnameChanged)
    Q_PROPERTY(quint16 port READ port WRITE setPort NOTIFY portChanged)
    Q_PROPERTY(quint16 keepAlive READ keepAlive WRITE setKeepAlive NOTIFY keepAliveChanged)
    Q_PROPERTY(ProtocolVersion protocolVersion READ protocolVersion WRITE setProtocolVersion NOTIFY protocolVersionChanged)
    Q_PROPERTY(ClientState state READ state WRITE setState NOTIFY stateChanged)
    Q_PROPERTY(ClientError error READ error WRITE setError NOTIFY errorChanged)
    Q_PROPERTY(QString username READ username WRITE setUsername NOTIFY usernameChanged)
    Q_PROPERTY(QString password READ password WRITE setPassword NOTIFY passwordChanged)
    Q_PROPERTY(bool cleanSession READ cleanSession WRITE setCleanSession NOTIFY cleanSessionChanged)
    Q_PROPERTY(QString willTopic READ willTopic WRITE setWillTopic NOTIFY willTopicChanged)
    Q_PROPERTY(QByteArray willMessage READ willMessage WRITE setWillMessage NOTIFY willMessageChanged)
    Q_PROPERTY(quint8 willQoS READ willQoS WRITE setWillQoS NOTIFY willQoSChanged)
    Q_PROPERTY(bool willRetain READ willRetain WRITE setWillRetain NOTIFY willRetainChanged)
    Q_PROPERTY(bool autoKeepAlive READ autoKeepAlive WRITE setAutoKeepAlive NOTIFY autoKeepAliveChanged)
public:
    explicit QMqttClient(QObject *parent = nullptr);

    void setTransport(QIODevice *device, TransportType transport);
    QIODevice *transport() const;

    QMqttSubscription *subscribe(const QMqttTopicFilter &topic, quint8 qos = 0);
    QMqttSubscription *subscribe(const QMqttTopicFilter &topic,
                                 const QMqttSubscriptionProperties &properties, quint8 qos = 0);
    void unsubscribe(const QMqttTopicFilter &topic);
    void unsubscribe(const QMqttTopicFilter &topic, const QMqttUnsubscriptionProperties &properties);

    Q_INVOKABLE qint32 publish(const QMqttTopicName &topic, const QByteArray &message = QByteArray(),
                 quint8 qos = 0, bool retain = false);
    Q_INVOKABLE qint32 publish(const QMqttTopicName &topic, const QMqttPublishProperties &properties,
                               const QByteArray &message = QByteArray(),
                               quint8 qos = 0,
                               bool retain = false);

    bool requestPing();

    QString hostname() const;
    quint16 port() const;
    QString clientId() const;
    quint16 keepAlive() const;
    ProtocolVersion protocolVersion() const;

    Q_INVOKABLE void connectToHost();
#ifndef QT_NO_SSL
#if QT_DEPRECATED_SINCE(5, 14)
    QT_DEPRECATED Q_INVOKABLE void connectToHostEncrypted(const QString &sslPeerName = QString()); /// ### Qt 6: remove
#endif
    void connectToHostEncrypted(const QSslConfiguration &conf);
#endif
    Q_INVOKABLE void disconnectFromHost();

    ClientState state() const;
    ClientError error() const;

    QString username() const;
    QString password() const;
    bool cleanSession() const;

    QString willTopic() const;
    quint8 willQoS() const;
    QByteArray willMessage() const;
    bool willRetain() const;
    bool autoKeepAlive() const;

    void setConnectionProperties(const QMqttConnectionProperties &prop);
    QMqttConnectionProperties connectionProperties() const;

    void setLastWillProperties(const QMqttLastWillProperties &prop);
    QMqttLastWillProperties lastWillProperties() const;

    QMqttServerConnectionProperties serverConnectionProperties() const;

    void authenticate(const QMqttAuthenticationProperties &prop);
Q_SIGNALS:
    void connected();
    void disconnected();
    void messageReceived(const QByteArray &message, const QMqttTopicName &topic = QMqttTopicName());
    void messageStatusChanged(qint32 id, QMqtt::MessageStatus s, const QMqttMessageStatusProperties &properties);
    void messageSent(qint32 id);
    void pingResponseReceived();
    void brokerSessionRestored();

    void hostnameChanged(QString hostname);
    void portChanged(quint16 port);
    void clientIdChanged(QString clientId);
    void keepAliveChanged(quint16 keepAlive);
    void protocolVersionChanged(ProtocolVersion protocolVersion);
    void stateChanged(ClientState state);
    void errorChanged(ClientError error);
    void usernameChanged(QString username);
    void passwordChanged(QString password);
    void cleanSessionChanged(bool cleanSession);

    void willTopicChanged(QString willTopic);
    void willQoSChanged(quint8 willQoS);
    void willMessageChanged(QByteArray willMessage);
    void willRetainChanged(bool willRetain);
    void autoKeepAliveChanged(bool autoKeepAlive);

    void authenticationRequested(const QMqttAuthenticationProperties &p);
    void authenticationFinished(const QMqttAuthenticationProperties &p);
public Q_SLOTS:
    void setHostname(const QString &hostname);
    void setPort(quint16 port);
    void setClientId(const QString &clientId);
    void setKeepAlive(quint16 keepAlive);
    void setProtocolVersion(ProtocolVersion protocolVersion);
    void setState(ClientState state);
    void setError(ClientError error);
    void setUsername(const QString &username);
    void setPassword(const QString &password);
    void setCleanSession(bool cleanSession);

    void setWillTopic(const QString &willTopic);
    void setWillQoS(quint8 willQoS);
    void setWillMessage(const QByteArray &willMessage);
    void setWillRetain(bool willRetain);
    void setAutoKeepAlive(bool autoKeepAlive);

private:
    void connectToHost(bool encrypted, const QString &sslPeerName);
    Q_DISABLE_COPY(QMqttClient)
    Q_DECLARE_PRIVATE(QMqttClient)
};

Q_DECLARE_TYPEINFO(QMqttClient::TransportType, Q_PRIMITIVE_TYPE);
Q_DECLARE_TYPEINFO(QMqttClient::ClientState, Q_PRIMITIVE_TYPE);
Q_DECLARE_TYPEINFO(QMqttClient::ClientError, Q_PRIMITIVE_TYPE);

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QMqttClient::TransportType)
Q_DECLARE_METATYPE(QMqttClient::ClientState)
Q_DECLARE_METATYPE(QMqttClient::ClientError)

#endif // QTMQTTCLIENT_H
