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

#ifndef QHTTPSERVERRESPONDER_H
#define QHTTPSERVERRESPONDER_H

#include <QtHttpServer/qthttpserverglobal.h>

#include <QtCore/qdebug.h>
#include <QtCore/qpair.h>
#include <QtCore/qglobal.h>
#include <QtCore/qstring.h>
#include <QtCore/qscopedpointer.h>
#include <QtCore/qmetatype.h>
#include <QtCore/qmimetype.h>

#include <utility>
#include <initializer_list>

QT_BEGIN_NAMESPACE

class QTcpSocket;
class QHttpServerRequest;

class QHttpServerResponderPrivate;
class Q_HTTPSERVER_EXPORT QHttpServerResponder final
{
    Q_DECLARE_PRIVATE(QHttpServerResponder)

    friend class QAbstractHttpServer;

public:
    enum class StatusCode {
        // 1xx: Informational
        Continue = 100,
        SwitchingProtocols,
        Processing,

        // 2xx: Success
        Ok = 200,
        Created,
        Accepted,
        NonAuthoritativeInformation,
        NoContent,
        ResetContent,
        PartialContent,
        MultiStatus,
        AlreadyReported,
        IMUsed = 226,

        // 3xx: Redirection
        MultipleChoices = 300,
        MovedPermanently,
        Found,
        SeeOther,
        NotModified,
        UseProxy,
        // 306: not used, was proposed as "Switch Proxy" but never standardized
        TemporaryRedirect = 307,
        PermanentRedirect,

        // 4xx: Client Error
        BadRequest = 400,
        Unauthorized,
        PaymentRequired,
        Forbidden,
        NotFound,
        MethodNotAllowed,
        NotAcceptable,
        ProxyAuthenticationRequired,
        RequestTimeout,
        Conflict,
        Gone,
        LengthRequired,
        PreconditionFailed,
        PayloadTooLarge,
        UriTooLong,
        UnsupportedMediaType,
        RequestRangeNotSatisfiable,
        ExpectationFailed,
        ImATeapot,
        MisdirectedRequest = 421,
        UnprocessableEntity,
        Locked,
        FailedDependency,
        UpgradeRequired = 426,
        PreconditionRequired = 428,
        TooManyRequests,
        RequestHeaderFieldsTooLarge = 431,
        UnavailableForLegalReasons = 451,

        // 5xx: Server Error
        InternalServerError = 500,
        NotImplemented,
        BadGateway,
        ServiceUnavailable,
        GatewayTimeout,
        HttpVersionNotSupported,
        VariantAlsoNegotiates,
        InsufficientStorage,
        LoopDetected,
        NotExtended = 510,
        NetworkAuthenticationRequired,
        NetworkConnectTimeoutError = 599,
    };

    using HeaderList = std::initializer_list<std::pair<QByteArray, QByteArray>>;

    QHttpServerResponder(QHttpServerResponder &&other);
    ~QHttpServerResponder();

    void write(QIODevice *data,
               HeaderList headers,
               StatusCode status = StatusCode::Ok);

    void write(QIODevice *data,
               const QByteArray &mimeType,
               StatusCode status = StatusCode::Ok);

    void write(const QJsonDocument &document,
               HeaderList headers,
               StatusCode status = StatusCode::Ok);

    void write(const QJsonDocument &document,
               StatusCode status = StatusCode::Ok);

    void write(const QByteArray &data,
               HeaderList headers,
               StatusCode status = StatusCode::Ok);

    void write(const QByteArray &data,
               const QByteArray &mimeType,
               StatusCode status = StatusCode::Ok);

    void write(HeaderList headers, StatusCode status = StatusCode::Ok);
    void write(StatusCode status = StatusCode::Ok);


    void writeStatusLine(StatusCode status = StatusCode::Ok,
                         const QPair<quint8, quint8> &version = qMakePair(1u, 1u));

    void writeHeader(const QByteArray &key, const QByteArray &value);
    void writeHeaders(HeaderList headers);

    void writeBody(const char *body, qint64 size);
    void writeBody(const char *body);
    void writeBody(const QByteArray &body);

    QTcpSocket *socket() const;

private:
    QHttpServerResponder(const QHttpServerRequest &request, QTcpSocket *socket);

    QScopedPointer<QHttpServerResponderPrivate> d_ptr;
};

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QHttpServerResponder::StatusCode)

#endif // QHTTPSERVERRESPONDER_H
