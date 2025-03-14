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

#include "qmqttpublishproperties.h"
#include "qmqttpublishproperties_p.h"
#include "qmqtttype.h"

#include <QtCore/QLoggingCategory>

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(lcMqttClient)

/*!
    \class QMqttPublishProperties

    \inmodule QtMqtt
    \since 5.12

    \brief The QMqttPublishProperties class represents configuration
    options for sending or receiving a message.

    Invoking QMqttClient::publish() to send a message to a broker can
    include QMqttPublishProperties to provide additional arguments on
    how the message should be treated on the broker.

    Furthermore receiving a message by an instantiated subscription
    might contain publish properties which have been forwarded or
    adapted by the server.

    \note Publish properties are part of the MQTT 5.0 specification and
    cannot be used when connecting with a lower protocol level. See
    QMqttClient::ProtocolVersion for more information.
*/

/*!
    \enum QMqttPublishProperties::PublishPropertyDetail

    This enum type specifies the available properties set by the
    server or the client when creating a message.

    \value None
           No property has been specified.
    \value PayloadFormatIndicator
           The type of content of the message.
    \value MessageExpiryInterval
           The duration a message is valid.
    \value TopicAlias
           The topic alias for this message.
    \value ResponseTopic
           The topic the recipient should respond to.
    \value CorrelationData
           An identifier of the response message.
    \value UserProperty
           Additional properties set by the user.
    \value SubscriptionIdentifier
           An identifier of subscriptions matching the publication.
    \value ContentType
           A description of the content of the message.
*/

QMqttPublishProperties::QMqttPublishProperties() : data(new QMqttPublishPropertiesData)
{
}

/*!
    \internal
*/
QMqttPublishProperties::QMqttPublishProperties(const QMqttPublishProperties &) = default;

/*!
    \internal
*/
QMqttPublishProperties &QMqttPublishProperties::operator=(const QMqttPublishProperties &rhs)
{
    if (this != &rhs)
        data.operator=(rhs.data);
    return *this;
}

QMqttPublishProperties::~QMqttPublishProperties() = default;

/*!
    Returns the available properties specified in this instance. When a message
    is created, it does not need to include all properties. This function
    serves as an indicator of those properties which have been explicitly
    set.
*/
QMqttPublishProperties::PublishPropertyDetails QMqttPublishProperties::availableProperties() const
{
    return data->details;
}

/*!
    Returns the payload format indicator.
*/
QMqtt::PayloadFormatIndicator QMqttPublishProperties::payloadFormatIndicator() const
{
    return data->payloadIndicator;
}

/*!
    Sets the payload format indicator to \a indicator.
*/
void QMqttPublishProperties::setPayloadFormatIndicator(QMqtt::PayloadFormatIndicator indicator)
{
    data->details |= QMqttPublishProperties::PayloadFormatIndicator;
    data->payloadIndicator = indicator;
}

/*!
    Returns the message expiry interval. This value specifies the number
    of seconds a server is allowed to forward the message. If the interval
    expires, the server must delete the message and abort publishing it.
*/
quint32 QMqttPublishProperties::messageExpiryInterval() const
{
    return data->messageExpiry;
}

/*!
    Sets the message expiry interval to \a interval.
*/
void QMqttPublishProperties::setMessageExpiryInterval(quint32 interval)
{
    data->details |= QMqttPublishProperties::MessageExpiryInterval;
    data->messageExpiry = interval;
}

/*!
    Returns the topic alias used for publishing a message.
*/
quint16 QMqttPublishProperties::topicAlias() const
{
    return data->topicAlias;
}

/*!
    Sets the topic alias for publishing a message to \a alias. A topic alias
    value must be greater than zero and less than the maximum topic alias
    specified by the server.

    \sa QMqttServerConnectionProperties::maximumTopicAlias()
*/
void QMqttPublishProperties::setTopicAlias(quint16 alias)
{
    if (alias == 0) {
        qCDebug(lcMqttClient) << "A topic alias with value 0 is not allowed.";
        return;
    }
    data->details |= QMqttPublishProperties::TopicAlias;
    data->topicAlias = alias;
}

/*!
    Returns the response topic a user should use as a follow up to
    a request.
*/
QString QMqttPublishProperties::responseTopic() const
{
    return data->responseTopic;
}

/*!
    Sets the response topic to \a topic.
*/
void QMqttPublishProperties::setResponseTopic(const QString &topic)
{
    data->details |= QMqttPublishProperties::ResponseTopic;
    data->responseTopic = topic;
}

/*!
    Returns the correlation data.
*/
QByteArray QMqttPublishProperties::correlationData() const
{
    return data->correlationData;
}

/*!
    Sets the correlation data to \a correlation.
*/
void QMqttPublishProperties::setCorrelationData(const QByteArray &correlation)
{
    data->details |= QMqttPublishProperties::CorrelationData;
    data->correlationData = correlation;
}

/*!
    Returns the user properties of a message.
*/
QMqttUserProperties QMqttPublishProperties::userProperties() const
{
    return data->userProperties;
}

/*!
    Sets the user properties of a message to \a properties.
*/
void QMqttPublishProperties::setUserProperties(const QMqttUserProperties &properties)
{
    data->details |= QMqttPublishProperties::UserProperty;
    data->userProperties = properties;
}

/*!
    Returns the subscription identifiers of subscriptions matching
    the topic filter of the message.
*/
QList<quint32> QMqttPublishProperties::subscriptionIdentifiers() const
{
    return data->subscriptionIdentifier;
}

/*!
    Sets the subscription identifiers to \a ids.
*/
void QMqttPublishProperties::setSubscriptionIdentifiers(const QList<quint32> &ids)
{
    if (ids.contains(0)) {
        qCDebug(lcMqttClient) << "A subscription identifier with value 0 is not allowed.";
        return;
    }
    data->details |= QMqttPublishProperties::SubscriptionIdentifier;
    data->subscriptionIdentifier = ids;
}

/*!
    Returns the content type of the message.
*/
QString QMqttPublishProperties::contentType() const
{
    return data->contentType;
}

/*!
    Sets the content type of the message to \a type.
*/
void QMqttPublishProperties::setContentType(const QString &type)
{
    data->details |= QMqttPublishProperties::ContentType;
    data->contentType = type;
}

/*!
    \class QMqttMessageStatusProperties

    \inmodule QtMqtt
    \since 5.12

    \brief The QMqttMessageStatusProperties class represents additional
    information provided by the server during message delivery.

    Depending on the QoS level of a message being sent by QMqttClient::publish(),
    a server reports the state of delivery. Additionally to the QMqtt::MessageStatus,
    complementary information might be included by the server. These are exposed to
    users via QMqttMessageStatusProperties.

    \note Message status properties are part of the MQTT 5.0 specification and
    cannot be used when connecting with a lower protocol level. See
    QMqttClient::ProtocolVersion for more information.
*/
QMqttMessageStatusProperties::QMqttMessageStatusProperties() : data(new QMqttMessageStatusPropertiesData)
{

}

QMqttMessageStatusProperties &QMqttMessageStatusProperties::operator=(const QMqttMessageStatusProperties &rhs)
{
    if (this != &rhs)
        data.operator=(rhs.data);
    return *this;
}

/*!
    Returns the reason code of a failed message delivery.
*/
QMqtt::ReasonCode QMqttMessageStatusProperties::reasonCode() const
{
    return data->reasonCode;
}

/*!
    Returns the reason string of a failed message delivery.
*/
QString QMqttMessageStatusProperties::reason() const
{
    return data->reasonString;
}

/*!
    Returns properties specified in conjunction with a message.
*/
QMqttUserProperties QMqttMessageStatusProperties::userProperties() const
{
    return data->userProperties;
}

QMqttMessageStatusProperties::~QMqttMessageStatusProperties() = default;
QMqttMessageStatusProperties::QMqttMessageStatusProperties(const QMqttMessageStatusProperties &) = default;

QT_END_NAMESPACE
