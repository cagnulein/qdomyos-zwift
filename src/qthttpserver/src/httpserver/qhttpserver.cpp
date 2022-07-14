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

#include <QtHttpServer/qhttpserver.h>


#include <QtHttpServer/qhttpserverrequest.h>
#include <QtHttpServer/qhttpserverresponder.h>
#include <QtHttpServer/qhttpserverresponse.h>

#include <private/qhttpserver_p.h>

#include <QtCore/qloggingcategory.h>

#include <QtNetwork/qtcpsocket.h>

QT_BEGIN_NAMESPACE

Q_LOGGING_CATEGORY(lcHS, "qt.httpserver");

/*!
    \class QHttpServer
    \brief QHttpServer is a simplified API for QAbstractHttpServer and QHttpServerRouter.

    \code

    QHttpServer server;

    server.route("/", [] () {
        return "hello world";
    });
    server.listen();

    \endcode
*/

QHttpServer::QHttpServer(QObject *parent)
    : QAbstractHttpServer(*new QHttpServerPrivate, parent)
{
    connect(this, &QAbstractHttpServer::missingHandler, this,
            [=] (const QHttpServerRequest &request, QTcpSocket *socket) {
        qCDebug(lcHS) << tr("missing handler:") << request.url().path();
        sendResponse(QHttpServerResponder::StatusCode::NotFound, request, socket);
    });
}

/*! \fn template<typename Rule = QHttpServerRouterRule, typename ... Args> bool route(Args && ... args)
    This function is just a wrapper to simplify the router API.

    This function takes variadic arguments. The last argument is \c a callback (ViewHandler).
    The remaining arguments are used to create a new \a Rule (the default is QHttpServerRouterRule).
    This is in turn added to the QHttpServerRouter.

    \c ViewHandler can only be a lambda. The lambda definition can take two optional special
    arguments: \c {const QHttpServerRequest&} and \c {QHttpServerResponder&&}.
    These special arguments must be the last in the parameter list.

    Examples:

    \code

    QHttpServer server;

    // Valid:
    server.route("test", [] (const int page) { return ""; });
    server.route("test", [] (const int page, const QHttpServerRequest &request) { return ""; });
    server.route("test", [] (QHttpServerResponder &&responder) { return ""; });

    // Invalid (compile time error):
    server.route("test", [] (const QHttpServerRequest &request, const int page) { return ""; }); // request must be last
    server.route("test", [] (QHttpServerRequest &request) { return ""; });      // request must be passed by const reference
    server.route("test", [] (QHttpServerResponder &responder) { return ""; });  // responder must be passed by universal reference

    \endcode

    \sa QHttpServerRouter::addRule
*/

/*! \fn template<typename ViewHandler> void afterRequest(ViewHandler &&viewHandler)
    Register a function to be run after each request.

    \c ViewHandler can only be a lambda. The lambda definition can take two
    arguments: \c {QHttpServerResponse &&} and \c {const QHttpServerRequest&} (optional).

    Examples:

    \code

    QHttpServer server;

    // Valid:
    server.afterRequest([] (QHttpServerResponse &&resp, const QHttpServerRequest &request) {
        return std::move(resp);
    }
    server.afterRequest([] (const QHttpServerRequest &request, QHttpServerResponse &&resp) {
        return std::move(resp);
    }
    server.afterRequest([] (QHttpServerResponse &&resp) { return std::move(resp); }

    // Invalid (compile time error):
    // resp must be passed by universal reference
    server.afterRequest([] (QHttpServerResponse &resp, const QHttpServerRequest &request) {
        return std::move(resp);
    }
    // request must be passed by const reference
    server.afterRequest([] (QHttpServerResponse &&resp, QHttpServerRequest &request) {
        return std::move(resp);
    }

    \endcode
*/

/*!
    Destroys a QHttpServer.
*/
QHttpServer::~QHttpServer()
{
}

/*!
    Returns the router object.
*/
QHttpServerRouter *QHttpServer::router()
{
    Q_D(QHttpServer);
    return &d->router;
}

void QHttpServer::afterRequestImpl(AfterRequestHandler &&afterRequestHandler)
{
    Q_D(QHttpServer);
    d->afterRequestHandlers.push_back(std::move(afterRequestHandler));
}

/*!
    \internal
*/
void QHttpServer::sendResponse(QHttpServerResponse &&response,
                               const QHttpServerRequest &request,
                               QTcpSocket *socket)
{
    Q_D(QHttpServer);
    for (auto afterRequestHandler : d->afterRequestHandlers)
        response = std::move(afterRequestHandler(std::move(response), request));
    response.write(makeResponder(request, socket));
}

/*!
    \internal
*/
bool QHttpServer::handleRequest(const QHttpServerRequest &request, QTcpSocket *socket)
{
    Q_D(QHttpServer);
    return d->router.handleRequest(request, socket);
}

QT_END_NAMESPACE
