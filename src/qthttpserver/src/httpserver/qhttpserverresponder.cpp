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

#include <QtHttpServer/qhttpserverresponder.h>
#include <QtHttpServer/qhttpserverrequest.h>
#include <private/qhttpserverresponder_p.h>
#include <private/qhttpserverliterals_p.h>
#include <private/qhttpserverrequest_p.h>
#include <QtCore/qjsondocument.h>
#include <QtCore/qloggingcategory.h>
#include <QtCore/qtimer.h>
#include <QtNetwork/qtcpsocket.h>
#include <map>
#include <memory>

#include "../3rdparty/http-parser/http_parser.h"

QT_BEGIN_NAMESPACE

static const QLoggingCategory &lc()
{
    static const QLoggingCategory category("qt.httpserver.response");
    return category;
}

// https://www.w3.org/Protocols/rfc2616/rfc2616-sec10.html
static const std::map<QHttpServerResponder::StatusCode, QByteArray> statusString {
#define XX(num, name, string) { static_cast<QHttpServerResponder::StatusCode>(num), QByteArrayLiteral(#string) },
  HTTP_STATUS_MAP(XX)
#undef XX
};

template <qint64 BUFFERSIZE = 512>
struct IOChunkedTransfer
{
    // TODO This is not the fastest implementation, as it does read & write
    // in a sequential fashion, but these operation could potentially overlap.
    // TODO Can we implement it without the buffer? Direct write to the target buffer
    // would be great.

    const qint64 bufferSize = BUFFERSIZE;
    char buffer[BUFFERSIZE];
    qint64 beginIndex = -1;
    qint64 endIndex = -1;
    QPointer<QIODevice> source;
    const QPointer<QIODevice> sink;
    const QMetaObject::Connection bytesWrittenConnection;
    const QMetaObject::Connection readyReadConnection;
    IOChunkedTransfer(QIODevice *input, QIODevice *output) :
        source(input),
        sink(output),
        bytesWrittenConnection(QObject::connect(sink.data(), &QIODevice::bytesWritten, [this] () {
              writeToOutput();
        })),
        readyReadConnection(QObject::connect(source.data(), &QIODevice::readyRead, [this] () {
            readFromInput();
        }))
    {
        Q_ASSERT(!source->atEnd());  // TODO error out
        QObject::connect(sink.data(), &QObject::destroyed, source.data(), &QObject::deleteLater);
        QObject::connect(source.data(), &QObject::destroyed, [this] () {
            delete this;
        });
        readFromInput();
    }

    ~IOChunkedTransfer()
    {
        QObject::disconnect(bytesWrittenConnection);
        QObject::disconnect(readyReadConnection);
    }

    inline bool isBufferEmpty()
    {
        Q_ASSERT(beginIndex <= endIndex);
        return beginIndex == endIndex;
    }

    void readFromInput()
    {
        if (!isBufferEmpty()) // We haven't consumed all the data yet.
            return;
        beginIndex = 0;
        endIndex = source->read(buffer, bufferSize);
        if (endIndex < 0) {
            endIndex = beginIndex; // Mark the buffer as empty
            qCWarning(lc, "Error reading chunk: %s", qPrintable(source->errorString()));
        } else if (endIndex) {
            memset(buffer + endIndex, 0, sizeof(buffer) - std::size_t(endIndex));
            writeToOutput();
        }
    }

    void writeToOutput()
    {
        if (isBufferEmpty())
            return;

        const auto writtenBytes = sink->write(buffer + beginIndex, endIndex);
        if (writtenBytes < 0) {
            qCWarning(lc, "Error writing chunk: %s", qPrintable(sink->errorString()));
            return;
        }
        beginIndex += writtenBytes;
        if (isBufferEmpty()) {
            if (source->bytesAvailable())
                QTimer::singleShot(0, source.data(), [this]() { readFromInput(); });
            else if (source->atEnd()) // Finishing
                source->deleteLater();
        }
    }
};

/*!
    Constructs a QHttpServerResponder using the request \a request
    and the socket \a socket.
*/
QHttpServerResponder::QHttpServerResponder(const QHttpServerRequest &request,
                                           QTcpSocket *socket) :
    d_ptr(new QHttpServerResponderPrivate(request, socket))
{
    Q_ASSERT(socket);
}

/*!
    Move-constructs a QHttpServerResponder instance, making it point
    at the same object that \a other was pointing to.
*/
QHttpServerResponder::QHttpServerResponder(QHttpServerResponder &&other) :
    d_ptr(other.d_ptr.take())
{}

/*!
    Destroys a QHttpServerResponder.
*/
QHttpServerResponder::~QHttpServerResponder()
{}

/*!
    Answers a request with an HTTP status code \a status and
    HTTP headers \a headers. The I/O device \a data provides the body
    of the response. If \a data is sequential, the body of the
    message is sent in chunks: otherwise, the function assumes all
    the content is available and sends it all at once but the read
    is done in chunks.

    \note This function takes the ownership of \a data.
*/
void QHttpServerResponder::write(QIODevice *data,
                                 HeaderList headers,
                                 StatusCode status)
{
    Q_D(QHttpServerResponder);
    Q_ASSERT(d->socket);
    QScopedPointer<QIODevice, QScopedPointerDeleteLater> input(data);

    input->setParent(nullptr);
    if (!input->isOpen()) {
        if (!input->open(QIODevice::ReadOnly)) {
            // TODO Add developer error handling
            qCDebug(lc, "500: Could not open device %s", qPrintable(input->errorString()));
            write(StatusCode::InternalServerError);
            return;
        }
    } else if (!(input->openMode() & QIODevice::ReadOnly)) {
        // TODO Add developer error handling
        qCDebug(lc) << "500: Device is opened in a wrong mode" << input->openMode();
        write(StatusCode::InternalServerError);
        return;
    }

    if (!d->socket->isOpen()) {
        qCWarning(lc, "Cannot write to socket. It's disconnected");
        return;
    }

    writeStatusLine(status);

    if (!input->isSequential()) { // Non-sequential QIODevice should know its data size
        writeHeader(QHttpServerLiterals::contentLengthHeader(),
                    QByteArray::number(input->size()));
    }

    for (auto &&header : headers)
        writeHeader(header.first, header.second);

    d->socket->write("\r\n");

    if (input->atEnd()) {
        qCDebug(lc, "No more data available.");
        return;
    }

    // input takes ownership of the IOChunkedTransfer pointer inside his constructor
    new IOChunkedTransfer<>(input.take(), d->socket);
}

/*!
    Answers a request with an HTTP status code \a status and a
    MIME type \a mimeType. The I/O device \a data provides the body
    of the response. If \a data is sequential, the body of the
    message is sent in chunks: otherwise, the function assumes all
    the content is available and sends it all at once but the read
    is done in chunks.

    \note This function takes the ownership of \a data.
*/
void QHttpServerResponder::write(QIODevice *data,
                                 const QByteArray &mimeType,
                                 StatusCode status)
{
    write(data,
          {{ QHttpServerLiterals::contentTypeHeader(), mimeType }},
          status);
}

/*!
    Answers a request with an HTTP status code \a status, JSON
    document \a document and HTTP headers \a headers.

    Note: This function sets HTTP Content-Type header as "application/json".
*/
void QHttpServerResponder::write(const QJsonDocument &document,
                                 HeaderList headers,
                                 StatusCode status)
{
    const QByteArray &json = document.toJson();

    writeStatusLine(status);
    writeHeader(QHttpServerLiterals::contentTypeHeader(),
                QHttpServerLiterals::contentTypeJson());
    writeHeader(QHttpServerLiterals::contentLengthHeader(),
                QByteArray::number(json.size()));
    writeHeaders(std::move(headers));
    writeBody(document.toJson());
}

/*!
    Answers a request with an HTTP status code \a status, and JSON
    document \a document.

    Note: This function sets HTTP Content-Type header as "application/json".
*/
void QHttpServerResponder::write(const QJsonDocument &document,
                                 StatusCode status)
{
    write(document, {}, status);
}

/*!
    Answers a request with an HTTP status code \a status,
    HTTP Headers \a headers and a body \a data.

    Note: This function sets HTTP Content-Length header.
*/
void QHttpServerResponder::write(const QByteArray &data,
                                 HeaderList headers,
                                 StatusCode status)
{
    writeStatusLine(status);

    for (auto &&header : headers)
        writeHeader(header.first, header.second);

    writeHeader(QHttpServerLiterals::contentLengthHeader(),
                QByteArray::number(data.size()));
    writeBody(data);
}

/*!
    Answers a request with an HTTP status code \a status, a
    MIME type \a mimeType and a body \a data.
*/
void QHttpServerResponder::write(const QByteArray &data,
                                 const QByteArray &mimeType,
                                 StatusCode status)
{
    write(data,
          {{ QHttpServerLiterals::contentTypeHeader(), mimeType }},
          status);
}

/*!
    Answers a request with an HTTP status code \a status.

    Note: This function sets HTTP Content-Type header as "application/x-empty".
*/
void QHttpServerResponder::write(StatusCode status)
{
    write(QByteArray(), QHttpServerLiterals::contentTypeXEmpty(), status);
}

/*!
    Answers a request with an HTTP status code \a status and
    HTTP Headers \a headers.
*/
void QHttpServerResponder::write(HeaderList headers, StatusCode status)
{
    write(QByteArray(), std::move(headers), status);
}

/*!
    This function writes HTTP status line with an HTTP status code \a status
    and an HTTP version \a version.
*/
void QHttpServerResponder::writeStatusLine(StatusCode status,
                                           const QPair<quint8, quint8> &version)
{
    Q_D(const QHttpServerResponder);
    Q_ASSERT(d->socket->isOpen());
    d->socket->write("HTTP/");
    d->socket->write(QByteArray::number(version.first));
    d->socket->write(".");
    d->socket->write(QByteArray::number(version.second));
    d->socket->write(" ");
    d->socket->write(QByteArray::number(quint32(status)));
    d->socket->write(" ");
    d->socket->write(statusString.at(status));
    d->socket->write("\r\n");
}

/*!
    This function writes an HTTP header \a header
    with \a value.
*/
void QHttpServerResponder::writeHeader(const QByteArray &header,
                                       const QByteArray &value)
{
    Q_D(const QHttpServerResponder);
    Q_ASSERT(d->socket->isOpen());
    d->socket->write(header);
    d->socket->write(": ");
    d->socket->write(value);
    d->socket->write("\r\n");
}

/*!
    This function writes HTTP headers \a headers.
*/
void QHttpServerResponder::writeHeaders(HeaderList headers)
{
    for (auto &&header : headers)
        writeHeader(header.first, header.second);
}

/*!
    This function writes HTTP body \a body with size \a size.
*/
void QHttpServerResponder::writeBody(const char *body, qint64 size)
{
    Q_D(QHttpServerResponder);
    Q_ASSERT(d->socket->isOpen());

    if (!d->bodyStarted) {
        d->socket->write("\r\n");
        d->bodyStarted = true;
    }

    d->socket->write(body, size);
}

/*!
    This function writes HTTP body \a body.
*/
void QHttpServerResponder::writeBody(const char *body)
{
    writeBody(body, qstrlen(body));
}

/*!
    This function writes HTTP body \a body.
*/
void QHttpServerResponder::writeBody(const QByteArray &body)
{
    writeBody(body.constData(), body.size());
}

/*!
    Returns the socket used.
*/
QTcpSocket *QHttpServerResponder::socket() const
{
    Q_D(const QHttpServerResponder);
    return d->socket;
}

QT_END_NAMESPACE
