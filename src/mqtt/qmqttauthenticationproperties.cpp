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

#include "qmqttauthenticationproperties.h"

QT_BEGIN_NAMESPACE

/*!
    \class QMqttAuthenticationProperties

    \inmodule QtMqtt
    \since 5.12

    \brief The QMqttAuthenticationProperties class represents configuration
    options during the authentication process.

    \note Authentication properties are part of the MQTT 5.0 specification and
    cannot be used when connecting with a lower protocol level. See
    QMqttClient::ProtocolVersion for more information.
*/

class QMqttAuthenticationPropertiesData : public QSharedData
{
public:
    QString authenticationMethod;
    QByteArray authenticationData;
    QString reason;
    QMqttUserProperties userProperties;
};

/*!
    \internal
*/
QMqttAuthenticationProperties::QMqttAuthenticationProperties() : data(new QMqttAuthenticationPropertiesData)
{

}

/*!
    \internal
*/
QMqttAuthenticationProperties::QMqttAuthenticationProperties(const QMqttAuthenticationProperties &) = default;

QMqttAuthenticationProperties &QMqttAuthenticationProperties::operator=(const QMqttAuthenticationProperties &rhs)
{
    if (this != &rhs)
        data.operator=(rhs.data);
    return *this;
}

QMqttAuthenticationProperties::~QMqttAuthenticationProperties() = default;

/*!
    Returns the authentication method.
*/
QString QMqttAuthenticationProperties::authenticationMethod() const
{
    return data->authenticationMethod;
}

/*!
    Sets the authentication method to \a method.
*/
void QMqttAuthenticationProperties::setAuthenticationMethod(const QString &method)
{
    data->authenticationMethod = method;
}

/*!
  Returns the authentication data
*/
QByteArray QMqttAuthenticationProperties::authenticationData() const
{
    return data->authenticationData;
}

/*!
    Sets the authentication data to \a adata.

    Authentication data can only be used if an authentication method has
    been specified.

    \sa authenticationMethod()
*/
void QMqttAuthenticationProperties::setAuthenticationData(const QByteArray &adata)
{
    data->authenticationData = adata;
}

/*!
    Returns the reason string. The reason string specifies the reason for
    a disconnect.
*/
QString QMqttAuthenticationProperties::reason() const
{
    return data->reason;
}

/*!
    Sets the reason string to \a r.
*/
void QMqttAuthenticationProperties::setReason(const QString &r)
{
    data->reason = r;
}

/*!
    Returns the user properties.
*/
QMqttUserProperties QMqttAuthenticationProperties::userProperties() const
{
    return data->userProperties;
}

/*!
    Sets the user properties to \a user.
*/
void QMqttAuthenticationProperties::setUserProperties(const QMqttUserProperties &user)
{
    data->userProperties = user;
}

QT_END_NAMESPACE
