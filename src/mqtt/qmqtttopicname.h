/******************************************************************************
**
** Copyright (C) 2017 Lorenz Haas
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

#ifndef QMQTTTOPICNAME_H
#define QMQTTTOPICNAME_H

#include "qmqttglobal.h"

#include <QtCore/QExplicitlySharedDataPointer>
#include <QtCore/QMetaType>
#include <QtCore/QString>
#include <QtCore/QStringList>

QT_BEGIN_NAMESPACE

class QMqttTopicNamePrivate;

class QMqttTopicName;
// qHash is a friend, but we can't use default arguments for friends (§8.3.6.4)
Q_MQTT_EXPORT uint qHash(const QMqttTopicName &name, uint seed = 0) Q_DECL_NOTHROW;

class Q_MQTT_EXPORT QMqttTopicName
{
public:
    QMqttTopicName(const QString &name = QString());
    QMqttTopicName(const QLatin1String &name);
    QMqttTopicName(const QMqttTopicName &name);
    ~QMqttTopicName();
    QMqttTopicName &operator=(const QMqttTopicName &name);

#ifdef Q_COMPILER_RVALUE_REFS
    inline QMqttTopicName &operator=(QMqttTopicName &&other) Q_DECL_NOTHROW { qSwap(d, other.d); return *this; }
#endif

    inline void swap(QMqttTopicName &other) Q_DECL_NOTHROW { qSwap(d, other.d); }

    QString name() const;
    void setName(const QString &name);

    Q_REQUIRED_RESULT bool isValid() const;
    Q_REQUIRED_RESULT int levelCount() const;
    Q_REQUIRED_RESULT QStringList levels() const;

    friend Q_MQTT_EXPORT bool operator==(const QMqttTopicName &lhs, const QMqttTopicName &rhs) Q_DECL_NOTHROW;
    friend inline bool operator!=(const QMqttTopicName &lhs, const QMqttTopicName &rhs) Q_DECL_NOTHROW { return !(lhs == rhs); }
    friend Q_MQTT_EXPORT bool operator<(const QMqttTopicName &lhs, const QMqttTopicName &rhs) Q_DECL_NOTHROW;
    friend Q_MQTT_EXPORT uint qHash(const QMqttTopicName &name, uint seed) Q_DECL_NOTHROW;

private:
    QExplicitlySharedDataPointer<QMqttTopicNamePrivate> d;
};

Q_DECLARE_SHARED(QMqttTopicName)

#ifndef QT_NO_DATASTREAM
Q_MQTT_EXPORT QDataStream &operator<<(QDataStream &, const QMqttTopicName &);
Q_MQTT_EXPORT QDataStream &operator>>(QDataStream &, QMqttTopicName &);
#endif

#ifndef QT_NO_DEBUG_STREAM
Q_MQTT_EXPORT QDebug operator<<(QDebug, const QMqttTopicName &);
#endif

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QMqttTopicName)

#endif // QMQTTTOPICNAME_H
