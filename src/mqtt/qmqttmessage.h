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

#ifndef QMQTTMESSAGE_H
#define QMQTTMESSAGE_H

#include "qmqttglobal.h"
#include "qmqttpublishproperties.h"
#include "qmqtttopicname.h"

#include <QtCore/QObject>
#include <QtCore/QSharedDataPointer>

QT_BEGIN_NAMESPACE

class QMqttMessagePrivate;

class /*Q_MQTT_EXPORT*/ QMqttMessage : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QMqttTopicName topic READ topic CONSTANT)
    Q_PROPERTY(QByteArray payload READ payload CONSTANT)
    Q_PROPERTY(quint16 id READ id CONSTANT)
    Q_PROPERTY(quint8 qos READ qos CONSTANT)
    Q_PROPERTY(bool duplicate READ duplicate CONSTANT)
    Q_PROPERTY(bool retain READ retain CONSTANT)

public:
    QMqttMessage();
    QMqttMessage(const QMqttMessage& other);
    ~QMqttMessage();

    QMqttMessage& operator=(const QMqttMessage &other);
    bool operator==(const QMqttMessage &other) const;
    inline bool operator!=(const QMqttMessage &other) const;

    const QByteArray &payload() const;
    quint8 qos() const;
    quint16 id() const;
    QMqttTopicName topic() const;
    bool duplicate() const;
    bool retain() const;

    QMqttPublishProperties publishProperties() const;
private:
    friend class QMqttConnection;
    QMqttMessage(const QMqttTopicName &topic, const QByteArray &payload,
                          quint16 id, quint8 qos,
                          bool dup, bool retain);
    QExplicitlySharedDataPointer<QMqttMessagePrivate> d;
};

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QMqttMessage)

#endif // QMQTTMESSAGE_H
