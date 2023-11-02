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

#ifndef QHTTPSERVERFUTURERESPONSE_H
#define QHTTPSERVERFUTURERESPONSE_H

#include <QtHttpServer/qhttpserverresponse.h>

#include <QtCore/qbytearray.h>
#include <QtCore/qfuture.h>

#include <QtConcurrent>

#include <mutex>

QT_BEGIN_NAMESPACE

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)

template <>
class QFutureInterface<QHttpServerResponse> : public QFutureInterfaceBase
{
public:
    QFutureInterface(State initialState = NoState)
        : QFutureInterfaceBase(initialState)
    {
        refT();
    }
    QFutureInterface(const QFutureInterface &other)
        : QFutureInterfaceBase(other)
    {
        refT();
    }
    ~QFutureInterface()
    {
        if (!derefT())
            resultStoreBase().template clear<QHttpServerResponse>();
    }

    static QFutureInterface canceledResult()
    { return QFutureInterface(State(Started | Finished | Canceled)); }

    QFutureInterface &operator=(const QFutureInterface &other)
    {
        other.refT();
        if (!derefT())
            resultStoreBase().template clear<QHttpServerResponse>();
        QFutureInterfaceBase::operator=(other);
        return *this;
    }

    inline QFuture<QHttpServerResponse> future()
    {
        return QFuture<QHttpServerResponse>(this);
    }

    void reportAndMoveResult(QHttpServerResponse &&result, int index = -1)
    {
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
        std::lock_guard<QMutex> locker{*mutex()};
#else
        std::lock_guard<QMutex> locker{mutex(0)};
#endif
        if (queryState(Canceled) || queryState(Finished))
            return;

        QtPrivate::ResultStoreBase &store = resultStoreBase();

        const int oldResultCount = store.count();
        const int insertIndex = store.addResult(
                index, static_cast<void *>(new QHttpServerResponse(std::move_if_noexcept(result))));
        if (!store.filterMode() || oldResultCount < store.count()) // Let's make sure it's not in pending results.
            reportResultsReady(insertIndex, store.count());
    }

    void reportFinished()
    {
        QFutureInterfaceBase::reportFinished();
    }

    QHttpServerResponse takeResult();
};

#endif // QT_VERSION < QT_VERSION_CHECK(6, 0, 0)

namespace QtConcurrent {

template <>
class RunFunctionTask<QHttpServerResponse> : public RunFunctionTaskBase<QHttpServerResponse>
{
public:
    void run() override
    {
        if (this->isCanceled()) {
            this->reportFinished();
            return;
        }
#ifndef QT_NO_EXCEPTIONS
        try {
#endif
            this->runFunctor();
#ifndef QT_NO_EXCEPTIONS
        } catch (QException &e) {
            QFutureInterface<QHttpServerResponse>::reportException(e);
        } catch (...) {
            QFutureInterface<QHttpServerResponse>::reportException(QUnhandledException());
        }
#endif
        this->reportAndMoveResult(std::move_if_noexcept(result));
        this->reportFinished();
    }

    QHttpServerResponse result{QHttpServerResponse::StatusCode::NotFound};
};

}

class QHttpServerFutureResponsePrivate;
class Q_HTTPSERVER_EXPORT QHttpServerFutureResponse : public QHttpServerResponse
{
    Q_DECLARE_PRIVATE(QHttpServerFutureResponse)

public:
    using QHttpServerResponse::QHttpServerResponse;

    QHttpServerFutureResponse(const QFuture<QHttpServerResponse> &futureResponse);

    virtual void write(QHttpServerResponder &&responder) const override;

protected:
    QHttpServerFutureResponse(QHttpServerFutureResponsePrivate *d);
};

QT_END_NAMESPACE

#endif // QHTTPSERVERFUTURERESPONSE_H
