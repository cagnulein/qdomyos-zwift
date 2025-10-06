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
#ifndef QMQTTPUBLISHPROPERTIES_H
#define QMQTTPUBLISHPROPERTIES_H

#include "qmqttglobal.h"
#include "qmqtttype.h"

#include <QtCore/QObject>
#include <QtCore/QSharedDataPointer>

QT_BEGIN_NAMESPACE

class QMqttPublishPropertiesData;
class QMqttMessageStatusPropertiesData;

class Q_MQTT_EXPORT QMqttPublishProperties
{
public:
    enum PublishPropertyDetail : quint32 {
        None                   = 0x00000000,
        PayloadFormatIndicator = 0x00000001,
        MessageExpiryInterval  = 0x00000002,
        TopicAlias             = 0x00000004,
        ResponseTopic          = 0x00000008,
        CorrelationData        = 0x00000010,
        UserProperty           = 0x00000020,
        SubscriptionIdentifier = 0x00000040,
        ContentType            = 0x00000080
    };
    Q_DECLARE_FLAGS(PublishPropertyDetails, PublishPropertyDetail)

    QMqttPublishProperties();
    QMqttPublishProperties(const QMqttPublishProperties &);
    QMqttPublishProperties &operator=(const QMqttPublishProperties &);
    ~QMqttPublishProperties();

    PublishPropertyDetails availableProperties() const;

    QMqtt::PayloadFormatIndicator payloadFormatIndicator() const;
    void setPayloadFormatIndicator(QMqtt::PayloadFormatIndicator indicator);

    quint32 messageExpiryInterval() const;
    void setMessageExpiryInterval(quint32 interval);

    quint16 topicAlias() const;
    void setTopicAlias(quint16 alias);

    QString responseTopic() const;
    void setResponseTopic(const QString &topic);

    QByteArray correlationData() const;
    void setCorrelationData(const QByteArray &correlation);

    QMqttUserProperties userProperties() const;
    void setUserProperties(const QMqttUserProperties &properties);

    QList<quint32> subscriptionIdentifiers() const;
    void setSubscriptionIdentifiers(const QList<quint32> &ids);

    QString contentType() const;
    void setContentType(const QString &type);
private:
    QSharedDataPointer<QMqttPublishPropertiesData> data;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QMqttPublishProperties::PublishPropertyDetails)

class Q_MQTT_EXPORT QMqttMessageStatusProperties
{
public:
    QMqttMessageStatusProperties();
    QMqttMessageStatusProperties(const QMqttMessageStatusProperties &);
    QMqttMessageStatusProperties &operator=(const QMqttMessageStatusProperties &);
    ~QMqttMessageStatusProperties();

    QMqtt::ReasonCode reasonCode() const;
    QString reason() const;
    QMqttUserProperties userProperties() const;

private:
    friend class QMqttConnection;
    QSharedDataPointer<QMqttMessageStatusPropertiesData> data;
};

QT_END_NAMESPACE

#endif // QMQTTPUBLISHPROPERTIES_H
