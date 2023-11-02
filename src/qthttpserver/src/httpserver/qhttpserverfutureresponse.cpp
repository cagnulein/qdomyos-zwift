/****************************************************************************
**
** Copyright (C) 2020 Mikhail Svetkin <mikhail.svetkin@gmail.com>
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

#include "qhttpserverfutureresponse.h"

#include <QtCore/qfuture.h>
#include <QtCore/qfuturewatcher.h>

#include <QtNetwork/qtcpsocket.h>

#include <QtHttpServer/qhttpserverresponder.h>

#include <private/qhttpserverresponse_p.h>


QT_BEGIN_NAMESPACE

/*!
    \class QHttpServerFutureResponse
    \brief QHttpServerFutureResponse is a simplified API for asynchronous responses.

    \code

    QHttpServer server;

    server.route("/feature/", [] (int id) -> QHttpServerFutureResponse {
        auto future = QtConcurrent::run([] () {
            return QHttpServerResponse("the future is coming");
        });

        return future;
    });
    server.listen();

    \endcode
*/

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)

QHttpServerResponse QFutureInterface<QHttpServerResponse>::takeResult()
{
    if (isCanceled()) {
        exceptionStore().throwPossibleException();
        return QHttpServerResponse::StatusCode::NotFound;
    }
    // Note: we wait for all, this is intentional,
    // not to mess with other unready results.
    waitForResult(-1);

#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    std::lock_guard<QMutex> locker{*mutex()};
#else
    std::lock_guard<QMutex> locker{mutex(0)};
#endif
    QtPrivate::ResultIteratorBase position = resultStoreBase().resultAt(0);
    auto ret = std::move_if_noexcept(
        *const_cast<QHttpServerResponse *>(position.pointer<QHttpServerResponse>()));
    resultStoreBase().template clear<QHttpServerResponse>();

    return ret;
}

#endif // QT_VERSION < QT_VERSION_CHECK(6, 0, 0)

struct QResponseWatcher : public QFutureWatcher<QHttpServerResponse>
{
    Q_OBJECT

public:
    QResponseWatcher(QHttpServerResponder &&_responder)
        : QFutureWatcher<QHttpServerResponse>(),
          responder(std::move(_responder)) {
    }

    QHttpServerResponder responder;
};

class QHttpServerFutureResponsePrivate : public QHttpServerResponsePrivate
{
public:
    QHttpServerFutureResponsePrivate(const QFuture<QHttpServerResponse> &futureResponse)
        : QHttpServerResponsePrivate(),
          futureResp(futureResponse)
    {
    }

    QFuture<QHttpServerResponse> futureResp;
};

/*!
    Constructs a new QHttpServerFutureResponse with the \a future response.
*/
QHttpServerFutureResponse::QHttpServerFutureResponse(const QFuture<QHttpServerResponse> &futureResp)
    : QHttpServerFutureResponse(new QHttpServerFutureResponsePrivate{futureResp})
{
}

/*!
    \internal
*/
QHttpServerFutureResponse::QHttpServerFutureResponse(QHttpServerFutureResponsePrivate *d)
    : QHttpServerResponse(d)
{
}

/*!
    \reimp
*/
void QHttpServerFutureResponse::write(QHttpServerResponder &&responder) const
{
    if (!d_ptr->derived) {
        QHttpServerResponse::write(std::move(responder));
        return;
    }

    Q_D(const QHttpServerFutureResponse);

    auto socket = responder.socket();
    auto futureWatcher = new QResponseWatcher(std::move(responder));

    QObject::connect(socket, &QObject::destroyed,
                     futureWatcher, &QObject::deleteLater);
    QObject::connect(futureWatcher, &QFutureWatcherBase::finished,
                     socket,
                    [futureWatcher] () mutable {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        auto resp = futureWatcher->future().d.takeResult();
#else
        auto resp = futureWatcher->future().takeResult();
#endif
        resp.write(std::move(futureWatcher->responder));
        futureWatcher->deleteLater();
    });

    futureWatcher->setFuture(d->futureResp);
}

QT_END_NAMESPACE

#include "qhttpserverfutureresponse.moc"
