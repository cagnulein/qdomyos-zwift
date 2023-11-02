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

#ifndef QSSLSERVER_H
#define QSSLSERVER_H

#include <QtSslServer/qtsslserverglobal.h>

#include <QtNetwork/qtcpserver.h>
#include <QtNetwork/qsslconfiguration.h>

#include <QtCore/qscopedpointer.h>

QT_BEGIN_NAMESPACE

class QSslServerPrivate;
class Q_SSLSERVER_EXPORT QSslServer : public QTcpServer
{
    Q_OBJECT
public:
    QSslServer(QObject *parent = nullptr);
    QSslServer(const QSslConfiguration &sslConfiguration, QObject *parent = nullptr);
    ~QSslServer();

    void setSslConfiguration(const QSslConfiguration &sslConfiguration);

Q_SIGNALS:
    void sslErrors(QSslSocket *socket, const QList<QSslError> &errors);

protected:
    void incomingConnection(qintptr handle) override final;

private:
    QScopedPointer<QSslServerPrivate> d;
};

QT_END_NAMESPACE

#endif // QSSLSERVER_HPP
