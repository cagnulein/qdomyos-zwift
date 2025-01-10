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

#ifndef QMQTTMESSAGE_P_H
#define QMQTTMESSAGE_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "qmqttglobal.h"
#include "qmqtttopicname.h"
#include "qmqttpublishproperties.h"

#include <QtCore/QSharedData>

QT_BEGIN_NAMESPACE

class QMqttMessagePrivate : public QSharedData
{
public:
    bool operator==(const QMqttMessagePrivate &other) const {
        return m_topic == other.m_topic
                && m_payload == other.m_payload
                && m_id == other.m_id
                && m_qos == other.m_qos
                && m_duplicate == other.m_duplicate
                && m_retain == other.m_retain;
    }
    QMqttTopicName m_topic;
    QByteArray m_payload;
    quint16 m_id{0};
    quint8 m_qos{0};
    bool m_duplicate{false};
    bool m_retain{false};
    QMqttPublishProperties m_publishProperties;
};

QT_END_NAMESPACE

#endif // QMQTTMESSAGE_P_H
