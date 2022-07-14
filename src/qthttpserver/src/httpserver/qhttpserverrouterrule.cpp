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

#include <QtHttpServer/qhttpserverrouterrule.h>

#include <private/qhttpserverrouterrule_p.h>
#include <private/qhttpserverrequest_p.h>

#include <QtCore/qmetaobject.h>
#include <QtCore/qloggingcategory.h>
#include <QtCore/qregularexpression.h>
#include <QtCore/qstringbuilder.h>
#include <QtCore/qdebug.h>

QT_BEGIN_NAMESPACE

Q_LOGGING_CATEGORY(lcRouterRule, "qt.httpserver.router.rule")

static const auto methodEnum = QMetaEnum::fromType<QHttpServerRequest::Method>();

static QHttpServerRequest::Methods strToMethods(const char *strMethods)
{
    QHttpServerRequest::Methods methods;

    bool ok = false;
    const int val = methodEnum.keysToValue(strMethods, &ok);
    if (ok)
        methods = static_cast<decltype(methods)>(val);
    else
        qCWarning(lcRouterRule, "Can not convert %s to QHttpServerRequest::Method", strMethods);

    return methods;
}

/*!
    \class QHttpServerRouterRule
    \brief The QHttpServerRouterRule is the base class for QHttpServerRouter rules.

    Use QHttpServerRouterRule to specify expected request parameters:

    \value path                 QUrl::path()
    \value HTTP methods         QHttpServerRequest::Methods
    \value callback             User-defined response callback

    \note This is a low level API, see QHttpServer for higher level alternatives.

    Example of QHttpServerRouterRule and QHttpServerRouter usage:

    \code
    template<typename ViewHandler>
    void route(const char *path, const QHttpServerRequest::Methods methods, ViewHandler &&viewHandler)
    {
        auto rule = new QHttpServerRouterRule(
                path, methods, [this, &viewHandler] (QRegularExpressionMatch &match,
                                                    const QHttpServerRequest &request,
                                                    QTcpSocket *const socket) {
            auto boundViewHandler = router.bindCaptured<ViewHandler>(
                    std::forward<ViewHandler>(viewHandler), match);
            // call viewHandler
            boundViewHandler();
        });

        // QHttpServerRouter
        router.addRule<ViewHandler>(rule);
    }

    // Valid:
    route("/user/", [] (qint64 id) { } );                            // "/user/1"
                                                                     // "/user/3"
                                                                     //
    route("/user/<arg>/history", [] (qint64 id) { } );               // "/user/1/history"
                                                                     // "/user/2/history"
                                                                     //
    route("/user/<arg>/history/", [] (qint64 id, qint64 page) { } ); // "/user/1/history/1"
                                                                     // "/user/2/history/2"

    // Invalid:
    route("/user/<arg>", [] () { } );  // ERROR: path pattern has <arg>, but ViewHandler does not have any arguments
    route("/user/\\d+", [] () { } );   // ERROR: path pattern does not support manual regexp
    \endcode

    \note Regular expressions in the path pattern are not supported, but
    can be registered (to match a use of "<val>" to a specific type) using
    QHttpServerRouter::addConverter().
*/

/*!
    Constructs a rule with pathPattern \a pathPattern, and routerHandler \a routerHandler.

    The rule accepts all HTTP methods by default.

    \sq QHttpServerRequest::Method
*/
QHttpServerRouterRule::QHttpServerRouterRule(const QString &pathPattern,
                                             RouterHandler &&routerHandler)
    : QHttpServerRouterRule(pathPattern,
                            QHttpServerRequest::Method::All,
                            std::forward<RouterHandler>(routerHandler))
{
}

/*!
    Constructs a rule with pathPattern \a pathPattern, methods \a methods
    and routerHandler \a routerHandler.

    The rule accepts any combinations of available HTTP methods.

    \sa QHttpServerRequest::Method
*/
QHttpServerRouterRule::QHttpServerRouterRule(const QString &pathPattern,
                                             const QHttpServerRequest::Methods methods,
                                             RouterHandler &&routerHandler)
    : QHttpServerRouterRule(
        new QHttpServerRouterRulePrivate{pathPattern,
                                         methods,
                                         std::forward<RouterHandler>(routerHandler), {}})
{
}

/*!
    Constructs a rule with pathPattern \a pathPattern, methods \a methods
    and routerHandler \a routerHandler.

    \note \a methods shall be joined with | as separator (not spaces or commas)
    and that either the upper-case or the capitalised form may be used.

    \sa QMetaEnum::keysToValue
*/
QHttpServerRouterRule::QHttpServerRouterRule(const QString &pathPattern,
                                             const char *methods,
                                             RouterHandler &&routerHandler)
    : QHttpServerRouterRule(pathPattern,
                            strToMethods(methods),
                            std::forward<RouterHandler>(routerHandler))
{
}

/*!
    \internal
 */
QHttpServerRouterRule::QHttpServerRouterRule(QHttpServerRouterRulePrivate *d)
    : d_ptr(d)
{
}

/*!
    Destroys a QHttpServerRouterRule.
*/
QHttpServerRouterRule::~QHttpServerRouterRule()
{
}

/*!
    Returns true if the methods is valid
*/
bool QHttpServerRouterRule::hasValidMethods() const
{
    Q_D(const QHttpServerRouterRule);
    return d->methods & QHttpServerRequest::Method::All;
}

/*!
    This function is called by QHttpServerRouter when a new request is received.
*/
bool QHttpServerRouterRule::exec(const QHttpServerRequest &request,
                                 QTcpSocket *socket) const
{
    Q_D(const QHttpServerRouterRule);

    QRegularExpressionMatch match;
    if (!matches(request, &match))
        return false;

    d->routerHandler(match, request, socket);
    return true;
}

/*!
    This virtual function is called by exec() to check if request matches the rule.
*/
bool QHttpServerRouterRule::matches(const QHttpServerRequest &request,
                                    QRegularExpressionMatch *match) const
{
    Q_D(const QHttpServerRouterRule);

    if (d->methods && !(d->methods & request.method()))
        return false;

    *match = d->pathRegexp.match(request.url().path());
    return (match->hasMatch() && d->pathRegexp.captureCount() == match->lastCapturedIndex());
}

/*!
    \internal
*/
bool QHttpServerRouterRule::createPathRegexp(const std::initializer_list<int> &metaTypes,
                                             const QMap<int, QLatin1String> &converters)
{
    Q_D(QHttpServerRouterRule);

    QString pathRegexp = d->pathPattern;
    const QLatin1String arg("<arg>");
    for (auto type : metaTypes) {
        if (type >= QMetaType::User
            && !QMetaType::hasRegisteredConverterFunction(qMetaTypeId<QString>(), type)) {
            qCWarning(lcRouterRule) << QMetaType::typeName(type)
                                    << "has not registered a converter to QString."
                                    << "Use QHttpServerRouter::addConveter<Type>(converter).";
            return false;
        }

        auto it = converters.constFind(type);
        if (it == converters.end()) {
            qCWarning(lcRouterRule) << "Can not find converter for type:"
                                    << QMetaType::typeName(type);
            return false;
        }

        if (it->isEmpty())
            continue;

        const auto index = pathRegexp.indexOf(arg);
        const QString &regexp = QLatin1Char('(') % *it % QLatin1Char(')');
        if (index == -1)
            pathRegexp.append(regexp);
        else
            pathRegexp.replace(index, arg.size(), regexp);
    }

    if (pathRegexp.indexOf(arg) != -1) {
        qCWarning(lcRouterRule) << "not enough types or one of the types is not supported, regexp:"
                                << pathRegexp
                                << ", pattern:" << d->pathPattern
                                << ", types:" << std::list<int>(metaTypes);
        return false;
    }

    if (!pathRegexp.startsWith(QLatin1Char('^')))
        pathRegexp = QLatin1Char('^') % pathRegexp;
    if (!pathRegexp.endsWith(QLatin1Char('$')))
        pathRegexp += QLatin1String("$");

    qCDebug(lcRouterRule) << "url pathRegexp:" << pathRegexp;

    d->pathRegexp.setPattern(pathRegexp);
    d->pathRegexp.optimize();
    return true;
}

QT_END_NAMESPACE
