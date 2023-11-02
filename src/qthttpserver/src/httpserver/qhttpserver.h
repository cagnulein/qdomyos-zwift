/****************************************************************************
**
** Copyright (C) 2020 Mikhail Svetkin <mikhail.svetkin@gmail.com>
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

#ifndef QHTTPSERVER_H
#define QHTTPSERVER_H

#include <QtHttpServer/qabstracthttpserver.h>
#include <QtHttpServer/qhttpserverrouter.h>
#include <QtHttpServer/qhttpserverrouterrule.h>
#include <QtHttpServer/qhttpserverresponse.h>
#include <QtHttpServer/qhttpserverrouterviewtraits.h>
#include <QtHttpServer/qhttpserverviewtraits.h>

#include <tuple>

QT_BEGIN_NAMESPACE

class QTcpSocket;
class QHttpServerRequest;

class QHttpServerPrivate;
class Q_HTTPSERVER_EXPORT QHttpServer final : public QAbstractHttpServer
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QHttpServer)

    template<int I, typename ... Ts>
    struct VariadicTypeAt { using Type = typename std::tuple_element<I, std::tuple<Ts...>>::type; };

    template<typename ... Ts>
    struct VariadicTypeLast {
        using Type = typename VariadicTypeAt<sizeof ... (Ts) - 1, Ts...>::Type;
    };


    template<typename T>
    using ResponseType =
        typename std::conditional<
            std::is_base_of<QHttpServerResponse, T>::value,
            T,
            QHttpServerResponse
        >::type;

public:
    explicit QHttpServer(QObject *parent = nullptr);
    ~QHttpServer();

    QHttpServerRouter *router();

    template<typename Rule = QHttpServerRouterRule, typename ... Args>
    bool route(Args && ... args)
    {
        using ViewHandler = typename VariadicTypeLast<Args...>::Type;
        using ViewTraits = QHttpServerRouterViewTraits<ViewHandler>;
        static_assert(ViewTraits::Arguments::StaticAssert,
                      "ViewHandler arguments are in the wrong order or not supported");
        return routeHelper<Rule, ViewHandler, ViewTraits>(
                QtPrivate::makeIndexSequence<sizeof ... (Args) - 1>{},
                std::forward<Args>(args)...);
    }

    template<typename ViewHandler>
    void afterRequest(ViewHandler &&viewHandler)
    {
        using ViewTraits = QHttpServerAfterRequestViewTraits<ViewHandler>;
        static_assert(ViewTraits::Arguments::StaticAssert,
                      "ViewHandler arguments are in the wrong order or not supported");
        afterRequestHelper<ViewTraits, ViewHandler>(std::move(viewHandler));
    }

    using AfterRequestHandler =
        std::function<QHttpServerResponse(QHttpServerResponse &&response,
                      const QHttpServerRequest &request)>;
private:
    template<typename ViewTraits, typename ViewHandler>
    typename std::enable_if<ViewTraits::Arguments::Last::IsRequest::Value &&
                            ViewTraits::Arguments::Count == 2, void>::type
            afterRequestHelper(ViewHandler &&viewHandler) {
        auto handler = [viewHandler](QHttpServerResponse &&resp,
                                     const QHttpServerRequest &request) {
            return std::move(viewHandler(std::move(resp), request));
        };

        afterRequestImpl(std::move(handler));
    }

    template<typename ViewTraits, typename ViewHandler>
    typename std::enable_if<ViewTraits::Arguments::Last::IsResponse::Value &&
                            ViewTraits::Arguments::Count == 1, void>::type
            afterRequestHelper(ViewHandler &&viewHandler) {
        auto handler = [viewHandler](QHttpServerResponse &&resp,
                                     const QHttpServerRequest &) {
            return std::move(viewHandler(std::move(resp)));
        };

        afterRequestImpl(std::move(handler));
    }

    template<typename ViewTraits, typename ViewHandler>
    typename std::enable_if<ViewTraits::Arguments::Last::IsResponse::Value &&
                            ViewTraits::Arguments::Count == 2, void>::type
            afterRequestHelper(ViewHandler &&viewHandler) {
        auto handler = [viewHandler](QHttpServerResponse &&resp,
                                     const QHttpServerRequest &request) {
            return std::move(viewHandler(request, std::move(resp)));
        };

        afterRequestImpl(std::move(handler));
    }

    void afterRequestImpl(AfterRequestHandler &&afterRequestHandler);

private:
    template<typename Rule, typename ViewHandler, typename ViewTraits, int ... I, typename ... Args>
    bool routeHelper(QtPrivate::IndexesList<I...>, Args &&... args)
    {
        return routeImpl<Rule,
                         ViewHandler,
                         ViewTraits,
                         typename VariadicTypeAt<I, Args...>::Type...>(std::forward<Args>(args)...);
    }

    template<typename Rule, typename ViewHandler, typename ViewTraits, typename ... Args>
    bool routeImpl(Args &&...args, ViewHandler &&viewHandler)
    {
        auto routerHandler = [this, viewHandler] (
                    const QRegularExpressionMatch &match,
                    const QHttpServerRequest &request,
                    QTcpSocket *socket) mutable {
            auto boundViewHandler = router()->bindCaptured<ViewHandler, ViewTraits>(
                    std::move(viewHandler), match);
            responseImpl<ViewTraits>(boundViewHandler, request, socket);
        };

        return router()->addRule<ViewHandler, ViewTraits>(
                new Rule(std::forward<Args>(args)..., std::move(routerHandler)));
    }

    template<typename ViewTraits, typename T>
    typename std::enable_if<!ViewTraits::Arguments::Last::IsSpecial::Value, void>::type
            responseImpl(T &boundViewHandler,
                         const QHttpServerRequest &request,
                         QTcpSocket *socket)
    {
        ResponseType<typename ViewTraits::ReturnType> response(boundViewHandler());
        sendResponse(std::move(response), request, socket);
    }

    template<typename ViewTraits, typename T>
    typename std::enable_if<ViewTraits::Arguments::Last::IsRequest::Value &&
                            ViewTraits::Arguments::PlaceholdersCount == 1, void>::type
            responseImpl(T &boundViewHandler, const QHttpServerRequest &request, QTcpSocket *socket)
    {
        ResponseType<typename ViewTraits::ReturnType> response(boundViewHandler(request));
        sendResponse(std::move(response), request, socket);
    }

    template<typename ViewTraits, typename T>
    typename std::enable_if<ViewTraits::Arguments::Last::IsRequest::Value &&
                            ViewTraits::Arguments::PlaceholdersCount == 2, void>::type
            responseImpl(T &boundViewHandler, const QHttpServerRequest &request, QTcpSocket *socket)
    {
        boundViewHandler(makeResponder(request, socket), request);
    }

    template<typename ViewTraits, typename T>
    typename std::enable_if<ViewTraits::Arguments::Last::IsResponder::Value &&
                            ViewTraits::Arguments::PlaceholdersCount == 2, void>::type
            responseImpl(T &boundViewHandler,
                         const QHttpServerRequest &request,
                         QTcpSocket *socket)
    {
        boundViewHandler(request, makeResponder(request, socket));
    }

    template<typename ViewTraits, typename T>
    typename std::enable_if<ViewTraits::Arguments::Last::IsResponder::Value &&
                            ViewTraits::Arguments::PlaceholdersCount == 1, void>::type
            responseImpl(T &boundViewHandler,
                         const QHttpServerRequest &request,
                         QTcpSocket *socket)
    {
        boundViewHandler(makeResponder(request, socket));
    }

    bool handleRequest(const QHttpServerRequest &request, QTcpSocket *socket) override final;

    void sendResponse(QHttpServerResponse &&response,
                      const QHttpServerRequest &request,
                      QTcpSocket *socket);
};

QT_END_NAMESPACE

#endif  // QHTTPSERVER_H
