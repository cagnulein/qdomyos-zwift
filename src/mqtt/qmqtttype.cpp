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

#include "qmqtttype.h"

QT_BEGIN_NAMESPACE

/*!
    \class QMqttStringPair

    \inmodule QtMqtt
    \since 5.12

    \brief The QMqttStringPair class represents the string pair data type
    of the MQTT 5.0 standard.

    This data type is used to hold a name-value pair.
*/

/*!
    \class QMqttUserProperties

    \inmodule QtMqtt
    \since 5.12

    \brief The QMqttUserProperties class represents a list of QMqttStringPair
    values which can be passed to the server.

    User properties allow a client to pass additional data to the server or
    other subscribers, which do not belong to the message payload.

    On the other hand it also provides a server the flexibility to provide
    further information to connected clients.

    The server might add additional user properties to the ones provided.
    However, the order of the provided properties is not changed during transfer.

    \note User properties are part of the MQTT 5.0 specification and
    cannot be used when using QtMqtt with an older protocol level. See
    QMqttClient::ProtocolVersion for more information.

    \sa QMqttAuthenticationProperties, QMqttConnectionProperties,
    QMqttLastWillProperties, QMqttPublishProperties, QMqttSubscriptionProperties,
    QMqttUnsubscriptionProperties, QMqttSubscription
*/

/*!
    \namespace QMqtt
    \inmodule QtMqtt
    \since 5.12

    \brief Contains miscellaneous identifiers used throughout the Qt MQTT module.
*/

/*!
    \enum QMqtt::PayloadFormatIndicator
    \since 5.12

    The payload format provides information on the content of a message.
    This can help other clients to handle the message faster.

    \value Unspecified
           The format is not specified.
    \value UTF8Encoded
           The payload of the message is formatted as UTF-8 Encoded
           Character Data.
*/

/*!
    \enum QMqtt::MessageStatus
    \since 5.12

    This enum type specifies the available states of a message. Depending
    on the QoS and role of the client, different message statuses are
    expected.

    \value Unknown
           The message status is unknown.
    \value Published
           The client received a message for one of its subscriptions. This
           applies to QoS levels 1 and 2.
    \value Acknowledged
           A message has been acknowledged. This applies to QoS 1 and states
           that the message handling has been finished from the client side.
    \value Received
           A message has been received. This applies to QoS 2.
    \value Released
           A message has been released. This applies to QoS 2. For a publisher
           the message handling has been finished.
    \value Completed
           A message has been completed. This applies to QoS 2 and states
           that the message handling has been finished from the client side.
*/

/*!
    \enum QMqtt::ReasonCode
    \since 5.12

    This enum type specifies the available error codes.

    \value Success
           The specified action has succeeded.
    \value SubscriptionQoSLevel0
           A subscription with QoS level 0 has been created.
    \value SubscriptionQoSLevel1
           A subscription with QoS level 1 has been created.
    \value SubscriptionQoSLevel2
           A subscription with QoS level 2 has been created.
    \value NoMatchingSubscriber
           The message has been accepted by the server, but there are no
           subscribers to receive this message. A broker may send this
           reason code instead of \l Success.
    \value [since 5.15] NoSubscriptionExisted
           No matching Topic Filter is being used by the Client.
    \value [since 5.15] ContinueAuthentication
           Continue the authentication with another step.
    \value [since 5.15] ReAuthenticate
           Initiate a re-authentication.
    \value UnspecifiedError
           An unspecified error occurred.
    \value MalformedPacket
           The packet sent to the server is invalid.
    \value ProtocolError
           A protocol error has occurred. In most cases, this will cause
           the server to disconnect the client.
    \value ImplementationSpecificError
           The packet is valid, but the recipient rejects it.
    \value UnsupportedProtocolVersion
           The requested protocol version is not supported by the server.
    \value InvalidClientId
           The client ID is invalid.
    \value InvalidUserNameOrPassword
           The username or password specified is invalid.
    \value NotAuthorized
           The client is not authorized for the specified action.
    \value ServerNotAvailable
           The server to connect to is not available.
    \value ServerBusy
           The server to connect to is not available. The client is asked to
           try at a later time.
    \value ClientBanned
           The client has been banned from the server.
    \value InvalidAuthenticationMethod
           The authentication method specified is invalid.
    \value InvalidTopicFilter
           The topic filter specified is invalid.
    \value InvalidTopicName
           The topic name specified is invalid.
    \value MessageIdInUse
           The message ID used in the previous packet is already in use.
    \value MessageIdNotFound
           The message ID used in the previous packet has not been found.
    \value PacketTooLarge
           The packet received is too large. See also
           \l QMqttServerConnectionProperties::maximumPacketSize().
    \value QuotaExceeded
           An administratively imposed limit has been exceeded.
    \value InvalidPayloadFormat
           The payload format is invalid.
           See also \l QMqttPublishProperties::payloadFormatIndicator().
    \value RetainNotSupported
           The server does not support retained messages.
           See also \l QMqttServerConnectionProperties::retainAvailable().
    \value QoSNotSupported
           The QoS level requested is not supported.
           See also \l QMqttServerConnectionProperties::maximumQoS().
    \value UseAnotherServer
           The server the client tries to connect to is not available. See also
           \l QMqttServerConnectionProperties::serverReference().
    \value ServerMoved
           The server the client tries to connect to has moved to a new address.
           See also \l QMqttServerConnectionProperties::serverReference().
    \value SharedSubscriptionsNotSupported
           Shared subscriptions are not supported.
           See also \l QMqttServerConnectionProperties::sharedSubscriptionSupported().
    \value ExceededConnectionRate
           The connection rate limit has been exceeded.
    \value SubscriptionIdsNotSupported
           Subscription IDs are not supported.
           See also \l QMqttServerConnectionProperties::subscriptionIdentifierSupported().
    \value WildCardSubscriptionsNotSupported
           Subscriptions using wildcards are not supported by the server.
           See also \l QMqttServerConnectionProperties::wildcardSupported().

    Not all values are available in every use case. Especially, some servers
    will reject a reason code not suited for a specific command. See below
    table to highlight expected reason codes for specific actions.

    \table
    \header
        \li Reason Code
        \li Connect Properties
        \li Subscription Properties
        \li Message Properties
    \row
        \li Success
        \li X
        \li X
        \li X
    \row
        \li SubscriptionQoSLevel0
        \li
        \li X
        \li
    \row
        \li SubscriptionQoSLevel1
        \li
        \li X
        \li
    \row
        \li SubscriptionQoSLevel2
        \li
        \li X
        \li
    \row
        \li NoMatchingSubscriber
        \li
        \li
        \li X
    \row
        \li UnspecifiedError
        \li X
        \li X
        \li X
    \row
        \li MalformedPacket
        \li X
        \li
        \li
    \row
        \li ProtocolError
        \li X
        \li
        \li
    \row
        \li ImplementationSpecificError
        \li X
        \li X
        \li X
    \row
        \li UnsupportedProtocolVersion
        \li X
        \li
        \li
    \row
        \li InvalidClientId
        \li X
        \li
        \li
    \row
        \li InvalidUserNameOrPassword
        \li X
        \li
        \li
    \row
        \li NotAuthorized
        \li X
        \li X
        \li X
    \row
        \li ServerNotAvailable
        \li X
        \li
        \li
    \row
        \li ServerBusy
        \li X
        \li
        \li
    \row
        \li ClientBanned
        \li X
        \li
        \li
    \row
        \li InvalidAuthenticationMethod
        \li X
        \li
        \li
    \row
        \li InvalidTopicFilter
        \li
        \li X
        \li
    \row
        \li InvalidTopicName
        \li X
        \li
        \li X
    \row
        \li MessageIdInUse
        \li
        \li X
        \li X
    \row
        \li MessageIdNotFound
        \li
        \li
        \li X
    \row
        \li PacketTooLarge
        \li X
        \li
        \li
    \row
        \li QuotaExceeded
        \li X
        \li X
        \li X
    \row
        \li InvalidPayloadFormat
        \li X
        \li
        \li X
    \row
        \li RetainNotSupported
        \li X
        \li
        \li
    \row
        \li QoSNotSupported
        \li X
        \li
        \li
    \row
        \li UseAnotherServer
        \li X
        \li
        \li
    \row
        \li ServerMoved
        \li X
        \li
        \li
    \row
        \li SharedSubscriptionsNotSupported
        \li
        \li X
        \li
    \row
        \li ExceededConnectionRate
        \li X
        \li
        \li
    \row
        \li SubscriptionIdsNotSupported
        \li
        \li X
        \li
    \row
        \li WildCardSubscriptionsNotSupported
        \li
        \li X
        \li
    \endtable
*/

class QMqttStringPairData : public QSharedData
{
public:
    QMqttStringPairData() = default;
    QMqttStringPairData(const QString &name, const QString &value);

    bool operator==(const QMqttStringPairData &rhs) const;
    QString m_name;
    QString m_value;
};

QMqttStringPairData::QMqttStringPairData(const QString &name, const QString &value)
    : m_name(name)
    , m_value(value)
{
}

bool QMqttStringPairData::operator==(const QMqttStringPairData &rhs) const
{
    return m_name == rhs.m_name && m_value == rhs.m_value;
}

QMqttStringPair::QMqttStringPair()
    : data(new QMqttStringPairData)
{

}

QMqttStringPair::QMqttStringPair(const QString &name, const QString &value)
    : data(new QMqttStringPairData(name, value))
{
}

QMqttStringPair::QMqttStringPair(const QMqttStringPair &) = default;

QMqttStringPair::~QMqttStringPair() = default;

/*!
    Returns the name of the string pair.
*/
QString QMqttStringPair::name() const
{
    return data->m_name;
}

/*!
    Sets the name to \a n.
*/
void QMqttStringPair::setName(const QString &n)
{
    data->m_name = n;
}

/*!
    Returns the value of the string pair.
*/
QString QMqttStringPair::value() const
{
    return data->m_value;
}

/*!
    Sets the value to \a v.
*/
void QMqttStringPair::setValue(const QString &v)
{
    data->m_value = v;
}

/*!
    Returns \c true if this instance matches \a other.
*/
bool QMqttStringPair::operator==(const QMqttStringPair &other) const
{
    return *data.constData() == *other.data.constData();
}

/*!
    Returns \c true if this instance does not match \a other.
*/
bool QMqttStringPair::operator!=(const QMqttStringPair &other) const
{
    return !operator==(other);
}

QMqttStringPair &QMqttStringPair::operator=(const QMqttStringPair &rhs)
{
    if (this != &rhs)
        data.operator=(rhs.data);
    return *this;
}

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug d, const QMqttStringPair &s)
{
    QDebugStateSaver saver(d);
    d.nospace() << "QMqttStringPair(" << s.name() << " : " << s.value() << ')';
    return d;
}
#endif

QT_END_NAMESPACE
