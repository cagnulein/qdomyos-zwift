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

#include "qmqttconnection_p.h"
#include "qmqttconnectionproperties_p.h"
#include "qmqttcontrolpacket_p.h"
#include "qmqttmessage_p.h"
#include "qmqttpublishproperties_p.h"
#include "qmqttsubscription_p.h"
#include "qmqttclient_p.h"

#include <QtCore/QLoggingCategory>
#include <QtNetwork/QSslSocket>
#include <QtNetwork/QTcpSocket>

#include <limits>
#include <cstdint>

QT_BEGIN_NAMESPACE

Q_LOGGING_CATEGORY(lcMqttConnection, "qt.mqtt.connection")
Q_LOGGING_CATEGORY(lcMqttConnectionVerbose, "qt.mqtt.connection.verbose");

template <typename T>
T QMqttConnection::readBufferTyped(qint64 *dataSize)
{
    Q_STATIC_ASSERT(std::is_integral<T>::value);

    T result = 0;
    if (Q_UNLIKELY(dataSize != nullptr && *dataSize < qint64(sizeof(result)))) {
        qCWarning(lcMqttConnection) << "Attempt to read past the data";
        return result;
    }
    if (readBuffer(reinterpret_cast<char *>(&result), sizeof(result)) && dataSize != nullptr)
        *dataSize -= sizeof(result);
    return qFromBigEndian(result);
}

template<>
QByteArray QMqttConnection::readBufferTyped(qint64 *dataSize)
{
    const quint16 size = readBufferTyped<quint16>(dataSize);
    if (Q_UNLIKELY(dataSize != nullptr && *dataSize < qint64(size))) {
        qCWarning(lcMqttConnection) << "Attempt to read past the data";
        return QByteArray();
    }
    QByteArray ba(int(size), Qt::Uninitialized);
    if (readBuffer(ba.data(), size) && dataSize != nullptr)
        *dataSize -= size;
    return ba;
}

template<>
QString QMqttConnection::readBufferTyped(qint64 *dataSize)
{
    return QString::fromUtf8(readBufferTyped<QByteArray>(dataSize));
}

QMqttConnection::QMqttConnection(QObject *parent) : QObject(parent)
{
}

QMqttConnection::~QMqttConnection()
{
    if (m_internalState == BrokerConnected)
        sendControlDisconnect();

    if (m_ownTransport && m_transport)
        delete m_transport;
}

void QMqttConnection::timerEvent(QTimerEvent *event)
{
    if (Q_LIKELY(event->timerId() == m_pingTimer.timerId())) {
        sendControlPingRequest();
        return;
    }

    QObject::timerEvent(event);
}

void QMqttConnection::setTransport(QIODevice *device, QMqttClient::TransportType transport)
{
    qCDebug(lcMqttConnection) << Q_FUNC_INFO << device << " Type:" << transport;

    if (m_transport) {
        disconnect(m_transport, &QIODevice::aboutToClose, this, &QMqttConnection::transportConnectionClosed);
        disconnect(m_transport, &QIODevice::readyRead, this, &QMqttConnection::transportReadReady);
        if (m_ownTransport)
            delete m_transport;
    }

    m_transport = device;
    m_transportType = transport;
    m_ownTransport = false;

    connect(m_transport, &QIODevice::aboutToClose, this, &QMqttConnection::transportConnectionClosed);
    connect(m_transport, &QIODevice::readyRead, this, &QMqttConnection::transportReadReady);
}

QIODevice *QMqttConnection::transport() const
{
    return m_transport;
}

bool QMqttConnection::ensureTransport(bool createSecureIfNeeded)
{
    Q_UNUSED(createSecureIfNeeded); // QT_NO_SSL
    qCDebug(lcMqttConnection) << Q_FUNC_INFO << m_transport;

    if (m_transport) {
        if (m_ownTransport)
            delete m_transport;
        else
            return true;
    }

    // We are asked to create a transport layer
    if (m_clientPrivate->m_hostname.isEmpty() || m_clientPrivate->m_port == 0) {
        qCDebug(lcMqttConnection) << "No hostname specified, not able to create a transport layer.";
        return false;
    }
    auto socket =
#ifndef QT_NO_SSL
            createSecureIfNeeded ? new QSslSocket() :
#endif
                                   new QTcpSocket();
    m_transport = socket;
    m_ownTransport = true;
    m_transportType =
#ifndef QT_NO_SSL
        createSecureIfNeeded ? QMqttClient::SecureSocket :
#endif
                               QMqttClient::AbstractSocket;

#ifndef QT_NO_SSL
    if (QSslSocket *sslSocket = qobject_cast<QSslSocket *>(socket))
        QObject::connect(sslSocket, &QSslSocket::encrypted, this, &QMqttConnection::transportConnectionEstablished);
    else
#endif
    connect(socket, &QAbstractSocket::connected, this, &QMqttConnection::transportConnectionEstablished);
    connect(socket, &QAbstractSocket::disconnected, this, &QMqttConnection::transportConnectionClosed);
    connect(socket, &QAbstractSocket::errorOccurred, this, &QMqttConnection::transportError);

    connect(m_transport, &QIODevice::aboutToClose, this, &QMqttConnection::transportConnectionClosed);
    connect(m_transport, &QIODevice::readyRead, this, &QMqttConnection::transportReadReady);
    return true;
}

bool QMqttConnection::ensureTransportOpen(const QString &sslPeerName)
{
    qCDebug(lcMqttConnection) << Q_FUNC_INFO << m_transportType;

    if (m_transportType == QMqttClient::IODevice) {
        if (m_transport->isOpen())
            return sendControlConnect();

        if (!m_transport->open(QIODevice::ReadWrite)) {
            qCDebug(lcMqttConnection) << "Could not open Transport IO device.";
            m_internalState = BrokerDisconnected;
            return false;
        }
        return sendControlConnect();
    }

    if (m_transportType == QMqttClient::AbstractSocket) {
        auto socket = qobject_cast<QTcpSocket *>(m_transport);
        Q_ASSERT(socket);
        if (socket->state() == QAbstractSocket::ConnectedState)
            return sendControlConnect();

        m_internalState = BrokerConnecting;
        socket->connectToHost(m_clientPrivate->m_hostname, m_clientPrivate->m_port);
    }
#ifndef QT_NO_SSL
    else if (m_transportType == QMqttClient::SecureSocket) {
        auto socket = qobject_cast<QSslSocket *>(m_transport);
        Q_ASSERT(socket);
        if (socket->state() == QAbstractSocket::ConnectedState)
            return sendControlConnect();

        m_internalState = BrokerConnecting;
        if (!m_sslConfiguration.isNull())
            socket->setSslConfiguration(m_sslConfiguration);
        socket->connectToHostEncrypted(m_clientPrivate->m_hostname, m_clientPrivate->m_port, sslPeerName);
    }
#else
    Q_UNUSED(sslPeerName);
#endif

    return true;
}

bool QMqttConnection::sendControlConnect()
{
    qCDebug(lcMqttConnection) << Q_FUNC_INFO;

    QMqttControlPacket packet(QMqttControlPacket::CONNECT);

    // Variable header
    // 3.1.2.1 Protocol Name
    // 3.1.2.2 Protocol Level
    switch (m_clientPrivate->m_protocolVersion) {
    case QMqttClient::MQTT_3_1:
        packet.append("MQIsdp");
        packet.append(char(3)); // Version 3.1
        break;
    case QMqttClient::MQTT_3_1_1:
        packet.append("MQTT");
        packet.append(char(4)); // Version 3.1.1
        break;
    case QMqttClient::MQTT_5_0:
        packet.append("MQTT");
        packet.append(char(5)); // Version 5.0
        break;
    }

    // 3.1.2.3 Connect Flags
    quint8 flags = 0;
    // Clean session
    if (m_clientPrivate->m_cleanSession)
        flags |= 1 << 1;

    if (!m_clientPrivate->m_willTopic.isEmpty()) {
        flags |= 1 << 2;
        if (m_clientPrivate->m_willQoS > 2) {
            qCDebug(lcMqttConnection) << "Invalid Will QoS specified.";
            return false;
        }
        if (m_clientPrivate->m_willQoS == 1)
            flags |= 1 << 3;
        else if (m_clientPrivate->m_willQoS == 2)
            flags |= 1 << 4;
        if (m_clientPrivate->m_willRetain)
            flags |= 1 << 5;
    }
    if (m_clientPrivate->m_username.size())
        flags |= 1 << 7;

    if (m_clientPrivate->m_password.size())
        flags |= 1 << 6;

    packet.append(char(flags));

    // 3.1.2.10 Keep Alive
    packet.append(m_clientPrivate->m_keepAlive);

    if (m_clientPrivate->m_protocolVersion == QMqttClient::MQTT_5_0)
        packet.appendRaw(writeConnectProperties());

    // 3.1.3 Payload
    // 3.1.3.1 Client Identifier
    const QByteArray clientStringArray = m_clientPrivate->m_clientId.toUtf8();
    if (clientStringArray.size()) {
        packet.append(clientStringArray);
    } else {
        packet.append(char(0));
        packet.append(char(0));
    }

    if (!m_clientPrivate->m_willTopic.isEmpty()) {
        if (m_clientPrivate->m_protocolVersion == QMqttClient::MQTT_5_0)
            packet.appendRaw(writeLastWillProperties());

        packet.append(m_clientPrivate->m_willTopic.toUtf8());
        packet.append(m_clientPrivate->m_willMessage);
    }

    if (m_clientPrivate->m_username.size())
        packet.append(m_clientPrivate->m_username.toUtf8());

    if (m_clientPrivate->m_password.size())
        packet.append(m_clientPrivate->m_password.toUtf8());

    m_internalState = BrokerWaitForConnectAck;
    m_missingData = 0;

    if (!writePacketToTransport(packet)) {
        qCDebug(lcMqttConnection) << "Could not write CONNECT frame to transport.";
        return false;
    }
    return true;
}

bool QMqttConnection::sendControlAuthenticate(const QMqttAuthenticationProperties &properties)
{
    qCDebug(lcMqttConnection) << Q_FUNC_INFO;

    QMqttControlPacket packet(QMqttControlPacket::AUTH);

    switch (m_internalState) {
    case BrokerDisconnected:
    case BrokerConnecting:
        qCDebug(lcMqttConnection) << "Using AUTH while disconnected.";
        return false;
    case BrokerWaitForConnectAck:
        qCDebug(lcMqttConnection) << "AUTH while connecting, set continuation flag.";
        packet.append(char(QMqtt::ReasonCode::ContinueAuthentication));
        break;
    case BrokerConnected:
        qCDebug(lcMqttConnection) << "AUTH while connected, initiate re-authentication.";
        packet.append(char(QMqtt::ReasonCode::ReAuthenticate));
        break;
    }

    packet.appendRaw(writeAuthenticationProperties(properties));

    if (!writePacketToTransport(packet)) {
        qCDebug(lcMqttConnection) << "Could not write AUTH frame to transport.";
        return false;
    }

    return true;
}

qint32 QMqttConnection::sendControlPublish(const QMqttTopicName &topic,
                                           const QByteArray &message,
                                           quint8 qos,
                                           bool retain,
                                           const QMqttPublishProperties &properties)
{
    qCDebug(lcMqttConnection) << Q_FUNC_INFO << topic << " Size:" << message.size() << " bytes."
                              << "QoS:" << qos << " Retain:" << retain;

    if (!topic.isValid())
        return -1;

    quint8 header = QMqttControlPacket::PUBLISH;
    if (qos == 1)
        header |= 0x02;
    else if (qos == 2)
        header |= 0x04;

    if (retain)
        header |= 0x01;

    QSharedPointer<QMqttControlPacket> packet(new QMqttControlPacket(header));
    // topic alias
    QMqttPublishProperties publishProperties(properties);
    if (m_clientPrivate->m_protocolVersion == QMqttClient::MQTT_5_0) {
        // 3.3.4 A PUBLISH packet sent from a Client to a Server MUST NOT contain a Subscription Identifier
        if (publishProperties.availableProperties() & QMqttPublishProperties::SubscriptionIdentifier) {
            qCWarning(lcMqttConnection) << "SubscriptionIdentifier must not be specified for publish.";
            return -1;
        }

        const quint16 topicAlias = publishProperties.topicAlias();
        if (topicAlias > 0) { // User specified topic Alias
            if (topicAlias > m_clientPrivate->m_serverConnectionProperties.maximumTopicAlias()) {
                qCDebug(lcMqttConnection) << "TopicAlias publish: overflow.";
                return -1;
            }
            if (m_publishAliases.at(topicAlias - 1) != topic) {
                qCDebug(lcMqttConnection) << "TopicAlias publish: Assign:" << topicAlias << ":" << topic;
                m_publishAliases[topicAlias - 1] = topic;
                packet->append(topic.name().toUtf8());
            } else {
                qCDebug(lcMqttConnectionVerbose) << "TopicAlias publish: Reuse:" << topicAlias;
                packet->append(quint16(0));
            }
        } else if (m_publishAliases.size() > 0) { // Automatic module alias assignment
            int autoAlias = m_publishAliases.indexOf(topic);
            if (autoAlias != -1) {
                qCDebug(lcMqttConnectionVerbose) << "TopicAlias publish: Use auto alias:" << autoAlias;
                packet->append(quint16(0));
                publishProperties.setTopicAlias(quint16(autoAlias + 1));
            } else {
                autoAlias = m_publishAliases.indexOf(QMqttTopicName());
                if (autoAlias != -1) {
                    qCDebug(lcMqttConnectionVerbose) << "TopicAlias publish: auto alias assignment:" << autoAlias;
                    m_publishAliases[autoAlias] = topic;
                    publishProperties.setTopicAlias(quint16(autoAlias) + 1);
                } else
                    qCDebug(lcMqttConnectionVerbose) << "TopicAlias publish: alias storage full, using full topic";
                packet->append(topic.name().toUtf8());
            }
        } else {
            packet->append(topic.name().toUtf8());
        }
    } else { // ! MQTT_5_0
        packet->append(topic.name().toUtf8());
    }
    quint16 identifier = 0;
    if (qos > 0) {
        identifier = unusedPacketIdentifier();
        packet->append(identifier);
        m_pendingMessages.insert(identifier, packet);
    }

    if (m_clientPrivate->m_protocolVersion == QMqttClient::MQTT_5_0)
        packet->appendRaw(writePublishProperties(publishProperties));

    packet->appendRaw(message);

    const bool written = writePacketToTransport(*packet.data());

    if (!written && qos > 0)
        m_pendingMessages.remove(identifier);
    return written ? identifier : -1;
}

bool QMqttConnection::sendControlPublishAcknowledge(quint16 id)
{
    qCDebug(lcMqttConnection) << Q_FUNC_INFO << id;
    QMqttControlPacket packet(QMqttControlPacket::PUBACK);
    packet.append(id);
    return writePacketToTransport(packet);
}

bool QMqttConnection::sendControlPublishRelease(quint16 id)
{
    qCDebug(lcMqttConnection) << Q_FUNC_INFO << id;
    quint8 header = QMqttControlPacket::PUBREL;
    header |= 0x02; // MQTT-3.6.1-1

    QMqttControlPacket packet(header);
    packet.append(id);
    return writePacketToTransport(packet);
}

bool QMqttConnection::sendControlPublishReceive(quint16 id)
{
    qCDebug(lcMqttConnection) << Q_FUNC_INFO << id;
    QMqttControlPacket packet(QMqttControlPacket::PUBREC);
    packet.append(id);
    return writePacketToTransport(packet);
}

bool QMqttConnection::sendControlPublishComp(quint16 id)
{
    qCDebug(lcMqttConnection) << Q_FUNC_INFO << id;
    QMqttControlPacket packet(QMqttControlPacket::PUBCOMP);
    packet.append(id);
    return writePacketToTransport(packet);
}

QMqttSubscription *QMqttConnection::sendControlSubscribe(const QMqttTopicFilter &topic,
                                                         quint8 qos,
                                                         const QMqttSubscriptionProperties &properties)
{
    qCDebug(lcMqttConnection) << Q_FUNC_INFO << " Topic:" << topic << " qos:" << qos;

    // Overflow protection
    if (Q_UNLIKELY(!topic.isValid())) {
        qCWarning(lcMqttConnection) << "Invalid subscription topic filter.";
        return nullptr;
    }

    if (Q_UNLIKELY(qos > 2)) {
        qCWarning(lcMqttConnection) << "Invalid subscription QoS.";
        return nullptr;
    }

    if (m_clientPrivate->m_protocolVersion == QMqttClient::MQTT_5_0) {
        const QString sharedSubscriptionName = topic.sharedSubscriptionName();
        if (!sharedSubscriptionName.isEmpty()) {
            const QMqttTopicFilter filter(topic.filter().section(QLatin1Char('/'), 2));
            auto it = m_activeSubscriptions.constFind(filter);
            if (it != m_activeSubscriptions.cend() && (*it)->sharedSubscriptionName() == sharedSubscriptionName)
                return *it;
        } else {
            auto it = m_activeSubscriptions.constFind(topic);
            if (it != m_activeSubscriptions.cend() && !(*it)->isSharedSubscription())
                return *it;
        }
    } else {
        auto it = m_activeSubscriptions.constFind(topic);
        if (it != m_activeSubscriptions.cend())
            return *it;
    }

    // has to have 0010 as bits 3-0, maybe update SUBSCRIBE instead?
    // MQTT-3.8.1-1
    const quint8 header = QMqttControlPacket::SUBSCRIBE + 0x02;
    QMqttControlPacket packet(header);

    // Add Packet Identifier
    const quint16 identifier = unusedPacketIdentifier();

    packet.append(identifier);

    if (m_clientPrivate->m_protocolVersion == QMqttClient::MQTT_5_0)
        packet.appendRaw(writeSubscriptionProperties(properties));

    packet.append(topic.filter().toUtf8());
    packet.append(char(qos));

    auto result = new QMqttSubscription(this);
    result->setTopic(topic);
    result->setClient(m_clientPrivate->m_client);
    result->setQos(qos);
    result->setState(QMqttSubscription::SubscriptionPending);
    if (m_clientPrivate->m_protocolVersion == QMqttClient::MQTT_5_0 && !topic.sharedSubscriptionName().isEmpty()) {
        result->setSharedSubscriptionName(topic.sharedSubscriptionName());
        result->setSharedSubscription(true);
        result->setTopic(topic.filter().section(QLatin1Char('/'), 2));
    }

    if (!writePacketToTransport(packet)) {
        delete result;
        return nullptr;
    }

    // SUBACK must contain identifier MQTT-3.8.4-2
    m_pendingSubscriptionAck.insert(identifier, result);
    m_activeSubscriptions.insert(result->topic(), result);
    return result;
}

bool QMqttConnection::sendControlUnsubscribe(const QMqttTopicFilter &topic, const QMqttUnsubscriptionProperties &properties)
{
    qCDebug(lcMqttConnection) << Q_FUNC_INFO << " Topic:" << topic;

    // MQTT-3.10.3-2
    if (!topic.isValid())
        return false;

    if (!m_activeSubscriptions.contains(topic))
        return false;

    if (m_internalState != QMqttConnection::BrokerConnected) {
        m_activeSubscriptions.remove(topic);
        return true;
    }

    // has to have 0010 as bits 3-0, maybe update UNSUBSCRIBE instead?
    // MQTT-3.10.1-1
    const quint8 header = QMqttControlPacket::UNSUBSCRIBE + 0x02;
    QMqttControlPacket packet(header);

    // Add Packet Identifier
    const quint16 identifier = unusedPacketIdentifier();

    packet.append(identifier);

    if (m_clientPrivate->m_protocolVersion == QMqttClient::MQTT_5_0) {
        packet.appendRaw(writeUnsubscriptionProperties(properties));
    }

    packet.append(topic.filter().toUtf8());
    auto sub = m_activeSubscriptions[topic];
    sub->setState(QMqttSubscription::UnsubscriptionPending);

    if (!writePacketToTransport(packet))
        return false;

    // Do not remove from m_activeSubscriptions as there might be QoS1/2 messages to still
    // be sent before UNSUBSCRIBE is acknowledged.
    m_pendingUnsubscriptions.insert(identifier, sub);

    return true;
}

bool QMqttConnection::sendControlPingRequest(bool isAuto)
{
    qCDebug(lcMqttConnection) << Q_FUNC_INFO;

    if (m_internalState != QMqttConnection::BrokerConnected)
        return false;


    if (!isAuto && m_clientPrivate->m_autoKeepAlive) {
        qCDebug(lcMqttConnection) << "Requesting a manual ping while autoKeepAlive is enabled "
                                  << "is not allowed.";
        return false;
    }

    // 3.1.2.10 If a Client does not receive a PINGRESP packet within a reasonable amount of time
    // after it has sent a PINGREQ, it SHOULD close the Network Connection to the Server
    if (m_pingTimeout > 1) {
        closeConnection(QMqttClient::ServerUnavailable);
        return false;
    }

    const QMqttControlPacket packet(QMqttControlPacket::PINGREQ);
    if (!writePacketToTransport(packet)) {
        qCDebug(lcMqttConnection) << "Failed to write PINGREQ to transport.";
        return false;
    }
    m_pingTimeout++;
    return true;
}

bool QMqttConnection::sendControlDisconnect()
{
    qCDebug(lcMqttConnection) << Q_FUNC_INFO;

    m_pingTimer.stop();
    m_pingTimeout = 0;

    m_activeSubscriptions.clear();

    m_receiveAliases.clear();
    m_publishAliases.clear();

    const QMqttControlPacket packet(QMqttControlPacket::DISCONNECT);
    if (!writePacketToTransport(packet)) {
        qCDebug(lcMqttConnection) << "Failed to write DISCONNECT to transport.";
        return false;
    }
    m_internalState = BrokerDisconnected;

    if (m_transport->waitForBytesWritten(30000)) {
        // MQTT-3.14.4-1 must disconnect
        m_transport->close();
        return true;
    }
    return false;
}

void QMqttConnection::setClientPrivate(QMqttClientPrivate *clientPrivate)
{
    m_clientPrivate = clientPrivate;
}

quint16 QMqttConnection::unusedPacketIdentifier() const
{
    // MQTT-2.3.1-1 Control Packets MUST contain a non-zero 16-bit Packet Identifier
    static quint16 packetIdentifierCounter = 1;
    const std::uint16_t u16max = std::numeric_limits<std::uint16_t>::max();

    // MQTT-2.3.1-2 ...it MUST assign it a currently unused Packet Identifier
    const quint16 lastIdentifier = packetIdentifierCounter;
    do {
        if (packetIdentifierCounter == u16max)
            packetIdentifierCounter = 1;
        else
            packetIdentifierCounter++;

        if (lastIdentifier == packetIdentifierCounter) {
            qCDebug(lcMqttConnection) << "Could not generate unique packet identifier.";
            break;
        }
    } while (m_pendingSubscriptionAck.contains(packetIdentifierCounter)
             || m_pendingUnsubscriptions.contains(packetIdentifierCounter)
             || m_pendingMessages.contains(packetIdentifierCounter)
             || m_pendingReleaseMessages.contains(packetIdentifierCounter));
    return packetIdentifierCounter;
}

void QMqttConnection::cleanSubscriptions()
{
    for (auto item : m_pendingSubscriptionAck)
        item->setState(QMqttSubscription::Unsubscribed);
    m_pendingSubscriptionAck.clear();

    for (auto item : m_pendingUnsubscriptions)
        item->setState(QMqttSubscription::Unsubscribed);
    m_pendingUnsubscriptions.clear();

    for (auto item : m_activeSubscriptions)
        item->setState(QMqttSubscription::Unsubscribed);
    m_activeSubscriptions.clear();
}

void QMqttConnection::transportConnectionEstablished()
{
    if (m_internalState != BrokerConnecting) {
        qCWarning(lcMqttConnection) << "Connection established at an unexpected time";
        return;
    }

    if (!sendControlConnect()) {
        qCDebug(lcMqttConnection) << "Failed to write CONNECT to transport.";
        // ### Who disconnects now? Connection or client?
        m_clientPrivate->setStateAndError(QMqttClient::Disconnected, QMqttClient::TransportInvalid);
    }
}

void QMqttConnection::transportConnectionClosed()
{
    m_readBuffer.clear();
    m_readPosition = 0;
    m_pingTimer.stop();
    m_pingTimeout = 0;
    if (m_internalState == BrokerDisconnected) // We manually disconnected
        m_clientPrivate->setStateAndError(QMqttClient::Disconnected, QMqttClient::NoError);
    else
        m_clientPrivate->setStateAndError(QMqttClient::Disconnected, QMqttClient::TransportInvalid);
}

void QMqttConnection::transportReadReady()
{
    qCDebug(lcMqttConnectionVerbose) << Q_FUNC_INFO;
    m_readBuffer.append(m_transport->readAll());
    processData();
}

void QMqttConnection::transportError(QAbstractSocket::SocketError e)
{
    qCDebug(lcMqttConnection) << Q_FUNC_INFO << e;
    closeConnection(QMqttClient::TransportInvalid);
}

bool QMqttConnection::readBuffer(char *data, quint64 size)
{
    if (Q_UNLIKELY(quint64(m_readBuffer.size() - m_readPosition) < size)) {
        qCDebug(lcMqttConnection) << "Reaching out of buffer, protocol violation";
        closeConnection(QMqttClient::ProtocolViolation);
        return false;
    }
    memcpy(data, m_readBuffer.constData() + m_readPosition, size);
    m_readPosition += size;
    return true;
}

qint32 QMqttConnection::readVariableByteInteger(qint64 *dataSize)
{
    quint32 multiplier = 1;
    qint32 msgLength = 0;
    quint8 b = 0;
    quint8 iteration = 0;
    do {
        b = readBufferTyped<quint8>(dataSize);
        msgLength += (b & 127) * multiplier;
        multiplier *= 128;
        iteration++;
        if (iteration > 4) {
            qCDebug(lcMqttConnection) << "Overflow trying to read variable integer.";
            closeConnection(QMqttClient::ProtocolViolation);
            return -1;
        }
    } while ((b & 128) != 0);
    return msgLength;
}

void QMqttConnection::closeConnection(QMqttClient::ClientError error)
{
    m_readBuffer.clear();
    m_readPosition = 0;
    m_pingTimer.stop();
    m_pingTimeout = 0;
    m_activeSubscriptions.clear();
    m_internalState = BrokerDisconnected;
    m_transport->disconnect();
    m_transport->close();
    m_clientPrivate->setStateAndError(QMqttClient::Disconnected, error);
}

QByteArray QMqttConnection::readBuffer(quint64 size)
{
    if (Q_UNLIKELY(quint64(m_readBuffer.size() - m_readPosition) < size)) {
        qCDebug(lcMqttConnection) << "Reaching out of buffer, protocol violation";
        closeConnection(QMqttClient::ProtocolViolation);
        return QByteArray();
    }
    QByteArray res(m_readBuffer.constData() + m_readPosition, int(size));
    m_readPosition += size;
    return res;
}

void QMqttConnection::readAuthProperties(QMqttAuthenticationProperties &properties)
{
    qint64 propertyLength = readVariableByteInteger(&m_missingData);
    m_missingData -= propertyLength;

    QMqttUserProperties userProperties;
    while (propertyLength > 0) {
        quint8 propertyId = readBufferTyped<quint8>(&propertyLength);

        switch (propertyId) {
        case 0x15: { //3.15.2.2.2 Authentication Method
            const QString method = readBufferTyped<QString>(&propertyLength);
            properties.setAuthenticationMethod(method);
            break;
        }
        case 0x16: { // 3.15.2.2.3 Authentication Data
            const QByteArray data = readBufferTyped<QByteArray>(&propertyLength);
            properties.setAuthenticationData(data);
            break;
        }
        case 0x1F: { // 3.15.2.2.4 Reason String
            const QString reasonString = readBufferTyped<QString>(&propertyLength);
            properties.setReason(reasonString);
            break;
        }
        case 0x26: { // 3.15.2.2.5 User property
            const QString propertyName = readBufferTyped<QString>(&propertyLength);
            const QString propertyValue = readBufferTyped<QString>(&propertyLength);

            userProperties.append(QMqttStringPair(propertyName, propertyValue));
            break;
        }
        default:
            qCDebug(lcMqttConnection) << "Unknown property id in AUTH:" << propertyId;
            break;
        }
    }
    if (!userProperties.isEmpty())
        properties.setUserProperties(userProperties);
}

void QMqttConnection::readConnackProperties(QMqttServerConnectionProperties &properties)
{
    qint64 propertyLength = readVariableByteInteger(&m_missingData);
    m_missingData -= propertyLength;

    properties.serverData->valid = true;

    while (propertyLength > 0) {
        quint8 propertyId = readBufferTyped<quint8>(&propertyLength);
        switch (propertyId) {
        case 0x11: { // 3.2.2.3.2 Session Expiry Interval
            const quint32 expiryInterval = readBufferTyped<quint32>(&propertyLength);
            properties.serverData->details |= QMqttServerConnectionProperties::SessionExpiryInterval;
            properties.setSessionExpiryInterval(expiryInterval);
            break;
        }
        case 0x21: { // 3.2.2.3.3 Receive Maximum
            const quint16 receiveMaximum = readBufferTyped<quint16>(&propertyLength);
            properties.serverData->details |= QMqttServerConnectionProperties::MaximumReceive;
            properties.setMaximumReceive(receiveMaximum);
            break;
        }
        case 0x24: { // 3.2.2.3.4 Maximum QoS Level
            const quint8 maxQoS = readBufferTyped<quint8>(&propertyLength);
            properties.serverData->details |= QMqttServerConnectionProperties::MaximumQoS;
            properties.serverData->maximumQoS = maxQoS;
            break;
        }
        case 0x25: { // 3.2.2.3.5 Retain available
            const quint8 retainAvailable = readBufferTyped<quint8>(&propertyLength);
            properties.serverData->details |= QMqttServerConnectionProperties::RetainAvailable;
            properties.serverData->retainAvailable = retainAvailable == 1;
            break;
        }
        case 0x27: { // 3.2.2.3.6 Maximum packet size
            const quint32 maxPacketSize = readBufferTyped<quint32>(&propertyLength);
            properties.serverData->details |= QMqttServerConnectionProperties::MaximumPacketSize;
            properties.setMaximumPacketSize(maxPacketSize);
            break;
        }
        case 0x12: { // 3.2.2.3.7 Assigned clientId
            const QString assignedClientId = readBufferTyped<QString>(&propertyLength);
            properties.serverData->details |= QMqttServerConnectionProperties::AssignedClientId;
            m_clientPrivate->setClientId(assignedClientId);
            break;
        }
        case 0x22: { // 3.2.2.3.8 Topic Alias Maximum
            const quint16 topicAliasMaximum = readBufferTyped<quint16>(&propertyLength);
            properties.serverData->details |= QMqttServerConnectionProperties::MaximumTopicAlias;
            properties.setMaximumTopicAlias(topicAliasMaximum);
            break;
        }
        case 0x1F: { // 3.2.2.3.9 Reason String
            const QString reasonString = readBufferTyped<QString>(&propertyLength);
            properties.serverData->details |= QMqttServerConnectionProperties::ReasonString;
            properties.serverData->reasonString = reasonString;
            break;
        }
        case 0x26: { // 3.2.2.3.10 User property
            const QString propertyName = readBufferTyped<QString>(&propertyLength);
            const QString propertyValue = readBufferTyped<QString>(&propertyLength);

            properties.serverData->details |= QMqttServerConnectionProperties::UserProperty;
            properties.data->userProperties.append(QMqttStringPair(propertyName, propertyValue));
            break;
        }
        case 0x28: { // 3.2.2.3.11 Wildcard subscriptions available
            const quint8 available = readBufferTyped<quint8>(&propertyLength);
            properties.serverData->details |= QMqttServerConnectionProperties::WildCardSupported;
            properties.serverData->wildcardSupported = available == 1;
            break;
        }
        case 0x29: { // 3.2.2.3.12 Subscription identifiers available
            const quint8 available = readBufferTyped<quint8>(&propertyLength);
            properties.serverData->details |= QMqttServerConnectionProperties::SubscriptionIdentifierSupport;
            properties.serverData->subscriptionIdentifierSupported = available == 1;
            break;
        }
        case 0x2A: { // 3.2.2.3.13 Shared subscriptions available
            const quint8 available = readBufferTyped<quint8>(&propertyLength);
            properties.serverData->details |= QMqttServerConnectionProperties::SharedSubscriptionSupport;
            properties.serverData->sharedSubscriptionSupported = available == 1;
            break;
        }
        case 0x13: { // 3.2.2.3.14 Server Keep Alive
            const quint16 serverKeepAlive = readBufferTyped<quint16>(&propertyLength);
            properties.serverData->details |= QMqttServerConnectionProperties::ServerKeepAlive;
            m_clientPrivate->m_client->setKeepAlive(serverKeepAlive);
            break;
        }
        case 0x1A: { // 3.2.2.3.15 Response information
            const QString responseInfo = readBufferTyped<QString>(&propertyLength);
            properties.serverData->details |= QMqttServerConnectionProperties::ResponseInformation;
            properties.serverData->responseInformation = responseInfo;
            break;
        }
        case 0x1C: { // 3.2.2.3.16 Server reference
            const QString serverReference = readBufferTyped<QString>(&propertyLength);
            properties.serverData->details |= QMqttServerConnectionProperties::ServerReference;
            properties.serverData->serverReference = serverReference;
            break;
        }
        case 0x15: { // 3.2.2.3.17 Authentication method
            const QString method = readBufferTyped<QString>(&propertyLength);
            properties.serverData->details |= QMqttServerConnectionProperties::AuthenticationMethod;
            properties.data->authenticationMethod = method;
            break;
        }
        case 0x16: { // 3.2.2.3.18 Authentication data
            const QByteArray data = readBufferTyped<QByteArray>(&propertyLength);
            properties.serverData->details |= QMqttServerConnectionProperties::AuthenticationData;
            properties.data->authenticationData = data;
            break;
        }
        default:
            qCDebug(lcMqttConnection) << "Unknown property id in CONNACK:" << int(propertyId);
            break;
        }
    }
}

void QMqttConnection::readMessageStatusProperties(QMqttMessageStatusProperties &properties)
{
    qint64 propertyLength = readVariableByteInteger(&m_missingData);
    m_missingData -= propertyLength;

    while (propertyLength > 0) {
        const quint8 propertyId = readBufferTyped<quint8>(&propertyLength);
        switch (propertyId) {
        case 0x1f: { // 3.4.2.2.2 Reason String
            const QString content = readBufferTyped<QString>(&propertyLength);
            properties.data->reasonString = content;
            break;
        }
        case 0x26: { // 3.4.2.2.3 User Properties
            const QString propertyName = readBufferTyped<QString>(&propertyLength);
            const QString propertyValue = readBufferTyped<QString>(&propertyLength);
            properties.data->userProperties.append(QMqttStringPair(propertyName, propertyValue));
            break;
        }
        default:
            qCDebug(lcMqttConnection) << "Unknown subscription property received.";
            break;
        }
    }
}

void QMqttConnection::readPublishProperties(QMqttPublishProperties &properties)
{
    qint64 propertyLength = readVariableByteInteger(&m_missingData);
    m_missingData -= propertyLength;

    QMqttUserProperties userProperties;
    QList<quint32> subscriptionIds;

    while (propertyLength > 0) {
        const quint8 propertyId = readBufferTyped<quint8>(&propertyLength);
        switch (propertyId) {
        case 0x01: { // 3.3.2.3.2 Payload Format Indicator
            const quint8 format = readBufferTyped<quint8>(&propertyLength);
            if (format == 1)
                properties.setPayloadFormatIndicator(QMqtt::PayloadFormatIndicator::UTF8Encoded);
            break;
        }
        case 0x02: { // 3.3.2.3.3 Message Expiry Interval
            const quint32 interval = readBufferTyped<quint32>(&propertyLength);
            properties.setMessageExpiryInterval(interval);
            break;
        }
        case 0x23: { // 3.3.2.3.4 Topic alias
            const quint16 alias = readBufferTyped<quint16>(&propertyLength);
            properties.setTopicAlias(alias);
            break;
        }
        case 0x08: { // 3.3.2.3.5 Response Topic
            const QString responseTopic = readBufferTyped<QString>(&propertyLength);
            properties.setResponseTopic(responseTopic);
            break;
        }
        case 0x09: { // 3.3.2.3.6 Correlation Data
            const QByteArray data = readBufferTyped<QByteArray>(&propertyLength);
            properties.setCorrelationData(data);
            break;
        }
        case 0x26: { // 3.3.2.3.7 User property
            const QString propertyName = readBufferTyped<QString>(&propertyLength);
            const QString propertyValue = readBufferTyped<QString>(&propertyLength);
            userProperties.append(QMqttStringPair(propertyName, propertyValue));
            break;
        }
        case 0x0b: { // 3.3.2.3.8 Subscription Identifier
            qint32 id = readVariableByteInteger(&propertyLength);
            if (id < 0)
                return; // readVariableByteInteger closes connection
            subscriptionIds.append(quint32(id));
            break;
        }
        case 0x03: { // 3.3.2.3.9 Content Type
            const QString content = readBufferTyped<QString>(&propertyLength);
            properties.setContentType(content);
            break;
        }
        default:
            qCDebug(lcMqttConnection) << "Unknown publish property received.";
            break;
        }
    }
    if (!userProperties.isEmpty())
        properties.setUserProperties(userProperties);

    if (!subscriptionIds.isEmpty())
        properties.setSubscriptionIdentifiers(subscriptionIds);
}

void QMqttConnection::readSubscriptionProperties(QMqttSubscription *sub)
{
    qint64 propertyLength = readVariableByteInteger(&m_missingData);
    m_missingData -= propertyLength;

    while (propertyLength > 0) {
        const quint8 propertyId = readBufferTyped<quint8>(&propertyLength);
        switch (propertyId) {
        case 0x1f: { // 3.9.2.1.2 Reason String
            const QString content = readBufferTyped<QString>(&propertyLength);
            sub->d_func()->m_reasonString = content;
            break;
        }
        case 0x26: { // 3.9.2.1.3
            const QString propertyName = readBufferTyped<QString>(&propertyLength);
            const QString propertyValue = readBufferTyped<QString>(&propertyLength);

            sub->d_func()->m_userProperties.append(QMqttStringPair(propertyName, propertyValue));
            break;
        }
        default:
            qCDebug(lcMqttConnection) << "Unknown subscription property received.";
            break;
        }
    }
}

QByteArray QMqttConnection::writeConnectProperties()
{
    QMqttControlPacket properties;

    // According to MQTT5 3.1.2.11 default values do not need to be included in the
    // connect statement.

    // 3.1.2.11.2
    if (m_clientPrivate->m_connectionProperties.sessionExpiryInterval() != 0) {
        qCDebug(lcMqttConnectionVerbose) << "Connection Properties: specify sessionExpiryInterval";
        properties.append(char(0x11));
        properties.append(m_clientPrivate->m_connectionProperties.sessionExpiryInterval());
    }

    // 3.1.2.11.3
    if (m_clientPrivate->m_connectionProperties.maximumReceive() != 65535) {
        qCDebug(lcMqttConnectionVerbose) << "Connection Properties: specify maximumReceive";
        properties.append(char(0x21));
        properties.append(m_clientPrivate->m_connectionProperties.maximumReceive());
    }

    // 3.1.2.11.4
    if (m_clientPrivate->m_connectionProperties.maximumPacketSize() != std::numeric_limits<quint32>::max()) {
        qCDebug(lcMqttConnectionVerbose) << "Connection Properties: specify maximumPacketSize";
        properties.append(char(0x27));
        properties.append(m_clientPrivate->m_connectionProperties.maximumPacketSize());
    }

    // 3.1.2.11.5
    if (m_clientPrivate->m_connectionProperties.maximumTopicAlias() != 0) {
        qCDebug(lcMqttConnectionVerbose) << "Connection Properties: specify maximumTopicAlias";
        properties.append(char(0x22));
        properties.append(m_clientPrivate->m_connectionProperties.maximumTopicAlias());
    }

    // 3.1.2.11.6
    if (m_clientPrivate->m_connectionProperties.requestResponseInformation()) {
        qCDebug(lcMqttConnectionVerbose) << "Connection Properties: specify requestResponseInformation";
        properties.append(char(0x19));
        properties.append(char(1));
    }

    // 3.1.2.11.7
    if (!m_clientPrivate->m_connectionProperties.requestProblemInformation()) {
        qCDebug(lcMqttConnectionVerbose) << "Connection Properties: specify requestProblemInformation";
        properties.append(char(0x17));
        properties.append(char(0));
    }

    // 3.1.2.11.8 Add User properties
    auto userProperties = m_clientPrivate->m_connectionProperties.userProperties();
    if (!userProperties.isEmpty()) {
        qCDebug(lcMqttConnectionVerbose) << "Connection Properties: specify user properties";
        for (const auto &prop : userProperties) {
            properties.append(char(0x26));
            properties.append(prop.name().toUtf8());
            properties.append(prop.value().toUtf8());
        }
    }

    // 3.1.2.11.9 Add Authentication
    const QString authenticationMethod = m_clientPrivate->m_connectionProperties.authenticationMethod();
    if (!authenticationMethod.isEmpty()) {
        qCDebug(lcMqttConnectionVerbose) << "Connection Properties: specify AuthenticationMethod:";
        qCDebug(lcMqttConnectionVerbose) << "    " << authenticationMethod;
        properties.append(char(0x15));
        properties.append(authenticationMethod.toUtf8());
        // 3.1.2.11.10
        const QByteArray authenticationData = m_clientPrivate->m_connectionProperties.authenticationData();
        if (!authenticationData.isEmpty()) {
            qCDebug(lcMqttConnectionVerbose) << "Connection Properties: Authentication Data:";
            qCDebug(lcMqttConnectionVerbose) << "    " << authenticationData;
            properties.append(char(0x16));
            properties.append(authenticationData);
        }
    }

    return properties.serializePayload();
}

QByteArray QMqttConnection::writeLastWillProperties() const
{
    QMqttControlPacket properties;
    const QMqttLastWillProperties &lastWillProperties = m_clientPrivate->m_lastWillProperties;
    // Will Delay interval 3.1.3.2.2
    if (lastWillProperties.willDelayInterval() > 0) {
        const quint32 delay = lastWillProperties.willDelayInterval();
        qCDebug(lcMqttConnectionVerbose) << "Last Will Properties: specify will delay interval:"
                                         << delay;
        properties.append(char(0x18));
        properties.append(delay);
    }

    // Payload Format Indicator 3.1.3.2.3
    if (lastWillProperties.payloadFormatIndicator() != QMqtt::PayloadFormatIndicator::Unspecified) {
        qCDebug(lcMqttConnectionVerbose) << "Last Will Properties: payload format indicator specified";
        properties.append(char(0x01));
        properties.append(char(0x01)); // UTF8
    }

    // Message Expiry Interval 3.1.3.2.4
    if (lastWillProperties.messageExpiryInterval() > 0) {
        const quint32 interval = lastWillProperties.messageExpiryInterval();
        qCDebug(lcMqttConnectionVerbose) << "Last Will Properties: Message Expiry interval:"
                                         << interval;
        properties.append(char(0x02));
        properties.append(interval);
    }

    // Content Type 3.1.3.2.5
    if (!lastWillProperties.contentType().isEmpty()) {
        qCDebug(lcMqttConnectionVerbose) << "Last Will Properties: Content Type:"
                                         << lastWillProperties.contentType();
        properties.append(char(0x03));
        properties.append(lastWillProperties.contentType().toUtf8());
    }

    // Response Topic 3.1.3.2.6
    if (!lastWillProperties.responseTopic().isEmpty()) {
        qCDebug(lcMqttConnectionVerbose) << "Last Will Properties: Response Topic:"
                                         << lastWillProperties.responseTopic();
        properties.append(char(0x08));
        properties.append(lastWillProperties.responseTopic().toUtf8());
    }

    // Correlation Data 3.1.3.2.7
    if (!lastWillProperties.correlationData().isEmpty()) {
        qCDebug(lcMqttConnectionVerbose) << "Last Will Properties: Correlation Data:"
                                         << lastWillProperties.correlationData();
        properties.append(char(0x09));
        properties.append(lastWillProperties.correlationData());
    }

    // User Properties 3.1.3.2.8
    if (!lastWillProperties.userProperties().isEmpty()) {
        auto userProperties = lastWillProperties.userProperties();
        qCDebug(lcMqttConnectionVerbose) << "Last Will Properties: specify user properties";
        for (const auto &prop : userProperties) {
            properties.append(char(0x26));
            properties.append(prop.name().toUtf8());
            properties.append(prop.value().toUtf8());
        }
    }

    return properties.serializePayload();
}

QByteArray QMqttConnection::writePublishProperties(const QMqttPublishProperties &properties)
{
    QMqttControlPacket packet;

    // 3.3.2.3.2 Payload Indicator
    if (properties.availableProperties() & QMqttPublishProperties::PayloadFormatIndicator &&
            properties.payloadFormatIndicator() != QMqtt::PayloadFormatIndicator::Unspecified) {
        qCDebug(lcMqttConnectionVerbose) << "Publish Properties: Payload Indicator:"
                                         << (properties.payloadFormatIndicator() == QMqtt::PayloadFormatIndicator::UTF8Encoded ? 1 : 0);
        packet.append(char(0x01));
        switch (properties.payloadFormatIndicator()) {
        case QMqtt::PayloadFormatIndicator::UTF8Encoded:
            packet.append(char(0x01));
            break;
        default:
            qCDebug(lcMqttConnection) << "Unknown payload indicator.";
            break;
        }
    }

    // 3.3.2.3.3 Message Expiry
    if (properties.availableProperties() & QMqttPublishProperties::MessageExpiryInterval &&
            properties.messageExpiryInterval() > 0) {
        qCDebug(lcMqttConnectionVerbose) << "Publish Properties: Message Expiry :"
                                         << properties.messageExpiryInterval();
        packet.append(char(0x02));
        packet.append(properties.messageExpiryInterval());
    }

    // 3.3.2.3.4 Topic alias
    if (properties.availableProperties() & QMqttPublishProperties::TopicAlias &&
            properties.topicAlias() > 0) {
        qCDebug(lcMqttConnectionVerbose) << "Publish Properties: Topic Alias :"
                                         << properties.topicAlias();
        if (m_clientPrivate->m_serverConnectionProperties.availableProperties() & QMqttServerConnectionProperties::MaximumTopicAlias
                && properties.topicAlias() > m_clientPrivate->m_serverConnectionProperties.maximumTopicAlias()) {
            qCDebug(lcMqttConnection) <<  "Invalid topic alias specified: " << properties.topicAlias()
                                      << " Maximum by server is:"
                                      << m_clientPrivate->m_serverConnectionProperties.maximumTopicAlias();

        } else {
            packet.append(char(0x23));
            packet.append(properties.topicAlias());
        }
    }

    // 3.3.2.3.5 Response Topic
    if (properties.availableProperties() & QMqttPublishProperties::ResponseTopic &&
            !properties.responseTopic().isEmpty()) {
        qCDebug(lcMqttConnectionVerbose) << "Publish Properties: Response Topic :"
                                         << properties.responseTopic();
        packet.append(char(0x08));
        packet.append(properties.responseTopic().toUtf8());
    }

    // 3.3.2.3.6 Correlation Data
    if (properties.availableProperties() & QMqttPublishProperties::CorrelationData &&
            !properties.correlationData().isEmpty()) {
        qCDebug(lcMqttConnectionVerbose) << "Publish Properties: Correlation Data :"
                                         << properties.correlationData();
        packet.append(char(0x09));
        packet.append(properties.correlationData());
    }

    // 3.3.2.3.7 User Property
    if (properties.availableProperties() & QMqttPublishProperties::UserProperty) {
        auto userProperties = properties.userProperties();
        if (!userProperties.isEmpty()) {
            qCDebug(lcMqttConnectionVerbose) << "Publish Properties: specify user properties";
            for (const auto &prop : userProperties) {
                packet.append(char(0x26));
                packet.append(prop.name().toUtf8());
                packet.append(prop.value().toUtf8());
            }
        }
    }

    // 3.3.2.3.8 Subscription Identifier
    if (properties.availableProperties() & QMqttPublishProperties::SubscriptionIdentifier) {
        for (auto id : properties.subscriptionIdentifiers()) {
            qCDebug(lcMqttConnectionVerbose) << "Publish Properties: Subscription ID:" << id;
            packet.append(char(0x0b));
            packet.appendRawVariableInteger(id);
        }
    }

    // 3.3.2.3.9 Content Type
    if (properties.availableProperties() & QMqttPublishProperties::ContentType &&
            !properties.contentType().isEmpty()) {
        qCDebug(lcMqttConnectionVerbose) << "Publish Properties: Content Type :"
                                         << properties.contentType();
        packet.append(char(0x03));
        packet.append(properties.contentType().toUtf8());
    }

    return packet.serializePayload();
}

QByteArray QMqttConnection::writeSubscriptionProperties(const QMqttSubscriptionProperties &properties)
{
    QMqttControlPacket packet;

    // 3.8.2.1.2 Subscription Identifier
    if (properties.subscriptionIdentifier() > 0) {
        qCDebug(lcMqttConnectionVerbose) << "Subscription Properties: Subscription Identifier";
        packet.append(char(0x0b));
        packet.appendRawVariableInteger(properties.subscriptionIdentifier());
    }

    // 3.8.2.1.3 User Property
    auto userProperties = properties.userProperties();
    if (!userProperties.isEmpty()) {
        qCDebug(lcMqttConnectionVerbose) << "Subscription Properties: specify user properties";
        for (const auto &prop : userProperties) {
            packet.append(char(0x26));
            packet.append(prop.name().toUtf8());
            packet.append(prop.value().toUtf8());
        }
    }

    return packet.serializePayload();
}

QByteArray QMqttConnection::writeUnsubscriptionProperties(const QMqttUnsubscriptionProperties &properties)
{
    QMqttControlPacket packet;

    // 3.10.2.1.2
    auto userProperties = properties.userProperties();
    if (!userProperties.isEmpty()) {
        qCDebug(lcMqttConnectionVerbose) << "Unsubscription Properties: specify user properties";
        for (const auto &prop : userProperties) {
            packet.append(char(0x26));
            packet.append(prop.name().toUtf8());
            packet.append(prop.value().toUtf8());
        }
    }

    return packet.serializePayload();
}

QByteArray QMqttConnection::writeAuthenticationProperties(const QMqttAuthenticationProperties &properties)
{
    QMqttControlPacket packet;

    // 3.15.2.2.2
    if (!properties.authenticationMethod().isEmpty()) {
        packet.append(char(0x15));
        packet.append(properties.authenticationMethod().toUtf8());
    }
    // 3.15.2.2.3
    if (!properties.authenticationData().isEmpty()) {
        packet.append(char(0x16));
        packet.append(properties.authenticationData());
    }

    // 3.15.2.2.4
    if (!properties.reason().isEmpty()) {
        packet.append(char(0x1F));
        packet.append(properties.reason().toUtf8());
    }

    // 3.15.2.2.5
    auto userProperties = properties.userProperties();
    if (!userProperties.isEmpty()) {
        qCDebug(lcMqttConnectionVerbose) << "Unsubscription Properties: specify user properties";
        for (const auto &prop : userProperties) {
            packet.append(char(0x26));
            packet.append(prop.name().toUtf8());
            packet.append(prop.value().toUtf8());
        }
    }

    return packet.serializePayload();
}

void QMqttConnection::finalize_auth()
{
    qCDebug(lcMqttConnectionVerbose) << "Finalize AUTH";

    quint8 authReason = 0;
    QMqttAuthenticationProperties authProperties;
    // 3.15.2.1 - The Reason Code and Property Length can be omitted if the Reason Code
    // is 0x00 (Success) and there are no Properties. In this case the AUTH has a
    // Remaining Length of 0.
    if (m_missingData > 0) {
        authReason = readBufferTyped<quint8>(&m_missingData);
        readAuthProperties(authProperties);
    }

    // 3.15.2.1
    switch (QMqtt::ReasonCode(authReason)) {
    case QMqtt::ReasonCode::Success:
        emit m_clientPrivate->m_client->authenticationFinished(authProperties);
        break;
    case QMqtt::ReasonCode::ContinueAuthentication:
    case QMqtt::ReasonCode::ReAuthenticate:
        emit m_clientPrivate->m_client->authenticationRequested(authProperties);
        break;
    default:
        qCDebug(lcMqttConnection) << "Received illegal AUTH reason code:" << authReason;
        closeConnection(QMqttClient::ProtocolViolation);
        break;
    }
}

void QMqttConnection::finalize_connack()
{
    qCDebug(lcMqttConnectionVerbose) << "Finalize CONNACK";

    const quint8 ackFlags = readBufferTyped<quint8>(&m_missingData);

    if (ackFlags > 1) { // MQTT-3.2.2.1
        qCDebug(lcMqttConnection) << "Unexpected CONNACK Flags specified:" << QString::number(ackFlags);
        readBuffer(quint64(m_missingData));
        m_missingData = 0;
        closeConnection(QMqttClient::ProtocolViolation);
        return;
    }
    bool sessionPresent = ackFlags == 1;

    // MQTT-3.2.2-1 & MQTT-3.2.2-2
    if (sessionPresent) {
        emit m_clientPrivate->m_client->brokerSessionRestored();
        if (m_clientPrivate->m_cleanSession)
            qCDebug(lcMqttConnection) << "Connected with a clean session, ack contains session present.";
    } else {
        // MQTT-4.1.0.-1 MQTT-4.1.0-2 Session not stored on broker side
        // regardless whether cleanSession is false
        cleanSubscriptions();
    }

    quint8 connectResultValue = readBufferTyped<quint8>(&m_missingData);
    QMqttServerConnectionProperties serverProp;
    serverProp.serverData->reasonCode = QMqtt::ReasonCode(connectResultValue);
    m_clientPrivate->m_serverConnectionProperties = serverProp;
    if (connectResultValue != 0 && m_clientPrivate->m_protocolVersion != QMqttClient::MQTT_5_0) {
        qCDebug(lcMqttConnection) << "Connection has been rejected.";
        closeConnection(static_cast<QMqttClient::ClientError>(connectResultValue));
        return;
    }

    // MQTT 5.0 has variable part != 2 in the header
    if (m_clientPrivate->m_protocolVersion == QMqttClient::MQTT_5_0) {
        readConnackProperties(m_clientPrivate->m_serverConnectionProperties);
        m_receiveAliases.resize(m_clientPrivate->m_serverConnectionProperties.maximumTopicAlias());
        m_publishAliases.resize(m_clientPrivate->m_connectionProperties.maximumTopicAlias());

        // 3.2.2.2
        switch (QMqtt::ReasonCode(connectResultValue)) {
        case QMqtt::ReasonCode::Success:
            break;
        case QMqtt::ReasonCode::MalformedPacket:
        case QMqtt::ReasonCode::ProtocolError:
            closeConnection(QMqttClient::ProtocolViolation);
            return;
        case QMqtt::ReasonCode::UnsupportedProtocolVersion:
            closeConnection(QMqttClient::InvalidProtocolVersion);
            return;
        case QMqtt::ReasonCode::InvalidClientId:
            closeConnection(QMqttClient::IdRejected);
            return;
        case QMqtt::ReasonCode::ServerNotAvailable:
        case QMqtt::ReasonCode::ServerBusy:
        case QMqtt::ReasonCode::UseAnotherServer:
        case QMqtt::ReasonCode::ServerMoved:
            closeConnection(QMqttClient::ServerUnavailable);
            return;
        case QMqtt::ReasonCode::InvalidUserNameOrPassword:
            closeConnection(QMqttClient::BadUsernameOrPassword);
            return;
        case QMqtt::ReasonCode::NotAuthorized:
            closeConnection(QMqttClient::NotAuthorized);
            return;
        case QMqtt::ReasonCode::UnspecifiedError:
            closeConnection(QMqttClient::UnknownError);
            return;
        case QMqtt::ReasonCode::ImplementationSpecificError:
        case QMqtt::ReasonCode::ClientBanned:
        case QMqtt::ReasonCode::InvalidAuthenticationMethod:
        case QMqtt::ReasonCode::InvalidTopicName:
        case QMqtt::ReasonCode::PacketTooLarge:
        case QMqtt::ReasonCode::QuotaExceeded:
        case QMqtt::ReasonCode::InvalidPayloadFormat:
        case QMqtt::ReasonCode::RetainNotSupported:
        case QMqtt::ReasonCode::QoSNotSupported:
        case QMqtt::ReasonCode::ExceededConnectionRate:
            closeConnection(QMqttClient::Mqtt5SpecificError);
            return;
        default:
            qCDebug(lcMqttConnection) << "Received illegal CONNACK reason code:" << connectResultValue;
            closeConnection(QMqttClient::ProtocolViolation);
            return;
        }
    }

    m_internalState = BrokerConnected;
    m_clientPrivate->setStateAndError(QMqttClient::Connected);

    if (m_clientPrivate->m_autoKeepAlive)
        m_pingTimer.start(m_clientPrivate->m_keepAlive * 1000, this);
}

void QMqttConnection::finalize_suback()
{
    const quint16 id = readBufferTyped<quint16>(&m_missingData);

    auto sub = m_pendingSubscriptionAck.take(id);
    if (Q_UNLIKELY(sub == nullptr)) {
        qCDebug(lcMqttConnection) << "Received SUBACK for unknown subscription request.";
        return;
    }

    if (m_clientPrivate->m_protocolVersion == QMqttClient::MQTT_5_0)
        readSubscriptionProperties(sub);

    // 3.9.3 - The Payload contains a list of Reason Codes. Each Reason Code corresponds to a Topic Filter in the SUBSCRIBE packet being acknowledged.
    // Whereas 3.8.3 states "The Payload MUST contain at least one Topic Filter and Subscription Options pair. A SUBSCRIBE packet with no Payload is a Protocol Error."
    do {
        quint8 reason = readBufferTyped<quint8>(&m_missingData);

        sub->d_func()->m_reasonCode = QMqtt::ReasonCode(reason);

        // 3.9.3
        switch (QMqtt::ReasonCode(reason)) {
        case QMqtt::ReasonCode::SubscriptionQoSLevel0:
        case QMqtt::ReasonCode::SubscriptionQoSLevel1:
        case QMqtt::ReasonCode::SubscriptionQoSLevel2:
            qCDebug(lcMqttConnectionVerbose) << "Finalize SUBACK: id:" << id << "qos:" << reason;
            // The broker might have a different support level for QoS than what
            // the client requested
            if (reason != sub->qos()) {
                sub->setQos(reason);
                emit sub->qosChanged(reason);
            }
            sub->setState(QMqttSubscription::Subscribed);
            break;
        case QMqtt::ReasonCode::UnspecifiedError:
            qCWarning(lcMqttConnection) << "Subscription for id " << id << " failed. Reason Code:" << reason;
            sub->setState(QMqttSubscription::Error);
            break;
        case QMqtt::ReasonCode::ImplementationSpecificError:
        case QMqtt::ReasonCode::NotAuthorized:
        case QMqtt::ReasonCode::InvalidTopicFilter:
        case QMqtt::ReasonCode::MessageIdInUse:
        case QMqtt::ReasonCode::QuotaExceeded:
        case QMqtt::ReasonCode::SharedSubscriptionsNotSupported:
        case QMqtt::ReasonCode::SubscriptionIdsNotSupported:
        case QMqtt::ReasonCode::WildCardSubscriptionsNotSupported:
            if (m_clientPrivate->m_protocolVersion == QMqttClient::MQTT_5_0) {
                qCWarning(lcMqttConnection) << "Subscription for id " << id << " failed. Reason Code:" << reason;
                sub->setState(QMqttSubscription::Error);
                break;
            }
            Q_FALLTHROUGH();
        default:
            qCWarning(lcMqttConnection) << "Received illegal SUBACK reason code:" << reason;
            closeConnection(QMqttClient::ProtocolViolation);
            break;
        }
    } while (m_missingData > 0);
}

void QMqttConnection::finalize_unsuback()
{
    const quint16 id = readBufferTyped<quint16>(&m_missingData);
    qCDebug(lcMqttConnectionVerbose) << "Finalize UNSUBACK: " << id;

    auto sub = m_pendingUnsubscriptions.take(id);
    if (Q_UNLIKELY(sub == nullptr)) {
        qCDebug(lcMqttConnection) << "Received UNSUBACK for unknown request.";
        return;
    }

    m_activeSubscriptions.remove(sub->topic());

    if (m_clientPrivate->m_protocolVersion == QMqttClient::MQTT_5_0) {
        readSubscriptionProperties(sub);
    } else {
        // 3.11.3 - The UNSUBACK Packet has no payload.
        // emulate successful unsubscription
        sub->d_func()->m_reasonCode = QMqtt::ReasonCode::Success;
        sub->setState(QMqttSubscription::Unsubscribed);
        return;
    }

    // 3.1.3 - The Payload contains a list of Reason Codes. Each Reason Code corresponds to a Topic Filter in the UNSUBSCRIBE packet being acknowledged.
    // Whereas 3.10.3 states "The Payload of an UNSUBSCRIBE packet MUST contain at least one Topic Filter. An UNSUBSCRIBE packet with no Payload is a Protocol Error."
    do {
        const quint8 reasonCode = readBufferTyped<quint8>(&m_missingData);
        sub->d_func()->m_reasonCode = QMqtt::ReasonCode(reasonCode);

        // 3.11.3
        switch (QMqtt::ReasonCode(reasonCode)) {
        case QMqtt::ReasonCode::Success:
            sub->setState(QMqttSubscription::Unsubscribed);
            break;
        case QMqtt::ReasonCode::NoSubscriptionExisted:
        case QMqtt::ReasonCode::ImplementationSpecificError:
        case QMqtt::ReasonCode::NotAuthorized:
        case QMqtt::ReasonCode::InvalidTopicFilter:
        case QMqtt::ReasonCode::MessageIdInUse:
        case QMqtt::ReasonCode::UnspecifiedError:
            qCWarning(lcMqttConnection) << "Unsubscription for id " << id << " failed. Reason Code:" << reasonCode;
            sub->setState(QMqttSubscription::Error);
            break;
        default:
            qCWarning(lcMqttConnection) << "Received illegal UNSUBACK reason code:" << reasonCode;
            closeConnection(QMqttClient::ProtocolViolation);
            break;
        }
    } while (m_missingData > 0);
}

void QMqttConnection::finalize_publish()
{
    // String topic
    QMqttTopicName topic = readBufferTyped<QString>(&m_missingData);
    const int topicLength = topic.name().length();

    quint16 id = 0;
    if (m_currentPublish.qos > 0)
        id = readBufferTyped<quint16>(&m_missingData);

    QMqttPublishProperties publishProperties;
    if (m_clientPrivate->m_protocolVersion == QMqttClient::MQTT_5_0)
        readPublishProperties(publishProperties);

    if (publishProperties.availableProperties() & QMqttPublishProperties::TopicAlias) {
        const quint16 topicAlias = publishProperties.topicAlias();
        if (topicAlias == 0 || topicAlias > m_clientPrivate->m_connectionProperties.maximumTopicAlias()) {
            qCDebug(lcMqttConnection) << "TopicAlias receive: overflow.";
            closeConnection(QMqttClient::ProtocolViolation);
            return;
        }
        if (topicLength == 0) { // New message on existing topic alias
            topic = m_receiveAliases.at(topicAlias - 1);
            if (topic.name().isEmpty()) {
                qCDebug(lcMqttConnection) << "TopicAlias receive: alias for unknown topic.";
                closeConnection(QMqttClient::ProtocolViolation);
                return;
            }
            qCDebug(lcMqttConnectionVerbose) << "TopicAlias receive: Using " << topicAlias;
        } else { // Resetting a topic alias
            qCDebug(lcMqttConnection) << "TopicAlias receive: Resetting:" << topic.name() << " : " << topicAlias;
            m_receiveAliases[topicAlias - 1] = topic;
        }
    }

    // message
    const quint64 payloadLength = quint64(m_missingData);
    const QByteArray message = readBuffer(payloadLength);
    m_missingData -= payloadLength;

    qCDebug(lcMqttConnectionVerbose) << "Finalize PUBLISH: topic:" << topic
                                     << " payloadLength:" << payloadLength;;

    emit m_clientPrivate->m_client->messageReceived(message, topic);

    QMqttMessage qmsg(topic, message, id, m_currentPublish.qos,
                      m_currentPublish.dup, m_currentPublish.retain);
    qmsg.d->m_publishProperties = publishProperties;

    if (id != 0) {
        QMqttMessageStatusProperties statusProp;
        statusProp.data->userProperties = publishProperties.userProperties();
        emit m_clientPrivate->m_client->messageStatusChanged(id, QMqtt::MessageStatus::Published, statusProp);
    }

    for (auto sub = m_activeSubscriptions.constBegin(); sub != m_activeSubscriptions.constEnd(); sub++) {
        if (sub.key().match(topic))
            emit sub.value()->messageReceived(qmsg);
    }

    if (m_currentPublish.qos == 1)
        sendControlPublishAcknowledge(id);
    else if (m_currentPublish.qos == 2)
        sendControlPublishReceive(id);
}

void QMqttConnection::finalize_pubAckRecRelComp()
{
    qCDebug(lcMqttConnectionVerbose) << "Finalize PUBACK/REC/REL/COMP";
    const quint16 id = readBufferTyped<quint16>(&m_missingData);

    QMqttMessageStatusProperties properties;
    if (m_clientPrivate->m_protocolVersion == QMqttClient::MQTT_5_0 && m_missingData > 0) {
        // Reason Code (1byte)
        const quint8 reasonCode = readBufferTyped<quint8>(&m_missingData);
        properties.data->reasonCode = QMqtt::ReasonCode(reasonCode);

        if ((m_currentPacket & 0xF0) == QMqttControlPacket::PUBACK || (m_currentPacket & 0xF0) == QMqttControlPacket::PUBREC) {
            // 3.4.2.1, 3.5.2.1
            switch (QMqtt::ReasonCode(reasonCode)) {
            case QMqtt::ReasonCode::Success:
            case QMqtt::ReasonCode::NoMatchingSubscriber:
            case QMqtt::ReasonCode::UnspecifiedError:
            case QMqtt::ReasonCode::ImplementationSpecificError:
            case QMqtt::ReasonCode::NotAuthorized:
            case QMqtt::ReasonCode::InvalidTopicName:
            case QMqtt::ReasonCode::MessageIdInUse:
            case QMqtt::ReasonCode::QuotaExceeded:
            case QMqtt::ReasonCode::InvalidPayloadFormat:
                break;
            default:
                qCWarning(lcMqttConnection) << "Received illegal PUBACK/REC reason code:" << reasonCode;
                closeConnection(QMqttClient::ProtocolViolation);
                return;
            }
        } else {
            // 3.6.2.1, 3.7.2.1
            switch (QMqtt::ReasonCode(reasonCode)) {
            case QMqtt::ReasonCode::Success:
            case QMqtt::ReasonCode::MessageIdNotFound:
                break;
            default:
                qCWarning(lcMqttConnection) << "Received illegal PUBREL/COMP reason code:" << reasonCode;
                closeConnection(QMqttClient::ProtocolViolation);
                return;
            }
        }

        readMessageStatusProperties(properties);
    }

    if ((m_currentPacket & 0xF0) == QMqttControlPacket::PUBREL) {
        qCDebug(lcMqttConnectionVerbose) << " PUBREL:" << id;
        emit m_clientPrivate->m_client->messageStatusChanged(id, QMqtt::MessageStatus::Released, properties);
        sendControlPublishComp(id);
        return;
    }

    if ((m_currentPacket & 0xF0) == QMqttControlPacket::PUBCOMP) {
        qCDebug(lcMqttConnectionVerbose) << " PUBCOMP:" << id;
        auto pendingRelease = m_pendingReleaseMessages.take(id);
        if (!pendingRelease)
            qCDebug(lcMqttConnection) << "Received PUBCOMP for unknown released message.";
        emit m_clientPrivate->m_client->messageStatusChanged(id, QMqtt::MessageStatus::Completed, properties);
        emit m_clientPrivate->m_client->messageSent(id);
        return;
    }

    auto pendingMsg = m_pendingMessages.take(id);
    if (!pendingMsg) {
        qCDebug(lcMqttConnection) << "Received PUBACK for unknown message: " << id;
        return;
    }
    if ((m_currentPacket & 0xF0) == QMqttControlPacket::PUBREC) {
        qCDebug(lcMqttConnectionVerbose) << " PUBREC:" << id;
        m_pendingReleaseMessages.insert(id, pendingMsg);
        emit m_clientPrivate->m_client->messageStatusChanged(id, QMqtt::MessageStatus::Received, properties);
        sendControlPublishRelease(id);
    } else {
        qCDebug(lcMqttConnectionVerbose) << " PUBACK:" << id;
        emit m_clientPrivate->m_client->messageStatusChanged(id, QMqtt::MessageStatus::Acknowledged, properties);
        emit m_clientPrivate->m_client->messageSent(id);
    }
}

void QMqttConnection::finalize_pingresp()
{
    qCDebug(lcMqttConnectionVerbose) << "Finalize PINGRESP";
    const quint8 v = readBufferTyped<quint8>(&m_missingData);

    if (v != 0) {
        qCDebug(lcMqttConnection) << "Received a PINGRESP including payload.";
        closeConnection(QMqttClient::ProtocolViolation);
        return;
    }
    m_pingTimeout--;
    emit m_clientPrivate->m_client->pingResponseReceived();
}

bool QMqttConnection::processDataHelper()
{
    if (m_missingData > 0) {
        if ((m_readBuffer.size() - m_readPosition) < m_missingData)
            return false;

        switch (m_currentPacket & 0xF0) {
        case QMqttControlPacket::AUTH:
            finalize_auth();
            break;
        case QMqttControlPacket::CONNACK:
            finalize_connack();
            break;
        case QMqttControlPacket::SUBACK:
            finalize_suback();
            break;
        case QMqttControlPacket::UNSUBACK:
            finalize_unsuback();
            break;
        case QMqttControlPacket::PUBLISH:
            finalize_publish();
            break;
        case QMqttControlPacket::PUBACK:
        case QMqttControlPacket::PUBREC:
        case QMqttControlPacket::PUBREL:
        case QMqttControlPacket::PUBCOMP:
            finalize_pubAckRecRelComp();
            break;
        case QMqttControlPacket::PINGRESP:
            finalize_pingresp();
            break;
        default:
            qCDebug(lcMqttConnection) << "Unknown packet to finalize.";
            closeConnection(QMqttClient::ProtocolViolation);
            break;
        }

        if (m_internalState == BrokerDisconnected)
            return false;

        Q_ASSERT(m_missingData == 0);

        m_readBuffer = m_readBuffer.mid(m_readPosition);
        m_readPosition = 0;
    }

    // MQTT-2.2 A fixed header of a control packet must be at least 2 bytes. If the payload is
    // longer than 127 bytes the header can be up to 5 bytes long.
    switch (m_readBuffer.size()) {
    case 0:
    case 1:
        return false;
    case 2:
        if ((m_readBuffer.at(1) & 128) != 0)
            return false;
        break;
    case 3:
        if ((m_readBuffer.at(1) & 128) != 0 && (m_readBuffer.at(2) & 128) != 0)
            return false;
        break;
    case 4:
        if ((m_readBuffer.at(1) & 128) != 0 && (m_readBuffer.at(2) & 128) != 0 && (m_readBuffer.at(3) & 128) != 0)
            return false;
        break;
    default:
        break;
    }

    readBuffer(reinterpret_cast<char *>(&m_currentPacket), 1);

    switch (m_currentPacket & 0xF0) {
    case QMqttControlPacket::CONNACK: {
        qCDebug(lcMqttConnectionVerbose) << "Received CONNACK";
        if (m_internalState != BrokerWaitForConnectAck) {
            qCDebug(lcMqttConnection) << "Received CONNACK at unexpected time.";
            closeConnection(QMqttClient::ProtocolViolation);
            return false;
        }

        qint32 payloadSize = readVariableByteInteger();
        if (m_clientPrivate->m_protocolVersion != QMqttClient::MQTT_5_0) {
            if (payloadSize != 2) {
                qCDebug(lcMqttConnection) << "Unexpected FRAME size in CONNACK.";
                closeConnection(QMqttClient::ProtocolViolation);
                return false;
            }
        }
        m_missingData = payloadSize;
        break;
    }
    case QMqttControlPacket::SUBACK: {
        qCDebug(lcMqttConnectionVerbose) << "Received SUBACK";
        const quint8 remaining = readBufferTyped<quint8>();
        m_missingData = remaining;
        break;
    }
    case QMqttControlPacket::PUBLISH: {
        qCDebug(lcMqttConnectionVerbose) << "Received PUBLISH";
        m_currentPublish.dup = m_currentPacket & 0x08;
        m_currentPublish.qos = (m_currentPacket & 0x06) >> 1;
        m_currentPublish.retain = m_currentPacket & 0x01;
        if ((m_currentPublish.qos == 0 && m_currentPublish.dup != 0)
            || m_currentPublish.qos > 2) {
            closeConnection(QMqttClient::ProtocolViolation);
            return false;
        }

        m_missingData = readVariableByteInteger();
        if (m_missingData == -1)
            return false; // Connection closed inside readVariableByteInteger
        break;
    }
    case QMqttControlPacket::PINGRESP:
        qCDebug(lcMqttConnectionVerbose) << "Received PINGRESP";
        m_missingData = 1;
        break;


    case QMqttControlPacket::PUBREL: {
        qCDebug(lcMqttConnectionVerbose) << "Received PUBREL";
        const quint8 remaining = readBufferTyped<quint8>();
        if (remaining != 0x02) {
            qCDebug(lcMqttConnection) << "Received 2 byte message with invalid remaining length.";
            closeConnection(QMqttClient::ProtocolViolation);
            return false;
        }
        if ((m_currentPacket & 0x0F) != 0x02) {
            qCDebug(lcMqttConnection) << "Malformed fixed header for PUBREL.";
            closeConnection(QMqttClient::ProtocolViolation);
            return false;
        }
        m_missingData = 2;
        break;
    }

    case QMqttControlPacket::UNSUBACK:
    case QMqttControlPacket::PUBACK:
    case QMqttControlPacket::PUBREC:
    case QMqttControlPacket::PUBCOMP: {
        qCDebug(lcMqttConnectionVerbose) << "Received UNSUBACK/PUBACK/PUBREC/PUBCOMP";
        if ((m_currentPacket & 0x0F) != 0) {
            qCDebug(lcMqttConnection) << "Malformed fixed header for UNSUBACK/PUBACK/PUBREC/PUBCOMP.";
            closeConnection(QMqttClient::ProtocolViolation);
            return false;
        }
        const quint8 remaining = readBufferTyped<quint8>();
        if (m_clientPrivate->m_protocolVersion != QMqttClient::MQTT_5_0 && remaining != 0x02) {
            qCDebug(lcMqttConnection) << "Received 2 byte message with invalid remaining length.";
            closeConnection(QMqttClient::ProtocolViolation);
            return false;
        }
        m_missingData = remaining;
        break;
    }
    case QMqttControlPacket::AUTH:
        if (m_clientPrivate->m_protocolVersion != QMqttClient::MQTT_5_0) {
            qCDebug(lcMqttConnection) << "Received unknown command.";
            closeConnection(QMqttClient::ProtocolViolation);
            return false;
        }
        qCDebug(lcMqttConnectionVerbose) << "Received AUTH";
        if ((m_currentPacket & 0x0F) != 0) {
            qCDebug(lcMqttConnection) << "Malformed fixed header for AUTH.";
            closeConnection(QMqttClient::ProtocolViolation);
            return false;
        }
        m_missingData = readVariableByteInteger();
        if (m_missingData == -1)
            return false; // Connection closed inside readVariableByteInteger
        break;
    case QMqttControlPacket::DISCONNECT:
        if (m_clientPrivate->m_protocolVersion != QMqttClient::MQTT_5_0) {
            qCDebug(lcMqttConnection) << "Received unknown command.";
            closeConnection(QMqttClient::ProtocolViolation);
            return false;
        }
        qCDebug(lcMqttConnectionVerbose) << "Received DISCONNECT";
        if ((m_currentPacket & 0x0F) != 0) {
            qCDebug(lcMqttConnection) << "Malformed fixed header for DISCONNECT.";
            closeConnection(QMqttClient::ProtocolViolation);
            return false;
        }
        if (m_internalState != BrokerConnected) {
            qCDebug(lcMqttConnection) << "Received DISCONNECT at unexpected time.";
            closeConnection(QMqttClient::ProtocolViolation);
            return false;
        }
        closeConnection(QMqttClient::NoError);
        return false;
    default:
        qCDebug(lcMqttConnection) << "Received unknown command.";
        closeConnection(QMqttClient::ProtocolViolation);
        return false;
    }

    /* set current command CONNACK - PINGRESP */
    /* read command size */
    /* calculate missing_data */
    return true; // reiterate. implicitly finishes and enqueues
}

void QMqttConnection::processData()
{
    while (processDataHelper())
        ;
}

bool QMqttConnection::writePacketToTransport(const QMqttControlPacket &p)
{
    const QByteArray writeData = p.serialize();
    qCDebug(lcMqttConnectionVerbose) << Q_FUNC_INFO << " DataSize:" << writeData.size();
    const qint64 res = m_transport->write(writeData.constData(), writeData.size());
    if (Q_UNLIKELY(res == -1)) {
        qCDebug(lcMqttConnection) << "Could not write frame to transport.";
        return false;
    }
    return true;
}

QT_END_NAMESPACE
