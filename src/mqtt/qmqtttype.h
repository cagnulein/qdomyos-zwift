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

#ifndef QMQTTTYPE_H
#define QMQTTTYPE_H

#include "qmqttglobal.h"

#include <QtCore/QDebug>
#include <QtCore/QPair>
#include <QtCore/QSharedDataPointer>
#include <QtCore/QString>
#include <QtCore/QVector>

QT_BEGIN_NAMESPACE

class QMqttStringPairData;
class Q_MQTT_EXPORT QMqttStringPair
{
public:
    QMqttStringPair();
    QMqttStringPair(const QString &name, const QString &value);
    QMqttStringPair(const QMqttStringPair &);
    ~QMqttStringPair();

    QString name() const;
    void setName(const QString &n);

    QString value() const;
    void setValue(const QString &v);

    bool operator==(const QMqttStringPair &other) const;
    bool operator!=(const QMqttStringPair &other) const;
    QMqttStringPair &operator=(const QMqttStringPair &);
private:
    QSharedDataPointer<QMqttStringPairData> data;
};

#ifndef QT_NO_DEBUG_STREAM
Q_MQTT_EXPORT QDebug operator<<(QDebug d, const QMqttStringPair &s);
#endif

class Q_MQTT_EXPORT QMqttUserProperties : public QVector<QMqttStringPair>
{
public:
};

QT_END_NAMESPACE

#endif // QMQTTTYPE_H
