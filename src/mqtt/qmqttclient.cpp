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

#include "qmqttclient.h"
#include "qmqttclient_p.h"

#include <QtCore/QLoggingCategory>
#include <QtCore/QUuid>
#include <QtCore/QtEndian>

QT_BEGIN_NAMESPACE

Q_LOGGING_CATEGORY(lcMqttClient, "qt.mqtt.client")

/*!
    \class QMqttClient

    \inmodule QtMqtt
    \brief The QMqttClient class represents the central access communicating
    with an MQTT broker.

    An MQTT client is a program or device that uses MQTT to create a network
    connection to an MQTT server, also called a \e broker. The connection
    request must contain a unique client identifier. Optionally, it can contain
    a Will Topic, Will Message, user name, and password.

    Once a connection is created, a client can send messages that other clients
    might be interested in receiving, subscribe to request notifications on
    topics, unsubscribe to remove a request for notifications, and disconnect
    from the broker.
*/

/*!
    \property QMqttClient::clientId
    \brief This property holds the client's identifier value.

    Each client needs to have a unique ID to be able to connect to an MQTT
    broker. If no client ID is specified by the user, one will be generated
    automatically when a connection is established.
*/

/*!
    \property QMqttClient::hostname
    \brief This property holds the hostname of the MQTT broker to connect to.

    If no transport is specified via setTransport(), the client will instantiate
    a socket connection to the specified hostname itself.
*/

/*!
    \property QMqttClient::port
    \brief This property holds the port to connect to the MQTT broker.

    If no transport is specified via setTransport(), the client will instantiate
    a socket connection to a host with this port number.
*/

/*!
    \property QMqttClient::keepAlive
    \brief This property holds the interval at which regular ping messages are
    sent to the broker.

    Once a connection to a broker is established, the client needs to send
    frequent updates to propagate it can still be reached. The interval between
    those updates is specified by this property.

    The interval is specified in seconds.

    If the broker does not respond within a grace period the connection will be
    closed.

    \sa autoKeepAlive(), requestPing(), pingResponseReceived()
*/

/*!
    \property QMqttClient::protocolVersion
    \brief This property holds the MQTT standard version to use for connections.

    Specifies the version of the standard the client uses for connecting to a
    broker. Valid values are:

    \list
        \li 3: MQTT standard version 3.1.
        \li 4: MQTT standard version 3.1.1, often referred to MQTT 4.
    \endlist
*/

/*!
    \property QMqttClient::state
    \brief This property holds the current state of the client.
*/

/*!
    \property QMqttClient::error
    \brief Specifies the current error of the client.
*/

/*!
    \property QMqttClient::username
    \brief This property holds the user name for connecting to a broker.
*/

/*!
    \property QMqttClient::password
    \brief This property holds the password for connecting to a broker.
*/

/*!
    \property QMqttClient::cleanSession
    \brief This property holds the state after connecting to a broker.
*/

/*!
    \property QMqttClient::willTopic
    \brief This property holds the Will Topic.
*/

/*!
    \property QMqttClient::willMessage
    \brief This property holds the payload of a Will Message.
*/

/*!
    \property QMqttClient::willQoS
    \brief This property holds the level of QoS for sending and storing the
    Will Message.
*/

/*!
    \property QMqttClient::willRetain
    \brief This property holds whether the Will Message should be retained on
    the broker for future subscribers to receive.
*/

/*!
    \property QMqttClient::autoKeepAlive
    \since 5.14
    \brief This property holds whether the client will automatically manage
    keep alive messages to the server.

    If this property is \c true, then the client will automatically send a
    ping message to the server at the keepAlive interval.

    Otherwise, a user will have to manually invoke requestPing
    within the specified interval of the connection. If no ping has been
    sent within the interval, the server will disconnect.

    The default of this property is \c true.

    \sa keepAlive(), requestPing(), serverConnectionProperties(), pingResponseReceived()
*/

/*!
    \enum QMqttClient::TransportType

    This enum type specifies the connection method to be used to instantiate a
    connection to a broker.

    \value IODevice
           The transport uses a class based on a QIODevice.
    \value AbstractSocket
           The transport uses a class based on a QAbstractSocket.
    \value SecureSocket
           The transport uses a class based on a QSslSocket.
*/

/*!
    \enum QMqttClient::ClientState

    This enum type specifies the states a client can enter.

    \value Disconnected
           The client is disconnected from the broker.
    \value Connecting
           A connection request has been made, but the broker has not approved
           the connection yet.
    \value Connected
           The client is connected to the broker.
*/

/*!
    \enum QMqttClient::ClientError

    This enum type specifies the error state of a client.

    \value NoError
           No error occurred.
    \value InvalidProtocolVersion
           The broker does not accept a connection using the specified protocol
           version.
    \value IdRejected
           The client ID is malformed. This might be related to its length.
    \value ServerUnavailable
           The network connection has been established, but the service is
           unavailable on the broker side.
    \value BadUsernameOrPassword
           The data in the username or password is malformed.
    \value NotAuthorized
           The client is not authorized to connect.
    \value TransportInvalid
           The underlying transport caused an error. For example, the connection
           might have been interrupted unexpectedly.
    \value ProtocolViolation
           The client encountered a protocol violation, and therefore closed the
           connection.
    \value UnknownError
           An unknown error occurred.
    \value Mqtt5SpecificError
           The error is related to MQTT protocol level 5. A reason code might
           provide more details.
*/

/*!
    \enum QMqttClient::ProtocolVersion

    This enum specifies the protocol version of the MQTT standard to use during
    communication with a broker.

    \value MQTT_3_1
           MQTT Standard 3.1
    \value MQTT_3_1_1
           MQTT Standard 3.1.1, publicly referred to as version 4
    \value MQTT_5_0
           MQTT Standard 5.0
*/

/*!
    \fn QMqttClient::connected()

    This signal is emitted when a connection has been established.
*/

/*!
    \fn QMqttClient::disconnected()

    This signal is emitted when a connection has been closed. A connection may
    be closed when disconnectFromHost() is called or when the broker
    disconnects.
*/

/*!
    \fn QMqttClient::messageReceived(const QByteArray &message, const QMqttTopicName &topic)

    This signal is emitted when a new message has been received. The category of
    the message is specified by \a topic with the content being \a message.
*/

/*!
    \fn QMqttClient::messageStatusChanged(qint32 id, QMqtt::MessageStatus s, const QMqttMessageStatusProperties &properties);
    \since 5.12

    This signal is emitted when the status for the message identified by \a id
    changes. \a s specifies the new status of the message, and
    \a properties specify additional properties provided by the server.
*/

/*!
    \fn QMqttClient::messageSent(qint32 id)

    Indicates that a message that was sent via the publish() function has been
    received by the broker. The \a id is the same as returned by \c publish() to
    help tracking the status of the message.
*/

/*!
    \fn QMqttClient::pingResponseReceived()

    This signal is emitted after the broker responds to a requestPing() call or
    a keepAlive() ping message, and the connection is still valid.
*/

/*!
    \fn QMqttClient::brokerSessionRestored()

    This signal is emitted after a client has successfully connected to a broker
    with the cleanSession property set to \c false, and the broker has restored
    the session.

    Sessions can be restored if a client has connected previously using the same
    clientId.
*/

/*!
    \since 5.12
    \fn QMqttClient::authenticationRequested(const QMqttAuthenticationProperties &p)

    This signal is emitted after a client invoked QMqttClient::connectToHost or
    QMqttClient::connectToHostEncrypted and before the connection is
    established. In extended authentication, a broker might request additional
    details which need to be provided by invoking QMqttClient::authenticate.
    \a p specifies properties provided by the broker.

    \note Extended authentication is part of the MQTT 5.0 standard and can
    only be used when the client specifies MQTT_5_0 as ProtocolVersion.

    \sa authenticationFinished(), authenticate()
*/

/*!
    \since 5.12
    \fn QMqttClient::authenticationFinished(const QMqttAuthenticationProperties &p)

    This signal is emitted after extended authentication has finished. \a p
    specifies available details on the authentication process.

    After successful authentication QMqttClient::connected is emitted.

    \note Extended authentication is part of the MQTT 5.0 standard and can
    only be used when the client specifies MQTT_5_0 as ProtocolVersion.

    \sa authenticationRequested(), authenticate()
*/

/*
    Creates a new MQTT client instance with the specified \a parent.
 */
QMqttClient::QMqttClient(QObject *parent) : QObject(*(new QMqttClientPrivate(this)), parent)
{
    Q_D(QMqttClient);
    d->m_connection.setClientPrivate(d);
}

/*!
    Sets the transport to \a device. A transport can be either a socket type
    or derived from QIODevice and is specified by \a transport.

    \note The transport can only be exchanged if the MQTT client is in the
    \l Disconnected state.

    \note Setting a custom transport for a client does not pass over responsibility
    on connection management. The transport has to be opened for QIODevice based
    transports or connected for socket type transports before calling QMqttClient::connectToHost().
 */
void QMqttClient::setTransport(QIODevice *device, QMqttClient::TransportType transport)
{
    Q_D(QMqttClient);

    if (d->m_state != Disconnected) {
        qCDebug(lcMqttClient) << "Changing transport layer while connected is not possible.";
        return;
    }
    d->m_connection.setTransport(device, transport);
}

/*!
    Returns the transport used for communication with the broker.
 */
QIODevice *QMqttClient::transport() const
{
    Q_D(const QMqttClient);
    return d->m_connection.transport();
}

/*!
    Adds a new subscription to receive notifications on \a topic. The parameter
    \a qos specifies the level at which security messages are received. For more
    information about the available QoS levels, see \l {Quality of Service}.

    This function returns a pointer to a \l QMqttSubscription. If the same topic
    is subscribed twice, the return value points to the same subscription
    instance. The MQTT client is the owner of the subscription.
 */
QMqttSubscription *QMqttClient::subscribe(const QMqttTopicFilter &topic, quint8 qos)
{
    return subscribe(topic, QMqttSubscriptionProperties(), qos);
}

/*!
    \since 5.12

    Adds a new subscription to receive notifications on \a topic. The parameter
    \a properties specifies additional subscription properties to be validated
    by the broker. The parameter \a qos specifies the level at which security
    messages are received. For more information about the available QoS levels,
    see \l {Quality of Service}.

    This function returns a pointer to a \l QMqttSubscription. If the same topic
    is subscribed twice, the return value points to the same subscription
    instance. The MQTT client is the owner of the subscription.

    \note \a properties will only be passed to the broker when the client
    specifies MQTT_5_0 as ProtocolVersion.
*/
QMqttSubscription *QMqttClient::subscribe(const QMqttTopicFilter &topic, const QMqttSubscriptionProperties &properties, quint8 qos)
{
    Q_D(QMqttClient);

    if (d->m_state != QMqttClient::Connected)
        return nullptr;

    return d->m_connection.sendControlSubscribe(topic, qos, properties);
}

/*!
    Unsubscribes from \a topic. No notifications will be sent to any of the
    subscriptions made by calling subscribe().

    \note If a client disconnects from a broker without unsubscribing, the
    broker will store all messages and publish them on the next reconnect.
 */
void QMqttClient::unsubscribe(const QMqttTopicFilter &topic)
{
    unsubscribe(topic, QMqttUnsubscriptionProperties());
}

/*!
    \since 5.12

    Unsubscribes from \a topic. No notifications will be sent to any of the
    subscriptions made by calling subscribe(). \a properties specifies
    additional user properties to be passed to the broker.

    \note If a client disconnects from a broker without unsubscribing, the
    broker will store all messages and publish them on the next reconnect.

    \note \a properties will only be passed to the broker when the client
    specifies MQTT_5_0 as ProtocolVersion.
*/
void QMqttClient::unsubscribe(const QMqttTopicFilter &topic, const QMqttUnsubscriptionProperties &properties)
{
    Q_D(QMqttClient);
    d->m_connection.sendControlUnsubscribe(topic, properties);
}

/*!
    Publishes a \a message to the broker with the specified \a topic. \a qos
    specifies the QoS level required for transferring the message.

    If \a retain is set to \c true, the message will stay on the broker for
    other clients to connect and receive the message.

    Returns an ID that is used internally to identify the message.
*/
qint32 QMqttClient::publish(const QMqttTopicName &topic, const QByteArray &message, quint8 qos, bool retain)
{
    return publish(topic, QMqttPublishProperties(), message, qos, retain);
}

/*!
    \since 5.12

    Publishes a \a message to the broker with the specified \a properties and
    \a topic. \a qos specifies the QoS level required for transferring
    the message.

    If \a retain is set to \c true, the message will stay on the broker for
    other clients to connect and receive the message.

    Returns an ID that is used internally to identify the message.

    \note \a properties will only be passed to the broker when the client
    specifies MQTT_5_0 as ProtocolVersion.
*/
qint32 QMqttClient::publish(const QMqttTopicName &topic, const QMqttPublishProperties &properties,
                            const QByteArray &message, quint8 qos, bool retain)
{
    Q_D(QMqttClient);
    if (qos > 2)
        return -1;

    if (d->m_state != QMqttClient::Connected)
        return -1;

    return d->m_connection.sendControlPublish(topic, message, qos, retain, properties);
}

/*!
    Sends a ping message to the broker and expects a reply.

    If the connection is active and \l autoKeepAlive is \c true, then calling this
    function will fail as the client is responsible for managing this process.

    Using \c requestPing() manually requires a call every time within the \l keepAlive
    interval as long as the connection is active.

    To check whether the ping is successful, connect to the
    \l pingResponseReceived() signal.

    Returns \c true if the ping request could be sent.

    \sa pingResponseReceived(), autoKeepAlive(), keepAlive()
 */
bool QMqttClient::requestPing()
{
    Q_D(QMqttClient);
    return d->m_connection.sendControlPingRequest(false);
}

QString QMqttClient::hostname() const
{
    Q_D(const QMqttClient);
    return d->m_hostname;
}

quint16 QMqttClient::port() const
{
    Q_D(const QMqttClient);
    return d->m_port;
}

/*!
    Initiates a connection to the MQTT broker.
 */
void QMqttClient::connectToHost()
{
    connectToHost(false, QString());
}

/*!
    \obsolete

    Initiates an encrypted connection to the MQTT broker.

    \a sslPeerName specifies the peer name to be passed to the socket.

    This function has been deprecated. Use
    \l QMqttClient::connectToHostEncrypted(const QSslConfiguration &conf) instead.
 */
#ifndef QT_NO_SSL
#if QT_DEPRECATED_SINCE(5, 14)
void QMqttClient::connectToHostEncrypted(const QString &sslPeerName)
{
    connectToHost(true, sslPeerName);
}
#endif

/*!
    \since 5.14
    Initiates an encrypted connection to the MQTT broker.

    \a conf specifies the SSL configuration to be used for the connection
 */
void QMqttClient::connectToHostEncrypted(const QSslConfiguration &conf)
{
    Q_D(QMqttClient);
    d->m_connection.m_sslConfiguration = conf;
    connectToHost(true, QString());
}
#endif

void QMqttClient::connectToHost(bool encrypted, const QString &sslPeerName)
{
    Q_D(QMqttClient);

    if (state() == QMqttClient::Connecting) {
        qCDebug(lcMqttClient) << "Connection request currently ongoing.";
        return;
    }

    if (state() == QMqttClient::Connected) {
        qCDebug(lcMqttClient) << "Already connected to a broker. Rejecting connection request.";
        return;
    }

    if (!d->m_connection.ensureTransport(encrypted)) {
        qCDebug(lcMqttClient) << "Could not ensure connection.";
        d->setStateAndError(Disconnected, TransportInvalid);
        return;
    }
    d->m_error = QMqttClient::NoError; // Fresh reconnect, unset error
    d->setStateAndError(Connecting);

    if (d->m_cleanSession)
        d->m_connection.cleanSubscriptions();

    if (!d->m_connection.ensureTransportOpen(sslPeerName)) {
        qCDebug(lcMqttClient) << "Could not ensure that connection is open.";
        d->setStateAndError(Disconnected, TransportInvalid);
        return;
    }

    // Once transport has connected, it will invoke
    // QMqttConnection::sendControlConnect to
    // handshake with the broker
}

/*!
    Disconnects from the MQTT broker.
 */
void QMqttClient::disconnectFromHost()
{
    Q_D(QMqttClient);

    switch (d->m_connection.internalState()) {
    case QMqttConnection::BrokerConnected:
        d->m_connection.sendControlDisconnect();
        break;
    case QMqttConnection::BrokerDisconnected:
        break;
    case QMqttConnection::BrokerConnecting:
    case QMqttConnection::BrokerWaitForConnectAck:
        d->m_connection.m_transport->close();
        break;
    }
}

QMqttClient::ClientState QMqttClient::state() const
{
    Q_D(const QMqttClient);
    return d->m_state;
}

QString QMqttClient::username() const
{
    Q_D(const QMqttClient);
    return d->m_username;
}

QString QMqttClient::password() const
{
    Q_D(const QMqttClient);
    return d->m_password;
}

bool QMqttClient::cleanSession() const
{
    Q_D(const QMqttClient);
    return d->m_cleanSession;
}

QString QMqttClient::willTopic() const
{
    Q_D(const QMqttClient);
    return d->m_willTopic;
}

quint8 QMqttClient::willQoS() const
{
    Q_D(const QMqttClient);
    return d->m_willQoS;
}

QByteArray QMqttClient::willMessage() const
{
    Q_D(const QMqttClient);
    return d->m_willMessage;
}

bool QMqttClient::willRetain() const
{
    Q_D(const QMqttClient);
    return d->m_willRetain;
}

bool QMqttClient::autoKeepAlive() const
{
    Q_D(const QMqttClient);
    return d->m_autoKeepAlive;
}

/*!
    \since 5.12

    Sets the connection properties to \a prop. \l QMqttConnectionProperties
    can be used to ask the server to use a specific feature set. After a
    connection request the server response can be obtained by calling
    \l QMqttClient::serverConnectionProperties.

    \note The connection properties can only be set if the MQTT client is in the
    \l Disconnected state.

    \note QMqttConnectionProperties can only be used when the client specifies
    MQTT_5_0 as ProtocolVersion.
*/
void QMqttClient::setConnectionProperties(const QMqttConnectionProperties &prop)
{
    Q_D(QMqttClient);
    d->m_connectionProperties = prop;
}

/*!
    \since 5.12

    Returns the connection properties the client requests to the broker.

    \note QMqttConnectionProperties can only be used when the client specifies
    MQTT_5_0 as ProtocolVersion.
*/
QMqttConnectionProperties QMqttClient::connectionProperties() const
{
    Q_D(const QMqttClient);
    return d->m_connectionProperties;
}

/*!
    \since 5.12

    Sets the last will properties to \a prop. QMqttLastWillProperties allows
    to set additional features for the last will message stored at the broker.

    \note The connection properties can only be set if the MQTT client is in the
    \l Disconnected state.

    \note QMqttLastWillProperties can only be used when the client specifies
    MQTT_5_0 as ProtocolVersion.
*/
void QMqttClient::setLastWillProperties(const QMqttLastWillProperties &prop)
{
    Q_D(QMqttClient);
    d->m_lastWillProperties = prop;
}

/*!
    \since 5.12

    Returns the last will properties.

    \note QMqttLastWillProperties can only be used when the client specifies
    MQTT_5_0 as ProtocolVersion.
*/
QMqttLastWillProperties QMqttClient::lastWillProperties() const
{
    Q_D(const QMqttClient);
    return d->m_lastWillProperties;
}

/*!
    \since 5.12

    Returns the QMqttServerConnectionProperties the broker returned after a
    connection attempt.

    This can be used to verify that client side connection properties set by
    QMqttClient::setConnectionProperties have been accepted by the broker. Also,
    in case of a failed connection attempt, it can be used for connection
    diagnostics.

    \note QMqttServerConnectionProperties can only be used when the client
    specifies MQTT_5_0 as ProtocolVersion.

    \sa connectionProperties()
*/
QMqttServerConnectionProperties QMqttClient::serverConnectionProperties() const
{
    Q_D(const QMqttClient);
    return d->m_serverConnectionProperties;
}

/*!
    \since 5.12

    Sends an authentication request to the broker. \a prop specifies
    the required information to fulfill the authentication request.

    This function should only be called after a
    QMqttClient::authenticationRequested signal has been emitted.

    \note Extended authentication is part of the MQTT 5.0 standard and can
    only be used when the client specifies MQTT_5_0 as ProtocolVersion.

    \sa authenticationRequested(), authenticationFinished()
*/
void QMqttClient::authenticate(const QMqttAuthenticationProperties &prop)
{
    Q_D(QMqttClient);
    if (protocolVersion() != QMqttClient::MQTT_5_0) {
        qCDebug(lcMqttClient) << "Authentication is only supported on protocol level 5.";
        return;
    }
    if (state() == QMqttClient::Disconnected) {
        qCDebug(lcMqttClient) << "Cannot send authentication request while disconnected.";
        return;
    }
    d->m_connection.sendControlAuthenticate(prop);
}

QMqttClient::ClientError QMqttClient::error() const
{
    Q_D(const QMqttClient);
    return d->m_error;
}

QMqttClient::ProtocolVersion QMqttClient::protocolVersion() const
{
    Q_D(const QMqttClient);
    return d->m_protocolVersion;
}

QString QMqttClient::clientId() const
{
    Q_D(const QMqttClient);
    return d->m_clientId;
}

quint16 QMqttClient::keepAlive() const
{
    Q_D(const QMqttClient);
    return d->m_keepAlive;
}

void QMqttClient::setHostname(const QString &hostname)
{
    Q_D(QMqttClient);

    if (state() != QMqttClient::Disconnected) {
        qCDebug(lcMqttClient) << "Changing hostname while connected is not possible.";
        return;
    }

    if (d->m_hostname == hostname)
        return;

    d->m_hostname = hostname;
    emit hostnameChanged(hostname);
}

void QMqttClient::setPort(quint16 port)
{
    Q_D(QMqttClient);

    if (state() != QMqttClient::Disconnected) {
        qCDebug(lcMqttClient) << "Changing port while connected is not possible.";
        return;
    }

    if (d->m_port == port)
        return;

    d->m_port = port;
    emit portChanged(port);
}

void QMqttClient::setClientId(const QString &clientId)
{
    Q_D(QMqttClient);

    if (state() != QMqttClient::Disconnected) {
        qCDebug(lcMqttClient) << "Changing client ID while connected is not possible.";
        return;
    }
    d->setClientId(clientId);
}

void QMqttClient::setKeepAlive(quint16 keepAlive)
{
    Q_D(QMqttClient);
    if (d->m_keepAlive == keepAlive)
        return;

    if (state() != QMqttClient::Disconnected) {
        qCDebug(lcMqttClient) << "Changing keepAlive while connected is not possible.";
        return;
    }

    d->m_keepAlive = keepAlive;
    emit keepAliveChanged(keepAlive);
}

void QMqttClient::setProtocolVersion(ProtocolVersion protocolVersion)
{
    Q_D(QMqttClient);

    if (state() != QMqttClient::Disconnected) {
        qCDebug(lcMqttClient) << "Changing protocol version while connected is not possible.";
        return;
    }

    if (d->m_protocolVersion == protocolVersion)
        return;

    if (protocolVersion < 3 || protocolVersion > 5)
        return;

    d->m_protocolVersion = protocolVersion;
    emit protocolVersionChanged(protocolVersion);
}

void QMqttClient::setState(ClientState state)
{
    Q_D(QMqttClient);
    if (d->m_state == state)
        return;

    d->m_state = state;
    emit stateChanged(state);
    if (d->m_state == QMqttClient::Disconnected)
        emit disconnected();
    else if (d->m_state == QMqttClient::Connected)
        emit connected();
}

void QMqttClient::setUsername(const QString &username)
{
    Q_D(QMqttClient);

    if (state() != QMqttClient::Disconnected) {
        qCDebug(lcMqttClient) << "Changing username while connected is not possible.";
        return;
    }

    if (d->m_username == username)
        return;

    d->m_username = username;
    emit usernameChanged(username);
}

void QMqttClient::setPassword(const QString &password)
{
    Q_D(QMqttClient);

    if (state() != QMqttClient::Disconnected) {
        qCDebug(lcMqttClient) << "Changing password while connected is not possible.";
        return;
    }

    if (d->m_password == password)
        return;

    d->m_password = password;
    emit passwordChanged(password);
}

void QMqttClient::setCleanSession(bool cleanSession)
{
    Q_D(QMqttClient);

    if (state() != QMqttClient::Disconnected) {
        qCDebug(lcMqttClient) << "Changing clean session while connected is not possible.";
        return;
    }

    if (d->m_cleanSession == cleanSession)
        return;

    d->m_cleanSession = cleanSession;
    emit cleanSessionChanged(cleanSession);
}

void QMqttClient::setWillTopic(const QString &willTopic)
{
    Q_D(QMqttClient);

    if (state() != QMqttClient::Disconnected) {
        qCDebug(lcMqttClient) << "Changing will topic while connected is not possible.";
        return;
    }

    if (d->m_willTopic == willTopic)
        return;

    d->m_willTopic = willTopic;
    emit willTopicChanged(willTopic);
}

void QMqttClient::setWillQoS(quint8 willQoS)
{
    Q_D(QMqttClient);

    if (state() != QMqttClient::Disconnected) {
        qCDebug(lcMqttClient) << "Changing will qos while connected is not possible.";
        return;
    }

    if (d->m_willQoS == willQoS)
        return;

    d->m_willQoS = willQoS;
    emit willQoSChanged(willQoS);
}

void QMqttClient::setWillMessage(const QByteArray &willMessage)
{
    Q_D(QMqttClient);

    if (state() != QMqttClient::Disconnected) {
        qCDebug(lcMqttClient) << "Changing will message while connected is not possible.";
        return;
    }

    if (d->m_willMessage == willMessage)
        return;

    d->m_willMessage = willMessage;
    emit willMessageChanged(willMessage);
}

void QMqttClient::setWillRetain(bool willRetain)
{
    Q_D(QMqttClient);

    if (state() != QMqttClient::Disconnected) {
        qCDebug(lcMqttClient) << "Changing will retain while connected is not possible.";
        return;
    }

    if (d->m_willRetain == willRetain)
        return;

    d->m_willRetain = willRetain;
    emit willRetainChanged(willRetain);
}

void QMqttClient::setAutoKeepAlive(bool autoKeepAlive)
{
    Q_D(QMqttClient);

    if (state() != QMqttClient::Disconnected) {
        qCDebug(lcMqttClient) << "Changing autoKeepAlive while connected is not possible.";
        return;
    }

    if (d->m_autoKeepAlive == autoKeepAlive)
        return;

    d->m_autoKeepAlive = autoKeepAlive;
    emit autoKeepAliveChanged(d->m_autoKeepAlive);
}

void QMqttClient::setError(ClientError e)
{
    Q_D(QMqttClient);
    if (d->m_error == e)
        return;

    d->m_error = e;
    emit errorChanged(d->m_error);
}

QMqttClientPrivate::QMqttClientPrivate(QMqttClient *c)
    : QObjectPrivate()
{
    m_client = c;
    m_clientId = QUuid::createUuid().toString();
    m_clientId.remove(QLatin1Char('{'));
    m_clientId.remove(QLatin1Char('}'));
    m_clientId.remove(QLatin1Char('-'));
    m_clientId.resize(23);
#ifdef QT_BUILD_INTERNAL
    // Some test servers require a username token
    if (qEnvironmentVariableIsSet("QT_MQTT_TEST_USERNAME"))
        m_username = qEnvironmentVariable("QT_MQTT_TEST_USERNAME");
    if (qEnvironmentVariableIsSet("QT_MQTT_TEST_PASSWORD"))
        m_password = qEnvironmentVariable("QT_MQTT_TEST_PASSWORD");
    if (qEnvironmentVariableIsSet("QT_MQTT_TEST_CLIENTID"))
        m_clientId = qEnvironmentVariable("QT_MQTT_TEST_CLIENTID");
#endif
}

QMqttClientPrivate::~QMqttClientPrivate()
{
}

void QMqttClientPrivate::setStateAndError(QMqttClient::ClientState s, QMqttClient::ClientError e)
{
    Q_Q(QMqttClient);

    if (s != m_state)
        q->setState(s);
    if (e != QMqttClient::NoError && m_error != e)
        q->setError(e);
}

void QMqttClientPrivate::setClientId(const QString &id)
{
    Q_Q(QMqttClient);

    if (m_clientId == id)
        return;

    m_clientId = id;
    emit q->clientIdChanged(id);
}

QT_END_NAMESPACE
