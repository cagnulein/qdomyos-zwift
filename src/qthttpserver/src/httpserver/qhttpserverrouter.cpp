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

#include "qhttpserverrouter_p.h"

#include <QtHttpServer/qhttpserverrouter.h>
#include <QtHttpServer/qhttpserverrouterrule.h>
#include <QtHttpServer/qhttpserverrequest.h>

#include <private/qhttpserverrouterrule_p.h>

#include <QtCore/qloggingcategory.h>
#include <QtCore/qmetatype.h>

QT_BEGIN_NAMESPACE

Q_LOGGING_CATEGORY(lcRouter, "qt.httpserver.router")

static const QMap<int, QLatin1String> defaultConverters = {
    { QMetaType::Int, QLatin1String("[+-]?\\d+") },
    { QMetaType::Long, QLatin1String("[+-]?\\d+") },
    { QMetaType::LongLong, QLatin1String("[+-]?\\d+") },
    { QMetaType::Short, QLatin1String("[+-]?\\d+") },

    { QMetaType::UInt, QLatin1String("[+]?\\d+") },
    { QMetaType::ULong, QLatin1String("[+]?\\d+") },
    { QMetaType::ULongLong, QLatin1String("[+]?\\d+") },
    { QMetaType::UShort, QLatin1String("[+]?\\d+") },

    { QMetaType::Double, QLatin1String("[+-]?(?:[0-9]+(?:[.][0-9]*)?|[.][0-9]+)") },
    { QMetaType::Float, QLatin1String("[+-]?(?:[0-9]+(?:[.][0-9]*)?|[.][0-9]+)") },

    { QMetaType::QString, QLatin1String("[^/]+") },
    { QMetaType::QByteArray, QLatin1String("[^/]+") },

    { QMetaType::QUrl, QLatin1String(".*") },

    { QMetaType::Void, QLatin1String("") },
};

/*!
    \class QHttpServerRouter
    \brief Provides functions to bind a URL to a \c ViewHandler.

    You can register \c ViewHandler as a callback for requests to a specific URL.
    Variable parts in the route can be specified by the arguments in ViewHandler.

    \note This is a low-level routing API for an HTTP server.

    See the following example:

    \code
    auto pageView = [] (const quint64 page) {
        qDebug() << "page" << page;
    };
    using ViewHandler = decltype(pageView);

    QHttpServerRouter router;

    // register callback pageView on request "/page/<number>"
    // for example: "/page/10", "/page/15"
    router.addRoute<ViewHandler>(
        new QHttpServerRouterRule("/page/", [=] (QRegularExpressionMatch &match,
                                                 const QHttpServerRequest &,
                                                 QTcpSocket *) {
        auto boundView = router.bindCaptured(pageView, match);

        // it calls pageView
        boundView();
    }));
    \endcode
*/

/*! \fn template <typename Type> bool QHttpServerRouter::addConverter(const QLatin1String &regexp)

    Adds a new converter for type \e Type matching regular expression \a regexp.

    Automatically try to register an implicit converter from QString to \e Type.
    If there is already a converter of type \e Type, that converter's regexp
    is replaced with \a regexp.

    \code
    struct CustomArg {
        int data = 10;

        CustomArg() {} ;
        CustomArg(const QString &urlArg) : data(urlArg.toInt()) {}
    };
    Q_DECLARE_METATYPE(CustomArg);

    QHttpServerRouter router;
    router.addConverter<CustomArg>(QLatin1String("[+-]?\\d+"));

    auto pageView = [] (const CustomArg &customArg) {
        qDebug("data: %d", customArg.data);
    };
    using ViewHandler = decltype(pageView);

    auto rule = new QHttpServerRouterRule(
        "/<arg>/<arg>/log",
        [&router, &pageView] (QRegularExpressionMatch &match,
                              const QHttpServerRequest &request,
                              QTcpSocket *socket) {
        // Bind and call viewHandler with match's captured string and quint32:
        router.bindCaptured(pageView, match)();
    });

    router.addRule<ViewHandler>(rule);
    \endcode
*/

/*! \fn template <typename ViewHandler, typename ViewTraits = QHttpServerRouterViewTraits<ViewHandler>> bool QHttpServerRouter::addRule(QHttpServerRouterRule *rule)

    Adds a new \a rule.

    Inside addRule, we determine ViewHandler arguments and generate a list of
    their QMetaType::Type ids. Then we parse the URL and replace each \c <arg>
    with a regexp for its type from the list.

    \code
    QHttpServerRouter router;

    using ViewHandler = decltype([] (const QString &page, const quint32 num) { });

    auto rule = new QHttpServerRouterRule(
        "/<arg>/<arg>/log",
        [] (QRegularExpressionMatch &match,
            const QHttpServerRequest &request,
            QTcpSocket *socket) {
    });

    router.addRule<ViewHandler>(rule);
    \endcode

    \note This function takes over ownership of \a rule.
*/

/*! \fn template<typename ViewHandler, typename ViewTraits = QHttpServerRouterViewTraits<ViewHandler>> auto bindCaptured(ViewHandler &&handler, QRegularExpressionMatch &match) const -> typename ViewTraits::BindableType

    Supplies the \a handler with arguments derived from a URL.
    Returns the bound function that accepts whatever remaining arguments the handler may take,
    supplying them to the handler after the URL-derived values.
    Each match of the regex applied to the URL (as a string) is converted to the type
    of the handler's parameter at its position, so that passing it works.

    \code
    QHttpServerRouter router;

    auto pageView = [] (const QString &page, const quint32 num) {
        qDebug("page: %s, num: %d", qPrintable(page), num);
    };
    using ViewHandler = decltype(pageView);

    auto rule = new QHttpServerRouterRule(
        "/<arg>/<arg>/log",
        [&router, &pageView] (QRegularExpressionMatch &match,
                              const QHttpServerRequest &request,
                              QTcpSocket *socket) {
        // Bind and call viewHandler with match's captured string and quint32:
        router.bindCaptured(pageView, match)();
    });

    router.addRule<ViewHandler>(rule);
    \endcode
*/

QHttpServerRouterPrivate::QHttpServerRouterPrivate()
    : converters(defaultConverters)
{}

/*!
    Creates a QHttpServerRouter object with \c defaultConverters.

    \sa defaultConverters()
*/
QHttpServerRouter::QHttpServerRouter()
    : d_ptr(new QHttpServerRouterPrivate)
{}

/*!
    Destroys a QHttpServerRouter.
*/
QHttpServerRouter::~QHttpServerRouter()
{}

/*!
    Adds a new converter for type \a type matching regular expression \a regexp.

    If there is already a converter of type \a type, that converter's regexp
    is replaced with \a regexp.
*/
void QHttpServerRouter::addConverter(const int type, const QLatin1String &regexp)
{
    Q_D(QHttpServerRouter);
    d->converters[type] = regexp;
}

/*!
    Removes the converter for type \a type.
*/
void QHttpServerRouter::removeConverter(const int type)
{
    Q_D(QHttpServerRouter);
    d->converters.remove(type);
}

/*!
    Removes all converters.

    \note clearConverters() does not set up \c defaultConverters.

    \sa defaultConverters()
*/
void QHttpServerRouter::clearConverters()
{
    Q_D(QHttpServerRouter);
    d->converters.clear();
}

/*!
    Returns a map of converter type and regexp.
*/
const QMap<int, QLatin1String> &QHttpServerRouter::converters() const
{
    Q_D(const QHttpServerRouter);
    return d->converters;
}

/*!
    Returns a map of default converter type and regexp.
    The following converters are available by default:

    \value QMetaType::Int
    \value QMetaType::Long
    \value QMetaType::LongLong
    \value QMetaType::Short
    \value QMetaType::UInt
    \value QMetaType::ULong
    \value QMetaType::ULongLong
    \value QMetaType::UShort
    \value QMetaType::Double
    \value QMetaType::Float
    \value QMetaType::QString
    \value QMetaType::QByteArray
    \value QMetaType::QUrl
    \value QMetaType::Void       An empty converter.
*/
const QMap<int, QLatin1String> &QHttpServerRouter::defaultConverters()
{
    return ::defaultConverters;
}

bool QHttpServerRouter::addRuleImpl(QHttpServerRouterRule *rule,
                                    const std::initializer_list<int> &types)
{
    Q_D(QHttpServerRouter);

    if (!rule->hasValidMethods() || !rule->createPathRegexp(types, d->converters)) {
        delete rule;
        return false;
    }

    d->rules.emplace_back(rule);
    return true;
}

/*!
    Handles each new request for the HTTP server.

    Iterates through the list of rules to find the first that matches,
    then executes this rule, returning \c true. Returns \c false if no rule
    matches the request.
*/
bool QHttpServerRouter::handleRequest(const QHttpServerRequest &request,
                                      QTcpSocket *socket) const
{
    Q_D(const QHttpServerRouter);
    for (const auto &rule : qAsConst(d->rules)) {
        if (rule->exec(request, socket))
            return true;
    }

    return false;
}

QT_END_NAMESPACE
