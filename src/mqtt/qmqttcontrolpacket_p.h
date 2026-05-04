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

#ifndef QMQTTCONTROLPACKET_P_H
#define QMQTTCONTROLPACKET_P_H

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

#include <QtCore/QByteArray>
#include <QtCore/QtGlobal>

QT_BEGIN_NAMESPACE

class Q_AUTOTEST_EXPORT QMqttControlPacket
{
public:
    enum PacketType {
        UNKNOWN     = 0x00,
        CONNECT     = 0x10,
        CONNACK     = 0x20,
        PUBLISH     = 0x30,
        PUBACK      = 0x40,
        PUBREC      = 0x50,
        PUBREL      = 0x60,
        PUBCOMP     = 0x70,
        SUBSCRIBE   = 0x80,
        SUBACK      = 0x90,
        UNSUBSCRIBE = 0xA0,
        UNSUBACK    = 0xB0,
        PINGREQ     = 0xC0,
        PINGRESP    = 0xD0,
        DISCONNECT  = 0xE0,
        AUTH        = 0xF0,
    };

    QMqttControlPacket();
    QMqttControlPacket(quint8 header);
    QMqttControlPacket(quint8 header, const QByteArray &pay);

    void clear();

    void setHeader(quint8 h);
    inline quint8 header() const { return m_header; }

    void append(char value);
    void append(quint16 value);
    void append(quint32 value);
    void append(const QByteArray &data);
    void appendRaw(const QByteArray &data);
    void appendRawVariableInteger(quint32 value);

    QByteArray serialize() const;
    QByteArray serializePayload() const;
    inline QByteArray payload() const { return m_payload; }
private:
    quint8 m_header{UNKNOWN};
    QByteArray m_payload;
};

QT_END_NAMESPACE

#endif // QMQTTCONTROLPACKET_P_H
