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
#include <limits>
#include "qmqttcontrolpacket_p.h"

#include <QtCore/QtEndian>
#include <QtCore/QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(lcMqttClient)

QMqttControlPacket::QMqttControlPacket()
{

}

QMqttControlPacket::QMqttControlPacket(quint8 header)
    : m_header(header)
{

}

QMqttControlPacket::QMqttControlPacket(quint8 header, const QByteArray &pay)
    : m_header(header)
    , m_payload(pay)
{

}

void QMqttControlPacket::clear()
{
    m_header = 0;
    m_payload.clear();
}

void QMqttControlPacket::setHeader(quint8 h)
{
    if (h < QMqttControlPacket::CONNECT || h > DISCONNECT || h & 0x0F)
        m_header = QMqttControlPacket::UNKNOWN;
    else
        m_header = h;
}

void QMqttControlPacket::append(char value)
{
    m_payload.append(value);
}

void QMqttControlPacket::append(quint16 value)
{
    const quint16 msb = qToBigEndian<quint16>(value);
    const char * msb_c = reinterpret_cast<const char*>(&msb);
    m_payload.append(msb_c, 2);
}

void QMqttControlPacket::append(quint32 value)
{
    const quint32 msb = qToBigEndian<quint32>(value);
    const char * msb_c = reinterpret_cast<const char*>(&msb);
    m_payload.append(msb_c, 4);
}

void QMqttControlPacket::append(const QByteArray &data)
{
    append(static_cast<quint16>(data.size()));
    m_payload.append(data);
}

void QMqttControlPacket::appendRaw(const QByteArray &data)
{
    m_payload.append(data);
}

void QMqttControlPacket::appendRawVariableInteger(quint32 value)
{
    QByteArray data;
    // Add length
    if (value > 268435455)
        qCDebug(lcMqttClient) << "Attempting to write variable integer overflow.";
    do {
        quint8 b = value % 128;
        value /= 128;
        if (value > 0)
            b |= 0x80;
        data.append(char(b));
    } while (value > 0);
    appendRaw(data);
}

QByteArray QMqttControlPacket::serialize() const
{
    // Create ByteArray
    QByteArray data;
    // Add Header
    data.append(char(m_header));
    data.append(serializePayload());

    return data;
}

QByteArray QMqttControlPacket::serializePayload() const
{
    QByteArray data;
    // Add length
    quint32 msgSize = quint32(m_payload.size());
    if (msgSize > 268435455)
        qCDebug(lcMqttClient) << "Publishing a message bigger than maximum size.";
    do {
        quint8 b = msgSize % 128;
        msgSize /= 128;
        if (msgSize > 0)
            b |= 0x80;
        data.append(char(b));
    } while (msgSize > 0);
    // Add payload
    data.append(m_payload);
    return data;
}

