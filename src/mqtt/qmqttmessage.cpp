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

#include "qmqttmessage.h"
#include "qmqttmessage_p.h"

QT_BEGIN_NAMESPACE

/*!
    \class QMqttMessage

    \inmodule QtMqtt
    \brief The QMqttMessage class provides information about a message received
    from a message broker based on a subscription.

    An MQTT message is created inside the module and returned via the
    \l QMqttSubscription::messageReceived() signal.
*/

/*!
    \property QMqttMessage::topic
    \brief This property holds the topic of a message.

    In case a wildcard has been used for a subscription, describes the topic
    matching this subscription. This property never contains wildcards.
*/

/*!
    \property QMqttMessage::payload
    \brief This property holds the payload of a message.
*/

/*!
    \property QMqttMessage::id
    \brief This property holds the ID of the message.

    IDs are used for messages with a QoS level above zero.
*/

/*!
    \property QMqttMessage::qos
    \brief This property holds the QoS level of a message.
*/

/*!
    \property QMqttMessage::duplicate
    \brief This property holds whether the message is a duplicate.

    Duplicate messages indicate that the message has been sent earlier, but it
    has not been confirmed yet. Hence, the broker assumes that it needs to
    resend to verify the transport of the message itself. Duplicate messages
    can only occur if the QoS level is one or two.
*/

/*!
    \property QMqttMessage::retain
    \brief This property holds whether the message has been retained.

    A retained message is kept on the broker for future clients to subscribe.
    Consequently, a retained message has been created previously and is not a
    live update. A broker can store only one retained message per topic.
*/

/*!
    Creates a new MQTT message.
*/
QMqttMessage::QMqttMessage()
    : d(new QMqttMessagePrivate())
{
}

/*!
    Constructs a new MQTT message that is a copy of \a other.
*/
QMqttMessage::QMqttMessage(const QMqttMessage &other)
    : d(other.d)
{
}

QMqttMessage::~QMqttMessage()
{
}

/*!
    Makes this object a copy of \a other and returns the new value of this object.
*/
QMqttMessage &QMqttMessage::operator=(const QMqttMessage &other)
{
    d = other.d;
    return *this;
}

/*!
    Returns \c true if the message and \a other are equal, otherwise returns \c false.
*/
bool QMqttMessage::operator==(const QMqttMessage &other) const
{
    return d == other.d;
}

/*!
    Returns \c true if the message and \a other are not equal, otherwise returns \c false.
*/
bool QMqttMessage::operator!=(const QMqttMessage &other) const
{
    return !(*this == other);
}

const QByteArray &QMqttMessage::payload() const
{
    return d->m_payload;
}

quint8 QMqttMessage::qos() const
{
    return d->m_qos;
}

quint16 QMqttMessage::id() const
{
    return d->m_id;
}

QMqttTopicName QMqttMessage::topic() const
{
    return d->m_topic;
}

bool QMqttMessage::duplicate() const
{
    return d->m_duplicate;
}

bool QMqttMessage::retain() const
{
    return d->m_retain;
}

/*!
    \since 5.12

    Returns the publish properties received as part of the message.

    \note This function only specifies the properties when a
    publish message is received. Messages with a QoS value of
    1 or 2 can contain additional properties when a message is released.
    Those can be obtained by the QMqttClient::messageStatusChanged signal.

    \note This function will only provide valid data when the client
    specifies QMqttClient::MQTT_5_0 as QMqttClient::ProtocolVersion.
*/
QMqttPublishProperties QMqttMessage::publishProperties() const
{
    return d->m_publishProperties;
}

/*!
    \internal
    Constructs a new MQTT message with \a content on the topic \a topic.
    Furthermore the properties \a id, \a qos, \a dup, \a retain must be specified.

    This constructor is mostly used internally to construct a QMqttMessage at message
    receival.
*/
QMqttMessage::QMqttMessage(const QMqttTopicName &topic, const QByteArray &content, quint16 id, quint8 qos, bool dup, bool retain)
    : d(new QMqttMessagePrivate)
{
    d->m_topic = topic;
    d->m_payload = content;
    d->m_id = id;
    d->m_qos = qos;
    d->m_duplicate = dup;
    d->m_retain = retain;
}

QT_END_NAMESPACE
