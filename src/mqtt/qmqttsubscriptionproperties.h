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

#ifndef QMQTTSUBSCRIPTIONPROPERTIES_H
#define QMQTTSUBSCRIPTIONPROPERTIES_H

#include "qmqttglobal.h"
#include "qmqtttype.h"

#include <QtCore/QHash>
#include <QtCore/QObject>
#include <QtCore/QSharedDataPointer>
#include <QtCore/QString>

QT_BEGIN_NAMESPACE

class QMqttSubscriptionPropertiesData;
class QMqttUnsubscriptionPropertiesData;

class Q_MQTT_EXPORT QMqttSubscriptionProperties
{
public:
    QMqttSubscriptionProperties();
    QMqttSubscriptionProperties(const QMqttSubscriptionProperties &);
    QMqttSubscriptionProperties &operator=(const QMqttSubscriptionProperties &);
    ~QMqttSubscriptionProperties();

    QMqttUserProperties userProperties() const;
    void setUserProperties(const QMqttUserProperties &user);

    quint32 subscriptionIdentifier() const;
    void setSubscriptionIdentifier(quint32 id);
private:
    QSharedDataPointer<QMqttSubscriptionPropertiesData> data;
};

class Q_MQTT_EXPORT QMqttUnsubscriptionProperties
{
public:
    QMqttUnsubscriptionProperties();
    QMqttUnsubscriptionProperties(const QMqttUnsubscriptionProperties &);
    QMqttUnsubscriptionProperties &operator=(const QMqttUnsubscriptionProperties &rhs);
    ~QMqttUnsubscriptionProperties();

    QMqttUserProperties userProperties() const;
    void setUserProperties(const QMqttUserProperties &user);

private:
    QSharedDataPointer<QMqttUnsubscriptionPropertiesData> data;
};

QT_END_NAMESPACE

#endif // QMQTTSUBSCRIPTIONPROPERTIES_H
