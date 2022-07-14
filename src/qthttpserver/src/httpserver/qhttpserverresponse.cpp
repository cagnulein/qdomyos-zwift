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

#include <QtHttpServer/qhttpserverresponse.h>

#include <private/qhttpserverliterals_p.h>
#include <private/qhttpserverresponse_p.h>
#include <private/qhttpserverresponder_p.h>

#include <QtCore/qfile.h>
#include <QtCore/qjsondocument.h>
#include <QtCore/qjsonobject.h>
#include <QtCore/qmimedatabase.h>
#include <QtNetwork/qtcpsocket.h>

QT_BEGIN_NAMESPACE

QHttpServerResponsePrivate::QHttpServerResponsePrivate(
        QByteArray &&d, const QHttpServerResponse::StatusCode sc)
    : data(std::move(d)),
      statusCode(sc)
{ }

QHttpServerResponsePrivate::QHttpServerResponsePrivate(const QHttpServerResponse::StatusCode sc)
    : statusCode(sc)
{ }

QHttpServerResponse::QHttpServerResponse(QHttpServerResponse &&other) noexcept
    : d_ptr(other.d_ptr.take())
{
}

QHttpServerResponse& QHttpServerResponse::operator=(QHttpServerResponse &&other) noexcept
{
    if (this == &other)
        return *this;

    qSwap(d_ptr, other.d_ptr);
    return *this;
}

QHttpServerResponse::QHttpServerResponse(
        const QHttpServerResponse::StatusCode statusCode)
    : QHttpServerResponse(QHttpServerLiterals::contentTypeXEmpty(),
                          QByteArray(),
                          statusCode)
{
}

QHttpServerResponse::QHttpServerResponse(const char *data)
    : QHttpServerResponse(QByteArray::fromRawData(data, qstrlen(data)))
{
}

QHttpServerResponse::QHttpServerResponse(const QString &data)
    : QHttpServerResponse(data.toUtf8())
{
}

QHttpServerResponse::QHttpServerResponse(const QByteArray &data)
    : QHttpServerResponse(QMimeDatabase().mimeTypeForData(data).name().toLocal8Bit(), data)
{
}

QHttpServerResponse::QHttpServerResponse(QByteArray &&data)
    : QHttpServerResponse(
            QMimeDatabase().mimeTypeForData(data).name().toLocal8Bit(),
            std::move(data))
{
}

QHttpServerResponse::QHttpServerResponse(const QJsonObject &data)
    : QHttpServerResponse(QHttpServerLiterals::contentTypeJson(),
                          QJsonDocument(data).toJson(QJsonDocument::Compact))
{
}

QHttpServerResponse::QHttpServerResponse(const QJsonArray &data)
    : QHttpServerResponse(QHttpServerLiterals::contentTypeJson(),
                          QJsonDocument(data).toJson(QJsonDocument::Compact))
{
}

QHttpServerResponse::QHttpServerResponse(const QByteArray &mimeType,
                                         const QByteArray &data,
                                         const StatusCode status)
    : d_ptr(new QHttpServerResponsePrivate(QByteArray(data), status))
{
    setHeader(QHttpServerLiterals::contentTypeHeader(), mimeType);
}

QHttpServerResponse::QHttpServerResponse(QByteArray &&mimeType,
                                         const QByteArray &data,
                                         const StatusCode status)
    : d_ptr(new QHttpServerResponsePrivate(QByteArray(data), status))
{
    setHeader(QHttpServerLiterals::contentTypeHeader(), std::move(mimeType));
}

QHttpServerResponse::QHttpServerResponse(const QByteArray &mimeType,
                                         QByteArray &&data,
                                         const StatusCode status)
    : d_ptr(new QHttpServerResponsePrivate(std::move(data), status))
{
    setHeader(QHttpServerLiterals::contentTypeHeader(), mimeType);
}

QHttpServerResponse::QHttpServerResponse(QByteArray &&mimeType,
                                         QByteArray &&data,
                                         const StatusCode status)
    : d_ptr(new QHttpServerResponsePrivate(std::move(data), status))
{
    setHeader(QHttpServerLiterals::contentTypeHeader(), std::move(mimeType));
}

QHttpServerResponse::~QHttpServerResponse()
{
}

QHttpServerResponse QHttpServerResponse::fromFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly))
        return QHttpServerResponse(StatusCode::NotFound);
    const QByteArray data = file.readAll();
    file.close();
    const QByteArray mimeType = QMimeDatabase().mimeTypeForFileNameAndData(fileName, data).name().toLocal8Bit();
    return QHttpServerResponse(mimeType, data);
}

QHttpServerResponse::QHttpServerResponse(QHttpServerResponsePrivate *d)
    : d_ptr(d)
{
    d->derived = true;
}

/*!
    Returns response body.
*/
QByteArray QHttpServerResponse::data() const
{
    Q_D(const QHttpServerResponse);
    return d->data;
}

QHttpServerResponse::StatusCode QHttpServerResponse::statusCode() const
{
    Q_D(const QHttpServerResponse);
    return d->statusCode;
}

/*!
    Returns HTTP "Content-Type" header.

    \note Default value is "text/html"
*/
QByteArray QHttpServerResponse::mimeType() const
{
    Q_D(const QHttpServerResponse);
    const auto res = d->headers.find(
            QHttpServerLiterals::contentTypeHeader());
    if (res == d->headers.end())
        return QHttpServerLiterals::contentTypeTextHtml();

    return res->second;
}

/*!
    Adds the HTTP header with name \a name and value \a value,
    does not override any previously set headers.
*/
void QHttpServerResponse::addHeader(QByteArray &&name, QByteArray &&value)
{
    Q_D(QHttpServerResponse);
    d->headers.emplace(std::move(name), std::move(value));
}

/*!
    Adds the HTTP header with name \a name and value \a value,
    does not override any previously set headers.
*/
void QHttpServerResponse::addHeader(QByteArray &&name, const QByteArray &value)
{
    Q_D(QHttpServerResponse);
    d->headers.emplace(std::move(name), value);
}

/*!
    Adds the HTTP header with name \a name and value \a value,
    does not override any previously set headers.
*/
void QHttpServerResponse::addHeader(const QByteArray &name, QByteArray &&value)
{
    Q_D(QHttpServerResponse);
    d->headers.emplace(name, std::move(value));
}

/*!
    Adds the HTTP header with name \a name and value \a value,
    does not override any previously set headers.
*/
void QHttpServerResponse::addHeader(const QByteArray &name, const QByteArray &value)
{
    Q_D(QHttpServerResponse);
    d->headers.emplace(name, value);
}

void QHttpServerResponse::addHeaders(QHttpServerResponder::HeaderList headers)
{
    for (auto &&header : headers)
        addHeader(header.first, header.second);
}

/*!
    Removes the HTTP header with name \a name.
*/
void QHttpServerResponse::clearHeader(const QByteArray &name)
{
    Q_D(QHttpServerResponse);
    d->headers.erase(name);
}

/*!
    Removes all HTTP headers.
*/
void QHttpServerResponse::clearHeaders()
{
    Q_D(QHttpServerResponse);
    d->headers.clear();
}

/*!
    Sets the HTTP header with name \a name and value \a value,
    overriding any previously set headers.
*/
void QHttpServerResponse::setHeader(QByteArray &&name, QByteArray &&value)
{
    clearHeader(name);
    addHeader(std::move(name), std::move(value));
}

/*!
    Sets the HTTP header with name \a name and value \a value,
    overriding any previously set headers.
*/
void QHttpServerResponse::setHeader(QByteArray &&name, const QByteArray &value)
{
    clearHeader(name);
    addHeader(std::move(name), value);
}

/*!
    Sets the HTTP header with name \a name and value \a value,
    overriding any previously set headers.
*/
void QHttpServerResponse::setHeader(const QByteArray &name, QByteArray &&value)
{
    clearHeader(name);
    addHeader(name, std::move(value));
}

/*!
    Sets the HTTP header with name \a name and value \a value,
    overriding any previously set headers.
*/
void QHttpServerResponse::setHeader(const QByteArray &name, const QByteArray &value)
{
    clearHeader(name);
    addHeader(name, value);
}

/*!
    Sets the headers \a headers, overriding any previously set headers.
*/
void QHttpServerResponse::setHeaders(QHttpServerResponder::HeaderList headers)
{
    for (auto &&header : headers)
        setHeader(header.first, header.second);
}

/*!
    Returns true if the response contains an HTTP header with name \a name,
    otherwise returns false.
*/
bool QHttpServerResponse::hasHeader(const QByteArray &header) const
{
    Q_D(const QHttpServerResponse);
    return d->headers.find(header) != d->headers.end();
}

/*!
    Returns true if the response contains an HTTP header with name \a name and
    with value \a value, otherwise returns false.
*/
bool QHttpServerResponse::hasHeader(const QByteArray &name,
                                    const QByteArray &value) const
{
    Q_D(const QHttpServerResponse);
    auto range = d->headers.equal_range(name);

    auto condition = [&value] (const std::pair<QByteArray, QByteArray> &pair) {
        return pair.second == value;
    };

    return std::find_if(range.first, range.second, condition) != range.second;
}

/*!
    Returns values of the HTTP header with name \a name
*/
QVector<QByteArray> QHttpServerResponse::headers(const QByteArray &name) const
{
    Q_D(const QHttpServerResponse);

    QVector<QByteArray> results;
    auto range = d->headers.equal_range(name);

    for (auto it = range.first; it != range.second; ++it)
        results.append(it->second);

    return results;
}

/*!
    Writes HTTP response into QHttpServerResponder \a responder.
*/
void QHttpServerResponse::write(QHttpServerResponder &&responder) const
{
    Q_D(const QHttpServerResponse);
    if (responder.socket()->state() != QAbstractSocket::ConnectedState)
        return;

    responder.writeStatusLine(d->statusCode);

    for (auto &&header : d->headers)
        responder.writeHeader(header.first, header.second);

    responder.writeHeader(QHttpServerLiterals::contentLengthHeader(),
                          QByteArray::number(d->data.size()));

    responder.writeBody(d->data);
}

QT_END_NAMESPACE
