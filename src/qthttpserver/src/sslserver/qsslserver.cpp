/****************************************************************************
**
** Copyright (C) 2019 Sylvain Garcia <garcia.6l20@gmail.com>.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtHttpServer module of the Qt Toolkit.
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
****************************************************************************/

#include <private/qsslserver_p.h>

#include <QtCore/qloggingcategory.h>

QT_BEGIN_NAMESPACE

Q_LOGGING_CATEGORY(lcSS, "qt.sslserver");

QSslServer::QSslServer(QObject *parent):
    QTcpServer (parent), d(new QSslServerPrivate)
{
}

QSslServer::QSslServer(const QSslConfiguration &sslConfiguration,
                       QObject *parent):
    QTcpServer (parent), d(new QSslServerPrivate)
{
    d->sslConfiguration = sslConfiguration;
}

QSslServer::~QSslServer() = default;

void QSslServer::incomingConnection(qintptr handle)
{
    QSslSocket *socket = new QSslSocket(this);
    connect(socket, QOverload<const QList<QSslError>&>::of(&QSslSocket::sslErrors),
            [this, socket](const QList<QSslError> &errors) {
        for (auto &err: errors)
            qCCritical(lcSS) << err;
        Q_EMIT sslErrors(socket, errors);
    });
    socket->setSocketDescriptor(handle);
    socket->setSslConfiguration(d->sslConfiguration);
    socket->startServerEncryption();

    addPendingConnection(socket);
}

void QSslServer::setSslConfiguration(const QSslConfiguration &sslConfiguration)
{
    d->sslConfiguration = sslConfiguration;
}
QT_END_NAMESPACE
