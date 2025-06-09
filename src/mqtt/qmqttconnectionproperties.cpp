/******************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
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

#include "qmqttconnectionproperties.h"

#include "qmqttconnectionproperties_p.h"

#include <QtCore/QLoggingCategory>

#include <limits>

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(lcMqttConnection)

/*!
    \class QMqttConnectionProperties

    \inmodule QtMqtt
    \since 5.12

    \brief The QMqttConnectionProperties class represents configuration
    options a QMqttClient can pass to the server when invoking
    QMqttClient::connectToHost().

    \note Connection properties are part of the MQTT 5.0 specification and
    cannot be used when connecting with a lower protocol level. See
    QMqttClient::ProtocolVersion for more information.
*/

/*!
    \class QMqttServerConnectionProperties

    \inmodule QtMqtt
    \since 5.12

    \brief The QMqttServerConnectionProperties class represents configuration
    options of a server a QMqttClient is connected to.

    When a connection has been established the server might send additional
    details about the connection properties. Use availableProperties() to
    identify properties set by the server. If a property is not set by the
    server, default values are assumed and can be obtained by invoking access
    functions of this instance.

    \note Connection properties are part of the MQTT 5.0 specification and
    cannot be used when connecting with a lower protocol level. See
    QMqttClient::ProtocolVersion for more information.
*/

/*!
    \class QMqttLastWillProperties

    \inmodule QtMqtt
    \since 5.12

    \brief The QMqttLastWillProperties class represents configuration
    options a QMqttClient can pass to the server when specifying the last will
    during connecting to a server.

    \note Last Will properties are part of the MQTT 5.0 specification and
    cannot be used when connecting with a lower protocol level. See
    QMqttClient::ProtocolVersion for more information.
*/

/*!
    \enum QMqttServerConnectionProperties::ServerPropertyDetail

    This enum type specifies the available properties set by the
    server or the client after establishing a connection.

    \value None
           No property has been specified.
    \value SessionExpiryInterval
           The number of seconds the server keeps the session after
           a disconnect.
    \value MaximumReceive
           The maximum number of QoS 1 and 2 message the server is
           capable of managing concurrently.
    \value MaximumQoS
           The maximum QoS level the server can understand.
    \value RetainAvailable
           Specifies whether retained messages are supported.
    \value MaximumPacketSize
           Specifies the maximum packet size including the message header
           and properties.
    \value AssignedClientId
           Specifies whether the server assigned a client identifier.
    \value MaximumTopicAlias
           Specifies the maximum amount of topic aliases.
    \value ReasonString
           Specifies a string providing more details on connection state.
    \value UserProperty
           Specifies additional user properties.
    \value WildCardSupported
           Specifies whether the server supports wildcard subscriptions.
    \value SubscriptionIdentifierSupport
           Specifies whether the server supports subscription identifiers.
    \value SharedSubscriptionSupport
           Specifies whether the server supports shared subscriptions.
    \value ServerKeepAlive
           Specifies the number of seconds the server expects a keep alive
           packet from the client.
    \value ResponseInformation
           Specifies the response information.
    \value ServerReference
           Specifies an alternative server address for the client to
           connect to.
    \value AuthenticationMethod
           Specifies the authentication method.
    \value AuthenticationData
           Specifies the authentication data.
*/

/*!
    \internal
*/
QMqttLastWillProperties::QMqttLastWillProperties() : data(new QMqttLastWillPropertiesData)
{
}

/*!
    \internal
*/
QMqttLastWillProperties::QMqttLastWillProperties(const QMqttLastWillProperties &) = default;

QMqttLastWillProperties &QMqttLastWillProperties::operator=(const QMqttLastWillProperties &rhs)
{
    if (this != &rhs)
        data.operator=(rhs.data);
    return *this;
}

QMqttLastWillProperties::~QMqttLastWillProperties() = default;

/*!
    Returns the delay in seconds a last will message will be sent after
    disconnecting from the server.
*/
quint32 QMqttLastWillProperties::willDelayInterval() const
{
    return data->willDelayInterval;
}

/*!
    Returns the payload format indicator.
*/
QMqtt::PayloadFormatIndicator QMqttLastWillProperties::payloadFormatIndicator() const
{
    return data->formatIndicator;
}

/*!
    Returns the lifetime of the last will message in seconds, starting from
    the will delay interval.
*/
quint32 QMqttLastWillProperties::messageExpiryInterval() const
{
    return data->messageExpiryInterval;
}

/*!
    Returns the content type of the last will message.
*/
QString QMqttLastWillProperties::contentType() const
{
    return data->contentType;
}

/*!
    Returns the topic that subscribers to the last will message should respond
    to.
*/
QString QMqttLastWillProperties::responseTopic() const
{
    return data->responseTopic;
}

/*!
    Returns the correlation data to identify the request.
*/
QByteArray QMqttLastWillProperties::correlationData() const
{
    return data->correlationData;
}

/*!
    Returns the user properties.
*/
QMqttUserProperties QMqttLastWillProperties::userProperties() const
{
    return data->userProperties;
}

/*!
    Sets the will delay interval to \a delay.
*/
void QMqttLastWillProperties::setWillDelayInterval(quint32 delay)
{
    data->willDelayInterval = delay;
}

/*!
    Sets the payload format indicator to \a p.
*/
void QMqttLastWillProperties::setPayloadFormatIndicator(QMqtt::PayloadFormatIndicator p)
{
    data->formatIndicator = p;
}

/*!
    Sets the message expiry interval to \a expiry.
*/
void QMqttLastWillProperties::setMessageExpiryInterval(quint32 expiry)
{
    data->messageExpiryInterval = expiry;
}

/*!
    Sets the content type to \a content.
*/
void QMqttLastWillProperties::setContentType(const QString &content)
{
    data->contentType = content;
}

/*!
    Sets the response topic to \a response.
*/
void QMqttLastWillProperties::setResponseTopic(const QString &response)
{
    data->responseTopic = response;
}

/*!
    Sets the correlation data to \a correlation.
*/
void QMqttLastWillProperties::setCorrelationData(const QByteArray &correlation)
{
    data->correlationData = correlation;
}

/*!
    Sets the user properties to \a properties.
*/
void QMqttLastWillProperties::setUserProperties(const QMqttUserProperties &properties)
{
    data->userProperties = properties;
}

/*!
    \internal
*/
QMqttConnectionProperties::QMqttConnectionProperties() : data(new QMqttConnectionPropertiesData)
{

}

/*!
    \internal
*/
QMqttConnectionProperties::QMqttConnectionProperties(const QMqttConnectionProperties &) = default;

QMqttConnectionProperties &QMqttConnectionProperties::operator=(const QMqttConnectionProperties &rhs)
{
    if (this != &rhs)
        data.operator=(rhs.data);
    return *this;
}

QMqttConnectionProperties::~QMqttConnectionProperties() = default;

/*!
    Sets the session expiry interval to \a expiry. The session expiry interval
    specifies the number of seconds a server holds information on the client
    state after a connection has been closed.

    The default value is 0, which specifies that the session is closed when
    the network connection ends. If the value is specified as maximum of
    quint32, then the session does not expire.
*/
void QMqttConnectionProperties::setSessionExpiryInterval(quint32 expiry)
{
    data->sessionExpiryInterval = expiry;
}

/*!
    Sets the maximum amount of QoS 1 and QoS 2 publications
    that the client is willing to process concurrently for this session
    to \a maximumReceive.

    A maximum receive value of 0 is not allowed.
*/
void QMqttConnectionProperties::setMaximumReceive(quint16 maximumReceive)
{
    if (maximumReceive == 0) {
        qCDebug(lcMqttConnection) << "Maximum Receive is not allowed to be 0.";
        return;
    }
    data->maximumReceive = maximumReceive;
}

/*!
    Sets the maximum packet size to \a packetSize. The maximum packet size
    specifies the maximum size one packet can contain. This includes the
    packet header and its properties.

    If no maximum packet size is specified, no limit is imposed beyond the
    limitations of the protocol itself.
*/
void QMqttConnectionProperties::setMaximumPacketSize(quint32 packetSize)
{
    if (packetSize == 0) {
        qCDebug(lcMqttConnection) << "Packet size is not allowed to be 0.";
        return;
    }
    data->maximumPacketSize = packetSize;
}

/*!
    Sets the maximum topic alias to \a alias. The maximum topic alias specifies
    the highest value that the client will accept from the server. The client
    uses this value to limit the number of topic aliases it is willing to hold
    for the connection.

    The default value is 0. 0 indicates that the client does not accept any
    topic aliases on this connection.
*/
void QMqttConnectionProperties::setMaximumTopicAlias(quint16 alias)
{
    data->maximumTopicAlias = alias;
}

/*!
    Sets the request response information to \a response. A client uses this
    to request the server to return response information after the connection
    request has been handled.

    The default value is \c false, which indicates that the client must not
    return any response information. If the value is \c true, the server
    may return response information, but is not enforced to do so.
*/
void QMqttConnectionProperties::setRequestResponseInformation(bool response)
{
    data->requestResponseInformation = response;
}

/*!
    Sets the request problem information to \a problem. A client uses this
    to request the server to return additional information in case of failure.
    Types of failure include connection and message management on the server
    side.

    The default value is \c false, which indicates that the client must not
    receive any problem information for anything but connection management.
    The server still may send problem information for connection handling.
    If the value is \c true, the server may return problem information.

    Problem information is available in user properties or reason strings
    of the property classes.
*/
void QMqttConnectionProperties::setRequestProblemInformation(bool problem)
{
    data->requestProblemInformation = problem;
}

/*!
    Sets the user properties of the connection to \a properties.

    The default value is to not send any user information.
*/
void QMqttConnectionProperties::setUserProperties(const QMqttUserProperties &properties)
{
    data->userProperties = properties;
}

/*!
    Sets the authentication method to \a authMethod.

    \sa authenticationData()
*/
void QMqttConnectionProperties::setAuthenticationMethod(const QString &authMethod)
{
    data->authenticationMethod = authMethod;
}

/*!
    Sets the authentication data to \a authData.

    Authentication data can only be used if an authentication method has
    been specified.

    \sa authenticationMethod()
*/
void QMqttConnectionProperties::setAuthenticationData(const QByteArray &authData)
{
    data->authenticationData = authData;
}

/*!
    Returns the session expiry interval.
*/
quint32 QMqttConnectionProperties::sessionExpiryInterval() const
{
    return data->sessionExpiryInterval;
}

/*!
    Returns the maximum amount of QoS 1 and QoS 2 publications
    that the client (when obtained from \l QMqttClient::connectionProperties())
    or the server (when obtained from \l QMqttClient::serverConnectionProperties())
    is willing to process concurrently for this session.
*/
quint16 QMqttConnectionProperties::maximumReceive() const
{
    return data->maximumReceive;
}

/*!
    Returns the maximum packet size the client can receive.
*/
quint32 QMqttConnectionProperties::maximumPacketSize() const
{
    return data->maximumPacketSize;
}

/*!
    Returns the maximum topic alias ID the client can use.
*/
quint16 QMqttConnectionProperties::maximumTopicAlias() const
{
    return data->maximumTopicAlias;
}

/*!
    Returns whether the client should receive response information.
*/
bool QMqttConnectionProperties::requestResponseInformation() const
{
    return data->requestResponseInformation;
}

/*!
    Returns whether the client should receive problem information.
*/
bool QMqttConnectionProperties::requestProblemInformation() const
{
    return data->requestProblemInformation;
}

/*!
    Returns the user properties for the connection.
*/
QMqttUserProperties QMqttConnectionProperties::userProperties() const
{
    return data->userProperties;
}

/*!
    Returns the authentication method.
*/
QString QMqttConnectionProperties::authenticationMethod() const
{
    return data->authenticationMethod;
}

/*!
    Returns the authentication data.
*/
QByteArray QMqttConnectionProperties::authenticationData() const
{
    return data->authenticationData;
}

/*!
    \internal
*/
QMqttServerConnectionProperties::QMqttServerConnectionProperties()
    : QMqttConnectionProperties()
    , serverData(new QMqttServerConnectionPropertiesData)
{

}

/*!
    \internal
*/
QMqttServerConnectionProperties::QMqttServerConnectionProperties(const QMqttServerConnectionProperties &rhs)
    : QMqttConnectionProperties(rhs)
    , serverData(rhs.serverData)
{

}

QMqttServerConnectionProperties &QMqttServerConnectionProperties::operator=(const QMqttServerConnectionProperties &rhs)
{
    if (this != &rhs) {
        serverData.operator=(rhs.serverData);
        QMqttConnectionProperties::operator=(rhs);
    }
    return *this;
}

QMqttServerConnectionProperties::~QMqttServerConnectionProperties() = default;

/*!
    Returns the available properties specified by the server.
*/
QMqttServerConnectionProperties::ServerPropertyDetails QMqttServerConnectionProperties::availableProperties() const
{
    return serverData->details;
}

/*!
    Returns \c true if the server provided properties as part of the connection
    aknowledgment. Returns \c false if no properties have been provided.
*/
bool QMqttServerConnectionProperties::isValid() const
{
    return serverData->valid;
}

/*!
    Returns the maximum QoS level the server supports for publishing messages.
    Publishing messages with QoS level exceeding the maximum QoS level reported by the server
    is a protocol violation.

    If the client does not need to support QoS 1 or QoS 2, it should restrict the maximum QoS level
    in any subscription it does to a value it can support; the server would then publish messages
    with the maximum of supported and restricted QoS levels.

    The default value is \c 2.

    \sa QMqttClient::publish(), QMqttClient::subscribe()
*/
quint8 QMqttServerConnectionProperties::maximumQoS() const
{
    return serverData->maximumQoS;
}

/*!
    Returns \c true if the server accepts retained messages.
    The default value is \c true.
*/
bool QMqttServerConnectionProperties::retainAvailable() const
{
    return serverData->retainAvailable;
}

/*!
    Returns \c true if the server assigned a new client identifier to
    the client.

    \sa QMqttClient::clientId()
*/
bool QMqttServerConnectionProperties::clientIdAssigned() const
{
    return serverData->details & QMqttServerConnectionProperties::AssignedClientId;
}

/*!
    Returns the reason string associated with this response.
*/
QString QMqttServerConnectionProperties::reason() const
{
    return serverData->reasonString;
}

/*!
    Returns the reason code associated with this response.
*/
QMqtt::ReasonCode QMqttServerConnectionProperties::reasonCode() const
{
    return serverData->reasonCode;
}

/*!
    Returns \c true if the server accepts subscriptions including wildcards.
    The default value is \c true.
*/
bool QMqttServerConnectionProperties::wildcardSupported() const
{
    return serverData->wildcardSupported;
}

/*!
    Returns \c true if the server accepts subscription identifiers.
    Subscription identifiers can be passed to the server when creating
    a new subscription.

    The default value is \c true.

    \sa QMqttSubscriptionProperties::setSubscriptionIdentifier()
*/
bool QMqttServerConnectionProperties::subscriptionIdentifierSupported() const
{
    return serverData->subscriptionIdentifierSupported;
}

/*!
    Returns \c true if the server accepts shared subscriptions.
    The default value is \c true.
*/
bool QMqttServerConnectionProperties::sharedSubscriptionSupported() const
{
    return serverData->sharedSubscriptionSupported;
}


/*!
    Returns the number of seconds the server requested as keep alive. This
    overwrites the keep alive being set from the client side.

    \sa QMqttClient::setKeepAlive()
*/
quint16 QMqttServerConnectionProperties::serverKeepAlive() const
{
    return serverData->serverKeepAlive;
}

/*!
    Returns the response information.
*/
QString QMqttServerConnectionProperties::responseInformation() const
{
    return serverData->responseInformation;
}

/*!
    Returns a server address which can be used by the client alternatively
    to connect to. Typically, this is used together with the reason
    code \c 0x9c (Use another server) or \c 0x9c (Server moved).
*/
QString QMqttServerConnectionProperties::serverReference() const
{
    return serverData->serverReference;
}

QT_END_NAMESPACE
