/****************************************************************************
**
** Copyright (C) 2019 The Qt Company Ltd.
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

#ifndef QHTTPSERVERROUTERRULE_H
#define QHTTPSERVERROUTERRULE_H

#include <QtHttpServer/qhttpserverrequest.h>

#include <QtCore/qmap.h>

QT_BEGIN_NAMESPACE

class QString;
class QHttpServerRequest;
class QTcpSocket;
class QRegularExpressionMatch;
class QHttpServerRouter;

class QHttpServerRouterRulePrivate;
class Q_HTTPSERVER_EXPORT QHttpServerRouterRule
{
    Q_DECLARE_PRIVATE(QHttpServerRouterRule)

public:
    using RouterHandler = std::function<void(const QRegularExpressionMatch &,
                                             const QHttpServerRequest &,
                                             QTcpSocket *)>;

    explicit QHttpServerRouterRule(const QString &pathPattern, RouterHandler &&routerHandler);
    explicit QHttpServerRouterRule(const QString &pathPattern,
                                   const QHttpServerRequest::Methods methods,
                                   RouterHandler &&routerHandler);
    explicit QHttpServerRouterRule(const QString &pathPattern,
                                   const char * methods,
                                   RouterHandler &&routerHandler);

    QHttpServerRouterRule(QHttpServerRouterRule &&other) = delete;
    QHttpServerRouterRule &operator=(QHttpServerRouterRule &&other) = delete;

    virtual ~QHttpServerRouterRule();

protected:
    bool exec(const QHttpServerRequest &request, QTcpSocket *socket) const;

    bool hasValidMethods() const;

    bool createPathRegexp(const std::initializer_list<int> &metaTypes,
                          const QMap<int, QLatin1String> &converters);

    virtual bool matches(const QHttpServerRequest &request,
                         QRegularExpressionMatch *match) const;

    QHttpServerRouterRule(QHttpServerRouterRulePrivate *d);

private:
    Q_DISABLE_COPY(QHttpServerRouterRule)
    QScopedPointer<QHttpServerRouterRulePrivate> d_ptr;

    friend class QHttpServerRouter;
};

QT_END_NAMESPACE

#endif // QHTTPSERVERROUTERRULE_H
