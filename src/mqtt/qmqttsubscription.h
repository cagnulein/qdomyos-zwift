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

#ifndef QMQTTSUBSCRIPTION_H
#define QMQTTSUBSCRIPTION_H

#include "qmqttglobal.h"
#include "qmqttmessage.h"
#include "qmqtttopicfilter.h"

#include <QtCore/QObject>

QT_BEGIN_NAMESPACE

class QMqttClient;
class QMqttSubscriptionPrivate;

class Q_MQTT_EXPORT QMqttSubscription : public QObject
{
    Q_OBJECT
    Q_ENUMS(SubscriptionState)
    Q_PROPERTY(SubscriptionState state READ state NOTIFY stateChanged)
    Q_PROPERTY(quint8 qos READ qos NOTIFY qosChanged)
    Q_PROPERTY(QMqttTopicFilter topic READ topic)
    Q_PROPERTY(QString reason READ reason)
    Q_PROPERTY(QMqtt::ReasonCode reasonCode READ reasonCode)
    Q_PROPERTY(bool sharedSubscription READ isSharedSubscription)
    Q_PROPERTY(QString sharedSubscriptionName READ sharedSubscriptionName)
public:
    ~QMqttSubscription() override;
    enum SubscriptionState {
        Unsubscribed = 0,
        SubscriptionPending,
        Subscribed,
        UnsubscriptionPending,
        Error
    };

    SubscriptionState state() const;
    QMqttTopicFilter topic() const;
    quint8 qos() const;
    QString reason() const;
    QMqtt::ReasonCode reasonCode() const;
    QMqttUserProperties userProperties() const;

    bool isSharedSubscription() const;
    QString sharedSubscriptionName() const;

Q_SIGNALS:
    void stateChanged(SubscriptionState state);
    void qosChanged(quint8); // only emitted when broker provides different QoS than requested
    void messageReceived(QMqttMessage msg);

public Q_SLOTS:
    void unsubscribe();

private:
    Q_DECLARE_PRIVATE(QMqttSubscription)
    Q_DISABLE_COPY(QMqttSubscription)
    void setState(SubscriptionState state);
    void setTopic(const QMqttTopicFilter &topic);
    void setClient(QMqttClient *client);
    void setQos(quint8 qos);
    void setSharedSubscription(bool s);
    void setSharedSubscriptionName(const QString &name);
    friend class QMqttConnection;
    friend class QMqttClient;
    explicit QMqttSubscription(QObject *parent = nullptr);
};

QT_END_NAMESPACE

#endif // QMQTTSUBSCRIPTION_H
